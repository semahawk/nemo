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
#include "debug.h"
#include "mem.h"
#include "nob.h"
#include "lexer.h"

/* one handy macro */
#define EXEC(node) ((node)->execf(node))
/* few more */
#define RETURN_NEXT return (NM_pc = nd->next, NM_pc)
#define RETURN return (NM_pc)

/* program counter */
static struct node *NM_pc = NULL;

/* the argument stack */
static Nob   *NM_as      = NULL;
static Nob   *NM_as_curr = NULL;
static size_t NM_as_size = 16;

/* the current node's id */
static unsigned currid = 0;

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

#if DEBUG
/* {{{ dump macros */
/* maintaining the proper amount of spaces before the node's dump */
static int nodes_sw = 3;
#define INDENT() do { nodes_sw += 2; } while (0)
#define DEDENT() do { nodes_sw -= 2; } while (0)
#define SPACES() do { \
  int i; \
  /* print the proper amount of spaces */ \
  for (i = 0; i < nodes_sw; putchar(' '), i++); \
} while (0)

#define DUMP(node) do { \
  SPACES(); \
  if ((node) == NULL) \
    printf("+ (null)\n"); \
  else \
    (node)->dumpf((node)); \
} while (0)

#define DUMPP(msg) do { \
  SPACES(); \
  /* meh, anonymous variadic macros were introduced in C99 */ \
  puts(msg); \
} while (0)
/* }}} */
/* {{{ dump_nodes */
void dump_nodes(struct node *node)
{
  if (NM_DEBUG_GET_FLAG(NM_DEBUG_AST)){
    printf("\n## AST Nodes Dump:\n\n");

    for (; node != NULL; node = node->next){
      DUMP(node);
    }

    printf("\n## End\n");
  }
}

void dump_nop(struct node *nd)
{
  printf("+ (#%u) nop\n", nd->id);
}

void dump_const(struct node *nd)
{
  printf("+ (#%u) const\n", nd->id);
}

void dump_unop(struct node *nd)
{
  printf("+ (#%u) unop\n", nd->id);
  INDENT();
  DUMP(nd->in.unop.target);
  DEDENT();
}

void dump_binop(struct node *nd)
{
  printf("+ (#%u) binop ", nd->id);

  switch (nd->in.binop.type){
    case BINARY_ADD:
      printf("'+'\n");
      break;
    case BINARY_SUB:
      printf("'-'\n");
      break;
    case BINARY_MUL:
      printf("'*'\n");
      break;
    case BINARY_DIV:
      printf("'/'\n");
      break;
    case BINARY_MOD:
      printf("'%%'\n");
      break;
    default:
      printf("-- unknown\n");
      break;
  }

  INDENT();
  DUMP(nd->in.binop.left);
  DUMP(nd->in.binop.right);
  DEDENT();
}

void dump_if(struct node *nd)
{
  printf("+ (#%u) if\n", nd->id);
  INDENT();
  DUMPP("- guard:");
  INDENT();
  DUMP(nd->in.iff.guard);
  DEDENT();
  DUMPP("- body:");
  INDENT();
  DUMP(nd->in.iff.body);
  DEDENT();
  DUMPP("- else:");
  INDENT();
  DUMP(nd->in.iff.elsee);
  DEDENT();
  DEDENT();
}

void dump_wobbly(struct node *nd)
{
  /* suspress the warning */
  (void)nd;

  printf("+ wobbly\n");
}
/* }}} */
#endif /* DEBUG */

/* {{{ exec_nodes */
void exec_nodes(struct node *node)
{
  NM_pc = node;

  while (EXEC(NM_pc))
    ;
}

struct node *exec_nop(struct node *nd)
{
  /* {{{ */
  debug_ast_exec(nd, "nop");

  RETURN_NEXT;
  /* }}} */
}

struct node *exec_const(struct node *nd)
{
  /* {{{  */
  if (nd->type == NT_INTEGER){
    debug_ast_exec(nd, "integer (%d)", nd->in.i);
    PUSH(new_nob(T_WORD, nd->in.i));
  } else if (nd->type == NT_FLOAT){
    debug_ast_exec(nd, "float (%f)", nd->in.f);
    /* FIXME */
    PUSH(new_nob(T_BYTE, (int)nd->in.f));
  }

  RETURN_NEXT;
  /* }}} */
}

struct node *exec_unop(struct node *nd)
{
  /* {{{ */
  debug_ast_exec(nd, "unop ('op?', #%u)", nd->in.unop.target->id);

  EXEC(nd->in.unop.target);

  switch (nd->in.unop.type){
    /* FIXME */
    default: /* WIP */;
  }

  RETURN_NEXT;
  /* }}} */
}

