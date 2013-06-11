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
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "nemo.h"

/* a tiny little helpful macro to init the line/column field */
#define INIT_POS() \
  n->pos.line   = pos.line; \
  n->pos.column = pos.column;

/*
 * Array of pointer functions responsible for executing an adequate kind of node
 */
static NmObject *(*execFuncs[])(Node *) =
{
  /* XXX order must match the one in "enum NodeType" in ast.h */
  NmAST_ExecNop,
  NmAST_ExecInt,
  NmAST_ExecFloat,
  NmAST_ExecString,
  NmAST_ExecArray,
  NmAST_ExecName,
  NmAST_ExecBinop,
  NmAST_ExecUnop,
  NmAST_ExecIf,
  NmAST_ExecWhile,
  NmAST_ExecDecl,
  NmAST_ExecBlock,
  NmAST_ExecCall,
  NmAST_ExecFuncDef,
  NmAST_ExecInclude
};

/*
 * Array of pointer functions responsible for freeing an adequate kind of node
 */
static void (*freeFuncs[])(Node *) =
{
  /* XXX order must match the one in "enum NodeType" in ast.h */
  NmAST_FreeNop,
  NmAST_FreeInt,
  NmAST_FreeFloat,
  NmAST_FreeString,
  NmAST_FreeArray,
  NmAST_FreeName,
  NmAST_FreeBinop,
  NmAST_FreeUnop,
  NmAST_FreeIf,
  NmAST_FreeWhile,
  NmAST_FreeDecl,
  NmAST_FreeBlock,
  NmAST_FreeCall,
  NmAST_FreeFuncDef,
  NmAST_FreeInclude
};

/*
 * @name - NmAST_Exec
 * @desc - executes given node and returns the { NmObject * } it resulted in
 */
NmObject *NmAST_Exec(Node *n)
{
  assert(n);

  if (execFuncs[n->type])
    return execFuncs[n->type](n);
  else
    return NmNull;
}

/*
 * @name - NmAST_Free
 * @desc - runs an appropriate function, that will free given <n>
 */
void NmAST_Free(Node *n)
{
  assert(n);

  freeFuncs[n->type](n);
}

/*
 * @name - NmAST_GenNop
 * @desc - create a n that does NOTHING
 */
Node *NmAST_GenNop(Pos pos)
{
  Node *n = NmMem_Malloc(sizeof(Node));

  n->type = NT_NOP;
  INIT_POS();

  NmDebug_AST(n, "create NOP node");

  return n;
}

/*
 * @name - NmAST_ExecNop
 * @desc - execute a NOP (eg, return null)
 */
NmObject *NmAST_ExecNop(Node *n)
{
  /* unused parameter */
  (void)n;

  return NmNull;
}

/*
 * @name - NmAST_FreeNop
 * @desc - frees the NOP n
 */
void NmAST_FreeNop(Node *n)
{
  assert(n);
  assert(n->type == NT_NOP);

  NmDebug_AST(n, "free NOP node");

  NmMem_Free(n);
}

/*
 * @name - NmAST_GenInt
 * @desc - create a n holding a single literal integer
 */
Node *NmAST_GenInt(Pos pos, int i)
{
  Node *n = NmMem_Malloc(sizeof(Node));

  n->type = NT_INTEGER;
  n->data.i = i;
  INIT_POS();

  NmDebug_AST(n, "create int node (value: %d)", i);

  return n;
}

/*
 * @name - NmAST_ExecInt
 * @desc - return the value of the int
 */
NmObject *NmAST_ExecInt(Node *n)
{
  NmDebug_AST(n, "execute integer node");

  return NmInt_New(n->data.i);
}

/*
 * @name - NmAST_FreeInt
 * @desc - responsible for freeing literal integer ns
 */
void NmAST_FreeInt(Node *n)
{
  assert(n);
  assert(n->type == NT_INTEGER);

  NmDebug_AST(n, "free int node");

  NmMem_Free(n);
}

/*
 * @name - NmAST_GenFloat
 * @desc - create a n holding a single literal float
 */
Node *NmAST_GenFloat(Pos pos, float f)
{
  Node *n = NmMem_Malloc(sizeof(Node));

  n->type = NT_FLOAT;
  n->data.f = f;
  INIT_POS();

  NmDebug_AST(n, "create float node (value: %f)", f);

  return n;
}

/*
 * @name - NmAST_ExecFloat
 * @desc - return the value of the float
 */
