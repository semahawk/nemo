/*
 *
 * parser.c
 *
 * Created at:  Fri Nov  8 18:27:39 2013 18:27:39
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "ast.h"
#include "debug.h"
#include "mem.h"
#include "nob.h"
#include "infer.h"
#include "infnum.h"
#include "lexer.h"
#include "parser.h"
#include "scope.h"
#include "utf8.h"
#include "util.h"

/* expressions end with a semicolon, unless it's the last expressions in the block
 * (or the whole program/module/unit) */
#define expr_end(parser, lex) \
  do { \
    if (!peek(parser, lex, TOK_RMUSTASHE) && \
        !peek(parser, lex, TOK_EOS)){ \
      force(parser, lex, TOK_SEMICOLON); \
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER)) \
        printf(" ENDSTMT;\n"); \
    } \
  } while (0)

/* forward declarations */
struct node *expr_list(struct parser *, struct lexer *);
static struct node *expr(struct parser *, struct lexer *);
static struct node *no_comma_expr(struct parser *, struct lexer *);
static struct node *primary_expr(struct parser *, struct lexer *);

/*
 * Prints a nicely error message.
 */
static void err(struct parser *parser, struct lexer *lex, const char *fmt, ...)
{
  va_list vl;

  va_start(vl, fmt);
  fprintf(stderr, "%s:%u.%u: error: ", lex->name, lex->line, lex->col);
  vfprintf(stderr, fmt, vl);
  fprintf(stderr, "\n");
  va_end(vl);

  parser->errorless = false;
}

/*
 * Fetches the type, at the lexer's current 'position'.
 * If the type already exists, like "int", it returns a pointer to it.
 *
 * If the type doesn't already exist (by the way, it can't be a single word,
 * lexer wouldn't let it through), like an anonymous tuple, creates it as a
 * whole new type, and returns a pointer to it.
 */
static struct nob_type *type(struct parser *parser, struct lexer *lex)
{
  /* {{{ */
  /* the nob_type to be returned */
  struct nob_type *ret = NULL;
  /* function's types; to be passed to `new_type' */
  struct nob_type *return_type = NULL;
  struct nob_type *param_type = NULL;
  /* used for tuples */
  struct types_list *types_list = NULL, *type_elem;

  if (accept(parser, lex, TOK_TYPE)){
    /* {{{ a single worded type */
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("%s", lex->curr_tok.value.s);

    ret = get_type_by_name(lex->curr_tok.value.s);
    /* }}} */
  } else if (accept(parser, lex, TOK_TYPE_VARIABLE)){
    /* {{{ a type variable */
    struct type_variables_list *type_var;
    bool found = false;

    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("'%lc", lex->curr_tok.value.c);

    /* let's see if such a type var was already mentioned */
    for (type_var = parser->type_vars; type_var != NULL; type_var = type_var->next){
      if (type_var->name == lex->curr_tok.value.c){
        found = true;
        break;
      }
    }

    if (!found){
      type_var = nmalloc(sizeof(struct type_variables_list));
      type_var->name = lex->curr_tok.value.c;
      type_var->type = new_type(OT_TYPE_VARIABLE);
      type_var->next = parser->type_vars;
      parser->type_vars = type_var;
    }

    ret = type_var->type;
    /* }}} */
  } else if (accept(parser, lex, TOK_LMUSTASHE)){
    /* {{{ a function */
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("{ ");

    /* fetch the return type */
    if ((return_type = type(parser, lex)) == NULL){
      err(parser, lex, "expected a return type in the function's prototype");
      return ret;
    }

    /* fetch the param type */
    if (accept(parser, lex, TOK_SEMICOLON)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("; ");

      if ((param_type = type(parser, lex)) == NULL){
        err(parser, lex, "expected a param type in the function's prototype");
        return ret;
      }
    }

    ret = new_type(OT_FUN, return_type, param_type);

    force(parser, lex, TOK_RMUSTASHE);
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf(" }");
    /* }}} */
  } else if (accept(parser, lex, TOK_LPAREN)){
    /* {{{ a tuple */
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("(");

    do {
      /* reusing `param_type` variable */
      if ((param_type = type(parser, lex)) != NULL){
        type_elem = nmalloc(sizeof(struct types_list));
        type_elem->type = param_type;
        type_elem->next = types_list;
        types_list = type_elem;
      }
    } while (accept(parser, lex, TOK_COMMA));

    ret = new_type(OT_TUPLE, types_list);

    force(parser, lex, TOK_RPAREN);
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf(")");
    /* }}} */
  }

  return ret;
  /* }}} */
}

static struct node *function(struct parser *parser, struct lexer *lex, struct nob_type *prototype)
{
  /* {{{ */
  struct scope *prev_scope, *functions_scope;
  struct node *ret, *body;
  struct nob_type *inferred_type;

