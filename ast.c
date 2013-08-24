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
#include "object.h"

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
  NmAST_ExecCall,
  NmAST_ExecStmt,
  NmAST_ExecBlock,
  NmAST_ExecFuncDef,
  NmAST_ExecUse
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
  NmAST_FreeCall,
  NmAST_FreeStmt,
  NmAST_FreeBlock,
  NmAST_FreeFuncDef,
  NmAST_FreeUse
};

/*
 * @name - NmAST_ExecNodes
 * @desc - executes all the statements depending on each node's "next" field
 * @return - the last executed statement's value
 * @param - nodest - the first statement to be executed
 */
NmObject *NmAST_ExecNodes(Node *nodest)
{
  /* TODO
   *
   * That one is going to be using the famous "ip", for instance.
   */
  (void)nodest;

  return NmNull;
}

/*
 * @name - NmAST_Exec
 * @desc - executes given node and returns the { NmObject * } it resulted in
 */
static inline NmObject *NmAST_Exec(Node *n)
{
  assert(n);

  return execFuncs[n->type] ? execFuncs[n->type](n) : NmNull;
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
  Node *n = NmMem_Calloc(1, sizeof(Node));

  n->type = NT_NOP;
  INIT_POS();

#if DEBUG
  NmDebug_AST(n, "create NOP node");
#endif

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

#if DEBUG
  NmDebug_AST(n, "free NOP node");
#endif

  NmMem_Free(n);
}

/*
 * @name - NmAST_GenInt
 * @desc - create a n holding a single literal integer
 */
Node *NmAST_GenInt(Pos pos, int i)
{
  Node_Int *n = NmMem_Calloc(1, sizeof(Node_Int));

  n->type = NT_INTEGER;
  n->i = i;
  INIT_POS();

#if DEBUG
  NmDebug_AST(n, "create int node (value: %d)", i);
#endif

  return (Node *)n;
}

/*
 * @name - NmAST_ExecInt
 * @desc - return the value of the int
 */
NmObject *NmAST_ExecInt(Node *n)
{
#if DEBUG
  NmDebug_AST(n, "execute integer node");
#endif

  return NmInt_New(((Node_Int *)n)->i);
}

/*
 * @name - NmAST_FreeInt
 * @desc - responsible for freeing literal integer ns
 */
void NmAST_FreeInt(Node *n)
{
  assert(n);
  assert(n->type == NT_INTEGER);

#if DEBUG
  NmDebug_AST(n, "free int node");
#endif

  NmMem_Free(n);
}

/*
 * @name - NmAST_GenFloat
 * @desc - create a n holding a single literal float
 */
Node *NmAST_GenFloat(Pos pos, float f)
{
  Node_Float *n = NmMem_Calloc(1, sizeof(Node_Float));

  n->type = NT_FLOAT;
  n->f = f;
  INIT_POS();

#if DEBUG
  NmDebug_AST(n, "create float node (value: %f)", f);
#endif

  return (Node *)n;
}

/*
 * @name - NmAST_ExecFloat
 * @desc - return the value of the float
 */
NmObject *NmAST_ExecFloat(Node *n)
{
#if DEBUG
  NmDebug_AST(n, "execute float node");
#endif

  return NmFloat_New(((Node_Float *)n)->f);
}

/*
 * @name - NmAST_FreeFloat
 * @desc - responsible for freeing literal float ns
 */
void NmAST_FreeFloat(Node *n)
{
  assert(n);
  assert(n->type == NT_FLOAT);

#if DEBUG
  NmDebug_AST(n, "free float node");
#endif

  NmMem_Free(n);
}

/*
 * @name - NmAST_GenString
 * @desc - create a node holding a single literal string
 */
Node *NmAST_GenString(Pos pos, char *s)
{
  Node_String *n = NmMem_Calloc(1, sizeof(Node_String));

  n->type = NT_STRING;
  n->s = NmMem_Strdup(s);
  INIT_POS();

#if DEBUG
  NmDebug_AST(n, "create string node (value: \"%s\")", s);
#endif

  return (Node *)n;
}

/*
 * @name - NmAST_ExecString
 * @desc - return the value of the string
 */
NmObject *NmAST_ExecString(Node *n)
{
  NmObject *ret = NmString_New(((Node_String *)n)->s);

#if DEBUG
  NmDebug_AST(n, "execute string node");
#endif

  if (!ret){
    NmError_Parser(n, NmError_GetCurr());
    Nm_Exit();
    return NULL;
  }

  return NmString_New(((Node_String *)n)->s);
}

/*
 * @name - NmAST_FreeString
 * @desc - responsible for freeing literal string node
 */
