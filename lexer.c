/*
 *
 * lexer.c
 *
 * Created at:  Thu Nov  7 18:38:54 2013 18:38:54
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License: please visit the LICENSE file for details.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>

#include "config.h"
#include "nemo.h"
#include "lexer.h"
#include "util.h"

static const char *keywords[] =
{
  "my", NULL
};

static void err(struct lexer_t *lex, const char *fmt, ...)
{
  va_list vl;

  va_start(vl, fmt);
  fprintf(stderr, "%s: ", lex->name);
  vfprintf(stderr, fmt, vl);
  fprintf(stderr, " at line %u column %u\n", lex->line, lex->col);
  va_end(vl);
}

static void advance(struct lexer_t *lex)
{
  lex->save.line  = lex->line;
  lex->save.col   = lex->col;
  lex->save.pos   = lex->curr_pos;
}

static void fallback(struct lexer_t *lex)
{
  lex->line       = lex->save.line;
  lex->col        = lex->save.col;
  lex->curr_pos   = lex->save.pos;
}

struct token_t fetch_token(struct lexer_t *lex)
{
  /* {{{ fetch_token body */

/* a nifty shorthand for the current position */
#define p (lex->curr_pos)

  char tmp[MAX_NAME_LENGTH] = { '\0' };
  int i = 0, keyword_found = 0;
  struct token_t ret;

  if (p == NULL || *p == '\0' || feof(lex->fptr)){
    ret.type = T_EOS;
    return ret;
  }

  /* return the current token if it's still valid */
  if (lex->valid_curr){
    return lex->curr_tok;
  }

  /* skip over the whitespace */
  for (; isspace(*p); p++){
    if (*p == '\n'){
      lex->line++;
      lex->col = 0;
    } else {
      lex->col++;
    }
  }

  if (isalpha(*p) || *p == '_'){
    /* {{{ NAME */
    const char **kptr = NULL;
    strncpy(tmp, p, MAX_NAME_LENGTH - 1);
    tmp[MAX_NAME_LENGTH] = '\0';

    while (isalpha(*p) || isdigit(*p) || *p == '_'){
      p++; i++;
    }
    tmp[i] = '\0';
    /* see if it's a keyword */
    for (kptr = keywords; *kptr != NULL; kptr++){
      if (!strcmp(*kptr, tmp)){
        keyword_found = 1;
        break;
      }
    }

    if (keyword_found){
      ret.type = T_KEYWORD;
      strcpy(ret.value.s, *kptr);
      lex->col += strlen(*kptr);
    } else {
      ret.type = T_NAME;
      strcpy(ret.value.s, tmp);
      lex->col += strlen(tmp);
    }
    /* }}} */
  }
  else if (isdigit(*p)){
    /* {{{ NUMBER */
    int i2 = 0;

    while (isdigit(*p) || (*p == '_' && isdigit(*(p + 1)))){
      if (isdigit(*p)){
        tmp[i2++] = *p;
      }
      p++; i++;
    }

    if (*p == '.'){
      /* {{{ FLOAT */
      tmp[i2++] = '.';
      p++; i++; /* skip over the '.' */
      if (isdigit(*p)){
        while (isdigit(*p) || (*p == '_' && isdigit(*(p + 1)))){
          if (isdigit(*p)){
            tmp[i2++] = *p;
          }
          p++; i++;
        }
        tmp[i2] = '\0';
        ret.type = T_FLOAT;
        ret.value.f = atof(tmp);
      } else {
        /* it's something like 2. */
        tmp[i] = '\0';
        ret.type = T_FLOAT;
        ret.value.f = atof(tmp);
      }
      /* }}} */
    } else {
      /* {{{ DECIMAL */
      ret.type = T_INTEGER;
      ret.value.i = atoi(tmp);
      /* }}} */
    }

    lex->col += i;
    /* }}} */
  }
  else switch (*p){
    /* {{{ SINGLE CHAR */
    case '=': p++; ret.type = T_EQ; break;
    case ':': p++; ret.type = T_COLON; break;
    case ';': p++; ret.type = T_SEMICOLON; break;
    case ',': p++; ret.type = T_COMMA; break;
    case '-': p++; ret.type = T_MINUS; break;
    case '+': p++; ret.type = T_PLUS; break;
    case '*': p++; ret.type = T_TIMES; break;
    case '%': p++; ret.type = T_PERCENT; break;
    case '/': p++; ret.type = T_SLASH; break;
    case '(': p++; ret.type = T_LPAREN; break;
    case ')': p++; ret.type = T_RPAREN; break;
    case '{': p++; ret.type = T_LMUSTASHE; break;
    case '}': p++; ret.type = T_RMUSTASHE; break;
    case '[': p++; ret.type = T_LBRACKET; break;
    case ']': p++; ret.type = T_RBRACKET; break;
    case '<': p++; ret.type = T_LCHEVRON; break;
    case '>': p++; ret.type = T_RCHEVRON; break;
    case '!': p++; ret.type = T_BANG; break;
    case '?': p++; ret.type = T_QUESTION; break;
    default:
      fprintf(stderr, "nemo: unknown character '%c' (%p) in %s at line %u column %u\n", *p, (void *)p, lex->name, lex->line, lex->col);
      exit(1);
    /* }}} */
  }

#undef p

  return ret;
  /* }}} */
}

struct token_t force(struct lexer_t *lex, enum token_type_t type)
{
  struct token_t tok = fetch_token(lex);

  if (tok.type == type){
    lex->curr_tok = tok;
    lex->valid_curr = false;
    advance(lex);
    return tok;
  } else {
    fprintf(stderr, "expected %d instead of %d\n", type, tok.type);
    exit(1);
  }
}

bool accept(struct lexer_t *lex, enum token_type_t type)
{
  struct token_t tok = fetch_token(lex);

  if (tok.type == type){
    lex->curr_tok = tok;
    lex->valid_curr = true;    /* <  I guess these two should be swapped    */
    advance(lex);              /*    but heck why is this working..?        */
    return true;               /*                                           */
  } else {                     /*    I thought that.. erm... it shouldn't.. */
    lex->valid_curr = false;   /* <  But it is working (somewhy)...         */
    fallback(lex);
    return false;
  }
}

bool peek(struct lexer_t *lex, enum token_type_t type)
{
  struct token_t tok = fetch_token(lex);

  fallback(lex);

  if (tok.type == type){
    lex->curr_tok = tok;
    lex->valid_curr = false;
    return true;
  } else {
    lex->valid_curr = true;
    return false;
  }
}

void skip(struct lexer_t *lex)
{
  struct token_t tok = fetch_token(lex);

  if (tok.type == T_EOS){
    err(lex, "unexpected <EOF>");
    exit(1);
  }

  advance(lex);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

