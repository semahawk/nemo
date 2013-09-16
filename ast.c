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
static Nob *(*execFuncs[])(Node *) =
{
  /* XXX order must match the one in "enum NodeType" in ast.h */
  nm_ast_exec_nop,
  nm_ast_exec_int,
  nm_ast_exec_float,
  nm_ast_exec_str,
  nm_ast_exec_arr,
  nm_ast_exec_name,
  nm_ast_exec_binop,
  nm_ast_exec_unop,
  nm_ast_exec_if,
  nm_ast_exec_while,
  nm_ast_exec_decl,
  nm_ast_exec_call,
  nm_ast_exec_stmt,
  nm_ast_exec_block,
  nm_ast_exec_funcdef,
  nm_ast_exec_use
};

/*
 * Array of pointer functions responsible for freeing an adequate kind of node
 */
static void (*freeFuncs[])(Node *) =
{
  /* XXX order must match the one in "enum NodeType" in ast.h */
  nm_ast_free_nop,
  nm_ast_free_int,
  nm_ast_free_float,
  nm_ast_free_str,
  nm_ast_free_arr,
  nm_ast_free_name,
  nm_ast_free_binop,
  nm_ast_free_unop,
  nm_ast_free_if,
  nm_ast_free_while,
  nm_ast_free_decl,
  nm_ast_free_call,
  nm_ast_free_stmt,
  nm_ast_free_block,
  nm_ast_free_funcdef,
  nm_ast_free_use
};

/*
 * @name - nm_ast_exec_nodes
 * @desc - executes all the statements depending on each node's "next" field
 * @return - the last executed statement's value
 * @param - nodest - the first statement to be executed
 */
Nob *nm_ast_exec_nodes(Node *nodest)
{
  /* TODO
   *
   * That one is going to be using the famous "ip", for instance.
   */
  (void)nodest;

  return null;
}

/*
 * @name - nm_ast_exec
 * @desc - executes given node and returns the { Nob * } it resulted in
 */
static inline Nob *nm_ast_exec(Node *n)
{
  assert(n);

  return execFuncs[n->type] ? execFuncs[n->type](n) : null;
}

/*
 * @name - nm_ast_free
 * @desc - runs an appropriate function, that will free given <n>
 */
void nm_ast_free(Node *n)
{
  assert(n);

  freeFuncs[n->type](n);
}

/*
 * @name - nm_ast_gen_nop
 * @desc - create a n that does NOTHING
 */
Node *nm_ast_gen_nop(Pos pos)
{
  Node *n = ncalloc(1, sizeof(Node));

  n->type = NT_NOP;
  INIT_POS();

#if DEBUG
  nm_debug_ast(n, "create NOP node");
#endif

  return n;
}

/*
 * @name - nm_ast_exec_nop
 * @desc - execute a NOP (eg, return null)
 */
Nob *nm_ast_exec_nop(Node *n)
{
  /* unused parameter */
  (void)n;

  return null;
}

/*
 * @name - nm_ast_free_nop
 * @desc - frees the NOP n
 */
void nm_ast_free_nop(Node *n)
{
  assert(n);
  assert(n->type == NT_NOP);

#if DEBUG
  nm_debug_ast(n, "free NOP node");
#endif

  nfree(n);
}

/*
 * @name - nm_ast_gen_int
 * @desc - create a n holding a single literal integer
 */
Node *nm_ast_gen_int(Pos pos, int i)
{
  Node_Int *n = ncalloc(1, sizeof(Node_Int));

  n->type = NT_INTEGER;
  n->i = i;
  INIT_POS();

#if DEBUG
  nm_debug_ast(n, "create int node (value: %d)", i);
#endif

  return (Node *)n;
}

/*
 * @name - nm_ast_exec_int
 * @desc - return the value of the int
 */
Nob *nm_ast_exec_int(Node *n)
{
#if DEBUG
  nm_debug_ast(n, "execute integer node");
#endif

  return nm_new_int(((Node_Int *)n)->i);
}

/*
 * @name - nm_ast_free_int
 * @desc - responsible for freeing literal integer ns
 */
void nm_ast_free_int(Node *n)
{
  assert(n);
  assert(n->type == NT_INTEGER);

#if DEBUG
  nm_debug_ast(n, "free int node");
#endif

  nfree(n);
}

/*
 * @name - nm_ast_gen_float
 * @desc - create a n holding a single literal float
 */
Node *nm_ast_gen_float(Pos pos, float f)
{
  Node_Float *n = ncalloc(1, sizeof(Node_Float));

  n->type = NT_FLOAT;
  n->f = f;
  INIT_POS();

#if DEBUG
  nm_debug_ast(n, "create float node (value: %f)", f);
#endif

  return (Node *)n;
}

/*
 * @name - nm_ast_exec_float
 * @desc - return the value of the float
 */
Nob *nm_ast_exec_float(Node *n)
{
#if DEBUG
  nm_debug_ast(n, "execute float node");
#endif

  return nm_new_float(((Node_Float *)n)->f);
}

