#include "parser.h"
#include "evaluator.h"

int main(int argc, char *argv[])
{
    Evaluator::init();
    yyparse();
    return 0;
}
