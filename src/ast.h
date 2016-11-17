#ifndef ____AST_H_12312313123123
#define ____AST_H_12312313123123

#define ID_LEN 30

void yyerror(const char* );

class nodeType;
class conNodeType;
class idNodeType;
class oprNodeType;

#include "environment.h"

typedef enum { typeCon, typeId, typeOpr } nodeEnum;


/* constants */
struct conNodeType{
    int value;                  /* value of constant */
};

/* identifiers */
struct idNodeType {
    char str[ID_LEN];
};

/* operators */
struct oprNodeType {
    int oper;                   /* operator */
    int nops;                   /* number of operands */
    struct nodeType *op[1];	/* operands, extended at runtime */
};

struct nodeType {
    nodeEnum type;              /* type of node */

    union {
        conNodeType con;        /* constants */
        idNodeType id;          /* identifiers */
        oprNodeType opr;        /* operators */
    };
};

extern Environment envir;


nodeType* con(int v);
nodeType* id(char str[ID_LEN]);
nodeType* opr(int open, int nops, ...);
void freeNode(nodeType* t);

#endif