/*
 * @name - nm_ast_free_float
 * @desc - responsible for freeing literal float ns
 */
void nm_ast_free_float(Node *n)
{
  assert(n);
  assert(n->type == NT_FLOAT);

#if DEBUG
  nm_debug_ast(n, "free float node");
#endif

  nfree(n);
}

/*
 * @name - nm_ast_gen_str
 * @desc - create a node holding a single literal string
 */
Node *nm_ast_gen_str(Pos pos, char *s)
{
  Node_String *n = ncalloc(1, sizeof(Node_String));

  n->type = NT_STRING;
  n->s = nm_strdup(s);
  INIT_POS();

#if DEBUG
  nm_debug_ast(n, "create string node (value: \"%s\")", s);
#endif

  return (Node *)n;
}

/*
 * @name - nm_ast_exec_str
 * @desc - return the value of the string
 */
Nob *nm_ast_exec_str(Node *n)
{
  Nob *ret = nm_new_str(((Node_String *)n)->s);

#if DEBUG
  nm_debug_ast(n, "execute string node");
#endif

  if (!ret){
    nm_parser_error(n, nm_curr_error());
    nexit();
    return NULL;
  }

  return nm_new_str(((Node_String *)n)->s);
}

/*
 * @name - nm_ast_free_str
 * @desc - responsible for freeing literal string node
 */
void nm_ast_free_str(Node *n)
{
  assert(n);
  assert(n->type == NT_STRING);

#if DEBUG
  nm_debug_ast(n, "free string node");
#endif

  nfree(((Node_String *)n)->s);
  nfree(n);
}

/*
 * @name - nm_ast_gen_arr
 * @desc - create a node holding a literal array
 */
Node *nm_ast_gen_arr(Pos pos, Node **a)
{
  Node_Array *n = ncalloc(1, sizeof(Node_Array));
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
  nm_debug_ast(n, "create array node (nmemb: %u, values: %p)", nmemb, (void *)a);
#endif

  return (Node *)n;
}

/*
 * @name - nm_ast_exec_arr
 * @desc - return the value of the string
 */
Nob *nm_ast_exec_arr(Node *n)
{
  Node_Array *n_arr = (Node_Array *)n;
  Nob *ob = nm_new_arr(n_arr->nmemb);
  size_t i = 0;

#if DEBUG
  nm_debug_ast(n, "execute array node");
#endif

  if (n_arr->a)
    for (Node **p = n_arr->a; *p != NULL; p++, i++)
      nm_arr_set_elem(ob, i, nm_ast_exec(*p));

  return ob;
}

/*
 * @name - nm_ast_free_arr
 * @desc - responsible for freeing literal string node
 */
void nm_ast_free_arr(Node *n)
{
  assert(n);
  assert(n->type == NT_ARRAY);

#if DEBUG
  nm_debug_ast(n, "free array node");
#endif

  if (((Node_Array *)n)->a)
    for (Node **p = ((Node_Array *)n)->a; *p != NULL; p++)
      nm_ast_free(*p);

  nfree(((Node_Array *)n)->a);
  nfree(n);
}

/*
 * @name - nm_ast_gen_name
 * @desc - creates a (eg. variable) name n
 */
Node *nm_ast_gen_name(Pos pos, char *name, struct Namespace *node_namespace)
{
  Node_Name *n = ncalloc(1, sizeof(Node_Name));
  bool found = false;
  Namespace *curr_namespace = nm_curr_namespace();

  n->type = NT_NAME;
  n->name = nm_strdup(name);
  n->namespace = node_namespace;
  INIT_POS();

#if DEBUG
  nm_debug_ast(n, "create name node (name: %s)", name);
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
          nm_parser_error((Node *)n, "can't access variable '%s.%s' which was declared as private", node_namespace->name, name);
          nexit();
        }
      } else {
        /* we found the variable, and it's public and accessible */
        found = true;
        break;
      }
    }
  }

  /*if (!found){*/
    /*nm_parser_error((Node *)n, "variable '%s.%s' was not found", node_namespace->name, name);*/
    /*nexit();*/
    /*return NULL;*/
  /*}*/

  return (Node *)n;
}

/*
 * @name - nm_ast_exec_name
 * @desc - return the value the name is carring
 */
Nob *nm_ast_exec_name(Node *n)
{
  Node_Name *nc = (Node_Name *)n;

#if DEBUG
  nm_debug_ast(n, "execute name node (name: %s)", nc->name);
#endif

  /* search for the variable */
  for (VariablesList *vars = nc->namespace->globals; vars != NULL; vars = vars->next)
    if (!strcmp(vars->var->name, nc->name))
      return vars->var->value;

  nm_parser_error(n, "variable '%s.%s' was not found", nc->namespace->name, nc->name);
  return NULL;
}

/*
 * @name - nm_ast_free_name
 * @desc - responsible for freeing (eg. variable) name ns
 */
