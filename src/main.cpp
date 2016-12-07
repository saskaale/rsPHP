#include "parser.h"
#include "evaluator.h"

static void interpretFile(FILE *file)
{
    Evaluator::init();
    Parser::parseFile(file);
    Evaluator::exit();
}

int main(int argc, char *argv[])
{
    srand (time(NULL));

    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            FILE *f = fopen(argv[i], "r");
            if (!f) {
                fprintf(stderr, "Cannot read file %s!\n", argv[i]);
                return 1;
            }
            interpretFile(f);
            fclose(f);
        }
    } else {
        interpretFile(stdin);
    }

    return 0;
}
