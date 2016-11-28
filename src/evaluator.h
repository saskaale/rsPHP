#pragma once

#include "ast.h"

class Environment;

namespace Evaluator
{
    void init();
    void exit();

    void eval(Ast::Node *p);
    void cleanup(Ast::Node *p);

    std::vector<Environment*> environments();
}
