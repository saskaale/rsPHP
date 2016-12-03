#include "aval.h"
#include "common.h"
#include "memorypool.h"

#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cstddef>
#include <sstream>

AVal::AVal()
    : _type(UNDEFINED)
    , thrown(false)
{
}

AVal::AVal(AVal *value, bool thrown)
    : _type(REFERENCE)
    , thrown(thrown)
{
    data = reinterpret_cast<Data*>(value);
}

AVal::AVal(int value, bool thrown)
    : _type(INT)
    , thrown(thrown)
{
    data = reinterpret_cast<Data*>(value);
}

AVal::AVal(bool value, bool thrown)
    : _type(BOOL)
    , thrown(thrown)
{
    data = reinterpret_cast<Data*>(value);
}

AVal::AVal(double value, bool thrown)
    : _type(DOUBLE)
    , thrown(thrown)
    , data(MemoryPool::alloc())
{
    data->type = _type;
    data->doubleValue = value;
}

AVal::AVal(const char *value, bool thrown)
    : _type(STRING)
    , thrown(thrown)
    , data(MemoryPool::alloc())
{
    data->type = _type;
    data->stringValue = MemoryPool::strdup(value);
}

AVal::AVal(AVal *arr, int size, bool thrown)
    : _type(ARRAY)
    , thrown(thrown)
    , data(MemoryPool::alloc())
{
    data->type = _type;
    data->arr = arr;
    data->arrsize = size;
}

AVal::AVal(Ast::Function *value, bool thrown)
    : _type(FUNCTION)
    , thrown(thrown)
    , data(MemoryPool::alloc())
{
    data->type = _type;
    data->functionValue = value;
}

AVal::AVal(BuiltinCall value)
    : _type(FUNCTION_BUILTIN)
    , thrown(false)
{
    data = reinterpret_cast<Data*>(value);
}

AVal::Type AVal::type() const
{
    return _type;
}

bool AVal::isWritable() const
{
    return !isUndefined();
}

const char* AVal::typeStr() const
{
    static const char* const tNames[] = {
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
    return thrown;
}

void AVal::markThrown()
{
    thrown = true;
}

AVal *AVal::toReference() const
{
    return reinterpret_cast<AVal*>(convertTo(REFERENCE).data);
}

int AVal::toInt() const
{
    return reinterpret_cast<std::ptrdiff_t>(convertTo(INT).data);
}

bool AVal::toBool() const
{
    return reinterpret_cast<std::ptrdiff_t>(convertTo(BOOL).data);
}

double AVal::toDouble() const
{
    return convertTo(DOUBLE).data->doubleValue;
}

Ast::Function *AVal::toFunction() const
{
    return convertTo(FUNCTION).data->functionValue;
}

const char *AVal::toString() const
{
    return convertTo(STRING).data->stringValue;
}

BuiltinCall AVal::toBuiltinFunction() const
{
    return reinterpret_cast<BuiltinCall>(convertTo(FUNCTION_BUILTIN).data);
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
            return AVal(nullptr, 0);
        default:
            X_UNREACHABLE();
        }

    case REFERENCE:
        return toReference()->convertTo(t);

    case INT:
        switch (t) {
        case BOOL:
            return bool(toInt());
        case DOUBLE:
            return double(toInt());
        case FUNCTION:
        case FUNCTION_BUILTIN:
            return static_cast<Ast::Function*>(nullptr);
        case STRING: {
            std::stringstream ss;
            ss << toInt();
            return ss.str().c_str();
        }
        case ARRAY:
            return AVal(nullptr, 0);
        default:
            X_UNREACHABLE();
        }

    case BOOL:
        switch (t) {
        case INT:
            return int(toBool());
        case DOUBLE:
            return double(toBool());
        case FUNCTION:
        case FUNCTION_BUILTIN:
            return static_cast<Ast::Function*>(nullptr);
        case STRING:
            return toBool() ? "true" : "false";
        case ARRAY:
            return AVal(nullptr, 0);
        default:
            X_UNREACHABLE();
        }

    case DOUBLE:
        switch (t) {
        case INT:
            return int(data->doubleValue);
        case BOOL:
            return bool(data->doubleValue);
        case FUNCTION:
        case FUNCTION_BUILTIN:
            return static_cast<Ast::Function*>(nullptr);
        case STRING: {
            std::stringstream ss;
            ss << data->doubleValue;
            return ss.str().c_str();
        }
        case ARRAY:
            return AVal(nullptr, 0);
        default:
            X_UNREACHABLE();
        }

    case FUNCTION:
        switch (t) {
        case INT:
        case BOOL:
        case DOUBLE:
        case FUNCTION_BUILTIN:
            return AVal(data->functionValue ? true : false).convertTo(t);
        case STRING:
            return "[function]";
        case ARRAY:
            return AVal(nullptr, 0);
        default:
            X_UNREACHABLE();
        }

    case FUNCTION_BUILTIN:
        switch (t) {
        case INT:
        case BOOL:
        case DOUBLE:
        case FUNCTION:
            return AVal(toBuiltinFunction() ? true : false).convertTo(t);
        case STRING:
            return "[builtin function]";
        case ARRAY:
            return AVal(nullptr, 0);
        default:
            X_UNREACHABLE();
        }

    case STRING:
        switch (t) {
        case INT:
            return atoi(data->stringValue);
        case BOOL:
            return strlen(data->stringValue) > 0;
        case DOUBLE:
            return atof(data->stringValue);
        case FUNCTION:
        case FUNCTION_BUILTIN:
            return static_cast<Ast::Function*>(nullptr);
        case ARRAY:
            return AVal(nullptr, 0);
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

AVal::Data::~Data()
{
    if (type == FUNCTION) {
        delete functionValue;
    } else if (type == STRING) {
        MemoryPool::strfree(stringValue);
    } else if (type == ARRAY) {
        delete []arr;
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
