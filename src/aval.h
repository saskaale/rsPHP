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

    AVal() {};
    AVal(int value): type(INT), value(value){};
    AVal(bool value): type(BOOL), value(value) {};
    AVal(double value): type(DOUBLE), fValue(value){};
    AVal(const char *value): type(STRING), str(strdup(value)) {};
    AVal(AVal *arr, size_t size): type(ARRAY), arr(arr), arrsize(size) {};
    AVal(Ast::Function* func): type(FUNCTION), func(func){};

    int toInt() const;
    bool toBool() const;
    double toDouble() const;
    Ast::Function *toFunction() const;
    const char *toString() const;

    AVal convertTo(Type t) const;

    void cleanup();

    Type type;
    union {
        double fValue;
        int value;
        Ast::Function *func;
        char *str;
        struct {
            AVal *arr;
            size_t arrsize;
        };
    };
};