void nm_ast_free_name(Node *n)
{
  assert(n);
  assert(n->type == NT_NAME);

#if DEBUG
  nm_debug_ast(n, "free name node");
#endif
  nfree(((Node_Name *)n)->name);
  nfree(n);
}

/*
 * @name - nm_ast_gen_binop
 * @desc - creates a binary operation n
 */
Node *nm_ast_gen_binop(Pos pos, Node *left, BinaryOp op, Node *right)
{
  Node_Binop *n = ncalloc(1, sizeof(Node_Binop));

  n->type = NT_BINOP;
  n->op = op;
  n->left = left;
  n->right = right;
  INIT_POS();

#if DEBUG
  nm_debug_ast(n, "create binary operation node (op: %s, left: %p, right: %p)", binopToS(op), (void*)left, (void*)right);
#endif

  return (Node *)n;
}

/*
 * @name - nm_ast_exec_binop
 * @desc - return the result of the binary operation
 */
Nob *nm_ast_exec_binop(Node *n)
{
  Node_Binop *nc = (Node_Binop *)n;
  Namespace *namespace = nm_curr_namespace();

  Node *left  = nc->left;
  Node *right = nc->right;

  Nob *ret = null;

#if DEBUG
  nm_debug_ast(n, "execute binary operation node");
#endif

  /* okay, that's easy things here */
  /*
   * XXX BINARY_ASSIGN
   */
  if (nc->op == BINARY_ASSIGN){
    Variable *var = NULL;
    VariablesList *p;
    char *name;
    bool found = false;
    /* left-hand side of the assignment must be a name
     * (at least for now (30 Apr 2013)) */
    if (nc->left->type != NT_NAME){
      nm_parser_error(n, "expected an lvalue in assignment");
      nexit();
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
      nm_parser_error(n, "variable '%s' was not found", name);
      nexit();
    }
    /* check for the flags, eg. the NM_VAR_FLAG_CONST flag causes the variable
     * to be not-assignable*/
    if (nm_var_get_flag(var, NMVAR_FLAG_CONST)){
      nm_parser_error(n, "cannot change the value of a constant variable '%s'", name);
      nexit();
    }
    /* actually assign the value */
    ret = nm_ast_exec(nc->right);
    var->value = ret;

    return ret;
  }
  /*
   * XXX BINARY_INDEX
   */
  else if (nc->op == BINARY_INDEX){
    Nob *ob_left = nm_ast_exec(left);
    Nob *ob_right = nm_ast_exec(right);
    BinaryFunc binaryfunc = nm_obj_has_binary_func(ob_left, BINARY_INDEX);
    if (!binaryfunc){
      nm_parser_error(n, "invalid binary operator '[]' for type '%s'", nm_str_value(nm_obj_typetos(ob_left)));
      nexit();
    }
    if (ob_right->type != OT_INTEGER){
      nm_parser_error(n, "expected type 'int' for the indexing value");
      nexit();
    }

    return binaryfunc(ob_left, ob_right);
  }
  /*
   * XXX BINARY_COMMA
   */
  else if (nc->op == BINARY_COMMA){
    /* discarding the left's value */
    nm_ast_exec(left);
    /* returning the right's value */
    return nm_ast_exec(right);
  }

/* a handy macro to check if an object supports given binary operation
 * <func>, and if not, print an error, and exit
 *
 * Note: both left and right operands should be of the same type */
#define ensure_ob_has_binop_func(TYPE) \
  BinaryFunc binaryfunc = nm_obj_has_binary_func(ob_left, TYPE); \
  if (!binaryfunc){ \
    nm_parser_error(n, "invalid binary operation %s for types '%s' and '%s'", binopToS(nc->op), nm_str_value(nm_obj_typetos(ob_left)), nm_str_value(nm_obj_typetos(ob_right))); \
    nexit(); \
  }

/* <TYPE> is of type { BinaryOp } */
#define op(TYPE) \
  case TYPE: \
  { \
    ensure_ob_has_binop_func(TYPE); \
\
    ret = binaryfunc(ob_left, ob_right); \
    /* if a binop function returns NULL it means something bad happend */ \
    if (!ret){ \
      nm_parser_error(n, nm_curr_error()); \
      nexit(); \
    } \
    break; \
  }

#define ensure_ob_has_cmp_func(TYPE) \
  CmpFunc cmpfunc = nm_obj_has_cmp_func(ob_left, TYPE); \
  if (!cmpfunc){ \
    nm_parser_error(n, "invalid binary operation %s for types '%s' and '%s'", binopToS(nc->op), nm_str_value(nm_obj_typetos(ob_left)), nm_str_value(nm_obj_typetos(ob_right))); \
    nexit(); \
  }

#define cmpop(TYPE, CMPRES) \
  case TYPE: { \
    ensure_ob_has_cmp_func(TYPE); \
\
    ret = cmpfunc(ob_left, ob_right) == CMPRES ? \
          nm_new_int(1) : nm_new_int(0); \
    break; \
  }

/* DRY */
#define binary_ops() \
  switch (nc->op){ \
    /* here are all the binary functions that are available */ \
    op(BINARY_ADD); \
    op(BINARY_SUB); \
    op(BINARY_MUL); \
    op(BINARY_DIV); \
    op(BINARY_MOD); \
    cmpop(BINARY_LT, CMP_LT); \
    cmpop(BINARY_GT, CMP_GT); \
    cmpop(BINARY_EQ, CMP_EQ); \
    case BINARY_LE: { \
      ensure_ob_has_cmp_func(BINARY_LE); \
\
      ret = cmpfunc(ob_left, ob_right) == CMP_EQ || \
            cmpfunc(ob_left, ob_right) == CMP_LT ? \
            nm_new_int(1) : nm_new_int(0); \
      break; \
    } \
    case BINARY_GE: { \
      ensure_ob_has_cmp_func(BINARY_GE); \
\
      ret = cmpfunc(ob_left, ob_right) == CMP_EQ || \
            cmpfunc(ob_left, ob_right) == CMP_GT ? \
            nm_new_int(1) : nm_new_int(0); \
      break; \
    } \
    case BINARY_NE: { \
      ensure_ob_has_cmp_func(BINARY_NE); \
\
      ret = cmpfunc(ob_left, ob_right) == CMP_GT || \
            cmpfunc(ob_left, ob_right) == CMP_LT ? \
            nm_new_int(1) : nm_new_int(0); \
      break; \
    } \
    default: \
      nm_parser_error(n, "invalid types '%s' and '%s' for binary operation %s", nm_str_value(nm_obj_typetos(ob_left)), nm_str_value(nm_obj_typetos(ob_right)), binopToS(nc->op)); \
    nexit(); \
  }

  Nob *ob_left = nm_ast_exec(left);
  Nob *ob_right = nm_ast_exec(right);

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
      if ((int)(nm_float_value(ob_right)) == nm_float_value(ob_right)){
        ob_right = nm_new_int((int)(nm_float_value(ob_right)));
      } else {
        ob_left = nm_new_float_from_int(nm_int_value(ob_left));
      }
      binary_ops();
    }
    /* XXX float and int */
    else if (ob_left->type == OT_FLOAT && ob_right->type == OT_INTEGER){
      /* check if the float is something like 2.0 or 1234.0, and if it is use it
       * as in int */
      if ((int)(nm_float_value(ob_left)) == nm_float_value(ob_left)){
        ob_left = nm_new_int((int)(nm_float_value(ob_left)));
      } else {
        ob_right = nm_new_float_from_int(nm_int_value(ob_right));
      }
      binary_ops();
    }
    /* if anything else, the operation is simply not permitted */
    else {
      nm_parser_error(n, "invalid types '%s' and '%s' for binary operation %s", nm_str_value(nm_obj_typetos(ob_left)), nm_str_value(nm_obj_typetos(ob_right)), binopToS(nc->op));
      nexit();
    }
  }

