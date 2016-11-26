#pragma once

#include "common.h"
#include "ast.h"
#include "parser.hpp"


struct AVal{
  enum Type {INT, BOOL, DOUBLE, FUNCTION};
  explicit AVal(Ast::Function* func): type(FUNCTION), func(func){};
  explicit AVal(double value): type(DOUBLE), fValue(value){};
  explicit AVal(int value): type(INT), value(value){};
  explicit AVal(bool value): type(BOOL), value(value) {};
  AVal() {};

  Type type;
  union{
    double fValue;
    int value;
    Ast::Function* func;
  };
};