void NmAST_FreeString(Node *n)
{
  assert(n);
  assert(n->type == NT_STRING);

#if DEBUG
  NmDebug_AST(n, "free string node");
#endif

  NmMem_Free(((Node_String *)n)->s);
  NmMem_Free(n);
}

/*
 * @name - NmAST_GenArray
 * @desc - create a node holding a literal array
 */
Node *NmAST_GenArray(Pos pos, Node **a)
{
  Node_Array *n = NmMem_Calloc(1, sizeof(Node_Array));
  size_t nmemb = 0;

  /* count how many elements there are */
  if (a)
    for (Node **p = a; *p != NULL; p++)
      nmemb++;

  n->type = NT_ARRAY;
  n->nmemb = nmemb;
  n->a = a;
  INIT_POS();

#if DEBUG
  NmDebug_AST(n, "create array node (nmemb: %u, values: %p)", nmemb, (void *)a);
#endif

  return (Node *)n;
}

/*
 * @name - NmAST_ExecArray
 * @desc - return the value of the string
 */
NmObject *NmAST_ExecArray(Node *n)
{
  Node_Array *n_arr = (Node_Array *)n;
  NmObject *ob = NmArray_New(n_arr->nmemb);
  size_t i = 0;

#if DEBUG
  NmDebug_AST(n, "execute array node");
#endif

  if (n_arr->a)
    for (Node **p = n_arr->a; *p != NULL; p++, i++)
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

#if DEBUG
  NmDebug_AST(n, "free array node");
#endif

  if (((Node_Array *)n)->a)
    for (Node **p = ((Node_Array *)n)->a; *p != NULL; p++)
      NmAST_Free(*p);

  NmMem_Free(((Node_Array *)n)->a);
  NmMem_Free(n);
}

/*
 * @name - NmAST_GenName
 * @desc - creates a (eg. variable) name n
 */
Node *NmAST_GenName(Pos pos, char *name, struct Namespace *node_namespace)
{
  Node_Name *n = NmMem_Calloc(1, sizeof(Node_Name));
  bool found = false;
  Namespace *curr_namespace = NmNamespace_GetCurr();

  n->type = NT_NAME;
  n->name = NmMem_Strdup(name);
  n->namespace = node_namespace;
  INIT_POS();

#if DEBUG
  NmDebug_AST(n, "create name node (name: %s)", name);
#endif

  /* search for the variable (although it should be the parser's job to ensure
   * that the variable actually exists) */
  for (VariablesList *vars = node_namespace->globals; vars != NULL; vars = vars->next){
    if (!strcmp(vars->var->name, name)){
      /* if the variable is declared as "my" then current's namespace name must
       * be the same as the namespace that we request the variable from, so that
       * the "private" variables are accessible from inside of the module in
       * which they were created */
      if (vars->var->flags & NMVAR_FLAG_PRIVATE){
        if (!strcmp(node_namespace->name, curr_namespace->name)){
          /* we found the variable, but it's private, but requested from the
           * file in which was declared, so we're cool */
          found = true;
          break;
        } else {
          NmError_Parser((Node *)n, "can't access variable '%s.%s' which was declared as private", node_namespace->name, name);
          Nm_Exit();
        }
      } else {
        /* we found the variable, and it's public and accessible */
        found = true;
        break;
      }
    }
  }

  /*if (!found){*/
    /*NmError_Parser((Node *)n, "variable '%s.%s' was not found", node_namespace->name, name);*/
    /*Nm_Exit();*/
    /*return NULL;*/
  /*}*/

  return (Node *)n;
}

/*
 * @name - NmAST_ExecName
 * @desc - return the value the name is carring
 */
NmObject *NmAST_ExecName(Node *n)
{
  Node_Name *nc = (Node_Name *)n;

#if DEBUG
  NmDebug_AST(n, "execute name node (name: %s)", nc->name);
#endif

  /* search for the variable */
  for (VariablesList *vars = nc->namespace->globals; vars != NULL; vars = vars->next)
    if (!strcmp(vars->var->name, nc->name))
      return vars->var->value;

  NmError_Parser(n, "variable '%s.%s' was not found", nc->namespace->name, nc->name);
  return NULL;
}

/*
 * @name - NmAST_FreeName
 * @desc - responsible for freeing (eg. variable) name ns
 */
void NmAST_FreeName(Node *n)
{
  assert(n);
  assert(n->type == NT_NAME);

#if DEBUG
  NmDebug_AST(n, "free name node");
#endif
  NmMem_Free(((Node_Name *)n)->name);
  NmMem_Free(n);
}

