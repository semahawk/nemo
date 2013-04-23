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
#include "debug.h"

static const char *binopToS(BinaryOp op);
static const char *unopToS(UnaryOp op);

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
  freeBlockNode,
  freeCallNode,
  freeFuncDefNode
};

/*
 * @name - freeDispatch
 * @desc - runs an appropriate function, that will free given <n>
 */
void freeDispatch(Nemo *NM, Node *n)
{
  assert(n);

  /* watch out for NOPs */
  if (n)
    freeFuncs[n->type](NM, n);
}

/*
 * @name - freeBlock
 * @desc - frees given block and every statement it holds
 */
void freeBlockNode(Nemo *NM, Node *n)
{
  Statement *s;

  assert(n);
  assert(n->type == NT_BLOCK);

  for (s = n->data.block.tail; s != NULL; s = s->next){
    freeDispatch(NM, s->stmt);
    debugAST(NM, n, "free statement node");
    nmFree(NM, s);
  }

  debugAST(NM, n, "free block node");
  nmFree(NM, n);
}

/*
 * @name - genNopNode
 * @desc - create a n that does NOTHING
 */
Node *genNopNode(Nemo *NM)
{
  Node *n = nmMalloc(NM, sizeof(Node));

  n->type = NT_NOP;

  debugAST(NM, n, "create NOP node");

  return n;
}

/*
 * @name - freeNopNode
 * @desc - frees the NOP n
 */
void freeNopNode(Nemo *NM, Node *n)
{
  assert(n);
  assert(n->type == NT_NOP);

  debugAST(NM, n, "free NOP node");

  nmFree(NM, n);
}

/*
 * @name - genIntNode
 * @desc - create a n holding a single literal integer
 */
Node *genIntNode(Nemo *NM, int i)
{
  Node *n = nmMalloc(NM, sizeof(Node));

  n->type = NT_INTEGER;
  n->data.i = i;

  debugAST(NM, n, "create int node (value: %d)", i);

  return n;
}

/*
 * @name - freeIntNode
 * @desc - responsible for freeing literal integer ns
 */
void freeIntNode(Nemo *NM, Node *n)
{
  assert(n);
  assert(n->type == NT_INTEGER);

  debugAST(NM, n, "free int node");

  nmFree(NM, n);
}

/*
 * @name - genFloatNode
 * @desc - create a n holding a single literal float
 */
Node *genFloatNode(Nemo *NM, float f)
{
  Node *n = nmMalloc(NM, sizeof(Node));

  n->type = NT_FLOAT;
  n->data.f = f;

  debugAST(NM, n, "create float node (value: %f)", f);

  return n;
}

/*
 * @name - freeFloatNode
 * @desc - responsible for freeing literal float ns
 */
void freeFloatNode(Nemo *NM, Node *n)
{
  assert(n);
  assert(n->type == NT_FLOAT);

  debugAST(NM, n, "free float node");

  nmFree(NM, n);
}

/*
 * @name - genNameNode
 * @desc - creates a (eg. variable) name n
 */
Node *genNameNode(Nemo *NM, char *s)
{
  Node *n = nmMalloc(NM, sizeof(Node));

  n->type = NT_NAME;
  n->data.s = strdup(NM, s);

  debugAST(NM, n, "create name node (name: %s)", s);

  return n;
}

/*
 * @name - freeNameNode
 * @desc - responsible for freeing (eg. variable) name ns
 */
void freeNameNode(Nemo *NM, Node *n)
{
  assert(n);
  assert(n->type == NT_NAME);

  debugAST(NM, n, "free name node");
  nmFree(NM, n);
  nmFree(NM, n->data.s);
}

/*
 * @name - genBinopNode
 * @desc - creates a binary operation n
 */
Node *genBinopNode(Nemo *NM, Node *left, BinaryOp op, Node *right)
{
  Node *n = nmMalloc(NM, sizeof(Node));

  n->type = NT_BINOP;
  n->data.binop.op = op;
  n->data.binop.left = left;
  n->data.binop.right = right;

  debugAST(NM, n, "create binary operation node (op: %s, left: %p, right: %p)", binopToS(op), (void*)left, (void*)right);

  return n;
}

/*
 * @name - freeBinopNode
 * @desc - responsible for freeing binary operation ns
 */
