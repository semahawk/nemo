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
#include <string.h>
#include <assert.h>

#include "nemo.h"
#include "ast.h"
#include "mem.h"
#include "debug.h"

static const char *binopToS(BinaryOp);
static const char *unopToS(UnaryOp);
static char *valueToS(Value);

/*
 * Array of pointer functions responsible for executing an adequate kind of node
 */
static Value (*execFuncs[])(Nemo *, Node *) =
{
  /* XXX order must match the one in "enum NodeType" in ast.h */
  NULL,
  execIntNode,
  execFloatNode,
  execNameNode,
  execBinopNode,
  execUnopNode,
  execIfNode,
  execWhileNode,
  execDeclNode,
  execBlockNode,
  execCallNode,
  execFuncDefNode
};

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
 * @name - execNode
 * @desc - executes given node and returns the Value it resulted in
 */
Value execNode(Nemo *NM, Node *n)
{
  assert(n);
  assert(execFuncs[n->type]);

  return execFuncs[n->type](NM, n);
}

/*
 * @name - freeNode
 * @desc - runs an appropriate function, that will free given <n>
 */
void freeNode(Nemo *NM, Node *n)
{
  assert(n);

  /* watch out for NOPs */
  if (n)
    freeFuncs[n->type](NM, n);
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
 * @name - execIntNode
 * @desc - return the value of the int
 */
Value execIntNode(Nemo *NM, Node *n)
{
  Value ret;

  ret.type = OT_INTEGER;
  ret.value.i = n->data.i;

  debugAST(NM, n, "execute integer node");

  return ret;
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
 * @name - execFloatNode
 * @desc - return the value of the float
 */
Value execFloatNode(Nemo *NM, Node *n)
{
  Value ret;

  ret.type = OT_FLOAT;
  ret.value.f = n->data.f;

  debugAST(NM, n, "execute float node");

  return ret;
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
 * @name - execNameNode
 * @desc - return the value the name is carring
 */
Value execNameNode(Nemo *NM, Node *n)
{
  Value ret;

  ret.type = OT_INTEGER;
  ret.value.i = 1337;

  debugAST(NM, n, "execute name node");

  return ret;
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
 * @name - execBinopNode
 * @desc - return the result of the binary operation
 */
Value execBinopNode(Nemo *NM, Node *n)
{
  Value ret;

  ret.type = OT_INTEGER;
  ret.value.i = 0xB14;

  debugAST(NM, n, "execute binary operation node");

  return ret;
}

/*
 * @name - freeBinopNode
 * @desc - responsible for freeing binary operation ns
 */
void freeBinopNode(Nemo *NM, Node *n)
{
  assert(n);
  assert(n->type == NT_BINOP);

  freeNode(NM, n->data.binop.left);
  freeNode(NM, n->data.binop.right);

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
 * @name - execUnopNode
 * @desc - return the result of the unary operation
 */
Value execUnopNode(Nemo *NM, Node *n)
{
  Value ret;

  ret.type = OT_INTEGER;
  ret.value.i = 0x0408;

  debugAST(NM, n, "execute unary operation node");

  return ret;
}

/*
 * @name - freeUnopNode
 * @desc - responsible for freeing unary operation ns
 */
void freeUnopNode(Nemo *NM, Node *n)
{
  assert(n);
  assert(n->type == NT_UNOP);

  freeNode(NM, n->data.unop.expr);

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
 * @name - execIfNode
 * @desc - do the if loop, return actually anything
 */
Value execIfNode(Nemo *NM, Node *n)
{
  Value ret;

  /*
   *Value guard = execNode(NM, n->data.iff.guard);
   *Value body = execNode(NM, n->data.iff.body);
   *Value elsee = execNode(NM, n->data.iff.elsee);
   */

  /* FIXME */

  ret.type = OT_INTEGER;
  ret.value.i = 0x1F;

  debugAST(NM, n, "execute if node");

  return ret;
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
    freeNode(NM, n->data.iff.guard);
  /* so is it's body */
  if (n->data.iff.body)
    freeNode(NM, n->data.iff.body);
  /* aaaaaand the else */
  if (n->data.iff.elsee)
    freeNode(NM, n->data.iff.elsee);

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
 * @name - execWhileNode
 * @desc - execute the loop and return, actually anything, it's a statement
 */
Value execWhileNode(Nemo *NM, Node *n)
{
  Value ret;

  /*
   *Value guard = execNode(NM, n->data.iff.guard);
   *Value body = execNode(NM, n->data.iff.body);
   *Value elsee = execNode(NM, n->data.iff.elsee);
   */

  /* FIXME */

  ret.type = OT_INTEGER;
  ret.value.i = 0x173;

  debugAST(NM, n, "execute while node");

  return ret;
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
    freeNode(NM, n->data.whilee.guard);
  /* so is it's body */
  if (n->data.whilee.body)
    freeNode(NM, n->data.whilee.body);
  /* aaaaaand the else */
  if (n->data.whilee.elsee)
    freeNode(NM, n->data.whilee.elsee);

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
 * @name - execDeclNode
 * @desc - declare/define the variable and return 1
 */
Value execDeclNode(Nemo *NM, Node *n)
{
  Value ret;

  /*char *name = n->data.decl.name;*/

  ret.type = OT_INTEGER;
  ret.value.i = 1;

  if (n->data.decl.value){
    /*Value = execNode(n->data.decl.value);*/
    debugAST(NM, n, "execute variable definition node");
  } else {
    debugAST(NM, n, "execute variable declaration node");
  }

  return ret;
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
    freeNode(NM, n->data.decl.value);

  debugAST(NM, n, "free declaration node");
  nmFree(NM, n);
}

Value execBlockNode(Nemo *NM, Node *n)
{
  Value ret;
  Statement *s;

  assert(n);
  assert(n->type == NT_BLOCK);

  debugAST(NM, n, "execute block node");

  for (s = n->data.block.tail; s != NULL; s = s->next){
    debugAST(NM, s->stmt, "execute statement node");
    ret = execNode(NM, s->stmt);
  }

  debugAST(NM, n, "done executing block node");

  return ret;
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
    freeNode(NM, s->stmt);
    debugAST(NM, n, "free statement node");
    nmFree(NM, s);
  }

  debugAST(NM, n, "free block node");
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
 * @name - execCallNode
 * @desc - call the given function
 */
Value execCallNode(Nemo *NM, Node *n)
{
  Value ret;
  unsigned i;
  char *name = n->data.call.name;

  debugAST(NM, n, "execute function call node");

  /*
   * calling "print"
   */
  if (!strcmp(name, "print")){
    for (i = 0; n->data.call.params[i] != NULL; i++){
      char *value = valueToS(execNode(NM, n->data.call.params[i]));
      /* this the last parameter */
      if (n->data.call.params[i + 1] == NULL){
        printf("%s\n", value);
      /* this is NOT the last parameter */
      } else {
        printf("%s, ", value);
      }
    }
  }

  ret.type = OT_INTEGER;
  ret.value.i = 0xCA11;

  return ret;
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
      freeNode(NM, n->data.call.params[i]);
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

  if (body){
    n->data.funcdef.body = body;
    debugAST(NM, n, "create function definition node (name: %s, body: %p)", name, (void*)body);
  } else {
    n->data.funcdef.body = NULL;
    debugAST(NM, n, "create function declaration node (name: %s)", name);
  }

  return n;
}

/*
 * @name - execFuncDefNode
 * @desc - declare/define given function
 */
Value execFuncDefNode(Nemo *NM, Node *n)
{
  Value ret;

  /* FIXME */

  ret.type = OT_INTEGER;
  ret.value.i = 1;

  if (n->data.funcdef.body)
    debugAST(NM, n, "execute function definition node");
  else
    debugAST(NM, n, "execute function declaration node");

  return ret;
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
  if (n->data.funcdef.body){
    freeNode(NM, n->data.funcdef.body);
    debugAST(NM, n, "free function definition node");
  } else {
    debugAST(NM, n, "free function declaration node");
  }
  nmFree(NM, n);
}

static char *valueToS(Value o)
{
  static char s[50];

  switch (o.type){
    case OT_INTEGER:
      sprintf(s, "%d", o.value.i);
      break;
    case OT_FLOAT:
      sprintf(s, "%f", o.value.f);
      break;
    default:
      sprintf(s, "#unknown#valueToS#");
  }

  return s;
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
 * The IT Crowd, Family Guy
 */

