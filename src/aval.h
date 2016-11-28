#pragma once

#include "parser.h"

#include <cstring>
#include <cstdlib>
#include <sstream>

class AVal
{
public:
    enum Type {
        INT,
        BOOL,
        DOUBLE,
        STRING,
        ARRAY,
        FUNCTION
    };

    AVal();
    AVal(int value);
    AVal(bool value);
    AVal(double value);
    AVal(const char *value);
    AVal(AVal *arr, size_t size);
    AVal(Ast::Function *value);

    int toInt() const;
    bool toBool() const;
    double toDouble() const;
    Ast::Function *toFunction() const;
    const char *toString() const;

    AVal convertTo(Type t) const;

    void cleanup();

    Type type;
    union {
        int intValue;
        bool boolValue;
        double doubleValue;
        Ast::Function *functionValue;
        char *stringValue;
        struct {
            AVal *arr;
            size_t arrsize;
        };
    };
};