NmObject *NmAST_ExecFloat(Node *n)
{
  NmDebug_AST(n, "execute float node");

  return NmFloat_New(n->data.f);
}

/*
 * @name - NmAST_FreeFloat
 * @desc - responsible for freeing literal float ns
 */
void NmAST_FreeFloat(Node *n)
{
  assert(n);
  assert(n->type == NT_FLOAT);

  NmDebug_AST(n, "free float node");

  NmMem_Free(n);
}

/*
 * @name - NmAST_GenString
 * @desc - create a node holding a single literal string
 */
Node *NmAST_GenString(Pos pos, char *s)
{
  Node *n = NmMem_Malloc(sizeof(Node));

  n->type = NT_STRING;
  n->data.s = NmMem_Strdup(s);
  INIT_POS();

  NmDebug_AST(n, "create string node (value: %s)", n->data.s);

  return n;
}

/*
 * @name - NmAST_ExecString
 * @desc - return the value of the string
 */
NmObject *NmAST_ExecString(Node *n)
{
  NmDebug_AST(n, "execute string node");

  return NmString_New(n->data.s);
}

/*
 * @name - NmAST_FreeString
 * @desc - responsible for freeing literal string node
 */
void NmAST_FreeString(Node *n)
{
  assert(n);
  assert(n->type == NT_STRING);

  NmDebug_AST(n, "free string node");

  NmMem_Free(n->data.s);
  NmMem_Free(n);
}

/*
 * @name - NmAST_GenArray
 * @desc - create a node holding a literal array
 */
Node *NmAST_GenArray(Pos pos, Node **a)
{
  Node *n = NmMem_Malloc(sizeof(Node));
  size_t nmemb = 0;

  /* count how many elements there are */
  if (a)
    for (Node **p = a; *p != NULL; p++)
      nmemb++;

  n->type = NT_ARRAY;
  n->data.array.nmemb = nmemb;
  n->data.array.a = a;
  INIT_POS();

  NmDebug_AST(n, "create array node (nmemb: %u, values: %p)", nmemb, (void *)a);

  return n;
}

/*
 * @name - NmAST_ExecArray
 * @desc - return the value of the string
 */
NmObject *NmAST_ExecArray(Node *n)
{
  NmObject *ob = NmArray_New(n->data.array.nmemb);
  size_t i = 0;

  NmDebug_AST(n, "execute array node");

  if (n->data.array.a)
    for (Node **p = n->data.array.a; *p != NULL; p++, i++)
      NmArray_SETELEM(ob, i, NmAST_Exec(*p));

  return ob;
}

/*
 * @name - NmAST_FreeArray
 * @desc - responsible for freeing literal string node
 */
void NmAST_FreeArray(Node *n)
{
  assert(n);
  assert(n->type == NT_ARRAY);

  NmDebug_AST(n, "free array node");

  if (n->data.array.a)
    for (Node **p = n->data.array.a; *p != NULL; p++)
      NmAST_Free(*p);

  NmMem_Free(n->data.array.a);
  NmMem_Free(n);
}

/*
 * @name - NmAST_GenName
 * @desc - creates a (eg. variable) name n
 */
Node *NmAST_GenName(Pos pos, char *s)
{
  Node *n = NmMem_Malloc(sizeof(Node));
  /*Scope *scope = NmScope_GetCurr();*/
  /*BOOL found = FALSE;*/

  n->type = NT_NAME;
  n->data.s = NmMem_Strdup(s);
  INIT_POS();

  NmDebug_AST(n, "create name node (name: %s)", s);
/*
  [> iterate through all (well, not all, from the current one, through it's
   * parents, to the main) the scopes <]
  for (scope = NmScope_GetCurr(); scope != NULL; scope = scope->parent){
    [> search for the variable <]
    for (VariablesList *vars = scope->globals; vars != NULL; vars = vars->next){
      if (!strcmp(vars->var->name, n->data.decl.name)){
        found = TRUE;
        break;
      }
    }
    [> or, it could be a name of a function that's being called <]
    [> search the C functions <]
    for (CFuncsList *cfuncs = scope->cfuncs; cfuncs != NULL; cfuncs = cfuncs->next){
      if (!strcmp(cfuncs->func->name, n->data.decl.name)){
        printf("found a cfunc in scope %s\n", scope->name);
        found = TRUE;
        break;
      }
    }
    [> search the user defined functions <]
    for (FuncsList *funcs = scope->funcs; funcs != NULL; funcs = funcs->next){
      if (!strcmp(funcs->func->name, n->data.decl.name)){
        found = TRUE;
        break;
      }
    }
  }

  if (!found){
    NmError_Parser(n, "variable '%s' was not found", n->data.decl.name);
    exit(EXIT_FAILURE);
  }
*/
  return n;
}

