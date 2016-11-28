#include "aval.h"
#include "common.h"

int AVal::toInt() const
{
    return convertTo(INT).value;
}

bool AVal::toBool() const
{
    return convertTo(BOOL).value;
}

double AVal::toDouble() const
{
    return convertTo(DOUBLE).fValue;
}

Ast::Function *AVal::toFunction() const
{
    return convertTo(FUNCTION).func;
}

const char *AVal::toString() const
{
    return convertTo(STRING).str;
}

AVal AVal::convertTo(Type t) const
{
    switch (type) {
    case INT:
        switch (t) {
        case INT:
            return value;
        case BOOL:
            return bool(value);
        case DOUBLE:
            return double(value);
        case FUNCTION:
            return static_cast<Ast::Function*>(nullptr);
        case STRING: {
            std::stringstream ss;
            ss << value;
            return ss.str().c_str();
        }
        case ARRAY:
            return AVal(nullptr, 0);
        default:
            X_UNREACHABLE();
        }

    case BOOL:
        return AVal(value).convertTo(t);

    case DOUBLE:
        switch (t) {
        case INT:
            return int(fValue);
        case BOOL:
            return bool(fValue);
        case DOUBLE:
            return double(fValue);
        case FUNCTION:
            return static_cast<Ast::Function*>(nullptr);
        case STRING: {
            std::stringstream ss;
            ss << fValue;
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
            return AVal(func ? true : false).convertTo(t);
        case FUNCTION:
            return func;
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
            return atoi(str);
        case BOOL:
            return strlen(str) > 0;
        case DOUBLE:
            return atof(str);
        case FUNCTION:
            return static_cast<Ast::Function*>(nullptr);
        case STRING:
            return str;
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
        case ARRAY:
            return AVal(arr, arrsize);
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
        delete func;
    } else if (type == STRING) {
        free(str);
    } else if (type == ARRAY) {
        delete [] arr;
    }
}
