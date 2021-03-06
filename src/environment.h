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

    AVal &get(const std::string &key);
    bool has(const std::string &key) const;
    void set(const std::string &key, const AVal &val);

    Environment *parent;
    std::unordered_map<std::string, AVal> keys;

    AVal returnValue;
    State state = Normal;
};