/*
 * @name - NmAST_ExecName
 * @desc - return the value the name is carring
 */
NmObject *NmAST_ExecName(Node *n)
{
  Scope *scope = NmScope_GetCurr();

  NmDebug_AST(n, "execute name node");

  /* search for the variable */
  for (VariablesList *vars = scope->globals; vars != NULL; vars = vars->next){
    if (!strcmp(vars->var->name, n->data.decl.name)){
      return vars->var->value;
    }
  }
  /*
   * FIXME: what should the name of a function itself return?
   */
  /* or, it could be a name of a function that's being called */
  /* search the C functions */
  for (CFuncsList *cfuncs = scope->cfuncs; cfuncs != NULL; cfuncs = cfuncs->next){
    if (!strcmp(cfuncs->func->name, n->data.decl.name)){
      return NmNull;
    }
  }
  /* search the user defined functions */
  for (FuncsList *funcs = scope->funcs; funcs != NULL; funcs = funcs->next){
    if (!strcmp(funcs->func->name, n->data.decl.name)){
      return NmNull;
    }
  }

  NmError_Parser(n, "variable '%s' was not found");
  exit(EXIT_FAILURE);
}

/*
 * @name - NmAST_FreeName
 * @desc - responsible for freeing (eg. variable) name ns
 */
void NmAST_FreeName(Node *n)
{
  assert(n);
  assert(n->type == NT_NAME);

  NmDebug_AST(n, "free name node");
  NmMem_Free(n->data.s);
  NmMem_Free(n);
}

/*
 * @name - NmAST_GenBinop
 * @desc - creates a binary operation n
 */
Node *NmAST_GenBinop(Pos pos, Node *left, BinaryOp op, Node *right)
{
  Node *n = NmMem_Malloc(sizeof(Node));

  n->type = NT_BINOP;
  n->data.binop.op = op;
  n->data.binop.left = left;
  n->data.binop.right = right;
  INIT_POS();

  NmDebug_AST(n, "create binary operation node (op: %s, left: %p, right: %p)", binopToS(op), (void*)left, (void*)right);

  return n;
}

/*
 * @name - NmAST_ExecBinop
 * @desc - return the result of the binary operation
 */
