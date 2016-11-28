#include "memorypool.h"
#include "environment.h"
#include "evaluator.h"

#include <vector>
#include <cstring>
#include <iostream>
#include <algorithm>

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

void collectGarbage()
{
    // Mark & Sweep
    size_t collected = 0;
    std::vector<bool> used(allocd.size(), false);

    for (Environment *e : Evaluator::environments()) {
        for (const AVal &val : e->values) {
            auto it = std::find_if(allocd.begin(), allocd.end(), [&](AVal::Data *v) {
                return v == val.data;
            });
            if (it != allocd.end()) {
                used[std::distance(allocd.begin(), it)] = true;
            }
        }
    }

    for (size_t i = 0; i < used.size(); ++i) {
        if (!used[i]) {
            delete allocd[i];
            allocd[i] = nullptr;
            collected++;
        }
    }

    std::cout << "------ GARBAGE COLLECTOR ------" << std::endl;
    std::cout << "   Objects before:    " << allocd.size() << std::endl;
    std::cout << "   Objects collected: " << collected << std::endl;
    std::cout << "   Objects after:     " << allocd.size() - collected << std::endl;
    std::cout << "-------------------------------" << std::endl;

    std::vector<AVal::Data*> condensed;
    condensed.reserve(allocd.size() - collected);
    for (AVal::Data *d : allocd) {
        if (d) {
            condensed.push_back(d);
        }
    }
    allocd = condensed;
}

} // namespace MemoryPool
