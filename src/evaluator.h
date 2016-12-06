#pragma once

#include <vector>
#include <unordered_set>
#include "ast.h"
#include "builtins.h"


#define THROW2(name, descr) {char buf[256];sprintf(buf, name, descr);AVal a(buf);a.markThrown(true);return a; }
#define THROW(name) {AVal a(name);a.markThrown(true);return a;}
#define CHECKTHROWN(v) {AVal s=(v);if(s.isThrown())return s;}



class Environment;

namespace Evaluator
{

    enum ExFlag {
        NoFlag = 0,
        ReturnLValue = 1
    };

    
    void init();
    void exit();

    void eval(Ast::Node *p);

    void setExFlag(ExFlag flag);
    bool testExFlag(ExFlag flag);
    void clearExFlag(ExFlag flag);
    AVal ex(Ast::Node *p, Environment* envir);

    AVal INVOKE_INTERNAL( const char* name, Environment* envir, std::initializer_list<AVal> list );

    std::vector<Environment*> environments();
}
