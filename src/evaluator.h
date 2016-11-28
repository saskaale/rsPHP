#pragma once

#include "ast.h"

namespace Evaluator
{
    void init();
    void exit();
    void eval(Ast::Node *p);
    void cleanup(Ast::Node *p);
}
