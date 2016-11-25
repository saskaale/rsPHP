%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "ast.h"
#include "environment.h"
#include <iostream>


/* prototypes */

Environment globalenvir;

/* int ex(nodeType *p); */
Ast::Value ex(Ast::Node *p, Environment* env = &globalenvir);

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
%token FOR WHILE IF PRINT TRUE FALSE
%nonassoc IFX
%nonassoc ELSE

%left GE LE EQ NE '>' '<'
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS

%type <nPtr> stmt stmt2 expr expr2 stmt_list value

%%

program:
        function                { exit(0); }
        ;

function:
          function stmt         { ex($2); /*freeNode($2);*/ }
        | /* NULL */
        ;

stmt2:
          expr                                    { $$ = $1; }
        | PRINT expr                              { $$ = new Ast::FunctionCall("print", $2); }
        |                                         { $$ = new Ast::Value(true); }
        ;

stmt:
          stmt2 ';'                                   { $$ = $1; }
        | WHILE '(' expr ')' stmt                     { $$ = new Ast::While($3, $5); }
        | IF '(' expr ')' stmt %prec IFX              { $$ = new Ast::If($3, $5, nullptr); }
        | IF '(' expr ')' stmt ELSE stmt              { $$ = new Ast::If($3, $5, $7); }
        | FOR '(' stmt2 ';' stmt2 ';' stmt2 ')' stmt  { $$ = new Ast::For($3, $5, $7, $9); }
        | '{' stmt_list '}'                           { $$ = $2; }
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
          expr2                  { $$ = $1; }
        | VARIABLE '=' expr2     { $$ = new Ast::Assignment(new Ast::Variable($1), $3); }
        ;


expr2:
          value                   { $$ = $1; }
        | VARIABLE                { $$ = new Ast::Variable($1); }
        | '-' expr2 %prec UMINUS  { $$ = new Ast::UnaryOperator(Ast::UnaryOperator::Minus, $2); }
        | expr2 '+' expr2         { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::Plus, $1, $3); }
        | expr2 '-' expr2         { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::Minus, $1, $3); }
        | expr2 '*' expr2         { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::Times, $1, $3); }
        | expr2 '/' expr2         { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::Div, $1, $3); }
        | expr2 '<' expr2         { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::LessThan, $1, $3); }
        | expr2 '>' expr2         { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::GreaterThan, $1, $3); }
        | expr2 GE expr2          { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::GreaterThanEqual, $1, $3); }
        | expr2 LE expr2          { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::LessThanEqual, $1, $3); }
        | expr2 NE expr2          { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::NotEqual, $1, $3); }
        | expr2 EQ expr2          { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::Equal, $1, $3); }
        | '(' expr ')'            { $$ = $2; }
        ;

value:  
          INTEGER                 { $$ = new Ast::Value($1); }
        | TRUE                    { $$ = new Ast::Value(true); }
        | FALSE                   { $$ = new Ast::Value(false); }
        ;
%%


void yyerror(const char *s) {
    fprintf(stdout, "%s\n", s);
}
