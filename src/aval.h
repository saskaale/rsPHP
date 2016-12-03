#pragma once

#include "parser.h"

class AVal;

class Environment;

typedef AVal (*BuiltinCall)(Ast::ExpressionList *v, Environment* envir);

class AVal
{
public:
    enum Type {
        UNDEFINED,
        INT,
        BOOL,
        DOUBLE,
        STRING,
        ARRAY,
        FUNCTION,
        FUNCTION_BUILTIN
    };

    struct Function {
        Ast::Function *function;
        Environment *environment;
    };

    AVal();
    AVal(int value);
    AVal(bool value);
    AVal(double value);
    AVal(const char *value);
    AVal(BuiltinCall value);
    AVal(AVal *arr, size_t size);
    AVal(Ast::Function *value, Environment *environment = nullptr);

    Type type() const;
    const char* typeStr() const;

    int toInt() const;
    bool toBool() const;
    double toDouble() const;
    Function toFunction() const;
    BuiltinCall toBuiltinFunction() const;
    const char *toString() const;

    AVal convertTo(Type t) const;

    Type _type;
    void* rawdata;

    struct Data {
        Data();
        Data(void* memmgr);
        ~Data();
        Type type;
        void* memmgr;
        union {
            int intValue;
            bool boolValue;
            double doubleValue;
            Function functionValue;
            BuiltinCall builtinFunction;
            char *stringValue;
            struct {
                AVal *arr;
                size_t arrsize;
            };
        };
    } *data = nullptr;
};