/*
 * @name - NmAST_GenBinop
 * @desc - creates a binary operation n
 */
Node *NmAST_GenBinop(Pos pos, Node *left, BinaryOp op, Node *right)
{
  Node_Binop *n = NmMem_Calloc(1, sizeof(Node_Binop));

  n->type = NT_BINOP;
  n->op = op;
  n->left = left;
  n->right = right;
  INIT_POS();

#if DEBUG
  NmDebug_AST(n, "create binary operation node (op: %s, left: %p, right: %p)", binopToS(op), (void*)left, (void*)right);
#endif

  return (Node *)n;
}

/*
 * @name - NmAST_ExecBinop
 * @desc - return the result of the binary operation
 */
NmObject *NmAST_ExecBinop(Node *n)
{
  Node_Binop *nc = (Node_Binop *)n;
  Namespace *namespace = NmNamespace_GetCurr();

  Node *left  = nc->left;
  Node *right = nc->right;

  NmObject *ret = NmNull;

#if DEBUG
  NmDebug_AST(n, "execute binary operation node");
#endif

  /* okay, that's easy things here */
  /*
   * XXX BINARY_ASSIGN
   */
  if (nc->op == BINARY_ASSIGN){
    Variable *var;
    VariablesList *p;
    char *name;
    bool found = false;
    /* left-hand side of the assignment must be a name
     * (at least for now (30 Apr 2013)) */
    if (nc->left->type != NT_NAME){
      NmError_Parser(n, "expected an lvalue in assignment");
      Nm_Exit();
    }
    name = ((Node_Name *)nc->left)->name;
    /* iterate through the variables */
    for (p = namespace->globals; p != NULL; p = p->next){
      if (!strcmp(p->var->name, name)){
        found = true;
        var = p->var;
        break;
      }
    }
    /* error if the variable was not found */
    if (!found){
      NmError_Parser(n, "variable '%s' was not found", name);
      Nm_Exit();
    }
    /* check for the flags, eg. the NM_VAR_FLAG_CONST flag causes the variable
     * to be not-assignable*/
    if (NmVar_GETFLAG(var, NMVAR_FLAG_CONST)){
      NmError_Parser(n, "cannot change the value of a constant variable '%s'", name);
      Nm_Exit();
    }
    /* actually assign the value */
    ret = NmAST_Exec(nc->right);
    var->value = ret;

    return ret;
  }
  /*
   * XXX BINARY_INDEX
   */
  else if (nc->op == BINARY_INDEX){
    NmObject *ob_left = NmAST_Exec(left);
    NmObject *ob_right = NmAST_Exec(right);
    if (!ob_left->fn.binary.index){
      NmError_Parser(n, "invalid binary operator '[]' for type '%s'", NmString_VAL(ob_left->fn.type_repr()));
      Nm_Exit();
    }
    if (ob_right->type != OT_INTEGER){
      NmError_Parser(n, "expected type 'int' for the indexing value");
      Nm_Exit();
    }

    return ob_left->fn.binary.index(ob_left, ob_right);
  }
  /*
   * XXX BINARY_COMMA
   */
  else if (nc->op == BINARY_COMMA){
    /* discarding the left's value */
    NmAST_Exec(left);
    /* returning the right's value */
    return NmAST_Exec(right);
  }

/* a handy macro to check if an object supports given binary operation
 * <func>, and if not, print an error, and exit
 *
 * <ob> is of type { NmObject * }
 * <func> is pretty much of type { void *(*)(NmObject *, NmObject *) }
 *
 * Note: both left and right operands should be of the same type */
#define ensure_ob_has_func(FUNC) \
  if (!ob_left->fn.binary.FUNC){ \
    NmError_Parser(n, "invalid binary operation %s for types '%s' and '%s'", binopToS(nc->op), NmString_VAL(ob_left->fn.type_repr()), NmString_VAL(ob_right->fn.type_repr())); \
    Nm_Exit(); \
  }

/* <TYPE> is of type { BinaryOp }
 * <FUNC> is of type { BinaryFunc } */
#define op(TYPE, FUNC) \
  case TYPE: \
  { \
    ensure_ob_has_func(FUNC); \
\
    ret = ob_left->fn.binary.FUNC(ob_left, ob_right); \
    /* if a binop function returns NULL it means something bad happend */ \
    if (!ret){ \
      NmError_Parser(n, NmError_GetCurr()); \
      Nm_Exit(); \
    } \
    break; \
  }

#define cmpop(TYPE, CMPRES) \
  case TYPE: { \
    ensure_ob_has_func(cmp); \
\
    ret = ob_left->fn.binary.cmp(ob_left, ob_right) == CMPRES ? \
          NmInt_New(1) : NmInt_New(0); \
    break; \
  }

/* DRY */
#define binary_ops() \
  switch (nc->op){ \
    /* here are all the binary functions that are available */ \
    op(BINARY_ADD, add); \
    op(BINARY_SUB, sub); \
    op(BINARY_MUL, mul); \
    op(BINARY_DIV, div); \
    op(BINARY_MOD, mod); \
    cmpop(BINARY_LT, CMP_LT); \
    cmpop(BINARY_GT, CMP_GT); \
    cmpop(BINARY_EQ, CMP_EQ); \
    case BINARY_LE: { \
      ensure_ob_has_func(cmp); \
\
      ret = ob_left->fn.binary.cmp(ob_left, ob_right) == CMP_EQ || \
            ob_left->fn.binary.cmp(ob_left, ob_right) == CMP_LT ? \
            NmInt_New(1) : NmInt_New(0); \
      break; \
    } \
    case BINARY_GE: { \
      ensure_ob_has_func(cmp); \
\
      ret = ob_left->fn.binary.cmp(ob_left, ob_right) == CMP_EQ || \
            ob_left->fn.binary.cmp(ob_left, ob_right) == CMP_GT ? \
            NmInt_New(1) : NmInt_New(0); \
      break; \
    } \
    case BINARY_NE: { \
      ensure_ob_has_func(cmp); \
\
      ret = ob_left->fn.binary.cmp(ob_left, ob_right) == CMP_GT || \
            ob_left->fn.binary.cmp(ob_left, ob_right) == CMP_LT ? \
            NmInt_New(1) : NmInt_New(0); \
      break; \
    } \
    default: \
      NmError_Parser(n, "invalid types '%s' and '%s' for binary operation %s", NmString_VAL(ob_left->fn.type_repr()), NmString_VAL(ob_right->fn.type_repr()), binopToS(nc->op)); \
    Nm_Exit(); \
  }

  NmObject *ob_left = NmAST_Exec(left);
  NmObject *ob_right = NmAST_Exec(right);

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
      NmError_Parser(n, "invalid types '%s' and '%s' for binary operation %s", NmString_VAL(ob_left->fn.type_repr()), NmString_VAL(ob_right->fn.type_repr()), binopToS(nc->op));
      Nm_Exit();
    }
  }

