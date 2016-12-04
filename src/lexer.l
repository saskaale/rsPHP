%{
#include <stdlib.h>

#include "parser.h"

void yyerror(const char *);
void comment();
%}

%x C_COMMENT
%x CPP_COMMENT

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
"&"             return REFERENCE;
"&&"            return AND;
"||"            return OR;
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
"==="           return EQ_TYPE;
"!=="           return NE_TYPE;
"for"           return FOR;
"function"      return FUNCTION;
"while"         return WHILE;
"if"            return IF;
"throw"         return THROW;
"try"           return TRY;
"catch"         return CATCH;
"else"          return ELSE;
"return"        return RETURN;
"break"         return BREAK;
"continue"      return CONTINUE;
"print"         return PRINT;
"true"          return TRUE;
"false"         return FALSE;
"undefined"     return UNDEFINED;

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

"/*"              { BEGIN(C_COMMENT); }
<C_COMMENT>"*/"   { BEGIN(INITIAL); }
<C_COMMENT>"\n" { }
<C_COMMENT>.      { }
"//"              { BEGIN(CPP_COMMENT); }
<CPP_COMMENT>"\n" { BEGIN(INITIAL); }
<CPP_COMMENT>.    { }

[ \t\n]+        { }

.               yyerror("Unknown character");

%%

int yywrap()
{
    return 1;
}
