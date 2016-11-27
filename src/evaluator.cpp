#include "common.h"
#include "ast.h"
#include "parser.hpp"
#include "environment.h"
#include "evaluator.h"

#include <iostream>


#define TO_INT(arg) IS_DOUBLE(arg) ? (int)(arg).fValue : (arg).value
#define TO_DOUBLE(arg) (IS_INT(arg) || IS_BOOL(arg)) ? (double)(arg).value : (arg).fValue
#define TO_BOOL(arg) (!!TO_INT(arg))
#define TO_STRING(arg) (arg).str
#define IS_FUNCTION(arg) ((arg).type == AVal::FUNCTION)
#define IS_INT(arg) ((arg).type == AVal::INT)
#define IS_BOOL(arg) ((arg).type == AVal::BOOL)
#define IS_DOUBLE(arg) ((arg).type == AVal::DOUBLE)
#define IS_STRING(arg) ((arg).type == AVal::STRING)

#define THROW(name, descr) \
    (fprintf(stderr, name, descr), fprintf(stderr, "\n"), X_ASSERT(false && name)); \
    return AVal(0);\


AVal ex(Ast::Node *p, Environment* envir);

// Operators
static AVal binaryOp_impl(Ast::BinaryOperator::Op op, char *a, char *b)
{
    std::string sa = a;
    std::string sb = b;

    switch (op) {
    case Ast::BinaryOperator::Plus:
        return strdup((sa + sb).c_str());
    case Ast::BinaryOperator::Minus:
    case Ast::BinaryOperator::Times:
    case Ast::BinaryOperator::Div:
    case Ast::BinaryOperator::Mod:
        THROW("Invalid operator for string %d", op);
        THROW("Invalid operator for string %d", op);
    case Ast::BinaryOperator::Equal:
        return a == b;
    case Ast::BinaryOperator::NotEqual:
        return a != b;
    case Ast::BinaryOperator::LessThan:
        return a < b;
    case Ast::BinaryOperator::GreaterThan:
        return a > b;
    case Ast::BinaryOperator::LessThanEqual:
        return a <= b;
    case Ast::BinaryOperator::GreaterThanEqual:
        return a >= b;
    case Ast::BinaryOperator::And:
        return a && b;
    case Ast::BinaryOperator::Or:
        return a || b;
    default:
        X_UNREACHABLE();
    }

    return 0;
}

template<typename T>
static AVal binaryOp_impl(Ast::BinaryOperator::Op op, T a, T b)
{
    switch (op) {
    case Ast::BinaryOperator::Plus:
        return a + b;
    case Ast::BinaryOperator::Minus:
        return a - b;
    case Ast::BinaryOperator::Times:
        return a * b;
    case Ast::BinaryOperator::Equal:
        return a == b;
    case Ast::BinaryOperator::NotEqual:
        return a != b;
    case Ast::BinaryOperator::LessThan:
        return a < b;
    case Ast::BinaryOperator::GreaterThan:
        return a > b;
    case Ast::BinaryOperator::LessThanEqual:
        return a <= b;
    case Ast::BinaryOperator::GreaterThanEqual:
        return a >= b;
    case Ast::BinaryOperator::Div:
        return a / b;
    case Ast::BinaryOperator::Mod:
        return int(a) % int(b);
    case Ast::BinaryOperator::And:
        return a && b;
    case Ast::BinaryOperator::Or:
        return a || b;
    default:
        X_UNREACHABLE();
    }

    return 0;
}

static AVal binaryOp(Ast::BinaryOperator::Op op, const AVal &arg1, const AVal &arg2)
{
    const AVal &a = arg1;
    const AVal &b = arg2.convertTo(a.type);

    switch (a.type) {
    case AVal::INT:
    case AVal::BOOL:
        return binaryOp_impl(op, a.value, b.value);
    case AVal::DOUBLE:
        return binaryOp_impl(op, a.fValue, b.fValue);
    case AVal::STRING:
        return binaryOp_impl(op, a.str, b.str);
    case AVal::FUNCTION:
        THROW("Cannot evaluate binary operator %d for functions", op);
    default:
        X_UNREACHABLE();
    }

    return 0;
}

// Functions
static AVal doBuiltInPrint(Ast::FunctionCall *v, Environment* envir)
{
    AVal printV = ex(v->arguments->expressions.front(), envir);

    int ret = -1;
    if(IS_BOOL(printV)){
      ret = printf("%s\n", TO_BOOL(printV)?"true":"false");
    }else if(IS_DOUBLE(printV)){
      ret = printf("%lf\n", TO_DOUBLE(printV));
    }else if(IS_STRING(printV)){
      ret = printf("\"%s\"\n", TO_STRING(printV));
    }else{
      ret = printf("%d\n", TO_INT(printV));
    }

    return AVal(ret);
}

