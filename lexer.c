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
#include "debug.h"
#include "nemo.h"
#include "nob.h"
#include "infnum.h"
#include "mem.h"
#include "lexer.h"
#include "util.h"

/* is `c' valid to begin a name with? */
#define name_beg(c) (isalpha(c) || c == '_' || (unsigned char)c >= 0xC0)
/* is `c' valid to be in a middle of a name? */
#define name_mid(c) (isalpha(c) || isdigit(c) || c == '_' || (unsigned char)c >= 0x80)

static const char *keywords[] =
{
  "if", "unless", "while", "until", "else",
  "my", "const",
  "typedef", "lim",
  "print",
  NULL
};

static void err(struct lexer *lex, const char *fmt, ...)
{
  /* {{{ */
  va_list vl;

  va_start(vl, fmt);
  fprintf(stderr, "%s:%u.%u: error: ", lex->name, lex->line, lex->col);
  vfprintf(stderr, fmt, vl);
  fprintf(stderr, "\n");
  va_end(vl);

  exit(1);
  /* }}} */
}

static void push_str(struct lexer *lex, char *str)
{
  /* {{{ */
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
  /* }}} */
}

static void advance(struct lexer *lex)
{
  /* {{{ */
  lex->save.line  = lex->line;
  lex->save.col   = lex->col;
  lex->save.pos   = lex->curr_pos;
  /* }}} */
}

static void fallback(struct lexer *lex)
{
  /* {{{ */
  lex->line       = lex->save.line;
  lex->col        = lex->save.col;
  lex->curr_pos   = lex->save.pos;
  /* }}} */
}

static struct token fetch_token(struct lexer *lex)
{
  /* {{{ fetch_token body */

/* a nifty shorthand for the current position */
#define p (lex->curr_pos)

  char tmp_arr[MAX_NAME_LENGTH + 1] = { '\0' };
  char *tmp_str = NULL;
  int i = 0;
  bool keyword_found = false, typename_found = false;
  int slen = 0;
  struct token ret;

  /* {{{ skip over whitespace and comments */
  do {
    if (isspace(*p)){
      for (; isspace(*p); p++){
        if (*p == '\n'){
          lex->line++;
          lex->col = 0;
        } else {
          lex->col++;
        }
      }
    } else if (*p == '/' && *(p + 1) == '*'){
      int nest_level = 1;

      p += 2;
      lex->col += 2;

      for (; *p != '\0' && nest_level > 0; p++){
        if (*p == '\n'){
          lex->line++;
          lex->col = 0;
        } else if (*p == '/' && *(p + 1) == '*'){
          nest_level++;
          p++; lex->col += 2;
        } else if (*p == '*' && *(p + 1) == '/'){
          nest_level--;
          p++; lex->col += 2;
        } else
          lex->col++;
      }
    }
  } while (isspace(*p) || (*p == '/' && *(p + 1) == '*'));
  /* }}} */

  if (p == NULL || *p == '\0' || (lex->fptr != NULL && feof(lex->fptr))){
    ret.type = TOK_EOS;
    return ret;
  }

