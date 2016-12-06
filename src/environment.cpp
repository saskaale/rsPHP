#include "common.h"
#include "environment.h"
#include "ast.h"

AVal undefined;

Environment::Environment(Environment* parent)
    : parent(parent)
{
}

Environment::~Environment()
{
}

AVal &Environment::get(const std::string& key)
{
    if(keys.find(key) != keys.end())
      return keys[key];
    if(parent)
      return parent->get(key);
    return undefined;
}

bool Environment::has(const std::string& key) const
{
    if(keys.find(key) != keys.end())
      return true;
    if(parent)
      return parent->has(key);
    return false;
}

void Environment::set(const std::string& key, const AVal &val)
{
    keys[key] = val;
}
