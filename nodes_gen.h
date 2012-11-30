//
// nodes.h
//
// Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
//

#ifndef NODES_GEN_H
#define NODES_GEN_H

#include "nemo.h"

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
    nt_DECLARATION,
    nt_ASSIGNMENT,
    nt_BLOCK,
    nt_STATEMENT,
    nt_CALL,
    nt_WHILST,
    nt_AN,
    nt_LASTELEMENT
  } kind;

  union {
    Value value;
    char *s;

    struct {
      struct Node *left, *right;
      char op;
    } expression;

    struct {
      Type type;
      char *name;
      // used when initializing a variable
      struct Node *right;
      // block in which the variable was declared
      struct Node *block;
    } declaration;

    struct {
      char *name;
      struct Node *right;
      // block in which the variable was assigned
      struct Node *block;
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
    } whilst;

    struct {
      char *name;
      struct Node *param;
      // block in which the call was made
      struct Node *block;
    } call;

    struct {
      struct Node *cond;
      struct Node *statements;
    } an;
  } data;

  // block in which the node was created in
  // gonna be NULL in some cases
  struct Node *block;
};

struct Node *declaration(Type, char *, struct Node *, struct Node *);
struct Node *assignment(char *, struct Node *, struct Node *);
//struct Node *block(struct Node *, struct Node *);
struct Node *emptyblock(struct Node *);
       void  blockappend(struct Node *, struct Node *);
struct Node *statement(struct Node *, struct Node *);
struct Node *binaryop(struct Node *, struct Node *, char);
struct Node *call(char *, struct Node *);
struct Node *whilst(struct Node *, struct Node *);
struct Node *an(struct Node *, struct Node *);
struct Node *expByNum(int);
struct Node *expByName(char *, struct Node *);

#endif // NODES_GEN_H
