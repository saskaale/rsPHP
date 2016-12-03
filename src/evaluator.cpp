#include "evaluator.h"
#include "common.h"
#include "parser.h"
#include "environment.h"
#include "memorypool.h"

#include <iostream>
#include <algorithm>

std::vector<Environment*> envirs;

// Operators
static AVal binaryOp_impl(Ast::BinaryOperator::Op op, const char *a, const char *b)
{
    std::string sa = a;
    std::string sb = b;

    switch (op) {
    case Ast::BinaryOperator::Plus:
        return (sa + sb).c_str();
    case Ast::BinaryOperator::Minus:
    case Ast::BinaryOperator::Times:
    case Ast::BinaryOperator::Div:
    case Ast::BinaryOperator::Mod:
        // Invalid operator for string
        return AVal();
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

    return AVal();
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

    return AVal();
}

static inline AVal binaryOp(Ast::BinaryOperator::Op op, const AVal &a, const AVal &b)
{
    if (a.type() == AVal::UNDEFINED || b.type() == AVal::UNDEFINED) {
        return AVal();
    } else if (a.type() == AVal::STRING || b.type() == AVal::STRING) {
        return binaryOp_impl(op, a.toString(), b.toString());
    } else if (a.type() == AVal::DOUBLE || b.type() == AVal::DOUBLE) {
        return binaryOp_impl(op, a.toDouble(), b.toDouble());
    } else if (a.type() == AVal::INT || b.type() == AVal::INT) {
        return binaryOp_impl(op, a.toInt(), b.toInt());
    } else if (a.type() == AVal::BOOL || b.type() == AVal::BOOL) {
        return binaryOp_impl(op, a.toBool(), b.toBool());
    } else if (
        a.type() == AVal::FUNCTION || b.type() == AVal::FUNCTION
        ||
        a.type() == AVal::FUNCTION_BUILTIN || b.type() == AVal::FUNCTION_BUILTIN
    ) {
        // Cannot evaluate binary operator for functions
    } else {
        X_UNREACHABLE();
    }

    return AVal();
}



namespace Evaluator
{

StackFrame::StackFrame(Stack* s) :
    cnt(0)
{
}

StackFrame::~StackFrame()
{
    for(int i = 0; i < cnt; i++){
        stack->pop_back();
    }
}

void StackFrame::push(const AVal& v)
{
    stack->push_back(v);
};

static AVal doUserdefFunction(Ast::FunctionCall *v, const AVal::Function &func, Environment *envir)
{
    // Create environment for this function
    Environment *funcEnvironment = new Environment(envir);
    envirs.push_back(funcEnvironment);

    Environment *oldEnvir = nullptr;
    if (func.environment && func.environment != envir) {
        funcEnvironment->parent = func.environment;
        oldEnvir = func.environment->parent;
        func.environment->parent = envir;
    }

    Ast::VariableList *parameters = func.function->parameters;
    Ast::ExpressionList *exprs    = v->arguments;
    if (parameters->variables.size() != exprs->expressions.size()) {
        // PARAMETERS MISMATCH FOR FUNCTION
        return AVal();
    }


    int argslen = exprs->expressions.size();
    for(int i = 0; i < argslen; i++){
        funcEnvironment->set(parameters->variables[i], ex(exprs->expressions[i], envir));
    }

    // Execute statement list of function
    ex(func.function->statements, funcEnvironment);

    AVal ret = funcEnvironment->returnValue;

    if (oldEnvir) {
        func.environment->parent = oldEnvir;
    }

    envirs.erase(std::remove(envirs.begin(), envirs.end(), funcEnvironment), envirs.end());
    delete funcEnvironment;

    return ret;
}

AVal ex(Ast::Node *p, Environment* envir)
{
    if (!p) {
        return AVal();
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
        return envir->get(v);
    }

    case Ast::Node::ArraySubscriptT: {
        Ast::ArraySubscript *v = p->as<Ast::ArraySubscript*>();
        const int index = ex(v->expression, envir).toInt();
        AVal arr = envir->get(v);
        return arr.data->arr[index];
    }

    case Ast::Node::AssignmentT: {
        Ast::Assignment *v = p->as<Ast::Assignment*>();
        AVal r = ex(v->expression, envir);
        if (r.type() == AVal::FUNCTION && r.toFunction().function->isLambda()) {
            r.data->functionValue.environment = envir;
        }
        if (Ast::ArraySubscript *as = v->variable->as<Ast::ArraySubscript*>()) {
            const int index = ex(as->expression, envir).toInt();
            if (!envir->has(as)) {
                envir->set(as, AVal(new AVal[100], 100));
            }
            AVal arr = envir->get(as);
            arr.data->arr[index] = r;
        } else {
            envir->set(v->variable, r);
        }
        return r;
    }

    case Ast::Node::FunctionT: {
         Ast::Function *v = p->as<Ast::Function*>();
         AVal fun = AVal(v);
         if (!v->isLambda()) {
             envir->setFunction(v->name, fun);
         }
         return fun;
    }

    case Ast::Node::FunctionCallT: {
         Ast::FunctionCall *v = p->as<Ast::FunctionCall*>();

         AVal func = ex(v->function, envir);
         if (func.type() == AVal::FUNCTION) {
              return doUserdefFunction(v, func.toFunction(), envir);
         }else if (func.type() == AVal::FUNCTION_BUILTIN) {
              BuiltinCall call = func.toBuiltinFunction();
              if(call)
                  return (*call)(v->arguments, envir);
         }

         THROW("Call of argument which is not function")

         return AVal();
    }

    case Ast::Node::UnaryOperatorT: {
        Ast::UnaryOperator *v = p->as<Ast::UnaryOperator*>();
        switch (v->op) {
        case Ast::UnaryOperator::Minus:
            return binaryOp(Ast::BinaryOperator::Minus, 0, ex(v->expr, envir));

        case Ast::UnaryOperator::PreIncrement: {
            AVal val = binaryOp(Ast::BinaryOperator::Plus, ex(v->expr, envir), 1);
            if (Ast::Variable *var = v->expr->as<Ast::Variable*>()) {
                envir->set(var, val);
            }
            return val;
        }
        case Ast::UnaryOperator::PreDecrement: {
            AVal val = binaryOp(Ast::BinaryOperator::Minus, ex(v->expr, envir), 1);
            if (Ast::Variable *var = v->expr->as<Ast::Variable*>()) {
                envir->set(var, val);
            }
            return val;
        }
        case Ast::UnaryOperator::PostIncrement: {
            AVal val = ex(v->expr, envir);
            if (Ast::Variable *var = v->expr->as<Ast::Variable*>()) {
                envir->set(var, binaryOp(Ast::BinaryOperator::Plus, val, 1));
            }
            return val;
        }
        case Ast::UnaryOperator::PostDecrement: {
            AVal val = ex(v->expr, envir);
            if (Ast::Variable *var = v->expr->as<Ast::Variable*>()) {
                envir->set(var, binaryOp(Ast::BinaryOperator::Minus, val, 1));
            }
            return val;
        }
        default:
            X_UNREACHABLE();
        }
        break;
    }

    case Ast::Node::BinaryOperatorT: {
        Ast::BinaryOperator *v = p->as<Ast::BinaryOperator*>();
        return binaryOp(v->op, ex(v->left, envir), ex(v->right, envir));
    }

    case Ast::Node::ReturnT: {
        Ast::Return *v = p->as<Ast::Return*>();
        if (envir->parent) { // Only process return in functions
            envir->returnValue = ex(v->expression, envir);
            envir->state = Environment::ReturnCalled;
        }
        return envir->returnValue;
    }

    case Ast::Node::BreakT: {
        envir->state = Environment::BreakCalled;
        break;
    }

    case Ast::Node::ContinueT: {
        envir->state = Environment::ContinueCalled;
        break;
    }

    case Ast::Node::StatementListT: {
        Ast::StatementList *v = p->as<Ast::StatementList*>();
        for (Ast::Statement *s : v->statements) {
            ex(s, envir);
            if (envir->state & Environment::FlowInterrupted) {
                break;
            }
        }
        break;
    }

    case Ast::Node::IfT: {
        Ast::If *v = p->as<Ast::If*>();
        AVal cond = ex(v->condition, envir);
        if (cond.toBool()) {
            ex(v->thenStatement, envir);
        } else {
            ex(v->elseStatement, envir);
        }
        break;
    }

    case Ast::Node::WhileT: {
        Ast::While *v = p->as<Ast::While*>();
        while (ex(v->condition, envir).toBool()) {
            ex(v->statement, envir);
            if (envir->state == Environment::BreakCalled) {
                envir->state = Environment::Normal;
                break;
            } else if (envir->state == Environment::ContinueCalled) {
                envir->state = Environment::Normal;
                continue;
            } else if (envir->state == Environment::ReturnCalled) {
                break;
            }
        }
        break;
    }

    case Ast::Node::ForT: {
        Ast::For *v = p->as<Ast::For*>();
        for (ex(v->init, envir); ex(v->cond, envir).toBool(); ex(v->after, envir)) {
            ex(v->statement, envir);
            if (envir->state == Environment::BreakCalled) {
                envir->state = Environment::Normal;
                break;
            } else if (envir->state == Environment::ContinueCalled) {
                envir->state = Environment::Normal;
                continue;
            } else if (envir->state == Environment::ReturnCalled) {
                break;
            }
        }
        break;
    }

    default:
        X_UNREACHABLE();
    }

    return AVal();
}

void init()
{
    Environment *global = new Environment;
    registerBuiltins(global);
    envirs.push_back(global);
}

void exit()
{
    for (Environment *e : envirs) {
        delete e;
    }

    MemoryPool::cleanup();
    ::exit(0);
}

void eval(Ast::Node *p)
{
    ex(p, envirs[0]);
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

    // Same for lambdas
    if (p->type() == Ast::Node::AssignmentT) {
        Ast::Assignment *a = p->as<Ast::Assignment*>();
        if (a->expression->type() == Ast::Node::FunctionT) {
            a->expression = nullptr;
        }
    }

    delete p;
}

std::vector<Environment*> environments()
{
    return envirs;
}

} // namespace Evaluator
