#include "evaluator.h"
#include "common.h"
#include "parser.h"
#include "environment.h"
#include "memorypool.h"
#include "evaluator.h"

#include <iostream>
#include <algorithm>
#include <cstring>


namespace Evaluator
{

// Functions
AVal doBuiltInPrint(const std::vector<Ast::Expression*> &arguments, Environment *envir)
{
    if (arguments.empty()) {
        THROW("Print function expects at least one parameter.");
    }

    std::string out;
    for (Ast::Expression *arg : arguments) {
        AVal printV = ex(arg, envir);
        CHECKTHROWN(printV);
        if (!out.empty()) {
            out += " ";
        }
        out += printV.toString();
    }

    return printf("%s\n", out.c_str());
}

AVal doBuiltInTypeof(const std::vector<Ast::Expression*> &arguments, Environment *envir)
{
    if (arguments.empty())
      THROW("Print function expects at least one parameter.")

    AVal printV = ex(arguments[0], envir);
    CHECKTHROWN(printV)

    return printV.typeStr();
}


AVal doBuiltInReadInt(const std::vector<Ast::Expression*> &, Environment *)
{
    int val;
    if(!scanf("%d\n", &val))
        return AVal();
    return val;
}

AVal doBuiltInReadDouble(const std::vector<Ast::Expression*> &, Environment *)
{
    double val;
    if(!scanf("%lf\n", &val))
        return AVal();
    return val;
}

AVal doBuiltInReadString(const std::vector<Ast::Expression*> &, Environment *)
{
    char str[100];
    if(!scanf("%99s\n", str))
        return AVal();
    str[99] = '\0';
    return str;
}

AVal doBuiltInReadBool(const std::vector<Ast::Expression*> &arguments, Environment *envir)
{
    AVal val = doBuiltInReadString(arguments, envir);
    CHECKTHROWN(val)
    if(val.isString()){
        if(strcmp(val.toString(), "true"))
            return true;
        if(strcmp(val.toString(), "false"))
            return true;
    }
    return AVal();
}

AVal doBuiltInGC(const std::vector<Ast::Expression*> &, Environment *)
{
    printf("MemoryPool::collectGarbage()\n");
    MemoryPool::collectGarbage(false);
    return AVal();
}

AVal doBuiltInThrow(const std::vector<Ast::Expression*> &arguments, Environment *envir)
{
    if (arguments.empty())
      THROW("Throw function expects at least one parameter.")

    AVal val = ex(arguments[0], envir);
    CHECKTHROWN(val)

    val.markThrown();

    return val;
}


#define PADDEDOUT(lvl) for(int i = 0; i < lvl; i++) printf(" ");

void astDump(Ast::Node* p, Environment* envir, int lvl = 0){
    if (!p) {
      PADDEDOUT(lvl); printf("???\n");
      return;
    }

    PADDEDOUT(lvl); printf("%s ", p->typeStr());

    switch (p->type()) {

    case Ast::Node::IntegerLiteralT:
        printf("%d\n", p->as<Ast::IntegerLiteral*>()->value);
        break;

    case Ast::Node::BoolLiteralT:
        printf("%s\n", p->as<Ast::BoolLiteral*>()->value ? "true":"false");
        break;

    case Ast::Node::DoubleLiteralT:
        printf("%lf\n", p->as<Ast::DoubleLiteral*>()->value);
        break;

    case Ast::Node::StringLiteralT:
        printf(">>%s<<\n", p->as<Ast::StringLiteral*>()->value.c_str());
        break;

    case Ast::Node::VariableT: {
        Ast::Variable *v = p->as<Ast::Variable*>();
        printf(">>%p %s<<\n", p, v->name.c_str());
        break;
    }

    case Ast::Node::AValLiteralT:
        Evaluator::INVOKE_INTERNAL( "print", envir, { *((AVal*)p->as<Ast::AValLiteral*>()->value()) } );
        break;

    case Ast::Node::ArraySubscriptT: {
        printf("\n");
        PADDEDOUT(lvl+1); printf("NAME >>%s<<:\n", p->as<Ast::Variable*>()->name.c_str());

        PADDEDOUT(lvl+1); printf("INDEX:\n");
          astDump(p->as<Ast::ArraySubscript*>()->expression(), envir, lvl+2);
        break;
    }

    case Ast::Node::AssignmentT: {
        Ast::Assignment *v = p->as<Ast::Assignment*>();
        printf("\n");
        astDump(v->destination(), envir, lvl);
        PADDEDOUT(lvl+1); printf("EXPR:\n");
          astDump(v->expression(), envir, lvl+2);
        break;
    }


    case Ast::Node::TryT: {
        Ast::Try *v = p->as<Ast::Try*>();
        printf("\n");
        PADDEDOUT(lvl+1); printf("BODY\n");
          astDump(v->body(), envir, lvl+2);
        PADDEDOUT(lvl+1); printf("VARIABLES\n");
          astDump(v->variables(), envir, lvl+2);
        PADDEDOUT(lvl+1); printf("CATCH\n");
          astDump(v->catchPart(), envir, lvl+2);
        break;
    }


    case Ast::Node::FunctionT: {
        Ast::Function *v = p->as<Ast::Function*>();
        printf("\n");
        PADDEDOUT(lvl+1); printf("NAME: >>%s<<", v->name.c_str());
        PADDEDOUT(lvl+1);  printf("PARAMETERS:\n");
          astDump(v->parameters(), envir, lvl+2);
        PADDEDOUT(lvl+1);  printf("STATEMENTS:\n");
          astDump(v->statements(), envir, lvl+2);
        break;
    }

    case Ast::Node::FunctionCallT: {
        Ast::FunctionCall *v = p->as<Ast::FunctionCall*>();
        printf("\n");
        PADDEDOUT(lvl+1); printf("FUNCTION:\n");
          astDump(v->function(), envir, lvl+2);
        PADDEDOUT(lvl+1); printf("ARGUMENTS:\n");
          astDump(v->arguments(), envir, lvl+2);
        break;
    }

    case Ast::Node::UnaryOperatorT: {
        Ast::UnaryOperator *v = p->as<Ast::UnaryOperator*>();

        switch (v->op) {
          case Ast::UnaryOperator::Minus:
            printf("MINUS:\n");
            break;
          case Ast::UnaryOperator::PreIncrement:
            printf("PreIncrement:\n");
            break;
          case Ast::UnaryOperator::PreDecrement:
            printf("PreDecrement:\n");
            break;
          case Ast::UnaryOperator::PostIncrement:
            printf("PostIncrement:\n");
            break;
          case Ast::UnaryOperator::PostDecrement:
            printf("PostDecrement:\n");
            break;
        }
        astDump(v->expr(), envir, lvl+1);
        break;
    }

    case Ast::Node::BinaryOperatorT: {
        Ast::BinaryOperator *v = p->as<Ast::BinaryOperator*>();

        printf("%s\n", v->opStr());
        PADDEDOUT(lvl+1); printf("LEFT:\n");
          astDump(v->left(), envir, lvl+2);
        PADDEDOUT(lvl+1); printf("RIGHT:\n");
          astDump(v->right(), envir, lvl+2);

        break;
    }

    case Ast::Node::ReturnT: {
        printf("\n");
        astDump(p->as<Ast::Return*>()->expression(), envir, lvl+1);
        return;
    }

    case Ast::Node::BreakT: {
        printf("\n");
        break;
    }

    case Ast::Node::ContinueT: {
        printf("\n");
        break;
    }

    case Ast::Node::StatementListT: {
        printf("\n");
        Ast::StatementList *v = p->as<Ast::StatementList*>();
        int i = 0;
        for (Ast::Statement *s : v->statements) {
            PADDEDOUT(lvl+1); printf("%d:\n", i++);
            astDump(s, envir, lvl+2);
        }
        break;
    }

    case Ast::Node::ExpressionListT: {
        printf("\n");
        Ast::ExpressionList *v = p->as<Ast::ExpressionList*>();
        int i = 0;
        for (Ast::Expression *s : v->expressions()) {
            PADDEDOUT(lvl+1); printf("%d:\n", i++);
            astDump(s, envir, lvl+2);
        }
        break;
    }

    case Ast::Node::VariableListT: {
        printf("\n");
        Ast::VariableList *v = p->as<Ast::VariableList*>();
        int i = 0;
        for (Ast::Variable *var : v->variables) {
            PADDEDOUT(lvl+1); printf("%d:\n", i++);
            astDump(var, envir, lvl+2);
        }
        break;
    }

    case Ast::Node::IfT: {
        Ast::If *v = p->as<Ast::If*>();
        printf("\n");
        PADDEDOUT(lvl+1); printf("COND:\n");
          astDump(v->condition(), envir, lvl+2);
        PADDEDOUT(lvl+1); printf("THEN:\n");
          astDump(v->thenStatement(), envir, lvl+2);
        PADDEDOUT(lvl+1); printf("ELSE:\n");
          astDump(v->elseStatement(), envir, lvl+2);
        break;
    }

    case Ast::Node::WhileT: {
        Ast::While *v = p->as<Ast::While*>();
        printf("\n");
        PADDEDOUT(lvl+1); printf("COND:\n");
          astDump(v->condition(), envir, lvl+2);
        PADDEDOUT(lvl+1); printf("BODY:\n");
          astDump(v->statement(), envir, lvl+2);
        break;
    }

    case Ast::Node::ForT: {
        Ast::For *v = p->as<Ast::For*>();

        printf("\n");
        PADDEDOUT(lvl+1); printf("INIT:\n");
          astDump(v->init(), envir, lvl+2);
        PADDEDOUT(lvl+1); printf("COND:\n");
          astDump(v->cond(), envir, lvl+2);
        PADDEDOUT(lvl+1); printf("AFTER:\n");
          astDump(v->after(), envir, lvl+2);
        PADDEDOUT(lvl+1); printf("BODY:\n");
          astDump(v->statement(), envir, lvl+2);
        break;
    }

    default:
        X_UNREACHABLE();
    }

    return;

}


AVal doBuiltInDumpAST(const std::vector<Ast::Expression*> &arguments, Environment *envir)
{
    if (arguments.empty())
      THROW("dumpAST function expects at least one parameter.")


    Ast::Node* toprint = arguments[0];
//    AVal printV = ex(v->expressions.front(), envir);
//    if(!printV.type() != AVal::FUNCTION)
//      THROW("type must be function")
    astDump(toprint, envir);

    return AVal();
}

AVal doBuiltInExit(const std::vector<Ast::Expression*> &arguments, Environment *envir)
{
    if (arguments.size() > 1) {
        THROW("exit() takes one or zero arguments.");
    }

    const int ret = arguments.empty() ? 0 : ex(arguments[0], envir).toInt();
    Evaluator::exit();
    ::exit(ret);
}

AVal doBuiltInArray(const std::vector<Ast::Expression*> &arguments, Environment *envir)
{
    if (arguments.size() > 2) {
        THROW("Array() takes one, zero or two arguments.");
    }

    const int size = arguments.empty() ? 0 : ex(arguments[0], envir).toInt();
    if (size < 0) {
        THROW("Array size cannot be negative.");
    }

    void *mem;
    AArray *a = (AArray*)MemoryPool::alloc(AArray::allocSize(size), &mem);
    a->mem = mem;
    a->count = 0;
    a->allocd = size;

    if (arguments.size() == 2) {
        AVal initializer = ex(arguments[1], envir);
        for (int i = 0; i < size; ++i) {
            a->array[i] = initializer.copy();
        }
        a->count = size;
    }

    return a;
}

AVal doBuiltInCount(const std::vector<Ast::Expression*> &arguments, Environment *envir)
{
    if (arguments.size() != 1) {
        THROW("count() takes one argument.");
    }

    AVal arr = ex(arguments[0], envir);
    if (arr.isArray()) {
        return int(arr.toArray()->count);
    } else if (arr.isString()) {
        return int(strlen(arr.toString()));
    } else {
        THROW("count() argument must be of type array or string.");
    }
}

AVal doBuiltInPush(const std::vector<Ast::Expression*> &arguments, Environment *envir)
{
    if (arguments.size() != 2) {
        THROW("push() takes two arguments.");
    }

    AVal arr = ex(arguments[0], envir);
    if (!arr.isReference() || !arr.toReference()->isArray()) {
        THROW("push() argument 1 must be reference to type array.");
    }

    AVal value = ex(arguments[1], envir).dereference();
    AVal *ref = arr.toReference();
    AArray *ar = ref->toArray();

    if (ar->count >= ar->allocd) {
        const int newallocd = (ar->count + 1) * 2;
        void *mem;
        AArray *tmp = (AArray*)MemoryPool::alloc(AArray::allocSize(newallocd), &mem);
        tmp->mem = mem;
        for (size_t i = 0; i < ar->count; ++i) {
            tmp->array[i] = ar->array[i];
        }
        tmp->count = ar->count;
        tmp->allocd = newallocd;
        ar = tmp;
        ref->arrayValue = ar;
    }

    ar->array[ar->count++] = value;
    return AVal();
}

void registerBuiltins(Environment* e)
{
    e->set("typeof", &doBuiltInTypeof);
    e->set("readInt", &doBuiltInReadInt);
    e->set("readDouble", &doBuiltInReadDouble);
    e->set("readString", &doBuiltInReadString);
    e->set("print", &doBuiltInPrint);
    e->set("dump", &doBuiltInPrint);
    e->set("throw", &doBuiltInThrow);
    e->set("dumpAST", &doBuiltInDumpAST);
    e->set("gc", &doBuiltInGC);
    e->set("exit", &doBuiltInExit);
    e->set("Array", &doBuiltInArray);
    e->set("count", &doBuiltInCount);
    e->set("pushinternal", &doBuiltInPush);
}

}
