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
#include "error.h"
#include "object.h"

static const char *binopToS(BinaryOp);
static const char *unopToS(UnaryOp);
static BOOL valueToB(NmObject *);

/*
 * Array of pointer functions responsible for executing an adequate kind of node
 */
static NmObject *(*execFuncs[])(Nemo *, Node *) =
{
  /* XXX order must match the one in "enum NodeType" in ast.h */
  NmAST_ExecNop,
  NmAST_ExecInt,
  NmAST_ExecFloat,
  NmAST_ExecString,
  NmAST_ExecName,
  NmAST_ExecBinop,
  NmAST_ExecUnop,
  NmAST_ExecIf,
  NmAST_ExecWhile,
  NmAST_ExecDecl,
  NmAST_ExecBlock,
  NmAST_ExecCall,
  NmAST_ExecFuncDef
};

/*
 * Array of pointer functions responsible for freeing an adequate kind of node
 */
static void (*freeFuncs[])(Nemo *, Node *) =
{
  /* XXX order must match the one in "enum NodeType" in ast.h */
  NmAST_FreeNop,
  NmAST_FreeInt,
  NmAST_FreeFloat,
  NmAST_FreeString,
  NmAST_FreeName,
  NmAST_FreeBinop,
  NmAST_FreeUnop,
  NmAST_FreeIf,
  NmAST_FreeWhile,
  NmAST_FreeDecl,
  NmAST_FreeBlock,
  NmAST_FreCall,
  NmAST_FreeFuncDef
};

/*
 * @name - NmAST_Exec
 * @desc - executes given node and returns the NmObject *it resulted in
 */
NmObject *NmAST_Exec(Nemo *NM, Node *n)
{
  NmObject *ret;

  assert(n);

  /* do nothing with NOPs */
  if (execFuncs[n->type])
    ret = execFuncs[n->type](NM, n);

  return ret;
}

/*
 * @name - NmAST_Free
 * @desc - runs an appropriate function, that will free given <n>
 */
void NmAST_Free(Nemo *NM, Node *n)
{
  assert(n);

  /* watch out for NOPs */
  if (n)
    freeFuncs[n->type](NM, n);
}

/*
 * @name - NmAST_GenNop
 * @desc - create a n that does NOTHING
 */
Node *NmAST_GenNop(Nemo *NM)
{
  Node *n = NmMem_Malloc(NM, sizeof(Node));

  n->type = NT_NOP;

  NmDebug_AST(NM, n, "create NOP node");

  return n;
}

/*
 * @name - NmAST_ExecNop
 * @desc - execute a NOP, actually, it's only for the return
 *         so for example this would work:
 *
 *           while ; {
 *             # do stuff
 *           }
 *
 *         which would result in a infinite loop
 */
NmObject *NmAST_ExecNop(Nemo *NM, Node *n)
{
  /* unused parameter */
  (void)NM;
  (void)n;

  return NmObject_NewFromInt(NM, 1);
}

/*
 * @name - NmAST_FreeNop
 * @desc - frees the NOP n
 */
void NmAST_FreeNop(Nemo *NM, Node *n)
{
  assert(n);
  assert(n->type == NT_NOP);

  NmDebug_AST(NM, n, "free NOP node");

  NmMem_Free(NM, n);
}

/*
 * @name - NmAST_GenInt
 * @desc - create a n holding a single literal integer
 */
Node *NmAST_GenInt(Nemo *NM, int i)
{
  Node *n = NmMem_Malloc(NM, sizeof(Node));

  n->type = NT_INTEGER;
  n->data.i = i;

  NmDebug_AST(NM, n, "create int node (value: %d)", i);

  return n;
}

/*
 * @name - NmAST_ExecInt
 * @desc - return the value of the int
 */
NmObject *NmAST_ExecInt(Nemo *NM, Node *n)
{
  NmDebug_AST(NM, n, "execute integer node");

  return NmObject_NewFromInt(NM, n->data.i);
}

/*
 * @name - NmAST_FreeInt
 * @desc - responsible for freeing literal integer ns
 */
void NmAST_FreeInt(Nemo *NM, Node *n)
{
  assert(n);
  assert(n->type == NT_INTEGER);

  NmDebug_AST(NM, n, "free int node");

  NmMem_Free(NM, n);
}