void freeBinopNode(Nemo *NM, Node *n)
{
  assert(n);
  assert(n->type == NT_BINOP);

  freeDispatch(NM, n->data.binop.left);
  freeDispatch(NM, n->data.binop.right);

  debugAST(NM, n, "free binary operation node");
  nmFree(NM, n);
}

/*
 * @name - genUnopNode
 * @desc - creates a n for unary operation
 */
Node *genUnopNode(Nemo *NM, Node *expr, UnaryOp op)
{
  Node *n = nmMalloc(NM, sizeof(Node));

  n->type = NT_UNOP;
  n->data.unop.op = op;
  n->data.unop.expr = expr;

  debugAST(NM, n, "create unary operation node (op: %s, expr: %p)", unopToS(op), (void*)expr);

  return n;
}

/*
 * @name - freeUnopNode
 * @desc - responsible for freeing unary operation ns
 */
void freeUnopNode(Nemo *NM, Node *n)
{
  assert(n);
  assert(n->type == NT_UNOP);

  freeDispatch(NM, n->data.unop.expr);

  debugAST(NM, n, "free unary operation node");
  nmFree(NM, n);
}

/*
 * @name - genIfNode
 * @desc - creates a n for the if statement
 *         <guard>, <body> and <elsee> can be NULL, it means a NOP then
 */
Node *genIfNode(Nemo *NM, Node *guard, Node *body, Node *elsee)
{
  Node *n = nmMalloc(NM, sizeof(Node));

  n->type = NT_IF;
  n->data.iff.guard = guard;
  n->data.iff.body = body;
  n->data.iff.elsee = elsee;

  debugAST(NM, n, "create if node (guard: %p, body: %p, else: %p)", (void*)guard, (void*)body, (void*)elsee);

  return n;
}

/*
 * @name - freeIfNode
 * @desc - responsible for freeing if ns, and optionally, if present, it's
 *         guarding statement and it's body, and the else statement connected
 *         with it
 */
void freeIfNode(Nemo *NM, Node *n)
{
  assert(n);
  assert(n->type == NT_IF);

  /* guard in if is optional */
  if (n->data.iff.guard)
    freeDispatch(NM, n->data.iff.guard);
  /* so is it's body */
  if (n->data.iff.body)
    freeDispatch(NM, n->data.iff.body);
  /* aaaaaand the else */
  if (n->data.iff.elsee)
    freeDispatch(NM, n->data.iff.elsee);

  debugAST(NM, n, "free if node");
  nmFree(NM, n);
}

/*
 * @name - genWhileNode
 * @desc - creates a while n
 *         <guard>, <body> and <elsee> are optional, can be NULL,
 *         it means then that they are NOPs
 *         <elsee> here gets evaluated when the while loop didn't run even once
 */
Node *genWhileNode(Nemo *NM, Node *guard, Node *body, Node *elsee)
{
  Node *n = nmMalloc(NM, sizeof(Node));

  n->type = NT_WHILE;
  n->data.whilee.guard = guard;
  n->data.whilee.body = body;
  n->data.whilee.elsee = elsee;

  debugAST(NM, n, "create while node (guard: %p, body: %p, else: %p)", (void*)guard, (void*)body, (void*)elsee);

  return n;
}

/*
 * @name - freeWhileNode
 * @desc - responsible for freeing while n, and optionally
 *         guarding statement and it's body, as they are optional
 */
void freeWhileNode(Nemo *NM, Node *n)
{
  assert(n);
  assert(n->type == NT_WHILE);

  /* guard in while is optional */
  if (n->data.whilee.guard)
    freeDispatch(NM, n->data.whilee.guard);
  /* so is it's body */
  if (n->data.whilee.body)
    freeDispatch(NM, n->data.whilee.body);
  /* aaaaaand the else */
  if (n->data.whilee.elsee)
    freeDispatch(NM, n->data.whilee.elsee);

  debugAST(NM, n, "free while node");
  nmFree(NM, n);
}

/*
 * @name - genDeclNode
 * @desc - creates a n for declaring a variable of given <name>
 *         parameter <value> is optional, may be NULL, then it means
 *         something like:
 *
 *           my variable;
 */