NmObject *NmAST_ExecBinop(Node *n)
{
  Scope *scope = NmScope_GetCurr();

  Node *left  = n->data.binop.left;
  Node *right = n->data.binop.right;

  NmObject *ob_left = NmAST_Exec(left);
  NmObject *ob_right = NmAST_Exec(right);
  NmObject *ret = NmNull;

  NmDebug_AST(n, "execute binary operation node");

  /* okay, that's easy things here */
  /*
   * XXX BINARY_ASSIGN
   */
  if (n->data.binop.op == BINARY_ASSIGN){
    Variable *var;
    VariablesList *p;
    char *name;
    BOOL found = FALSE;
    /* left-hand side of the assignment must be a name
     * (at least for now (30 Apr 2013)) */
    if (n->data.binop.left->type != NT_NAME){
      NmError_Parser(n, "expected an lvalue in assignment");
      exit(EXIT_FAILURE);
    }
    name = n->data.binop.left->data.s;
    /* iterate through the variables */
    for (p = scope->globals; p != NULL; p = p->next){
      if (!strcmp(p->var->name, name)){
        found = TRUE;
        var = p->var;
        break;
      }
    }
    /* error if the variable was not found */
    if (!found){
      NmError_Parser(n, "variable '%s' was not found", name);
      exit(EXIT_FAILURE);
    }
    /* check for the flags, eg. the NM_VAR_FLAG_CONST flag causes the variable
     * to be not-assignable*/
    if (NmVar_GETFLAG(var, NMVAR_FLAG_CONST)){
      NmError_Parser(n, "cannot change the value of a constant variable '%s'", name);
      /* FIXME */
      exit(EXIT_FAILURE);
    }
    /* actually assign the value */
    ret = NmAST_Exec(n->data.binop.right);
    var->value = ret;

    return ret;
  }
  /*
   * XXX BINARY_INDEX
   */
  else if (n->data.binop.op == BINARY_INDEX){
    if (!ob_left->fn.binary.index){
      NmError_Parser(n, "invalid binary operator '[]' for type '%s'", NmString_VAL(ob_left->fn.type_repr()));
      /* FIXME */
      exit(EXIT_FAILURE);
    }
    if (ob_right->type != OT_INTEGER){
      NmError_Parser(n, "expected type 'int' for the indexing value");
      /* FIXME */
      exit(EXIT_FAILURE);
    }

    return ob_left->fn.binary.index(ob_left, ob_right);
  }

/* <TYPE> is of type { BinaryOp }
 * <FUNC> is of type { BinaryFunc } */
#define op(TYPE, FUNC) \
  case TYPE: \
  { \
    if (!ob_left->fn.binary.FUNC){ \
      NmError_Parser(n, "invalid binary operation %s for types '%s' and '%s'", binopToS(n->data.binop.op), NmString_VAL(ob_left->fn.type_repr()), NmString_VAL(ob_right->fn.type_repr())); \
      /* FIXME: shouldn't exit here */ \
      exit(EXIT_FAILURE); \
    } \
\
    ret = ob_left->fn.binary.FUNC(ob_left, ob_right); \
    break; \
  }

/* DRY */
#define binary_ops() \
  switch (n->data.binop.op){ \
    /* here are all the binary functions that are available */ \
    op(BINARY_ADD, add); \
    default: \
    NmError_Parser(n, "invalid types '%s' and '%s' for binary operation %s", NmString_VAL(ob_left->fn.type_repr()), NmString_VAL(ob_right->fn.type_repr()), binopToS(n->data.binop.op)); \
      /* FIXME: shouldn't exit here */ \
      exit(EXIT_FAILURE); \
  }

  /* it's all easy when the two types are the same, just look if the type has
   * some function that does the operation, and simply run it and simply return
   * it's result */
  if (ob_left->type == ob_right->type){
    binary_ops();
  }
  /*
   * only ints and floats (so far?) can be used together in a binary operation
   */
  else {
    /* XXX int and float */
    if (ob_left->type == OT_INTEGER && ob_right->type == OT_FLOAT){
      /* check if the float is something like 2.0 or 1234.0, and if it is use it
       * as in int */
      if ((int)(NmFloat_VAL(ob_right)) == NmFloat_VAL(ob_right)){
        ob_right = NmInt_New((int)(NmFloat_VAL(ob_right)));
      } else {
        ob_left = NmFloat_NewFromInt(NmInt_VAL(ob_left));
      }
      binary_ops();
    }
    /* XXX float and int */
    else if (ob_left->type == OT_FLOAT && ob_right->type == OT_INTEGER){
      /* check if the float is something like 2.0 or 1234.0, and if it is use it
       * as in int */
      if ((int)(NmFloat_VAL(ob_left)) == NmFloat_VAL(ob_left)){
        ob_left = NmInt_New((int)(NmFloat_VAL(ob_left)));
      } else {
        ob_right = NmFloat_NewFromInt(NmInt_VAL(ob_right));
      }
      binary_ops();
    }
    /* if anything else, the operation is simply not permitted */
    else {
      NmError_Parser(n, "invalid types '%s' and '%s' for binary operation %s", NmString_VAL(ob_left->fn.type_repr()), NmString_VAL(ob_right->fn.type_repr()), binopToS(n->data.binop.op));
      /* FIXME: shouldn't exit here */
      exit(EXIT_FAILURE);
    }
  }

#undef op

  return ret;
}

/*
 * @name - NmAST_FreeBinop
 * @desc - responsible for freeing binary operation ns
 */
void NmAST_FreeBinop(Node *n)
{
  assert(n);
  assert(n->type == NT_BINOP);

  NmAST_Free(n->data.binop.left);
  NmAST_Free(n->data.binop.right);

  NmDebug_AST(n, "free binary operation node");
  NmMem_Free(n);
}

/*
 * @name - NmAST_GenUnop
 * @desc - creates a n for unary operation
 */
Node *NmAST_GenUnop(Pos pos, Node *target, UnaryOp op)
{
  Node *n = NmMem_Malloc(sizeof(Node));

  n->type = NT_UNOP;
  n->data.unop.op = op;
  n->data.unop.target = target;
  INIT_POS();

  NmDebug_AST(n, "create unary operation node (op: %s, expr: %p)", unopToS(op), (void*)target);

  return n;
}

/*
 * @name - NmAST_ExecUnop
 * @desc - return the result of the unary operation
 */
