#pragma once

#include "aval.h"

namespace MemoryPool
{

static const int MEMCHUNK_SIZE = 1000;


struct MemChunk{
    MemChunk();

    static const int FREE = 0;
    static const int USED = 1;
    static const int MARKED = 1<<1;

    int freeCnt;

    struct Data{
      Data();
      void *d;
      char flags;
    } d[MEMCHUNK_SIZE];
};

void *alloc(size_t size, void **memchunk);
void cleanup();
void collectGarbage(bool silent = true, bool singlestep = false);

} // namespace MemoryPool
