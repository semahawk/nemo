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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "ast.h"
#include "mem.h"
#include "nob.h"
#include "lexer.h"
#include "utf8.h"
#include "util.h"

/*
 * Fetches the type, at the lexer's current 'position'.
 * If the type already exists, like "int", it return's the pointer to it.
 *
 * If the type doesn't already exist (by the way, it can't be a single word,
 * lexer wouldn't let it through), like an anonymous tuple, creates it as a
 * whole new type, and returns the pointer to it.
 */
static struct nob_type *type(struct lexer *lex)
{
  /* the nob_type to be returned */
  struct nob_type *ret = NULL;
  /* the fields (names + associated types) used to be passed over to `new_type' to create a tuple type */
  struct field fields[MAX_TUPLE_FIELDS + 1] = { { 0, 0 } };
  /* 'pointer' to the current field */
  unsigned curr_field = 0;
  /* if inside of the <>s turns out to be a function's type this might come in
   * handy; to be passed to `new_type' */
  struct nob_type *return_type = NULL;
  /* function's parameters, to be passed to `new_type' */
  struct nob_type *params[MAX_FUN_PARAMS + 1] = { 0 };
  /* 'pointer' to the current parameter */
  unsigned curr_param = 0;

  if (accept(lex, TOK_TYPE)){
    /* {{{ a single worded type */
    printf("%s", lex->curr_tok.value.s);
    ret = get_type_by_name(lex->curr_tok.value.s);
    /* }}} */
  } else if (accept(lex, TOK_LCHEVRON)){
    /* {{{ a tuple or a function */
    printf("<");
    return_type = fields[curr_field].type = type(lex);

    if (accept(lex, TOK_SEMICOLON)){
      /* {{{ a function <type; ...> */
      printf("; ");

      if ((params[curr_param++] = type(lex)) != NULL){
        /* a function with at least one parameter */
        while (accept(lex, TOK_COMMA) && curr_param < MAX_FUN_PARAMS){
          /* a function with more parameters */
          printf(", ");

          if ((params[curr_param++] = type(lex)) == NULL){
            printf("note: expected a type after the comma\n");
          }
        }

        ret = new_type(NULL /* no name */, OT_FUN, return_type, params);
      } else {
        /* the function takes no parameters */

        /* meh, I'm kind of pedantic */
        /* clear the space and the semicolon */
        printf("\b\b");
        /* create the type */
        ret = new_type(NULL /* no name */, OT_FUN, return_type, NULL);
      }

      /* }}} */
    } else if (peek(lex, TOK_NAME)){
      /* {{{ a tuple <type name> */
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
        fields[curr_field].type = type(lex);
        fetch_name(lex);
        curr_field++;
      }

      /* NOTE: there's really no need to NULL-terminate the array, because it was
       * initialized to zeros, and && curr_field < MAX_TUPLE_FIELDS guards against
       * writing to the last element */
      ret = new_type(NULL /* no name */, OT_TUPLE, fields);
      /* bye! */
#undef fetch_name
      /* }}} */
    } else {
      /* {{{ a function with no parameters <type> */
      ret = new_type(NULL /* no name */, OT_FUN, return_type, NULL);
      /* }}} */
    }

    force(lex, TOK_RCHEVRON);
    printf(">");
    /* }}} */
  }

  /* meh, pointers */
  while (accept(lex, TOK_TIMES)){
    printf("*");
    ret = new_type(NULL /* no name */, OT_PTR, ret);
  }

  /* an array is basically any type followed by a pair of brackets */
  if (accept(lex, TOK_LBRACKET)){
    size_t nmemb = 0;
    printf("[");
    /* the array's nmemb */
    force(lex, TOK_INTEGER);
    printf("%d", lex->curr_tok.value.i);
    nmemb = lex->curr_tok.value.i;
    force(lex, TOK_RBRACKET);
    printf("]");

    ret = new_type(NULL /* no name */, OT_ARRAY, nmemb, ret);
  }

  return ret;
}

