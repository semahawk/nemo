//
// nodes.c
//
// Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
//

#include "nemo.h"
#include "handy.h"
#include "nodes_gen.h"

struct Node *assignment(char *name, struct Node *val)
{
  struct Node *new = myalloc(sizeof(struct Node));

  new->kind = nt_ASSIGNMENT;
  new->data.assignment.name = name;
  new->data.assignment.right = val;

  return new;
}

struct Node *expByNum(int val)
{
  struct Node *new = myalloc(sizeof(struct Node));

  new->kind = nt_INTEGER;
  new->data.i = val;

  return new;
}

struct Node *expByName(char *name)
{
  struct Node *new = myalloc(sizeof(struct Node));

  new->kind = nt_ID;
  new->data.s = name;

  return new;
}

struct Node *expression(struct Node *left, struct Node *right, char op)
{
  struct Node *new = myalloc(sizeof(struct Node));

  new->kind = nt_BINARYOP;
  new->data.expression.left = left;
  new->data.expression.right = right;
  new->data.expression.op = op;

  return new;
}

struct Node *statement(struct Node *res, struct Node *toappend)
{
  if (!res){
    res = myalloc(sizeof(struct Node));

    res->kind = nt_STATEMENTS;
    res->data.statements.count = 0;
    res->data.statements.statements = 0;
  }

  assert(nt_STATEMENTS == res->kind);
  res->data.statements.count++;
  res->data.statements.statements = realloc(res->data.statements.statements, res->data.statements.count * sizeof(*res->data.statements.statements));
  res->data.statements.statements[res->data.statements.count - 1] = toappend;

  return res;
}

struct Node *call(char *name, struct Node *param)
{
  struct Node *new = myalloc(sizeof(struct Node));

  new->kind = nt_CALL;
  new->data.call.name = name;
  new->data.call.param = param;

  return new;
}
