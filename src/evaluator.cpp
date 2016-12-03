#include "evaluator.h"
#include "common.h"
#include "parser.h"
#include "environment.h"
#include "memorypool.h"
#include "bootstrap.h"

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
    CHECKTHROWN(a)
    CHECKTHROWN(b)

    if (a.isReference() || b.isReference()) {
        AVal ar = a;
        AVal br = b;
        if (a.isReference()) {
            ar = *a.toReference();
        }
        if (b.isReference()) {
            br = *b.toReference();
        }
        return binaryOp(op, ar, br);
    }

    if (a.isUndefined() || b.isUndefined()) {
        return AVal();
    } else if (a.isString() || b.isString()) {
        return binaryOp_impl(op, a.toString(), b.toString());
    } else if (a.isDouble() || b.isDouble()) {
        return binaryOp_impl(op, a.toDouble(), b.toDouble());
    } else if (a.isInt() || b.isInt()) {
        return binaryOp_impl(op, a.toInt(), b.toInt());
    } else if (a.isBool() || b.isBool()) {
        return binaryOp_impl(op, a.toBool(), b.toBool());
    } else if (a.isFunction() || b.isFunction() || a.isBuiltinFunction() || b.isBuiltinFunction()
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
};

StackFrame::~StackFrame()
{
    for(int i = 0; i < cnt; i++){
        stack->pop_back();
    }
};

void StackFrame::push(const AVal& v)
{
    stack->push_back(v);
};

static AVal doUserdefFunction(Ast::FunctionCall *v, Ast::Function *func, Environment *envir)
{
    // Create environment for this function
    Environment *funcEnvironment = new Environment(envir);
    envirs.push_back(funcEnvironment);

    Ast::VariableList *parameters = func->parameters;
    Ast::ExpressionList *exprs    = v->arguments;
    if (parameters->variables.size() != exprs->expressions.size()) {
        THROW("PARAMETERS MISMATCH FOR FUNCTION");
    }


    for (int i = 0; i < exprs->expressions.size(); i++) {
        Ast::Variable *v = parameters->variables[i];
        Ast::Expression *e = exprs->expressions[i];
        AVal r;
        if (v->ref) {
            if (e->type() != Ast::Node::VariableT) {
                THROW2("Argument %d expects reference!", i);
            }
            Ast::Variable *ev = e->as<Ast::Variable*>();
            r = &envir->get(ev);
        } else {
            r = ex(e, envir);
            CHECKTHROWN(r)
        }
        funcEnvironment->set(v, r);
    }

    // Execute statement list of function
    CHECKTHROWN(ex(func->statements, funcEnvironment));

    AVal ret = funcEnvironment->returnValue;

    envirs.erase(std::remove(envirs.begin(), envirs.end(), funcEnvironment), envirs.end());
    delete funcEnvironment;

    return ret;
}

