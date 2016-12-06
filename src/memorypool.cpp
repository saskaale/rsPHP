#include "memorypool.h"
#include "environment.h"
#include "evaluator.h"

#include <vector>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <list>

namespace MemoryPool
{


#define MASKUNSET(t, mask) ((t)&=(~0 ^ mask))
#define MASKSET(t, mask) ((t)|=(mask))
#define HASMASK(t, mask) ((t)&(mask))


std::list<MemChunk*> allocd;
bool memDirty = false;

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



void checkCollectGarbage(){
    if(memDirty){
      collectGarbage();
    }
}

void *alloc(size_t size, void **memchunk)
{
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

      //mark memory as dirty and allocate new chunk
      memDirty = true;
      allocd.push_back(freechunk = new MemChunk());

    }else{

      //mark memory as dirty, when there is less than 10 empty values in the last memory chunk
      auto nextit = it;
      ++nextit;
      if(nextit == allocd.end()){
        if(chunks < 5){
          if((*it)->freeCnt <= 10){
            memDirty = true;
          }
        }else{
          if((*it)->freeCnt <= MEMCHUNK_SIZE/2){
            memDirty = true;
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
    return d;
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

int poolSize(){
    int s = 0;
    for(auto m: allocd){
      s+= MEMCHUNK_SIZE - m->freeCnt;
    }
    return s;
}


inline static void DFSMark(const AVal& val){
    if(!val.isArray() && !val.isString())
      return;

    void *m = nullptr;
    if (val.isArray()) {
        m = val.arrayValue->mem;
    } else if (val.isString()) {
        m = val.stringValue->mem;
    }

    MemChunk::Data* s = (MemChunk::Data*) m;
    if(s == nullptr)
      return;

    if(HASMASK(s->flags, MemChunk::MARKED))
      return;

    MASKSET(s->flags, MemChunk::MARKED);


    //deep recursion to mark each elements of array
    if(val.isArray()){
      for(int i = 0; i < val.toArray()->allocd; i++){
        DFSMark(val.toArray()->array[i]);
      }
    }
}

void collectGarbage( bool silent )
{

    int size;
    if(!silent)
        size = poolSize();

    // Mark & Sweep
    size_t collected = 0;

    for (Environment *e : Evaluator::environments()) {
        for (auto it : e->keys) {
          DFSMark(it.second);
        }
    }

    for(auto m: allocd){

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
    }

    memDirty = false;

    if(!silent){
        std::cout << "------ GARBAGE COLLECTOR ------" << std::endl;
        std::cout << "   Objects before:    " << size << std::endl;
        std::cout << "   Objects collected: " << collected << std::endl;
        std::cout << "   Objects after:     " << size - collected << std::endl;
        std::cout << "-------------------------------" << std::endl;
    }
}

} // namespace MemoryPool