  struct params_info pinfo;
  unsigned idx;
  char param_name[8];

  if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
    printf("{\n");

  /* create a new scope for the function */
  prev_scope = parser->curr_scope;
  parser->curr_scope = functions_scope = new_scope(NULL, parser->curr_scope);

  /* all the expressions here will 'use' the new scope */
  body = expr_list(parser, lex);

  pinfo = count_params(body, NULL);

  /* restore the parser's former scope */
  parser->curr_scope = prev_scope;

  force(parser, lex, TOK_RMUSTASHE);
  if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
    printf("}\n");

  ret = new_fun(parser, lex, NULL, "%1", body, NULL, false);
  ret->scope = functions_scope;

  for (idx = 1; idx < 32; idx++){
    if (pinfo.value & (1 << idx)){
      snprintf(param_name, 8, "%%%d", idx + 1);

      ret = new_fun(parser, lex, NULL, param_name, ret, NULL, false);
      ret->scope = functions_scope;
    }
  }

  inferred_type = infer_node_type(parser->curr_scope, ret);

  if (prototype){
    /* unify does a longjmp into `infer_node_type` which is probably not ideal */
    /* but for now it suffices */
    unify(prototype, inferred_type);

    ret->result_type = prototype;
  } else {
    ret->result_type = inferred_type;
  }

  ret->lvalue = false; /* hmm.. */

  return ret;
  /* }}} */
}

static struct node *primary_expr(struct parser *parser, struct lexer *lex)
{
  /* {{{ */
  struct node *ret = NULL;
  struct nob_type *return_type;

  if (accept(parser, lex, TOK_LPAREN)){
    /* {{{ */
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("(");

    ret = no_comma_expr(parser, lex);

    if ((return_type = type(parser, lex)) != NULL){
      /* {{{ function */
      struct nob_type *param_type;

      force(parser, lex, TOK_RCHEVRON);
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf(" > ");

      if ((param_type = type(parser, lex)) == NULL){
        err(parser, lex, "expected a param type for the function prototype");
        return NULL;
      }

      force(parser, lex, TOK_RPAREN);
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf(") ");

      force(parser, lex, TOK_LMUSTASHE);
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("{\n");

      return function(parser, lex, new_type(OT_FUN, return_type, param_type));
      /* }}} */
    } else if (accept(parser, lex, TOK_COMMA)){
      /* {{{ now, that's a tuple */
      struct nodes_list *elems = NULL;
      struct nodes_list *elem;
      struct node *node;

      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf(", ");

      elem = nmalloc(sizeof(struct nodes_list));
      elem->node = ret;
      elem->next = elems;
      elems = elem;

      if ((node = no_comma_expr(parser, lex)) != NULL){
        elem = nmalloc(sizeof(struct nodes_list));
        elem->node = node;
        elem->next = elems;
        elems = elem;

        while (accept(parser, lex, TOK_COMMA)){
          if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
            printf(", ");

          node = no_comma_expr(parser, lex);
          elem = nmalloc(sizeof(struct nodes_list));
          elem->node = node;
          elem->next = elems;
          elems = elem;
        }
      }

      ret = new_tuple(parser, lex, reverse_nodes_list(elems));
      /* }}} */
    }
    /* else
     *   it's just an expression grouping
     */

    force(parser, lex, TOK_RPAREN);

    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf(")\n");
    /* }}} */
  } else if (accept(parser, lex, TOK_LMUSTASHE)){
    /* {{{ A FUNCTION */
    /* as adversited */
    ret = function(parser, lex, NULL);
    /* }}} */
  } else if (accept(parser, lex, TOK_INTEGER)){
    /* {{{ INTEGER LITERAL */
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("%d ", lex->curr_tok.value.i);

    ret = new_int(parser, lex, lex->curr_tok.value.i);
    ret->result_type = T_INT;
    ret->lvalue = false;
    /* }}} */
  } else if (accept(parser, lex, TOK_REAL)){
    /* {{{ FLOAT LITERAL */
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("%g ", lex->curr_tok.value.f);

    ret = new_real(parser, lex, lex->curr_tok.value.f);
    ret->result_type = T_REAL;
    ret->lvalue = false;
    /* }}} */
  } else if (accept(parser, lex, TOK_STRING)){
    /* {{{ STRING LITERAL */
    struct nodes_list *chars = NULL;
    struct nodes_list *new;
    char *p = lex->curr_tok.value.sp;

    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("\"%s\" ", lex->curr_tok.value.sp);

    while (*p){
      new = nmalloc(sizeof(struct nodes_list));
      new->node = new_char(parser, lex, u8_fetch_char(&p));
      new->next = chars;
      chars = new;
    }

    /* TODO create that list of chars */
    ret = new_int(parser, lex, 789456 /* that's kind of magic, greppable */);
    ret->result_type = T_STRING;
    /* }}} */
  } else if (accept(parser, lex, TOK_CHAR)){
    /* {{{ CHARACTER LITERAL */
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("'%c' ", lex->curr_tok.value.c);

    ret = new_char(parser, lex, lex->curr_tok.value.c);
    ret->result_type = T_CHAR;
    ret->lvalue = false;
    /* }}} */
  } else if (accept(parser, lex, TOK_NAME)){
    /* {{{ NAME */
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("%s ", lex->curr_tok.value.s /* meh */);

    if (!var_lookup(lex->curr_tok.value.s, parser->curr_scope)){
      err(parser, lex, "variable '%s' not found", lex->curr_tok.value.s);
      return NULL;
    }

    ret = new_name(parser, lex, lex->curr_tok.value.s);
    ret->lvalue = true;
    /* }}} */
  } else if (accept(parser, lex, TOK_ACCUMULATOR)){
    /* {{{ AN ACCUMULATOR */
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("%%%s ", lex->curr_tok.value.s);

    /* fetch the numerical value after the % sign */
    /*int num = atoi(lex->curr_tok.value.s + 1);*/

    /* right now it's assumed that every param is sizeof 4 bytes, which is
     * prooobably wrong */
#if 0
    new_var(lex->curr_tok.value.s, 0x0 /* TODO */, NULL, NULL,
        parser->curr_scope, true /* a param */, (num - 1) * 4 /* FIXME */);
#endif

    ret = new_name(parser, lex, lex->curr_tok.value.s);
    ret->lvalue = true;
    /* }}} */
  }

