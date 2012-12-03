//
// nodes.c
//
// Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
//

#include "nemo.h"
#include "handy.h"
#include "nodes_gen.h"

struct Node *genDeclaration(Type type, char *name, struct Node *val, struct Node *block)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("creating declaration node <type: %d, name: %s> at %p", type, name, new);

  new->kind = nt_DECLARATION;
  new->data.declaration.type = type;
  new->data.declaration.name = name;
  new->data.declaration.right = val;
  new->block = block;

  return new;
}

struct Node *genAssignment(char *name, struct Node *val, struct Node *block)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("creating assignment node <name: %s> at %p", name, new);

  new->kind = nt_ASSIGNMENT;
  new->data.assignment.name = name;
  new->data.assignment.right = val;
  new->block = block;

  return new;
}

struct Node *genExpByNum(int val)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("creating integer node <val: %d> at %p", val, new);

  new->kind = nt_INTEGER;
  new->data.value.i = val;
  new->block = NULL;

  return new;
}

struct Node *genExpByName(char *name, struct Node *block)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("creating identifier node <name: %s> at %p", name, new);

  new->kind = nt_ID;
  new->data.s = name;
  new->block = block;

  return new;
}

struct Node *genBinaryop(struct Node *left, struct Node *right, char op)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("creating binary operation node <op: '%c'> at %p", op, new);

  new->kind = nt_BINARYOP;
  new->data.binaryop.left = left;
  new->data.binaryop.right = right;
  new->data.binaryop.op = op;
  new->block = NULL;

  return new;
}

struct Node *genUnaryop(struct Node *left, Unary op, struct Node *currentblock)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("creating unary operation node <op: '%d'> at %p", op, new);

  if (left->kind != nt_ID){
    cerror("trying to change value of a constant object");
    exit(1);
  }

  new->kind = nt_UNARYOP;
  new->data.unaryop.expression = left;
  new->data.unaryop.op = op;
  new->block = currentblock;

  return new;
}

struct Node *genEmptyBlock(struct Node *parent)
{
  struct Node *new = myalloc(sizeof(struct Node));

  new->kind = nt_BLOCK;
  new->data.block.count = 0;
  new->data.block.statements = 0;
  new->data.block.vars = NULL;
  new->data.block.parent = parent;
  new->block = NULL;

  debug("creating empty block node at %p with parent at %p", new, parent);

  return new;
}

void blockappend(struct Node *currentblock, struct Node *toappend)
{
  currentblock->data.block.count++;
  currentblock->data.block.statements = realloc(currentblock->data.block.statements, currentblock->data.block.count * sizeof(*currentblock->data.block.statements));
  currentblock->data.block.statements[currentblock->data.block.count - 1] = toappend;

  debug("appending statement (%p) to block node (%p)", toappend, currentblock);
}

struct Node *genStatement(struct Node *new, struct Node *toappend)
{
  if (!new){
    new = myalloc(sizeof(struct Node));

    new->kind = nt_STATEMENT;
    new->data.statement.count = 0;
    new->data.statement.nodes = 0;
  }

  debug("creating statement node at %p", new);
  assert(nt_STATEMENT == new->kind);

  new->data.statement.count++;
  new->data.statement.nodes = realloc(new->data.statement.nodes, new->data.statement.count * sizeof(*new->data.statement.nodes));
  new->data.statement.nodes[new->data.statement.count - 1] = toappend;

  return new;
}

struct Node *genWhile(struct Node *cond, struct Node *stmt)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("creating whilst node at %p", new);

  new->kind = nt_WHILE;
  new->data.whilee.cond = cond;
  new->data.whilee.statements = stmt;
  new->block = NULL;

  return new;
}

struct Node *genIf(struct Node *cond, struct Node *stmt)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("creating an node at %p", new);

  new->kind = nt_IF;
  new->data.iff.cond = cond;
  new->data.iff.statements = stmt;
  new->block = NULL;

  return new;
}

struct Node *genFuncDef(Type returntype, char *name, struct Node *body)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("creating function definiton <name: %s> at %p", name, new);

  new->kind = nt_FUNCDEF;
  new->data.funcdef.returntype = returntype;
  new->data.funcdef.name = name;
  new->data.funcdef.body = body;

  return new;
}

struct Node *genCall(char *name, struct Node *param)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("creating call node <name: %s> at %p", name, new);

  new->kind = nt_CALL;
  new->data.call.name = name;
  new->data.call.param = param;
  // TODO: it will be needed later on
  new->block = NULL;

  return new;
}