static AVal doUserdefFunction(Ast::FunctionCall *v, Ast::Function *func, Environment *envir)
{
    //create environment for this function
    Environment funcEnvironment(envir);

    Ast::VariableList *parameters = func->parameters;
    Ast::ExpressionList *exprs    = v->arguments;
    if(parameters->variables.size() != exprs->expressions.size()){
      THROW("PARAMETERS MISMATCH FOR FUNCTION %s", v->functionName.c_str())
    }


    int argslen = exprs->expressions.size();
    for(int i = 0; i < argslen; i++){
      funcEnvironment.set(parameters->variables[i]->name, ex(exprs->expressions[i], envir));
    }

    //execute statement list of function
    ex(func->statements,&funcEnvironment);

    return AVal(0);
}


Environment globalenvir;

AVal ex(Ast::Node *p, Environment* envir)
{
    if (!p) {
        return AVal(0);
    }

    switch (p->type()) {

    case Ast::Node::IntegerLiteralT:
        return AVal(p->as<Ast::IntegerLiteral*>()->value);

    case Ast::Node::BoolLiteralT:
        return AVal(p->as<Ast::BoolLiteral*>()->value);

    case Ast::Node::DoubleLiteralT:
        return AVal(p->as<Ast::DoubleLiteral*>()->value);

    case Ast::Node::StringLiteralT:
        return AVal(p->as<Ast::StringLiteral*>()->value.c_str());

    case Ast::Node::VariableT: {
        Ast::Variable *v = p->as<Ast::Variable*>();
        if(!envir->has(v->name)){
            THROW("SYMBOL %s LOOKUP ERROR", v->name.c_str())
        }
        return envir->get(v->name);
    }

    case Ast::Node::AssignmentT: {
        Ast::Assignment *v = p->as<Ast::Assignment*>();
        AVal r = ex(v->expression, envir);
        envir->set(v->variable->name, r);
        return r;
    }

    case Ast::Node::FunctionT: {
         Ast::Function *v = p->as<Ast::Function*>();
         AVal fun = AVal(v);
         envir->set(v->name, fun);
         return fun;
    }

    case Ast::Node::FunctionCallT: {
         Ast::FunctionCall *v = p->as<Ast::FunctionCall*>();

         //handle built-in functions
         if (v->functionName == "print") {
             return doBuiltInPrint(v, envir);
         }

         if(envir->has(v->functionName)){
            AVal func = envir->get(v->functionName);
            if(IS_FUNCTION(func)){
                return doUserdefFunction(v, func.func, envir);
            }
         }

         return AVal(0);
    }

    case Ast::Node::UnaryOperatorT: {
        Ast::UnaryOperator *v = p->as<Ast::UnaryOperator*>();
        switch (v->op) {
        case Ast::UnaryOperator::Minus:
            return AVal(-ex(v->expr, envir).value);
        case Ast::UnaryOperator::PreIncrement:
        case Ast::UnaryOperator::PreDecrement:
        case Ast::UnaryOperator::PostIncrement:
        case Ast::UnaryOperator::PostDecrement:
            return AVal(0);
        default:
            X_UNREACHABLE();
        };
        break;
    }

    case Ast::Node::BinaryOperatorT: {
        Ast::BinaryOperator *v = p->as<Ast::BinaryOperator*>();
        return binaryOp(v->op, ex(v->left, envir), ex(v->right, envir));
    }

    case Ast::Node::StatementListT: {
        Ast::StatementList *v = p->as<Ast::StatementList*>();

        AVal ret = AVal(false);
        for (Ast::Statement *s : v->statements) {
            ret = ex(s, envir);
        }
        return ret;
    }

    case Ast::Node::IfT: {
        Ast::If *v = p->as<Ast::If*>();

        AVal cond = ex(v->condition, envir);
        if (TO_BOOL(cond)) {
            ex(v->thenStatement, envir);
        } else {
            ex(v->elseStatement, envir);
        }
        return cond;
    }

    case Ast::Node::WhileT: {
        Ast::While *v = p->as<Ast::While*>();
        while (TO_BOOL(ex(v->condition, envir))) {
            ex(v->statement, envir);
        }
        return AVal(false);
    }

    case Ast::Node::ForT: {
        Ast::For *v = p->as<Ast::For*>();
        for (ex(v->init, envir); TO_BOOL(ex(v->cond, envir)); ex(v->after, envir)) {
            ex(v->statement, envir);
        }
        return AVal(false);
    }

    default: {
        X_UNREACHABLE();
        break;
    }
    }

    return AVal(false);
}

namespace Evaluator
{

void eval(Ast::Node *p)
{
    ex(p, &globalenvir);
}

void cleanup(Ast::Node *p)
{
    if (!p) {
        return;
    }

    // We keep the parsed function in Environment, so don't delete it
    if (p->type() == Ast::Node::FunctionT) {
        return;
    }

    delete p;
}

} // namespace Evaluator