#undef op
#undef ensure_ob_has_func

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

  NmAST_Free(((Node_Binop *)n)->left);
  NmAST_Free(((Node_Binop *)n)->right);

#if DEBUG
  NmDebug_AST(n, "free binary operation node");
#endif
  NmMem_Free(n);
}

/*
 * @name - NmAST_GenUnop
 * @desc - creates a n for unary operation
 */
Node *NmAST_GenUnop(Pos pos, Node *target, UnaryOp op)
{
  Node_Unop *n = NmMem_Calloc(1, sizeof(Node_Unop));

  n->type = NT_UNOP;
  n->op = op;
  n->target = target;
  INIT_POS();

#if DEBUG
  NmDebug_AST(n, "create unary operation node (op: %s, expr: %p)", unopToS(op), (void*)target);
#endif

  return (Node *)n;
}

/*
 * @name - NmAST_ExecUnop
 * @desc - return the result of the unary operation
 */
NmObject *NmAST_ExecUnop(Node *n)
{
  Node_Unop *nc = (Node_Unop *)n;
  NmObject *ret;

  NmObject *target = NmAST_Exec(nc->target);

#if DEBUG
  NmDebug_AST(n, "execute unary operation node");
#endif

/* a handy macro to check if an object supports given unary operation
 * <func>, and if not, print an error, and exit
 *
 * <ob> is of type { NmObject * }
 * <func> is of type { UnaryFunc }
 */
#define ensure_ob_has_func(FUNC) \
  if (!target->fn.unary.FUNC){ \
    NmError_Parser(n, "invalid type '%s' for unary operator %s", NmString_VAL(target->fn.type_repr()), unopToS(nc->op)); \
    Nm_Exit(); \
  }

/* <TYPE> is of type { UnaryOp }
 * <FUNC> is of type { UnaryFunc } */
#define op(TYPE, FUNC) \
  case TYPE: { \
    ensure_ob_has_func(FUNC); \
\
    ret = target->fn.unary.FUNC(target); \
    break; \
  }

  switch (nc->op){
    op(UNARY_PLUS, plus);
    op(UNARY_MINUS, minus);
    op(UNARY_NEGATE, negate);
    case UNARY_PREINC: {
      ensure_ob_has_func(increment);
      ret = target->fn.unary.increment(target);
      break;
    }
    case UNARY_PREDEC: {
      ensure_ob_has_func(decrement);
      ret = target->fn.unary.decrement(target);
      break;
    }
    case UNARY_POSTINC: {
      ensure_ob_has_func(increment);
      NmObject *save = NmObject_Dup(target);
      target->fn.unary.increment(target);
      ret = save;
      break;
    }
    case UNARY_POSTDEC: {
      ensure_ob_has_func(decrement);
      NmObject *save = NmObject_Dup(target);
      target->fn.unary.decrement(target);
      ret = save;
      break;
    }
    default:
      NmError_Parser(n, "invalid unary operator %s for type '%s'", unopToS(nc->op), NmString_VAL(target->fn.type_repr()));
      Nm_Exit();
  }

#undef op
#undef ensure_ob_has_func

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

  NmAST_Free(((Node_Unop *)n)->target);

#if DEBUG
  NmDebug_AST(n, "free unary operation node");
#endif
  NmMem_Free(n);
}

