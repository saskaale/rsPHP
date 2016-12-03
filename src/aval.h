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
        REFERENCE,
        INT,
        BOOL,
        DOUBLE,
        STRING,
        ARRAY,
        FUNCTION,
        FUNCTION_BUILTIN
    };

    AVal();
    AVal(AVal *value);
    AVal(int value);
    AVal(bool value);
    AVal(double value);
    AVal(const char *value);
    AVal(BuiltinCall value);
    AVal(AVal *arr, size_t size);
    AVal(Ast::Function *value);

    Type type() const;
    bool isWritable() const;
    const char* typeStr() const;

    bool isUndefined() const;
    bool isReference() const;
    bool isInt() const;
    bool isBool() const;
    bool isDouble() const;
    bool isString() const;
    bool isArray() const;
    bool isFunction() const;
    bool isBuiltinFunction() const;

    AVal *toReference() const;
    int toInt() const;
    bool toBool() const;
    double toDouble() const;
    Ast::Function *toFunction() const;
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
            Ast::Function *functionValue;
            BuiltinCall builtinFunction;
            char *stringValue;
            struct {
                AVal *arr;
                size_t arrsize;
            };
        };
    } *data = nullptr;
};
