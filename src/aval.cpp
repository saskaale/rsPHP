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
{
}

AVal::AVal(int value)
    : _type(INT)
{
    data = reinterpret_cast<Data*>(value);
}

AVal::AVal(bool value)
    : _type(BOOL)
{
    data = reinterpret_cast<Data*>(value);
}

AVal::AVal(double value)
    : _type(DOUBLE)
    , data(MemoryPool::alloc())
{
    data->type = _type;
    data->doubleValue = value;
}

AVal::AVal(const char *value)
    : _type(STRING)
    , data(MemoryPool::alloc())
{
    data->type = _type;
    data->stringValue = MemoryPool::strdup(value);
}

AVal::AVal(AVal *arr, size_t size)
    : _type(ARRAY)
    , data(MemoryPool::alloc())
{
    data->type = _type;
    data->arr = arr;
    data->arrsize = size;
}

AVal::AVal(Ast::Function *value)
    : _type(FUNCTION)
    , data(MemoryPool::alloc())
{
    data->type = _type;
    data->functionValue = value;
}

AVal::AVal(BuiltinCall value)
    : _type(FUNCTION_BUILTIN)
{
    data = reinterpret_cast<Data*>(value);
}

AVal::Type AVal::type() const
{
    return _type;
}

const char* AVal::typeStr() const
{
    static const char* const tNames[] = {
        "undefined",
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
