#include "ast.h"
#include "common.h"
#include "aval.h"

#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cstdlib>
#include <unordered_set>

namespace Ast
{

std::unordered_set<Function*> funcs;

void cleanup()
{
    for (Function *f : funcs) {
        delete f;
    }
}

void del(Node *n)
{
    if (!n || funcs.find(static_cast<Function*>(n)) != funcs.end()) {
        return;
    }
    if (n->type() != Node::FunctionT) {
        delete n;
    }
}

Node::Node(Node* n1, Node* n2, Node* n3, Node* n4)
  : n1(n1)
  , n2(n2)
  , n3(n3)
  , n4(n4)
{
}

Node::~Node()
{
}

const char* Node::typeStr() const
{
    static const char* const tNames[] = {
        "VariableT", "ArraySubscriptT", "IntegerLiteralT", "DoubleLiteralT", "BoolLiteralT", "CharLiteralT", "UndefinedLiteralT", "AValLiteralT", "ConstantLiteralT",
        "UnaryOperatorT", "BinaryOperatorT", "FunctionCallT", "ExpressionListT", "AssignmentT", "TryT",
        "IfT", "WhileT", "ForT", "ReturnT", "BreakT", "ContinueT",
        "StatementListT", "VariableListT", "FunctionT"
    };

    return tNames[(int)this->type()];
}





Variable::Variable(const std::string &name, bool ref, bool cnst)
    : ref(ref)
    , isconst(cnst)
    , name(name)
{
}

Node::Type Variable::type() const
{
    return VariableT;
}


ArraySubscript::ArraySubscript(Expression *src, Expression *expr)
    : Node(src, expr)
{
}

ArraySubscript::~ArraySubscript()
{
    del(source());
    del(expression());
}

Node::Type ArraySubscript::type() const
{
    return ArraySubscriptT;
}

Expression *ArraySubscript::source() const
{
    return (Expression*)n1;
}

Expression *ArraySubscript::expression() const
{
    return (Expression*)n2;
}


IntegerLiteral::IntegerLiteral(int value)
    : value(value)
{
}

Node::Type IntegerLiteral::type() const
{
    return IntegerLiteralT;
}

AValLiteral::AValLiteral(const AVal* value)
{
      aval1 = value;
}

Node::Type AValLiteral::type() const
{
    return AValLiteralT;
}

const AVal* AValLiteral::value() const
{
    return aval1;
}


ConstantLiteral::ConstantLiteral(const AVal& value)
{
    aval1 = new AVal(value);
}

ConstantLiteral::~ConstantLiteral()
{
    delete aval1;
}

Node::Type ConstantLiteral::type() const
{
    return ConstantLiteralT;
}

const AVal& ConstantLiteral::value() const
{
    return *aval1;
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


CharLiteral::CharLiteral(char value)
    : value(value)
{
}

Node::Type CharLiteral::type() const
{
    return CharLiteralT;
}


UndefinedLiteral::UndefinedLiteral()
{
}

Node::Type UndefinedLiteral::type() const
{
    return UndefinedLiteralT;
}


UnaryOperator::UnaryOperator(Op op, Expression *expr)
    : Expression(expr)
    , op(op)
{
}

UnaryOperator::~UnaryOperator()
{
    del(expr());
}

Node::Type UnaryOperator::type() const
{
    return UnaryOperatorT;
}

Expression* UnaryOperator::expr() const
{
    return (Expression*)n1;
}



BinaryOperator::BinaryOperator(Op op, Expression *left, Expression *right)
    : Node(left, right)
    , op(op)
{
}

BinaryOperator::~BinaryOperator()
{
    del(left());
    del(right());
}

Node::Type BinaryOperator::type() const
{
    return BinaryOperatorT;
}

Expression* BinaryOperator::left() const
{
    return (Expression*)n1;
}

Expression* BinaryOperator::right() const
{
    return (Expression*)n2;
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



FunctionCall::FunctionCall(Expression* function, Expression *args, Expression *object)
    : Expression(function)
{
    Node*& arguments = n2;
    if (!args) {
        arguments = new ExpressionList();
    } else if (ExpressionList *lst = args->as<ExpressionList*>()) {
        arguments = lst;
    } else {
        arguments = new ExpressionList(args);
    }
    n3 = object;
}

FunctionCall::~FunctionCall()
{
    del(object());
    del(function());
    del(arguments());
}

Node::Type FunctionCall::type() const
{
    return FunctionCallT;
}

Expression* FunctionCall::object() const
{
    return n3;
}

Expression* FunctionCall::function() const
{
    return n1;
}

ExpressionList* FunctionCall::arguments() const
{
    return (ExpressionList*)n2;
}


ExpressionList::ExpressionList()
{
}

ExpressionList::ExpressionList(const std::vector<Expression*>& expressions)
{
    exprVec1 = expressions;
}

ExpressionList::ExpressionList(Expression *expr, ExpressionList *lst)
{
    if (lst) {
        exprVec1 = lst->exprVec1;
        lst->exprVec1.clear();
        del(lst);
    }
    exprVec1.push_back(expr);
}

ExpressionList::~ExpressionList()
{
    for (Expression *e : exprVec1) {
        del(e);
    }
}

Node::Type ExpressionList::type() const
{
    return ExpressionListT;
}

const std::vector<Expression*>& ExpressionList::expressions() const
{
    return exprVec1;
}



Try::Try(StatementList *body, VariableList* variables, StatementList *catchPart)
    : Node(body, variables, catchPart)
{
}

Try::~Try()
{
    del(body());
    del(variables());
    del(catchPart());
}

Node::Type Try::type() const
{
    return TryT;
}


StatementList* Try::body() const
{
    return (StatementList*)n1;
}

VariableList* Try::variables() const
{
    return (VariableList*)n2;
}

StatementList* Try::catchPart() const
{
    return (StatementList*)n3;
}



Assignment::Assignment(Expression *dst, Expression *expr)
    : Node(dst, expr)
{
}

Assignment::~Assignment()
{
    del(destination());
    del(expression());
}

Expression *Assignment::destination() const
{
    return (Expression*)n1;
}

Expression *Assignment::expression() const
{
    return (Expression*)n2;
}

void Assignment::nullExpression()
{
    n2 = nullptr;
}



Node::Type Assignment::type() const
{
    return AssignmentT;
}


If::If(Expression *cond, Statement *thenStm, Statement *elseStm)
    : Node(cond)
{
    Node*& thenStatement = n2;
    Node*& elseStatement = n3;
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
    del(condition());
    del(thenStatement());
    del(elseStatement());
}

Node::Type If::type() const
{
    return IfT;
}

Expression* If::condition() const
{
    return (Expression*)n1;
}

StatementList* If::thenStatement() const
{
    return (StatementList*)n2;
}

StatementList* If::elseStatement() const
{
    return (StatementList*)n3;
}



While::While(Expression *cond, Statement *stm)
    : Node(cond)
{
    Node*& statement = n2;
    if (StatementList *lst = stm->as<StatementList*>()) {
        statement = lst;
    } else {
        statement = new StatementList(stm);
    }
}

While::~While()
{
    del(condition());
    del(statement());
}

Node::Type While::type() const
{
    return WhileT;
}

Expression* While::condition() const
{
    return (Expression*)n1;
}

StatementList* While::statement() const
{
    return (StatementList*)n2;
}


For::For(Expression *init, Expression *cond, Expression *after, Statement *stm)
    : Node(init, cond, after)
{
    Node*& statement = n4;
    if (StatementList *lst = stm->as<StatementList*>()) {
        statement = lst;
    } else {
        statement = new StatementList(stm);
    }
}

For::~For()
{
    del(init());
    del(cond());
    del(after());
    del(statement());
}

Node::Type For::type() const
{
    return ForT;
}

Expression *For::init() const
{
    return (Expression*)n1;
}

Expression *For::cond() const
{
    return (Expression*)n2;
}

Expression *For::after() const
{
    return (Expression*)n3;
}

StatementList *For::statement() const
{
    return (StatementList*)n4;
}




Return::Return(Expression *expr)
    : Statement(expr)
{
}

Return::~Return()
{
    del(expression());
}

Node::Type Return::type() const
{
    return ReturnT;
}

Expression* Return::expression() const
{
    return (Expression*)n1;
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
        del(lst);
    }
    if (stm) {
        statements.push_back(stm);
    }
}

StatementList::~StatementList()
{
    for (Statement *s : statements) {
        del(s);
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
        del(lst);
    }
    if (var) {
        variables.push_back(var);
    }
}

VariableList::~VariableList()
{
    for (Variable *v : variables) {
        del(v);
    }
}

Node::Type VariableList::type() const
{
    return VariableListT;
}


Function::Function(VariableList *params, StatementList *stm)
    : Node(params, stm)
{
    funcs.insert(this);
}

Function::Function(const std::string &name, VariableList *params, StatementList *stm)
    : Node(params, stm)
    , name(name)
{
    funcs.insert(this);
}

Function::~Function()
{
    del(parameters());
    del(statements());
}

Node::Type Function::type() const
{
    return FunctionT;
}

bool Function::isLambda() const
{
    return name.empty();
}

VariableList *Function::parameters() const
{
    return (VariableList*)n1;
}

StatementList *Function::statements() const
{
    return (StatementList*)n2;
}

} // namespace Ast
