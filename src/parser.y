%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "ast.h"
#include <iostream>

/* prototypes */
/* int ex(nodeType *p); */
int ex(Ast::Node *p);

int yylex(void);

void yyerror(const char *s);
%}

%union {
    int iValue;
    char *str;
    Ast::Node *nPtr;
};

%token <iValue> INTEGER
%token <str> VARIABLE
%token WHILE IF PRINT
%nonassoc IFX
%nonassoc ELSE

%left GE LE EQ NE '>' '<'
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS

%type <nPtr> stmt expr stmt_list

%%

program:
        function                { exit(0); }
        ;

function:
          function stmt         { ex($2); /*freeNode($2);*/ }
        | /* NULL */
        ;

stmt:
          ';'                            { $$ = /*new Ast::Statement()*/0; }
        | expr ';'                       { $$ = $1; }
        | PRINT expr ';'                 { $$ = new Ast::FunctionCall("print", $2); }
        | VARIABLE '=' expr ';'          { $$ = new Ast::Assignment(new Ast::Variable($1), $3); }
        | WHILE '(' expr ')' stmt        { $$ = /*opr(WHILE, 2, $3, $5)*/0; }
        | IF '(' expr ')' stmt %prec IFX { $$ = /*opr(IF, 2, $3, $5)*/0; }
        | IF '(' expr ')' stmt ELSE stmt { $$ = /*opr(IF, 3, $3, $5, $7)*/0; }
        | '{' stmt_list '}'              { $$ = $2; }
        ;

stmt_list:
          stmt                  { $$ = $1; }
        | stmt_list stmt        { $$ = /*opr(';', 2, $1, $2)*/0; }
        ;

expr:
          INTEGER               { $$ = new Ast::IntegerLiteral($1); }
        | VARIABLE              { $$ = new Ast::Variable($1); }
        | '-' expr %prec UMINUS { $$ = /*opr(UMINUS, 1, $2)*/0; }
        | expr '+' expr         { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::Plus, $1, $3); }
        | expr '-' expr         { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::Minus, $1, $3); }
        | expr '*' expr         { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::Times, $1, $3); }
        | expr '/' expr         { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::Div, $1, $3); }
        | expr '<' expr         { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::LessThan, $1, $3); }
        | expr '>' expr         { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::GreaterThan, $1, $3); }
        | expr GE expr          { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::GreaterThanEqual, $1, $3); }
        | expr LE expr          { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::LessThanEqual, $1, $3); }
        | expr NE expr          { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::NotEqual, $1, $3); }
        | expr EQ expr          { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::Equal, $1, $3); }
        | '(' expr ')'          { $$ = $2; }
        ;

%%


void yyerror(const char *s) {
    fprintf(stdout, "%s\n", s);
}