  if (name_beg(*p)){
    /* {{{ NAME / KEYWORD / TYPE NAME */
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
        keyword_found = true;
        break;
      }
    }
    /* see if it's a type name (only if it's not a keyword already) */
    if (!keyword_found){
      unsigned i = 0;

      for (; i < NM_types_curr - NM_types; i++){
        /* don't check anonymous types */
        if (NM_types[i]->name != NULL){
          if (!strcmp(NM_types[i]->name, tmp_arr)){
            typename_found = true;
            break;
          }
        }
      }
    }

    if (keyword_found){
      ret.type = TOK_KEYWORD;
      strcpy(ret.value.s, *kptr);
      lex->col += strlen(*kptr);
    } else if (typename_found){
      ret.type = TOK_TYPE;
      strcpy(ret.value.s, tmp_arr);
      lex->col += strlen(tmp_arr);
    } else {
      ret.type = TOK_NAME;
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
        ret.type = TOK_FLOAT;
        ret.value.f = atof(tmp_arr);
      } else {
        /* it's something like 2. */
        tmp_arr[i] = '\0';
        ret.type = TOK_FLOAT;
        ret.value.f = atof(tmp_arr);
      }
      /* }}} */
    } else {
      /* {{{ DECIMAL */
      ret.type = TOK_INTEGER;
      ret.value.i = infnum_from_str(tmp_arr);
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

    tmp_str = nmalloc(/* sizeof(char) times */slen + 1);

    while (*savep != '"')
      *(tmp_str + i2++) = *savep++;

    i--;
    p++; /* jump to the next character so the next `fetch_token' doesn't start
            lexing at the closing '"' */

    push_str(lex, tmp_str);
    ret.type = TOK_STRING;
    ret.value.sp = tmp_str;
    /* }}} */
  }
  else switch (*p){
    /* {{{ SINGLE CHAR */
    case '=': ret.value.c = *p; lex->col++; p++; ret.type = TOK_EQ; break;
    case ':': ret.value.c = *p; lex->col++; p++; ret.type = TOK_COLON; break;
    case ';': ret.value.c = *p; lex->col++; p++; ret.type = TOK_SEMICOLON; break;
    case ',': ret.value.c = *p; lex->col++; p++; ret.type = TOK_COMMA; break;
    case '-': ret.value.c = *p; lex->col++; p++; ret.type = TOK_MINUS; break;
    case '+': ret.value.c = *p; lex->col++; p++; ret.type = TOK_PLUS; break;
    case '*': ret.value.c = *p; lex->col++; p++; ret.type = TOK_TIMES; break;
    case '%': ret.value.c = *p; lex->col++; p++; ret.type = TOK_PERCENT; break;
    case '/': ret.value.c = *p; lex->col++; p++; ret.type = TOK_SLASH; break;
    case '(': ret.value.c = *p; lex->col++; p++; ret.type = TOK_LPAREN; break;
    case ')': ret.value.c = *p; lex->col++; p++; ret.type = TOK_RPAREN; break;
    case '{': ret.value.c = *p; lex->col++; p++; ret.type = TOK_LMUSTASHE; break;
    case '}': ret.value.c = *p; lex->col++; p++; ret.type = TOK_RMUSTASHE; break;
    case '[': ret.value.c = *p; lex->col++; p++; ret.type = TOK_LBRACKET; break;
    case ']': ret.value.c = *p; lex->col++; p++; ret.type = TOK_RBRACKET; break;
    case '<': ret.value.c = *p; lex->col++; p++; ret.type = TOK_LCHEVRON; break;
    case '>': ret.value.c = *p; lex->col++; p++; ret.type = TOK_RCHEVRON; break;
    case '!': ret.value.c = *p; lex->col++; p++; ret.type = TOK_BANG; break;
    case '?': ret.value.c = *p; lex->col++; p++; ret.type = TOK_QUESTION; break;
    default:
      fprintf(stderr, "nemo: unknown character '%c' (0x%x) in %s"
          " at line %u column %u\n", *p, *p, lex->name, lex->line, lex->col);
      exit(1);
    /* }}} */
  }

#undef p

  return ret;
  /* }}} */
}

#if DEBUG
/* one nifty function to print a token (when -dl is on) */
static void debug_print_token(struct token tok)
{
  /* {{{ */
  switch (tok.type){
    case TOK_INTEGER:
      /* {{{ */
      fprintf(stderr, "integer %s", infnum_to_str(tok.value.i));
      /* }}} */
      break;
    case TOK_FLOAT:
      /* {{{ */
      fprintf(stderr, "float %f", tok.value.f);
      /* }}} */
      break;
    case TOK_STRING:
      /* {{{ */
      fprintf(stderr, "string \"%s\"", tok.value.sp);
      /* }}} */
      break;
    case TOK_TYPE:
      /* {{{ */
      fprintf(stderr, "type name \"%s\"", tok.value.s);
      /* }}} */
      break;
    case TOK_NAME:
      /* {{{ */
      fprintf(stderr, "name \"%s\"", tok.value.s);
      /* }}} */
      break;
    case TOK_KEYWORD:
      /* {{{ */
      fprintf(stderr, "keyword \"%s\"", tok.value.s);
      /* }}} */
      break;
    case TOK_EQ:
    case TOK_SEMICOLON:
    case TOK_COMMA:
    case TOK_MINUS:
    case TOK_PLUS:
    case TOK_TIMES:
    case TOK_PERCENT:
    case TOK_SLASH:
    case TOK_LPAREN:
    case TOK_RPAREN:
    case TOK_LMUSTASHE:
    case TOK_RMUSTASHE:
    case TOK_LBRACKET:
    case TOK_RBRACKET:
    case TOK_LCHEVRON:
    case TOK_RCHEVRON:
    case TOK_BANG:
    case TOK_COLON:
    case TOK_QUESTION:
      /* {{{ */
      fprintf(stderr, "'%c'", tok.value.c);
      /* }}} */
      break;
    case TOK_EOS:
      /* {{{ */
      fprintf(stderr, "<EOS>");
      /* }}} */
    default:
      fprintf(stderr, "unknown");
      break;
  }
  /* }}} */
}
#endif

void skip(struct lexer *lex)
{
  /* {{{ skip body */
  struct token tok = fetch_token(lex);

  if (tok.type == TOK_EOS){
    err(lex, "unexpected <EOS>");
  }

#if DEBUG
  /* {{{ */
  if (NM_DEBUG_GET_FLAG(NM_DEBUG_LEXER)){
    fprintf(stderr, "%s:%05u:%03u    skip  ", lex->name, lex->line, lex->col);
    debug_print_token(tok);
    fprintf(stderr, "\n");
  }
  /* }}} */
#endif

  advance(lex);
  /* }}} */
}

