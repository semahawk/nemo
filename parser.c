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
#include "infnum.h"
#include "lexer.h"
#include "parser.h"
#include "utf8.h"
#include "util.h"

/* expressions end with a semicolon, unless it's the last expressions in the block
 * (or the whole program/module/unit) */
#define expr_end(parser, lex) \
  do { \
    if (!peek(lex, TOK_RMUSTASHE) && \
        !peek(lex, TOK_EOS)){ \
      force(parser, lex, TOK_SEMICOLON); \
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER)) \
        printf(" ENDSTMT;\n"); \
    } \
  } while (0)

/* forward declarations */
struct node *expr_list(struct parser *, struct lexer *);
static struct node *expr(struct parser *, struct lexer *);
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
  /* the fields (names + associated types) used to be passed over to `new_type' to create a tuple type */
  struct field fields[MAX_TUPLE_FIELDS + 1] = { { 0, 0 } };
  /* 'pointer' to the current field */
  unsigned curr_field = 0;
  /* function's return type; to be passed to `new_type' */
  struct nob_type *return_type = NULL;
  /* function's parameters, to be passed to `new_type' */
  struct nob_type *params[MAX_FUN_PARAMS + 1] = { 0 };
  /* 'pointer' to the current parameter */
  unsigned curr_param = 0;

  if (accept(lex, TOK_TIMES)){
    /* {{{ a polymorphic type (any) */
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("* ");
    ret = T_ANY;
    /* }}} */
  } else if (accept(lex, TOK_TYPE)){
    /* {{{ a single worded type */
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("%s", lex->curr_tok.value.s);

    ret = get_type_by_name(lex->curr_tok.value.s);

    if (ret)
      if (ret->primitive == OT_INTEGER)
        if (accept_keyword(lex, "lim")){
          struct nob_type *new_type = nmalloc(sizeof(struct nob_type));
          struct node *lower, *upper;

          new_type->name = NULL;
          new_type->primitive = OT_INTEGER;

          if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
            printf(" lim ");

          lower = primary_expr(parser, lex);
          force(parser, lex, TOK_COMMA);

          if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
            printf(", ");

          upper = primary_expr(parser, lex);

          if (infnum_cmp(lower->in.i, upper->in.i) == INFNUM_CMP_GE){
            err(parser, lex, "invalid values for `lim'");
            return ret;
          }

          new_type->info.integer.limitless = 0;
          new_type->info.integer.limit_lower = infnum_to_qword(lower->in.i);
          new_type->info.integer.limit_upper = infnum_to_qword(upper->in.i);

          /* 'register' the new type */
          push_type(new_type);
          /* TODO: see if there already exists such type */
        }
    /* }}} */
  } else if (accept(lex, TOK_LMUSTASHE)){
    /* {{{ a function */
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("{ ");

    if (accept(lex, TOK_TIMES)){
      /* {{{ a polymorphic function {*} */
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("*");
      ret = new_type(NULL /* no name */, OT_FUN, NULL, NULL /* hmm.. */);
      /* }}} */
    } else {
      /* {{{ a function's 'proper' prototype { return type... } */
      /* fetch the return type */
      return_type = type(parser, lex);

      if (!return_type){
        err(parser, lex, "expected a return type in the function's prototype");
        return ret;
      }

      /* fetch the optional params types */
      if (accept(lex, TOK_SEMICOLON)){
        if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
          printf("; ");

        /* {{{ { return type; ... } */
        if ((params[curr_param++] = type(parser, lex)) != NULL){
          /* a function with at least one parameter */
          while (accept(lex, TOK_COMMA) && curr_param < MAX_FUN_PARAMS){
            /* a function with more parameters */
            if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
              printf(", ");

            if ((params[curr_param++] = type(parser, lex)) == NULL){
              fprintf(stderr, "note: expected a type after the comma\n");
            }
          }

          ret = new_type(NULL /* no name */, OT_FUN, return_type, params);
        } else {
          /* the function takes no parameters */
          ret = new_type(NULL /* no name */, OT_FUN, return_type, NULL);
        }
        /* }}} */
      } else {
        /* the function takes no parameters */
        ret = new_type(NULL /* no name */, OT_FUN, return_type, NULL);
      }
      /* }}} */
    }

    force(parser, lex, TOK_RMUSTASHE);
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf(" }");
    /* }}} */
  } else if (accept(lex, TOK_LPAREN)){
    /* {{{ a tuple */
    /* TODO there should be no polymorphic types inside a tuple */
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("(");
    fields[curr_field].type = type(parser, lex);

    if (fields[curr_field].type == NULL){
      err(parser, lex, "expected a type");
      return ret;
    }

#define fetch_name(lex) \
      /* {{{ fetch_name body */ \
      do { \
        /* see if the field was given a name, and if not, print a meaningful \
         * message */ \
        if (accept(lex, TOK_NAME)){ \
          struct field *p = fields; \
          for (; p->name != NULL && p->type != NULL; p++){ \
            if (!strcmp(p->name, lex->curr_tok.value.s)){ \
              err(parser, lex, "duplicate field names ('%s') in a tuple", p->name); \
              return ret; \
            } \
          } \
          if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER)) \
            printf(" %s", lex->curr_tok.value.s); \
          fields[curr_field].name = strdup(lex->curr_tok.value.s); \
        } else { \
          err(parser, lex, "the field is missing it's name"); \
          return ret; \
        } \
      } while (0)
      /* }}} */

    fetch_name(lex);
    curr_field++;

    /* fetch more fields if present */
    while (accept(lex, TOK_COMMA) && curr_field < MAX_TUPLE_FIELDS){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf(", ");
      fields[curr_field].type = type(parser, lex);
      fetch_name(lex);
      curr_field++;
    }

    /* NOTE: there's really no need to NULL-terminate the array, because it was
     * initialized to zeros, and && curr_field < MAX_TUPLE_FIELDS guards against
     * writing to the last element */
    ret = new_type(NULL /* no name */, OT_TUPLE, fields);
    /* bye! */
