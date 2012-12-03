//
// nodes.h
//
// Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
//

#ifndef NODES_GEN_H
#define NODES_GEN_H

#include "nemo.h"

typedef enum {
  UNARY_POSTINC,
  UNARY_POSTDEC,
  UNARY_PREINC,
  UNARY_PREDEC,
} Unary;

typedef union {
  int i;
} Value;

typedef enum {
  TYPE_INTEGER
} Type;

struct Node {
  enum {
    nt_ID,
    nt_INTEGER,
    nt_BINARYOP,
    nt_UNARYOP,
    nt_DECLARATION,
    nt_ASSIGNMENT,
    nt_BLOCK,
    nt_STATEMENT,
    nt_CALL,
    nt_WHILE,
    nt_IF,
    nt_FUNCDEF,
    nt_LASTELEMENT
  } kind;

  union {
    Value value;
    char *s;

    struct {
      struct Node *left, *right;
      char op;
    } binaryop;

    struct {
      struct Node *expression;
      Unary op;
    } unaryop;

    struct {
      Type type;
      char *name;
      // used when initializing a variable
      struct Node *right;
    } declaration;

    struct {
      char *name;
      struct Node *right;
    } assignment;

    struct {
      int count;
      // pointer to parent block
      struct Node *parent;
      struct VariableList *vars;
      struct Node **statements;
    } block;

    struct {
      int count;
      struct Node **nodes;
    } statement;

    struct {
      struct Node *cond;
      struct Node *statements;
    } whilee;

    struct {
      char *name;
      struct Node *param;
    } call;

    struct {
      struct Node *cond;
      struct Node *statements;
    } iff;

    struct {
      Type returntype;
      char *name;
      // TODO: arguments
      // struct Node *args[10];
      struct Node *body;
    } funcdef;
  } data;

  // block in which the node was created in
  // gonna be NULL in some cases
  struct Node *block;
};

struct Node *genDeclaration(Type, char *, struct Node *, struct Node *);
struct Node *genAssignment(char *, struct Node *, struct Node *);
struct Node *genEmptyBlock(struct Node *);
       void  blockappend(struct Node *, struct Node *);
struct Node *genStatement(struct Node *, struct Node *);
struct Node *genBinaryop(struct Node *, struct Node *, char);
struct Node *genUnaryop(struct Node *, Unary, struct Node *);
struct Node *genCall(char *, struct Node *);
struct Node *genWhile(struct Node *, struct Node *);
struct Node *genIf(struct Node *, struct Node *);
struct Node *genFuncDef(Type, char *, struct Node *);
struct Node *genExpByNum(int);
struct Node *genExpByName(char *, struct Node *);

#endif // NODES_GEN_H
