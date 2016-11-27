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

[0-9]+("."[0-9]+)? {
                if (strchr(yytext, '.')) {
                    yylval.fValue = atof(yytext);
                    return DOUBLE;
                } else {
                    yylval.iValue = atoi(yytext);
                    return INTEGER;
                }
            }

[()<>=*/;{}.] {
                return *yytext;
             }

"+"             return PLUS;
"-"             return MINUS;
"++"            return INCREMENT;
"--"            return DECREMENT;
">="            return GE;
"<="            return LE;
"=="            return EQ;
"!="            return NE;
"for"           return FOR;
"function"      return FUNCTION;
"while"         return WHILE;
"if"            return IF;
"else"          return ELSE;
"print"         return PRINT;
"true"          return TRUE;
"false"         return FALSE;

\"(\\.|[^\\"])*\" {
                size_t size = strlen(yytext) + 1;
                yylval.str = (char*) malloc(size);
                strncpy(yylval.str, yytext, size);
                return STRING;
            }

[A-Za-z][0-9A-Za-z]* {
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
