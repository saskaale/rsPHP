#include "common.h"
#include "ast.h"
#include "parser.hpp"
#include "environment.h"

#include <iostream>

Environment envir;

int ex(Ast::Node *p)
{
    if (!p) {
        return 0;
    }

    switch (p->type()) {
    case Ast::Node::ValueLiteralT:
        return p->as<Ast::ValueLiteral*>()->value;

    case Ast::Node::VariableT: {
        Ast::Variable *v = p->as<Ast::Variable*>();
        Ast::Node *stored = envir.get(v->name.c_str());
        return ex(stored); // return n->type == typeCon ? n->con.value : 0;
    }

    case Ast::Node::AssignmentT: {
        Ast::Assignment *v = p->as<Ast::Assignment*>();
        int r = ex(v->expression);
        envir.set(v->variable->name.c_str(), new Ast::ValueLiteral(r));
        return 0;
    }

    case Ast::Node::FunctionCallT: {
         Ast::FunctionCall *v = p->as<Ast::FunctionCall*>();
         if (v->functionName == "print") {
             printf("%d\n", ex(v->arguments->expressions.front()));
         }
         return 0;
    }

    case Ast::Node::UnaryOperatorT: {
        Ast::UnaryOperator *v = p->as<Ast::UnaryOperator*>();
        switch (v->op) {
        case Ast::UnaryOperator::Minus:
            return -ex(v->expr);
        default:
            X_UNREACHABLE();
        };
        break;
    }

    case Ast::Node::BinaryOperatorT: {
        Ast::BinaryOperator *v = p->as<Ast::BinaryOperator*>();
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
            X_UNREACHABLE();
        }
        break;
    }

    case Ast::Node::StatementListT: {
        Ast::StatementList *v = p->as<Ast::StatementList*>();
        for (Ast::Statement *s : v->statements) {
            ex(s);
        }
        return 0;
    }

    case Ast::Node::IfT: {
        Ast::If *v = p->as<Ast::If*>();
        if (ex(v->condition)) {
            ex(v->thenStatement);
        } else {
            ex(v->elseStatement);
        }
        return 0;
    }

    case Ast::Node::WhileT: {
        Ast::While *v = p->as<Ast::While*>();
        while (ex(v->condition)) {
            ex(v->statement);
        }
        return 0;
    }

    case Ast::Node::ForT: {
        Ast::For *v = p->as<Ast::For*>();
        for (ex(v->init); ex(v->cond); ex(v->after)) {
            ex(v->statement);
        }
        return 0;
    }

    default:
        std::cout << "Unhandled node " << p << std::endl;
        break;
    }

    return 0;
}
