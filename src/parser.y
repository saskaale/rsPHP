%{
#include "ast.h"
#include "evaluator.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

extern "C" FILE *yyin;

int yylex(void);
void yyerror(const char *s);

static Ast::Node *create_assign(Ast::BinaryOperator::Op op, Ast::Expression *dst, Ast::Expression *right)
{
    Ast::Variable *var = dst->as<Ast::Variable*>();
    if (!var) {
        fprintf(stderr, "AssignOp only implemented for variables!\n");
        abort();
    }
    Ast::BinaryOperator *o = new Ast::BinaryOperator(op, new Ast::Variable(var->name), right);
    return new Ast::Assignment(dst, o);
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
%token FOR WHILE IF PRINT THROW TRUE FALSE UNDEFINED FUNCTION RETURN BREAK CONTINUE TRY CATCH
%token ASSIGN AS_PLUS AS_MINUS AS_TIMES AS_DIV AS_MOD REFERENCE
%nonassoc IFX
%nonassoc ELSE

%left GE LE EQ EQ_TYPE NE NE_TYPE GREATER LESS
%left PLUS MINUS NOT
%left TIMES DIV MOD
%left INCREMENT DECREMENT
%left AND OR
%nonassoc UMINUS

%type <nPtr> stmt stmt2 expr expr2 stmt_list empty_stmt_list value fundecl var_list var_list2 expr_list variable lambda

%%

program:
        function                { }
        ;

function:
          function stmt         { Evaluator::eval($2); Evaluator::cleanup($2); }
        | /* NULL */
        ;

stmt2:
          expr                                    { $$ = $1; }
        | PRINT expr                              { $$ = new Ast::FunctionCall(new Ast::Variable("print"), $2); }
        | THROW expr                              { $$ = new Ast::FunctionCall(new Ast::Variable("throw"), $2); }
        | RETURN expr                             { $$ = new Ast::Return($2); }
        | BREAK                                   { $$ = new Ast::Break(); }
        | CONTINUE                                { $$ = new Ast::Continue(); }
        |                                         { $$ = new Ast::BoolLiteral(true); }
        ;

expr_list:
          expr                                        { $$ = new Ast::ExpressionList($1); }
        | expr_list ',' expr                          { $$ = new Ast::ExpressionList($3, $1->as<Ast::ExpressionList*>()); }
        ;

stmt:
          stmt2 ';'                                               { $$ = $1; }
        | fundecl                                                 { $$ = $1; }
        | TRY '{' stmt_list '}' CATCH '(' var_list ')' '{' empty_stmt_list '}'   { $$ = new Ast::Try($3->as<Ast::StatementList*>(), $7->as<Ast::VariableList*>(), $10->as<Ast::StatementList*>()); }
        | WHILE '(' expr ')' stmt                                 { $$ = new Ast::While($3, $5); }
        | IF '(' expr ')' stmt %prec IFX                          { $$ = new Ast::If($3, $5, nullptr); }
        | IF '(' expr ')' stmt ELSE stmt                          { $$ = new Ast::If($3, $5, $7); }
        | FOR '(' stmt2 ';' stmt2 ';' stmt2 ')' stmt              { $$ = new Ast::For($3, $5, $7, $9); }
        | '{' stmt_list '}'                                       { $$ = $2; }
        ;

fundecl:
        FUNCTION VARIABLE '(' var_list ')' '{' stmt_list '}'   { $$ = new Ast::Function($2, $4->as<Ast::VariableList*>(), $7->as<Ast::StatementList*>()); free($2); }
        ;

var_list:
          var_list2               { $$ = $1; }
        |                         { $$ = new Ast::VariableList(); }
        ;

variable:
          VARIABLE                { $$ = new Ast::Variable($1); free($1); }
        | REFERENCE VARIABLE      { $$ = new Ast::Variable($2, true); free($2); }
        ;

var_list2:
          variable                 { $$ = new Ast::VariableList($1->as<Ast::Variable*>()); }
        | var_list2 ',' variable   { $$ = new Ast::VariableList($3->as<Ast::Variable*>(), $1->as<Ast::VariableList*>()); }
        ;

empty_stmt_list:
          stmt_list               { $$ = $1; }
        |                         { $$ = new Ast::StatementList(); }
        ;

stmt_list:
          stmt                    { $$ = new Ast::StatementList($1); }
        | stmt_list stmt          { $$ = new Ast::StatementList($2->as<Ast::Statement*>(), $1->as<Ast::StatementList*>()); }
        ;

expr:
          expr2                    { $$ = $1; }
        | expr2 ASSIGN expr        { $$ = new Ast::Assignment($1, $3); }
        | expr2 AS_PLUS expr       { $$ = create_assign(Ast::BinaryOperator::Plus, $1, $3); }
        | expr2 AS_MINUS expr      { $$ = create_assign(Ast::BinaryOperator::Minus, $1, $3); }
        | expr2 AS_TIMES expr      { $$ = create_assign(Ast::BinaryOperator::Times, $1, $3); }
        | expr2 AS_DIV expr        { $$ = create_assign(Ast::BinaryOperator::Div, $1, $3); }
        | expr2 AS_MOD expr        { $$ = create_assign(Ast::BinaryOperator::Mod, $1, $3); }
        ;

expr2:
          value                       { $$ = $1; }
        | variable                    { $$ = $1->as<Ast::Variable*>(); }
        | VARIABLE '(' ')'            { $$ = new Ast::FunctionCall(new Ast::Variable($1)); free($1); }
        | VARIABLE '(' expr_list ')'  { $$ = new Ast::FunctionCall(new Ast::Variable($1), $3->as<Ast::ExpressionList*>()); free($1); }
        | MINUS expr2 %prec UMINUS    { $$ = new Ast::UnaryOperator(Ast::UnaryOperator::Minus, $2); }
        | NOT expr2                   { $$ = new Ast::UnaryOperator(Ast::UnaryOperator::Not, $2); }
        | INCREMENT expr2             { $$ = new Ast::UnaryOperator(Ast::UnaryOperator::PreIncrement, $2); }
        | DECREMENT expr2             { $$ = new Ast::UnaryOperator(Ast::UnaryOperator::PreDecrement, $2); }
        | expr2 DECREMENT             { $$ = new Ast::UnaryOperator(Ast::UnaryOperator::PostDecrement, $1); }
        | expr2 INCREMENT             { $$ = new Ast::UnaryOperator(Ast::UnaryOperator::PostIncrement, $1); }
        | expr2 PLUS expr2            { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::Plus, $1, $3); }
        | expr2 MINUS expr2           { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::Minus, $1, $3); }
        | expr2 TIMES expr2           { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::Times, $1, $3); }
        | expr2 DIV expr2             { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::Div, $1, $3); }
        | expr2 MOD expr2             { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::Mod, $1, $3); }
        | expr2 LESS expr2            { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::LessThan, $1, $3); }
        | expr2 GREATER expr2         { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::GreaterThan, $1, $3); }
        | expr2 GE expr2              { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::GreaterThanEqual, $1, $3); }
        | expr2 LE expr2              { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::LessThanEqual, $1, $3); }
        | expr2 NE expr2              { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::NotEqual, $1, $3); }
        | expr2 EQ expr2              { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::Equal, $1, $3); }
        | expr2 NE_TYPE expr2         { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::NotEqualType, $1, $3); }
        | expr2 EQ_TYPE expr2         { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::EqualType, $1, $3); }
        | expr2 AND expr2             { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::And, $1, $3); }
        | expr2 OR expr2              { $$ = new Ast::BinaryOperator(Ast::BinaryOperator::Or, $1, $3); }
        | expr2 '[' expr2 ']'         { $$ = new Ast::ArraySubscript($1, $3); }
        | '(' expr ')'                { $$ = $2; }
        | lambda                      { $$ = $1; }
        ;

lambda:
        FUNCTION '(' var_list ')' '{' stmt_list '}'   { $$ = new Ast::Function($3->as<Ast::VariableList*>(), $6->as<Ast::StatementList*>()); }

value:
          INTEGER                 { $$ = new Ast::IntegerLiteral($1); }
        | DOUBLE                  { $$ = new Ast::DoubleLiteral($1); }
        | TRUE                    { $$ = new Ast::BoolLiteral(true); }
        | FALSE                   { $$ = new Ast::BoolLiteral(false); }
        | UNDEFINED               { $$ = new Ast::UndefinedLiteral(); }
        | STRING                  { $$ = new Ast::StringLiteral($1); free($1); }
        ;
%%

void yyerror(const char *s)
{
    fprintf(stdout, "%s\n", s);
}

extern void yyrestart(FILE *f);

namespace Parser
{

void parseFile(FILE *file)
{
    yyrestart(file);
    yyin = file;
    yyparse();
}

void parseString(const char *str)
{
    FILE *file = fmemopen((char*)str, strlen(str), "r");
    parseFile(file);
    fclose(yyin);
}

} // namespace Parser
