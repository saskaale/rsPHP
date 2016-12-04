#pragma once

#include <vector>
#include <string>

namespace Ast
{

class Node;
class Expression;
class Statement;

// Expressions
class Variable;
class Array;
class ArraySubscript;
class IntegerLiteral;
class DoubleLiteral;
class BoolLiteral;
class UndefinedLiteral;
class StringLiteral;
class BinaryOperator;
class FunctionCall;
class ExpressionList;

// Statements
class Assignment;
class If;
class While;
class For;
class Return;
class Break;
class Continue;
class StatementList;

// Functions
class VariableList;
class Function;

// Program
class Program;

class Node
{
public:
    enum Type {
        VariableT, ArraySubscriptT, IntegerLiteralT, DoubleLiteralT, BoolLiteralT, UndefinedLiteralT, StringLiteralT, AValLiteralT,
        UnaryOperatorT, BinaryOperatorT, FunctionCallT, ExpressionListT, AssignmentT, TryT,
        IfT, WhileT, ForT, ReturnT, BreakT, ContinueT,
        StatementListT, VariableListT, FunctionT
    };

    explicit Node();
    virtual ~Node();

    virtual Type type() const = 0;
    const char* typeStr() const;

    template<typename T> T as() { return dynamic_cast<T>(this); }
    template<typename T> T as() const { return dynamic_cast<T>(this); }
};

#if 1
#define Expression Node
#define Statement  Node
#else
class Expression : public Node
{
};

class Statement : public Node
{
};
#endif




// public Expression
class Variable : public Expression
{
public:
    explicit Variable(const std::string &name, bool ref = false);

    Type type() const;

    std::string name;
    bool ref;
};

class ArraySubscript : public Variable
{
public:
    explicit ArraySubscript(const std::string &name, Expression *expr);
    ~ArraySubscript();

    Type type() const;

    Expression *expression;
};

class AValLiteral : public Expression
{
public:
    explicit AValLiteral(const void* value);

    Type type() const;

    const void* value;
};


class IntegerLiteral : public Expression
{
public:
    explicit IntegerLiteral(int value);

    Type type() const;

    int value;
};

class DoubleLiteral : public Expression
{
public:
    explicit DoubleLiteral(double value);

    Type type() const;

    double value;
};

class BoolLiteral : public Expression
{
public:
    explicit BoolLiteral(bool value);

    Type type() const;

    bool value;
};

class UndefinedLiteral : public Expression
{
public:
    explicit UndefinedLiteral();

    Type type() const;
};

class StringLiteral : public Expression
{
public:
    explicit StringLiteral(const std::string &value);

    Type type() const;

    std::string value;
};

class UnaryOperator : public Expression
{
public:
    enum Op {
        Not, Minus, PreIncrement, PostIncrement, PreDecrement, PostDecrement
    };

    explicit UnaryOperator(Op op, Expression *expr);
    ~UnaryOperator();

    Type type() const;

    Op op;
    Expression *expr;
};

class BinaryOperator : public Expression
{
public:
    enum Op {
        Plus, Minus, Times, Equal, EqualType, NotEqual, NotEqualType,
        LessThan, GreaterThan, LessThanEqual, GreaterThanEqual,
        Div, Mod, And, Or
    };

    explicit BinaryOperator(Op op, Expression *left, Expression *right);
    ~BinaryOperator();

    Type type() const;
    const char* opStr() const;

    Op op;
    Expression *left;
    Expression *right;
};

class FunctionCall : public Expression
{
public:
    explicit FunctionCall(Expression* function, Expression *args = nullptr);
    ~FunctionCall();

    Type type() const;

    Expression* function;
    ExpressionList *arguments;
};

class ExpressionList : public Expression
{
public:
    explicit ExpressionList();
    explicit ExpressionList(const std::vector<Expression*>& expr);
    explicit ExpressionList(Expression *expr, ExpressionList *lst = nullptr);
    ~ExpressionList();

    Type type() const;

    std::vector<Expression*> expressions;
};




// public Statement

class Try : public Statement
{
public:
    explicit Try(StatementList *body, VariableList* variables, StatementList *catchPart);
    ~Try();

    Type type() const;

    StatementList *body;
    VariableList  *variables;
    StatementList *catchPart;
};

class Assignment : public Statement
{
public:
    explicit Assignment(Variable *var, Expression *expr);
    ~Assignment();

    Type type() const;

    Variable *variable;
    Expression *expression;
};

class If : public Statement
{
public:
    explicit If(Expression *cond, Statement *thenStm, Statement *elseStm);
    ~If();

    Type type() const;

    Expression *condition;
    StatementList *thenStatement;
    StatementList *elseStatement;
};

class While : public Statement
{
public:
    explicit While(Expression *cond, Statement *stm);
    ~While();

    Type type() const;

    Expression *condition;
    StatementList *statement;
};

class For : public Statement
{
public:
    explicit For(Expression *init, Expression *cond, Expression *after, Statement *stm);
    ~For();

    Type type() const;

    Expression *init;
    Expression *cond;
    Expression *after;
    StatementList *statement;
};

class Return : public Statement
{
public:
    explicit Return(Expression *expr);

    Type type() const;

    Expression *expression;
};

class Break : public Statement
{
public:
    explicit Break();

    Type type() const;
};

class Continue : public Statement
{
public:
    explicit Continue();

    Type type() const;
};

class StatementList : public Statement
{
public:
    explicit StatementList(Statement *stm = nullptr, StatementList *lst = nullptr);
    ~StatementList();

    Type type() const;

    std::vector<Statement*> statements;
};




// Functions
class VariableList : public Node
{
public:
    explicit VariableList(Variable *var = nullptr, VariableList *lst = nullptr);
    ~VariableList();

    Type type() const;

    std::vector<Variable*> variables;
};

class Function : public Node
{
public:
    explicit Function(VariableList *params, StatementList *stm = nullptr);
    explicit Function(const std::string &name, VariableList *params, StatementList *stm = nullptr);
    ~Function();

    Type type() const;
    bool isLambda() const;

    std::string name;
    VariableList *parameters;
    StatementList *statements;
};




// Program
class Program
{
public:
    explicit Program();

    void setGlobalVariables(VariableList *globvars);
    void addFunction(Function *function);

    void print() const;

private:
    VariableList* m_globVars;
    std::vector<Function*> m_functions;

    Function *m_main;

    friend class Utils;
    friend class Generic;
};

} // namespace Ast