Node *genDeclNode(Nemo *NM, char *name, Node *value)
{
  Node *n = nmMalloc(NM, sizeof(Node));

  n->type = NT_DECL;
  n->data.decl.name = strdup(NM, name);
  n->data.decl.value = value;

  debugAST(NM, n, "create declaration node (name: %s)", name);

  return n;
}

/*
 * @name - freeDeclNode
 * @desc - responsible for freeing declaration n
 *         and optionally a init value it was holding
 */
void freeDeclNode(Nemo *NM, Node *n)
{
  assert(n);
  assert(n->type == NT_DECL);

  nmFree(NM, n->data.decl.name);
  /* initialized value is optional */
  if (n->data.decl.value)
    freeDispatch(NM, n->data.decl.value);

  debugAST(NM, n, "free declaration node");
  nmFree(NM, n);
}

/*
 * @name - genCallNode
 * @desc - creates a n for calling a function of a given <name>
 *         parameter <params> is optional, may be NULL, then it means
 *         that no parameters have been passed
 */
Node *genCallNode(Nemo *NM, char *name, Node **params)
{
  Node *n = nmMalloc(NM, sizeof(Node));

  n->type = NT_CALL;
  n->data.call.name = strdup(NM, name);
  n->data.call.params = params;

  debugAST(NM, n, "create call node (name: %s, params: %p)", name, params);

  return n;
}

/*
 * @name - freeCallNode
 * @desc - responsible for freeing call n and optionally any params it had
 */
void freeCallNode(Nemo *NM, Node *n)
{
  unsigned i;

  assert(n);
  assert(n->type == NT_CALL);

  nmFree(NM, n->data.call.name);
  /* parameters are optional */
  if (n->data.call.params){
    for (i = 0; n->data.call.params[i] != NULL; i++){
      freeDispatch(NM, n->data.call.params[i]);
    }
    debugAST(NM, n->data.call.params, "free params list");
    nmFree(NM, n->data.call.params);
  }

  debugAST(NM, n, "free call node");
  nmFree(NM, n);
}

/*
 * @name - genFuncDefNode
 * @desc - creates a node for defining a function of a given <name>
 */
Node *genFuncDefNode(Nemo *NM, char *name, Node *body)
{
  Node *n = nmMalloc(NM, sizeof(Node));

  n->type = NT_FUNCDEF;
  n->data.funcdef.name = strdup(NM, name);
  n->data.funcdef.body = body;

  debugAST(NM, n, "create function definition node (name: %s, body: %p)", name, (void*)body);

  return n;
}

/*
 * @name - freeFuncDefNode
 * @desc - responsible for freeing call n and optionally any params it had
 */
void freeFuncDefNode(Nemo *NM, Node *n)
{
  assert(n);
  assert(n->type == NT_FUNCDEF);

  nmFree(NM, n->data.funcdef.name);
  freeDispatch(NM, n->data.funcdef.body);
  debugAST(NM, n, "free function definition node");
  nmFree(NM, n);
}

static const char *binopToS(BinaryOp op)
{
  switch (op){
    case BINARY_GT:         return "'>'";
    case BINARY_LT:         return "'<'";
    case BINARY_ADD:        return "'+'";
    case BINARY_SUB:        return "'-'";
    case BINARY_MUL:        return "'*'";
    case BINARY_DIV:        return "'/'";
    case BINARY_MOD:        return "'%'";
    case BINARY_ASSIGN_ADD: return "'+='";
    case BINARY_ASSIGN_SUB: return "'-='";
    case BINARY_ASSIGN_MUL: return "'*='";
    case BINARY_ASSIGN_DIV: return "'/='";
    case BINARY_ASSIGN_MOD: return "'%='";
  }

  return "#unknown#binopToS#";
}

static const char *unopToS(UnaryOp op)
{
  switch (op){
    case UNARY_PLUS:    return "'+'";
    case UNARY_MINUS:   return "'-'";
    case UNARY_NEGATE:  return "'!'";
    case UNARY_PREINC:  return "prefix '++'";
    case UNARY_PREDEC:  return "prefix '--'";
    case UNARY_POSTINC: return "postfix '++'";
    case UNARY_POSTDEC: return "postfix '--'";
  }

  return "#unknown#unopToS#";
}

/*
 * Steve Vai
 *
 * The IT Crowd
 */
