#pragma once

class Environment;

#include "ast.h"
#include "parser.hpp"
#include <string>
#include <map>

class Environment{
    std::map<std::string, Ast::Node*> entries;
    public:
        Ast::Node* get(const char* key);
        void set(const char* key, Ast::Node* val);
};

