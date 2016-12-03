#pragma once

#include "ast.h"
#include "aval.h"

namespace Evaluator
{

    void registerBuiltins(Environment *e);

    AVal doBuiltInTypeof(Ast::ExpressionList *v, Environment*);
    AVal doBuiltInReadInt(Ast::ExpressionList *v, Environment*);
    AVal doBuiltInReadDouble(Ast::ExpressionList *v, Environment*);
    AVal doBuiltInReadString(Ast::ExpressionList *v, Environment*);
    AVal doBuiltInReadBool(Ast::ExpressionList *v, Environment*);
    AVal doBuiltInPrint(Ast::ExpressionList *v, Environment*);
    AVal doBuiltInGC(Ast::ExpressionList *, Environment *);
    AVal doBuiltInDumpAST(Ast::ExpressionList *, Environment *);

}