NmObject *NmAST_ExecUnop(Node *n)
{
  NmObject *ret;

  NmObject *target = NmAST_Exec(n->data.unop.target);

  NmDebug_AST(n, "execute unary operation node");

/* <TYPE> is of type { UnaryOp }
 * <FUNC> is of type { UnaryFunc } */
#define op(TYPE, FUNC) \
  case TYPE: { \
    if (!target->fn.unary.FUNC){ \
      /* FIXME: print that type good */ \
      NmError_Parser(n, "invalid type '%s' for unary operator %s", NmString_VAL(target->fn.type_repr()), unopToS(n->data.unop.op)); \
      /* FIXME: shouldn't exit here */ \
      exit(EXIT_FAILURE); \
    } \
\
    ret = target->fn.unary.FUNC(target); \
    break; \
  }

  switch (n->data.unop.op){
    op(UNARY_PLUS, plus);
    op(UNARY_MINUS, minus);
    op(UNARY_NEGATE, negate);
    default:
      /* FIXME: print than type good */
      NmError_Parser(n, "invalid unary operator %s for type '%d'", unopToS(n->data.unop.op), target->type);
      /* FIXME: shouldn't exit here */
      exit(EXIT_FAILURE);
  }

#undef op

  return ret;
}

/*
 * @name - NmAST_FreeUnop
 * @desc - responsible for freeing unary operation ns
 */
void NmAST_FreeUnop(Node *n)
{
  assert(n);
  assert(n->type == NT_UNOP);

  NmAST_Free(n->data.unop.target);

  NmDebug_AST(n, "free unary operation node");
  NmMem_Free(n);
}

/*
 * @name - NmAST_GenIf
 * @desc - creates a n for the if statement
 *         <guard>, <body> and <elsee> can be NULL, it means a NOP then
 */
Node *NmAST_GenIf(Pos pos, Node *guard, Node *body, Node *elsee, BOOL unless)
{
  Node *n = NmMem_Malloc(sizeof(Node));

  n->type = NT_IF;
  n->data.iff.guard = guard;
  n->data.iff.body = body;
  n->data.iff.elsee = elsee;
  n->data.iff.unless = unless;
  INIT_POS();

  if (unless)
    NmDebug_AST(n, "create unless node (guard: %p, body: %p, else: %p)", (void*)guard, (void*)body, (void*)elsee);
  else
    NmDebug_AST(n, "create if node (guard: %p, body: %p, else: %p)", (void*)guard, (void*)body, (void*)elsee);

  return n;
}

/*
 * @name - NmAST_ExecIf
 * @desc - do the if loop, return actually anything
 */
NmObject *NmAST_ExecIf(Node *n)
{
  Node *guard = n->data.iff.guard;
  Node *body = n->data.iff.body;
  Node *elsee = n->data.iff.elsee;
  BOOL unless = n->data.iff.unless;

  if (unless)
    NmDebug_AST(n, "execute unless node");
  else
    NmDebug_AST(n, "execute if node");

  if (unless){
    if (!NmObject_Boolish(NmAST_Exec(guard))){
      NmAST_Exec(body);
    } else {
      if (elsee)
        NmAST_Exec(elsee);
    }
  } else {
    if (NmObject_Boolish(NmAST_Exec(guard))){
      NmAST_Exec(body);
    } else {
      if (elsee)
        NmAST_Exec(elsee);
    }
  }

  return NmInt_New(1);
}

/*
 * @name - NmAST_FreeIf
 * @desc - responsible for freeing if ns, and optionally, if present, it's
 *         guarding statement and it's body, and the else statement connected
 *         with it
 */
void NmAST_FreeIf(Node *n)
{
  assert(n);
  assert(n->type == NT_IF);

  /* guard in if is optional */
  if (n->data.iff.guard)
    NmAST_Free(n->data.iff.guard);
  /* so is it's body */
  if (n->data.iff.body)
    NmAST_Free(n->data.iff.body);
  /* aaaaaand the else */
  if (n->data.iff.elsee)
    NmAST_Free(n->data.iff.elsee);

  if (n->data.iff.unless)
    NmDebug_AST(n, "free unless node");
  else
    NmDebug_AST(n, "free if node");
  NmMem_Free(n);
}

/*
 * @name - NmAST_GenWhile
 * @desc - creates a while n
 *         <guard>, <body> and <elsee> are optional, can be NULL,
 *         it means then that they are NOPs
 *         <elsee> here gets evaluated when the while loop didn't run even once
 */