#undef op
#undef ensure_ob_has_func

  return ret;
}

/*
 * @name - nm_ast_free_binop
 * @desc - responsible for freeing binary operation ns
 */
void nm_ast_free_binop(Node *n)
{
  assert(n);
  assert(n->type == NT_BINOP);

  nm_ast_free(((Node_Binop *)n)->left);
  nm_ast_free(((Node_Binop *)n)->right);

#if DEBUG
  nm_debug_ast(n, "free binary operation node");
#endif
  nfree(n);
}

/*
 * @name - nm_ast_gen_unop
 * @desc - creates a n for unary operation
 */
Node *nm_ast_gen_unop(Pos pos, Node *target, UnaryOp op)
{
  Node_Unop *n = ncalloc(1, sizeof(Node_Unop));

  n->type = NT_UNOP;
  n->op = op;
  n->target = target;
  INIT_POS();

#if DEBUG
  nm_debug_ast(n, "create unary operation node (op: %s, expr: %p)", unopToS(op), (void*)target);
#endif

  return (Node *)n;
}

/*
 * @name - nm_ast_exec_unop
 * @desc - return the result of the unary operation
 */
Nob *nm_ast_exec_unop(Node *n)
{
  Node_Unop *nc = (Node_Unop *)n;
  Nob *ret = null;

  Nob *target = nm_ast_exec(nc->target);

#if DEBUG
  nm_debug_ast(n, "execute unary operation node");
#endif

/* a handy macro to check if an object supports given unary operation
 * <func>, and if not, print an error, and exit
 *
 * <ob> is of type { Nob * }
 * <func> is of type { UnaryFunc }
 */
#define ensure_ob_has_func(TYPE) \
  UnaryFunc unaryfunc = nm_obj_has_unary_func(target, TYPE); \
  if (!unaryfunc){ \
    nm_parser_error(n, "invalid type '%s' for unary operator %s", nm_str_value(nm_obj_typetos(target)), unopToS(nc->op)); \
    nexit(); \
  }

/* <TYPE> is of type { UnaryOp } */
#define op(TYPE) \
  case TYPE: { \
    ensure_ob_has_func(TYPE); \
\
    ret = unaryfunc(target); \
    break; \
  }

  switch (nc->op){
    op(UNARY_PLUS);
    op(UNARY_MINUS);
    op(UNARY_NEGATE);
    case UNARY_PREINC: {
      ensure_ob_has_func(UNARY_PREINC);
      ret = unaryfunc(target);
      break;
    }
    case UNARY_PREDEC: {
      ensure_ob_has_func(UNARY_PREDEC);
      ret = unaryfunc(target);
      break;
    }
    case UNARY_POSTINC: {
      ensure_ob_has_func(UNARY_POSTINC);
      Nob *save = nm_obj_dup(target);
      unaryfunc(target);
      ret = save;
      break;
    }
    case UNARY_POSTDEC: {
      ensure_ob_has_func(UNARY_POSTDEC);
      Nob *save = nm_obj_dup(target);
      unaryfunc(target);
      ret = save;
      break;
    }
    default:
      nm_parser_error(n, "invalid unary operator %s for type '%s'", unopToS(nc->op), nm_str_value(nm_obj_typetos(target)));
      nexit();
  }

#undef op
#undef ensure_ob_has_func

  return ret;
}

