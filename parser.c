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
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "ast.h"
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
      printf(" ENDSTMT;\n"); \
    } \
  } while (0)

/* forward declarations */
struct node *expr_list(struct parser *parser, struct lexer *lex);
static struct node *expr(struct parser *parser, struct lexer *lex);

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
    printf("* ");
    ret = T_ANY;
    /* }}} */
  } else if (accept(lex, TOK_TYPE)){
    /* {{{ a single worded type */
    printf("%s", lex->curr_tok.value.s);
    ret = get_type_by_name(lex->curr_tok.value.s);
    /* }}} */
  } else if (accept(lex, TOK_LMUSTASHE)){
    /* {{{ a function */
    printf("{ ");
    if (accept(lex, TOK_TIMES)){
      /* {{{ a polymorphic function {*} */
      printf("*");
      ret = new_type(NULL /* no name */, OT_FUN, NULL, NULL /* hmm.. */);
      /* }}} */
    } else {
      /* {{{ a function's 'proper' prototype { return type... } */
      /* fetch the return type */
      return_type = type(parser, lex);

      /* fetch the optional params types */
      if (accept(lex, TOK_SEMICOLON)){
        printf("; ");

        /* {{{ { return type; ... } */
        if ((params[curr_param++] = type(parser, lex)) != NULL){
          /* a function with at least one parameter */
          while (accept(lex, TOK_COMMA) && curr_param < MAX_FUN_PARAMS){
            /* a function with more parameters */
            printf(", ");

            if ((params[curr_param++] = type(parser, lex)) == NULL){
              printf("note: expected a type after the comma\n");
            }
          }

          ret = new_type(NULL /* no name */, OT_FUN, return_type, params);
        } else {
          /* the function takes no parameters */
          ret = new_type(NULL /* no name */, OT_FUN, return_type, NULL);
        }
        /* }}} */
      }
      /* }}} */
    }

    force(parser, lex, TOK_RMUSTASHE);
    printf(" }");
    /* }}} */
  } else if (accept(lex, TOK_LPAREN)){
    /* {{{ a tuple */
    /* TODO there should be no polymorphic types inside a tuple */
    printf("(");
    fields[curr_field].type = type(parser, lex);

    if (fields[curr_field].type == NULL){
      fprintf(stderr, "error: expected a type\n");
      exit(1);
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
              fprintf(stderr, "error: duplicate field names ('%s') in a tuple\n", p->name); \
              exit(1); \
            } \
          } \
          printf(" %s", lex->curr_tok.value.s); \
          fields[curr_field].name = strdup(lex->curr_tok.value.s); \
        } else { \
          fprintf(stderr, "the field is missing it's name"); \
          exit(1); \
        } \
      } while (0)
      /* }}} */

    fetch_name(lex);
    curr_field++;

    /* fetch more fields if present */
    while (accept(lex, TOK_COMMA) && curr_field < MAX_TUPLE_FIELDS){
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
    printf(")");
    /* }}} */
  } else if (accept(lex, TOK_LBRACKET)){
    /* {{{ a list */
    printf("[");

    return_type = type(parser, lex);
    /* it's not a return type, just reusing the variable */
    if (!return_type){
      fprintf(stderr, "error: expected a type for the list\n");
      exit(1);
    }

    ret = new_type(NULL /* no name */, OT_LIST, return_type);

    force(parser, lex, TOK_RBRACKET);
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

    printf("{\n");

    body = expr_list(parser, lex);

    force(parser, lex, TOK_RMUSTASHE);
    printf("}\n");

    ret = new_fun(lex, NULL /* anonymous */, T_INT /* wait for inference */,
        NULL /* no params */, body, NULL /* no opts */,
        true /* execute right away */);
    ret->lvalue = false; /* hmm.. */
    /* }}} */
  } else if (accept(lex, TOK_INTEGER)){
    printf("%s ", infnum_to_str(lex->curr_tok.value.i));
    ret = new_int(lex, lex->curr_tok.value.i);
    ret->lvalue = false;
  } else if (accept(lex, TOK_FLOAT)){
    printf("%f=16 ", lex->curr_tok.value.f);
    ret = new_int(lex, infnum_from_int(16));
    ret->lvalue = false;
  } else if (accept(lex, TOK_STRING)){
    printf("\"%s\"=32", lex->curr_tok.value.sp);
    ret = new_int(lex, infnum_from_int(32));
    ret->lvalue = false;
  } else if (accept(lex, TOK_NAME)){
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
        printf(" postfix(++)");
        ret = new_unop(lex, UNARY_POSTINC, target);
        ret->lvalue = false;
      } else {
        *lex = save_lex;
      }
    } else if (accept(lex, TOK_MINUS)){
      if (accept(lex, TOK_MINUS)){
        /* target -- */
        printf(" postfix(--)");
        ret = new_unop(lex, UNARY_POSTDEC, target);
        ret->lvalue = false;
      } else {
        *lex = save_lex;
      }
    } else if (accept(lex, TOK_LPAREN)){
      force(parser, lex, TOK_RPAREN);
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
        printf("prefix(++) ");
        target = postfix_expr(parser, lex);
        target->lvalue = false;
        type = UNARY_PREINC;
      } else {
        /*  + target */
        printf("prefix(+) ");
        target = postfix_expr(parser, lex);
        target->lvalue = false;
        type = UNARY_PLUS;
      }
    } else if (accept(lex, TOK_MINUS)){
      if (accept(lex, TOK_MINUS)){
        /* -- target */
        printf("prefix(--) ");
        target = postfix_expr(parser, lex);
        target->lvalue = false;
        type = UNARY_PREDEC;
      } else {
        /*  - target */
        printf("prefix(-) ");
        target = postfix_expr(parser, lex);
        target->lvalue = false;
        type = UNARY_MINUS;
      }
    } else if (accept(lex, TOK_BANG)){
      /*  ! target */
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
      printf("* ");
      type = BINARY_MUL;
    } else if (accept(lex, TOK_SLASH)){
      printf("/ ");
      type = BINARY_DIV;
    } else if (accept(lex, TOK_PERCENT)){
      printf("%% ");
      type = BINARY_MOD;
    }

    right = prefix_expr(parser, lex);

    if (!right){
      fprintf(stderr, "mul_expr: expected an expression at the RHS of the binary '%s' operation\n", binop_to_s(type));
      parser->errorless = false;
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
      printf("+ ");
      type = BINARY_ADD;
    } else if (accept(lex, TOK_MINUS)){
      printf("- ");
      type = BINARY_SUB;
    }

    right = mul_expr(parser, lex);

    if (!right){
      fprintf(stderr, "add_expr: expected an expression at the RHS of the binary '%s' operation\n", binop_to_s(type));
      parser->errorless = false;
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
        printf("<= ");
        type = BINARY_LE;
      } else {
        printf("< ");
        type = BINARY_LT;
      }
    } else if (accept(lex, TOK_RCHEVRON)){
      if (accept(lex, TOK_EQ)){
        printf(">= ");
        type = BINARY_GE;
      } else {
        printf("> ");
        type = BINARY_GT;
      }
    }

    right = add_expr(parser, lex);

    if (!right){
      fprintf(stderr, "cond_expr: expected an expression at the RHS of the binary '%s' operation\n", binop_to_s(type));
      parser->errorless = false;
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
        printf("== ");
        type = BINARY_EQ;
      } else {
        *lex = save_lex;
        return ret;
      }
    } else if (accept(lex, TOK_BANG)){
      if (accept(lex, TOK_EQ)){
        printf("!= ");
        type = BINARY_NE;
      }
    }

    right = cond_expr(parser, lex);

    if (!right){
      fprintf(stderr, "eq_expr: expected an expression at the RHS of the binary '%s' operation\n", binop_to_s(type));
      parser->errorless = false;
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
      fprintf(stderr, "expected an expression for the predicate\n");
      parser->errorless = false;
      return NULL;
    }

    printf("? (");

    if ((yes = expr(parser, lex)) == NULL){
      yes = predicate;
    }

    printf(") ");
    force(parser, lex, TOK_COLON);
    printf(": (");

    if ((no = ternary_expr(parser, lex)) == NULL){
      fprintf(stderr, "expected an expression for the 'no' branch\n");
      parser->errorless = false;
      return NULL;
    }

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
        fprintf(stderr, "expected an lvalue at the LHS of the binary '=' operation\n");
        parser->errorless = false;
        return NULL;
      }

      printf("= ");
      type = BINARY_ASSIGN;
    }

    right = ternary_expr(parser, lex);

    if (!right){
      fprintf(stderr, "assign_expr: expected an expression at the RHS of the binary '%s' operation\n", binop_to_s(type));
      parser->errorless = false;
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
    printf(", ");

    right = no_comma_expr(parser, lex);

    if (!right){
      fprintf(stderr, "comma_expr: expected an expression at the RHS of the"
          " binary '%s' operation\n", binop_to_s(BINARY_COMMA));
      parser->errorless = false;
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

    printf("if ");
    force(parser, lex, TOK_LPAREN);
    printf("(");

    guard = expr(parser, lex);
    /* no expression, that's a bummer */
    if (!guard){
      fprintf(stderr, "%s:%u.%u: warning: expected an expression for if's guard\n", lex->name, lex->line, lex->col);
      parser->errorless = false;
      return NULL;
    }

    force(parser, lex, TOK_RPAREN);
    printf(")\n");

    body = expr(parser, lex);

    if (!body){
      fprintf(stderr, "%s:%u.%u: warning: expected an expression for if's body\n", lex->name, lex->line, lex->col);
      parser->errorless = false;
      return NULL;
    }

    force_keyword(parser, lex, "else");
    printf("else ");
    elsee = expr(parser, lex);

    if (!elsee){
      fprintf(stderr, "%s:%u.%u: warning: expected an expression for if's else branch\n", lex->name, lex->line, lex->col);
      parser->errorless = false;
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

    printf("my ");

    if (accept_keyword(lex, "const")){
      NOB_FLAG_SET(flags, NOB_FLAG_CONST);
    }

    var_type = type(parser, lex);

    putchar(' ');
    force(parser, lex, TOK_NAME);
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
      printf(" = ");
      /* my [type] name = expr... */
      /* the variable's initial value */
      if ((value = expr(parser, lex)) == NULL){
        fprintf(stderr, "nothing was initialized\n");
        parser->errorless = false;
        return NULL;
      }
    } else {
      /* see if a type was given */
      if (!var_type){
        fprintf(stderr, "uninitialized variables lacks a type declaration\n");
        parser->errorless = false;
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

    printf("print ");

    if ((e = no_comma_expr(parser, lex)) != NULL){
      struct nodes_list *new = nmalloc(sizeof(struct nodes_list));

      new->node = e;
      new->next = exprs;
      exprs = new;

      while (accept(lex, TOK_COMMA)){
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

    printf("typedef ");
    new_type = type(parser, lex);
    /* ouch, it's not really a type! */
    if (!new_type){
      fprintf(stderr, "error: expected a type\n");
      parser->errorless = false;
      return NULL;
    }
    /* get the name for the type */
    force(parser, lex, TOK_NAME);
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

      if (accept_keyword(lex, "lim")){
        struct node *lower, *upper;
        printf(" lim ");

        if (newer_type->primitive != OT_INTEGER){
          fprintf(stderr, "the construct `lim' is only supported for integers\n");
          parser->errorless = false;
          return NULL;
        }

        lower = primary_expr(parser, lex);
        force(parser, lex, TOK_COMMA);
        printf(", ");
        upper = primary_expr(parser, lex);

        if (infnum_cmp(lower->in.i, upper->in.i) == INFNUM_CMP_GE){
          fprintf(stderr, "invalid values for `lim'\n");
          parser->errorless = false;
          return NULL;
        }

        newer_type->info.integer.limitless = 0;
        newer_type->info.integer.limit_lower = infnum_to_qword(lower->in.i);
        newer_type->info.integer.limit_upper = infnum_to_qword(upper->in.i);
      }
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

int parse_file(char *fname)
{
  FILE *fptr;
  size_t flen;
  char *fbuf; /* the file's contents */
  struct stat st;
  struct lexer lex;
  struct parser parser;
  struct node *node;
  /* return value (allgood by default) */
  int ret = 1;
  /* used when freeing lex's `str_gc' and `nds_gc' */
  unsigned i;

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
  node = expr_list(&parser, &lex);
  dump_nodes(node);

  if (parser.errorless)
    exec_nodes(node);
  else
    /* not returning right away, because there is memory to be freed */
    ret = 0;

  /* free the lexer's `str_gc' */
  for (i = 0; i < lex.str_gc.size; i++)
    nfree(lex.str_gc.ptr[i]);
  nfree(lex.str_gc.ptr);
  /* same for the nodes */
  for (i = 0; i < lex.nds_gc.size; i++)
    nfree(lex.nds_gc.ptr[i]);
  nfree(lex.nds_gc.ptr);
  /* free the rest */
  nfree(fbuf);
  fclose(fptr);

  return ret;
}

int parse_string(char *string)
{
  struct lexer lex;
  struct parser parser;

  struct node *node;
  /* return value */
  int ret = 1;
  /* used when freeing lex's `str_gc' and `nds_gc' */
  unsigned i;

  /* initialize the parser's state */
  parser.errorless      = true;
  /* initialize the lexer's state */
  lex.fptr              = NULL;
  lex.name              = string;
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
  node = expr_list(&parser, &lex);
  dump_nodes(node);

  if (parser.errorless)
    exec_nodes(node);
  else
    ret = 0;

  /* free the lexer's `str_gc' */
  for (i = 0; i < lex.str_gc.size; i++)
    nfree(lex.str_gc.ptr[i]);
  nfree(lex.str_gc.ptr);
  /* same for the nodes */
  for (i = 0; i < lex.nds_gc.size; i++)
    nfree(lex.nds_gc.ptr[i]);
  nfree(lex.nds_gc.ptr);

  return ret;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