#undef fetch_name

    force(parser, lex, TOK_RPAREN);
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf(")");
    /* }}} */
  } else if (accept(lex, TOK_LBRACKET)){
    /* {{{ a list */
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("[");

    return_type = type(parser, lex);
    /* it's not a return type, just reusing the variable */
    if (!return_type){
      err(parser, lex, "expected a type for the list");
      return ret;
    }

    ret = new_type(NULL /* no name */, OT_LIST, return_type);

    force(parser, lex, TOK_RBRACKET);
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("]");
    /* }}} */
  }

  return ret;
  /* }}} */
}

static struct node *primary_expr(struct parser *parser, struct lexer *lex)
{
  /* {{{ */
  struct node *ret = NULL;

  if (accept(lex, TOK_LMUSTASHE)){ /* a block */
    /* {{{ */
    struct node *body;

    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("{\n");

    body = expr_list(parser, lex);

    force(parser, lex, TOK_RMUSTASHE);
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("}\n");

    ret = new_fun(lex, NULL /* anonymous */, T_INT /* wait for inference */,
        NULL /* no params */, body, NULL /* no opts */,
        true /* execute right away */);
    ret->lvalue = false; /* hmm.. */
    /* }}} */
  } else if (accept(lex, TOK_INTEGER)){
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("%s ", infnum_to_str(lex->curr_tok.value.i));
    ret = new_int(lex, lex->curr_tok.value.i);
    ret->lvalue = false;
  } else if (accept(lex, TOK_FLOAT)){
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("%f=16 ", lex->curr_tok.value.f);
    ret = new_int(lex, infnum_from_int(16));
    ret->lvalue = false;
  } else if (accept(lex, TOK_STRING)){
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("\"%s\"=32", lex->curr_tok.value.sp);
    ret = new_int(lex, infnum_from_int(32));
    ret->lvalue = false;
  } else if (accept(lex, TOK_NAME)){
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("%s=0x6e ", lex->curr_tok.value.s /* meh */);
    ret = new_int(lex, infnum_from_int(0x6e));
    ret->lvalue = true;
  }

  return ret;
  /* }}} */
}

static struct node *postfix_expr(struct parser *parser, struct lexer *lex)
{
  /* {{{ */
  struct node *target, *ret;
  struct lexer save_lex;

  target = ret = primary_expr(parser, lex);

