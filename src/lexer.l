%{
#include <stdlib.h>

#include "parser.h"

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

("[")    { return '['; }
("]")    { return ']'; }
(",")    { return ','; }
[();{}.] { return *yytext; }

"="             return ASSIGN;
"+"             return PLUS;
"-"             return MINUS;
"*"             return TIMES;
"/"             return DIV;
"%"             return MOD;
"<"             return LESS;
">"             return GREATER;
"++"            return INCREMENT;
"--"            return DECREMENT;
">="            return GE;
"<="            return LE;
"=="            return EQ;
"!="            return NE;
"+="            return AS_PLUS;
"-="            return AS_MINUS;
"*="            return AS_TIMES;
"/="            return AS_DIV;
"%="            return AS_MOD;
"for"           return FOR;
"function"      return FUNCTION;
"while"         return WHILE;
"if"            return IF;
"else"          return ELSE;
"return"        return RETURN;
"break"         return BREAK;
"continue"      return CONTINUE;
"print"         return PRINT;
"true"          return TRUE;
"false"         return FALSE;

\"(\\.|[^\\"])*\" {
                size_t size = strlen(yytext);
                yylval.str = (char*) malloc(size);
                strncpy(yylval.str, yytext + 1, size);
                yylval.str[size - 2] = 0;
                return STRING;
            }

[A-Za-z][0-9A-Za-z]* {
                yylval.str = strdup(yytext);
                return VARIABLE;
            }

[ \t\n]+        ;       /* ignore whitespace */

.               yyerror("Unknown character");

%%

int yywrap(void) {
    return 1;
}
