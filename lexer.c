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
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "config.h"
#include "nemo.h"
#include "mem.h"
#include "lexer.h"
#include "util.h"

/* is `c' valid to begin a name with? */
#define name_beg(c) (isalpha(c) || c == '_' || (unsigned char)c >= 0xC0)
/* is `c' valid to be in a middle of a name? */
#define name_mid(c) (isalpha(c) || isdigit(c) || c == '_' || (unsigned char)c >= 0x80)

static const char *keywords[] =
{
  "my", NULL
};

static void err(struct lexer *lex, const char *fmt, ...)
{
  va_list vl;

  va_start(vl, fmt);
  fprintf(stderr, "%s: ", lex->name);
  vfprintf(stderr, fmt, vl);
  fprintf(stderr, " at line %u column %u\n", lex->line, lex->col);
  va_end(vl);

  exit(1);
}

static void push_str(struct lexer *lex, char *str)
{
  ptrdiff_t offset = lex->str_gc.curr - lex->str_gc.ptr;

  /* handle overflow */
  if (offset >= (signed)lex->str_gc.size){
    /* grow the stack to be twice as big as it was */
    lex->str_gc.size <<= 1;
    lex->str_gc.ptr = nrealloc(lex->str_gc.ptr, sizeof(char *) * lex->str_gc.size);
    lex->str_gc.curr = lex->str_gc.ptr + offset;
  }

  *lex->str_gc.curr = str; /* set up the current 'cell' */
  lex->str_gc.curr++; /* move on to the next 'cell' */
}

static void advance(struct lexer *lex)
{
  lex->save.line  = lex->line;
  lex->save.col   = lex->col;
  lex->save.pos   = lex->curr_pos;
}

static void fallback(struct lexer *lex)
{
  lex->line       = lex->save.line;
  lex->col        = lex->save.col;
  lex->curr_pos   = lex->save.pos;
}

static struct token fetch_token(struct lexer *lex)
{
  /* {{{ fetch_token body */

/* a nifty shorthand for the current position */
#define p (lex->curr_pos)

  char tmp_arr[MAX_NAME_LENGTH + 1] = { '\0' };
  char *tmp_str = NULL;
  int i = 0, keyword_found = 0;
  int slen = 0;
  struct token ret;

  if (p == NULL || *p == '\0' || (lex->fptr != NULL && feof(lex->fptr))){
    ret.type = T_EOS;
    return ret;
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

  if (name_beg(*p)){
    /* {{{ NAME */
    const char **kptr = NULL;
    strncpy(tmp_arr, p, MAX_NAME_LENGTH - 1);
    tmp_arr[MAX_NAME_LENGTH] = '\0';

    while (name_mid(*p)){
      p++; i++;
    }

    tmp_arr[i] = '\0';
    /* see if it's a keyword */
    for (kptr = keywords; *kptr != NULL; kptr++){
      if (!strcmp(*kptr, tmp_arr)){
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
      strcpy(ret.value.s, tmp_arr);
      lex->col += strlen(tmp_arr);
    }
    /* }}} */
  }
  else if (isdigit(*p)){
    /* {{{ NUMBER */
    int i2 = 0;

    while (isdigit(*p) || (*p == '_' && isdigit(*(p + 1)))){
      if (isdigit(*p)){
        tmp_arr[i2++] = *p;
      }
      p++; i++;
    }

    if (*p == '.'){
      /* {{{ FLOAT */
      tmp_arr[i2++] = '.';
      p++; i++; /* skip over the '.' */
      if (isdigit(*p)){
        while (isdigit(*p) || (*p == '_' && isdigit(*(p + 1)))){
          if (isdigit(*p)){
            tmp_arr[i2++] = *p;
          }
          p++; i++;
        }
        tmp_arr[i2] = '\0';
        ret.type = T_FLOAT;
        ret.value.f = atof(tmp_arr);
      } else {
        /* it's something like 2. */
        tmp_arr[i] = '\0';
        ret.type = T_FLOAT;
        ret.value.f = atof(tmp_arr);
      }
      /* }}} */
    } else {
      /* {{{ DECIMAL */
      ret.type = T_INTEGER;
      ret.value.i = atoi(tmp_arr);
      /* }}} */
    }

    lex->col += i;
    /* }}} */
  }
  else if (*p == '"'){
    /* {{{ STRING */
    char *savep;
    int i2 = 0;
    p++; i++; lex->col++; savep = p; /* skip over the opening '"' */

    while (*p != '"' && *p != '\n'){
      p++; i++; lex->col++; slen++;
    }

    if (*p == '\n'){
      err(lex, "unterminated string");
    }

    tmp_str = nmalloc(/* sizeof(char) times */slen);

    while (*savep != '"')
      *(tmp_str + i2++) = *savep++;

    i--;
    p++; /* jump to the next character so the next `fetch_token' doesn't start
            lexing at the closing '"' */

    push_str(lex, tmp_str);
    ret.type = T_STRING;
    ret.value.sp = tmp_str;
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

struct token force(struct lexer *lex, enum token_type type)
{
  struct token tok = fetch_token(lex);

  if (tok.type == type){
    lex->curr_tok = tok;
    advance(lex);
    return tok;
  } else {
    fprintf(stderr, "expected %d instead of %d\n", type, tok.type);
    exit(1);
  }
}

bool accept(struct lexer *lex, enum token_type type)
{
  struct token tok = fetch_token(lex);

  if (tok.type == type){
    lex->curr_tok = tok;
    advance(lex);
    return true;
  } else {
    fallback(lex);
    return false;
  }
}

bool peek(struct lexer *lex, enum token_type type)
{
  struct token tok = fetch_token(lex);

  fallback(lex);

  if (tok.type == type){
    lex->curr_tok = tok;
    return true;
  } else {
    return false;
  }
}

void skip(struct lexer *lex)
{
  struct token tok = fetch_token(lex);

  if (tok.type == T_EOS){
    err(lex, "unexpected <EOF>");
  }

  advance(lex);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

