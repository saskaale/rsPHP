#include "memorypool.h"

#include <vector>

namespace MemoryPool
{

std::vector<AVal::Data*> allocd;

AVal::Data *alloc()
{
    AVal::Data *a = new AVal::Data;
    allocd.push_back(a);
    return a;
}

void cleanup()
{
    for (AVal::Data *a : allocd) {
        delete a;
    }
}

} // namespace MemoryPool
