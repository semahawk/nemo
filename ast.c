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
#include "nob.h"
#include "lexer.h"

/* one handy macro */
#define EXEC(node) ((node)->execf(node))
/* another one */
#define RETURN_NEXT return (NM_pc = nd->next, (intptr_t)NM_pc)
#define RETURN return ((intptr_t)NM_pc)

/* program counter */
static struct node *NM_pc = NULL;

/* the argument stack */
static Nob   *NM_as      = NULL;
static Nob   *NM_as_curr = NULL;
static size_t NM_as_size = 16;

/* {{{ argument stack manipulation functions */
void arg_stack_init(void)
{
  NM_as = ncalloc(NM_as_size, sizeof(Nob *));
  NM_as_curr = NM_as;
}

void arg_stack_finish(void)
{
  nfree(NM_as);
}

void arg_stack_push(Nob *ob, const char *file, unsigned line)
{
  ptrdiff_t offset = NM_as_curr - NM_as;
  /* hmm, will they be ever used? */
  (void)file;
  (void)line;

  /* handle overflow */
  if (offset >= (signed)NM_as_size){
    NM_as_size *= 1.5;
    NM_as = nrealloc(NM_as, sizeof(int *) * NM_as_size);
    NM_as_curr = NM_as + offset; /* adjust the 'current' pointer */
  }

  NM_as_curr = ob; /* set up the current `cell' */
  NM_as_curr++; /* move on to the next `cell' */
}

Nob *arg_stack_pop(const char *file, unsigned line)
{
  ptrdiff_t offset = NM_as_curr - NM_as;

  if (offset < 0){
    fprintf(stderr, "nemo: argument stack underflow! in %s line %u\n", file, line);
    exit(1);
  }

  NM_as_curr--;

  return NM_as_curr;
}

Nob *arg_stack_top(void)
{
  return NM_as_curr - 1;
}

void arg_stack_dump(void)
{
  int i = 0;

  printf("\n## Stack dump:\n");
  for (; i < NM_as_curr - NM_as; i++){
    printf("  %x - %p", i, (void *)&NM_as[i]);
    if (&NM_as[i] == NM_as_curr)
      printf(" <<<");
    printf("\n");
  }
  printf("## END\n\n");
}
/* }}} */

static struct node *push_node(struct lexer *lex, struct node *node)
{
  /* {{{ */
  ptrdiff_t offset = lex->nds_pool.curr - lex->nds_pool.ptr;
  struct node *ret = lex->nds_pool.curr;

  /* handle overflow */
  if (offset >= (signed)lex->nds_pool.size){
    /* grow the stack to be twice as big as it was */
    lex->nds_pool.size <<= 1;
    lex->nds_pool.ptr = nrealloc(lex->nds_pool.ptr, sizeof(struct node) * lex->nds_pool.size);
    lex->nds_pool.curr = lex->nds_pool.ptr + offset; /* adjust the 'current' pointer */
  }

  *lex->nds_pool.curr = *node; /* set up the current 'cell' */
  lex->nds_pool.curr++; /* move on to the next 'cell' */

  return ret;
  /* }}} */
}

/* {{{ exec_nodes */
void exec_nodes(struct node *node)
{
  NM_pc = node;

  while ((EXEC(node)))
    ;
}

int exec_const(struct node *nd)
{
  /* {{{  */
  if (nd->type == NT_INTEGER){
    /*printf("executing an integer (%d)\n", nd->in.i);*/
    PUSH(new_nob(T_WORD, nd->in.i));
  } else if (nd->type == NT_FLOAT){
    /*printf("executing a float (%f)\n", nd->in.f);*/
    /* FIXME */
    PUSH(new_nob(T_BYTE, (int)nd->in.f));
  }

  RETURN_NEXT;
  /* }}} */
}

int exec_unop(struct node *nd)
{
  /* {{{ */
  EXEC(nd->in.unop.target);

  /*printf("executing unary operation\n");*/

  switch (nd->in.unop.type){
    /* FIXME */
    default: /* WIP */;
  }

  RETURN_NEXT;
  /* }}} */
}

int exec_binop(struct node *nd)
{
  /* {{{ */
  EXEC(nd->in.binop.left);
  EXEC(nd->in.binop.right);

  /*printf("executing binary operation\n");*/

  switch (nd->in.binop.type){
    /* FIXME */
    default: PUSH((POP(), POP()));
  }

  RETURN_NEXT;
  /* }}} */
}

int exec_if(struct node *nd)
{
  Nob *guard;
  EXEC(nd->in.iff.guard);

  guard = TOP();

  /*printf("guard: %p\n", (void *)guard);*/

  if (guard)
    EXEC(nd->in.iff.body);
  else
    EXEC(nd->in.iff.elsee);

  RETURN_NEXT;
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
  n.next = NULL;

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
  n.next = NULL;

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
  n.next = NULL;

  return push_node(lex, &n);
  /* }}} */
}

struct node *new_if(struct lexer *lex, struct node *guard, struct node *body, struct node *elsee)
{
  /* {{{ */
  struct node n;

  n.type = NT_IF;
  n.in.iff.guard = guard;
  n.in.iff.body  = body;
  n.in.iff.elsee = elsee;
  n.in.iff.unless = false;
  n.execf = exec_if;
  n.next = NULL;

  return push_node(lex, &n);
  /* }}} */
}
/* }}} */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

