#include "common.h"
#include "environment.h"
#include "ast.h"

Environment::Environment(Environment* parent)
    : parent(parent)
{
}

Environment::~Environment()
{
}

Environment *Environment::copy() const
{
    Environment *c = new Environment(parent);
//    c->values = values;
    c->keys = keys;
    c->returnValue = returnValue;
    c->state = state;
    return c;
}

AVal Environment::get(Ast::Variable *v)
{
    return get(v->name);
}

bool Environment::has(Ast::Variable *v) const
{
    return has(v->name);
}

void Environment::set(Ast::Variable *v, const AVal &val)
{
    set(v->name, val);
}

AVal Environment::getFunction(const std::string &key)
{
    return get(key);
}

bool Environment::hasFunction(const std::string &key) const
{
    return has(key);
}

void Environment::setFunction(const std::string &key, const AVal &val)
{
    set(key, val);
}

AVal Environment::get(const std::string& key)
{
    if(keys.find(key) != keys.end())
      return keys[key];
    if(parent)
      return parent->get(key);
    return AVal();
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
//    int nextidx = values.size();
//    keys[key] = nextidx;
//    values.push_back(val);
    keys[key] = val;
}
