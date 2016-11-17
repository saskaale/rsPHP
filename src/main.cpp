#include <iostream>
#include "ast.h"
#include "parser.hpp"

int main(int argc, char *argv[])
{
    std::cout << "rsphp" << std::endl;
    yyparse();
    return 0;
}