/*
 * @name - NmAST_GenIf
 * @desc - creates a n for the if statement
 *         <guard>, <body> and <elsee> can be NULL, it means a NOP then
 */
Node *NmAST_GenIf(Pos pos, Node *guard, Node *body, Node *elsee, bool unless)
{
  Node_If *n = NmMem_Calloc(1, sizeof(Node_If));

  n->type = NT_IF;
  n->guard = guard;
  n->body = body;
  n->elsee = elsee;
  n->unless = unless;
  INIT_POS();

#if DEBUG
  if (unless)
    NmDebug_AST(n, "create unless node (guard: %p, body: %p, else: %p)", (void*)guard, (void*)body, (void*)elsee);
  else
    NmDebug_AST(n, "create if node (guard: %p, body: %p, else: %p)", (void*)guard, (void*)body, (void*)elsee);
#endif

  return (Node *)n;
}

/*
 * @name - NmAST_ExecIf
 * @desc - do the if loop, return actually anything
 */
NmObject *NmAST_ExecIf(Node *n)
{
  Node_If *nc = (Node_If *)n;
  Node *guard = nc->guard;
  Node *body  = nc->body;
  Node *elsee = nc->elsee;
  bool unless = nc->unless;

#if DEBUG
  if (unless)
    NmDebug_AST(n, "execute unless node");
  else
    NmDebug_AST(n, "execute if node");
#endif

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

  Node_If *nc = (Node_If *)n;

  /* guard in if is optional */
  if (nc->guard)
    NmAST_Free(nc->guard);
  /* so is it's body */
  if (nc->body)
    NmAST_Free(nc->body);
  /* aaaaaand the else */
  if (nc->elsee)
    NmAST_Free(nc->elsee);

#if DEBUG
  if (nc->unless)
    NmDebug_AST(n, "free unless node");
  else
    NmDebug_AST(n, "free if node");
#endif

  NmMem_Free(n);
}

/*
 * @name - NmAST_GenWhile
 * @desc - creates a while n
 *         <guard>, <body> and <elsee> are optional, can be NULL,
 *         it means then that they are NOPs
 *         <elsee> here gets evaluated when the while loop didn't run even once
 */
Node *NmAST_GenWhile(Pos pos, Node *guard, Node *body, Node *elsee, bool until)
{
  Node_While *n = NmMem_Calloc(1, sizeof(Node_While));

  n->type = NT_WHILE;
  n->guard = guard;
  n->body = body;
  n->elsee = elsee;
  n->until = until;
  INIT_POS();

#if DEBUG
  if (until)
    NmDebug_AST(n, "create until node (guard: %p, body: %p, else: %p)", (void*)guard, (void*)body, (void*)elsee);
  else
    NmDebug_AST(n, "create while node (guard: %p, body: %p, else: %p)", (void*)guard, (void*)body, (void*)elsee);
#endif

  return (Node *)n;
}

/*
 * @name - NmAST_ExecWhile
 * @desc - execute the loop and return, actually anything, it's a statement
 */
