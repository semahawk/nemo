//
// nodes.c
//
// Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
//

#include "nemo.h"
#include "handy.h"
#include "nodes_gen.h"

struct Node *declaration(Type type, char *name, struct Node *val)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("creating declaration node <type: %d, name: %s> at 0x%x", type, name, new);

  new->kind = nt_DECLARATION;
  new->data.declaration.type = type;
  new->data.declaration.name = name;
  new->data.declaration.right = val;

  return new;
}

struct Node *assignment(char *name, struct Node *val)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("creating assignment node <name: %s> at 0x%x", name, new);

  new->kind = nt_ASSIGNMENT;
  new->data.assignment.name = name;
  new->data.assignment.right = val;

  return new;
}

struct Node *expByNum(int val)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("creating integer node <val: %d> at 0x%x", val, new);

  new->kind = nt_INTEGER;
  new->data.value.i = val;

  return new;
}

struct Node *expByName(char *name)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("creating identifier node <name: %s> at 0x%x", name, new);

  new->kind = nt_ID;
  new->data.s = name;

  return new;
}

struct Node *binaryop(struct Node *left, struct Node *right, char op)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("creating binary operation node <op: %c> at 0x%x", op, new);

  new->kind = nt_BINARYOP;
  new->data.expression.left = left;
  new->data.expression.right = right;
  new->data.expression.op = op;

  return new;
}

struct Node *block(struct Node *new, struct Node *toappend)
{
  if (!new){
    new = myalloc(sizeof(struct Node));

    new->kind = nt_BLOCK;
    new->data.block.count = 0;
    new->data.block.statements = 0;
  }

  debug("creating block node at 0x%x", new);
  assert(nt_BLOCK == new->kind);

  new->data.block.count++;
  new->data.block.statements = realloc(new->data.block.statements, new->data.block.count * sizeof(*new->data.block.statements));
  new->data.block.statements[new->data.block.count - 1] = toappend;

  return new;
}

struct Node *statement(struct Node *new, struct Node *toappend)
{
  if (!new){
    new = myalloc(sizeof(struct Node));

    new->kind = nt_STATEMENT;
    new->data.statement.count = 0;
    new->data.statement.nodes = 0;
  }

  debug("creating statement node at 0x%x", new);
  assert(nt_STATEMENT == new->kind);

  new->data.statement.count++;
  new->data.statement.nodes = realloc(new->data.statement.nodes, new->data.statement.count * sizeof(*new->data.statement.nodes));
  new->data.statement.nodes[new->data.statement.count - 1] = toappend;

  return new;
}

struct Node *whilst(struct Node *cond, struct Node *stmt)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("creating whilst node at 0x%x", new);

  new->kind = nt_WHILST;
  new->data.whilst.cond = cond;
  new->data.whilst.statements = stmt;

  return new;
}

struct Node *an(struct Node *cond, struct Node *stmt)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("creating an node at 0x%x", new);

  new->kind = nt_AN;
  new->data.an.cond = cond;
  new->data.an.statements = stmt;

  return new;
}

struct Node *call(char *name, struct Node *param)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("creating call node <name: %s> at 0x%x", name, new);

  new->kind = nt_CALL;
  new->data.call.name = name;
  new->data.call.param = param;

  return new;
}

