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
%token FOR WHILE IF PRINT
%nonassoc IFX
%nonassoc ELSE

%left GE LE EQ NE '>' '<'
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS

%type <nPtr> stmt stmt2 expr stmt_list

%%

program:
        function                { exit(0); }
        ;

function:
          function stmt         { ex($2); /*freeNode($2);*/ }
        | /* NULL */
        ;

stmt2:
                                                  { $$ = new Ast::ValueLiteral(true); }
        | expr                                    { $$ = $1; }
        | PRINT expr                              { $$ = new Ast::FunctionCall("print", $2); }


stmt:
        | stmt2 ';'                               { $$ = $1; }
        | WHILE '(' expr ')' stmt                 { $$ = new Ast::While($3, $5); }
        | IF '(' expr ')' stmt %prec IFX          { $$ = new Ast::If($3, $5, nullptr); }
        | IF '(' expr ')' stmt ELSE stmt          { $$ = new Ast::If($3, $5, $7); }
        | FOR '(' stmt2 ';' stmt2 ';' stmt2 ')' stmt { $$ = new Ast::For($3, $5, $7, $9); }
        | '{' stmt_list '}'                       { $$ = $2; }
        ;

stmt_list:
          stmt                  { $$ = new Ast::StatementList($1); }
        | stmt_list stmt        {
                                    Ast::StatementList *stmlist = $1->as<Ast::StatementList*>();
                                    Ast::Statement *stm = $2->as<Ast::Statement*>();
                                    $$ = new Ast::StatementList(stm, stmlist);
                                }
        ;

expr:
          INTEGER               { $$ = new Ast::ValueLiteral($1); }
        | VARIABLE              { $$ = new Ast::Variable($1); }
        | VARIABLE '=' expr     { $$ = new Ast::Assignment(new Ast::Variable($1), $3); }
        | '-' expr %prec UMINUS { $$ = new Ast::UnaryOperator(Ast::UnaryOperator::Minus, $2); }
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
