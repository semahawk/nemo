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
  execNopNode,
  execIntNode,
  execFloatNode,
  execStringNode,
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
  freeStringNode,
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
 * @desc - executes given node and returns the NmObject *it resulted in
 */
NmObject *execNode(Nemo *NM, Node *n)
{
  NmObject *ret;

  assert(n);

  /* do nothing with NOPs */
  if (execFuncs[n->type])
    ret = execFuncs[n->type](NM, n);

  return ret;
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
 * @name - execNopNode
 * @desc - execute a NOP, actually, it's only for the return
 *         so for example this would work:
 *
 *           while ; {
 *             # do stuff
 *           }
 *
 *         which would result in a infinite loop
 */
NmObject *execNopNode(Nemo *NM, Node *n)
{
  /* unused parameter */
  (void)NM;
  (void)n;

  return NmObject_NewFromInt(NM, 1);
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
NmObject *execIntNode(Nemo *NM, Node *n)
{
  debugAST(NM, n, "execute integer node");

  return NmObject_NewFromInt(NM, n->data.i);
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
NmObject *execFloatNode(Nemo *NM, Node *n)
{
  debugAST(NM, n, "execute float node");

  return NmObject_NewFromFloat(NM, n->data.f);
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
 * @name - genStringNode
 * @desc - create a node holding a single literal string
 */
Node *genStringNode(Nemo *NM, char *s)
{
  Node *n = nmMalloc(NM, sizeof(Node));

  n->type = NT_STRING;
  n->data.s = strdup(NM, s);

  debugAST(NM, n, "create string node (value: %s)", n->data.s);

  return n;
}

/*
 * @name - execStringNode
 * @desc - return the value of the string
 */
NmObject *execStringNode(Nemo *NM, Node *n)
{
  debugAST(NM, n, "execute string node");

  return NmObject_NewFromString(NM, n->data.s);
}

/*
 * @name - freeStringNode
 * @desc - responsible for freeing literal string node
 */
void freeStringNode(Nemo *NM, Node *n)
{
  assert(n);
  assert(n->type == NT_STRING);

  debugAST(NM, n, "free string node");

  nmFree(NM, n->data.s);
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
NmObject *execNameNode(Nemo *NM, Node *n)
{
  /*NmObject *ret;*/
  /*NmObject *vars_value;*/
  VariablesList *p;
  BOOL found = FALSE;

  debugAST(NM, n, "execute name node");

  /* search for the variable */
  for (p = NM->globals; p != NULL; p = p->next){
    if (!strcmp(p->var->name, n->data.decl.name)){
      found = TRUE;
      /*vars_value = p->var->value;*/
      break;
    }
  }

  if (!found){
    nmError("variable '%s' was not found");
    exit(EXIT_FAILURE);
  }

  /* FIXME */
  /* FIXME */
  /* FIXME */
  /* FIXME */
  /* FIXME */
  /*ret = vars_value;*/

  return NmObject_NewFromInt(NM, 8642862);
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
  nmFree(NM, n->data.s);
  nmFree(NM, n);
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
NmObject *execBinopNode(Nemo *NM, Node *n)
{
  NmObject *ret;

  debugAST(NM, n, "execute binary operation node");

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
        nmError("expected an lvalue in assignment");
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
        nmError("variable '%s' was not found", name);
        exit(EXIT_FAILURE);
      }
      /* actually assign the value */
      ret = execNode(NM, n->data.binop.right);
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
NmObject *execUnopNode(Nemo *NM, Node *n)
{
  /* FIXME */
  /* FIXME */
  /* FIXME */
  /* FIXME */

  debugAST(NM, n, "execute unary operation node");

  return NmObject_NewFromInt(NM, 8642573);
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
NmObject *execIfNode(Nemo *NM, Node *n)
{
  Node *guard = n->data.iff.guard;
  Node *body = n->data.iff.body;
  Node *elsee = n->data.iff.elsee;

  debugAST(NM, n, "execute if node");

  if (valueToB(execNode(NM, guard))){
    execNode(NM, body);
  } else {
    execNode(NM, elsee);
  }

  return NmObject_NewFromInt(NM, 1);
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
NmObject *execWhileNode(Nemo *NM, Node *n)
{
  Node *guard = n->data.iff.guard;
  Node *body = n->data.iff.body;
  Node *elsee = n->data.iff.elsee;

  debugAST(NM, n, "execute while node");

  if (valueToB(execNode(NM, guard))){
    while (valueToB(execNode(NM, guard))){
      execNode(NM, body);
    }
  } else {
    execNode(NM, elsee);
  }

  return NmObject_NewFromInt(NM, 1);
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

  debugAST(NM, n, "create variable declaration node (name: %s)", name);

  return n;
}

/*
 * @name - execDeclNode
 * @desc - declare/define the variable and return 1
 */
NmObject *execDeclNode(Nemo *NM, Node *n)
{
  VariablesList *new_list = nmMalloc(NM, sizeof(VariablesList));
  Variable *new_var = nmMalloc(NM, sizeof(Variable));
  VariablesList *p;

  /* before doing anything, check if that variable was already declared */
  for (p = NM->globals; p != NULL; p = p->next){
    if (!strcmp(p->var->name, n->data.decl.name)){
      nmError("global variable '%s' already declared", n->data.decl.name);
      exit(EXIT_FAILURE);
    }
  }

  debugAST(NM, n, "execute variable declaration node");

  if (n->data.decl.value){
    /*NmObject *value = execNode(NM, n->data.decl.value);*/
    /* FIXME */
    /*new_var->value = value;*/
  } else {
    /* zero out the variables value */
    memset(&new_var->value, 0, sizeof(NmObject *));
  }
  /* append to the globals list */
  new_var->name = strdup(NM, n->data.decl.name);
  new_list->var = new_var;
  new_list->next = NM->globals;
  NM->globals = new_list;

  return NmObject_NewFromInt(NM, 1);
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

NmObject *execBlockNode(Nemo *NM, Node *n)
{
  NmObject *ret;
  Statement *s;
  Statement *next;

  /*
   * FIXME: block should return what the last statement has returned
   */

  assert(n);
  assert(n->type == NT_BLOCK);

  debugAST(NM, n, "execute block node");

  for (s = n->data.block.tail; s != NULL; s = next){
    next = s->next;
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
  Statement *next;

  assert(n);
  assert(n->type == NT_BLOCK);

  for (s = n->data.block.tail; s != NULL; s = next){
    next = s->next;
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
NmObject *execCallNode(Nemo *NM, Node *n)
{
  NmObject *ret;
  unsigned i;
  char *name = n->data.call.name;

  debugAST(NM, n, "execute function call node");

  /*
   * calling "print"
   */
  if (!strcmp(name, "print")){
    for (i = 0; n->data.call.params != NULL && n->data.call.params[i] != NULL; i++){
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

  /* FIXME: when running an actuall function works, it should return whatever
   *        the function would have */
  (void)ret;

  return NmObject_NewFromInt(NM, 640420);
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
NmObject *execFuncDefNode(Nemo *NM, Node *n)
{
  /* FIXME */

  if (n->data.funcdef.body)
    debugAST(NM, n, "execute function definition node");
  else
    debugAST(NM, n, "execute function declaration node");

  return NmObject_NewFromInt(NM, 1);
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

