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

void stmt(struct lexer_t *lex)
{
  /*if (accept(lex, T_KEYWORD)){*/
    /*printf("got a keyword: %s\n", lex->curr_tok.value.s);*/
  /*} else if (accept(lex, T_NAME)){*/
    /*printf("got a name: %s\n", lex->curr_tok.value.s);*/
  /*} else if (accept(lex, T_INTEGER)){*/
    /*printf("got an int: %d\n", lex->curr_tok.value.i);*/
  /*} else if (accept(lex, T_FLOAT)){*/
    /*printf("got a float: %f\n", lex->curr_tok.value.f);*/
  /*}*/

  while (peek(lex, T_KEYWORD) || peek(lex, T_NAME)){
    printf("got a keyword or name: ");
    if (accept(lex, T_KEYWORD)){
      printf("a keyword %s\n", lex->curr_tok.value.s);
    } else {
      accept(lex, T_NAME);
      printf("a name %s\n", lex->curr_tok.value.s);
    }
  }
}

void parse_file(char *fname)
{
  FILE *fptr;
  size_t flen;
  char *fbuf;  /* the file's contents */
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
  lex.valid_curr        = false;

  /* start the parsing process */
  stmt(&lex);
  stmt(&lex);

  free(fbuf);
  fclose(fptr);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

