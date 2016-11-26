#pragma once

class Environment;

#include "ast.h"
#include "evaluator.h"
#include "parser.hpp"
#include <string>
#include <map>

class Environment{
    Environment* parent;
    std::vector<AVal>     values;
    std::map<std::string, int>  keys;
    public:
        Environment(Environment* parent = nullptr);
        AVal get(const std::string& key);
        bool has(const std::string& key) const;
        void set(const std::string& key, const AVal& val);
};