  return ret;
  /* }}} */
}

static struct node *postfix_expr(struct parser *parser, struct lexer *lex)
{
  /* {{{ */
  struct node *target, *ret;
  struct node *arg = NULL;

  target = ret = primary_expr(parser, lex);

  if (peek(parser, lex, TOK_PLUS_2) ||
      peek(parser, lex, TOK_MINUS_2)){
    if (accept(parser, lex, TOK_PLUS_2)){
      /* target ++ */
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf(" postfix(++)");
      ret = new_unop(parser, lex, UNARY_POSTINC, target);
      ret->lvalue = false;
    } else if (accept(parser, lex, TOK_MINUS_2)){
      /* target -- */
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf(" postfix(--)");
      ret = new_unop(parser, lex, UNARY_POSTDEC, target);
      ret->lvalue = false;
    }
  } else if ((arg = primary_expr(parser, lex))){
    /* function application */
    if (target == NULL)
      return arg;

    /* TODO: check whether the type of supplied argument matches
     * the target function's prototype */

    if (target->type == NT_NAME || target->type == NT_FUN){
      ret = new_call(parser, lex, target, arg, NULL);
      ret->lvalue = false; /* hmm.. */
    } else {
      err(parser, lex, "trying to apply a function call on a non-function");
      return NULL;
    }
  }

  return ret;
  /* }}} */
}

static struct node *prefix_expr(struct parser *parser, struct lexer *lex)
{
  /* {{{ */
  struct node *target, *ret;
  enum unop_type type;

  if (peek(parser, lex, TOK_PLUS_2) ||
      peek(parser, lex, TOK_MINUS_2) ||
      peek(parser, lex, TOK_BANG)){
    if (accept(parser, lex, TOK_PLUS_2)){
      /* ++ target */
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("prefix(++) ");

      target = postfix_expr(parser, lex);
      target->lvalue = false;
      type = UNARY_PREINC;
    } else if (accept(parser, lex, TOK_MINUS_2)){
      /* -- target */
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("prefix(--) ");

      target = postfix_expr(parser, lex);
      target->lvalue = false;
      type = UNARY_PREDEC;
    } else if (accept(parser, lex, TOK_BANG)){
      /*  ! target */
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("prefix(!) ");

      target = postfix_expr(parser, lex);
      target->lvalue = false;
      type = UNARY_LOGNEG;
    }

    ret = new_unop(parser, lex, type, target);
    ret->lvalue = false;
  } else {
    ret = postfix_expr(parser, lex);
  }

  return ret;
  /* }}} */
}

static struct node *mul_expr(struct parser *parser, struct lexer *lex)
{
  /* {{{ */
  struct node *left, *right, *ret;
  enum binop_type type;

  left = ret = prefix_expr(parser, lex);

  while (peek(parser, lex, TOK_TIMES) ||
         peek(parser, lex, TOK_SLASH) ||
         peek(parser, lex, TOK_PERCENT)){
    if (accept(parser, lex, TOK_TIMES)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("* ");

      type = BINARY_MUL;
    } else if (accept(parser, lex, TOK_SLASH)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("/ ");

      type = BINARY_DIV;
    } else if (accept(parser, lex, TOK_PERCENT)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("%% ");

      type = BINARY_MOD;
    }

    right = prefix_expr(parser, lex);

    if (!right){
      err(parser, lex, "expected an expression at the RHS of the binary '%s' operation", binop_to_s(type));
      return NULL;
    }

    ret = new_binop(parser, lex, type, left, right);
    ret->lvalue = false;
    left = ret;
  }

