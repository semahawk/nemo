/*
 * nodes_free.c
 *
 * Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
 *
 */

#include "nemo.h"
#include "handy.h"
#include "nodes_gen.h"
#include "nodes_free.h"

void(*nodeFrees[])(struct Node *) =
{
  freeTermExpression,
  freeTermExpression,
  freeBinExpression,
  freeUnExpression,
  freeDeclaration,
  freeAssignment,
  freeBlock,
  freeStatement,
  freeCall,
  freeWhile,
  freeIf,
  freeFuncDef
};

void freeNode(struct Node *n)
{
  assert(n);
  assert(nodeFrees[n->kind]);

  nodeFrees[n->kind](n);
  n = NULL;
}

void freeNodes(struct Node *nodest)
{
  assert(nodest);
  assert(nt_BLOCK == nodest->kind);

  freeNode(nodest);

  nodest = NULL;
}

void freeTermExpression(struct Node *n)
{
  assert(n);

  debug("freeing id/integer node at %p", n);

  free(n);
}

void freeBinExpression(struct Node *n)
{
  assert(n);
  assert(nt_BINARYOP == n->kind);

  debug("freeing binary operation node at %p", n);

  freeNode(n->data.binaryop.left);
  freeNode(n->data.binaryop.right);

  free(n);
}

void freeUnExpression(struct Node *n)
{
  assert(n);
  assert(nt_UNARYOP == n->kind);

  debug("freeing unary operation node at %p", n);

  freeNode(n->data.unaryop.expression);

  free(n);
}

void freeDeclaration(struct Node *n)
{
  assert(n);
  assert(nt_DECLARATION == n->kind);

  debug("freeing declaration node at %p", n);

  if (n->data.declaration.right)
    freeNode(n->data.declaration.right);

  free(n);
}

void freeAssignment(struct Node *n)
{
  assert(n);
  assert(nt_ASSIGNMENT == n->kind);

  debug("freeing assignment node at %p", n);

  freeNode(n->data.assignment.right);

  free(n);
}

void freeCall(struct Node *n)
{
  assert(n);
  assert(nt_CALL == n->kind);

  debug("freeing call node at %p", n);

  freeNode(n->data.call.param);

  free(n);
}

void freeWhile(struct Node *n)
{
  assert(n);
  assert(nt_WHILE == n->kind);

  debug("freeing whilst node at %p", n);

  freeNode(n->data.whilee.cond);
  freeNode(n->data.whilee.statements);

  free(n);
}

void freeIf(struct Node *n)
{
  assert(n);
  assert(nt_IF == n->kind);

  debug("freeing an node at %p", n);

  freeNode(n->data.iff.cond);
  freeNode(n->data.iff.statements);

  free(n);
}

void freeBlock(struct Node *n)
{
  assert(n);
  assert(nt_BLOCK == n->kind);

  debug("freeing block node at %p", n);

  for (int i = 0; i < n->data.block.count; i++){
    freeNode(n->data.block.statements[i]);
  }

  free(n);
}

void freeStatement(struct Node *n)
{
  assert(n);
  assert(nt_STATEMENT == n->kind);

  debug("freeing statement node at %p", n);

  for (int i = 0; i < n->data.statement.count; i++){
    freeNode(n->data.statement.nodes[i]);
  }

  free(n);
}

void freeFuncDef(struct Node *n)
{
  assert(n);
  assert(nt_FUNCDEF == n->kind);

  debug("freeing funcion declaration node at %p", n);

  freeNode(n->data.funcdef.body);

  free(n);
}

