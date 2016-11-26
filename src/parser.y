%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "ast.h"
#include "environment.h"
#include <iostream>


/* prototypes */

/* int ex(nodeType *p); */
void eval(Ast::Node *p);

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
%token FOR WHILE IF PRINT TRUE FALSE FUNCTION
%nonassoc IFX
%nonassoc ELSE

%left GE LE EQ NE '>' '<'
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS

%type <nPtr> stmt stmt2 expr expr2 stmt_list value fundecl fun_list fun_list2 expr_list variable

%%

program:
        function                { exit(0); }
        ;

function:
          function stmt         { eval($2); /*freeNode($2);*/ }
        | /* NULL */
        ;

stmt2:
          expr                                    { $$ = $1; }
        | PRINT expr                              { $$ = new Ast::FunctionCall("print", $2); }
        | VARIABLE '(' ')'                        { $$ = new Ast::FunctionCall($1); }
        | VARIABLE '(' expr_list ')'              { $$ = new Ast::FunctionCall($1, $3->as<Ast::ExpressionList*>()); }
        |                                         { $$ = new Ast::BoolLiteral(true); }
        ;
        
expr_list:
          expr                                        { $$ = new Ast::ExpressionList($1); }
        | expr_list ',' expr                          { $$ = new Ast::ExpressionList($3, $1->as<Ast::ExpressionList*>()); }
        ;

stmt:
          stmt2 ';'                                   { $$ = $1; }
        | fundecl                                     { $$ = $1; }
        | WHILE '(' expr ')' stmt                     { $$ = new Ast::While($3, $5); }
        | IF '(' expr ')' stmt %prec IFX              { $$ = new Ast::If($3, $5, nullptr); }
        | IF '(' expr ')' stmt ELSE stmt              { $$ = new Ast::If($3, $5, $7); }
        | FOR '(' stmt2 ';' stmt2 ';' stmt2 ')' stmt  { $$ = new Ast::For($3, $5, $7, $9); }
        | '{' stmt_list '}'                           { $$ = $2; }
        ;

fundecl:
        FUNCTION VARIABLE '(' fun_list ')' '{' stmt_list '}'   { $$ = new Ast::Function($2, $4->as<Ast::VariableList*>(), $7->as<Ast::StatementList*>());}
        ;

fun_list:
          fun_list2               { $$ = $1; }
        |                         { $$ = new Ast::VariableList(); }
        ;
        
variable:
          VARIABLE                { $$ = new Ast::Variable($1);}
        ;

fun_list2:
          variable                 { $$ = new Ast::VariableList($1->as<Ast::Variable*>()); }
        | fun_list2 ',' variable   { $$ = new Ast::VariableList($3->as<Ast::Variable*>(), $1->as<Ast::VariableList*>()); }
        ;
        
stmt_list:
          stmt                    { $$ = new Ast::StatementList($1); }
        | stmt_list stmt          { $$ = new Ast::StatementList($2->as<Ast::Statement*>(), $1->as<Ast::StatementList*>()); }
        ;

expr:
          expr2                   { $$ = $1; }
        | variable '=' expr2      { $$ = new Ast::Assignment($1->as<Ast::Variable*>(), $3); }
        ;

expr2:
          value                   { $$ = $1; }
        | variable                { $$ = $1->as<Ast::Variable*>(); }
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
          INTEGER                 { $$ = new Ast::IntegerLiteral($1); }
        | TRUE                    { $$ = new Ast::BoolLiteral(true); }
        | FALSE                   { $$ = new Ast::BoolLiteral(false); }
        ;
%%


void yyerror(const char *s) {
    fprintf(stdout, "%s\n", s);
}
