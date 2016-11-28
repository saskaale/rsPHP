#include "memorypool.h"

#include <vector>
#include <cstring>

namespace MemoryPool
{

std::vector<AVal::Data*> allocd;

AVal::Data *alloc()
{
    AVal::Data *a = new AVal::Data;
    allocd.push_back(a);
    return a;
}

char *strdup(const char *s)
{
    return ::strdup(s);
}

void strfree(char *s)
{
    free(s);
}

void cleanup()
{
    for (AVal::Data *a : allocd) {
        delete a;
    }
}

} // namespace MemoryPool