  return ret;
  /* }}} */
}

static struct node *add_expr(struct parser *parser, struct lexer *lex)
{
  /* {{{ */
  struct node *left, *right, *ret;
  enum binop_type type;

  left = ret = mul_expr(parser, lex);

  while (peek(parser, lex, TOK_PLUS) || peek(parser, lex, TOK_MINUS)){
    if (accept(parser, lex, TOK_PLUS)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("+ ");

      type = BINARY_ADD;
    } else if (accept(parser, lex, TOK_MINUS)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("- ");

      type = BINARY_SUB;
    }

    right = mul_expr(parser, lex);

    if (!right){
      err(parser, lex, "expected an expression at the RHS of the binary '%s' operation", binop_to_s(type));
      return NULL;
    }

    ret = new_binop(parser, lex, type, left, right);
    ret->lvalue = false;
    left = ret;
  }

  return ret;
  /* }}} */
}

static struct node *bitshift_expr(struct parser *parser, struct lexer *lex)
{
  /* {{{ */
  struct node *left, *right, *ret;
  enum binop_type type;

  left = ret = add_expr(parser, lex);

  while (peek(parser, lex, TOK_LCHEVRON_2) || peek(parser, lex, TOK_RCHEVRON_2)){
    if (accept(parser, lex, TOK_LCHEVRON_2)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("<< ");

      type = BINARY_SHL;
    } else if (accept(parser, lex, TOK_RCHEVRON_2)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf(">> ");

      type = BINARY_SHR;
    }

    right = add_expr(parser, lex);

    if (!right){
      err(parser, lex, "expected an expression at the RHS of the binary '%s' operation", binop_to_s(type));
      return NULL;
    }

    ret = new_binop(parser, lex, type, left, right);
    ret->lvalue = false;
    left = ret;
  }

  return ret;
  /* }}} */
}

static struct node *cond_expr(struct parser *parser, struct lexer *lex)
{
  /* {{{ */
  struct node *left, *right, *ret;
  enum binop_type type;

  left = ret = bitshift_expr(parser, lex);

  if (peek(parser, lex, TOK_LCHEVRON) ||
      peek(parser, lex, TOK_LCHEVRON_EQ) ||
      peek(parser, lex, TOK_RCHEVRON) ||
      peek(parser, lex, TOK_RCHEVRON_EQ)){
    if (accept(parser, lex, TOK_LCHEVRON)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("< ");

      type = BINARY_LT;
    } else if (accept(parser, lex, TOK_LCHEVRON_EQ)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("<= ");

      type = BINARY_LE;
    } else if (accept(parser, lex, TOK_RCHEVRON)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("> ");

      type = BINARY_GT;
    } else if (accept(parser, lex, TOK_RCHEVRON_EQ)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf(">= ");

      type = BINARY_GE;
    }

    right = bitshift_expr(parser, lex);

    if (!right){
      err(parser, lex, "expected an expression at the RHS of the binary '%s' operation", binop_to_s(type));
      return NULL;
    }

    ret = new_binop(parser, lex, type, left, right);
    ret->lvalue = false;
    left = ret;
  }

  return ret;
  /* }}} */
}

static struct node *eq_expr(struct parser *parser, struct lexer *lex)
{
  /* {{{ */
  struct node *left, *right, *ret;
  enum binop_type type;

  left = ret = cond_expr(parser, lex);

  while (peek(parser, lex, TOK_EQ_2) || peek(parser, lex, TOK_BANG_EQ)){
    if (accept(parser, lex, TOK_EQ_2)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("== ");

      type = BINARY_EQ;
    } else if (accept(parser, lex, TOK_BANG_EQ)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("!= ");

      type = BINARY_NE;
    }

    right = cond_expr(parser, lex);

    if (!right){
      err(parser, lex, "expected an expression at the RHS of the binary '%s' operation", binop_to_s(type));
      return NULL;
    }

    ret = new_binop(parser, lex, type, left, right);
    ret->lvalue = false;
    left = ret;
  }

  return ret;
  /* }}} */
}

static struct node *bitand_expr(struct parser *parser, struct lexer *lex)
{
  /* {{{ */
  struct node *left, *right, *ret;
  enum binop_type type;

  left = ret = eq_expr(parser, lex);

