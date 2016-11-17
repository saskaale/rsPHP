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
"while"         return WHILE;
"if"            return IF;
"else"          return ELSE;
"print"         return PRINT;


[a-z][a-z]* { 
                yylval.sIndex = *yytext - 'a';
                strncpy(yylval.str, yytext, ID_LEN);
                yylval.str[ID_LEN-1] = '\0';
                return VARIABLE;
            }

[ \t\n]+        ;       /* ignore whitespace */

.               yyerror("Unknown character");

%%

int yywrap(void) {
    return 1;
}