static struct node *primary(struct lexer *lex)
{
  if (accept(lex, TOK_INTEGER)){
    printf("%d ", lex->curr_tok.value.i);
    return new_int(lex, lex->curr_tok.value.i);
  } else if (accept(lex, TOK_FLOAT)){
    printf("%f=16 ", lex->curr_tok.value.f);
    return new_int(lex, 16);
  } else if (accept(lex, TOK_STRING)){
    printf("\"%s\"=32", lex->curr_tok.value.sp);
    return new_int(lex, 32);
  } else if (accept(lex, TOK_NAME)){
    printf("%s=0x6e ", lex->curr_tok.value.s /* meh */);
    return new_int(lex, 0x6e);
  }

  return NULL;
}

static struct node *expr(struct lexer *lex)
{
  struct node *right, *left, *ret;

  left = ret = primary(lex);

  while (peek(lex, TOK_PLUS)  || peek(lex, TOK_MINUS)
      || peek(lex, TOK_TIMES) || peek(lex, TOK_SLASH)){
    if (accept(lex, TOK_PLUS)){
      printf("+ ");
    } else if (accept(lex, TOK_MINUS)){
      printf("- ");
    } else if (accept(lex, TOK_TIMES)){
      printf("* ");
    } else if (accept(lex, TOK_SLASH)){
      printf("/ ");
    }

    right = primary(lex);

    if (!right){
      fprintf(stderr, "expected an expression at the RHS of the binary operation\n");
      exit(1);
    }

    ret = new_binop(lex, BINARY_ADD, left, right);
    left = ret;
  }

  return ret;
}

struct node *stmt(struct lexer *lex)
{
  struct node *ret = new_nop(lex);

  while (!peek(lex, TOK_EOS)){
    if (accept_keyword(lex, "my")){
      printf("my ");
      if (!type(lex)){
        fprintf(stderr, "syntax error: expected a type for the variable declaration\n");
        exit(1);
      }
      putchar(' ');
      force(lex, TOK_NAME);
      printf("%s", lex->curr_tok.value.s);
    }
    else if (accept_keyword(lex, "typedef")){
      struct nob_type *new_type;

      printf("typedef ");
      new_type = type(lex);
      /* ouch, it's not really a type! */
      if (!new_type){
        fprintf(stderr, "error: expected a type\n");
        exit(1);
      }
      /* get the name for the type */
      force(lex, TOK_NAME);
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
    }
    else {
      ret = expr(lex);
    }

    force(lex, TOK_SEMICOLON);
    printf(";\n");
  }

  return ret;
}

int parse_file(char *fname)
{
  FILE *fptr;
  size_t flen;
  char *fbuf; /* the file's contents */
  char **p; /* used when freeing lex's `str_gc' */
  struct stat st;
  struct lexer lex;
  struct node *node;

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
  if ((fbuf = malloc(sizeof(char) * flen)) == NULL){
    fprintf(stderr, "malloc: %s\n", strerror(errno));
    return 0;
  }

  /* fetch the file's contents */
  if (fread(fbuf, sizeof(char), flen, fptr) != flen){
    fprintf(stderr, "fread: %s\n", strerror(errno));
    return 0;
  }
  /* nul-terminate the contents */
  fbuf[flen - 1] = '\0';

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
  lex.nds_pool.size     = 32;
  lex.nds_pool.ptr      = ncalloc(lex.nds_pool.size, sizeof(struct node));
  lex.nds_pool.curr     = lex.nds_pool.ptr;

  /* start the parsing process */
  node = stmt(&lex);
  exec_nodes(node);

  /* free the lexer's `str_gc' */
  for (p = lex.str_gc.ptr; p != lex.str_gc.curr; p++){
    nfree(*p);
  }
  nfree(lex.str_gc.ptr);
  /* free the rest */
  nfree(lex.nds_pool.ptr);
  nfree(fbuf);
  fclose(fptr);

  return 1;
}

int parse_string(char *string)
{
  struct lexer lex;
  struct node *node;

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
  lex.nds_pool.size     = 16;
  lex.nds_pool.ptr      = ncalloc(lex.nds_pool.size, sizeof(struct node));
  lex.nds_pool.curr     = lex.nds_pool.ptr;

  /* start the parsing process */
  node = stmt(&lex);
  exec_nodes(node);

  /* tidy up */
  nfree(lex.str_gc.ptr);
  nfree(lex.nds_pool.ptr);

  return 1;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