  while (accept(parser, lex, TOK_AMPERSAND)){
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("& ");

    type = BINARY_BITAND;
    right = eq_expr(parser, lex);

    if (!right){
      err(parser, lex, "expected an expression at the RHS of the binary '%s' operation", binop_to_s(type));
      return NULL;
    }

    ret = new_binop(parser, lex, type, left, right);
    ret->lvalue = false;
    left = ret;
  }

  return ret;
  /* }}} */
}

static struct node *bitxor_expr(struct parser *parser, struct lexer *lex)
{
  /* {{{ */
  struct node *left, *right, *ret;
  enum binop_type type;

  left = ret = bitand_expr(parser, lex);

  while (accept(parser, lex, TOK_CARET)){
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("^ ");

    type = BINARY_BITXOR;
    right = bitand_expr(parser, lex);

    if (!right){
      err(parser, lex, "expected an expression at the RHS of the binary '%s' operation", binop_to_s(type));
      return NULL;
    }

    ret = new_binop(parser, lex, type, left, right);
    ret->lvalue = false;
    left = ret;
  }

  return ret;
  /* }}} */
}

static struct node *bitor_expr(struct parser *parser, struct lexer *lex)
{
  /* {{{ */
  struct node *left, *right, *ret;
  enum binop_type type;

  left = ret = bitxor_expr(parser, lex);

  while (accept(parser, lex, TOK_PIPE)){
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("| ");

    type = BINARY_BITOR;
    right = bitxor_expr(parser, lex);

    if (!right){
      err(parser, lex, "expected an expression at the RHS of the binary '%s' operation", binop_to_s(type));
      return NULL;
    }

    ret = new_binop(parser, lex, type, left, right);
    ret->lvalue = false;
    left = ret;
  }

  return ret;
  /* }}} */
}

static struct node *logand_expr(struct parser *parser, struct lexer *lex)
{
  /* {{{ */
  struct node *left, *right, *ret;
  enum binop_type type;

  left = ret = bitor_expr(parser, lex);

  while (accept(parser, lex, TOK_AMPERSAND_2)){
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("&& ");

    type = BINARY_LOGAND;
    right = bitor_expr(parser, lex);

    if (!right){
      err(parser, lex, "expected an expression at the RHS of the binary '%s' operation", binop_to_s(type));
      return NULL;
    }

    ret = new_binop(parser, lex, type, left, right);
    ret->lvalue = false;
    left = ret;
  }

  return ret;
  /* }}} */
}

static struct node *logor_expr(struct parser *parser, struct lexer *lex)
{
  /* {{{ */
  struct node *left, *right, *ret;
  enum binop_type type;

  left = ret = logand_expr(parser, lex);

  while (accept(parser, lex, TOK_PIPE_2)){
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("|| ");

    type = BINARY_LOGOR;
    right = logand_expr(parser, lex);

    if (!right){
      err(parser, lex, "expected an expression at the RHS of the binary '%s' operation", binop_to_s(type));
      return NULL;
    }

    ret = new_binop(parser, lex, type, left, right);
    ret->lvalue = false;
    left = ret;
  }

  return ret;
  /* }}} */
}

static struct node *ternary_expr(struct parser *parser, struct lexer *lex)
{
  /* {{{ */
  struct node *predicate, *yes, *no, *ret;

  predicate = ret = logor_expr(parser, lex);

  if (accept(parser, lex, TOK_QUESTION)){
    if (predicate == NULL){
      err(parser, lex, "expected an expression for the predicate");
      return NULL;
    }

    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("? (");

    if ((yes = expr(parser, lex)) == NULL){
      yes = predicate;
    }

    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf(") ");
    force(parser, lex, TOK_COLON);
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf(": (");

    if ((no = ternary_expr(parser, lex)) == NULL){
      err(parser, lex, "expected an expression for the 'no' branch");
      return NULL;
    }

    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf(")");

    ret = new_ternop(parser, lex, predicate, yes, no);
    ret->lvalue = true; /* hmmm... */
  }

  return ret;
  /* }}} */
}

static struct node *assign_expr(struct parser *parser, struct lexer *lex)
{
  /* {{{ */
  struct node *left, *right, *ret;
  enum binop_type type;

  left = ret = ternary_expr(parser, lex);

