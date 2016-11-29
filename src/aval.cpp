#include "aval.h"
#include "common.h"
#include "memorypool.h"

#include <cmath>
#include <cstring>
#include <cstdlib>
#include <sstream>

AVal::AVal()
{
}

AVal::AVal(int value)
    : data(MemoryPool::alloc())
{
    data->type = INT;
    data->intValue = value;
}

AVal::AVal(bool value)
    : data(MemoryPool::alloc())
{
    data->type = BOOL;
    data->boolValue = value;
}

AVal::AVal(double value)
    : data(MemoryPool::alloc())
{
    data->type = DOUBLE;
    data->doubleValue = value;
}

AVal::AVal(const char *value)
    : data(MemoryPool::alloc())
{
    data->type = STRING;
    data->stringValue = MemoryPool::strdup(value);
}

AVal::AVal(AVal *arr, size_t size)
    : data(MemoryPool::alloc())
{
    data->type = ARRAY;
    data->arr = arr;
    data->arrsize = size;
}

AVal::AVal(Ast::Function *value)
    : data(MemoryPool::alloc())
{
    data->type = FUNCTION;
    data->functionValue = value;
}

AVal::AVal(BuiltinCall funccall)
    : data(MemoryPool::alloc())
{
    data->type = FUNCTION_BUILTIN;
    data->builtinFunction = funccall;
}


AVal::Type AVal::type() const
{
    return data ? data->type : UNDEFINED;
}

int AVal::toInt() const
{
    return convertTo(INT).data->intValue;
}

bool AVal::toBool() const
{
    return convertTo(BOOL).data->boolValue;
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
    return convertTo(FUNCTION_BUILTIN).data->builtinFunction;
}

AVal AVal::convertTo(Type t) const
{
    if (type() == t) {
        return *this;
    }

    if (t == UNDEFINED) {
        return AVal();
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

    case INT:
        switch (t) {
        case BOOL:
            return bool(data->intValue);
        case DOUBLE:
            return double(data->intValue);
        case FUNCTION:
        case FUNCTION_BUILTIN:
            return static_cast<Ast::Function*>(nullptr);
        case STRING: {
            std::stringstream ss;
            ss << data->intValue;
            return ss.str().c_str();
        }
        case ARRAY:
            return AVal(nullptr, 0);
        default:
            X_UNREACHABLE();
        }

    case BOOL:
        return AVal(int(data->boolValue)).convertTo(t);

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
            return AVal(data->builtinFunction ? true : false).convertTo(t);
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