NmObject *NmAST_ExecWhile(Node *n)
{
  Node_While *nc = (Node_While *)n;
  Node *guard    = nc->guard;
  Node *body     = nc->body;
  Node *elsee    = nc->elsee;
  bool until     = nc->until;

#if DEBUG
  if (until)
    NmDebug_AST(n, "execute until node");
  else
    NmDebug_AST(n, "execute while node");
#endif

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

  Node_While *nc = (Node_While *)n;

  /* guard in while is optional */
  if (nc->guard)
    NmAST_Free(nc->guard);
  /* so is it's body */
  if (nc->body)
    NmAST_Free(nc->body);
  /* aaaaaand the else */
  if (nc->elsee)
    NmAST_Free(nc->elsee);

#if DEBUG
  if (nc->until)
    NmDebug_AST(n, "free until node");
  else
    NmDebug_AST(n, "free while node");
#endif
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
  Node_Decl *n = NmMem_Calloc(1, sizeof(Node_Decl));
  Namespace *namespace = NmNamespace_GetCurr();

  n->type = NT_DECL;
  n->name = NmMem_Strdup(name);
  n->value = value;
  n->flags = flags;
  INIT_POS();

#if DEBUG
  NmDebug_AST(n, "create variable declaration node (name: %s, namespace: %s)", name, namespace->name);
#endif

  VariablesList *new_list = NmMem_Malloc(sizeof(VariablesList));
  Variable *new_var = NmMem_Malloc(sizeof(Variable));
  VariablesList *p;

  /* before doing anything, check if that variable was already declared */
  for (p = namespace->globals; p != NULL; p = p->next){
    if (!strcmp(p->var->name, name)){
      NmError_Parser((Node *)n, "global variable '%s' already declared", name);
      Nm_Exit();
    }
  }

  new_var->name = NmMem_Strdup(name);
  if (n->value){
    NmObject *value = NmAST_Exec(n->value);
    new_var->value = value;
  } else {
    /* declared variables get to be a integer with the value of 0 */
    new_var->value = NmInt_New(0);
  }
  new_var->flags = n->flags;
  /* append to the globals list */
  new_list->var = new_var;
  new_list->next = namespace->globals;
  namespace->globals = new_list;

  return (Node *)n;
}

/*
 * @name - NmAST_ExecDecl
 * @desc - declare/define the variable and return 1
 */
NmObject *NmAST_ExecDecl(Node *n)
{
  /* unused parameter */
  (void)n;

#if DEBUG
  NmDebug_AST(n, "execute variable declaration node");
#endif

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

  Node_Decl *nc = (Node_Decl *)n;

  NmMem_Free(nc->name);
  /* initialized value is optional */
  if (nc->value)
    NmAST_Free(nc->value);

#if DEBUG
  NmDebug_AST(n, "free declaration node");
#endif
  NmMem_Free(n);
}

/*
 * @name - NmAST_GenCall
 * @desc - creates a n for calling a function of a given <name>
 *         parameter <params> is optional, may be NULL, then it means
 *         that no parameters have been passed
 */
Node *NmAST_GenCall(Pos pos, char *name, Node **params, char *opts, struct Namespace *namespace)
{
  Node_Call *n = NmMem_Calloc(1, sizeof(Node_Call));

  n->type = NT_CALL;
  n->name = NmMem_Strdup(name);
  n->params = params;
  n->opts = NmMem_Strdup(opts);
  n->namespace = namespace;
  INIT_POS();

#if DEBUG
  NmDebug_AST(n, "create call node (name: %s, params: %p, opts: '%s')", name, params, opts);
#endif

  return (Node *)n;
}

/*
 * @name - NmAST_ExecCall
 * @desc - call the given function
 */
