#include "aval.h"
#include "common.h"

AVal::AVal()
{
}

AVal::AVal(int value)
    : type(INT)
    , intValue(value)
{
}

AVal::AVal(bool value)
    : type(BOOL)
    , boolValue(value)
{
}

AVal::AVal(double value)
    : type(DOUBLE)
    , doubleValue(value)
{
}

AVal::AVal(const char *value)
    : type(STRING)
    , stringValue(strdup(value))
{
}

AVal::AVal(AVal *arr, size_t size)
    : type(ARRAY)
    , arr(arr)
    , arrsize(size)
{
}

AVal::AVal(Ast::Function *value)
    : type(FUNCTION)
    , functionValue(value)
{
}

int AVal::toInt() const
{
    return convertTo(INT).intValue;
}

bool AVal::toBool() const
{
    return convertTo(BOOL).boolValue;
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
    return convertTo(STRING).stringValue;
}

AVal AVal::convertTo(Type t) const
{
    if (type == t) {
        return *this;
    }

    switch (type) {
    case INT:
        switch (t) {
        case BOOL:
            return bool(intValue);
        case DOUBLE:
            return double(intValue);
        case FUNCTION:
            return static_cast<Ast::Function*>(nullptr);
        case STRING: {
            std::stringstream ss;
            ss << intValue;
            return ss.str().c_str();
        }
        case ARRAY:
            return AVal(nullptr, 0);
        default:
            X_UNREACHABLE();
        }

    case BOOL:
        return AVal(int(boolValue)).convertTo(t);

    case DOUBLE:
        switch (t) {
        case INT:
            return int(doubleValue);
        case BOOL:
            return bool(doubleValue);
        case FUNCTION:
            return static_cast<Ast::Function*>(nullptr);
        case STRING: {
            std::stringstream ss;
            ss << doubleValue;
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
            return AVal(functionValue ? true : false).convertTo(t);
        case STRING:
            return "[function]";
        case ARRAY:
            return AVal(nullptr, 0);
        default:
            X_UNREACHABLE();
        }

    case STRING:
        switch (t) {
        case INT:
            return atoi(stringValue);
        case BOOL:
            return strlen(stringValue) > 0;
        case DOUBLE:
            return atof(stringValue);
        case FUNCTION:
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

void AVal::cleanup()
{
    if (type == FUNCTION) {
        delete functionValue;
    } else if (type == STRING) {
        free(stringValue);
    } else if (type == ARRAY) {
        delete [] arr;
    }
}
