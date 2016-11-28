#pragma once

#include "aval.h"

namespace MemoryPool
{

AVal::Data *alloc();
char *strdup(const char *s);
void strfree(char *s);
void cleanup();
void collectGarbage();

} // namespace MemoryPool
