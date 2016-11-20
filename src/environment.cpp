#include "environment.h"
#include "ast.h"

void Environment::set(const char* key, Ast::Node* val){
    entries[key] = val;
}

Ast::Node* Environment::get(const char* key){
    return entries[key];
}