  while (peek(parser, lex, TOK_EQ)             ||
         peek(parser, lex, TOK_PLUS_EQ)        ||
         peek(parser, lex, TOK_MINUS_EQ)       ||
         peek(parser, lex, TOK_TIMES_EQ)       ||
         peek(parser, lex, TOK_SLASH_EQ)       ||
         peek(parser, lex, TOK_PERCENT_EQ)     ||
         peek(parser, lex, TOK_AMPERSAND_EQ)   ||
         peek(parser, lex, TOK_CARET_EQ)       ||
         peek(parser, lex, TOK_PIPE_EQ)        ||
         peek(parser, lex, TOK_LCHEVRON_2_EQ)  ||
         peek(parser, lex, TOK_RCHEVRON_2_EQ)){
    if (accept(parser, lex, TOK_EQ)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("= ");

      type = BINARY_ASSIGN;
    } else if (accept(parser, lex, TOK_PLUS_EQ)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("+= ");

      type = BINARY_ASSIGN_ADD;
    } else if (accept(parser, lex, TOK_MINUS_EQ)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("-= ");

      type = BINARY_ASSIGN_SUB;
    } else if (accept(parser, lex, TOK_TIMES_EQ)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("*= ");

      type = BINARY_ASSIGN_MUL;
    } else if (accept(parser, lex, TOK_SLASH_EQ)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("/= ");

      type = BINARY_ASSIGN_DIV;
    } else if (accept(parser, lex, TOK_PERCENT_EQ)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("%%= ");

      type = BINARY_ASSIGN_MOD;
    } else if (accept(parser, lex, TOK_AMPERSAND_EQ)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("&= ");

      type = BINARY_ASSIGN_AND;
    } else if (accept(parser, lex, TOK_CARET_EQ)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("^= ");

      type = BINARY_ASSIGN_XOR;
    } else if (accept(parser, lex, TOK_PIPE_EQ)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("|= ");

      type = BINARY_ASSIGN_OR;
    } else if (accept(parser, lex, TOK_LCHEVRON_2_EQ)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("<<= ");

      type = BINARY_ASSIGN_SHL;
    } else if (accept(parser, lex, TOK_RCHEVRON_2_EQ)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf(">>= ");

      type = BINARY_ASSIGN_SHR;
    }

    if (left == NULL || left->lvalue == false){
      err(parser, lex, "expected an lvalue at the LHS of the binary '%s' operation", binop_to_s(type));
      return NULL;
    }

    right = ternary_expr(parser, lex);

    if (!right){
      err(parser, lex, "expected an expression at the RHS of the binary '%s' operation", binop_to_s(type));
      return NULL;
    }

    ret = new_binop(parser, lex, type, left, right);
    ret->lvalue = false;
    right = ret;
  }

  return ret;
  /* }}} */
}

static struct node *no_comma_expr(struct parser *parser, struct lexer *lex)
{
  /* {{{ */
  return assign_expr(parser, lex);
  /* }}} */
}

static struct node *comma_expr(struct parser *parser, struct lexer *lex)
{
  /* {{{ */
  struct node *left, *right, *ret;

  left = ret = no_comma_expr(parser, lex);

  while (accept(parser, lex, TOK_COMMA)){
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf(", ");

    right = no_comma_expr(parser, lex);

    if (!right){
      err(parser, lex, "expected an expression at the RHS of the"
          " binary '%s' operation", binop_to_s(BINARY_COMMA));
      return NULL;
    }

    ret = new_binop(parser, lex, BINARY_COMMA, left, right);
    ret->lvalue = false; /* hmm.. */
    left = ret;
  }

  return ret;
  /* }}} */
}

static struct node *expr(struct parser *parser, struct lexer *lex)
{
  /* {{{ */
  struct node *ret = NULL;

