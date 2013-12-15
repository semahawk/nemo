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
  TOK_INTEGER,        /*                */
  TOK_FLOAT,          /*                */
  TOK_STRING,         /*                */
  TOK_NAME,           /*                */
  TOK_KEYWORD,        /*                */
  TOK_TYPE,           /*                */
  TOK_OPT,            /*  -[a-zA-Z]     */
  TOK_EQ,             /*  "="           */
  TOK_SEMICOLON,      /*  ";"           */
  TOK_COMMA,          /*  ","           */
  TOK_MINUS,          /*  "-"           */
  TOK_PLUS,           /*  "+"           */
  TOK_TIMES,          /*  "*"           */
  TOK_PERCENT,        /*  "%"           */
  TOK_SLASH,          /*  "/"           */
  TOK_LPAREN,         /*  "("           */
  TOK_RPAREN,         /*  ")"           */
  TOK_LMUSTASHE,      /*  "{"           */
  TOK_RMUSTASHE,      /*  "}"           */
  TOK_LBRACKET,       /*  "["           */
  TOK_RBRACKET,       /*  "]"           */
  TOK_LCHEVRON,       /*  "<"           */
  TOK_RCHEVRON,       /*  ">"           */
  TOK_BANG,           /*  "!"           */
  TOK_COLON,          /*  ":"           */
  TOK_QUESTION,       /*  "?"           */
  TOK_EOS             /*  end of script */
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
bool accept_keyword(struct lexer *lexer_state, const char *name);
bool peek(struct lexer *lexer_state, enum token_type type);

#endif /* LEXER_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

