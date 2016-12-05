#include "aval.h"
#include "common.h"
#include "memorypool.h"

#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cstddef>
#include <sstream>

AArray emptyArray;

static AString *rstrdup(const char *str)
{
    void *mem;
    size_t size = strlen(str) + 1;
    AString *out = (AString*)MemoryPool::alloc(AString::allocSize(size), &mem);
    strncpy(out->string, str, size);
    out->mem = mem;
    return out;
}

AVal::AVal()
    : _type(UNDEFINED)
{
}

AVal::AVal(AVal *value)
    : _type(REFERENCE)
    , referenceValue(value)
{
}

AVal::AVal(int value)
    : _type(INT)
{
    intValue = value;
}

AVal::AVal(bool value)
    : _type(BOOL)
{
    boolValue = value;
}

AVal::AVal(char value)
    : _type(CHAR)
{
    charValue = value;
}

AVal::AVal(double value)
    : _type(DOUBLE)
{
    doubleValue = value;
}

AVal::AVal(const char *value)
    : _type(STRING)
{
    stringValue = rstrdup(value);
}

AVal::AVal(AArray *value)
    : _type(ARRAY)
{
    arrayValue = value;
}

AVal::AVal(Ast::Function *value)
    : _type(FUNCTION)
{
    functionValue = value;
}

AVal::AVal(BuiltinCall value)
    : _type(FUNCTION_BUILTIN)
{
    builtinFunctionValue = value;
}

AVal::Type AVal::type() const
{
    return _type;
}

bool AVal::isWritable() const
{
    return !isUndefined();
}

const char *AVal::typeStr() const
{
    static const char *tNames[] = {
        "undefined",
        "reference",
        "int",
        "bool",
        "double",
        "string",
        "array",
        "function",
        "function" //FUNCTION_BUILTIN
    };
    return tNames[(int)type()];
}

AVal AVal::copy() const
{
    switch (type()) {
    case STRING:
        return toString();

    case ARRAY: {
        void *mem;
        size_t size =AArray::allocSize(arrayValue->count);
        AArray *a = (AArray*)MemoryPool::alloc(size, &mem);
        memcpy(a, arrayValue, size);
        a->mem = mem;
        return a;
    }

    default:
        return *this;
    }
}

AVal AVal::dereference() const
{
    if (isReference()) {
        return *toReference();
    }
    return *this;
}

bool AVal::isUndefined() const
{
    return _type == UNDEFINED;
}

bool AVal::isReference() const
{
    return _type == REFERENCE;
}

bool AVal::isInt() const
{
    return _type == INT;
}

bool AVal::isBool() const
{
    return _type == BOOL;
}

bool AVal::isChar() const
{
    return _type == CHAR;
}

bool AVal::isDouble() const
{
    return _type == DOUBLE;
}

bool AVal::isString() const
{
    return _type == STRING;
}

bool AVal::isArray() const
{
    return _type == ARRAY;
}

bool AVal::isFunction() const
{
    return _type == FUNCTION;
}

bool AVal::isBuiltinFunction() const
{
    return _type == FUNCTION_BUILTIN;
}

bool AVal::isThrown() const
{
    return _thrown;
}

void AVal::markThrown(bool is)
{
    _thrown = is;
}

AVal *AVal::toReference() const
{
    return convertTo(REFERENCE).referenceValue;
}

int AVal::toInt() const
{
    return convertTo(INT).intValue;
}

bool AVal::toBool() const
{
    return convertTo(BOOL).boolValue;
}

char AVal::toChar() const
{
    return convertTo(CHAR).charValue;
}

double AVal::toDouble() const
{
    return convertTo(DOUBLE).doubleValue;
}

Ast::Function *AVal::toFunction() const
{
    return convertTo(FUNCTION).functionValue;
}

const char *AVal::toString() const
{
    return convertTo(STRING).stringValue->string;
}

AArray *AVal::toArray() const
{
    return convertTo(ARRAY).arrayValue;
}

BuiltinCall AVal::toBuiltinFunction() const
{
    return convertTo(FUNCTION_BUILTIN).builtinFunctionValue;
}