/*
 * @name - nm_ast_free_unop
 * @desc - responsible for freeing unary operation ns
 */
void nm_ast_free_unop(Node *n)
{
  assert(n);
  assert(n->type == NT_UNOP);

  nm_ast_free(((Node_Unop *)n)->target);

#if DEBUG
  nm_debug_ast(n, "free unary operation node");
#endif
  nfree(n);
}

/*
 * @name - nm_ast_gen_if
 * @desc - creates a n for the if statement
 *         <guard>, <body> and <elsee> can be NULL, it means a NOP then
 */
Node *nm_ast_gen_if(Pos pos, Node *guard, Node *body, Node *elsee, bool unless)
{
  Node_If *n = ncalloc(1, sizeof(Node_If));

  n->type = NT_IF;
  n->guard = guard;
  n->body = body;
  n->elsee = elsee;
  n->unless = unless;
  INIT_POS();

#if DEBUG
  if (unless)
    nm_debug_ast(n, "create unless node (guard: %p, body: %p, else: %p)", (void*)guard, (void*)body, (void*)elsee);
  else
    nm_debug_ast(n, "create if node (guard: %p, body: %p, else: %p)", (void*)guard, (void*)body, (void*)elsee);
#endif

  return (Node *)n;
}

/*
 * @name - nm_ast_exec_if
 * @desc - do the if loop, return actually anything
 */
Nob *nm_ast_exec_if(Node *n)
{
  Node_If *nc = (Node_If *)n;
  Node *guard = nc->guard;
  Node *body  = nc->body;
  Node *elsee = nc->elsee;
  bool unless = nc->unless;

#if DEBUG
  if (unless)
    nm_debug_ast(n, "execute unless node");
  else
    nm_debug_ast(n, "execute if node");
#endif

  if (unless){
    if (!nm_obj_boolish(nm_ast_exec(guard))){
      nm_ast_exec(body);
    } else {
      if (elsee)
        nm_ast_exec(elsee);
    }
  } else {
    if (nm_obj_boolish(nm_ast_exec(guard))){
      nm_ast_exec(body);
    } else {
      if (elsee)
        nm_ast_exec(elsee);
    }
  }

  return nm_new_int(1);
}

/*
 * @name - nm_ast_free_if
 * @desc - responsible for freeing if ns, and optionally, if present, it's
 *         guarding statement and it's body, and the else statement connected
 *         with it
 */
void nm_ast_free_if(Node *n)
{
  assert(n);
  assert(n->type == NT_IF);

  Node_If *nc = (Node_If *)n;

  /* guard in if is optional */
  if (nc->guard)
    nm_ast_free(nc->guard);
  /* so is it's body */
  if (nc->body)
    nm_ast_free(nc->body);
  /* aaaaaand the else */
  if (nc->elsee)
    nm_ast_free(nc->elsee);

#if DEBUG
  if (nc->unless)
    nm_debug_ast(n, "free unless node");
  else
    nm_debug_ast(n, "free if node");
#endif

  nfree(n);
}

/*
 * @name - nm_ast_gen_while
 * @desc - creates a while n
 *         <guard>, <body> and <elsee> are optional, can be NULL,
 *         it means then that they are NOPs
 *         <elsee> here gets evaluated when the while loop didn't run even once
 */
Node *nm_ast_gen_while(Pos pos, Node *guard, Node *body, Node *elsee, bool until)
{
  Node_While *n = ncalloc(1, sizeof(Node_While));

  n->type = NT_WHILE;
  n->guard = guard;
  n->body = body;
  n->elsee = elsee;
  n->until = until;
  INIT_POS();

#if DEBUG
  if (until)
    nm_debug_ast(n, "create until node (guard: %p, body: %p, else: %p)", (void*)guard, (void*)body, (void*)elsee);
  else
    nm_debug_ast(n, "create while node (guard: %p, body: %p, else: %p)", (void*)guard, (void*)body, (void*)elsee);
#endif

  return (Node *)n;
}

