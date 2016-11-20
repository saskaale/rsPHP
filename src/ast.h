#pragma once

#include <list>
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
        VariableT, ArrayT, ArraySubscriptT, IntegerLiteralT, StringLiteralT,
        BinaryOperatorT, FunctionCallT, ExpressionListT, AssignmentT,
        IfT, WhileT, ForT, ExitT, WriteT,
        StatementListT, VariableListT, FunctionT, LoopT
    };

    Node();
    virtual ~Node();

    virtual Type type() const = 0;

    void *data;
};

class Expression : public Node
{
};

class Statement : public Node
{
};




// public Expression
class Variable : public Expression
{
public:
    enum Classifier {
        Const,
        Normal
    };

    Variable(const std::string &name, bool decl = false,
             Classifier cls = Normal, int val = 0);

    Type type() const;
    bool isConst() const;
    bool isDeclaration() const;
    void print() const;

    std::string name;
    Classifier classifier;
    int value;
    bool declaration;
};

class Array : public Variable
{
public:
    Array(const std::string &name, int from, int to);

    Type type() const;

    int from;
    int to;
    int size;
};

class ArraySubscript : public Variable
{
public:
    ArraySubscript(const std::string &name, Expression *expr);

    Type type() const;
    void updateExpression();

    Array *array; // needs to be resolved
    Expression *expression;
};

class IntegerLiteral : public Expression
{
public:
    IntegerLiteral(int value);

    Type type() const;

    int value;
};

class StringLiteral : public Expression
{
public:
    StringLiteral(const std::string &value);

    Type type() const;

    std::string value;
};

class BinaryOperator : public Expression
{
public:
    enum Op {
        Plus, Minus, Times, Equal, NotEqual,
        LessThan, GreaterThan, LessThanEqual, GreaterThanEqual,
        Div, Mod, And, Or
    };

    BinaryOperator(Op op, Node *left, Node *right);

    Type type() const;

    Op op;
    Expression *left;
    Expression *right;
};

class FunctionCall : public Expression
{
public:
    FunctionCall(const std::string &name, Node *args);

    Type type() const;

    std::string functionName;
    Function *function; // needs to be resolved
    ExpressionList *arguments;
};

class ExpressionList : public Expression
{
public:
    ExpressionList(Expression *expr, ExpressionList *list = 0);

    Type type() const;
    void append(ExpressionList *list);

    std::list<Expression*> expressions;
};




// public Statement
class Assignment : public Statement
{
public:
    Assignment(Node *var, Node *expr);

    Type type() const;

    Variable *variable;
    Expression *expression;
};

class If : public Statement
{
public:
    If(Expression *cond, StatementList *thenStm, StatementList *elseStm);

    Type type() const;

    Expression *condition;
    StatementList *thenStatement;
    StatementList *elseStatement;
};

class While : public Statement
{
public:
    While(Expression *cond, StatementList *stm);

    Type type() const;

    Expression *condition;
    StatementList *statement;
};

class For : public Statement
{
public:
    enum Direction {
        To,
        Downto
    };

    For(Variable *var, Direction d, Expression *from, Expression *to, StatementList *stm);

    Type type() const;

    Variable *variable;
    Direction direction;
    Expression *from;
    Expression *to;
    StatementList *statement;
};

class Exit : public Statement
{
public:
    Exit();

    Type type() const;
};

class Write : public Statement
{
public:
    Write(const std::string &str);

    Type type() const;

    std::string str;
};

class StatementList : public Statement
{
public:
    StatementList(Statement *stm, StatementList *list = 0);

    Type type() const;
    void append(StatementList *list);

    std::list<Statement*> statements;
};




// Functions
class VariableList : public Node
{
public:
    VariableList(Variable *var, VariableList *list = 0);

    Type type() const;
    void append(VariableList *list);

    std::list<Variable*> variables;
};

class Function : public Node
{
public:
    Function(const std::string &name, Variable *ret,
             VariableList *params, VariableList* locvars,
             StatementList *stm = 0);

    Type type() const;
    bool isForward() const;
    bool isProcedure() const;

    std::string name;
    Variable *ret;
    VariableList *parameters;
    VariableList *localVariables;
    StatementList *statements;

    bool forward;
    Function *forwardedFunc;
};




class Loop : public Statement
{
public:
    Loop(Expression *cond, StatementList *stmlist);

    Type type() const;

    Expression *condition;
    StatementList *statement;
};




// Program
class Program
{
public:
    Program();

    void setGlobalVariables(VariableList *globvars);
    void addFunction(Function *function);

    void print() const;

private:
    VariableList* m_globVars;
    std::list<Function*> m_functions;

    Function *m_main;
    Function *m_print;
#if 0
    Function *m_write;
    Function *m_readln;
    Function *m_dec;
#endif

    friend class Utils;
    friend class Generic;
};

} // namespace Ast
