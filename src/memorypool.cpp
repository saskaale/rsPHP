#include "memorypool.h"
#include "environment.h"
#include "evaluator.h"

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
static const int GC_MAX_STEP = 0.05;  //in s
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


void *alloc(size_t size, void **memchunk)
{
    collectGarbage();
  
    // printf("MemoryPool::alloc()\n");

    //find free chunk
    MemChunk* freechunk = nullptr;

    //first step to check if there is free chunk, then try run garbage collector, and then second run to find if there is any free chunk
    int cnt = 0;

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


    if(freechunk==nullptr){
      markDirty();
      //mark memory as dirty and allocate new chunk
      allocd.push_back(freechunk = new MemChunk());
    }else{

      //mark memory as dirty, when there is less than 10 empty values in the last memory chunk
      auto nextit = it;
      ++nextit;
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


    //find free position in chunk
    int freepos = -1;
    for(int i = 0; i < MEMCHUNK_SIZE; i++){
      if(freechunk->d[i].d == nullptr){
        freepos = i;
        break;
      }
    }

    freechunk->freeCnt--;

    void *d = calloc(1, size);
    *memchunk = &freechunk->d[freepos];
    freechunk->d[freepos].d = d;
    freechunk->d[freepos].flags = MemChunk::FREE;
    if(GCstate != OK){
      //actually doing GC >> must set as marked
      MASKSET(freechunk->d[freepos].flags, MemChunk::MARKED);
    }
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
    if(val.isArray()){
      //deep recursion to mark each elements of array
      for(int i = 0; i < val.toArray()->allocd; i++){
        if(dfs){
          Mark(val.toArray()->array[i]);
        }else{
          AVal& v = val.toArray()->array[i];

          MemChunk::Data* toq = getUnvisitedMemChunk(v);
          if(toq && HASMASK(toq->flags, MemChunk::MARKED)){
            MASKSET(toq->flags, MemChunk::MARKED);
            GCqueue.push(&val);
          }
        }
      }
    }
}

inline static void DFSMark(const AVal& val){
    MemChunk::Data* s = getUnvisitedMemChunk(val);
    if(s == nullptr)
      return;
    
    MASKSET(s->flags, MemChunk::MARKED);
    Mark(val);
}


// Mark & Sweep
double timeGCrawSpent = 0., timeGCStart = 0., lastGcEnd = 0.;
size_t collected = 0;
int chunks = 0;
int size;
int silent;
std::list<MemoryPool::MemChunk*>::iterator curMemChunk;

void collectGarbage( int s, int whole)
{
    if(GCstate == OK)
      return;
    
    //does not elapsed enough time to rerun GC
    if(CPUTime() - lastGcEnd <= GC_MIN_WAIT){
      return;
    }
  
    s = false;
    
    double starttime = CPUTime();

    bool doing = true;
    while(doing){
//      printf("STEP %d\n", GCstate);

      doing = false;
      switch(GCstate){
        case DIRTY: {
//          printf("INIT\n");
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
          for(AVal* v : localAVals){
            DFSMark(*v);
          }
          

          //push all available avals into the local stack
          for (Environment *e : Evaluator::environments()) {
              for (auto it : e->keys) {
                GCqueue.push(&it.second);
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
            
            //doing it in BFS way
            Mark(GCqueue.front(), false);
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
              if(m->d[i].d == nullptr)
                continue;

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
            
            if(m->freeCnt >= MEMCHUNK_SIZE){
              //memchunk is empty >> delete it
              auto oldMemChunk = curMemChunk;
              ++curMemChunk;
              allocd.erase(oldMemChunk);
            }else{
              chunks++;
              ++curMemChunk;
            }
            
            do_sweep = CPUTime() - starttime < GC_MAX_STEP;
          }
          
          doing = whole || do_sweep;
          if(curMemChunk == allocd.end()){
            GCstate = DONE;
          }
          break;
        }
        case DONE: {
//            printf("DONE %d\n", silent);
          if(silent){
              std::cout << "------ GARBAGE COLLECTOR ------" << std::endl;
              std::cout << "   Objects before:     " << size << std::endl;
              std::cout << "   Objects collected:  " << collected << std::endl;
              std::cout << "   Objects after:      " << size - collected << " ( " << chunks << " blocks )" << std::endl;
              std::cout << "   CPU time elapsed:   " << CPUTime() - timeGCStart  <<"( raw " << timeGCrawSpent + (CPUTime() - starttime) << " )" << std::endl;
              std::cout << "-------------------------------" << std::endl;
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
