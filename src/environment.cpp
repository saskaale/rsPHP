#include "environment.h"
#include "ast.h"

void Environment::set(const char* key, nodeType* val){
    entries[key] = val;
}

nodeType* Environment::get(const char* key){
    return entries[key];
}