AVal ex(Ast::Node *p, Environment* envir)
{
    if (!p) {
        return AVal();
    }

    auto assignToVariable = [](Ast::Variable *v, const AVal &value, Environment *envir) {
        if (Ast::ArraySubscript *as = v->as<Ast::ArraySubscript*>()) {
            AVal ind = ex(as->expression, envir);
            CHECKTHROWN(ind)
            const int index = ind.toInt();
            if (!envir->has(as)) {
                envir->set(as, AVal(new AVal[100], 100));
            }
            AVal arr = envir->get(as);
            arr.data->arr[index] = value;
        } else {
            AVal &stored = envir->get(v);
            if (stored.isReference()) {
                *stored.toReference() = value.dereference();
            } else if (stored.isWritable()) {
                stored = value.dereference();
            } else {
                envir->set(v, value.dereference());
            }
        }
    };

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
        AVal ind = ex(v->expression, envir);
        CHECKTHROWN(ind)
        const int index = ind.toInt();
        AVal arr = envir->get(v);
        return arr.data->arr[index];
    }

    case Ast::Node::AssignmentT: {
        Ast::Assignment *v = p->as<Ast::Assignment*>();
        AVal r = ex(v->expression, envir);
        CHECKTHROWN(r)
        assignToVariable(v->variable, r, envir);
        return r;
    }

    case Ast::Node::TryT: {
        Ast::Try *v = p->as<Ast::Try*>();

        if(v->variables->variables.empty()){
          THROW("Try expects one name of variable to catch")
        }

        AVal r = ex(v->body, envir);

        if(r.isThrown()){
          AVal catched = r;
          catched.markThrown(false);
          assignToVariable(v->variables->variables[0], catched, envir);

          AVal catchP = ex(v->catchPart, envir);
          CHECKTHROWN(catchP)
        }
//        assignToVariable(v->variable, r, envir);
        return r;
    }

    case Ast::Node::FunctionT: {
         Ast::Function *v = p->as<Ast::Function*>();
         AVal fun = AVal(v);
         envir->setFunction(v->name, fun);
         return fun;
    }

    case Ast::Node::FunctionCallT: {
         Ast::FunctionCall *v = p->as<Ast::FunctionCall*>();

         AVal func = ex(v->function, envir);
         CHECKTHROWN(func)
         if (func.isFunction()) {
              return doUserdefFunction(v, func.toFunction(), envir);
         }else if (func.isBuiltinFunction()) {
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
                assignToVariable(var, val, envir);
            }
            return val;
        }
        case Ast::UnaryOperator::PreDecrement: {
            AVal val = binaryOp(Ast::BinaryOperator::Minus, ex(v->expr, envir), 1);
            if (Ast::Variable *var = v->expr->as<Ast::Variable*>()) {
                assignToVariable(var, val, envir);
            }
            return val;
        }
        case Ast::UnaryOperator::PostIncrement: {
            AVal val = ex(v->expr, envir);
            CHECKTHROWN(val)
            if (Ast::Variable *var = v->expr->as<Ast::Variable*>()) {
                assignToVariable(var, binaryOp(Ast::BinaryOperator::Plus, val, 1), envir);
            }
            return val;
        }
        case Ast::UnaryOperator::PostDecrement: {
            AVal val = ex(v->expr, envir);
            CHECKTHROWN(val)
            if (Ast::Variable *var = v->expr->as<Ast::Variable*>()) {
                assignToVariable(var, binaryOp(Ast::BinaryOperator::Minus, val, 1), envir);
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
            AVal r = ex(s, envir);
            CHECKTHROWN(r)
            MemoryPool::checkCollectGarbage();
            if (envir->state & Environment::FlowInterrupted) {
                break;
            }
        }
        break;
    }

    case Ast::Node::IfT: {
        Ast::If *v = p->as<Ast::If*>();
        AVal cond = ex(v->condition, envir);
        CHECKTHROWN(cond)
        if (cond.toBool()) {
            CHECKTHROWN(ex(v->thenStatement, envir));
        } else {
            CHECKTHROWN(ex(v->elseStatement, envir));
        }
        break;
    }

    case Ast::Node::WhileT: {
        Ast::While *v = p->as<Ast::While*>();
        do  {
            AVal cond = ex(v->condition, envir);
            CHECKTHROWN(cond)
            if(!cond.toBool())
              break;

            CHECKTHROWN(ex(v->statement, envir))
            MemoryPool::checkCollectGarbage();
            if (envir->state == Environment::BreakCalled) {
                envir->state = Environment::Normal;
                break;
            } else if (envir->state == Environment::ContinueCalled) {
                envir->state = Environment::Normal;
                continue;
            } else if (envir->state == Environment::ReturnCalled) {
                break;
            }
        } while (true);
        break;
    }

    case Ast::Node::ForT: {
        Ast::For *v = p->as<Ast::For*>();

        CHECKTHROWN(ex(v->init, envir));
        while(1){
            AVal cond = ex(v->cond, envir);
            CHECKTHROWN(cond)
            if(!cond.toBool())
                break;

            CHECKTHROWN(ex(v->statement, envir))

            MemoryPool::checkCollectGarbage();
            if (envir->state == Environment::BreakCalled) {
                envir->state = Environment::Normal;
                break;
            } else if (envir->state == Environment::ContinueCalled) {
                envir->state = Environment::Normal;
                continue;
            } else if (envir->state == Environment::ReturnCalled) {
                break;
            }

            CHECKTHROWN(ex(v->after, envir))
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
    Parser::parseString(RSPHP_BOOTSTRAP);
}

void exit()
{
    for (Environment *e : envirs) {
        delete e;
    }
    envirs.clear();

    MemoryPool::cleanup();
}

void eval(Ast::Node *p)
{
    ex(p, envirs[0]);
    MemoryPool::checkCollectGarbage();
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
        if (a->expression && a->expression->type() == Ast::Node::FunctionT) {
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