Node *NmAST_GenWhile(Pos pos, Node *guard, Node *body, Node *elsee, BOOL until)
{
  Node *n = NmMem_Malloc(sizeof(Node));

  n->type = NT_WHILE;
  n->data.whilee.guard = guard;
  n->data.whilee.body = body;
  n->data.whilee.elsee = elsee;
  n->data.whilee.until = until;
  INIT_POS();

  if (until)
    NmDebug_AST(n, "create until node (guard: %p, body: %p, else: %p)", (void*)guard, (void*)body, (void*)elsee);
  else
    NmDebug_AST(n, "create while node (guard: %p, body: %p, else: %p)", (void*)guard, (void*)body, (void*)elsee);

  return n;
}

/*
 * @name - NmAST_ExecWhile
 * @desc - execute the loop and return, actually anything, it's a statement
 */
NmObject *NmAST_ExecWhile(Node *n)
{
  Node *guard = n->data.whilee.guard;
  Node *body = n->data.whilee.body;
  Node *elsee = n->data.whilee.elsee;
  BOOL until = n->data.whilee.until;

  if (until)
    NmDebug_AST(n, "execute until node");
  else
    NmDebug_AST(n, "execute while node");

  if (until){
    if (!NmObject_Boolish(NmAST_Exec(guard))){
      while (!NmObject_Boolish(NmAST_Exec(guard))){
        NmAST_Exec(body);
      }
    } else {
      if (elsee)
        NmAST_Exec(elsee);
    }
  } else {
    if (NmObject_Boolish(NmAST_Exec(guard))){
      while (NmObject_Boolish(NmAST_Exec(guard))){
        NmAST_Exec(body);
      }
    } else {
      if (elsee)
        NmAST_Exec(elsee);
    }
  }

  return NmInt_New(1);
}

/*
 * @name - NmAST_FreeWhile
 * @desc - responsible for freeing while n, and optionally
 *         guarding statement and it's body, as they are optional
 */
void NmAST_FreeWhile(Node *n)
{
  assert(n);
  assert(n->type == NT_WHILE);

  /* guard in while is optional */
  if (n->data.whilee.guard)
    NmAST_Free(n->data.whilee.guard);
  /* so is it's body */
  if (n->data.whilee.body)
    NmAST_Free(n->data.whilee.body);
  /* aaaaaand the else */
  if (n->data.whilee.elsee)
    NmAST_Free(n->data.whilee.elsee);

  if (n->data.whilee.until)
    NmDebug_AST(n, "free until node");
  else
    NmDebug_AST(n, "free while node");
  NmMem_Free(n);
}

/*
 * @name - NmAST_GenDecl
 * @desc - creates a n for declaring a variable of given <name>
 *         parameter <value> is optional, may be NULL, then it means
 *         something like:
 *
 *           my variable;
 */
Node *NmAST_GenDecl(Pos pos, char *name, Node *value, uint8_t flags)
{
  Node *n = NmMem_Malloc(sizeof(Node));

  n->type = NT_DECL;
  n->data.decl.name = NmMem_Strdup(name);
  n->data.decl.value = value;
  n->data.decl.flags = flags;
  INIT_POS();

  NmDebug_AST(n, "create variable declaration node (name: %s)", name);

  Scope *scope = NmScope_GetCurr();
  VariablesList *new_list = NmMem_Malloc(sizeof(VariablesList));
  Variable *new_var = NmMem_Malloc(sizeof(Variable));
  VariablesList *p;

  /* before doing anything, check if that variable was already declared */
  for (p = scope->globals; p != NULL; p = p->next){
    if (!strcmp(p->var->name, n->data.decl.name)){
      NmError_Parser(n, "global variable '%s' already declared", n->data.decl.name);
      exit(EXIT_FAILURE);
    }
  }

  new_var->name = NmMem_Strdup(n->data.decl.name);
  if (n->data.decl.value){
    NmObject *value = NmAST_Exec(n->data.decl.value);
    new_var->value = value;
  } else {
    /* declared variables get to be a integer with the value of 0 */
    new_var->value = NmInt_New(0);
  }
  new_var->flags = n->data.decl.flags;
  /* append to the globals list */
  new_list->var = new_var;
  new_list->next = scope->globals;
  scope->globals = new_list;

  return n;
}

/*
 * @name - NmAST_ExecDecl
 * @desc - declare/define the variable and return 1
 */
