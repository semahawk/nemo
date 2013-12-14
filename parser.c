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
#include "lexer.h"
#include "utf8.h"
#include "util.h"

struct node *stmt(struct lexer *lex)
{
  while (peek(lex, T_KEYWORD) || peek(lex, T_NAME) || peek(lex, T_STRING) || peek(lex, T_TYPE)){
    printf("got a keyword, name, type name or a string: ");
    if (accept(lex, T_KEYWORD)){
      printf("a keyword: %s\n", lex->curr_tok.value.s);
    } else if (accept(lex, T_STRING)){
      printf("a string:  %s (len %u)\n", lex->curr_tok.value.sp, u8_strlen(lex->curr_tok.value.sp));
    } else if (accept(lex, T_TYPE)){
      printf("a type: %s\n", lex->curr_tok.value.s);
    } else {
      accept(lex, T_NAME);
      printf("a name:    %s (len %u)\n", lex->curr_tok.value.s, u8_strlen(lex->curr_tok.value.s));
    }
  }

  return
    new_if(lex,
      new_int(lex, (1 << 16) + 16),
      new_unop(lex, UNARY_MINUS,
        new_int(lex, 1 << 7)),
      new_binop(lex, BINARY_ADD,
        new_int(lex, 1 << 7),
        new_if(lex,
          new_int(lex, 1 << 7),
          new_unop(lex, UNARY_MINUS, new_int(lex, 1 << 7)),
          new_binop(lex, BINARY_SUB,
            new_int(lex, 1 << 7),
            new_int(lex, 1 << 7)
          )
        )
      )
    );
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
  lex.curr_tok.type     = T_EOS;
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
  lex.curr_tok.type     = T_EOS;
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

