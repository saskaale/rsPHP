#include "aval.h"
#include "common.h"
#include "memorypool.h"

#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cstddef>
#include <sstream>

AArray emptyArray;


std::unordered_set<AVal*> localAVals;


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

AVal::AVal(const AVal& v)
{
    if(&v == this)
        return;

    memcpy(this,&v, sizeof(*this));
    if( isTracked() ){
        localAVals.insert(this);
    }
}

AVal& AVal::operator=(const AVal& v)
{
    if(&v == this)
        return *this;

    bool old_tracked = isTracked();

    memcpy(this,&v, sizeof(*this));

    bool new_tracked = isTracked();
    if( new_tracked != old_tracked ){
        if( new_tracked ){
            localAVals.insert(this);
        }else{
            localAVals.erase(this);
        }
    }
    return *this;
}

AVal::AVal(AVal *value)
    : _type(REFERENCE)
    , referenceValue(value)
{
    if( isTracked() ){
        localAVals.insert(this);
    }
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
    , charValue(value)
{
}

AVal::AVal(double value)
    : _type(DOUBLE)
    , doubleValue(value)
{
}

AVal::AVal(const char *value)
    : _type(STRING)
    , stringValue(nullptr)
{
    stringValue = rstrdup(value);
    localAVals.insert(this);
}

AVal::AVal(AArray *value)
    : _type(ARRAY)
{
    arrayValue = value;
    localAVals.insert(this);
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


AVal::~AVal()
{
    if( isTracked() ){
        localAVals.erase(this);
    }
}

AVal::Type AVal::type() const
{
    return _type;
}

const char *AVal::typeStr() const
{
    static const char *tNames[] = {
        "undefined",
        "reference",
        "int",
        "bool",
        "char",
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

void AVal::assign(const AVal &value)
{
    if (_charref) {
        if (value.isChar()) {
            *reinterpret_cast<char*>(referenceValue) = value.toChar();
        } else {
            fprintf(stderr, "Cannot assign '%s' to char\n", value.dereference().typeStr());
        }
    } else {
        *this = value;
    }
}

// static
AVal AVal::createCharReference(char *value)
{
    AVal v(reinterpret_cast<AVal*>(value));
    v._charref = true;
    return v;
}

bool AVal::isUndefined() const
{
    if (isReference()) {
        return dereference()._type == UNDEFINED;
    }
    return _type == UNDEFINED;
}

bool AVal::isReference() const
{
    return _type == REFERENCE;
}

bool AVal::isInt() const
{
    if (isReference()) {
        return dereference()._type == INT;
    }
    return _type == INT;
}

bool AVal::isBool() const
{
    if (isReference()) {
        return dereference()._type == BOOL;
    }
    return _type == BOOL;
}

bool AVal::isChar() const
{
    if (isReference()) {
        return dereference()._type == CHAR;
    }
    return _type == CHAR;
}

bool AVal::isDouble() const
{
    if (isReference()) {
        return dereference()._type == DOUBLE;
    }
    return _type == DOUBLE;
}

bool AVal::isString() const
{
    if (isReference()) {
        return dereference()._type == STRING;
    }
    return _type == STRING;
}

bool AVal::isArray() const
{
    if (isReference()) {
        return dereference()._type == ARRAY;
    }
    return _type == ARRAY;
}

bool AVal::isFunction() const
{
    if (isReference()) {
        return dereference()._type == FUNCTION;
    }
    return _type == FUNCTION;
}

bool AVal::isBuiltinFunction() const
{
    if (isReference()) {
        return dereference()._type == FUNCTION_BUILTIN;
    }
    return _type == FUNCTION_BUILTIN;
}

bool AVal::isConst() const
{
    return _const;
}

bool AVal::isThrown() const
{
    return _thrown;
}

bool AVal::isTracked() const
{
    Type t;
    if(isReference()) {
        t = referenceValue->type();
    }else{
        t = type();
    }
    return t == STRING || t == ARRAY;
}

void AVal::markConst(bool is)
{
    _const = is;
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
        case CHAR:
            return char(0);
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
        case CHAR:
            return char(intValue);
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
        case CHAR:
            return char(boolValue);
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
        case CHAR:
            return char(doubleValue);
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
        case CHAR:
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
        case CHAR:
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
        case CHAR:
            return char(0);
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
        case CHAR:
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