  if (accept_keyword(parser, lex, "if")){
    /* {{{ */
    struct node *guard;
    struct node *body;
    struct node *elsee;

    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("if ");
    force(parser, lex, TOK_LPAREN);
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("(");

    guard = expr(parser, lex);
    /* no expression, that's a bummer */
    if (!guard){
      err(parser, lex, "expected an expression for if's guard");
      return NULL;
    }

    force(parser, lex, TOK_RPAREN);
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf(")\n");

    body = expr(parser, lex);

    if (!body){
      err(parser, lex, "expected an expression for if's body");
      return NULL;
    }

    force_keyword(parser, lex, "else");
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("else ");
    elsee = expr(parser, lex);

    if (!elsee){
      err(parser, lex, "expected an expression for if's else branch");
      return NULL;
    }

    /* we turn these into calls so that the user isn't forced to do this every
     * time he wants to branch out:
     *
     *   if (...){
     *     ...
     *   }() else {
     *     ...
     *   }();
     *
     * this feature is actually open to conversation as it strikes a bit against
     * consistency, observe:
     *
     *   my function = { ... };
     *
     *   if (...)
     *     function();
     *   else
     *     ...;
     *
     * in this example the 'true' branch would return the result of the
     * function, and not the function itself, like here:
     *
     *   my function = { ... };
     *
     *   if (...)
     *     function;
     *   else
     *     ...;
     *
     * in order to actually return an anonymous function using brackets, the
     * user would have to do this:
     *
     *   if (...){
     *     { ... }
     *   } else {
     *     { ... }
     *   };
     */
    if (body->type == NT_FUN)
      body = new_call(parser, lex, body, NULL, NULL);

    if (elsee->type == NT_FUN)
      elsee = new_call(parser, lex, elsee, NULL, NULL);

    ret = new_if(parser, lex, guard, body, elsee);
    ret->lvalue = false;
    /* }}} */
  }
  else if (accept_keyword(parser, lex, "my")){
    /* {{{ */
    char *name;
    uint8_t flags = 0x0;
    struct node *value = NULL;
    struct nob_type *var_type, *val_type;

    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("my ");

    if (accept_keyword(parser, lex, "mutable")){
      NOB_FLAG_SET(flags, NOB_FLAG_MUTABLE);

      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("mutable ");
    }

    var_type = type(parser, lex);

    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      putchar(' ');

    force(parser, lex, TOK_NAME);

    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("%s ", lex->curr_tok.value.s);

    name = strdup(lex->curr_tok.value.s);

    /*
     * if the type was omitted, then the initialization should probably be
     * obligatory.
     *
     * the point is to know the variable's type beforehand.
     *
     * but hey, we need type inference first..
     */

    if (peek(parser, lex, TOK_LMUSTASHE)){
      value = expr(parser, lex);
    } else if (accept(parser, lex, TOK_EQ)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf(" = ");
      /* my [type] name = expr... */
      /* the variable's initial value */
      if ((value = expr(parser, lex)) == NULL){
        err(parser, lex, "nothing was initialized");
        return NULL;
      }
    } else {
      /* see if a type was given */
      if (!var_type){
        err(parser, lex, "uninitialized variable lacks a type declaration");
        return NULL;
      }
    }

    /* see if the variable already exists */
    /* TODO there can be two different variables with the same name, but only
     *      when they are in two different modules */
    if (var_lookup(name, parser->curr_scope)){
      err(parser, lex, "variable '%s' already exists", name);
      return NULL;
    }

    val_type = infer_node_type(parser->curr_scope, value);

    if (val_type == NULL){
      err(parser, lex, "couldn't infer the type of the expression");
      return NULL;
    }

    /* TODO check whether the val_type and var_type are equal, and shout if
     * they're not */

    /* declare the variable in the current scope */
    struct var *var = new_var(name, flags, value, val_type, parser->curr_scope,
        false /* not a param */, 0);
    ret = new_decl(parser, lex, var);
    var->decl = ret;
    ret->lvalue = false; /* hmm.. */

    if (value)
      /* see if the assigned value is a function */
      if (value->type == NT_FUN){
        /* we make the assignment so that the function knows the name it was
         * given during the declaration (necessary for the assembly so the
         * functions don't have cryptic names) */
        value->in.fun.name = name;
      }
    /* }}} */
  }
  else if (accept_keyword(parser, lex, "print")){
    /* {{{ */
    struct nodes_list *exprs = NULL;
    struct node *e;

    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("print ");

    if ((e = no_comma_expr(parser, lex)) != NULL){
      struct nodes_list *new = nmalloc(sizeof(struct nodes_list));

      new->node = e;
      new->next = exprs;
      exprs = new;

      while (accept(parser, lex, TOK_COMMA)){
        if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
          printf(", ");
        e = no_comma_expr(parser, lex);

        if (e){
          new = nmalloc(sizeof(struct nodes_list));
          new->node = e;
          new->next = exprs;
          exprs = new;
        }
      }
    }

    ret = new_print(parser, lex, reverse_nodes_list(exprs));
    ret->lvalue = false;
    /* }}} */
  }
  else if (accept_keyword(parser, lex, "typedef")){
    /* {{{ */
    /*
     * given an example typedef:
     *
     *     typedef option of 'a {
     *       :None;
     *       :Some of 'a;
     *     };
     *
     * `custom_type` is option of 'a (ie. the whole type)
     * `gen_type`    is (just) the single 'a
     * `param`       is the param a constructor will take
     */
    struct nob_type *custom_type;
    struct nob_type *gen_type = NULL;
    struct node *value;
    struct nob_type *param = T_VOID;
    char *ctor_name;

    force(parser, lex, TOK_NAME);

    custom_type = new_type(OT_CUSTOM, lex->curr_tok.value.s, NULL);

    if (accept_keyword(parser, lex, "of")){
      if ((gen_type = type(parser, lex)) == NULL){
        err(parser, lex, "expected a type after 'of'");
        return NULL;
      }

      custom_type->info.custom.var = gen_type;
    }

    if (accept(parser, lex, TOK_LMUSTASHE)){
      while (accept(parser, lex, TOK_NAME)){
        ctor_name = strdup(lex->curr_tok.value.s);

        if (accept_keyword(parser, lex, "of")){
          if (gen_type == NULL){
            /* is "parametrized" the right word? */
            err(parser, lex, "the data type isn't parametrized");
            return NULL;
          } else if ((param = type(parser, lex)) == NULL){
            err(parser, lex, "expected a type after '%s of'", ctor_name);
            return NULL;
          }

          value = new_fun(parser, lex, ctor_name, "%1", NULL, NULL, false);
          value->result_type = new_type(OT_FUN, custom_type, param);
        } else {
          /* hmr.. it probably doesn't matter what kind of node it actually is
           * because this constructor isn't carrying any additional data */
          value = new_nop(parser, lex);
          /* oh this definitely matters */
          value->result_type = custom_type;
        }

        new_var(ctor_name, 0x0, value, value->result_type, parser->curr_scope, 0, 0);

        force(parser, lex, TOK_SEMICOLON);
      }

      force(parser, lex, TOK_RMUSTASHE);
    }

    /* FIXME? */
    ret = new_int(parser, lex, 1);
    ret->lvalue = false;
    /* }}} */
  }
  else { /* none of the above */
    /* {{{ */
    ret = comma_expr(parser, lex);

    if (ret)
      ret->lvalue = false;
    /* }}} */
  }

