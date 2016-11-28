#include <iostream>
#include "parser.h"

int main(int argc, char *argv[])
{
    std::cout << "rsphp" << std::endl;
    yyparse();
    return 0;
}
