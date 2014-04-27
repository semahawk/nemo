/*
 *
 * ast.h
 *
 * Created at:  Fri 15 Nov 22:29:09 2013 22:29:09
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef AST_H
#define AST_H

#include <stdint.h>

#include "nemo.h"
#include "nob.h"
#include "infnum.h"
#include "lexer.h"

/* forward */
struct lexer;

enum node_type {
  NT_NOP       = 0,
  NT_INTEGER   = 1 << 0,
  NT_FLOAT     = 1 << 1,
  NT_STRING    = 1 << 2,
  NT_NAME      = 1 << 3,
  NT_UNOP      = 1 << 4,
  NT_BINOP     = 1 << 5,
  NT_IF        = 1 << 6,
  NT_WHILE     = 1 << 7,
  NT_DECL      = 1 << 8,
  NT_CALL      = 1 << 9,
  NT_STMT      = 1 << 10,
  NT_BLOCK     = 1 << 11,
  NT_FUNDEF    = 1 << 12,
  NT_USE       = 1 << 13,
  NT_WOBBLY    = 1 << 14
};

enum unop_type {
  UNARY_PLUS,
  UNARY_MINUS,
  UNARY_NEGATE,
  UNARY_PREINC,
  UNARY_PREDEC,
  UNARY_POSTINC,
  UNARY_POSTDEC
};

enum binop_type {
  BINARY_GT,
  BINARY_LT,
  BINARY_GE,
  BINARY_LE,
  BINARY_EQ,
  BINARY_NE,
  BINARY_ADD,
  BINARY_SUB,
  BINARY_MUL,
  BINARY_DIV,
  BINARY_MOD,
  BINARY_ASSIGN,
  BINARY_ASSIGN_ADD,
  BINARY_ASSIGN_SUB,
  BINARY_ASSIGN_MUL,
  BINARY_ASSIGN_DIV,
  BINARY_ASSIGN_MOD,
  BINARY_INDEX,
  BINARY_COMMA
};

struct node {
  enum node_type type;
  struct node *next;
  struct node *(*execf)(struct node *);
  unsigned id; /* used to better recognize/find the nodes in debug output */
  /* the `id' probably should also be inside #if DEBUG but then it gives
   * compilation errors that I can't nicely resolve, meh */
#if DEBUG
  void (*dumpf)(struct node *);
#endif
  union {
    struct infnum i;   /* NT_INTEGER */
    float f; /* NT_FLOAT */
    char *s; /* NT_STRING */

    struct { /* NT_UNOP */
      enum unop_type type;
      struct node *target;
    } unop;

    struct { /* NT_BINOP */
      enum binop_type type;
      struct node *left, *right;
    } binop;

    struct { /* NT_DECL (my [flags] <name> [= <value>]) */
      char *name;
      uint8_t flags;
      struct node *value;
    } decl;

    struct { /* NT_CALL (<name> [opts]()) */
      char *name;
      char opts[26];
      struct node **args;
    } call;

    struct { /* NT_IF */
      struct node *guard, *body, *elsee;
      /* is it actually an if or an unless? */
      bool unless;
    } iff;

    struct { /* NT_WHILE */
      struct node *guard, *body, *elsee;
      /* is it actually a while or an until? */
      bool until;
    } whilee;

    struct { /* NT_FUNDEF (function definition) */
      char *name;
      struct node *body;
      unsigned paramc, optc; /* number of params/options the function can take */
      char opts[26];
    } fundef;
  } in;
};

struct node *new_nop(struct lexer *lex);
struct node *new_int(struct lexer *lex, struct infnum value);
struct node *new_unop(struct lexer *lex, enum unop_type type, struct node *target);
struct node *new_binop(struct lexer *lex, enum binop_type type, struct node *left, struct node *right);
struct node *new_if(struct lexer *lex, struct node *guard, struct node *body, struct node *elsee);
struct node *new_wobbly(struct lexer *lex);

void exec_nodes(struct node *node);

#if DEBUG
void dump_nodes(struct node *node);
#else
#define dump_nodes(n) /* NOP */;
#endif

void arg_stack_init(void);
void arg_stack_finish(void);

#define PUSH(i) arg_stack_push(i, __FILE__, __LINE__)
#define POP() arg_stack_pop(__FILE__, __LINE__)
#define TOP() arg_stack_top()
void arg_stack_push(Nob *ob, const char *file, unsigned line);
Nob *arg_stack_pop(const char *file, unsigned line);
Nob *arg_stack_top(void);

#endif /* AST_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

