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
class BoolLiteral;
class StringLiteral;
class BinaryOperator;
class FunctionCall;
class ExpressionList;

// Statements
class Assignment;
class If;
class While;
class For;
class Exit;
class Write;
class StatementList;

// Functions
class VariableList;
class Function;

// Generic Loop
class Loop;

// Program
class Program;

class Node
{
public:
    enum Type {
        VariableT, ArrayT, ArraySubscriptT, IntegerLiteralT, BoolLiteralT, StringLiteralT,
        UnaryOperatorT, BinaryOperatorT, FunctionCallT, ExpressionListT, AssignmentT,
        IfT, WhileT, ForT, ExitT, WriteT,
        StatementListT, VariableListT, FunctionT, LoopT
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
    explicit Variable(const std::string &name);

    Type type() const;
    void print() const;

    std::string name;
};

class Array : public Variable
{
public:
    explicit Array(const std::string &name, int size);

    Type type() const;

    int size;
};

class ArraySubscript : public Variable
{
public:
    explicit ArraySubscript(const std::string &name, Expression *expr);

    Type type() const;

    Expression *expression;
};

class IntegerLiteral : public Expression
{
public:
    explicit IntegerLiteral(int value);

    Type type() const;

    int value;
};

class BoolLiteral : public Expression
{
public:
    explicit BoolLiteral(bool value);

    Type type() const;

    bool value;
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
        Minus, PreIncrement, PostIncrement, PreDecrement, PostDecrement
    };

    explicit UnaryOperator(Op op, Expression *expr);

    Type type() const;

    Op op;
    Expression *expr;
};

class BinaryOperator : public Expression
{
public:
    enum Op {
        Plus, Minus, Times, Equal, NotEqual,
        LessThan, GreaterThan, LessThanEqual, GreaterThanEqual,
        Div, Mod, And, Or
    };

    explicit BinaryOperator(Op op, Expression *left, Expression *right);

    Type type() const;

    Op op;
    Expression *left;
    Expression *right;
};

class FunctionCall : public Expression
{
public:
    explicit FunctionCall(const std::string &name, Expression *args = nullptr);

    Type type() const;

    std::string functionName;
    ExpressionList *arguments;
};

class ExpressionList : public Expression
{
public:
    explicit ExpressionList();
    explicit ExpressionList(Expression *expr, ExpressionList *lst = nullptr);

    Type type() const;

    std::vector<Expression*> expressions;
};




// public Statement
class Assignment : public Statement
{
public:
    explicit Assignment(Node *var, Expression *expr);

    Type type() const;

    Variable *variable;
    Expression *expression;
};

class If : public Statement
{
public:
    explicit If(Expression *cond, Statement *thenStm, Statement *elseStm);

    Type type() const;

    Expression *condition;
    StatementList *thenStatement;
    StatementList *elseStatement;
};

class While : public Statement
{
public:
    explicit While(Expression *cond, Statement *stm);

    Type type() const;

    Expression *condition;
    StatementList *statement;
};

class For : public Statement
{
public:
    explicit For(Expression *init, Expression *cond, Expression *after, Statement *stm);

    Type type() const;

    Expression *init;
    Expression *cond;
    Expression *after;
    StatementList *statement;
};

class Exit : public Statement
{
public:
    explicit Exit();

    Type type() const;
};

class Write : public Statement
{
public:
    explicit Write(const std::string &str);

    Type type() const;

    std::string str;
};

class StatementList : public Statement
{
public:
    explicit StatementList(Statement *stm, StatementList *lst = nullptr);

    Type type() const;

    std::vector<Statement*> statements;
};




// Functions
class VariableList : public Node
{
public:
    explicit VariableList();
    explicit VariableList(Variable *var, VariableList *lst = nullptr);

    Type type() const;

    std::vector<Variable*> variables;
};

class Function : public Node
{
public:
    explicit Function(const std::string &name, VariableList *params, StatementList *stm = nullptr);

    Type type() const;

    std::string name;
    VariableList *parameters;
    StatementList *statements;
};




class Loop : public Statement
{
public:
    explicit Loop(Expression *cond, StatementList *stmlist);

    Type type() const;

    Expression *condition;
    StatementList *statement;
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
