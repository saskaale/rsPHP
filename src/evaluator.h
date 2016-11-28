#pragma once

#include "ast.h"

namespace Evaluator
{
    void eval(Ast::Node *p);
    void cleanup(Ast::Node *p);
}
