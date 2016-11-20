#include "ast.h"

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


Variable::Variable(const std::string &name, bool decl, Classifier cls, int val)
    : name(name)
    , classifier(cls)
    , value(val)
    , declaration(decl)
{
}

Node::Type Variable::type() const
{
    return VariableT;
}

bool Variable::isConst() const
{
    return classifier == Const;
}

bool Variable::isDeclaration() const
{
    return declaration;
}

void Variable::print() const
{
    printf("Variable ");
    printf("'%s' ", name.c_str());
    if (isConst())
        printf("const = %d ", value);

#if 0
    if (const Ast::Array *arr = dynamic_cast<const Ast::Array*>(this))
        printf(": array[%d .. %d] - %d\n", arr->from, arr->to, arr->size);
    else
        printf(": integer\n");
#endif
}


Array::Array(const std::string &name, int from, int to)
    : Variable(name, /*decl*/true, Normal)
    , from(from)
    , to(to)
{
    size = to - from + 1;
}

Node::Type Array::type() const
{
    return ArrayT;
}


ArraySubscript::ArraySubscript(const std::string &name, Expression *expr)
    : Variable(name)
    , array(0)
    , expression(expr)
{
}

Node::Type ArraySubscript::type() const
{
    return ArraySubscriptT;
}

void ArraySubscript::updateExpression()
{
    if (array->from == 0)
        return;

    // So the array can start from 0 ...
    IntegerLiteral *num = new IntegerLiteral(array->from);
    expression = new BinaryOperator(BinaryOperator::Minus, expression, num);
}


IntegerLiteral::IntegerLiteral(int value)
    : value(value)
{
}

Node::Type IntegerLiteral::type() const
{
    return IntegerLiteralT;
}


StringLiteral::StringLiteral(const std::string &value)
    : value(value)
{
}

Node::Type StringLiteral::type() const
{
    return StringLiteralT;
}


BinaryOperator::BinaryOperator(Op op, Node *left, Node *right)
    : op(op)
    , left(static_cast<Expression*>(left))
    , right(static_cast<Expression*>(right))
{
}

Node::Type BinaryOperator::type() const
{
    return BinaryOperatorT;
}


FunctionCall::FunctionCall(const std::string &name, Node *args)
    : functionName(name)
    , function(0)
{
    if (dynamic_cast<Expression*>(args)) {
        Expression *exp = static_cast<Expression*>(args);
        arguments = new ExpressionList(exp);
    } else if (dynamic_cast<ExpressionList*>(args)) {
        arguments = static_cast<ExpressionList*>(args);
    } else {
        // assert(false);
    }
}

Node::Type FunctionCall::type() const
{
    return FunctionCallT;
}


ExpressionList::ExpressionList(Expression *expr, ExpressionList *list)
{
    if (list)
        expressions = list->expressions;
    expressions.push_front(expr);
}

Node::Type ExpressionList::type() const
{
    return ExpressionListT;
}

void ExpressionList::append(ExpressionList *list)
{
    if (list)
        expressions.insert(expressions.end(), list->expressions.begin(), list->expressions.end());
}


Assignment::Assignment(Node *var, Node *expr)
    : variable(static_cast<Variable*>(var))
    , expression(static_cast<Expression*>(expr))
{
}

Node::Type Assignment::type() const
{
    return AssignmentT;
}


If::If(Expression *cond, StatementList *thenStm, StatementList *elseStm)
    : condition(cond)
    , thenStatement(thenStm)
    , elseStatement(elseStm)
{
}

Node::Type If::type() const
{
    return IfT;
}


While::While(Expression *cond, StatementList *stm)
    : condition(cond)
    , statement(stm)
{
}

Node::Type While::type() const
{
    return WhileT;
}


For::For(Variable *var, Direction d, Expression *from, Expression *to, StatementList *stm)
    : variable(var)
    , direction(d)
    , from(from)
    , to(to)
    , statement(stm)
{
}

Node::Type For::type() const
{
    return ForT;
}


Exit::Exit()
{
}

Node::Type Exit::type() const
{
    return ExitT;
}


Write::Write(const std::string &str)
    : str(str + "\n")
{
}

Node::Type Write::type() const
{
    return WriteT;
}


StatementList::StatementList(Statement *stm, StatementList *list)
{
    if (list)
        statements = list->statements;
    statements.push_front(stm);
}

Node::Type StatementList::type() const
{
    return StatementListT;
}

void StatementList::append(StatementList *list)
{
    if (list)
        statements.insert(statements.end(), list->statements.begin(), list->statements.end());
}


VariableList::VariableList(Variable *var, VariableList *list)
{
    if (list)
        variables = list->variables;
    variables.push_front(var);
}

Node::Type VariableList::type() const
{
    return VariableListT;
}

void VariableList::append(VariableList *list)
{
    if (list)
        variables.insert(variables.end(), list->variables.begin(), list->variables.end());
}


Function::Function(const std::string &name, Variable *ret,
                   VariableList *params, VariableList *locvars,
                   StatementList *stm)
    : name(name)
    , ret(ret)
    , parameters(params)
    , localVariables(locvars)
    , statements(stm)
    , forward(stm == 0)
    , forwardedFunc(0)
{
}

Node::Type Function::type() const
{
    return FunctionT;
}

bool Function::isForward() const
{
    return forward;
}

bool Function::isProcedure() const
{
    return ret == 0;
}


// Generic Loop
Loop::Loop(Expression *cond, StatementList *stmlist)
    : condition(cond)
    , statement(stmlist)
{
}

Node::Type Loop::type() const
{
    return LoopT;
}


Program::Program()
    : m_globVars(0)
{
    // Create dummy built-in functions
    m_print = new Function("print", 0, 0, 0, 0);
#if 0
    m_write = new Function("write", 0, 0, 0, 0);
    m_readln = new Function("readln", 0, 0, 0, 0);
    m_dec = new Function("dec", 0, 0, 0, 0);
#endif
}

void Program::setGlobalVariables(VariableList *globvars)
{
    if (globvars)
        m_globVars = globvars;
}

void Program::addFunction(Function *function)
{
    if (function)
        m_functions.push_back(function);
}

void Program::print() const
{
#if 0
    if (m_globVars) {
        std::list<Variable*>::const_iterator it;
        for (it = m_globVars->variables.begin();
                it != m_globVars->variables.end(); ++it) {
            (*it)->print();
        }
    }

    if (!m_functions.empty()) {
        std::list<Function*>::const_iterator it;
        for (it = m_functions.begin();
                it != m_functions.end(); ++it) {
            Function *func = *it;
            printf("Func: %s Statements: %d LocVars: %d Params: %d\n",
                    func->name.c_str(),
                    !func->isForward() ? (int) func->statements->statements.size() : 0,
                    func->localVariables ? (int) func->localVariables->variables.size() : 0,
                    func->parameters ? (int) func->parameters->variables.size() : 0
                    );
        }
    }
#endif
}

} // namespace Ast
