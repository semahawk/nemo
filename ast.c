/*
 *
 * ast.c
 *
 * Created at:  Wed Apr 17 20:11:54 2013 20:11:54
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License: the MIT license
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

/*
 * "Come sing along with the pirate song
 *  Hail to the wind, hooray to the glory
 *  We're gonna fight 'til the battle's won
 *  On the raging sea"
 *
 *  Running Wild - Pirate Song
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "nemo.h"
#include "ast.h"
#include "mem.h"

/*
 * Array of pointer functions responsible for freeing an adequate kind of node
 */
static void (*freeFuncs[])(Nemo *, Node *) =
{
  /* XXX order must match the one in "enum NodeType" in ast.h */
  freeNopNode,
  freeIntNode,
  freeFloatNode,
  freeNameNode,
  freeBinopNode,
  freeUnopNode,
  freeIfNode,
  freeWhileNode,
  freeDeclNode,
  freeBlockNode
};

/*
 * @name - freeDispatch
 * @desc - runs an appropriate function, that will free given <node>
 */
void freeDispatch(Nemo *NM, Node *node)
{
  /* watch out for NOPs */
  if (node)
    freeFuncs[node->type](NM, node);
}

/*
 * @name - freeBlock
 * @desc - frees given block and every statement it holds
 */
void freeBlockNode(Nemo *NM, Node *node)
{
  Statement *s;

  assert(node);
  assert(node->type == NT_BLOCK);

  for (s = node->data.block.tail; s != NULL; s = s->next){
    freeDispatch(NM, s->stmt);
    nmFree(NM, s);
  }

  nmFree(NM, node);
}

/*
 * @name - genNopNode
 * @desc - create a node that does NOTHING
 */
Node *genNopNode(Nemo *NM)
{
  Node *new = nmMalloc(NM, sizeof(Node));

  new->type = NT_NOP;

  return new;
}

/*
 * @name - freeNopNode
 * @desc - frees the NOP node
 */
void freeNopNode(Nemo *NM, Node *node)
{
  assert(node);
  assert(node->type == NT_NOP);

  nmFree(NM, node);
}

/*
 * @name - genIntNode
 * @desc - create a node holding a single literal integer
 */
Node *genIntNode(Nemo *NM, int i)
{
  Node *new = nmMalloc(NM, sizeof(Node));

  new->type = NT_INTEGER;
  new->data.i = i;

  return new;
}

/*
 * @name - freeIntNode
 * @desc - responsible for freeing literal integer nodes
 */
void freeIntNode(Nemo *NM, Node *node)
{
  assert(node);
  assert(node->type == NT_INTEGER);

  nmFree(NM, node);
}

/*
 * @name - genFloatNode
 * @desc - create a node holding a single literal float
 */
Node *genFloatNode(Nemo *NM, float f)
{
  Node *new = nmMalloc(NM, sizeof(Node));

  new->type = NT_FLOAT;
  new->data.f = f;

  return new;
}

/*
 * @name - freeFloatNode
 * @desc - responsible for freeing literal float nodes
 */
void freeFloatNode(Nemo *NM, Node *node)
{
  assert(node);
  assert(node->type == NT_FLOAT);

  nmFree(NM, node);
}

/*
 * @name - genNameNode
 * @desc - creates a (eg. variable) name node
 */
Node *genNameNode(Nemo *NM, char *s)
{
  Node *new = nmMalloc(NM, sizeof(Node));

  new->type = NT_NAME;
  new->data.s = strdup(NM, s);

  return new;
}

/*
 * @name - freeNameNode
 * @desc - responsible for freeing (eg. variable) name nodes
 */
void freeNameNode(Nemo *NM, Node *node)
{
  assert(node);
  assert(node->type == NT_NAME);

  nmFree(NM, node);
  nmFree(NM, node->data.s);
}

/*
 * @name - genBinopNode
 * @desc - creates a binary operation node
 */
Node *genBinopNode(Nemo *NM, Node *left, BinaryOp op, Node *right)
{
  Node *new = nmMalloc(NM, sizeof(Node));

  new->type = NT_BINOP;
  new->data.binop.op = op;
  new->data.binop.left = left;
  new->data.binop.right = right;

  return new;
}

/*
 * @name - freeBinopNode
 * @desc - responsible for freeing binary operation nodes
 */
