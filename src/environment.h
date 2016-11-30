#pragma once

class Environment;

#include "ast.h"
#include "aval.h"

#include <string>
#include <unordered_map>

class Environment
{
public:
    enum State {
        Normal = 1,
        ReturnCalled = 2,
        BreakCalled = 4,
        ContinueCalled = 8,
        FlowInterrupted = ReturnCalled | BreakCalled | ContinueCalled
    };

    explicit Environment(Environment *parent = nullptr);
    ~Environment();

    Environment *copy() const;

    AVal get(Ast::Variable *v);
    bool has(Ast::Variable *v) const;
    void set(Ast::Variable *v, const AVal &val);

    AVal getFunction(const std::string &key);
    bool hasFunction(const std::string &key) const;
    void setFunction(const std::string &key, const AVal &val);

    Environment *parent;
    std::vector<AVal> values;
    std::unordered_map<std::string, int> keys;

    AVal returnValue;
    State state = Normal;

private:
    AVal get(const std::string &key);
    bool has(const std::string &key) const;
    void set(const std::string &key, const AVal &val);
};

