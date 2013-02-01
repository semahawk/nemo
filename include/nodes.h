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
  UNARY_PLUS,
  UNARY_MINUS,
} Unary;

typedef enum {
  BINARY_ADD,
  BINARY_SUB,
  BINARY_MUL,
  BINARY_DIV,
  BINARY_MOD,
  BINARY_GT,
  BINARY_LT,
  BINARY_GE,
  BINARY_LE,
  BINARY_NE,
  BINARY_EQ,
  BINARY_EQ_ADD,
  BINARY_EQ_SUB,
  BINARY_EQ_MUL,
  BINARY_EQ_DIV,
  BINARY_EQ_MOD,
  BINARY_EQ_CON,
  BINARY_STR_GT,
  BINARY_STR_LT,
  BINARY_STR_GE,
  BINARY_STR_LE,
  BINARY_STR_NE,
  BINARY_STR_EQ,
  BINARY_CON
} Binary;

typedef enum {
  TYPE_INTEGER,
  TYPE_FLOATING,
  TYPE_STRING
} Type;

typedef struct {
  Type type;
  union {
    int i;
    float f;
    char *s;
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
    nt_ID,         // 0
    nt_INTEGER,    // 1
    nt_FLOATING,   // 2
    nt_STRING,     // 3
    nt_BINARYOP,   // 4
    nt_UNARYOP,    // 5
    nt_ASSIGNMENT, // 6
    nt_BLOCK,      // 7
    nt_STATEMENT,  // 8
    nt_CALL,       // 9
    nt_RETURN,     // 10
    nt_WHILE,      // 11
    nt_IF,         // 12
    nt_FOR,        // 13
    nt_FUNCDEF,    // 14
    nt_ITER,       // 15
    nt_NOOP        // 16
  } kind;

  union {
    Value value;
    char *s;

    struct {
      Value value;
      char **vars;
      // total number of variables
      unsigned int vars_count;
      // actually, it's an array (of where does a variable of a
      // given index start in the string)
      unsigned int *vars_start;
      // total length of variables
      unsigned int vars_size;
    } string;

    struct {
      struct Node *left, *right;
      Binary op;
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
      // pointer to function definition in which that block
      // is a body of (yup, gonna be NULL sometimes)
      struct Node *funcdef;
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

    struct {
      char *type;
      struct Node *count;
      struct Node *stmt;
    } iter;
  } data;

  // block in which the node was created in
  // gonna be NULL in some cases
  struct Node *block;
};

#endif // NODES_H
