//
// nodes.h
//
// Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
//

#ifndef NODES_GEN_H
#define NODES_GEN_H

#include "nemo.h"

typedef enum {
  TYPE_INTEGER
} var_t;

struct Node {
  enum {
    nt_ID,
    nt_INTEGER,
    nt_BINARYOP,
    nt_DECLARATION,
    nt_ASSIGNMENT,
    nt_STATEMENTS,
    nt_CALL,
    nt_WHILST,
    nt_AN,
    nt_LASTELEMENT
  } kind;

  union {
    int i;
    char *s;

    struct {
      struct Node *left, *right;
      char op;
    } expression;

    struct {
      var_t type;
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
      struct Node **statements;
    } statements;

    struct {
      struct Node *cond;
      struct Node *statements;
    } whilst;

    struct {
      char *name;
      struct Node *param;
    } call;

    struct {
      struct Node *cond;
      struct Node *statements;
    } an;
  } data;
};

struct Node *declaration(var_t, char *, struct Node *);
struct Node *assignment(char *, struct Node *);
struct Node *statement(struct Node *, struct Node *);
struct Node *expression(struct Node *, struct Node *, char);
struct Node *call(char *, struct Node *);
struct Node *whilst(struct Node *, struct Node *);
struct Node *an(struct Node *, struct Node *);
struct Node *expByNum(int);
struct Node *expByName(char *);

#endif // NODES_GEN_H
