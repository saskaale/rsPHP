#pragma once

#include "aval.h"

namespace MemoryPool
{

AVal::Data *alloc();
void cleanup();

} // namespace MemoryPool
