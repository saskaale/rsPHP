#pragma once


class AVal;

class Environment;


#include "parser.h"

typedef AVal (*BuiltinCall)(Ast::ExpressionList *v, Environment* envir);

class AVal
{
public:
    enum Type {
        UNDEFINED = 0,
        REFERENCE,
        INT,
        BOOL,
        DOUBLE,
        STRING,
        ARRAY,
        FUNCTION,
        FUNCTION_BUILTIN
    };

    struct Array {
        AVal *array = nullptr;
        size_t count = 0;
        size_t allocd = 0;
        void *mem = nullptr;
    };

    AVal();
    AVal(AVal *value);
    AVal(int value);
    AVal(bool value);
    AVal(double value);
    AVal(const char *value, bool isThrown = false);
    AVal(BuiltinCall value);
    AVal(Array *value);
    AVal(Ast::Function *value);

    Type type() const;
    bool isWritable() const;
    const char* typeStr() const;
    AVal dereference() const;

    bool isUndefined() const;
    bool isReference() const;
    bool isInt() const;
    bool isBool() const;
    bool isDouble() const;
    bool isString() const;
    bool isArray() const;
    bool isFunction() const;
    bool isBuiltinFunction() const;
    bool isThrown() const;

    void markThrown(bool is = true);

    AVal *toReference() const;
    int toInt() const;
    bool toBool() const;
    double toDouble() const;
    Ast::Function *toFunction() const;
    BuiltinCall toBuiltinFunction() const;
    const char *toString() const;
    Array *toArray() const;

    AVal convertTo(Type t) const;

    bool _thrown = false;
    Type _type = UNDEFINED;
    union {
        AVal *referenceValue;
        int intValue;
        bool boolValue;
        double doubleValue;
        Ast::Function *functionValue;
        BuiltinCall builtinFunctionValue;
        char *stringValue;
        Array *arrayValue;
    };
};
