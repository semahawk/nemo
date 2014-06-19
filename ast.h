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
#include "scope.h"
#include "utf8.h"

/* forward */
struct lexer;
struct nodes_list;

enum node_type {
  NT_NOP,
  NT_INTEGER,
  NT_REAL,
  NT_STRING,
  NT_CHAR,
  NT_LIST,
  NT_NAME,
  NT_UNOP,
  NT_BINOP,
  NT_TERNOP,
  NT_IF,
  NT_WHILE,
  NT_DECL,
  NT_CALL,
  NT_BLOCK,
  NT_FUN,
  NT_USE,
  NT_PRINT
};

enum unop_type {
  UNARY_PLUS,
  UNARY_MINUS,
  UNARY_LOGNEG, /* logic negation */
  UNARY_BITNEG, /* bitwise negation */
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
  BINARY_SHL,
  BINARY_SHR,
  BINARY_BITAND,
  BINARY_BITXOR,
  BINARY_BITOR,
  BINARY_ASSIGN,
  BINARY_ASSIGN_ADD,
  BINARY_ASSIGN_SUB,
  BINARY_ASSIGN_MUL,
  BINARY_ASSIGN_DIV,
  BINARY_ASSIGN_MOD,
  BINARY_ASSIGN_AND,
  BINARY_ASSIGN_XOR,
  BINARY_ASSIGN_OR,
  BINARY_ASSIGN_SHL,
  BINARY_ASSIGN_SHR,
  BINARY_COMMA
};

struct node {
  /* d'uh */
  enum node_type type;
  /* type of a Nob the expression is going to result in */
  struct nob_type *result_type;
  /* pointer to the next node (expression) to be executed in the execution
   * chain */
  struct node *next;
  /* pointer to a function that is responsible for executing the node
   * and maintaining the stack (ie. pushing the node's result value onto it) */
  struct node *(*execf)(struct node *);
  /* a scope in which the expression exists */
  /* this is where all the variables and alike where be searched for */
  /* (of cource, following `scope`s parent scopes) */
  struct scope *scope;
  /* used to better recognize/find the nodes in debug output */
  /* the `id' probably should also be inside #if DEBUG but then it gives
   * compilation errors that I can't nicely resolve, meh */
  unsigned id;
#if DEBUG
  /* function which dumps the node (option `-da`) */
  void (*dumpf)(struct node *);
#endif
  /* is it an lvalue? if not, it's only a rvalue */
  bool lvalue;
  /* values specific to a certain kind of a node */
  union {
    struct infnum i; /* NT_INTEGER */
    double  f; /* NT_REAL */
    char   *s; /* NT_STRING */
    nchar_t c; /* NT_CHAR */

    struct { /* NT_LIST */
      struct nob_type *type;
      struct nodes_list *elems;
    } list;

    struct { /* NT_UNOP */
      enum unop_type type;
      struct node *target;
    } unop;

    struct { /* NT_BINOP */
      enum binop_type type;
      struct node *left, *right;
    } binop;

    struct { /* NT_TERNOP */
      struct node *predicate, *yes, *no;
    } ternop;

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

    struct { /* NT_FUN (function) */
      char *name;
      struct node *body;
      struct nob_type *return_type;
      struct nob_type **params;
      unsigned paramc, optc; /* number of params/options the function can take */
      char *opts;
      /* `true' in case of 'blocks', `false' in case of a 'normal' function */
      /* that is, if it's `true', the function will be executed automatically */
      /* if `false' it has to be called explicitly */
      bool execute;
    } fun;

    struct { /* NT_PRINT */
      struct nodes_list *exprs;
    } print;
  } in;
};

struct nodes_list {
  struct node *node;
  struct nodes_list *next;
};

struct node *new_nop(struct parser *parser, struct lexer *lex);
struct node *new_int(struct parser *parser, struct lexer *lex,
    struct infnum value);
struct node *new_char(struct parser *parser, struct lexer *lex, nchar_t value);
struct node *new_real(struct parser *parser, struct lexer *lex, double value);
struct node *new_list(struct parser *parser, struct lexer *lex,
    struct nodes_list *elems);
struct node *new_decl(struct parser *parser, struct lexer *lex, char *name,
    uint8_t flags, struct node *value, struct scope *scope);
struct node *new_name(struct parser *parser, struct lexer *lex, char *name);
struct node *new_unop(struct parser *parser, struct lexer *lex,
    enum unop_type type, struct node *target);
struct node *new_binop(struct parser *parser, struct lexer *lex,
    enum binop_type type, struct node *left, struct node *right);
struct node *new_ternop(struct parser *parser, struct lexer *lex,
    struct node *predicate, struct node *yes, struct node *no);
struct node *new_if(struct parser *parser, struct lexer *lex,
    struct node *guard, struct node *body, struct node *elsee);
struct node *new_fun(struct parser *parser, struct lexer *lex, char *name,
    struct nob_type *ret_type, struct nob_type **params, struct node *body,
    char *opts, bool execute);
struct node *new_print(struct parser *parser, struct lexer *lex,
    struct nodes_list *exprs);

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

const char *binop_to_s(enum binop_type);

#endif /* AST_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