NmObject *NmAST_ExecDecl(Node *n)
{
  /* unused parameter */
  (void)n;

  NmDebug_AST(n, "execute variable declaration node");

  return NmInt_New(1);
}

/*
 * @name - NmAST_FreeDecl
 * @desc - responsible for freeing declaration n
 *         and optionally a init value it was holding
 */
void NmAST_FreeDecl(Node *n)
{
  assert(n);
  assert(n->type == NT_DECL);

  NmMem_Free(n->data.decl.name);
  /* initialized value is optional */
  if (n->data.decl.value)
    NmAST_Free(n->data.decl.value);

  NmDebug_AST(n, "free declaration node");
  NmMem_Free(n);
}

NmObject *NmAST_ExecBlock(Node *n)
{
  NmObject *ret = NmNull;
  Statement *s;
  Statement *next;

  assert(n);
  assert(n->type == NT_BLOCK);

  NmDebug_AST(n, "execute block node");

  for (s = n->data.block.tail; s != NULL; s = next){
    next = s->next;
    if (s->stmt){
      NmDebug_AST(s->stmt, "execute statement node");
      ret = NmAST_Exec(s->stmt);
    }
  }

  NmDebug_AST(n, "done executing block node");

  return ret;
}

/*
 * @name - freeBlock
 * @desc - frees given block and every statement it holds
 */
void NmAST_FreeBlock(Node *n)
{
  Statement *s;
  Statement *next;

  assert(n);
  assert(n->type == NT_BLOCK);

  for (s = n->data.block.tail; s != NULL; s = next){
    next = s->next;
    NmAST_Free(s->stmt);
    NmDebug_AST(n, "free statement node");
    NmMem_Free(s);
  }

  NmDebug_AST(n, "free block node");
  NmMem_Free(n);
}

/*
 * @name - NmAST_GenCall
 * @desc - creates a n for calling a function of a given <name>
 *         parameter <params> is optional, may be NULL, then it means
 *         that no parameters have been passed
 */
Node *NmAST_GenCall(Pos pos, char *name, Node **params)
{
  Node *n = NmMem_Malloc(sizeof(Node));

  n->type = NT_CALL;
  n->data.call.name = NmMem_Strdup(name);
  n->data.call.params = params;
  INIT_POS();

  NmDebug_AST(n, "create call node (name: %s, params: %p)", name, params);

  return n;
}

/*
 * @name - NmAST_ExecCall
 * @desc - call the given function
 */
NmObject *NmAST_ExecCall(Node *n)
{
  NmObject *ret = NULL;
  Scope *scope;
  char *name = n->data.call.name;

  NmDebug_AST(n, "execute function call node");

  /* iterate through all (well, not all, from the current one, through it's
   * parents, to the main) the scopes */
  for (scope = NmScope_GetCurr(); scope != NULL; scope = scope->parent){
    /* first check for the C functions */
    for (CFuncsList *list = scope->cfuncs; list != NULL; list = list->next){
      if (!strcmp(list->func->name, name)){
        size_t nmemb = 0;
        size_t i = 0;
        /* count how many elements there are */
        if (n->data.call.params)
          for (Node **p = n->data.call.params; *p != NULL; p++)
            nmemb++;
        /* parameters are stored as an array */
        NmObject *array = NmArray_New(nmemb);
        /* set the arrays elements */
        if (n->data.array.a)
          for (Node **p = n->data.array.a; *p != NULL; p++, i++)
            NmArray_SETELEM(array, i, NmAST_Exec(*p));
        /* execute the function */
        ret = list->func->body(array);
        /* if a function returns NULL it means something went wrong */
        if (ret == NULL){
          NmError_Parser(n, NmError_GetCurr());
          /* FIXME: shouldn't exit here */
          exit(EXIT_FAILURE);
        } else {
          return ret;
        }
      }
    }
    /* and then for the Nemo functions */
    for (FuncsList *list = scope->funcs; list != NULL; list = list->next){
      if (!strcmp(list->func->name, name)){
        ret = NmAST_Exec(list->func->body);
        /* if a function returns NULL it means something went wrong */
        if (ret == NULL){
          NmError_Parser(n, "executing function '%s' went wrong", name);
          /* FIXME: shouldn't exit here */
          exit(EXIT_FAILURE);
        } else {
          return ret;
        }
      }
    }
  }

  NmError_Parser(n, "function '%s' not found", name, scope->name);
  /* FIXME */
  exit(EXIT_FAILURE);
}

/*
 * @name - NmAST_FreeCall
 * @desc - responsible for freeing call node and optionally any params it had
 */
