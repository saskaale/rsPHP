#include "environment.h"
#include "ast.h"

void Environment::set(const char* key, const Ast::Value& val){
    int nextidx = values.size();
    keys[key] = nextidx;
    values.push_back(val);
}

Ast::Value* Environment::get(const char* key){
    return &values[keys[key]];
}

