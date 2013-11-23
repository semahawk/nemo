/*
 *
 * lexer.h
 *
 * Created at:  Thu Nov  7 19:12:48 2013 19:12:48
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License: please visit the LICENSE file for details.
 *
 */

#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

#include "config.h"
#include "nemo.h"
#include "ast.h"

enum token_type {
  T_INTEGER,        /*                */
  T_FLOAT,          /*                */
  T_STRING,         /*                */
  T_NAME,           /*                */
  T_KEYWORD,        /*                */
  T_OPT,            /*  -[a-zA-Z]     */
  T_EQ,             /*  "="           */
  T_SEMICOLON,      /*  ";"           */
  T_COMMA,          /*  ","           */
  T_MINUS,          /*  "-"           */
  T_PLUS,           /*  "+"           */
  T_TIMES,          /*  "*"           */
  T_PERCENT,        /*  "%"           */
  T_SLASH,          /*  "/"           */
  T_LPAREN,         /*  "("           */
  T_RPAREN,         /*  ")"           */
  T_LMUSTASHE,      /*  "{"           */
  T_RMUSTASHE,      /*  "}"           */
  T_LBRACKET,       /*  "["           */
  T_RBRACKET,       /*  "]"           */
  T_LCHEVRON,       /*  "<"           */
  T_RCHEVRON,       /*  ">"           */
  T_BANG,           /*  "!"           */
  T_COLON,          /*  ":"           */
  T_QUESTION,       /*  "?"           */
  T_EOS             /*  end of script */
};

struct token {
  enum token_type type;
  union {
    int    i;
    double f;
    char   s[MAX_NAME_LENGTH];
    char *sp;
    char   c;
  } value;
};

struct lexer {
  FILE *fptr;
  /* name of the source (eg. the file's name) */
  char *name;
  /* malloced/fread chars of the source */
  char *source;
  /* position at which to start fetching a token */
  char *curr_pos;
  unsigned line;
  unsigned col;
  struct {
    /* saved curr_pos */
    char *pos;
    unsigned line;
    unsigned col;
  } save;
  struct token curr_tok;
  struct {
    /* pointer to the malloced array of char pointers */
    char **ptr;
    /* pointer to the current `cell' in the above array */
    char **curr;
    /* size of the array */
    size_t size;
  } str_gc;
  struct {
    /* pointer to the malloced array of struct nodes */
    struct node *ptr;
    /* pointer to the current `cell' in the above array */
    struct node *curr;
    /* size of the pool */
    size_t size;
  } nds_pool; /* nodes pool */
};

struct token force(struct lexer *lexer_state, enum token_type type);
bool accept(struct lexer *lexer_state, enum token_type type);
bool peek(struct lexer *lexer_state, enum token_type type);

#endif /* LEXER_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

