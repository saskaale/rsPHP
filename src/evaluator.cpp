#include <stdio.h>
#include "ast.h"
#include "parser.hpp"
#include "environment.h"

Environment envir;

int ex(Ast::Node *p)
{
    if (!p) {
        return 0;
    }

    switch (p->type()) {
    case Ast::Node::IntegerLiteralT:
        return static_cast<Ast::IntegerLiteral*>(p)->value;

    case Ast::Node::VariableT: {
        Ast::Variable *v = static_cast<Ast::Variable*>(p);
        Ast::Node *stored = envir.get(v->name.c_str());
        return ex(stored); // return n->type == typeCon ? n->con.value : 0;
    }

    case Ast::Node::AssignmentT: {
        Ast::Assignment *v = static_cast<Ast::Assignment*>(p);
        int r = ex(v->expression);
        envir.set(v->variable->name.c_str(), new Ast::IntegerLiteral(r));
        return 0;
    }

    case Ast::Node::FunctionCallT: {
         Ast::FunctionCall *v = static_cast<Ast::FunctionCall*>(p);
         if (v->functionName == "print") {
             printf("%d\n", ex(v->arguments->expressions.front()));
         }
         return 0;
    }

    case Ast::Node::BinaryOperatorT: {
        Ast::BinaryOperator *v = static_cast<Ast::BinaryOperator*>(p);
        switch (v->op) {
        case Ast::BinaryOperator::Plus:
            return ex(v->left) + ex(v->right);
        case Ast::BinaryOperator::Minus:
            return ex(v->left) - ex(v->right);
        case Ast::BinaryOperator::Times:
            return ex(v->left) * ex(v->right);
        case Ast::BinaryOperator::Equal:
            return ex(v->left) == ex(v->right);
        case Ast::BinaryOperator::NotEqual:
            return ex(v->left) != ex(v->right);
        case Ast::BinaryOperator::LessThan:
            return ex(v->left) < ex(v->right);
        case Ast::BinaryOperator::GreaterThan:
            return ex(v->left) > ex(v->right);
        case Ast::BinaryOperator::LessThanEqual:
            return ex(v->left) <= ex(v->right);
        case Ast::BinaryOperator::GreaterThanEqual:
            return ex(v->left) >= ex(v->right);
        case Ast::BinaryOperator::Div:
            return ex(v->left) / ex(v->right);
        case Ast::BinaryOperator::Mod:
            return ex(v->left) % ex(v->right);
        case Ast::BinaryOperator::And:
            return ex(v->left) && ex(v->right);
        case Ast::BinaryOperator::Or:
            return ex(v->left) || ex(v->right);
        default:
            break;
            // assert(false);
        }
    }

    }
    return 0;
}
