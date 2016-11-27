#include "common.h"
#include "environment.h"
#include "ast.h"

Environment::Environment(Environment* parent)
    : parent(parent)
{
}

Environment::~Environment()
{
    for (AVal &v : values) {
        v.cleanup();
    }
}

void Environment::set(const std::string& key, const AVal& val)
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

AVal Environment::get(const std::string& key)
{
    if(keys.find(key) != keys.end())
      return values[keys[key]];
    X_ASSERT(false && "symbol lookup error in global scope");
    return parent->get(key);
}

