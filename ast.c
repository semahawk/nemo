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

/* new_node
 *
 * Allocates a place for a new node, stores it's address in lex's `nds_gc', and
 * returns the new node.
 *
 */
static struct node *new_node(struct lexer *lex)
{
  /* {{{ */
  ptrdiff_t offset = lex->nds_gc.curr - lex->nds_gc.ptr;
  struct node *new = nmalloc(sizeof(struct node));

  /* handle overflow */
  if (offset >= (signed)lex->nds_gc.size){
    /* grow the stack to be twice as big as it was */
    lex->nds_gc.size <<= 1;
    lex->nds_gc.ptr = nrealloc(lex->nds_gc.ptr, sizeof(struct node *) * lex->nds_gc.size);
    /* adjust the 'current' */
    lex->nds_gc.curr = lex->nds_gc.ptr + offset;
  }

  /* set up the current 'cell' */
  *lex->nds_gc.curr = new;
  /* move on to the next 'cell' */
  lex->nds_gc.curr++;

  return new;
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
  printf("+ (#%u) unop ", nd->id);

  switch (nd->in.unop.type){
    case UNARY_PLUS:
      printf("'+'");
      break;
    case UNARY_MINUS:
      printf("'-'");
      break;
    case UNARY_NEGATE:
      printf("'!'");
      break;
    case UNARY_PREINC:
      printf("prefix '++'");
      break;
    case UNARY_PREDEC:
      printf("prefix '--'");
      break;
    case UNARY_POSTINC:
      printf("postfix '++'");
      break;
    case UNARY_POSTDEC:
      printf("postfix '--'");
      break;
    default:
      printf(" -- unknown");
      break;
  }
  printf("\n");

  INDENT();
  DUMP(nd->in.unop.target);
  DEDENT();
}

void dump_binop(struct node *nd)
{
  printf("+ (#%u) binop ", nd->id);

  switch (nd->in.binop.type){
    case BINARY_ADD:
      printf("'+'");
      break;
    case BINARY_SUB:
      printf("'-'");
      break;
    case BINARY_MUL:
      printf("'*'");
      break;
    case BINARY_DIV:
      printf("'/'");
      break;
    case BINARY_MOD:
      printf("'%%'");
      break;
    case BINARY_EQ:
      printf("'=='");
      break;
    case BINARY_NE:
      printf("'!='");
      break;
    case BINARY_LE:
      printf("'<='");
      break;
    case BINARY_LT:
      printf("'<'");
      break;
    case BINARY_GE:
      printf("'>='");
      break;
    case BINARY_GT:
      printf("'>'");
      break;
    case BINARY_COMMA:
      printf("','");
      break;
    default:
      printf("-- unknown");
      break;
  }
  printf("\n");

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
  printf("+ (#%u) wobbly\n", nd->id);
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
  struct node *nd = new_node(lex);

  nd->id = currid++;
  nd->type = NT_NOP;
  nd->execf = exec_nop;
#if DEBUG
  nd->dumpf = dump_nop;
#endif
  nd->next = NULL;

  debug_ast_new(nd, "nop");

  return nd;
  /* }}} */
}

struct node *new_int(struct lexer *lex, int value)
{
  /* {{{ */
  struct node *nd = new_node(lex);

  nd->id = currid++;
  nd->type = NT_INTEGER;
  nd->in.i = value;
  nd->execf = exec_const;
#if DEBUG
  nd->dumpf = dump_const;
#endif
  nd->next = NULL;

  debug_ast_new(nd, "integer (%d)", value);

  return nd;
  /* }}} */
}

struct node *new_unop(struct lexer *lex, enum unop_type type, struct node *target)
{
  /* {{{ */
  struct node *nd = new_node(lex);

  nd->id = currid++;
  nd->type = NT_UNOP;
  nd->in.unop.type = type;
  nd->in.unop.target = target;
  nd->execf = exec_unop;
#if DEBUG
  nd->dumpf = dump_unop;
#endif
  nd->next = NULL;

  debug_ast_new(nd, "unop ('op?', #%u)", nd->in.unop.target->id);

  return nd;
  /* }}} */
}

struct node *new_binop(struct lexer *lex, enum binop_type type, struct node *left, struct node *right)
{
  /* {{{ */
  struct node *nd = new_node(lex);

  nd->id = currid++;
  nd->type = NT_BINOP;
  nd->in.binop.type = type;
  nd->in.binop.left = left;
  nd->in.binop.right = right;
  nd->execf = exec_binop;
#if DEBUG
  nd->dumpf = dump_binop;
#endif
  nd->next = NULL;

  debug_ast_new(nd, "binop ('op?', #%u, #%u)", nd->in.binop.left->id, nd->in.binop.right->id);

  return nd;
  /* }}} */
}

struct node *new_if(struct lexer *lex, struct node *guard, struct node *body, struct node *elsee)
{
  /* {{{ */
  struct node *nd = new_node(lex);

  nd->id = currid++;
  nd->type = NT_IF;
  nd->in.iff.guard = guard;
  nd->in.iff.body  = body;
  nd->in.iff.elsee = elsee;
  nd->in.iff.unless = false;
  nd->execf = exec_if;
#if DEBUG
  nd->dumpf = dump_if;
#endif
  nd->next = NULL;

  if (elsee)
    debug_ast_new(nd, "if (#%u, #%u, #%u)", guard->id, body->id, elsee->id);
  else
    debug_ast_new(nd, "if (#%u, #%u, --)", guard->id, body->id);

  return nd;
  /* }}} */
}

struct node *new_wobbly(struct lexer *lex)
{
  /* {{{ */
  struct node *nd = new_node(lex);

  nd->id = currid++;
  nd->type = NT_WOBBLY;
  nd->execf = exec_wobbly;
#if DEBUG
  nd->dumpf = dump_wobbly;
#endif
  nd->next = NULL;

  debug_ast_new(nd, "wobbly");

  return nd;
  /* }}} */
}
/* }}} */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

