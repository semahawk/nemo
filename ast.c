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

#include <stdio.h>
#include <stdlib.h>

#include "nemo.h"
#include "ast.h"
#include "mem.h"

static void (*freeFuncs[])(Nemo *, Node *) =
{
  /* XXX order must match the one in "enum NodeType" in ast.h */
  freeIntNode,
  freeFloatNode,
  freeNameNode,
  freeBinopNode,
  freeUnopNode,
  freeIfNode,
  freeWhileNode,
  freeDeclNode
};

void freeDispatch(Nemo *NM, Node *node)
{
  freeFuncs[node->type](NM, node);
}

Node *genIntNode(Nemo *NM, int i)
{
  Node *new = nmMalloc(NM, sizeof(Node));

  new->type = NT_INTEGER;
  new->data.i = i;

  return new;
}

void freeIntNode(Nemo *NM, Node *node)
{
  nmFree(NM, node);
}

Node *genFloatNode(Nemo *NM, float f)
{
  Node *new = nmMalloc(NM, sizeof(Node));

  new->type = NT_FLOAT;
  new->data.f = f;

  return new;
}

void freeFloatNode(Nemo *NM, Node *node)
{
  nmFree(NM, node);
}

Node *genNameNode(Nemo *NM, char *s)
{
  Node *new = nmMalloc(NM, sizeof(Node));

  new->type = NT_NAME;
  new->data.s = strdup(NM, s);

  return new;
}

void freeNameNode(Nemo *NM, Node *node)
{
  nmFree(NM, node);
  nmFree(NM, node->data.s);
}

Node *genBinopNode(Nemo *NM, Node *left, BinaryOp op, Node *right)
{
  Node *new = nmMalloc(NM, sizeof(Node));

  new->type = NT_BINOP;
  new->data.binop.op = op;
  new->data.binop.left = left;
  new->data.binop.right = right;

  return new;
}

void freeBinopNode(Nemo *NM, Node *node)
{
  freeDispatch(NM, node->data.binop.left);
  freeDispatch(NM, node->data.binop.right);

  nmFree(NM, node);
}

Node *genUnopNode(Nemo *NM, Node *expr, UnaryOp op)
{
  Node *new = nmMalloc(NM, sizeof(Node));

  new->type = NT_UNOP;
  new->data.unop.op = op;
  new->data.unop.expr = expr;

  return new;
}

void freeUnopNode(Nemo *NM, Node *node)
{
  freeDispatch(NM, node->data.unop.expr);

  nmFree(NM, node);
}

Node *genIfNode(Nemo *NM, Node *guard, Node *body)
{
  Node *new = nmMalloc(NM, sizeof(Node));

  new->type = NT_IF;
  new->data.iff.guard = guard;
  new->data.iff.body = body;

  return new;
}

void freeIfNode(Nemo *NM, Node *node)
{
  /* guard in if is optional */
  if (node->data.iff.guard)
    freeDispatch(NM, node->data.iff.guard);
  /* so is it's body */
  if (node->data.iff.body)
    freeDispatch(NM, node->data.iff.body);

  nmFree(NM, node);
}

Node *genWhileNode(Nemo *NM, Node *guard, Node *body)
{
  Node *new = nmMalloc(NM, sizeof(Node));

  new->type = NT_WHILE;
  new->data.whilee.guard = guard;
  new->data.whilee.body = body;

  return new;
}

void freeWhileNode(Nemo *NM, Node *node)
{
  /* guard in while is optional */
  if (node->data.whilee.guard)
    freeDispatch(NM, node->data.whilee.guard);
  /* so is it's body */
  if (node->data.whilee.body)
    freeDispatch(NM, node->data.whilee.body);

  nmFree(NM, node);
}

Node *genDeclNode(Nemo *NM, char *name, Node *value)
{
  Node *new = nmMalloc(NM, sizeof(Node));

  new->type = NT_DECL;
  new->data.decl.name = name;
  new->data.decl.value = value;

  return new;
}

void freeDeclNode(Nemo *NM, Node *node)
{
  /* guard in while is optional */
  if (node->data.whilee.guard)
    freeDispatch(NM, node->data.whilee.guard);
  /* so is it's body */
  if (node->data.whilee.body)
    freeDispatch(NM, node->data.whilee.body);

  nmFree(NM, node);
}

