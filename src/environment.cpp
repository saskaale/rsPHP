#include "common.h"
#include "environment.h"
#include "ast.h"

Environment::Environment(Environment* parent)
    : parent(parent)
{
}


void Environment::set(const std::string& key, const Ast::Value& val)
{
    int nextidx = values.size();
    keys[key] = nextidx;
    values.push_back(val);
}

bool Environment::has(const std::string& key) const
{
    if(keys.find(key) != keys.end())
      return true;
    if(parent != nullptr)
      return parent->has(key);
    return false;
}

Ast::Value* Environment::get(const std::string& key)
{
    if(keys.find(key) != keys.end())
      return &values[keys[key]];
    X_ASSERT(false && "symbol lookup error in global scope");
    return parent->get(key);
}

