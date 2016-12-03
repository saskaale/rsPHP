#pragma once

#include "ast.h"
#include "builtins.h"


#define THROW2(name, descr) {(fprintf(stderr, name, descr), fprintf(stderr, "\n"), X_ASSERT(false && name)); return AVal()};

#define THROW(name) { (fprintf(stderr, name), fprintf(stderr, "\n"), X_ASSERT(false && name)); return AVal();}



class Environment;

namespace Evaluator
{
    void init();
    void exit();

    void eval(Ast::Node *p);
    void cleanup(Ast::Node *p);

    AVal ex(Ast::Node *p, Environment* envir);
    
    std::vector<Environment*> environments();
}