/*
 * @name - NmAST_GenFloat
 * @desc - create a n holding a single literal float
 */
Node *NmAST_GenFloat(Nemo *NM, float f)
{
  Node *n = NmMem_Malloc(NM, sizeof(Node));

  n->type = NT_FLOAT;
  n->data.f = f;

  NmDebug_AST(NM, n, "create float node (value: %f)", f);

  return n;
}

/*
 * @name - NmAST_ExecFloat
 * @desc - return the value of the float
 */
NmObject *NmAST_ExecFloat(Nemo *NM, Node *n)
{
  NmDebug_AST(NM, n, "execute float node");

  return NmObject_NewFromFloat(NM, n->data.f);
}

/*
 * @name - NmAST_FreeFloat
 * @desc - responsible for freeing literal float ns
 */
void NmAST_FreeFloat(Nemo *NM, Node *n)
{
  assert(n);
  assert(n->type == NT_FLOAT);

  NmDebug_AST(NM, n, "free float node");

  NmMem_Free(NM, n);
}

/*
 * @name - NmAST_GenString
 * @desc - create a node holding a single literal string
 */
Node *NmAST_GenString(Nemo *NM, char *s)
{
  Node *n = NmMem_Malloc(NM, sizeof(Node));

  n->type = NT_STRING;
  n->data.s = NmMem_Strdup(NM, s);

  NmDebug_AST(NM, n, "create string node (value: %s)", n->data.s);

  return n;
}

/*
 * @name - NmAST_ExecString
 * @desc - return the value of the string
 */
NmObject *NmAST_ExecString(Nemo *NM, Node *n)
{
  NmDebug_AST(NM, n, "execute string node");

  return NmObject_NewFromString(NM, n->data.s);
}

/*
 * @name - NmAST_FreeString
 * @desc - responsible for freeing literal string node
 */
void NmAST_FreeString(Nemo *NM, Node *n)
{
  assert(n);
  assert(n->type == NT_STRING);

  NmDebug_AST(NM, n, "free string node");

  NmMem_Free(NM, n->data.s);
  NmMem_Free(NM, n);
}

/*
 * @name - NmAST_GenName
 * @desc - creates a (eg. variable) name n
 */
Node *NmAST_GenName(Nemo *NM, char *s)
{
  Node *n = NmMem_Malloc(NM, sizeof(Node));

  n->type = NT_NAME;
  n->data.s = NmMem_Strdup(NM, s);

  NmDebug_AST(NM, n, "create name node (name: %s)", s);

  return n;
}

/*
 * @name - NmAST_ExecName
 * @desc - return the value the name is carring
 */
NmObject *NmAST_ExecName(Nemo *NM, Node *n)
{
  NmObject *ret;
  VariablesList *p;
  BOOL found = FALSE;

  NmDebug_AST(NM, n, "execute name node");

  /* search for the variable */
  for (p = NM->globals; p != NULL; p = p->next){
    if (!strcmp(p->var->name, n->data.decl.name)){
      found = TRUE;
      ret = p->var->value;
      break;
    }
  }

  if (!found){
    NmError_Error("variable '%s' was not found");
    exit(EXIT_FAILURE);
  }

  return ret;
}

/*
 * @name - NmAST_FreeName
 * @desc - responsible for freeing (eg. variable) name ns
 */
void NmAST_FreeName(Nemo *NM, Node *n)
{
  assert(n);
  assert(n->type == NT_NAME);

  NmDebug_AST(NM, n, "free name node");
  NmMem_Free(NM, n->data.s);
  NmMem_Free(NM, n);
}

/*
 * @name - NmAST_GenBinop
 * @desc - creates a binary operation n
 */
Node *NmAST_GenBinop(Nemo *NM, Node *left, BinaryOp op, Node *right)
{
  Node *n = NmMem_Malloc(NM, sizeof(Node));

  n->type = NT_BINOP;
  n->data.binop.op = op;
  n->data.binop.left = left;
  n->data.binop.right = right;

  NmDebug_AST(NM, n, "create binary operation node (op: %s, left: %p, right: %p)", binopToS(op), (void*)left, (void*)right);

  return n;
}

