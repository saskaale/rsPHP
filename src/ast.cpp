#include "ast.h"
#include "common.h"
#include "aval.h"

#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cstdlib>

namespace Ast
{

Node::Node()
{
}

Node::~Node()
{
}

const char* Node::typeStr() const
{
    static const char* const tNames[] = {
        "VariableT", "ArraySubscriptT", "IntegerLiteralT", "DoubleLiteralT", "BoolLiteralT", "UndefinedLiteralT", "StringLiteralT", "AValLiteralT",
        "UnaryOperatorT", "BinaryOperatorT", "FunctionCallT", "ExpressionListT", "AssignmentT", "TryT",
        "IfT", "WhileT", "ForT", "ReturnT", "BreakT", "ContinueT",
        "StatementListT", "VariableListT", "FunctionT"
    };

    return tNames[(int)this->type()];
}




Variable::Variable(const std::string &name, bool ref)
    : name(name)
    , ref(ref)
{
}

Node::Type Variable::type() const
{
    return VariableT;
}


ArraySubscript::ArraySubscript(const std::string &name, Expression *expr)
    : Variable(name)
    , expression(expr)
{
}

ArraySubscript::~ArraySubscript()
{
    delete expression;
}

Node::Type ArraySubscript::type() const
{
    return ArraySubscriptT;
}


IntegerLiteral::IntegerLiteral(int value)
    : value(value)
{
}

Node::Type IntegerLiteral::type() const
{
    return IntegerLiteralT;
}

AValLiteral::AValLiteral(const void* value)
    : value(value)
{
}

Node::Type AValLiteral::type() const
{
    return AValLiteralT;
}


DoubleLiteral::DoubleLiteral(double value)
    : value(value)
{
}

Node::Type DoubleLiteral::type() const
{
    return DoubleLiteralT;
}


BoolLiteral::BoolLiteral(bool value)
    : value(value)
{
}

Node::Type BoolLiteral::type() const
{
    return BoolLiteralT;
}


UndefinedLiteral::UndefinedLiteral()
{
}

Node::Type UndefinedLiteral::type() const
{
    return UndefinedLiteralT;
}


StringLiteral::StringLiteral(const std::string &value)
    : value(value)
{
}

Node::Type StringLiteral::type() const
{
    return StringLiteralT;
}


UnaryOperator::UnaryOperator(Op op, Expression *expr)
    : op(op)
    , expr(expr)
{
}

UnaryOperator::~UnaryOperator()
{
    delete expr;
}

Node::Type UnaryOperator::type() const
{
    return UnaryOperatorT;
}


BinaryOperator::BinaryOperator(Op op, Expression *left, Expression *right)
    : op(op)
    , left(left)
    , right(right)
{
}

BinaryOperator::~BinaryOperator()
{
    delete left;
    delete right;
}

Node::Type BinaryOperator::type() const
{
    return BinaryOperatorT;
}

const char* BinaryOperator::opStr() const
{
    static const char* const opNames[] = {
        "Plus", "Minus", "Times", "Equal", "EqualType", "NotEqual", "NotEqualType",
        "LessThan", "GreaterThan", "LessThanEqual", "GreaterThanEqual",
        "Div", "Mod", "And", "Or"
    };

    return opNames[(int)this->op];
}



FunctionCall::FunctionCall(Expression* function, Expression *args)
    : function(function)
{
    if (!args) {
        arguments = new ExpressionList();
    } else if (ExpressionList *lst = args->as<ExpressionList*>()) {
        arguments = lst;
    } else {
        arguments = new ExpressionList(args);
    }
}

FunctionCall::~FunctionCall()
{
    delete function;
    delete arguments;
}

Node::Type FunctionCall::type() const
{
    return FunctionCallT;
}


ExpressionList::ExpressionList()
{
}

ExpressionList::ExpressionList(const std::vector<Expression*>& expressions)
  : expressions(expressions)
{
}

ExpressionList::ExpressionList(Expression *expr, ExpressionList *lst)
{
    if (lst) {
        expressions = lst->expressions;
        lst->expressions.clear();
        delete lst;
    }
    expressions.push_back(expr);
}

ExpressionList::~ExpressionList()
{
    for (Expression *e : expressions) {
        if (e->type() == FunctionT) {
            continue;
        }
        delete e;
    }
}

Node::Type ExpressionList::type() const
{
    return ExpressionListT;
}


Try::Try(StatementList *body, VariableList* variables, StatementList *catchPart)
    : body(body)
    , variables(variables)
    , catchPart(catchPart)
{
}

Try::~Try()
{
    delete body;
    delete variables;
    delete catchPart;
}

Node::Type Try::type() const
{
    return TryT;
}



Assignment::Assignment(Variable *var, Expression *expr)
    : variable(var)
    , expression(expr)
{
}

Assignment::~Assignment()
{
    delete variable;
    delete expression;
}

Node::Type Assignment::type() const
{
    return AssignmentT;
}


If::If(Expression *cond, Statement *thenStm, Statement *elseStm)
    : condition(cond)
{
    if (StatementList *lst = thenStm->as<StatementList*>()) {
        thenStatement = lst;
    } else {
        thenStatement = new StatementList(thenStm);
    }

    if (StatementList *lst = elseStm->as<StatementList*>()) {
        elseStatement = lst;
    } else {
        elseStatement = new StatementList(elseStm);
    }
}

If::~If()
{
    delete condition;
    delete thenStatement;
    delete elseStatement;
}

Node::Type If::type() const
{
    return IfT;
}


While::While(Expression *cond, Statement *stm)
    : condition(cond)
{
    if (StatementList *lst = stm->as<StatementList*>()) {
        statement = lst;
    } else {
        statement = new StatementList(stm);
    }
}

While::~While()
{
    delete condition;
    delete statement;
}

Node::Type While::type() const
{
    return WhileT;
}


For::For(Expression *init, Expression *cond, Expression *after, Statement *stm)
    : init(init)
    , cond(cond)
    , after(after)
{
    if (StatementList *lst = stm->as<StatementList*>()) {
        statement = lst;
    } else {
        statement = new StatementList(stm);
    }
}

For::~For()
{
    delete init;
    delete cond;
    delete after;
    delete statement;
}

Node::Type For::type() const
{
    return ForT;
}


Return::Return(Expression *expr)
    : expression(expr)
{
}

Return::~Return()
{
    delete expression;
}

Node::Type Return::type() const
{
    return ReturnT;
}


Break::Break()
{
}

Node::Type Break::type() const
{
    return BreakT;
}


Continue::Continue()
{
}

Node::Type Continue::type() const
{
    return ContinueT;
}


StatementList::StatementList(Statement *stm, StatementList *lst)
{
    if (lst) {
        statements = lst->statements;
        lst->statements.clear();
        delete lst;
    }
    if (stm) {
        statements.push_back(stm);
    }
}

StatementList::~StatementList()
{
    for (Statement *s : statements) {
        if (Assignment *a = s->as<Assignment*>()) {
            if (a->expression->type() == FunctionT) {
                a->expression = nullptr;
            }
        }
        delete s;
    }
}

Node::Type StatementList::type() const
{
    return StatementListT;
}


VariableList::VariableList(Variable *var, VariableList *lst)
{
    if (lst) {
        variables = lst->variables;
        lst->variables.clear();
        delete lst;
    }
    if (var) {
        variables.push_back(var);
    }
}

VariableList::~VariableList()
{
    for (Variable *v : variables) {
        delete v;
    }
}

Node::Type VariableList::type() const
{
    return VariableListT;
}


Function::Function(VariableList *params, StatementList *stm)
    : parameters(params)
    , statements(stm)
{
}

Function::Function(const std::string &name, VariableList *params, StatementList *stm)
    : name(name)
    , parameters(params)
    , statements(stm)
{
}

Function::~Function()
{
    delete parameters;
    delete statements;
}

Node::Type Function::type() const
{
    return FunctionT;
}

bool Function::isLambda() const
{
    return name.empty();
}

} // namespace Ast
