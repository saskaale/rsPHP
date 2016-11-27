#include "common.h"
#include "ast.h"
#include "parser.hpp"
#include "environment.h"
#include "evaluator.h"

#include <iostream>

#define THROW(name, descr) \
    (fprintf(stderr, name, descr), fprintf(stderr, "\n"), X_ASSERT(false && name)); \
    return AVal(0);\


// Operators
static AVal binaryOp_impl(Ast::BinaryOperator::Op op, const char *a, const char *b)
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

static AVal binaryOp(Ast::BinaryOperator::Op op, const AVal &a, const AVal &b)
{
    if (a.type == AVal::STRING || b.type == AVal::STRING) {
        return binaryOp_impl(op, a.toString(), b.toString());
    } else if (a.type == AVal::DOUBLE || b.type == AVal::DOUBLE) {
        return binaryOp_impl(op, a.toDouble(), b.toDouble());
    } else if (a.type == AVal::INT || b.type == AVal::INT) {
        return binaryOp_impl(op, a.toInt(), b.toInt());
    } else if (a.type == AVal::BOOL || b.type == AVal::BOOL) {
        return binaryOp_impl(op, a.toBool(), b.toBool());
    } else if (a.type == AVal::FUNCTION || b.type == AVal::FUNCTION) {
        THROW("Cannot evaluate binary operator %d for functions", op);
    } else {
        X_UNREACHABLE();
    }

    return 0;
}

AVal ex(Ast::Node *p, Environment* envir);

// Functions
static AVal doBuiltInPrint(Ast::FunctionCall *v, Environment* envir)
{
    AVal printV = ex(v->arguments->expressions.front(), envir);

    int ret = -1;

    switch (printV.type) {
    case AVal::BOOL:
        ret = printf("%s\n", printV.toBool() ? "true" : "false");
        break;
    case AVal::DOUBLE:
        ret = printf("%lf\n", printV.toDouble());
        break;
    case AVal::STRING:
    case AVal::FUNCTION:
        ret = printf("\"%s\"\n", printV.toString());
        break;
    default:
        ret = printf("%d\n", printV.toInt());
        break;
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
            if (func.type == AVal::FUNCTION) {
                return doUserdefFunction(v, func.func, envir);
            }
         }

         return AVal(0);
    }

    case Ast::Node::UnaryOperatorT: {
        Ast::UnaryOperator *v = p->as<Ast::UnaryOperator*>();
        switch (v->op) {
        case Ast::UnaryOperator::Minus:
            return binaryOp(Ast::BinaryOperator::Minus, 0, ex(v->expr, envir));

        case Ast::UnaryOperator::PreIncrement: {
            AVal val = binaryOp(Ast::BinaryOperator::Plus, ex(v->expr, envir), 1);
            if (Ast::Variable *var = v->expr->as<Ast::Variable*>()) {
                envir->set(var->name, val);
            }
            return val;
        }
        case Ast::UnaryOperator::PreDecrement: {
            AVal val = binaryOp(Ast::BinaryOperator::Minus, ex(v->expr, envir), 1);
            if (Ast::Variable *var = v->expr->as<Ast::Variable*>()) {
                envir->set(var->name, val);
            }
            return val;
        }
        case Ast::UnaryOperator::PostIncrement: {
            AVal val = ex(v->expr, envir);
            if (Ast::Variable *var = v->expr->as<Ast::Variable*>()) {
                envir->set(var->name, binaryOp(Ast::BinaryOperator::Plus, val, 1));
            }
            return val;
        }
        case Ast::UnaryOperator::PostDecrement: {
            AVal val = ex(v->expr, envir);
            if (Ast::Variable *var = v->expr->as<Ast::Variable*>()) {
                envir->set(var->name, binaryOp(Ast::BinaryOperator::Minus, val, 1));
            }
            return val;
        }
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
        if (cond.toBool()) {
            ex(v->thenStatement, envir);
        } else {
            ex(v->elseStatement, envir);
        }
        return cond;
    }

    case Ast::Node::WhileT: {
        Ast::While *v = p->as<Ast::While*>();
        while (ex(v->condition, envir).toBool()) {
            ex(v->statement, envir);
        }
        return AVal(false);
    }

    case Ast::Node::ForT: {
        Ast::For *v = p->as<Ast::For*>();
        for (ex(v->init, envir); ex(v->cond, envir).toBool(); ex(v->after, envir)) {
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