/*
 * @name - NmAST_ExecBinop
 * @desc - return the result of the binary operation
 */
NmObject *NmAST_ExecBinop(Nemo *NM, Node *n)
{
  NmObject *ret;

  NmDebug_AST(NM, n, "execute binary operation node");

  switch (n->data.binop.op){
    case BINARY_ASSIGN:
    {
      Variable *var;
      VariablesList *p;
      char *name;
      BOOL found = FALSE;
      /* left-hand side of the assignment must be a name
       * (at least for now (30 Apr 2013)) */
      if (n->data.binop.left->type != NT_NAME){
        NmError_Error("expected an lvalue in assignment");
        exit(EXIT_FAILURE);
      }
      name = n->data.binop.left->data.s;
      /* iterate through the variables */
      for (p = NM->globals; p != NULL; p = p->next){
        if (!strcmp(p->var->name, name)){
          found = TRUE;
          var = p->var;
          break;
        }
      }
      /* error if the variable was not found */
      if (!found){
        NmError_Error("variable '%s' was not found", name);
        exit(EXIT_FAILURE);
      }
      /* actually assign the value */
      ret = NmAST_Exec(NM, n->data.binop.right);
      /* FIXME */
      /* FIXME */
      /* FIXME */
      /* FIXME */
      /* FIXME */
      /*var->value = ret;*/
      break;
    }
    case BINARY_ADD:
    case BINARY_SUB:
    case BINARY_MUL:
    case BINARY_DIV:
    case BINARY_MOD:
    case BINARY_ASSIGN_ADD:
    case BINARY_ASSIGN_SUB:
    case BINARY_ASSIGN_MUL:
    case BINARY_ASSIGN_DIV:
    case BINARY_ASSIGN_MOD:
    case BINARY_GT:
    case BINARY_LT:
      /* FIXME: to be implemented */
      ret = NmObject_NewFromInt(NM, 73753753);
      break;
  }

  return ret;
}

/*
 * @name - NmAST_FreeBinop
 * @desc - responsible for freeing binary operation ns
 */
void NmAST_FreeBinop(Nemo *NM, Node *n)
{
  assert(n);
  assert(n->type == NT_BINOP);

  NmAST_Free(NM, n->data.binop.left);
  NmAST_Free(NM, n->data.binop.right);

  NmDebug_AST(NM, n, "free binary operation node");
  NmMem_Free(NM, n);
}

/*
 * @name - NmAST_GenUnop
 * @desc - creates a n for unary operation
 */
Node *NmAST_GenUnop(Nemo *NM, Node *expr, UnaryOp op)
{
  Node *n = NmMem_Malloc(NM, sizeof(Node));

  n->type = NT_UNOP;
  n->data.unop.op = op;
  n->data.unop.expr = expr;

  NmDebug_AST(NM, n, "create unary operation node (op: %s, expr: %p)", unopToS(op), (void*)expr);

  return n;
}

/*
 * @name - NmAST_ExecUnop
 * @desc - return the result of the unary operation
 */
NmObject *NmAST_ExecUnop(Nemo *NM, Node *n)
{
  /* FIXME */
  /* FIXME */
  /* FIXME */
  /* FIXME */

  NmDebug_AST(NM, n, "execute unary operation node");

  return NmObject_NewFromInt(NM, 8642573);
}

/*
 * @name - NmAST_FreeUnop
 * @desc - responsible for freeing unary operation ns
 */
void NmAST_FreeUnop(Nemo *NM, Node *n)
{
  assert(n);
  assert(n->type == NT_UNOP);

  NmAST_Free(NM, n->data.unop.expr);

  NmDebug_AST(NM, n, "free unary operation node");
  NmMem_Free(NM, n);
}

/*
 * @name - NmAST_GenIf
 * @desc - creates a n for the if statement
 *         <guard>, <body> and <elsee> can be NULL, it means a NOP then
 */
