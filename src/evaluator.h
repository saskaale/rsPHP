#pragma once

#include <vector>
#include "ast.h"
#include "builtins.h"


#define THROW2(name, descr) {(fprintf(stderr, name, descr), fprintf(stderr, "\n"), X_ASSERT(false && name)); return AVal(name, true);};
#define THROW(name) { (fprintf(stderr, name), fprintf(stderr, "\n"), X_ASSERT(false && name)); return AVal(name, true);}
#define CHECKTHROWN(v) {if((v).isThrown()) return v;}



class Environment;

namespace Evaluator
{

    typedef std::vector<AVal> Stack;

    class StackFrame{
        Stack* stack;
        int cnt;
        public:
            StackFrame(Stack*);
            ~StackFrame();
            void push(const AVal& v);
    };

    void init();
    void exit();

    void eval(Ast::Node *p);
    void cleanup(Ast::Node *p);

    AVal ex(Ast::Node *p, Environment* envir);

    AVal INVOKE_INTERNAL( const char* name, Environment* envir, std::initializer_list<AVal> list );
    
    std::vector<Environment*> environments();
}
