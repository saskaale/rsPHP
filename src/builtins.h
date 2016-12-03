#pragma once

#include "ast.h"
#include "aval.h"


namespace Evaluator
{

    void registerBuiltins(Environment *e);
  
    AVal doBuiltInScan(Ast::ExpressionList *v, Environment*);
    AVal doBuiltInPrint(Ast::ExpressionList *v, Environment*);
    AVal doBuiltInGC(Ast::ExpressionList *, Environment *);
    AVal doBuiltInDumpAST(Ast::ExpressionList *, Environment *);

}