Node *NmAST_GenIf(Nemo *NM, Node *guard, Node *body, Node *elsee)
{
  Node *n = NmMem_Malloc(NM, sizeof(Node));

  n->type = NT_IF;
  n->data.iff.guard = guard;
  n->data.iff.body = body;
  n->data.iff.elsee = elsee;

  NmDebug_AST(NM, n, "create if node (guard: %p, body: %p, else: %p)", (void*)guard, (void*)body, (void*)elsee);

  return n;
}

/*
 * @name - NmAST_ExecIf
 * @desc - do the if loop, return actually anything
 */
NmObject *NmAST_ExecIf(Nemo *NM, Node *n)
{
  Node *guard = n->data.iff.guard;
  Node *body = n->data.iff.body;
  Node *elsee = n->data.iff.elsee;

  NmDebug_AST(NM, n, "execute if node");

  if (valueToB(NmAST_Exec(NM, guard))){
    NmAST_Exec(NM, body);
  } else {
    NmAST_Exec(NM, elsee);
  }

  return NmObject_NewFromInt(NM, 1);
}

/*
 * @name - NmAST_FreeIf
 * @desc - responsible for freeing if ns, and optionally, if present, it's
 *         guarding statement and it's body, and the else statement connected
 *         with it
 */
void NmAST_FreeIf(Nemo *NM, Node *n)
{
  assert(n);
  assert(n->type == NT_IF);

  /* guard in if is optional */
  if (n->data.iff.guard)
    NmAST_Free(NM, n->data.iff.guard);
  /* so is it's body */
  if (n->data.iff.body)
    NmAST_Free(NM, n->data.iff.body);
  /* aaaaaand the else */
  if (n->data.iff.elsee)
    NmAST_Free(NM, n->data.iff.elsee);

  NmDebug_AST(NM, n, "free if node");
  NmMem_Free(NM, n);
}

/*
 * @name - NmAST_GenWhile
 * @desc - creates a while n
 *         <guard>, <body> and <elsee> are optional, can be NULL,
 *         it means then that they are NOPs
 *         <elsee> here gets evaluated when the while loop didn't run even once
 */
Node *NmAST_GenWhile(Nemo *NM, Node *guard, Node *body, Node *elsee)
{
  Node *n = NmMem_Malloc(NM, sizeof(Node));

  n->type = NT_WHILE;
  n->data.whilee.guard = guard;
  n->data.whilee.body = body;
  n->data.whilee.elsee = elsee;

  NmDebug_AST(NM, n, "create while node (guard: %p, body: %p, else: %p)", (void*)guard, (void*)body, (void*)elsee);

  return n;
}

/*
 * @name - NmAST_ExecWhile
 * @desc - execute the loop and return, actually anything, it's a statement
 */
NmObject *NmAST_ExecWhile(Nemo *NM, Node *n)
{
  Node *guard = n->data.iff.guard;
  Node *body = n->data.iff.body;
  Node *elsee = n->data.iff.elsee;

  NmDebug_AST(NM, n, "execute while node");

  if (valueToB(NmAST_Exec(NM, guard))){
    while (valueToB(NmAST_Exec(NM, guard))){
      NmAST_Exec(NM, body);
    }
  } else {
    NmAST_Exec(NM, elsee);
  }

  return NmObject_NewFromInt(NM, 1);
}

/*
 * @name - NmAST_FreeWhile
 * @desc - responsible for freeing while n, and optionally
 *         guarding statement and it's body, as they are optional
 */
void NmAST_FreeWhile(Nemo *NM, Node *n)
{
  assert(n);
  assert(n->type == NT_WHILE);

  /* guard in while is optional */
  if (n->data.whilee.guard)
    NmAST_Free(NM, n->data.whilee.guard);
  /* so is it's body */
  if (n->data.whilee.body)
    NmAST_Free(NM, n->data.whilee.body);
  /* aaaaaand the else */
  if (n->data.whilee.elsee)
    NmAST_Free(NM, n->data.whilee.elsee);

  NmDebug_AST(NM, n, "free while node");
  NmMem_Free(NM, n);
}

/*
 * @name - NmAST_GenDecl
 * @desc - creates a n for declaring a variable of given <name>
 *         parameter <value> is optional, may be NULL, then it means
 *         something like:
 *
 *           my variable;
 */
