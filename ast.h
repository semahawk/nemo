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
#include "lexer.h"

/* forward */
struct lexer;

enum node_type {
  NT_NOP       = 0,
  NT_INTEGER   = 1,
  NT_FLOAT     = 2,
  NT_STRING    = 4,
  NT_ARRAY     = 8,
  NT_NAME      = 16,
  NT_UNOP      = 32,
  NT_BINOP     = 64,
  NT_IF        = 128,
  NT_WHILE     = 256,
  NT_DECL      = 512,
  NT_CALL      = 1024,
  NT_STMT      = 2048,
  NT_BLOCK     = 4096,
  NT_FUNDEF    = 8192,
  NT_USE       = 16384
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
  int (*execf)(struct node *);
  union {
    int i;   /* NT_INTEGER */
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

struct node *new_int(struct lexer *lex, int value);
struct node *new_unop(struct lexer *lex, enum unop_type type, struct node *target);
struct node *new_binop(struct lexer *lex, enum binop_type type, struct node *left, struct node *right);

void exec_nodes(struct node *node);

#endif /* AST_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

