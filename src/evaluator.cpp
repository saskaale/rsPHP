#include "evaluator.h"
#include "common.h"
#include "parser.h"
#include "environment.h"
#include "memorypool.h"
#include "bootstrap.h"

#include <memory>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <unordered_set>

std::vector<Environment*> envirs;
Environment *currentEnvironment = nullptr;

typedef std::unordered_set<std::string> Scope;
Scope globalFunctions;
std::vector<Scope> scopes;
std::unordered_map<Ast::Function*, Scope> functionScopes;

static bool symbolLookup(const std::string &s)
{
    if (scopes.back().find(s) != scopes.back().end()) {
        return true;
    }
    if (globalFunctions.find(s) != globalFunctions.end()) {
        return true;
    }
    return false;
}

static void createFunctionScope(Ast::Function *f)
{
     Scope scope = scopes.back();
     for (Ast::Variable *v : f->parameters()->variables) {
        scope.insert(v->name);
     }
     functionScopes[f] = scope;
}

static void createGlobalScope(Environment *e)
{
    Scope scope;
    for (auto i : e->keys) {
        if (i.second.isFunction() || i.second.isBuiltinFunction()) {
            globalFunctions.insert(i.first);
        } else {
            scope.insert(i.first);
        }
    }
    scopes.push_back(scope);
}

// Operators
static AVal binaryOp(Ast::BinaryOperator::Op op, const AVal &a, const AVal &b);

static AVal binaryOp_impl(Ast::BinaryOperator::Op op, AArray *a, AArray *b)
{
    switch (op) {
    case Ast::BinaryOperator::Plus:
        // return (sa + sb).c_str();
        return Evaluator::INVOKE_INTERNAL("merge", currentEnvironment, { a, b });
    case Ast::BinaryOperator::Minus:
    case Ast::BinaryOperator::Times:
    case Ast::BinaryOperator::Div:
    case Ast::BinaryOperator::Mod:
    case Ast::BinaryOperator::LessThan:
    case Ast::BinaryOperator::GreaterThan:
    case Ast::BinaryOperator::LessThanEqual:
    case Ast::BinaryOperator::GreaterThanEqual:
        // Invalid operator for array
        return AVal();
    case Ast::BinaryOperator::Equal:
        if (a->count != b->count) {
            return false;
        }
        for (size_t i = 0; i < a->count; ++i) {
            if (!binaryOp(Ast::BinaryOperator::Equal, a->array[i], b->array[i]).toBool()) {
                return false;
            }
        }
        return true;
    case Ast::BinaryOperator::NotEqual:
        return !binaryOp_impl(Ast::BinaryOperator::Equal, a, b).toBool();
    case Ast::BinaryOperator::And:
        return a->count && b->count;
    case Ast::BinaryOperator::Or:
        return a->count || b->count;
    default:
        X_UNREACHABLE();
    }

    return AVal();
}

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
        return sa == sb;
    case Ast::BinaryOperator::NotEqual:
        return sa != sb;
    case Ast::BinaryOperator::LessThan:
        return sa < sb;
    case Ast::BinaryOperator::GreaterThan:
        return sa > sb;
    case Ast::BinaryOperator::LessThanEqual:
        return sa <= sb;
    case Ast::BinaryOperator::GreaterThanEqual:
        return sa >= sb;
    case Ast::BinaryOperator::And:
        return !sa.empty() && !sb.empty();
    case Ast::BinaryOperator::Or:
        return !sa.empty() || !sb.empty();
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