void NmAST_FreeCall(Node *n)
{
  unsigned i;

  assert(n);
  assert(n->type == NT_CALL);

  NmMem_Free(n->data.call.name);
  /* parameters are optional */
  if (n->data.call.params){
    for (i = 0; n->data.call.params[i] != NULL; i++){
      NmAST_Free(n->data.call.params[i]);
    }
    NmDebug_AST(n->data.call.params, "free params list");
    NmMem_Free(n->data.call.params);
  }

  NmDebug_AST(n, "free call node");
  NmMem_Free(n);
}

/*
 * @name - NmAST_GenFuncDef
 * @desc - creates a node for defining a function of a given <name>
 */
Node *NmAST_GenFuncDef(Pos pos, char *name, Node *body)
{
  Scope *scope = NmScope_GetCurr();
  FuncsList *l = NmMem_Malloc(sizeof(FuncsList));
  Func *f = NmMem_Malloc(sizeof(Func));
  Node *n = NmMem_Malloc(sizeof(Node));

  n->type = NT_FUNCDEF;
  n->data.funcdef.name = NmMem_Strdup(name);
  INIT_POS();

  if (body){
    n->data.funcdef.body = body;
    NmDebug_AST(n, "create function definition node (name: %s, body: %p)", name, (void*)body);
  } else {
    n->data.funcdef.body = NULL;
    NmDebug_AST(n, "create function declaration node (name: %s)", name);
  }

  /* initialize */
  f->name = NmMem_Strdup(name);
  f->body = body;
  l->func = f;
  /* append the function */
  l->next = scope->funcs;
  scope->funcs = l;

  return n;
}

/*
 * @name - NmAST_ExecFuncDef
 * @desc - declare/define given function
 */
NmObject *NmAST_ExecFuncDef(Node *n)
{
  if (n->data.funcdef.body)
    NmDebug_AST(n, "execute function definition node");
  else
    NmDebug_AST(n, "execute function declaration node");

  printf("funcdef %s in scope %s\n", n->data.funcdef.name, NmScope_GetCurr()->name);

  return NmInt_New(1);
}

/*
 * @name - NmAST_FreeFuncDef
 * @desc - responsible for freeing call n and optionally any params it had
 */
void NmAST_FreeFuncDef(Node *n)
{
  assert(n);
  assert(n->type == NT_FUNCDEF);

  NmMem_Free(n->data.funcdef.name);
  if (n->data.funcdef.body){
    NmAST_Free(n->data.funcdef.body);
    NmDebug_AST(n, "free function definition node");
  } else {
    NmDebug_AST(n, "free function declaration node");
  }
  NmMem_Free(n);
}

/*
 * @name - NmAST_GenInclude
 * @desc - creates a node that creates does the including thing
 */
Node *NmAST_GenInclude(Pos pos, char *fname, char *custom_path, BOOL use)
{
  Node *n = NmMem_Malloc(sizeof(Node));

  n->type = NT_INCLUDE;
  n->data.include.fname = NmMem_Strdup(fname);
  n->data.include.custom_path = NmMem_Strdup(custom_path);
  n->data.include.use = use;
  INIT_POS();

  NmDebug_AST(n, "create include node (fname: %s, custom_path: %p, use: %d)", fname, custom_path, use);

  return n;
}

/*
 * @name - NmAST_ExecInclude
 * @desc - actually does the including thing
 * @return 1 if everything went fine
 */
NmObject *NmAST_ExecInclude(Node *n)
{
  assert(n);
  assert(n->type == NT_INCLUDE);

  if (n->data.include.use){
    if (!Nm_UseModule(n->data.include.fname, n->data.include.custom_path)){
      NmError_Parser(n, NmError_GetCurr());
      return NmInt_New(0);
    }
  } else {
    if (!Nm_IncludeModule(n->data.include.fname, n->data.include.custom_path)){
      NmError_Parser(n, NmError_GetCurr());
      return NmInt_New(0);
    }
  }

  return NmInt_New(1);
}

void NmAST_FreeInclude(Node *n)
{
  assert(n);
  assert(n->type == NT_INCLUDE);

  NmMem_Free(n->data.include.fname);
  NmMem_Free(n->data.include.custom_path);
}

const char *binopToS(BinaryOp op)
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
    case BINARY_INDEX:      return "'[]'";
  }

  return "#unknown#binopToS#";
}

const char *unopToS(UnaryOp op)
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

