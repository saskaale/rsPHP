#pragma once

#include "ast.h"
#include "aval.h"


namespace Evaluator
{

    AVal doBuiltInPrint(Ast::ExpressionList *v, Environment*);
    AVal doBuiltInGC(Ast::ExpressionList *, Environment *);
    AVal doBuiltInDumpAST(Ast::ExpressionList *, Environment *);

}