AVal AVal::convertTo(Type t) const
{
    if (type() == t) {
        return *this;
    }

    if (t == UNDEFINED) {
        return AVal();
    }

    if (t == REFERENCE) {
        // Cannot convert references
        X_UNREACHABLE();
    }

    switch (type()) {
    case UNDEFINED:
        switch (t) {
        case BOOL:
            return false;
        case DOUBLE:
            return NAN;
        case FUNCTION:
        case FUNCTION_BUILTIN:
            return static_cast<Ast::Function*>(nullptr);
        case STRING:
            return "[undefined]";
        case ARRAY:
            return &emptyArray;
        default:
            X_UNREACHABLE();
        }

    case REFERENCE:
        return dereference().convertTo(t);

    case INT:
        switch (t) {
        case BOOL:
            return bool(intValue);
        case DOUBLE:
            return double(intValue);
        case FUNCTION:
        case FUNCTION_BUILTIN:
            return static_cast<Ast::Function*>(nullptr);
        case STRING: {
            std::stringstream ss;
            ss << intValue;
            return ss.str().c_str();
        }
        case ARRAY:
            return &emptyArray;
        default:
            X_UNREACHABLE();
        }

    case BOOL:
        switch (t) {
        case INT:
            return int(boolValue);
        case DOUBLE:
            return double(boolValue);
        case FUNCTION:
        case FUNCTION_BUILTIN:
            return static_cast<Ast::Function*>(nullptr);
        case STRING:
            return boolValue ? "true" : "false";
        case ARRAY:
            return &emptyArray;
        default:
            X_UNREACHABLE();
        }

    case CHAR:
        switch (t) {
        case INT:
            return int(charValue);
        case DOUBLE:
            return double(charValue);
        case FUNCTION:
        case FUNCTION_BUILTIN:
            return static_cast<Ast::Function*>(nullptr);
        case STRING: {
            char buf[] = { charValue, '\0' };
            return buf;
        }
        case ARRAY:
            return &emptyArray;
        default:
            X_UNREACHABLE();
        }

    case DOUBLE:
        switch (t) {
        case INT:
            return int(doubleValue);
        case BOOL:
            return bool(doubleValue);
        case FUNCTION:
        case FUNCTION_BUILTIN:
            return static_cast<Ast::Function*>(nullptr);
        case STRING: {
            std::stringstream ss;
            ss << doubleValue;
            return ss.str().c_str();
        }
        case ARRAY:
            return &emptyArray;
        default:
            X_UNREACHABLE();
        }

    case FUNCTION:
        switch (t) {
        case INT:
        case BOOL:
        case DOUBLE:
        case FUNCTION_BUILTIN:
            return AVal(functionValue ? true : false).convertTo(t);
        case STRING:
            return "[function]";
        case ARRAY:
            return &emptyArray;
        default:
            X_UNREACHABLE();
        }

    case FUNCTION_BUILTIN:
        switch (t) {
        case INT:
        case BOOL:
        case DOUBLE:
        case FUNCTION:
            return AVal(builtinFunctionValue ? true : false).convertTo(t);
        case STRING:
            return "[builtin function]";
        case ARRAY:
            return &emptyArray;
        default:
            X_UNREACHABLE();
        }

    case STRING:
        switch (t) {
        case INT:
            return atoi(stringValue->string);
        case BOOL:
            return strlen(stringValue->string) > 0;
        case DOUBLE:
            return atof(stringValue->string);
        case FUNCTION:
        case FUNCTION_BUILTIN:
            return static_cast<Ast::Function*>(nullptr);
        case ARRAY:
            return &emptyArray;
        default:
            X_UNREACHABLE();
        }

    case ARRAY:
        switch (t) {
        case INT:
        case BOOL:
        case DOUBLE:
        case FUNCTION:
        case FUNCTION_BUILTIN:
            return AVal(0).convertTo(t);
        case STRING:
            return "[array]";
        default:
            X_UNREACHABLE();
        }

    default:
        X_UNREACHABLE();
    }

    return 0;
}

#if 0
AVal::Data::~Data()
{
    if (type == FUNCTION) {
        delete functionValue;
    } else if (type == STRING) {
        MemoryPool::strfree(stringValue);
    } else if (type == ARRAY) {
        delete []arrayValue.array;
    }
}


AVal::Data::Data(void* memmgr) :
  memmgr(memmgr)
{
}

AVal::Data::Data() :
  memmgr(nullptr)
{
}
#endif
