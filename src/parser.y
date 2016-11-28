%{
#include "ast.h"
#include "evaluator.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int yylex(void);
void yyerror(const char *s);

static Ast::Node *create_assign(Ast::BinaryOperator::Op op, Ast::Variable *var, Ast::Expression *right)
{
    Ast::BinaryOperator *o = new Ast::BinaryOperator(op, new Ast::Variable(var->name), right);
    return new Ast::Assignment(var, o);
}

%}

%union {
    double fValue;
    int iValue;
    char *str;
    Ast::Node *nPtr;
};

%token <fValue> DOUBLE
%token <iValue> INTEGER
%token <str> VARIABLE
%token <str> STRING
%token FOR WHILE IF PRINT TRUE FALSE FUNCTION RETURN BREAK CONTINUE
%token ASSIGN AS_PLUS AS_MINUS AS_TIMES AS_DIV AS_MOD
%nonassoc IFX
%nonassoc ELSE

%left GE LE EQ NE GREATER LESS
%left PLUS MINUS
%left TIMES DIV MOD
%left INCREMENT DECREMENT
%nonassoc UMINUS
%nonassoc UPREDECRE

%type <nPtr> stmt stmt2 expr expr2 stmt_list value fundecl fun_list fun_list2 expr_list variable

%%

program:
        function                { Evaluator::exit(); }
        ;

function:
          function stmt         { Evaluator::eval($2); Evaluator::cleanup($2); }
        | /* NULL */
        ;

stmt2:
          expr                                    { $$ = $1; }
        | PRINT expr                              { $$ = new Ast::FunctionCall("print", $2); }
        | RETURN                                  { $$ = new Ast::Return(); }
        | BREAK                                   { $$ = new Ast::Break(); }
        | CONTINUE                                { $$ = new Ast::Continue(); }
        | VARIABLE '(' ')'                        { $$ = new Ast::FunctionCall($1); free($1); }
        | VARIABLE '(' expr_list ')'              { $$ = new Ast::FunctionCall($1, $3->as<Ast::ExpressionList*>()); free($1); }
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
        FUNCTION VARIABLE '(' fun_list ')' '{' stmt_list '}'   { $$ = new Ast::Function($2, $4->as<Ast::VariableList*>(), $7->as<Ast::StatementList*>()); free($2); }
        ;

fun_list:
          fun_list2               { $$ = $1; }
        |                         { $$ = new Ast::VariableList(); }
        ;

variable:
          VARIABLE                { $$ = new Ast::Variable($1); free($1); }
        | VARIABLE '[' expr ']'   { $$ = new Ast::ArraySubscript($1, $3); free($1); }
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
          expr2                    { $$ = $1; }
        | variable ASSIGN expr2    { $$ = new Ast::Assignment($1->as<Ast::Variable*>(), $3); }
        | variable AS_PLUS expr2   { $$ = create_assign(Ast::BinaryOperator::Plus, $1->as<Ast::Variable*>(), $3); }
        | variable AS_MINUS expr2  { $$ = create_assign(Ast::BinaryOperator::Minus, $1->as<Ast::Variable*>(), $3); }
        | variable AS_TIMES expr2  { $$ = create_assign(Ast::BinaryOperator::Times, $1->as<Ast::Variable*>(), $3); }
        | variable AS_DIV expr2    { $$ = create_assign(Ast::BinaryOperator::Div, $1->as<Ast::Variable*>(), $3); }
        | variable AS_MOD expr2    { $$ = create_assign(Ast::BinaryOperator::Mod, $1->as<Ast::Variable*>(), $3); }
        ;

expr2:
          value                    { $$ = $1; }
        | variable                 { $$ = $1->as<Ast::Variable*>(); }
        | MINUS expr2 %prec UMINUS { $$ = new Ast::UnaryOperator(Ast::UnaryOperator::Minus, $2); }
        | INCREMENT expr2          { $$ = new Ast::UnaryOperator(Ast::UnaryOperator::PreIncrement, $2); }
        | DECREMENT expr2          { $$ = new Ast::UnaryOperator(Ast::UnaryOperator::PreDecrement, $2); }
        | expr2 DECREMENT          { $$ = new Ast::UnaryOperator(Ast::UnaryOperator::PostDecrement, $1); }
        | expr2 INCREMENT          { $$ = new Ast::UnaryOperator(Ast::UnaryOperator::PostIncrement, $1); }
        | expr2 PLUS expr2         { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::Plus, $1, $3); }
        | expr2 MINUS expr2        { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::Minus, $1, $3); }
        | expr2 TIMES expr2        { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::Times, $1, $3); }
        | expr2 DIV expr2          { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::Div, $1, $3); }
        | expr2 MOD expr2          { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::Mod, $1, $3); }
        | expr2 LESS expr2         { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::LessThan, $1, $3); }
        | expr2 GREATER expr2      { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::GreaterThan, $1, $3); }
        | expr2 GE expr2           { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::GreaterThanEqual, $1, $3); }
        | expr2 LE expr2           { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::LessThanEqual, $1, $3); }
        | expr2 NE expr2           { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::NotEqual, $1, $3); }
        | expr2 EQ expr2           { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::Equal, $1, $3); }
        | '(' expr ')'             { $$ = $2; }
        ;

value:
          INTEGER                 { $$ = new Ast::IntegerLiteral($1); }
        | DOUBLE                  { $$ = new Ast::DoubleLiteral($1); }
        | TRUE                    { $$ = new Ast::BoolLiteral(true); }
        | FALSE                   { $$ = new Ast::BoolLiteral(false); }
        | STRING                  { $$ = new Ast::StringLiteral($1); free($1); }
        ;
%%


void yyerror(const char *s) {
    fprintf(stdout, "%s\n", s);
}
