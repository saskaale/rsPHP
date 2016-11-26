#include "common.h"
#include "ast.h"
#include "parser.hpp"
#include "environment.h"
#include "evaluator.h"

#include <iostream>


#define TO_INT(arg) IS_DOUBLE(arg) ? (int)(arg).fValue : (arg).value
#define TO_DOUBLE(arg) (IS_INT(arg) || IS_BOOL(arg)) ? (double)(arg).value : (arg).fValue
#define TO_BOOL(arg) (!!TO_INT(arg))
#define IS_FUNCTION(arg) ((arg).type == AVal::FUNCTION)
#define IS_INT(arg) ((arg).type == AVal::INT)
#define IS_BOOL(arg) ((arg).type == AVal::BOOL)
#define IS_DOUBLE(arg) ((arg).type == AVal::DOUBLE)


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

#define INT_BINARY_OPERATOR(arg1, arg2, opname) ( AVal ( OP_ ## opname ( TO_INT(arg1), TO_INT(arg2) ) ) ) 
#define DOUBLE_BINARY_OPERATOR(arg1, arg2, opname) ( AVal ( OP_ ## opname ( TO_DOUBLE(arg1), TO_DOUBLE(arg2) ) ) ) 

#define BINARY_OPERATOR(arg1, arg2, opname) ( \
    (IS_BOOL(arg1) || IS_BOOL(arg2)) \
      ? \
    DOUBLE_BINARY_OPERATOR ( arg1, arg2, opname ) \
      : \
    INT_BINARY_OPERATOR ( arg1, arg2, opname ) \
    ) 


Environment globalenvir;

AVal ex(Ast::Node *p, Environment* envir)
{
    if (!p) {
        return AVal(0);
    }
    
    switch (p->type()) {

    case Ast::Node::IntegerLiteralT:
        return AVal(p->as<Ast::IntegerLiteral*>()->value);

    case Ast::Node::BoolLiteralT:
        return AVal(p->as<Ast::BoolLiteral*>()->value);

    case Ast::Node::DoubleLiteralT:
        return AVal(p->as<Ast::DoubleLiteral*>()->value);

    case Ast::Node::VariableT: {
        Ast::Variable *v = p->as<Ast::Variable*>();
        if(!envir->has(v->name)){
            fprintf(stderr, "SYMBOL %s LOOKUP ERROR\n", v->name.c_str());
            X_ASSERT(false && "SYMBOL LOOKUP ERROR");
        }
        return envir->get(v->name);
    }

    case Ast::Node::AssignmentT: {
        Ast::Assignment *v = p->as<Ast::Assignment*>();
        AVal r = ex(v->expression, envir);
        envir->set(v->variable->name, r);
        return r;
    }

    case Ast::Node::FunctionT: {
         Ast::Function *v = p->as<Ast::Function*>();
         AVal fun = AVal(v);
         envir->set(v->name, fun);
         return fun;
    }    
    
    case Ast::Node::FunctionCallT: {
         Ast::FunctionCall *v = p->as<Ast::FunctionCall*>();

         //handle built-in functions
         if (v->functionName == "print") {
             AVal printV = ex(v->arguments->expressions.front(), envir);
             if(IS_BOOL(printV)){
                printf("%s\n", TO_BOOL(printV)?"true":"false");
             }else if(IS_DOUBLE(printV)){
                printf("%lf\n", TO_DOUBLE(printV));
             }else{
                printf("%d\n", TO_INT(printV));
             }
         }

         if(envir->has(v->functionName)){
            AVal func = envir->get(v->functionName);
            if(IS_FUNCTION(func)){
                //create environment for this function
                Environment funcEnvironment(envir);

                Ast::VariableList *parameters = func.func->parameters;
                Ast::ExpressionList *exprs    = v->arguments;
                if(parameters->variables.size() != exprs->expressions.size()){
                  fprintf(stderr, "PARAMETERS MISMATCH FOR FUNCTION %s\n", v->functionName.c_str());
                  X_ASSERT(false && "PARAMETERS MISMATCH");
                  return AVal(0);
                }


                int argslen = exprs->expressions.size();
                for(int i = 0; i < argslen; i++){
                  funcEnvironment.set(parameters->variables[i]->name, ex(exprs->expressions[i], envir));
                }

                //execute statement list of function
                ex(func.func->statements,&funcEnvironment);
            }
         }

         return AVal(0);
    }

    case Ast::Node::UnaryOperatorT: {
        Ast::UnaryOperator *v = p->as<Ast::UnaryOperator*>();
        switch (v->op) {
        case Ast::UnaryOperator::Minus:
            return AVal(-ex(v->expr, envir).value);
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
            return INT_BINARY_OPERATOR(ex(v->left, envir), ex(v->right, envir), MOD);
        case Ast::BinaryOperator::And:
            return INT_BINARY_OPERATOR(ex(v->left, envir), ex(v->right, envir), LAND);
        case Ast::BinaryOperator::Or:
            return INT_BINARY_OPERATOR(ex(v->left, envir), ex(v->right, envir), LOR);
        default:
            X_UNREACHABLE();
        }
        break;
    }

    case Ast::Node::StatementListT: {
        Ast::StatementList *v = p->as<Ast::StatementList*>();
        
        AVal ret = AVal(false);
        for (Ast::Statement *s : v->statements) {
            ret = ex(s, envir);
        }
        return ret;
    }

    case Ast::Node::IfT: {
        Ast::If *v = p->as<Ast::If*>();
        
        AVal cond = ex(v->condition, envir);
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
        return AVal(false);
    }

    case Ast::Node::ForT: {
        Ast::For *v = p->as<Ast::For*>();
        for (ex(v->init, envir); TO_BOOL(ex(v->cond, envir)); ex(v->after, envir)) {
            ex(v->statement, envir);
        }
        return AVal(false);
    }

    default: {
        X_UNREACHABLE();
        break;
    }
    }

    return AVal(false);
}



void eval(Ast::Node *p)
{
  ex(p, &globalenvir);
}