static AVal binaryOp(Ast::BinaryOperator::Op op, const AVal &a, const AVal &b)
{
    CHECKTHROWN(a)
    CHECKTHROWN(b)

    if (op == Ast::BinaryOperator::EqualType) {
        if (a.type() != b.type()) {
            return false;
        }
        op = Ast::BinaryOperator::Equal;
    } else if (op == Ast::BinaryOperator::NotEqualType) {
        return !binaryOp(Ast::BinaryOperator::EqualType, a, b).toBool();
    }

    if (a.isUndefined() || b.isUndefined()) {
        if (op == Ast::BinaryOperator::Equal) {
            return a.isUndefined() && b.isUndefined();
        } else if (op == Ast::BinaryOperator::NotEqual) {
            return !(a.isUndefined() && b.isUndefined());
        }
        return AVal();
    } else if (a.isString() || b.isString()) {
        return binaryOp_impl(op, a.toString(), b.toString());
    } else if (a.isDouble() || b.isDouble()) {
        return binaryOp_impl(op, a.toDouble(), b.toDouble());
    } else if (a.isInt() || b.isInt()) {
        return binaryOp_impl(op, a.toInt(), b.toInt());
    } else if (a.isChar() || b.isChar()) {
        return binaryOp_impl(op, a.toChar(), b.toChar());
    } else if (a.isBool() || b.isBool()) {
        return binaryOp_impl(op, a.toBool(), b.toBool());
    } else if (a.isArray() | b.isArray()) {
        return binaryOp_impl(op, a.toArray(), b.toArray());
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






static AVal doUserdefFunction(Ast::Function *func, const std::vector<Ast::Expression*> &arguments, Environment *envir)
{
    // Create environment for this function
    bool pushedScope = false;
    using Environment_ptr = std::unique_ptr<Environment, std::function<void(Environment*)>>;
    Environment_ptr funcEnvironment(new Environment(envir), [&](Environment *e) {
        envirs.erase(std::remove(envirs.begin(), envirs.end(), e), envirs.end());
        delete e;
        if (pushedScope) {
            scopes.pop_back();
        }
    });
    envirs.push_back(funcEnvironment.get());

    for (int i = 0; i < func->parameters()->variables.size(); i++) {
        Ast::Variable *v = func->parameters()->variables[i];
        Ast::Expression *e = arguments.size() > i ? arguments[i] : nullptr;
        AVal r;
        if (e && v->ref) {
            setExFlag(ReturnLValue);
            r = ex(e, envir);
            clearExFlag(ReturnLValue);
            CHECKTHROWN(r);
            if (r.isReference() && r.toReference()->isReference()) {
                // It already was reference
                r = r.dereference();
                if (r.isConst() && !v->isconst) {
                    THROW2("Argument %d violates const-correctness!", i);
                }
                if (!r.isConst() && v->isconst) {
                    // Passing non-const reference to const reference
                    r = r.copy();
                    r.markConst(true);
                }
            } else {
                // Reference was created
                r.markConst(v->isconst);
            }
            if (!r.isConst() && !r.isReference()) {
                THROW2("Argument %d expects reference!", i);
            }
        } else if (e) {
            r = ex(e, envir);
            CHECKTHROWN(r);
            r = r.dereference().copy();
        }
        funcEnvironment->set(v->name, r);
    }

    scopes.push_back(functionScopes.at(func));
    pushedScope = true;

    // Execute statement list of function
    CHECKTHROWN(ex(func->statements(), funcEnvironment.get()));

    return funcEnvironment->returnValue;
}

int exFlags = NoFlag;

void setExFlag(ExFlag flag)
{
    exFlags = exFlags | flag;
}

bool testExFlag(ExFlag flag)
{
    return exFlags & flag;
}

void clearExFlag(ExFlag flag)
{
    exFlags &= ~flag;
}


AVal ex(Ast::Node *p, Environment* envir)
{
    if (!p) {
        return AVal();
    }

    currentEnvironment = envir;

    auto assignToExpression = [](Ast::Expression *v, const AVal &value, Environment *envir) {
        setExFlag(ReturnLValue);
        AVal dest = ex(v, envir);
        clearExFlag(ReturnLValue);
        if (!dest.isReference()) {
            THROW("Cannot write to rvalue!");
        }
        AVal *ref = dest.toReference();
        if (ref->isConst()) {
            THROW("Cannot write to const!");
        }
        if (ref->isReference()) {
            *ref->toReference() = value.dereference();
        } else {
            *ref = value.dereference();
        }
        return AVal();
    };

    switch (p->type()) {

    case Ast::Node::UndefinedLiteralT:
        return AVal();

    case Ast::Node::IntegerLiteralT:
        return AVal(p->as<Ast::IntegerLiteral*>()->value);

    case Ast::Node::BoolLiteralT:
        return AVal(p->as<Ast::BoolLiteral*>()->value);

    case Ast::Node::DoubleLiteralT:
        return AVal(p->as<Ast::DoubleLiteral*>()->value);

    case Ast::Node::CharLiteralT:
        return AVal(p->as<Ast::CharLiteral*>()->value);

    case Ast::Node::StringLiteralT:
        return AVal(p->as<Ast::StringLiteral*>()->value.c_str());

    case Ast::Node::VariableT: {
        Ast::Variable *v = p->as<Ast::Variable*>();
        if (!symbolLookup(v->name)) {
            envir->set(v->name, AVal());
            scopes.back().insert(v->name);
        }
        return testExFlag(ReturnLValue) ? &envir->get(v->name) : envir->get(v->name);
    }

    case Ast::Node::AValLiteralT:
        return *((AVal*)p->as<Ast::AValLiteral*>()->value());

    case Ast::Node::ArraySubscriptT: {
        Ast::ArraySubscript *v = p->as<Ast::ArraySubscript*>();
        AVal ind = ex(v->expression(), envir);
        CHECKTHROWN(ind)
        const int index = ind.toInt();
        AVal arr = ex(v->source(), envir);
        if (testExFlag(ReturnLValue)) {
            arr = arr.dereference();
        }
        if (arr.isArray()) {
            AArray *a = arr.toArray();
            if (index < 0 || index >= a->count) {
                THROW2("Index %d out of bounds", index);
            }
            return testExFlag(ReturnLValue) ? &a->array[index] : a->array[index];
        } else if (arr.isString()) {
            AString *s = arr.dereference().stringValue;
            size_t count = strlen(s->string);
            if (index < 0 || index >= count) {
                THROW2("Index %d out of bounds", index);
            }
            return s->string[index];
        } else {
            THROW2("Variable %s is not array", arr.dereference().typeStr());
        }
    }

    case Ast::Node::AssignmentT: {
        Ast::Assignment *v = p->as<Ast::Assignment*>();
        AVal r = ex(v->expression(), envir);
        CHECKTHROWN(r);
        CHECKTHROWN(assignToExpression(v->destination(), r, envir));
        return r;
    }

    case Ast::Node::TryT: {
        Ast::Try *v = p->as<Ast::Try*>();

        if(v->variables()->variables.empty()){
          THROW("Try expects one name of variable to catch")
        }

        AVal r = ex(v->body(), envir);

        if(r.isThrown()){
          AVal catched = r;
          catched.markThrown(false);
          CHECKTHROWN(assignToExpression(v->variables()->variables[0], catched, envir));

          AVal catchP = ex(v->catchPart(), envir);
          CHECKTHROWN(catchP)
        }

        r.markThrown(false);
        return r;
    }

    case Ast::Node::FunctionT: {
         Ast::Function *v = p->as<Ast::Function*>();
         if (!v->isLambda()) {
             if (envir->parent) {
                 THROW2("Cannot register function '%s' outside global scope.", v->name.c_str());
             }
             globalFunctions.insert(v->name);
         }
         createFunctionScope(v);
         if (!v->isLambda()) {
             envir->set(v->name, v);
         }
         return v;
    }

    case Ast::Node::FunctionCallT: {
         Ast::FunctionCall *v = p->as<Ast::FunctionCall*>();

         AVal func = ex(v->function(), envir);
         CHECKTHROWN(func)

         std::vector<Ast::Expression*> args = v->arguments()->expressions();
         if (v->object()) {
             args.insert(args.begin(), v->object());
         }

         if (func.isFunction()) {
              return doUserdefFunction(func.toFunction(), args, envir);
         } else if (func.isBuiltinFunction()) {
              BuiltinCall call = func.toBuiltinFunction();
              if (call) {
                  return (*call)(args, envir);
              }
         }

         THROW2("Call of argument '%s' which is not function", func.toString());
    }

    case Ast::Node::UnaryOperatorT: {
        Ast::UnaryOperator *v = p->as<Ast::UnaryOperator*>();
        switch (v->op) {
        case Ast::UnaryOperator::Not:
            return !ex(v->expr(), envir).toBool();

        case Ast::UnaryOperator::Minus:
            return binaryOp(Ast::BinaryOperator::Minus, 0, ex(v->expr(), envir));

        case Ast::UnaryOperator::PreIncrement: {
            AVal val = binaryOp(Ast::BinaryOperator::Plus, ex(v->expr(), envir), 1);
            CHECKTHROWN(assignToExpression(v->expr(), val, envir));
            return val;
        }
        case Ast::UnaryOperator::PreDecrement: {
            AVal val = binaryOp(Ast::BinaryOperator::Minus, ex(v->expr(), envir), 1);
            CHECKTHROWN(assignToExpression(v->expr(), val, envir));
            return val;
        }
        case Ast::UnaryOperator::PostIncrement: {
            AVal val = ex(v->expr(), envir);
            CHECKTHROWN(val)
            CHECKTHROWN(assignToExpression(v->expr(), binaryOp(Ast::BinaryOperator::Plus, val, 1), envir));
            return val;
        }
        case Ast::UnaryOperator::PostDecrement: {
            AVal val = ex(v->expr(), envir);
            CHECKTHROWN(val)
            CHECKTHROWN(assignToExpression(v->expr(), binaryOp(Ast::BinaryOperator::Minus, val, 1), envir));
            return val;
        }
        default:
            X_UNREACHABLE();
        }
        break;
    }

    case Ast::Node::BinaryOperatorT: {
        Ast::BinaryOperator *v = p->as<Ast::BinaryOperator*>();
        return binaryOp(v->op, ex(v->left(), envir), ex(v->right(), envir));
    }

    case Ast::Node::ReturnT: {
        Ast::Return *v = p->as<Ast::Return*>();
        if (envir->parent) { // Only process return in functions
            envir->returnValue = ex(v->expression(), envir);
            envir->state = Environment::ReturnCalled;
        }
        return AVal();
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
            if (envir->state & Environment::FlowInterrupted) {
                break;
            }
        }
        break;
    }

    case Ast::Node::IfT: {
        Ast::If *v = p->as<Ast::If*>();
        AVal cond = ex(v->condition(), envir);
        CHECKTHROWN(cond)
        if (cond.toBool()) {
            CHECKTHROWN(ex(v->thenStatement(), envir));
        } else {
            CHECKTHROWN(ex(v->elseStatement(), envir));
        }
        break;
    }

    case Ast::Node::WhileT: {
        Ast::While *v = p->as<Ast::While*>();
        do  {
            AVal cond = ex(v->condition(), envir);
            CHECKTHROWN(cond)
            if(!cond.toBool())
              break;

            CHECKTHROWN(ex(v->statement(), envir))
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

        CHECKTHROWN(ex(v->init(), envir));
        while(1){
            AVal cond = ex(v->cond(), envir);
            CHECKTHROWN(cond)
            if(!cond.toBool())
                break;

            CHECKTHROWN(ex(v->statement(), envir))

            if (envir->state == Environment::BreakCalled) {
                envir->state = Environment::Normal;
                break;
            } else if (envir->state == Environment::ContinueCalled) {
                envir->state = Environment::Normal;
                continue;
            } else if (envir->state == Environment::ReturnCalled) {
                break;
            }

            CHECKTHROWN(ex(v->after(), envir))
        }
        break;
    }

    default:
        X_UNREACHABLE();
    }

    return AVal();
}


AVal INVOKE_INTERNAL( const char* name, Environment* envir, std::initializer_list<AVal> list )
{
    std::vector<Ast::Expression*> exprs;
    for(const AVal& v: list)
    {
        exprs.push_back(new Ast::AValLiteral(&v));
    }

    Ast::Node* call = new Ast::FunctionCall(new Ast::Variable(name), new Ast::ExpressionList(exprs));
    AVal r = ex(call, envir);
    delete call;
    CHECKTHROWN(r);
    return r;
}



static void defaultExceptionHandler(Environment* envir, AVal toPrint){
  toPrint.markThrown(false);
  printf("An uncaught exception occured\n");
  printf("Catched: ");
  INVOKE_INTERNAL("dump", envir, { toPrint });
  printf("Sorry, Bye :(\n");
  Evaluator::exit();
  ::exit(EXIT_FAILURE);
}


void init()
{
    Environment *global = new Environment;
    registerBuiltins(global);
    createGlobalScope(global);
    envirs.push_back(global);
    Parser::parseString(RSPHP_BOOTSTRAP);
}

void exit()
{
    for (Environment *e : envirs) {
        delete e;
    }
    envirs.clear();

    Ast::cleanup();
    MemoryPool::cleanup();
}

void eval(Ast::Node *p)
{
    Environment* global = envirs[0];
    AVal ret = ex(p, global);
    if(ret.isThrown()){
      defaultExceptionHandler(global, ret);
    }
}

std::vector<Environment*> environments()
{
    return envirs;
}

} // namespace Evaluator
