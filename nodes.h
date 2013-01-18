//
// nodes.h
//
// Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
//

#ifndef NODES_H
#define NODES_H

#include "nemo.h"

typedef enum {
  UNARY_POSTINC,
  UNARY_POSTDEC,
  UNARY_PREINC,
  UNARY_PREDEC,
} Unary;

typedef enum {
  TYPE_INTEGER,
  TYPE_FLOATING
} Type;

typedef struct {
  Type type;
  union {
    int i;
    float f;
  } v;
} Value;

struct Arg {
  Type type;
  char *name;
};

struct ArgList {
  int pos;
  struct Arg *arg;
  struct ArgList *next;
};

struct ParamList {
  int pos;
  struct Node *param;
  struct ParamList *next;
};

struct Node {
  enum {
    nt_ID,
    nt_INTEGER,
    nt_FLOATING,
    nt_BINARYOP,
    nt_UNARYOP,
    nt_ASSIGNMENT,
    nt_BLOCK,
    nt_STATEMENT,
    nt_CALL,
    nt_RETURN,
    nt_WHILE,
    nt_IF,
    nt_FOR,
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
      struct ParamList *params;
      int paramcount;
    } call;

    struct {
      struct Node *expr;
    } returnn;

    struct {
      struct Node *cond;
      struct Node *stmt;
      struct Node *elsestmt;
    } iff;

    struct {
      struct Node *init;
      struct Node *cond;
      struct Node *action;
      struct Node *stmt;
    } forr;

    struct {
      Type returntype;
      char *name;
      int argcount;
      struct ArgList *args;
      struct Node *body;
    } funcdef;
  } data;

  // block in which the node was created in
  // gonna be NULL in some cases
  struct Node *block;
};

#endif // NODES_H
