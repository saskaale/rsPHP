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
AVal doBuiltInPrint(Ast::ExpressionList *v, Environment* envir)
{
    if(v->expressions.empty())
      THROW("Print function expects at least one parameter.")

    AVal printV = ex(v->expressions.front(), envir);

    return printf("%s\n", printV.toString());
}

AVal doBuiltInTypeof(Ast::ExpressionList *v, Environment* envir)
{
    if(v->expressions.empty())
      THROW("Print function expects at least one parameter.")

    AVal printV = ex(v->expressions.front(), envir);

    return printV.typeStr();
}


AVal doBuiltInReadInt(Ast::ExpressionList *v, Environment* envir)
{
    int val;
    if(!scanf("%d\n", &val))
        return AVal();
    return val;
}

AVal doBuiltInReadDouble(Ast::ExpressionList *v, Environment* envir)
{
    double val;
    if(!scanf("%lf\n", &val))
        return AVal();
    return val;
}

AVal doBuiltInReadString(Ast::ExpressionList *v, Environment* envir)
{
    char str[100];
    if(!scanf("%99s\n", str))
        return AVal();
    str[99] = '\0';
    return str;
}

AVal doBuiltInReadBool(Ast::ExpressionList *v, Environment* envir)
{
    AVal val = doBuiltInReadString(v, envir);
    if(val.type() == AVal::Type::STRING){
        if(strcmp(val.toString(), "true"))
            return true;
        if(strcmp(val.toString(), "false"))
            return true;
    }
    return AVal();
}

AVal doBuiltInGC(Ast::ExpressionList *, Environment *)
{
    printf("MemoryPool::collectGarbage()\n");
    MemoryPool::collectGarbage();
    return AVal();
}


#define PADDEDOUT(lvl) for(int i = 0; i < lvl; i++) printf(" ");

void astDump(Ast::Node* p, int lvl = 0){
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
        printf(">>%p<<\n", p);
        break;
    }

    case Ast::Node::ArraySubscriptT: {
        printf("\n");
        PADDEDOUT(lvl+1); printf("NAME >>%s<<:\n", p->as<Ast::Variable*>()->name.c_str());

        PADDEDOUT(lvl+1); printf("INDEX:\n");
          astDump(p->as<Ast::ArraySubscript*>()->expression, lvl+2);
        break;
    }

    case Ast::Node::AssignmentT: {
        Ast::Assignment *v = p->as<Ast::Assignment*>();
        printf("\n");
        PADDEDOUT(lvl+1); printf("EXPR:\n");
          astDump(v->expression, lvl+2);

        if (Ast::ArraySubscript *as = v->variable->as<Ast::ArraySubscript*>()) {
            PADDEDOUT(lvl+1); printf("ARRAYSUBSCRIPT:\n");
              astDump(v->variable, lvl+2);
        }

        break;
    }

    case Ast::Node::FunctionT: {
        Ast::Function *v = p->as<Ast::Function*>();
        printf("\n");
        PADDEDOUT(lvl+1); printf("NAME: >>%s<<", v->name.c_str());
        PADDEDOUT(lvl+1);  printf("PARAMETERS:\n");
          astDump(v->parameters, lvl+2);
        PADDEDOUT(lvl+1);  printf("STATEMENTS:\n");
          astDump(v->statements, lvl+2);
        break;
    }

    case Ast::Node::FunctionCallT: {
        Ast::FunctionCall *v = p->as<Ast::FunctionCall*>();
        printf("\n");
        PADDEDOUT(lvl+1); printf("FUNCTION:\n");
          astDump(v->function, lvl+2);
        PADDEDOUT(lvl+1); printf("ARGUMENTS:\n");
          astDump(v->arguments, lvl+2);
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
        astDump(v->expr, lvl+1);
        break;
    }

    case Ast::Node::BinaryOperatorT: {
        Ast::BinaryOperator *v = p->as<Ast::BinaryOperator*>();

        printf("%s\n", v->opStr());
        PADDEDOUT(lvl+1); printf("LEFT:\n");
          astDump(v->left, lvl+2);
        PADDEDOUT(lvl+1); printf("RIGHT:\n");
          astDump(v->right, lvl+2);

        break;
    }

    case Ast::Node::ReturnT: {
        printf("\n");
        astDump(p->as<Ast::Return*>()->expression, lvl+1);
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
            astDump(s, lvl+2);
        }
        break;
    }

    case Ast::Node::ExpressionListT: {
        printf("\n");
        Ast::ExpressionList *v = p->as<Ast::ExpressionList*>();
        int i = 0;
        for (Ast::Expression *s : v->expressions) {
            PADDEDOUT(lvl+1); printf("%d:\n", i++);
            astDump(s, lvl+2);
        }
        break;
    }

    case Ast::Node::VariableListT: {
        printf("\n");
        Ast::VariableList *v = p->as<Ast::VariableList*>();
        int i = 0;
        for (Ast::Variable *var : v->variables) {
            PADDEDOUT(lvl+1); printf("%d:\n", i++);
            astDump(var, lvl+2);
        }
        break;
    }

    case Ast::Node::IfT: {
        Ast::If *v = p->as<Ast::If*>();
        printf("\n");
        PADDEDOUT(lvl+1); printf("COND:\n");
          astDump(v->condition, lvl+2);
        PADDEDOUT(lvl+1); printf("THEN:\n");
          astDump(v->thenStatement, lvl+2);
        PADDEDOUT(lvl+1); printf("ELSE:\n");
          astDump(v->elseStatement, lvl+2);
        break;
    }

    case Ast::Node::WhileT: {
        Ast::While *v = p->as<Ast::While*>();
        printf("\n");
        PADDEDOUT(lvl+1); printf("COND:\n");
          astDump(v->condition, lvl+2);
        PADDEDOUT(lvl+1); printf("BODY:\n");
          astDump(v->statement, lvl+2);
        break;
    }

    case Ast::Node::ForT: {
        Ast::For *v = p->as<Ast::For*>();

        printf("\n");
        PADDEDOUT(lvl+1); printf("INIT:\n");
          astDump(v->init, lvl+2);
        PADDEDOUT(lvl+1); printf("COND:\n");
          astDump(v->cond, lvl+2);
        PADDEDOUT(lvl+1); printf("AFTER:\n");
          astDump(v->after, lvl+2);
        PADDEDOUT(lvl+1); printf("BODY:\n");
          astDump(v->statement, lvl+2);
        break;
    }

    default:
        X_UNREACHABLE();
    }

    return;

}


AVal doBuiltInDumpAST(Ast::ExpressionList *v, Environment *envir){
    if(v->expressions.empty())
      THROW("dumpAST function expects at least one parameter.")


    Ast::Node* toprint = v->expressions.front();
//    AVal printV = ex(v->expressions.front(), envir);
//    if(!printV.type() != AVal::FUNCTION)
//      THROW("type must be function")
    astDump(toprint);

    return AVal();
}


void registerBuiltins(Environment* e){
    e->setFunction("typeof", &doBuiltInTypeof);
    e->setFunction("readInt", &doBuiltInReadInt);
    e->setFunction("readDouble", &doBuiltInReadDouble);
    e->setFunction("readString", &doBuiltInReadString);
    e->setFunction("print", &doBuiltInPrint);
    e->setFunction("dumpAST", &doBuiltInDumpAST);
    e->setFunction("gc", &doBuiltInGC);
}


}
