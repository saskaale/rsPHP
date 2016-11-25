%{
#include <stdlib.h>
#include "ast.h"
#include "parser.hpp"

void yyerror(const char *);
%}

%%


0           {
                yylval.iValue = atoi(yytext);
                return INTEGER;
            }

[1-9][0-9]* {
                yylval.iValue = atoi(yytext);
                return INTEGER;
            }

[-()<>=+*/;{}.] {
                return *yytext;
             }

">="            return GE;
"<="            return LE;
"=="            return EQ;
"!="            return NE;
"for"           return FOR;
"while"         return WHILE;
"if"            return IF;
"else"          return ELSE;
"print"         return PRINT;
"true"          return TRUE;
"false"         return FALSE;


[a-z][a-z]* {
                size_t size = strlen(yytext) + 1;
                yylval.str = (char*) malloc(size);
                strncpy(yylval.str, yytext, size);
                return VARIABLE;
            }

[ \t\n]+        ;       /* ignore whitespace */

.               yyerror("Unknown character");

%%

int yywrap(void) {
    return 1;
}
