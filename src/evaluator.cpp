#include "common.h"
#include "ast.h"
#include "parser.hpp"
#include "environment.h"

#include <iostream>


#define OP_PLUS(arg1, arg2) ((arg1)+(arg2))
#define OP_MINUS(arg1, arg2) ((arg1)-(arg2))
#define OP_EQ(arg1, arg2) ((arg1)==(arg2))
#define OP_NEQ(arg1, arg2) ((arg1)!=(arg2))
#define OP_LT(arg1, arg2) ((arg1)<(arg2))
#define OP_GT(arg1, arg2) ((arg1)>(arg2))
#define OP_LTE(arg1, arg2) ((arg1)<=(arg2))
#define OP_GTE(arg1, arg2) ((arg1)>=(arg2))
#define OP_TIMES(arg1, arg2) ((arg1)*(arg2))
#define OP_DIV(arg1, arg2) ((arg1)/(arg2))
#define OP_MOD(arg1, arg2) ((arg1)%(arg2))
#define OP_LAND(arg1, arg2) ((arg1)&&(arg2))
#define OP_LOR(arg1, arg2) ((arg1)||(arg2))
#define BINARY_OPERATOR(arg1, arg2, opname) ( Ast::Value ( OP_ ## opname ( (arg1).value, (arg2).value) ) )
#define TO_INT(arg) (arg).value
#define TO_BOOL(arg) (!!TO_INT(arg))
#define IS_FUNCTION(arg) ((arg).valueType == Ast::Value::FUNCTION)

Ast::Value ex(Ast::Node *p, Environment* envir)
{
    if (!p) {
        return Ast::Value(0);
    }
    
    switch (p->type()) {
    case Ast::Node::ValueT:
        return *(p->as<Ast::Value*>());

    case Ast::Node::VariableT: {
        Ast::Variable *v = p->as<Ast::Variable*>();
        Ast::Node *stored = envir->get(v->name);
        return ex(stored, envir); // return n->type == typeCon ? n->con.value : 0;
    }

    case Ast::Node::AssignmentT: {
        Ast::Assignment *v = p->as<Ast::Assignment*>();
        Ast::Value r = ex(v->expression, envir);
        envir->set(v->variable->name, r);
        return r;
    }

    case Ast::Node::FunctionT: {
         Ast::Function *v = p->as<Ast::Function*>();
         Ast::Value fun = Ast::Value(v->parameters->as<Ast::VariableList*>(), v->statements->as<Ast::StatementList*>());
         envir->set(v->name, fun);
         return fun;
    }    
    
    case Ast::Node::FunctionCallT: {
         Ast::FunctionCall *v = p->as<Ast::FunctionCall*>();

         //handle built-in functions
         if (v->functionName == "print") {
             Ast::Value printV = ex(v->arguments->expressions.front(), envir);
             if(printV.valueType == Ast::Value::BOOL){
                printf("%s\n", TO_BOOL(printV)?"true":"false");
             }else{
                printf("%d\n", TO_INT(printV));
             }
         }
         
         if(envir->has(v->functionName)){
            Ast::Value* func = envir->get(v->functionName);
            if(IS_FUNCTION(*func)){
                //create environment for this function
                Environment funcEnvironment(envir);
                //execute statement list of function
                ex(func->statements,&funcEnvironment);
            }
         }
         
         return Ast::Value(0);
    }

    case Ast::Node::UnaryOperatorT: {
        Ast::UnaryOperator *v = p->as<Ast::UnaryOperator*>();
        switch (v->op) {
        case Ast::UnaryOperator::Minus:
            return Ast::Value(-ex(v->expr, envir).value);
        default:
            X_UNREACHABLE();
        };
        break;
    }

    case Ast::Node::BinaryOperatorT: {
        Ast::BinaryOperator *v = p->as<Ast::BinaryOperator*>();
        switch (v->op) {
        case Ast::BinaryOperator::Plus:
            return BINARY_OPERATOR(ex(v->left, envir), ex(v->right, envir), PLUS );
        case Ast::BinaryOperator::Minus:
            return BINARY_OPERATOR(ex(v->left, envir), ex(v->right, envir), MINUS );
        case Ast::BinaryOperator::Times:
            return BINARY_OPERATOR(ex(v->left, envir), ex(v->right, envir), TIMES);
        case Ast::BinaryOperator::Equal:
            return BINARY_OPERATOR(ex(v->left, envir), ex(v->right, envir), EQ );
        case Ast::BinaryOperator::NotEqual:
            return BINARY_OPERATOR(ex(v->left, envir), ex(v->right, envir), NEQ );
        case Ast::BinaryOperator::LessThan:
            return BINARY_OPERATOR(ex(v->left, envir), ex(v->right, envir), LT);
        case Ast::BinaryOperator::GreaterThan:
            return BINARY_OPERATOR(ex(v->left, envir), ex(v->right, envir), GT);
        case Ast::BinaryOperator::LessThanEqual:
            return BINARY_OPERATOR(ex(v->left, envir), ex(v->right, envir), LTE);
        case Ast::BinaryOperator::GreaterThanEqual:
            return BINARY_OPERATOR(ex(v->left, envir), ex(v->right, envir), GTE);
        case Ast::BinaryOperator::Div:
            return BINARY_OPERATOR(ex(v->left, envir), ex(v->right, envir), DIV);
        case Ast::BinaryOperator::Mod:
            return BINARY_OPERATOR(ex(v->left, envir), ex(v->right, envir), MOD);
        case Ast::BinaryOperator::And:
            return BINARY_OPERATOR(ex(v->left, envir), ex(v->right, envir), LAND);
        case Ast::BinaryOperator::Or:
            return BINARY_OPERATOR(ex(v->left, envir), ex(v->right, envir), LOR);
        default:
            X_UNREACHABLE();
        }
        break;
    }

    case Ast::Node::StatementListT: {
        Ast::StatementList *v = p->as<Ast::StatementList*>();
        
        Ast::Value ret = Ast::Value(false);
        for (Ast::Statement *s : v->statements) {
            ret = ex(s, envir);
        }
        return ret;
    }

    case Ast::Node::IfT: {
        Ast::If *v = p->as<Ast::If*>();
        
        Ast::Value cond = ex(v->condition, envir);
        if (TO_BOOL(cond)) {
            ex(v->thenStatement, envir);
        } else {
            ex(v->elseStatement, envir);
        }
        return cond;
    }

    case Ast::Node::WhileT: {
        Ast::While *v = p->as<Ast::While*>();
        while (TO_BOOL(ex(v->condition, envir))) {
            ex(v->statement, envir);
        }
        return Ast::Value(false);
    }

    case Ast::Node::ForT: {
        Ast::For *v = p->as<Ast::For*>();
        for (ex(v->init, envir); TO_BOOL(ex(v->cond, envir)); ex(v->after, envir)) {
            ex(v->statement, envir);
        }
        return Ast::Value(false);
    }

    default: {
        X_UNREACHABLE();
        break;
    }
    }

    return Ast::Value(false);
}