struct node *exec_binop(struct node *nd)
{
  /* {{{ */
  debug_ast_exec(nd, "binop ('op?', #%u, #%u)", nd->in.binop.left->id, nd->in.binop.right->id);

  EXEC(nd->in.binop.left);
  EXEC(nd->in.binop.right);

  switch (nd->in.binop.type){
    /* FIXME */
    default: /* meh */;
  }

  RETURN_NEXT;
  /* }}} */
}

struct node *exec_if(struct node *nd)
{
  /* {{{ */
  Nob *guard;

  if (nd->in.iff.elsee != NULL)
    debug_ast_exec(nd, "if (#%u, #%u, #%u)",
      nd->in.iff.guard->id,
      nd->in.iff.body->id,
      nd->in.iff.elsee->id);
  else
    debug_ast_exec(nd, "if (#%u, #%u, --)",
      nd->in.iff.guard->id,
      nd->in.iff.body->id);

  EXEC(nd->in.iff.guard);

  guard = TOP();

  if (guard)
    EXEC(nd->in.iff.body);
  else
    if (nd->in.iff.elsee != NULL)
      EXEC(nd->in.iff.elsee);

  RETURN_NEXT;
  /* }}} */
}

struct node *exec_wobbly(struct node *nd)
{
  /* {{{ */
  printf("wobbly dobbly!\n");

  RETURN_NEXT;
  /* }}} */
}
/* }}} */
/* {{{ new_nodes */
struct node *new_nop(struct lexer *lex)
{
  /* {{{ */
  struct node n, *ret;

  n.id = currid++;
  n.type = NT_NOP;
  n.execf = exec_nop;
#if DEBUG
  n.dumpf = dump_nop;
#endif
  n.next = NULL;

  ret = push_node(lex, &n);
  debug_ast_new(ret, "nop");

  return ret;
  /* }}} */
}

struct node *new_int(struct lexer *lex, int value)
{
  /* {{{ */
  struct node n, *ret;

  n.id = currid++;
  n.type = NT_INTEGER;
  n.in.i = value;
  n.execf = exec_const;
#if DEBUG
  n.dumpf = dump_const;
#endif
  n.next = NULL;

  ret = push_node(lex, &n);
  debug_ast_new(ret, "integer (%d)", value);

  return ret;
  /* }}} */
}

struct node *new_unop(struct lexer *lex, enum unop_type type, struct node *target)
{
  /* {{{ */
  struct node n, *ret;

  n.id = currid++;
  n.type = NT_UNOP;
  n.in.unop.type = type;
  n.in.unop.target = target;
  n.execf = exec_unop;
#if DEBUG
  n.dumpf = dump_unop;
#endif
  n.next = NULL;

  ret = push_node(lex, &n);
  debug_ast_new(ret, "unop ('op?', #%u)", ret->in.unop.target->id);

  return ret;
  /* }}} */
}

struct node *new_binop(struct lexer *lex, enum binop_type type, struct node *left, struct node *right)
{
  /* {{{ */
  struct node n, *ret;

  n.id = currid++;
  n.type = NT_BINOP;
  n.in.binop.type = type;
  n.in.binop.left = left;
  n.in.binop.right = right;
  n.execf = exec_binop;
#if DEBUG
  n.dumpf = dump_binop;
#endif
  n.next = NULL;

  ret = push_node(lex, &n);
  debug_ast_new(ret, "binop ('op?', #%u, #%u)", ret->in.binop.left->id, ret->in.binop.right->id);

  return ret;
  /* }}} */
}

struct node *new_if(struct lexer *lex, struct node *guard, struct node *body, struct node *elsee)
{
  /* {{{ */
  struct node n, *ret;

  n.id = currid++;
  n.type = NT_IF;
  n.in.iff.guard = guard;
  n.in.iff.body  = body;
  n.in.iff.elsee = elsee;
  n.in.iff.unless = false;
  n.execf = exec_if;
#if DEBUG
  n.dumpf = dump_if;
#endif
  n.next = NULL;

  ret = push_node(lex, &n);
  if (elsee)
    debug_ast_new(ret, "if (#%u, #%u, #%u)", guard->id, body->id, elsee->id);
  else
    debug_ast_new(ret, "if (#%u, #%u, --)", guard->id, body->id);

  return ret;
  /* }}} */
}

struct node *new_wobbly(struct lexer *lex)
{
  /* {{{ */
  struct node n, *ret;

  n.id = currid++;
  n.type = NT_WOBBLY;
  n.execf = exec_wobbly;
#if DEBUG
  n.dumpf = dump_wobbly;
#endif
  n.next = NULL;

  ret = push_node(lex, &n);
  debug_ast_new(ret, "wobbly");

  return ret;
  /* }}} */
}
/* }}} */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

