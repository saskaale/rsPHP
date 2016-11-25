#pragma once

class Environment;

#include "ast.h"
#include "parser.hpp"
#include <string>
#include <map>

class Environment{
    Environment* parent;
    std::vector<Ast::Value>     values;
    std::map<std::string, int>  keys;
    public:
        Environment(Environment* parent = nullptr);
        Ast::Value* get(const std::string& key);
        bool has(const std::string& key) const;
        void set(const std::string& key, const Ast::Value& val);
};

