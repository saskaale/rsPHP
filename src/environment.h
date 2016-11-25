#pragma once

class Environment;

#include "ast.h"
#include "parser.hpp"
#include <string>
#include <map>

class Environment{
    std::vector<Ast::Value>     values;
    std::map<std::string, int>  keys;
    public:
        Ast::Value* get(const char* key);
        void set(const char* key, const Ast::Value& val);
};

