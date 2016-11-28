#pragma once

class Environment;

#include "ast.h"
#include "aval.h"

#include <string>
#include <unordered_map>

class Environment
{
public:
    explicit Environment(Environment *parent = nullptr);
    ~Environment();

    AVal get(Ast::Variable *v);
    bool has(Ast::Variable *v) const;
    void set(Ast::Variable *v, const AVal &val);

    AVal getFunction(const std::string &key);
    bool hasFunction(const std::string &key) const;
    void setFunction(const std::string &key, const AVal &val);

private:
    AVal get(const std::string &key);
    bool has(const std::string &key) const;
    void set(const std::string &key, const AVal &val);

    Environment *parent;
    std::vector<AVal> values;
    std::unordered_map<std::string, int> keys;
};