/*
 * @name - nm_ast_exec_while
 * @desc - execute the loop and return, actually anything, it's a statement
 */
Nob *nm_ast_exec_while(Node *n)
{
  Node_While *nc = (Node_While *)n;
  Node *guard    = nc->guard;
  Node *body     = nc->body;
  Node *elsee    = nc->elsee;
  bool until     = nc->until;

#if DEBUG
  if (until)
    nm_debug_ast(n, "execute until node");
  else
    nm_debug_ast(n, "execute while node");
#endif

  if (until){
    if (!nm_obj_boolish(nm_ast_exec(guard))){
      while (!nm_obj_boolish(nm_ast_exec(guard))){
        nm_ast_exec(body);
      }
    } else {
      if (elsee)
        nm_ast_exec(elsee);
    }
  } else {
    if (nm_obj_boolish(nm_ast_exec(guard))){
      while (nm_obj_boolish(nm_ast_exec(guard))){
        nm_ast_exec(body);
      }
    } else {
      if (elsee)
        nm_ast_exec(elsee);
    }
  }

  return nm_new_int(1);
}

/*
 * @name - nm_ast_free_while
 * @desc - responsible for freeing while n, and optionally
 *         guarding statement and it's body, as they are optional
 */
void nm_ast_free_while(Node *n)
{
  assert(n);
  assert(n->type == NT_WHILE);

  Node_While *nc = (Node_While *)n;

  /* guard in while is optional */
  if (nc->guard)
    nm_ast_free(nc->guard);
  /* so is it's body */
  if (nc->body)
    nm_ast_free(nc->body);
  /* aaaaaand the else */
  if (nc->elsee)
    nm_ast_free(nc->elsee);

#if DEBUG
  if (nc->until)
    nm_debug_ast(n, "free until node");
  else
    nm_debug_ast(n, "free while node");
#endif
  nfree(n);
}

/*
 * @name - nm_ast_gen_decl
 * @desc - creates a n for declaring a variable of given <name>
 *         parameter <value> is optional, may be NULL, then it means
 *         something like:
 *
 *           my variable;
 */
Node *nm_ast_gen_decl(Pos pos, char *name, Node *value, uint8_t flags)
{
  Node_Decl *n = ncalloc(1, sizeof(Node_Decl));
  Namespace *namespace = nm_curr_namespace();

  n->type = NT_DECL;
  n->name = nm_strdup(name);
  n->value = value;
  n->flags = flags;
  INIT_POS();

#if DEBUG
  nm_debug_ast(n, "create variable declaration node (name: %s, namespace: %s)", name, namespace->name);
#endif

  VariablesList *new_list = nmalloc(sizeof(VariablesList));
  Variable *new_var = nmalloc(sizeof(Variable));
  VariablesList *p;

  /* before doing anything, check if that variable was already declared */
  for (p = namespace->globals; p != NULL; p = p->next){
    if (!strcmp(p->var->name, name)){
      nm_parser_error((Node *)n, "global variable '%s' already declared", name);
      nexit();
    }
  }

  new_var->name = nm_strdup(name);
  if (n->value){
    Nob *value = nm_ast_exec(n->value);
    new_var->value = value;
  } else {
    /* declared variables get to be a integer with the value of 0 */
    new_var->value = nm_new_int(0);
  }
  new_var->flags = n->flags;
  /* append to the globals list */
  new_list->var = new_var;
  new_list->next = namespace->globals;
  namespace->globals = new_list;

  return (Node *)n;
}

/*
 * @name - nm_ast_exec_decl
 * @desc - declare/define the variable and return 1
 */
Nob *nm_ast_exec_decl(Node *n)
{
  /* unused parameter */
  (void)n;

#if DEBUG
  nm_debug_ast(n, "execute variable declaration node");
#endif

  return nm_new_int(1);
}

/*
 * @name - nm_ast_free_decl
 * @desc - responsible for freeing declaration n
 *         and optionally a init value it was holding
 */
void nm_ast_free_decl(Node *n)
{
  assert(n);
  assert(n->type == NT_DECL);

  Node_Decl *nc = (Node_Decl *)n;

  nfree(nc->name);
  /* initialized value is optional */
  if (nc->value)
    nm_ast_free(nc->value);

#if DEBUG
  nm_debug_ast(n, "free declaration node");
#endif
  nfree(n);
}

/*
 * @name - nm_ast_gen_call
 * @desc - creates a n for calling a function of a given <name>
 *         parameter <params> is optional, may be NULL, then it means
 *         that no parameters have been passed
 */
Node *nm_ast_gen_call(Pos pos, char *name, Node **params, char *opts, struct Namespace *namespace)
{
  Node_Call *n = ncalloc(1, sizeof(Node_Call));

  n->type = NT_CALL;
  n->name = nm_strdup(name);
  n->params = params;
  n->opts = nm_strdup(opts);
  n->namespace = namespace;
  INIT_POS();

#if DEBUG
  nm_debug_ast(n, "create call node (name: %s, params: %p, opts: '%s')", name, params, opts);
#endif

  return (Node *)n;
}