  return ret;
  /* }}} */
}

struct node *expr_list(struct parser *parser, struct lexer *lex)
{
  /* {{{ */
  struct node *first = NULL, *prev = NULL, *last;

  while (!peek(parser, lex, TOK_EOS) && !peek(parser, lex, TOK_RMUSTASHE)){
    /* overwrite `first' if NULL (ie. set it only the first time) */
    last = expr(parser, lex);
    first = !first ? last : first;

    if (prev)
      if (!prev->next)
        prev->next = last;

    prev = last;

    expr_end(parser, lex);
  }

  /* set the last expression's `next' to NULL (ie. terminate the sequence) */
  if (last)
    last->next = NULL;

  return first;
  /* }}} */
}

struct node *parse_file(char *fname, struct scope *scope)
{
  FILE *fptr;
  size_t flen;
  char *fbuf; /* the file's contents */
  struct stat st;
  struct lexer lex;
  struct parser parser;
  struct node *ret;

  if ((fptr = fopen(fname, "r")) == NULL){
    fprintf(stderr, "fopen: %s: %s\n", fname, strerror(errno));
    return 0;
  }

  /* get the file's size in bytes */
  if (stat(fname, &st) == -1){
    fprintf(stderr, "stat: %s: %s\n", fname, strerror(errno));
    return 0;
  }

  flen = st.st_size;

  /* make space for the file's contents */
  fbuf = nmalloc(sizeof(char) * flen);

  /* fetch the file's contents */
  if (fread(fbuf, sizeof(char), flen, fptr) != flen){
    fprintf(stderr, "fread: %s\n", strerror(errno));
    return 0;
  }
  /* nul-terminate the contents */
  fbuf[flen - 1] = '\0';

  /* initialize the parser's state */
  parser.errorless      = true;
  parser.curr_scope     = scope;
  parser.type_vars      = NULL;
  /* initialize the lexer's state */
  lex.fptr              = fptr;
  lex.name              = fname;
  lex.source            = fbuf;
  lex.curr_pos          = fbuf;
  lex.curr_tok.type     = TOK_EOS;
  lex.curr_tok.value.sp = NULL;
  lex.line              = 1;
  lex.col               = 1;
  lex.save.line         = 1;
  lex.save.col          = 1;
  lex.save.pos          = fbuf;
  lex.str_gc.size       = 16;
  lex.str_gc.ptr        = ncalloc(lex.str_gc.size, sizeof(char *));
  lex.str_gc.curr       = lex.str_gc.ptr;
  lex.nodes             = NULL;

  /* start the parsing process */
  ret = expr_list(&parser, &lex);
  dump_nodes(ret);

  if (!parser.errorless)
    return NULL;

  fclose(fptr);
  nfree(fbuf);

  return ret;
}

struct node *parse_string(char *name, char *string, struct scope *scope)
{
  struct lexer lex;
  struct parser parser;
  struct node *ret;

  /* initialize the parser's state */
  parser.errorless      = true;
  parser.curr_scope     = scope;
  parser.type_vars      = NULL;
  /* initialize the lexer's state */
  lex.fptr              = NULL;
  lex.name              = name;
  lex.source            = string;
  lex.curr_pos          = string;
  lex.curr_tok.type     = TOK_EOS;
  lex.curr_tok.value.sp = NULL;
  lex.line              = 1;
  lex.col               = 1;
  lex.save.line         = 1;
  lex.save.col          = 1;
  lex.save.pos          = string;
  lex.str_gc.size       = 4;
  lex.str_gc.ptr        = ncalloc(lex.str_gc.size, sizeof(char *));
  lex.str_gc.curr       = lex.str_gc.ptr;
  lex.nodes             = NULL;

  /* start the parsing process */
  ret = expr_list(&parser, &lex);
  dump_nodes(ret);

  if (parser.errorless)
    return ret;
  else
    return NULL;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