Node *NmAST_GenDecl(Nemo *NM, char *name, Node *value)
{
  Node *n = NmMem_Malloc(NM, sizeof(Node));

  n->type = NT_DECL;
  n->data.decl.name = NmMem_Strdup(NM, name);
  n->data.decl.value = value;

  NmDebug_AST(NM, n, "create variable declaration node (name: %s)", name);

  return n;
}

/*
 * @name - NmAST_ExecDecl
 * @desc - declare/define the variable and return 1
 */
NmObject *NmAST_ExecDecl(Nemo *NM, Node *n)
{
  VariablesList *new_list = NmMem_Malloc(NM, sizeof(VariablesList));
  Variable *new_var = NmMem_Malloc(NM, sizeof(Variable));
  VariablesList *p;

  /* before doing anything, check if that variable was already declared */
  for (p = NM->globals; p != NULL; p = p->next){
    if (!strcmp(p->var->name, n->data.decl.name)){
      NmError_Error("global variable '%s' already declared", n->data.decl.name);
      exit(EXIT_FAILURE);
    }
  }

  NmDebug_AST(NM, n, "execute variable declaration node");

  if (n->data.decl.value){
    NmObject *value = NmAST_Exec(NM, n->data.decl.value);
    new_var->value = value;
  } else {
    /* zero out the variables value */
    memset(&new_var->value, 0, sizeof(NmObject));
  }
  /* append to the globals list */
  new_var->name = NmMem_Strdup(NM, n->data.decl.name);
  new_list->var = new_var;
  new_list->next = NM->globals;
  NM->globals = new_list;

  return NmObject_NewFromInt(NM, 1);
}

/*
 * @name - NmAST_FreeDecl
 * @desc - responsible for freeing declaration n
 *         and optionally a init value it was holding
 */
void NmAST_FreeDecl(Nemo *NM, Node *n)
{
  assert(n);
  assert(n->type == NT_DECL);

  NmMem_Free(NM, n->data.decl.name);
  /* initialized value is optional */
  if (n->data.decl.value)
    NmAST_Free(NM, n->data.decl.value);

  NmDebug_AST(NM, n, "free declaration node");
  NmMem_Free(NM, n);
}

NmObject *NmAST_ExecBlock(Nemo *NM, Node *n)
{
  NmObject *ret;
  Statement *s;
  Statement *next;

  assert(n);
  assert(n->type == NT_BLOCK);

  NmDebug_AST(NM, n, "execute block node");

  for (s = n->data.block.tail; s != NULL; s = next){
    next = s->next;
    NmDebug_AST(NM, s->stmt, "execute statement node");
    ret = NmAST_Exec(NM, s->stmt);
  }

  NmDebug_AST(NM, n, "done executing block node");

  return ret;
}

/*
 * @name - freeBlock
 * @desc - frees given block and every statement it holds
 */
void NmAST_FreeBlock(Nemo *NM, Node *n)
{
  Statement *s;
  Statement *next;

  assert(n);
  assert(n->type == NT_BLOCK);

  for (s = n->data.block.tail; s != NULL; s = next){
    next = s->next;
    NmAST_Free(NM, s->stmt);
    NmDebug_AST(NM, n, "free statement node");
    NmMem_Free(NM, s);
  }

  NmDebug_AST(NM, n, "free block node");
  NmMem_Free(NM, n);
}

/*
 * @name - NmAST_GenCall
 * @desc - creates a n for calling a function of a given <name>
 *         parameter <params> is optional, may be NULL, then it means
 *         that no parameters have been passed
 */
Node *NmAST_GenCall(Nemo *NM, char *name, Node **params)
{
  Node *n = NmMem_Malloc(NM, sizeof(Node));

  n->type = NT_CALL;
  n->data.call.name = NmMem_Strdup(NM, name);
  n->data.call.params = params;

  NmDebug_AST(NM, n, "create call node (name: %s, params: %p)", name, params);

  return n;
}

/*
 * @name - NmAST_ExecCall
 * @desc - call the given function
 */