struct token force(struct parser *parser, struct lexer *lex, enum token_type type)
{
  /* {{{ force body */
  struct token tok = fetch_token(lex);

  if (tok.type == type){
#if DEBUG
    /* {{{ */
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_LEXER)){
      fprintf(stderr, "%s:%05u:%03u   force  ", lex->name, lex->line, lex->col);
      debug_print_token(tok);
      fprintf(stderr, "\n");
    }
    /* }}} */
#endif
    lex->curr_tok = tok;
    advance(lex);
    return tok;
  } else {
    if (tok.type == TOK_KEYWORD)
      fprintf(stderr, "%s:%u.%u: error: expected a %s instead of a keyword (%s)\n", lex->name, lex->save.line, lex->save.col, tok_to_s(type), tok.value.s);
    else if (tok.type == TOK_NAME)
      fprintf(stderr, "%s:%u.%u: error: expected a %s instead of a name (%s)\n", lex->name, lex->save.line, lex->save.col, tok_to_s(type), tok.value.s);
    else
      fprintf(stderr, "%s:%u.%u: error: expected a %s instead of a %s\n", lex->name, lex->save.line, lex->save.col, tok_to_s(type), tok_to_s(tok.type));

    parser->errorless = false;
    /* advance to the next token (sometimes there is an infinite loop that
     * fails to `force` a token) */
    skip(lex);
    /* shut up, errors */
    return tok;
  }
  /* }}} */
}

bool force_keyword(struct parser *parser, struct lexer *lex, const char *name)
{
  /* {{{ force_keyword body */
  if (peek(lex, TOK_KEYWORD)){
    if (!strcmp(name, lex->curr_tok.value.s)){
      skip(lex);
      return true;
    }
  }

  parser->errorless = false;

  return false;
  /* }}} */
}

bool accept(struct lexer *lex, enum token_type type)
{
  /* {{{ accept body */
  struct token tok = fetch_token(lex);

  if (tok.type == type){
#if DEBUG
    /* {{{ */
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_LEXER)){
      fprintf(stderr, "%s:%05u:%03u  accept  ", lex->name, lex->line, lex->col);
      debug_print_token(tok);
      fprintf(stderr, "\n");
    }
    /* }}} */
#endif
    lex->curr_tok = tok;
    advance(lex);
    return true;
  } else {
    fallback(lex);
    return false;
  }
  /* }}} */
}

bool accept_keyword(struct lexer *lex, const char *name)
{
  /* {{{ accept_keyword body */
  if (peek(lex, TOK_KEYWORD)){
    if (!strcmp(name, lex->curr_tok.value.s)){
      skip(lex);
      return true;
    }
  }

  return false;
  /* }}} */
}

bool peek(struct lexer *lex, enum token_type type)
{
  /* {{{ peek_body */
  struct token tok = fetch_token(lex);

  fallback(lex);

  if (tok.type == type){
#if DEBUG
    /* {{{ */
    if (NM_DEBUG_GET_FLAG(NM_DEBUG_LEXER)){
      fprintf(stderr, "%s:%05u:%03u    peek  ", lex->name, lex->line, lex->col);
      debug_print_token(tok);
      fprintf(stderr, "\n");
    }
    /* }}} */
#endif
    lex->curr_tok = tok;
    return true;
  } else {
    return false;
  }
  /* }}} */
}

const char *tok_to_s(enum token_type type)
{
  /* {{{ */
  switch (type){
    case TOK_INTEGER:   return "integer";
    case TOK_FLOAT:     return "float";
    case TOK_STRING:    return "string";
    case TOK_NAME:      return "name";
    case TOK_KEYWORD:   return "keyword";
    case TOK_TYPE:      return "type name";
    case TOK_OPT:       return "option";
    case TOK_EQ:        return "'='";
    case TOK_SEMICOLON: return "';'";
    case TOK_COMMA:     return "','";
    case TOK_MINUS:     return "'-'";
    case TOK_PLUS:      return "'+'";
    case TOK_TIMES:     return "'*'";
    case TOK_PERCENT:   return "'%'";
    case TOK_SLASH:     return "'/'";
    case TOK_LPAREN:    return "'('";
    case TOK_RPAREN:    return "')'";
    case TOK_LMUSTASHE: return "'{'";
    case TOK_RMUSTASHE: return "'}'";
    case TOK_LBRACKET:  return "'['";
    case TOK_RBRACKET:  return "']'";
    case TOK_LCHEVRON:  return "'<'";
    case TOK_RCHEVRON:  return "'>'";
    case TOK_BANG:      return "'!'";
    case TOK_COLON:     return "':'";
    case TOK_QUESTION:  return "'?'";
    case TOK_EOS:       return "<EOS>";
    default:            return "##unknown##tok_to_s##";
  }
  /* }}} */
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

