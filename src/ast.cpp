#include "ast.h"
#include "common.h"

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


Variable::Variable(const std::string &name)
    : name(name)
{
}

Node::Type Variable::type() const
{
    return VariableT;
}

void Variable::print() const
{
    printf("Variable %s", name.c_str());
}


Array::Array(const std::string &name, int size)
    : Variable(name)
    , size(size)
{
}

Node::Type Array::type() const
{
    return ArrayT;
}


ArraySubscript::ArraySubscript(const std::string &name, Expression *expr)
    : Variable(name)
    , expression(expr)
{
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

Node::Type BinaryOperator::type() const
{
    return BinaryOperatorT;
}


FunctionCall::FunctionCall(const std::string &name, Expression *args)
    : functionName(name)
{
    if (ExpressionList *lst = args->as<ExpressionList*>()) {
        arguments = lst;
    } else {
        arguments = new ExpressionList(args);
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


Assignment::Assignment(Node *var, Expression *expr)
    : variable(var->as<Variable*>())
    , expression(expr)
{
    X_ASSERT(variable);
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


Function::Function(const std::string &name, Variable *ret, VariableList *params, StatementList *stm)
    : name(name)
    , ret(ret)
    , parameters(params)
    , statements(stm)
{
}

Node::Type Function::type() const
{
    return FunctionT;
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
    : m_globVars(nullptr)
{
    // Create dummy built-in functions
    m_main = new Function("main", nullptr, nullptr, nullptr);
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