NmObject *NmAST_ExecCall(Nemo *NM, Node *n)
{
  NmObject *ret;
  unsigned i;
  char *name = n->data.call.name;

  NmDebug_AST(NM, n, "execute function call node");

  /*
   * calling "print"
   */
  if (!strcmp(name, "print")){
    for (i = 0; n->data.call.params != NULL && n->data.call.params[i] != NULL; i++){
      NmObject *value = NmAST_Exec(NM, n->data.call.params[i]);
      /* this the last parameter */
      if (n->data.call.params[i + 1] == NULL){
        value->fn.print(NM, stdout, value);
        putchar('\n');
      /* this is NOT the last parameter */
      } else {
        value->fn.print(NM, stdout, value);
        putchar(',');
        putchar(' ');
      }
    }
  }

  /* FIXME: when running an actuall function works, it should return whatever
   *        the function would have */
  (void)ret;

  return NmObject_NewFromInt(NM, 640420);
}

/*
 * @name - NmAST_FreCall
 * @desc - responsible for freeing call n and optionally any params it had
 */
void NmAST_FreCall(Nemo *NM, Node *n)
{
  unsigned i;

  assert(n);
  assert(n->type == NT_CALL);

  NmMem_Free(NM, n->data.call.name);
  /* parameters are optional */
  if (n->data.call.params){
    for (i = 0; n->data.call.params[i] != NULL; i++){
      NmAST_Free(NM, n->data.call.params[i]);
    }
    NmDebug_AST(NM, n->data.call.params, "free params list");
    NmMem_Free(NM, n->data.call.params);
  }

  NmDebug_AST(NM, n, "free call node");
  NmMem_Free(NM, n);
}

/*
 * @name - NmAST_GenFuncDef
 * @desc - creates a node for defining a function of a given <name>
 */
Node *NmAST_GenFuncDef(Nemo *NM, char *name, Node *body)
{
  Node *n = NmMem_Malloc(NM, sizeof(Node));

  n->type = NT_FUNCDEF;
  n->data.funcdef.name = NmMem_Strdup(NM, name);

  if (body){
    n->data.funcdef.body = body;
    NmDebug_AST(NM, n, "create function definition node (name: %s, body: %p)", name, (void*)body);
  } else {
    n->data.funcdef.body = NULL;
    NmDebug_AST(NM, n, "create function declaration node (name: %s)", name);
  }

  return n;
}

/*
 * @name - NmAST_ExecFuncDef
 * @desc - declare/define given function
 */
NmObject *NmAST_ExecFuncDef(Nemo *NM, Node *n)
{
  /* FIXME */

  if (n->data.funcdef.body)
    NmDebug_AST(NM, n, "execute function definition node");
  else
    NmDebug_AST(NM, n, "execute function declaration node");

  return NmObject_NewFromInt(NM, 1);
}

/*
 * @name - NmAST_FreeFuncDef
 * @desc - responsible for freeing call n and optionally any params it had
 */
void NmAST_FreeFuncDef(Nemo *NM, Node *n)
{
  assert(n);
  assert(n->type == NT_FUNCDEF);

  NmMem_Free(NM, n->data.funcdef.name);
  if (n->data.funcdef.body){
    NmAST_Free(NM, n->data.funcdef.body);
    NmDebug_AST(NM, n, "free function definition node");
  } else {
    NmDebug_AST(NM, n, "free function declaration node");
  }
  NmMem_Free(NM, n);
}

/*
 * Check if given value is a true/false boolean-wise.
 *
 * In Nemo there is no "bool" type as is.
 */
static BOOL valueToB(NmObject *o)
{
  /*
   * 0 and 0.0 are false
   * everything else is true
   */
  switch (o->type){
    /* FIXME */
    case OT_FLOAT:
      return TRUE;
    /* FIXME */
    case OT_STRING:
      return TRUE;
    default:
      return FALSE;
  }
}

char *valueToS(NmObject *o)
{
  /*
   * FIXME
   * FIXMEE
   *
   * FIXXXMEEEE
   */
  switch (o->type){
    case OT_INTEGER:
      return "INTEGER";
    case OT_FLOAT:
      return "FLOAT";
    case OT_STRING:
      return "STRING";
    default:
      return "#unknown#valueToS#";
  }
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
    case BINARY_ASSIGN:     return "'='";
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
 * Steve Vai, Testament
 *
 * The IT Crowd, Family Guy
 */