void freeBinopNode(Nemo *NM, Node *node)
{
  assert(node);
  assert(node->type == NT_BINOP);

  freeDispatch(NM, node->data.binop.left);
  freeDispatch(NM, node->data.binop.right);

  nmFree(NM, node);
}

/*
 * @name - genUnopNode
 * @desc - creates a node for unary operation
 */
Node *genUnopNode(Nemo *NM, Node *expr, UnaryOp op)
{
  Node *new = nmMalloc(NM, sizeof(Node));

  new->type = NT_UNOP;
  new->data.unop.op = op;
  new->data.unop.expr = expr;

  return new;
}

/*
 * @name - freeUnopNode
 * @desc - responsible for freeing unary operation nodes
 */
void freeUnopNode(Nemo *NM, Node *node)
{
  assert(node);
  assert(node->type == NT_UNOP);

  freeDispatch(NM, node->data.unop.expr);

  nmFree(NM, node);
}

/*
 * @name - genIfNode
 * @desc - creates a node for the if statement
 *         <guard>, <body> and <elsee> can be NULL, it means a NOP then
 */
Node *genIfNode(Nemo *NM, Node *guard, Node *body, Node *elsee)
{
  Node *new = nmMalloc(NM, sizeof(Node));

  new->type = NT_IF;
  new->data.iff.guard = guard;
  new->data.iff.body = body;
  new->data.iff.elsee = elsee;

  return new;
}

/*
 * @name - freeIfNode
 * @desc - responsible for freeing if nodes, and optionally, if present, it's
 *         guarding statement and it's body, and the else statement connected
 *         with it
 */
void freeIfNode(Nemo *NM, Node *node)
{
  assert(node);
  assert(node->type == NT_IF);

  /* guard in if is optional */
  if (node->data.iff.guard)
    freeDispatch(NM, node->data.iff.guard);
  /* so is it's body */
  if (node->data.iff.body)
    freeDispatch(NM, node->data.iff.body);
  /* aaaaaand the else */
  if (node->data.iff.elsee)
    freeDispatch(NM, node->data.iff.elsee);

  nmFree(NM, node);
}

/*
 * @name - genWhileNode
 * @desc - creates a while node
 *         <guard>, <body> and <elsee> are optional, can be NULL,
 *         it means then that they are NOPs
 *         <elsee> here gets evaluated when the while loop didn't run even once
 */
Node *genWhileNode(Nemo *NM, Node *guard, Node *body, Node *elsee)
{
  Node *new = nmMalloc(NM, sizeof(Node));

  new->type = NT_WHILE;
  new->data.whilee.guard = guard;
  new->data.whilee.body = body;
  new->data.whilee.elsee = elsee;

  return new;
}

/*
 * @name - freeWhileNode
 * @desc - responsible for freeing while node, and optionally
 *         guarding statement and it's body, as they are optional
 */
void freeWhileNode(Nemo *NM, Node *node)
{
  assert(node);
  assert(node->type == NT_WHILE);

  /* guard in while is optional */
  if (node->data.whilee.guard)
    freeDispatch(NM, node->data.whilee.guard);
  /* so is it's body */
  if (node->data.whilee.body)
    freeDispatch(NM, node->data.whilee.body);
  /* aaaaaand the else */
  if (node->data.whilee.elsee)
    freeDispatch(NM, node->data.whilee.elsee);

  nmFree(NM, node);
}

/*
 * @name - genDeclNode
 * @desc - creates a node for declaring a variable of given <name>
 *         parameter <value> is optional, may be NULL, then it means
 *         something like:
 *
 *           my variable;
 */
Node *genDeclNode(Nemo *NM, char *name, Node *value)
{
  Node *new = nmMalloc(NM, sizeof(Node));

  new->type = NT_DECL;
  new->data.decl.name = strdup(NM, name);
  new->data.decl.value = value;

  return new;
}

/*
 * @name - freeDeclNode
 * @desc - responsible for freeing declaration node
 *         and optionally a init value it was holding
 */
void freeDeclNode(Nemo *NM, Node *node)
{
  assert(node);
  assert(node->type == NT_DECL);

  nmFree(NM, node->data.decl.name);
  /* initialized value is optional */
  if (node->data.decl.value)
    freeDispatch(NM, node->data.decl.value);

  nmFree(NM, node);
}

/*
 * Steve Vai
 */
