#ifndef ___ENVIRONMENT_H_414253451242412
#define ___ENVIRONMENT_H_414253451242412

class Environment;

#include "ast.h"
#include "parser.hpp"
#include <string>
#include <map>

class Environment{
    std::map<std::string, nodeType*> entries;
    public:
        nodeType* get(const char* key);
        void set(const char* key, nodeType* val);
};

#endif