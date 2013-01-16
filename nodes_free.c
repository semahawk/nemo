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
  freeTermExpression,
  freeBinExpression,
  freeUnExpression,
  /*freeDeclaration,*/
  freeAssignment,
  freeBlock,
  freeStatement,
  freeCall,
  freeReturn,
  freeWhile,
  freeIf,
  freeFor,
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

  debug("freeing id/constant node at %p", n);

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

  if (n->data.call.params)
    for (struct ParamList *p = n->data.call.params; p != NULL; p = p->next)
      freeNode(p->param);

  free(n);
}

void freeReturn(struct Node *n)
{
  assert(n);
  assert(nt_RETURN == n->kind);

  debug("freeing return node at %p", n);

  if (n->data.returnn.expr)
    freeNode(n->data.returnn.expr);

  free(n);
}

void freeWhile(struct Node *n)
{
  assert(n);
  assert(nt_WHILE == n->kind);

  debug("freeing while node at %p", n);

  freeNode(n->data.whilee.cond);
  freeNode(n->data.whilee.statements);

  free(n);
}

void freeIf(struct Node *n)
{
  assert(n);
  assert(nt_IF == n->kind);

  debug("freeing if node at %p", n);

  freeNode(n->data.iff.cond);
  freeNode(n->data.iff.stmt);

  if (n->data.iff.elsestmt)
    freeNode(n->data.iff.elsestmt);

  free(n);
}

void freeFor(struct Node *n)
{
  assert(n);
  assert(nt_FOR == n->kind);

  debug("freeing for node at %p", n);

  if (n->data.forr.init)
    freeNode(n->data.forr.init);

  if (n->data.forr.cond)
    freeNode(n->data.forr.cond);

  if (n->data.forr.action)
    freeNode(n->data.forr.action);

  if (n->data.forr.stmt)
    freeNode(n->data.forr.stmt);

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

  if (n->data.funcdef.args)
    for (struct ArgList *a = n->data.funcdef.args; a != NULL; a = a->next)
      free(a->arg);

  freeNode(n->data.funcdef.body);

  free(n);
}

