#pragma once


class AVal;

class Environment;


#include "parser.h"

typedef AVal (*BuiltinCall)(const std::vector<Ast::Expression*> &, Environment *);

struct AArray;
struct AString;

class AVal
{
public:
    enum Type {
        UNDEFINED = 0,
        REFERENCE,
        INT,
        BOOL,
        CHAR,
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
    AVal(char value);
    AVal(double value);
    AVal(const char *value);
    AVal(BuiltinCall value);
    AVal(AArray *value);
    AVal(Ast::Function *value);

    Type type() const;
    bool isWritable() const;
    const char* typeStr() const;

    AVal copy() const;
    AVal dereference() const;

    bool isUndefined() const;
    bool isReference() const;
    bool isInt() const;
    bool isBool() const;
    bool isChar() const;
    bool isDouble() const;
    bool isString() const;
    bool isArray() const;
    bool isFunction() const;
    bool isBuiltinFunction() const;
    bool isConst() const;
    bool isThrown() const;

    void markConst(bool is = true);
    void markThrown(bool is = true);

    AVal *toReference() const;
    int toInt() const;
    bool toBool() const;
    char toChar() const;
    double toDouble() const;
    Ast::Function *toFunction() const;
    BuiltinCall toBuiltinFunction() const;
    const char *toString() const;
    AArray *toArray() const;

    AVal convertTo(Type t) const;

    bool _const = false;
    bool _thrown = false;
    Type _type = UNDEFINED;
    union {
        AVal *referenceValue;
        int intValue;
        bool boolValue;
        char charValue;
        double doubleValue;
        Ast::Function *functionValue;
        BuiltinCall builtinFunctionValue;
        AString *stringValue;
        AArray *arrayValue;
    };
};

struct AArray {
    size_t count = 0;
    size_t allocd = 0;
    void *mem = nullptr;
    AVal array[1];

    static size_t allocSize(size_t elements) {
        return sizeof(AArray) + sizeof(AVal) * (elements - 1);
    }
};

struct AString {
    void *mem = nullptr;
    char string[1];

    static size_t allocSize(size_t elements) {
        return sizeof(AString) + sizeof(char) * (elements - 1);
    }
};
