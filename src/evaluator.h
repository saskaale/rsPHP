#pragma once

#include "common.h"
#include "ast.h"
#include "parser.hpp"

#include <cstring>

struct AVal{
  enum Type {INT, BOOL, DOUBLE, FUNCTION, STRING};
  explicit AVal(Ast::Function* func): type(FUNCTION), func(func){};
  explicit AVal(double value): type(DOUBLE), fValue(value){};
  explicit AVal(int value): type(INT), value(value){};
  explicit AVal(bool value): type(BOOL), value(value) {};
  explicit AVal(const char *value): type(STRING), str(strdup(value)) {};
  AVal() {};
  void cleanup() {
      if (type == FUNCTION) {
          delete func;
      } else if (type == STRING) {
          free(str);
      }
  }

  Type type;
  union{
    double fValue;
    int value;
    Ast::Function *func;
    char *str;
  };
};

namespace Evaluator {
    void eval(Ast::Node *p);
    void cleanup(Ast::Node *p);
}
