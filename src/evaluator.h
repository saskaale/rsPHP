#pragma once

#include "common.h"
#include "ast.h"
#include "parser.hpp"


struct AVal{
  enum Type {INT, BOOL, FUNCTION};
  explicit AVal(Ast::Function* func): type(FUNCTION), func(func){};
  explicit AVal(int value): type(INT), value(value){};
  explicit AVal(bool value): type(BOOL), value(value) {};
  AVal() {};

  Type type;
  union{
    int value;
    Ast::Function* func;
  };
};