  if (peek(lex, TOK_PLUS) || peek(lex, TOK_MINUS) || peek(lex, TOK_LPAREN)){
    /* save the lexer's state in case it's actually only one '+' or '-' */
    save_lex = *lex;

    if (accept(lex, TOK_PLUS)){
      if (accept(lex, TOK_PLUS)){
        /* target ++ */
        if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
          printf(" postfix(++)");
        ret = new_unop(lex, UNARY_POSTINC, target);
        ret->lvalue = false;
      } else {
        *lex = save_lex;
      }
    } else if (accept(lex, TOK_MINUS)){
      if (accept(lex, TOK_MINUS)){
        /* target -- */
        if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
          printf(" postfix(--)");
        ret = new_unop(lex, UNARY_POSTDEC, target);
        ret->lvalue = false;
      } else {
        *lex = save_lex;
      }
    } else if (accept(lex, TOK_LPAREN)){
      force(parser, lex, TOK_RPAREN);
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("(function call)");
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

  if (peek(lex, TOK_PLUS) || peek(lex, TOK_MINUS) || peek(lex, TOK_BANG)){
    if (accept(lex, TOK_PLUS)){
      if (accept(lex, TOK_PLUS)){
        /* ++ target */
        if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
          printf("prefix(++) ");
        target = postfix_expr(parser, lex);
        target->lvalue = false;
        type = UNARY_PREINC;
      } else {
        /*  + target */
        if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
          printf("prefix(+) ");
        target = postfix_expr(parser, lex);
        target->lvalue = false;
        type = UNARY_PLUS;
      }
    } else if (accept(lex, TOK_MINUS)){
      if (accept(lex, TOK_MINUS)){
        /* -- target */
        if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
          printf("prefix(--) ");
        target = postfix_expr(parser, lex);
        target->lvalue = false;
        type = UNARY_PREDEC;
      } else {
        /*  - target */
        if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
          printf("prefix(-) ");
        target = postfix_expr(parser, lex);
        target->lvalue = false;
        type = UNARY_MINUS;
      }
    } else if (accept(lex, TOK_BANG)){
      /*  ! target */
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("prefix(!) ");
      target = postfix_expr(parser, lex);
      target->lvalue = false;
      type = UNARY_NEGATE;
    }

    ret = new_unop(lex, type, target);
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

  while (peek(lex, TOK_TIMES) || peek(lex, TOK_SLASH) || peek(lex, TOK_PERCENT)){
    if (accept(lex, TOK_TIMES)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("* ");
      type = BINARY_MUL;
    } else if (accept(lex, TOK_SLASH)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("/ ");
      type = BINARY_DIV;
    } else if (accept(lex, TOK_PERCENT)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("%% ");
      type = BINARY_MOD;
    }

    right = prefix_expr(parser, lex);

    if (!right){
      err(parser, lex, "expected an expression at the RHS of the binary '%s' operation", binop_to_s(type));
      return NULL;
    }

    ret = new_binop(lex, type, left, right);
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

  while (peek(lex, TOK_PLUS) || peek(lex, TOK_MINUS)){
    if (accept(lex, TOK_PLUS)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("+ ");
      type = BINARY_ADD;
    } else if (accept(lex, TOK_MINUS)){
      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("- ");
      type = BINARY_SUB;
    }

    right = mul_expr(parser, lex);

    if (!right){
      err(parser, lex, "expected an expression at the RHS of the binary '%s' operation", binop_to_s(type));
      return NULL;
    }

    ret = new_binop(lex, type, left, right);
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

  left = ret = add_expr(parser, lex);

  if (peek(lex, TOK_LCHEVRON) || peek(lex, TOK_RCHEVRON)){
    if (accept(lex, TOK_LCHEVRON)){
      if (accept(lex, TOK_EQ)){
        if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
          printf("<= ");
        type = BINARY_LE;
      } else {
        if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
          printf("< ");
        type = BINARY_LT;
      }
    } else if (accept(lex, TOK_RCHEVRON)){
      if (accept(lex, TOK_EQ)){
        if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
          printf(">= ");
        type = BINARY_GE;
      } else {
        if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
          printf("> ");
        type = BINARY_GT;
      }
    }

    right = add_expr(parser, lex);

    if (!right){
      err(parser, lex, "expected an expression at the RHS of the binary '%s' operation", binop_to_s(type));
      return NULL;
    }

    ret = new_binop(lex, type, left, right);
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
  struct lexer save_lex;

  left = ret = cond_expr(parser, lex);

  while (peek(lex, TOK_EQ) || peek(lex, TOK_BANG)){
    save_lex = *lex;

    if (accept(lex, TOK_EQ)){
      if (accept(lex, TOK_EQ)){
        if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
          printf("== ");
        type = BINARY_EQ;
      } else {
        *lex = save_lex;
        return ret;
      }
    } else if (accept(lex, TOK_BANG)){
      if (accept(lex, TOK_EQ)){
        if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
          printf("!= ");
        type = BINARY_NE;
      }
    }

    right = cond_expr(parser, lex);

    if (!right){
      err(parser, lex, "expected an expression at the RHS of the binary '%s' operation", binop_to_s(type));
      return NULL;
    }

    ret = new_binop(lex, type, left, right);
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

  predicate = ret = eq_expr(parser, lex);

  if (accept(lex, TOK_QUESTION)){
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

    ret = new_ternop(lex, predicate, yes, no);
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

  while (peek(lex, TOK_EQ) /* TODO */){
    if (accept(lex, TOK_EQ)){
      if (left->lvalue == false){
        err(parser, lex, "expected an lvalue at the LHS of the binary '=' operation");
        return NULL;
      }

      if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
        printf("= ");
      type = BINARY_ASSIGN;
    }

    right = ternary_expr(parser, lex);

    if (!right){
      err(parser, lex, "expected an expression at the RHS of the binary '%s' operation", binop_to_s(type));
      return NULL;
    }

    ret = new_binop(lex, type, left, right);
    ret->lvalue = false;
    left = ret;
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

  while (accept(lex, TOK_COMMA)){
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf(", ");

    right = no_comma_expr(parser, lex);

    if (!right){
      err(parser, lex, "expected an expression at the RHS of the"
          " binary '%s' operation", binop_to_s(BINARY_COMMA));
      return NULL;
    }

    ret = new_binop(lex, BINARY_COMMA, left, right);
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

  if (accept_keyword(lex, "if")){
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

    ret = new_if(lex, guard, body, elsee);
    ret->lvalue = false;
    /* }}} */
  }
  else if (accept_keyword(lex, "my")){
    /* {{{ */
    char *name;
    uint8_t flags = 0x0;
    struct node *value = NULL;
    struct nob_type *var_type;

    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("my ");

    if (accept_keyword(lex, "const")){
      NOB_FLAG_SET(flags, NOB_FLAG_CONST);
    }

    var_type = type(parser, lex);

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

    if (peek(lex, TOK_LMUSTASHE)){
      value = expr(parser, lex);
    } else if (accept(lex, TOK_EQ)){
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

    ret = new_decl(lex, name, flags, value);
    ret->lvalue = false; /* hmm.. */
    /* }}} */
  }
  else if (accept_keyword(lex, "print")){
    /* {{{ */
    struct nodes_list *exprs = NULL;
    struct nodes_list *prev, *curr, *next;
    struct node *e;

    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("print ");

    if ((e = no_comma_expr(parser, lex)) != NULL){
      struct nodes_list *new = nmalloc(sizeof(struct nodes_list));

      new->node = e;
      new->next = exprs;
      exprs = new;

      while (accept(lex, TOK_COMMA)){
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

    /* reverse the list */
    prev = NULL;
    curr = exprs;

    while (curr != NULL){
      next = curr->next;
      curr->next = prev;
      prev = curr;
      curr = next;
    }

    exprs = prev;

    ret = new_print(lex, exprs);
    ret->lvalue = false;
    /* }}} */
  }
  else if (accept_keyword(lex, "typedef")){
    /* {{{ */
    struct nob_type *new_type;

    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf("typedef ");
    new_type = type(parser, lex);
    /* ouch, it's not really a type! */
    if (!new_type){
      err(parser, lex, "expected a type");
      return NULL;
    }
    /* get the name for the type */
    force(parser, lex, TOK_NAME);
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_PARSER))
      printf(" %s", lex->curr_tok.value.s);
    /* if the type's name is NULL, then it's an anonymous type, which means
     * that simply setting it's name would do the thing just perfectly */
    if (new_type->name == NULL){
      new_type->name = strdup(lex->curr_tok.value.s);
    } else {
      /* if the type has already been named, then we need to copy the type,
       * with a proper name */
      /* a FIXME/TODO here is not to create a whole new type if we're, kind
       * of, aliasing a type */
      struct nob_type *newer_type = nmalloc(sizeof(struct nob_type));
      /* copy the contents */
      memcpy(newer_type, new_type, sizeof(struct nob_type));
      /* set up the name */
      newer_type->name = strdup(lex->curr_tok.value.s);

      /* 'register' the type */
      push_type(newer_type);
    }

    ret = new_nop(lex);
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

  while (!peek(lex, TOK_EOS) && !peek(lex, TOK_RMUSTASHE)){
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

struct node *parse_file(char *fname)
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
  lex.nds_gc.size       = 32;
  lex.nds_gc.ptr        = ncalloc(lex.nds_gc.size, sizeof(struct node *));
  lex.nds_gc.curr       = lex.nds_gc.ptr;

  /* start the parsing process */
  ret = expr_list(&parser, &lex);
  dump_nodes(ret);

  if (!parser.errorless)
    return NULL;

  fclose(fptr);
  nfree(fbuf);

  return ret;
}

struct node *parse_string(char *name, char *string)
{
  struct lexer lex;
  struct parser parser;
  struct node *ret;

  /* initialize the parser's state */
  parser.errorless      = true;
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
  lex.nds_gc.size       = 8;
  lex.nds_gc.ptr        = ncalloc(lex.nds_gc.size, sizeof(struct node *));
  lex.nds_gc.curr       = lex.nds_gc.ptr;

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