NmObject *NmAST_ExecCall(Node *n)
{
  Node_Call *nc = (Node_Call *)n;
  NmObject *ret = NmNull;
  char *name = nc->name;

#if DEBUG
  NmDebug_AST(n, "execute function call node");
#endif

  Namespace *namespaces[3], *namespace;
  namespaces[0] = NmNamespace_GetByName("core");
  namespaces[1] = nc->namespace;
  namespaces[2] = NULL;

  for (int i = 0; namespaces[i] != NULL; i++){
    namespace = namespaces[i];
    /* first check for the C functions */
    for (CFuncsList *list = namespace->cfuncs; list != NULL; list = list->next){
      if (!strcmp(list->func->name, name)){
        size_t nmemb = 0;
        size_t i = 0;
        /* count how many elements there are */
        if (nc->params)
          for (Node **p = nc->params; *p != NULL; p++)
            nmemb++;
        /* parameters are stored as an array */
        NmObject *array = NmArray_New(nmemb);
        /* set the arrays elements */
        if (nc->params){
          /* but first, type checking
           * if the function takes -1 then every argument should of type
           * <list->func->types[0]>, if the number of arguments is >= 0 then
           * every argument should be of the matching element in
           * <list->func->types> */
          for (Node **p = nc->params; *p != NULL; p++, i++){
            NmObjectType type;
            if (list->func->argc < 0){
              type = list->func->types[0];
            } else {
              type = list->func->types[i];
            }
            /* actually check the type of the current argument */
            NmObject *ob = NmAST_Exec(*p);
            if (!(ob->type & type)){
              NmError_Parser(n, "wrong argument's #%d type for the function '%s' (%s given when %s expected)", i, name, NmString_VAL(ob->fn.type_repr()), NmString_VAL(NmObject_TypeToS(type)));
              Nm_Exit();
              return NULL;
            }
            NmArray_SETELEM(array, i, ob);
          }
        }
        /* now, let's check what options were passed */
        bool opts[strlen(nc->opts)];
        /* false-out the options */
        memset(opts, 0, sizeof(opts));
        /* actually set the options */
        unsigned j = 0;
        for (char *p = nc->opts; *p != '\0'; p++, j++)
          /* NOTE: we are not checking if the option is not supported, or if
           *       there are too many options or w/e because the parser already
           *       did it */
          if (strchr(list->func->opts, *p))
            opts[j] = true;
        /* execute the function */
        ret = list->func->body(array, (bool *)&opts);
        /* if a function returns NULL it means something went wrong */
        if (ret == NULL){
          NmError_Parser(n, NmError_GetCurr());
          Nm_Exit();
          return NULL;
        } else {
          return ret;
        }
      }
    }
    /* and then for the Nemo functions */
    for (FuncsList *list = namespace->funcs; list != NULL; list = list->next){
      if (!strcmp(list->func->name, name)){
        if (!list->func->body){
          NmError_Parser(n, "cannot call function '%s.%s' which was only declared but not defined", namespace->name, name);
          Nm_Exit();
          return NULL;
        }
        /* let's see if the types match */
        /* it should happen at compile time, though */
        bool wrong_types = false;
        for (unsigned i = 0; i < list->func->argc; i++){
          NmObject *param = NmAST_Exec(nc->params[i]);
          if (list->func->argv[i] != param->type){
            NmError_Parser(n, "wrong argument's #%d type for the function '%s' (%s given when %s expected)", i, name, NmString_VAL(NmObject_TypeToS(param->type)), NmString_VAL(NmObject_TypeToS(list->func->argv[i])));
            wrong_types = true;
          }
        }

        if (wrong_types){
          Nm_Exit();
          return NULL;
        }
        /* execute the functions body */
        ret = NmAST_Exec(list->func->body);
        /* if a function returns NULL it means something went wrong */
        if (ret == NULL){
          NmError_Parser(n, "executing function '%s' went wrong", name);
          Nm_Exit();
          return NULL;
        } else {
          return ret;
        }
      }
    }
  }

  NmError_Parser(n, "function '%s' not found", name);
  Nm_Exit();
  return NULL;
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

  Node_Call *nc = (Node_Call *)n;

  NmMem_Free(nc->name);
  /* parameters are optional */
  if (nc->params){
    for (i = 0; nc->params[i] != NULL; i++){
      NmAST_Free(nc->params[i]);
    }
#if DEBUG
    NmDebug_AST(nc->params, "free params list");
#endif
    NmMem_Free(nc->params);
  }

  NmMem_Free(nc->opts);
#if DEBUG
  NmDebug_AST(n, "free call node");
#endif
  NmMem_Free(n);
}

/*
 * @name - NmAST_GenStmt
 * @desc - create a statement node, but with only one expression
 */
Node *NmAST_GenStmt(Pos pos, Node *expr)
{
  Node_Stmt *n = NmMem_Calloc(1, sizeof(Node_Stmt));

  n->next = expr->next;
  n->type = NT_STMT;
  n->expr = expr;
  INIT_POS();

#if DEBUG
  NmDebug_AST(n, "create statement node (expr: %p)", expr);
#endif

  return (Node *)n;
}

NmObject *NmAST_ExecStmt(Node *n)
{
  return NmAST_Exec(((Node_Stmt *)n)->expr);
}

void NmAST_FreeStmt(Node *n)
{
  assert(n);
  assert(n->type == NT_STMT);

#if DEBUG
  NmDebug_AST(n, "free statement node");
#endif

  NmAST_Free(((Node_Stmt *)n)->expr);
  NmMem_Free(n);
}

