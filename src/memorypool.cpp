#include "memorypool.h"
#include "environment.h"
#include "evaluator.h"
#include "common.h"

#include <vector>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <list>
#include <queue>

namespace MemoryPool
{


#define MASKUNSET(t, mask) ((t)&=(~0 ^ mask))
#define MASKSET(t, mask) ((t)|=(mask))
#define HASMASK(t, mask) ((t)&(mask))


std::list<MemChunk*> allocd;
std::queue<const AVal*> GCqueue;
enum GCstateType {OK = 0, DIRTY, INITMARK, BFSMARK, INITSWEEP, SWEEPSTEP, DONE};
GCstateType GCstate = OK;
static const int GC_MAX_STEP = 0.005;  //in s
static const int GC_MIN_WAIT = 0.03;  //in s

MemChunk::MemChunk():
    freeCnt(MEMCHUNK_SIZE)
{
};

MemChunk::Data::Data()
{
    d = nullptr;
    flags = 0;
};



AVal PRINTVAL(const AVal& printV){
    switch (printV.type()) {
    case AVal::STRING:
        return printf("\"%s\"\n", printV.toString());
    default:
        return printf("%s\n", printV.toString());
    }
};



void markDirty(){
    if(GCstate == OK){
        GCstate = DIRTY;
    }
}


static inline int findChunkFreePos(MemChunk* freechunk){
  
    //try 10 times random position
    #pragma unroll
    for(int i = 0; i < 10; i++){
        int pos = ((unsigned int)std::rand())%MEMCHUNK_SIZE;
        if(pos < 0){
            pos = 0;
        }
        if(freechunk->d[pos].d == nullptr){
            return pos;
        }
    }
    
    //do linear lookup
    const int startpos = ((unsigned int)std::rand())%MEMCHUNK_SIZE;
    unsigned int i = startpos;
    do{
        if(freechunk->d[i].d == nullptr){
            return i;
        }
        i=(i+1)%MEMCHUNK_SIZE;
    }while(i != startpos);
    
    X_UNREACHABLE();
}

static inline MemChunk* findFreeChunk(){
    //find free chunk
    MemChunk* freechunk = nullptr;
        
    //linear lokup in chunks
    auto it = allocd.begin();
    int chunks = 1;
    for(; it != allocd.end(); ++it){
        auto m = *it;
        chunks++;
        if(m->freeCnt<=0)
          continue;
        freechunk = m;
        break;
    }


    //no free chunk found
    if(freechunk==nullptr){
      markDirty();
      //mark memory as dirty and allocate new chunk
      allocd.push_back(freechunk = new MemChunk());
    }else{
      auto nextit = it;
      ++nextit;
      
      //HEURISTIC for marking memory as dirty
      //mark memory as dirty, when there is less than 10 empty values in the last memory chunk
      if(nextit == allocd.end()){
        if(chunks < 5){
          if((*it)->freeCnt <= 10){
            markDirty();
          }
        }else{
          if((*it)->freeCnt <= MEMCHUNK_SIZE/2){
            markDirty();
          }
        }
      }

    }
    
    
    return freechunk;
}


void *alloc(size_t size, void **memchunk)
{

    //find free Memory chunk
    MemChunk* freechunk = findFreeChunk();
  
    //find free position in chunk
    int freepos = findChunkFreePos( freechunk );


    freechunk->freeCnt--;

    void *d = calloc(1, size);
    *memchunk = &freechunk->d[freepos];
    freechunk->d[freepos].d = d;
    freechunk->d[freepos].flags = MemChunk::FREE;

    //actually doing GC >> must set new memory as marked
    if(GCstate != OK){
        MASKSET(freechunk->d[freepos].flags, MemChunk::MARKED);
    }
    

    //check if there is need to collect garbage
    collectGarbage();
    
    return d;
}

static inline double CPUTime(){
    return clock() / (double) CLOCKS_PER_SEC;
}

void cleanup()
{
    for (MemChunk *m : allocd) {
        for(int i = 0; i < MEMCHUNK_SIZE; i++){
            free(m->d[i].d);
        }
        delete m;
    }
    allocd.clear();
}

static inline int poolSize(){
    int s = 0;
    for(auto m: allocd){
        s+= MEMCHUNK_SIZE - m->freeCnt;
    }
    return s;
}


static inline MemChunk::Data* getUnvisitedMemChunk(const AVal& val){
    if(!&val)
        return nullptr;

    if(!val.isArray() && !val.isString())
        return nullptr;
    
    void *m = nullptr;
    if (val.isArray()) {
        m = val.arrayValue->mem;
    } else if (val.isString()) {
        m = val.stringValue->mem;
    }

    MemChunk::Data* s = (MemChunk::Data*) m;
    if(s == nullptr)
        return nullptr;

    if(HASMASK(s->flags, MemChunk::MARKED))
        return nullptr;

    return s;
}

inline static void Mark(const AVal& val, bool dfs = true){
    MemChunk::Data* s = getUnvisitedMemChunk(val);
    if(s == nullptr)
      return;
    
    MASKSET(s->flags, MemChunk::MARKED);

  
    if(val.dereference().isArray()){
      //deep recursion to mark each elements of array
        for(int i = 0; i < val.toArray()->allocd; i++){
            if(dfs){
                Mark(val.toArray()->array[i].dereference(), true);
            }else{
                AVal v = val.toArray()->array[i].dereference();

                MemChunk::Data* toq = getUnvisitedMemChunk(v);
                if(toq && !HASMASK(toq->flags, MemChunk::MARKED)){
                    MASKSET(toq->flags, MemChunk::MARKED);
                    GCqueue.push(&val);
                }
            }
        }
    }
}


// Mark & Sweep
double timeGCrawSpent = 0., timeGCStart = 0., lastGcEnd = 0.;
size_t collected = 0;
int chunks = 0;
int size;
bool silent;
std::list<MemoryPool::MemChunk*>::iterator curMemChunk;

void collectGarbage( bool s, bool whole)
{
    //just for skipping elapsed time-counters
    if(GCstate == OK)
      return;
    
    //does not elapsed enough time to rerun GC
    if(CPUTime() - lastGcEnd <= GC_MIN_WAIT){
      return;
    }
  
    double starttime = CPUTime();

    bool doing = true;
    while(doing){
        doing = false;
        switch(GCstate){
            case DIRTY: {
                silent = s;
                if(!silent)
                    size = poolSize();
                collected = 0;
                timeGCStart = CPUTime();
                timeGCrawSpent = 0.;

                GCstate = INITMARK;
                doing = true;
                break;
            }
            case INITMARK: {
                //we must iterate all local values on stack in the single step
                int locals = 0;

                //must create copy of the locals hash map, because there would be modified this map over there
                auto copiedLocalAVals = localAVals;
                for(AVal* v : copiedLocalAVals){
                  Mark(v->dereference(), true);
                  locals++;
                }

                int envirs = 0;
                int vals = 0;
                //push all available avals into the local stack
                for (Environment *e : Evaluator::environments()) {
                    envirs++;
                    for (auto it : e->keys) {
                      GCqueue.push(&it.second);
                      vals++;
                    }
                }

                GCstate = BFSMARK;
                doing = whole || CPUTime() - starttime < GC_MAX_STEP;
                break;
            }
            case BFSMARK: {
                bool do_bfs = true;          
                int steps = 0;

                while(!GCqueue.empty() && do_bfs){
                    Mark(GCqueue.front()->dereference(), false);
                    GCqueue.pop();

                    //check running time, each few steps
                    if(steps++%1000 == 0){
                        do_bfs = CPUTime() - starttime < GC_MAX_STEP;
                    }
                }

                doing = whole || do_bfs;
                if(GCqueue.empty()){
                    GCstate = INITSWEEP;
                }
                break;
            }
            case INITSWEEP: {
                curMemChunk = allocd.begin();
                chunks = 0;
                
                doing = whole || CPUTime() - starttime < GC_MAX_STEP;
                GCstate = SWEEPSTEP;
                break;
            }
            case SWEEPSTEP: {
                bool do_sweep = true;
                
                while(curMemChunk != allocd.end() && do_sweep){
                    MemChunk* m = *curMemChunk;

                    for(int i = 0; i < MEMCHUNK_SIZE; i++){
                        if(m->d[i].d == nullptr){
                          continue;
                        }

                        if(!HASMASK(m->d[i].flags, MemChunk::MARKED)){
                            m->d[i].flags = MemChunk::FREE;
                            free(m->d[i].d);
                            m->d[i].d = nullptr;

                            m->freeCnt++;
                            collected++;
                        }else{
                            MASKUNSET(m->d[i].flags, MemChunk::MARKED);
                        }
                    }
                  
                    chunks++;
                    curMemChunk++;
                  
                    do_sweep = CPUTime() - starttime < GC_MAX_STEP;
                }
                
                doing = whole || do_sweep;
                if(curMemChunk == allocd.end()){
                  GCstate = DONE;
                }
                break;
            }
            case DONE: {
                if(!silent){
                    printf("------ GARBAGE COLLECTOR ------\n");
                    printf("   Objects before:     %d\n", size);
                    printf("   Objects collected:  %ld\n", collected);
                    printf("   Objects after:      %ld ( %d blocks )\n", size - collected, chunks);
                    printf("   CPU Time elapsed:   %lf ( raw %lf )\n", CPUTime() - timeGCStart, timeGCrawSpent + (CPUTime() - starttime));
                    printf("-------------------------------\n");
                }
                lastGcEnd = CPUTime();
                GCstate = OK;
                break;
            }
        }
    }
    timeGCrawSpent += CPUTime() - starttime;
}

} // namespace MemoryPool