/*
 * @name - nm_ast_exec_call
 * @desc - call the given function
 */
Nob *nm_ast_exec_call(Node *n)
{
  Node_Call *nc = (Node_Call *)n;
  Nob *ret = null;
  char *name = nc->name;

#if DEBUG
  nm_debug_ast(n, "execute function call node");
#endif

  Namespace *namespaces[3], *namespace;
  namespaces[0] = nm_get_namespace_by_name("core");
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
        Nob *array = nm_new_arr(nmemb);
        /* set the arrays elements */
        if (nc->params){
          /* but first, type checking
           * if the function takes -1 then every argument should of type
           * <list->func->types[0]>, if the number of arguments is >= 0 then
           * every argument should be of the matching element in
           * <list->func->types> */
          for (Node **p = nc->params; *p != NULL; p++, i++){
            NobType type;
            if (list->func->argc < 0){
              type = list->func->types[0];
            } else {
              type = list->func->types[i];
            }
            /* actually check the type of the current argument */
            Nob *ob = nm_ast_exec(*p);
            if (!(ob->type & type)){
              Nob *dummy = nmalloc(sizeof(Nob));
              dummy->type = type;
              nm_parser_error(n, "wrong argument's #%d type for the function '%s' (%s given when %s expected)", i, name, nm_str_value(nm_obj_typetos(ob)), nm_str_value(nm_obj_typetos(dummy)));
              nfree(dummy);
              nexit();
              return NULL;
            }
            nm_arr_set_elem(array, i, ob);
          }
        }
        /* now, let's check what options were passed */
        bool *opts = NULL;
        if (strlen(list->func->opts) > 0){
          opts = nmalloc(sizeof(bool) * strlen(list->func->opts));
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
        }
        /* execute the function */
        ret = list->func->body(array, opts);
        free(opts);
        /* if a function returns NULL it means something went wrong */
        if (ret == NULL){
          nm_parser_error(n, nm_curr_error());
          nexit();
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
          nm_parser_error(n, "cannot call function '%s.%s' which was only declared but not defined", namespace->name, name);
          nexit();
          return NULL;
        }
        /* let's see if the types match */
        /* it should happen at compile time, though */
        bool wrong_types = false;
        for (unsigned i = 0; i < list->func->argc; i++){
          Nob *param = nm_ast_exec(nc->params[i]);
          if (list->func->argv[i] != param->type){
            Nob *param_dummy = nmalloc(sizeof(Nob));
            Nob *arg_dummy = nmalloc(sizeof(Nob));
            param_dummy->type = param->type;
            arg_dummy->type = list->func->argv[i];
            nm_parser_error(n, "wrong argument's #%d type for the function '%s' (%s given when %s expected)", i, name, nm_str_value(nm_obj_typetos(param_dummy)), nm_str_value(nm_obj_typetos(arg_dummy)));
            wrong_types = true;
            nfree(param_dummy);
            nfree(arg_dummy);
          }
        }

        if (wrong_types){
          nexit();
          return NULL;
        }
        /* execute the functions body */
        ret = nm_ast_exec(list->func->body);
        /* if a function returns NULL it means something went wrong */
        if (ret == NULL){
          nm_parser_error(n, "executing function '%s' went wrong", name);
          nexit();
          return NULL;
        } else {
          return ret;
        }
      }
    }
  }

  nm_parser_error(n, "function '%s' not found", name);
  nexit();
  return NULL;
}

/*
 * @name - nm_ast_free_call
 * @desc - responsible for freeing call node and optionally any params it had
 */
void nm_ast_free_call(Node *n)
{
  unsigned i;

  assert(n);
  assert(n->type == NT_CALL);

  Node_Call *nc = (Node_Call *)n;

  nfree(nc->name);
  /* parameters are optional */
  if (nc->params){
    for (i = 0; nc->params[i] != NULL; i++){
      nm_ast_free(nc->params[i]);
    }
#if DEBUG
    nm_debug_ast(nc->params, "free params list");
#endif
    nfree(nc->params);
  }

  nfree(nc->opts);
#if DEBUG
  nm_debug_ast(n, "free call node");
#endif
  nfree(n);
}

/*
 * @name - nm_ast_gen_stmt
 * @desc - create a statement node, but with only one expression
 */
Node *nm_ast_gen_stmt(Pos pos, Node *expr)
{
  Node_Stmt *n = ncalloc(1, sizeof(Node_Stmt));

  n->next = expr->next;
  n->type = NT_STMT;
  n->expr = expr;
  INIT_POS();

#if DEBUG
  nm_debug_ast(n, "create statement node (expr: %p)", expr);
#endif

  return (Node *)n;
}

Nob *nm_ast_exec_stmt(Node *n)
{
  return nm_ast_exec(((Node_Stmt *)n)->expr);
}