NmObject *NmAST_ExecBlock(Node *n)
{
  Node_Block *nc = (Node_Block *)n;
  NmObject *ret = NmNull;
  Statement *s;
  Statement *next;

  assert(n);
  assert(n->type == NT_BLOCK);

#if DEBUG
  NmDebug_AST(n, "execute block node");
#endif

  for (s = nc->tail; s != NULL; s = next){
    next = s->next;
    if (s->stmt){
#if DEBUG
      NmDebug_AST(s->stmt, "execute statement node");
#endif
      ret = NmAST_Exec(s->stmt);
    }
  }

#if DEBUG
  NmDebug_AST(n, "done executing block node");
#endif

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
  Node_Block *nc = (Node_Block *)n;

  assert(n);
  assert(n->type == NT_BLOCK);

  for (s = nc->tail; s != NULL; s = next){
    next = s->next;
    NmAST_Free(s->stmt);
#if DEBUG
    NmDebug_AST(n, "free statement node");
#endif
    NmMem_Free(s);
  }

#if DEBUG
  NmDebug_AST(n, "free block node");
#endif
  NmMem_Free(n);
}

/*
 * @name - NmAST_GenFuncDef
 * @desc - creates a node for defining a function of a given <name>
 */
Node *NmAST_GenFuncDef(Pos pos, char *name, Node *body,
                       unsigned argc, unsigned optc,
                       NmObjectType *argv, char *opts)
{
  Namespace *namespace = NmNamespace_GetCurr();
  FuncsList *l = NmMem_Malloc(sizeof(FuncsList));
  Func *f = NmMem_Malloc(sizeof(Func));
  Node_Funcdef *n = NmMem_Calloc(1, sizeof(Node_Funcdef));

  n->type = NT_FUNCDEF;
  n->name = NmMem_Strdup(name);
  n->argc = argc;
  n->optc = optc;
  n->argv = argv;
  n->opts = opts;
  n->body = body;
  INIT_POS();

#if DEBUG
  if (body)
    NmDebug_AST(n, "create function definition node (name: %s.%s, body: %p, argc: %d, optc: %d, opts: '%s')", namespace->name, name, (void*)body, argc, optc, opts);
  else
    NmDebug_AST(n, "create function declaration node (name: %s.%s, argc: %d, optc: %d, namespace: '%s')", namespace->name, name, argc, optc);
#endif


  /* initialize */
  f->name = NmMem_Strdup(name);
  f->body = body;
  f->argc = argc;
  f->argv = argv;
  f->opts = opts;
  l->func = f;
  /* append the function */
  l->next = namespace->funcs;
  namespace->funcs = l;

  return (Node *)n;
}

/*
 * @name - NmAST_ExecFuncDef
 * @desc - declare/define given function
 */
NmObject *NmAST_ExecFuncDef(Node *n)
{
  Node_Funcdef *nc = (Node_Funcdef *)n;

#if DEBUG
  if (nc->body)
    NmDebug_AST(n, "execute function definition node");
  else
    NmDebug_AST(n, "execute function declaration node");
#endif

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

  Node_Funcdef *nc = (Node_Funcdef *)n;

  NmMem_Free(nc->name);
  if (nc->body){
    NmAST_Free(nc->body);
#if DEBUG
    NmDebug_AST(n, "free function definition node");
#endif
  } else {
#if DEBUG
    NmDebug_AST(n, "free function declaration node");
#endif
  }
  NmMem_Free(nc->argv);
  NmMem_Free(nc->opts);
  NmMem_Free(n);
}

/*
 * @name - NmAST_GenUse
 * @desc - creates a node that creates does the including thing
 */
Node *NmAST_GenUse(Pos pos, char *fname)
{
  Node_Use *n = NmMem_Calloc(1, sizeof(Node_Use));

  n->type = NT_USE;
  n->fname = NmMem_Strdup(fname);
  INIT_POS();

#if DEBUG
  NmDebug_AST(n, "create use node (fname: %s)", fname);
#endif

  if (!Nm_UseModule(fname)){
    NmError_Parser((Node *)n, NmError_GetCurr());
    Nm_Exit();
  }

  return (Node *)n;
}

/*
 * @name - NmAST_ExecUse
 * @desc - actually does the including thing
 * @return 1 if everything went fine
 */
NmObject *NmAST_ExecUse(Node *n)
{
  assert(n);
  assert(n->type == NT_USE);

  return NmInt_New(1);
}

void NmAST_FreeUse(Node *n)
{
  assert(n);
  assert(n->type == NT_USE);

  Node_Use *nc = (Node_Use *)n;

  NmMem_Free(nc->fname);
  NmMem_Free(nc->custom_path);
}

const char *binopToS(BinaryOp op)
{
  switch (op){
    case BINARY_GT:         return "'>'";
    case BINARY_LT:         return "'<'";
    case BINARY_GE:         return "'>='";
    case BINARY_LE:         return "'<='";
    case BINARY_EQ:         return "'=='";
    case BINARY_NE:         return "'!='";
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
    case BINARY_COMMA:      return "','";
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

