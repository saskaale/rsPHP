#pragma once

#include <cassert>

#define X_ASSERT(x) assert(x)
#define X_UNREACHABLE() X_ASSERT(false && "X_UNREACHABLE was reached");