void nm_ast_free_stmt(Node *n)
{
  assert(n);
  assert(n->type == NT_STMT);

#if DEBUG
  nm_debug_ast(n, "free statement node");
#endif

  nm_ast_free(((Node_Stmt *)n)->expr);
  nfree(n);
}

Nob *nm_ast_exec_block(Node *n)
{
  Node_Block *nc = (Node_Block *)n;
  Nob *ret = null;
  Statement *s;
  Statement *next;

  assert(n);
  assert(n->type == NT_BLOCK);

#if DEBUG
  nm_debug_ast(n, "execute block node");
#endif

  for (s = nc->tail; s != NULL; s = next){
    next = s->next;
    if (s->stmt){
#if DEBUG
      nm_debug_ast(s->stmt, "execute statement node");
#endif
      ret = nm_ast_exec(s->stmt);
    }
  }

#if DEBUG
  nm_debug_ast(n, "done executing block node");
#endif

  return ret;
}

/*
 * @name - freeBlock
 * @desc - frees given block and every statement it holds
 */
void nm_ast_free_block(Node *n)
{
  Statement *s;
  Statement *next;
  Node_Block *nc = (Node_Block *)n;

  assert(n);
  assert(n->type == NT_BLOCK);

  for (s = nc->tail; s != NULL; s = next){
    next = s->next;
    nm_ast_free(s->stmt);
#if DEBUG
    nm_debug_ast(n, "free statement node");
#endif
    nfree(s);
  }

#if DEBUG
  nm_debug_ast(n, "free block node");
#endif
  nfree(n);
}

/*
 * @name - nm_ast_gen_funcdef
 * @desc - creates a node for defining a function of a given <name>
 */
Node *nm_ast_gen_funcdef(Pos pos, char *name, Node *body,
                       unsigned argc, unsigned optc,
                       NobType *argv, char *opts)
{
  Namespace *namespace = nm_curr_namespace();
  FuncsList *l = nmalloc(sizeof(FuncsList));
  Func *f = nmalloc(sizeof(Func));
  Node_Funcdef *n = ncalloc(1, sizeof(Node_Funcdef));

  n->type = NT_FUNCDEF;
  n->name = nm_strdup(name);
  n->argc = argc;
  n->optc = optc;
  n->argv = argv;
  n->opts = opts;
  n->body = body;
  INIT_POS();

#if DEBUG
  if (body)
    nm_debug_ast(n, "create function definition node (name: %s.%s, body: %p, argc: %d, optc: %d, opts: '%s')", namespace->name, name, (void*)body, argc, optc, opts);
  else
    nm_debug_ast(n, "create function declaration node (name: %s.%s, argc: %d, optc: %d, namespace: '%s')", namespace->name, name, argc, optc);
#endif


  /* initialize */
  f->name = nm_strdup(name);
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
 * @name - nm_ast_exec_funcdef
 * @desc - declare/define given function
 */
Nob *nm_ast_exec_funcdef(Node *n)
{
  Node_Funcdef *nc = (Node_Funcdef *)n;

#if DEBUG
  if (nc->body)
    nm_debug_ast(n, "execute function definition node");
  else
    nm_debug_ast(n, "execute function declaration node");
#endif

  return nm_new_int(1);
}

/*
 * @name - nm_ast_free_funcdef
 * @desc - responsible for freeing call n and optionally any params it had
 */
void nm_ast_free_funcdef(Node *n)
{
  assert(n);
  assert(n->type == NT_FUNCDEF);

  Node_Funcdef *nc = (Node_Funcdef *)n;

  nfree(nc->name);
  if (nc->body){
    nm_ast_free(nc->body);
#if DEBUG
    nm_debug_ast(n, "free function definition node");
#endif
  } else {
#if DEBUG
    nm_debug_ast(n, "free function declaration node");
#endif
  }
  nfree(nc->argv);
  nfree(nc->opts);
  nfree(n);
}

/*
 * @name - nm_ast_gen_use
 * @desc - creates a node that creates does the including thing
 */
Node *nm_ast_gen_use(Pos pos, char *fname)
{
  Node_Use *n = ncalloc(1, sizeof(Node_Use));

  n->type = NT_USE;
  n->fname = nm_strdup(fname);
  INIT_POS();

#if DEBUG
  nm_debug_ast(n, "create use node (fname: %s)", fname);
#endif

  if (!nm_use_module(fname)){
    nm_parser_error((Node *)n, nm_curr_error());
    nexit();
  }

  return (Node *)n;
}

/*
 * @name - nm_ast_exec_use
 * @desc - actually does the including thing
 * @return 1 if everything went fine
 */
Nob *nm_ast_exec_use(Node *n)
{
  assert(n);
  assert(n->type == NT_USE);

  return nm_new_int(1);
}

void nm_ast_free_use(Node *n)
{
  assert(n);
  assert(n->type == NT_USE);

  Node_Use *nc = (Node_Use *)n;

  nfree(nc->fname);
  nfree(nc->custom_path);
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

