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

#include "lexer.h"
#include "utf8.h"
#include "util.h"

void stmt(struct lexer_t *lex)
{
  while (peek(lex, T_KEYWORD) || peek(lex, T_NAME) || peek(lex, T_STRING)){
    printf("got a keyword, name or a string: ");
    if (accept(lex, T_KEYWORD)){
      printf("a keyword: %s\n", lex->curr_tok.value.s);
    } else if (accept(lex, T_STRING)){
      printf("a string:  %s (len %u)\n", lex->curr_tok.value.sp, u8_strlen(lex->curr_tok.value.sp));
    } else {
      accept(lex, T_NAME);
      printf("a name:    %s (len %u)\n", lex->curr_tok.value.s, u8_strlen(lex->curr_tok.value.s));
    }
  }
}

void parse_file(char *fname)
{
  FILE *fptr;
  size_t flen;
  char *fbuf; /* the file's contents */
  char **p; /* used when freeing lex's `str_gc' */
  struct stat st;
  struct lexer_t lex;

  if ((fptr = fopen(fname, "r")) == NULL){
    fprintf(stderr, "fopen: %s: %s\n", fname, strerror(errno));
    return /* NULL */;
  }

  /* get the file's size in bytes */
  if (stat(fname, &st) == -1){
    fprintf(stderr, "stat: %s: %s\n", fname, strerror(errno));
    return /* NULL */;
  }

  flen = st.st_size;

  /* make space for the file's contents */
  if ((fbuf = malloc(sizeof(char) * flen)) == NULL){
    fprintf(stderr, "malloc: %s\n", strerror(errno));
    return /* NULL */;
  }

  /* fetch the file's contents */
  if (fread(fbuf, sizeof(char), flen, fptr) != flen){
    fprintf(stderr, "fread: %s\n", strerror(errno));
    return /* NULL */;
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
  lex.str_gc.size       = 32;
  if ((lex.str_gc.ptr = calloc(lex.str_gc.size, sizeof(char *))) == NULL){
    fprintf(stderr, "malloc failed to allocate %lu bytes in `parse_file'\n", sizeof(char *) * lex.str_gc.size);
    exit(1);
  }
  lex.str_gc.curr       = lex.str_gc.ptr;

  /* start the parsing process */
  stmt(&lex);

  /* free the lexer's `str_gc' */
  for (p = lex.str_gc.ptr; p != lex.str_gc.curr; p++){
    free(*p);
  }
  free(lex.str_gc.ptr);
  /* free the rest */
  free(fbuf);
  fclose(fptr);
}

void parse_string(char *string)
{
  struct lexer_t lex;

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
  lex.str_gc.size       = 32;
  if ((lex.str_gc.ptr = calloc(lex.str_gc.size, sizeof(char *))) == NULL){
    fprintf(stderr, "malloc failed to allocate %lu bytes in `parse_file'\n", sizeof(char *) * lex.str_gc.size);
    exit(1);
  }
  lex.str_gc.curr       = lex.str_gc.ptr;

  /* start the parsing process */
  stmt(&lex);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

