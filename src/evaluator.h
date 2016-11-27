#pragma once

#include "common.h"
#include "ast.h"
#include "parser.hpp"

#include <cstring>
#include <cstdlib>
#include <sstream>

struct AVal{
  enum Type {INT, BOOL, DOUBLE, FUNCTION, STRING};
  AVal(Ast::Function* func): type(FUNCTION), func(func){};
  AVal(double value): type(DOUBLE), fValue(value){};
  AVal(int value): type(INT), value(value){};
  AVal(bool value): type(BOOL), value(value) {};
  AVal(const char *value): type(STRING), str(strdup(value)) {};
  AVal() {};
  void cleanup() {
      if (type == FUNCTION) {
          delete func;
      } else if (type == STRING) {
          free(str);
      }
  }

  int toInt() const {
      return convertTo(INT).value;
  }

  bool toBool() const {
      return convertTo(BOOL).value;
  }

  double toDouble() const {
      return convertTo(DOUBLE).fValue;
  }

  Ast::Function *toFunction() const {
      return convertTo(FUNCTION).func;
  }

  const char *toString() const {
      return convertTo(STRING).str;
  }

  AVal convertTo(Type t) const {
      switch (type) {
      case INT:
          switch (t) {
          case INT:
              return value;
          case BOOL:
              return bool(value);
          case DOUBLE:
              return double(value);
          case FUNCTION:
              return static_cast<Ast::Function*>(nullptr);
          case STRING: {
              std::stringstream ss;
              ss << value;
              return strdup(ss.str().c_str());
          }
          default:
              X_UNREACHABLE();
          }

      case BOOL:
          return AVal(value).convertTo(t);

      case DOUBLE:
          switch (t) {
          case INT:
              return int(fValue);
          case BOOL:
              return bool(fValue);
          case DOUBLE:
              return double(fValue);
          case FUNCTION:
              return static_cast<Ast::Function*>(nullptr);
          case STRING: {
              std::stringstream ss;
              ss << fValue;
              return strdup(ss.str().c_str());
          }
          default:
              X_UNREACHABLE();
          }

      case FUNCTION:
          switch (t) {
          case INT:
          case BOOL:
          case DOUBLE:
              return AVal(func ? true : false).convertTo(t);
          case FUNCTION:
              return func;
          case STRING: {
              return "[function]";
          }
          default:
              X_UNREACHABLE();
          }

      case STRING:
          switch (t) {
          case INT:
              return atoi(str);
          case BOOL:
              return strlen(str) > 0;
          case DOUBLE:
              return atof(str);
          case FUNCTION:
              return static_cast<Ast::Function*>(nullptr);
          case STRING:
              return str;
          default:
              X_UNREACHABLE();
          }

      default:
          X_UNREACHABLE();
      }
      return 0;
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
