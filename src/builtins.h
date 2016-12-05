#pragma once

#include "ast.h"
#include "aval.h"

namespace Evaluator
{
    void registerBuiltins(Environment *e);

    AVal doBuiltInTypeof(const std::vector<Ast::Expression*> &, Environment *);
    AVal doBuiltInReadInt(const std::vector<Ast::Expression*> &, Environment *);
    AVal doBuiltInReadDouble(const std::vector<Ast::ExpressionList*> &, Environment *);
    AVal doBuiltInReadString(const std::vector<Ast::ExpressionList*> &, Environment *);
    AVal doBuiltInReadBool(const std::vector<Ast::ExpressionList*> &, Environment *);
    AVal doBuiltInPrint(const std::vector<Ast::ExpressionList*> &, Environment *);
    AVal doBuiltInGC(const std::vector<Ast::ExpressionList*> &, Environment *);
    AVal doBuiltInDumpAST(const std::vector<Ast::ExpressionList*> &, Environment *);
    AVal doBuiltInExit(const std::vector<Ast::ExpressionList*> &, Environment *);

    // Arrays
    AVal doBuiltInArray(const std::vector<Ast::ExpressionList*> &, Environment *);
    AVal doBuiltInCount(const std::vector<Ast::ExpressionList*> &, Environment *);
    AVal doBuiltInPush(const std::vector<Ast::Expression*> &, Environment *);

}
