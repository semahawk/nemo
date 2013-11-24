/*
 *
 * ast.c
 *
 * Created at:  Fri 15 Nov 22:28:17 2013 22:28:17
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "ast.h"
#include "mem.h"
#include "lexer.h"

/* one handy macro */
#define EXEC(node) ((node)->execf(node))

static struct node *push_node(struct lexer *lex, struct node *node)
{
  ptrdiff_t offset = lex->nds_pool.curr - lex->nds_pool.ptr;
  struct node *ret = lex->nds_pool.curr;

  /* handle overflow */
  if (offset >= (signed)lex->nds_pool.size){
    /* grow the stack to be twice as big as it was */
    lex->nds_pool.size <<= 1;
    lex->nds_pool.ptr = nrealloc(lex->nds_pool.ptr, sizeof(struct node) * lex->nds_pool.size);
    lex->nds_pool.curr = lex->nds_pool.ptr + offset;
  }

  *lex->nds_pool.curr = *node; /* set up the current 'cell' */
  lex->nds_pool.curr++; /* move on to the next 'cell' */

  return ret;
}

/* {{{ exec_nodes */
void exec_const(struct node *n)
{
  /* {{{  */
  if (n->type == NT_INTEGER)
    printf("executing an integer (%d)\n", n->in.i);
  else if (n->type == NT_FLOAT)
    printf("executing a float (%f)\n", n->in.f);
  /* }}} */
}

void exec_unop(struct node *n)
{
  /* {{{ */
  EXEC(n->in.unop.target);

  printf("executing unary operation\n");
  /* }}} */
}

void exec_binop(struct node *n)
{
  /* {{{ */
  EXEC(n->in.binop.left);
  EXEC(n->in.binop.right);

  printf("executing binary operation\n");
  /* }}} */
}
/* }}} */
/* {{{ new_nodes */
struct node *new_int(struct lexer *lex, int value)
{
  /* {{{ */
  struct node n;

  n.type = NT_INTEGER;
  n.in.i = value;
  n.execf = exec_const;

  return push_node(lex, &n);
  /* }}} */
}

struct node *new_unop(struct lexer *lex, enum unop_type type, struct node *target)
{
  /* {{{ */
  struct node n;

  n.type = NT_UNOP;
  n.in.unop.type = type;
  n.in.unop.target = target;
  n.execf = exec_unop;

  return push_node(lex, &n);
  /* }}} */
}

struct node *new_binop(struct lexer *lex, enum binop_type type, struct node *left, struct node *right)
{
  /* {{{ */
  struct node n;

  n.type = NT_BINOP;
  n.in.binop.type = type;
  n.in.binop.left = left;
  n.in.binop.right = right;
  n.execf = exec_binop;

  return push_node(lex, &n);
  /* }}} */
}
/* }}} */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

