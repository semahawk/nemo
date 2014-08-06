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
#include "utf8.h"

/* is `c' valid to begin a name with? */
#define name_beg(c) (isalpha(c) || c == '_' || (unsigned char)c >= 0xC0)
/* is `c' valid to be in a middle of a name? */
#define name_mid(c) (isalpha(c) || isdigit(c) || c == '_' || (unsigned char)c >= 0x80)

static const char *keywords[] =
{
  "if", "unless", "while", "until", "else",
  "my", "mutable",
  "typedef", "lim",
  "print",
  NULL
};

static void err(struct parser *parser, struct lexer *lex, const char *fmt, ...)
{
  /* {{{ */
  va_list vl;

  va_start(vl, fmt);
  fprintf(stderr, "%s:%u.%u: error: ", lex->name, lex->line, lex->col);
  vfprintf(stderr, fmt, vl);
  fprintf(stderr, "\n");
  va_end(vl);

  parser->errorless = false;
  /* the skip makes the error messages not repeat themselves */
  skip(parser, lex);
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

static struct token fetch_token(struct parser *parser, struct lexer *lex)
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
      struct types_list *q;

      for (q = NM_types; q != NULL; q = q->next){
        /* don't check anonymous types */
        if (q->type->name != NULL){
          if (!strcmp(q->type->name, tmp_arr)){
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
        ret.type = TOK_REAL;
        ret.value.f = strtod(tmp_arr, NULL);
      } else {
        /* it's something like 2. */
        tmp_arr[i] = '\0';
        ret.type = TOK_REAL;
        ret.value.f = strtod(tmp_arr, NULL);
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
  else if (*p == '\''){
    /* {{{ CHAR */
    nchar_t value;

    /* skip over the opening "'" */
    p++;
    /* fetch the (possibly multibyte) character */
    value = u8_fetch_char(&p);

    if (*p != '\'')
      err(parser, lex, "unterminated character");

    /* skip over the closing "'" */
    p++;

    ret.type = TOK_CHAR;
    ret.value.c = value;
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
      err(parser, lex, "unterminated string");
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
  else if (*p == '%'){
    if (isdigit(*(p + 1))){
      /* {{{ ACCUMULATOR */
      unsigned i = 0;
      p++;

      while (isdigit(*p) && i < MAX_NAME_LENGTH)
        tmp_arr[i++] = *p++;

      lex->col += i;
      strcpy(ret.value.s, tmp_arr);
      ret.type = TOK_ACCUMULATOR;
      /* }}} */
    } else {
      /* {{{ PERCENT SIGN */
      if (*(p + 1) == '='){
        /* %= */
        lex->col += 2;
        p += 2;
        ret.type = TOK_PERCENT_EQ;
      } else {
        /* % */
        lex->col++;
        p++;
        ret.type = TOK_PERCENT;
      }
      /* }}} */
    }
  }
  else switch (*p){
    /* {{{ OPERATOR */
#define single(TYPE) {                       \
      lex->col++;                            \
      p++;                                   \
      ret.type = TOK_##TYPE;                 \
    }                                        \
    break /* no semicolon */

#define possibly_double(TYPE)                \
    if (*(p + 1) == *p){                     \
      lex->col += 2;                         \
      p += 2;                                \
      ret.type = TOK_##TYPE##_2;             \
    } else                                   \
      single(TYPE);
    /* `single` already handles the `break` */

#define possibly_eq(TYPE)                    \
    if (*(p + 1) == '='){                    \
      lex->col += 2;                         \
      p += 2;                                \
      ret.type = TOK_##TYPE##_EQ;            \
    } else                                   \
      single(TYPE);
    /* `single` already handles the `break` */

#define possibly_double_or_eq(TYPE)          \
    if (*(p + 1) == *p){                     \
      lex->col += 2;                         \
      p += 2;                                \
      ret.type = TOK_##TYPE##_2;             \
    } else if (*(p + 1) == '='){             \
      lex->col += 2;                         \
      p += 2;                                \
      ret.type = TOK_##TYPE##_EQ;            \
    } else                                   \
      single(TYPE);
    /* `single` already handles the `break` */

#define possibly_double_or_eq_or_both(TYPE)  \
    if (*(p + 1) == *p){                     \
      if (*(p + 2) == '='){                  \
        /* double and eq */                  \
        lex->col += 3;                       \
        p += 3;                              \
        ret.type = TOK_##TYPE##_2_EQ;        \
      } else {                               \
        /* double */                         \
        lex->col += 2;                       \
        p += 2;                              \
        ret.type = TOK_##TYPE##_2;           \
      }                                      \
    } else if (*(p + 1) == '='){             \
      /* single and eq */                    \
      lex->col += 2;                         \
      p += 2;                                \
      ret.type = TOK_##TYPE##_EQ;            \
    } else                                   \
      single(TYPE);
      /* `single` already handles the `break` */

    case '=': possibly_double(EQ);
    case ':': single(COLON);
    case ';': single(SEMICOLON);
    case ',': single(COMMA);
    case '-': possibly_double_or_eq(MINUS);
    case '+': possibly_double_or_eq(PLUS);
    case '*': possibly_eq(TIMES);
    case '/': possibly_eq(SLASH);
    case '(': single(LPAREN);
    case ')': single(RPAREN);
    case '{': single(LMUSTASHE);
    case '}': single(RMUSTASHE);
    case '[': single(LBRACKET);
    case ']': single(RBRACKET);
    case '!': possibly_eq(BANG);
    case '?': single(QUESTION);
    case '&': possibly_double_or_eq(AMPERSAND);
    case '^': possibly_eq(CARET);
    case '|': possibly_double_or_eq(PIPE);
    case '<': possibly_double_or_eq_or_both(LCHEVRON);
    case '>': possibly_double_or_eq_or_both(RCHEVRON);
    /* no case for the percent sign - it was already covered by the
     * 'accumulator' thingy */

    default:
      fprintf(stderr, "nemo: unknown character '%c' (0x%x) in %s"
          " at line %u column %u\n", *p, *p, lex->name, lex->line, lex->col);
      /* FIXME don't exit here */
      exit(1);
    /* }}} */
  }

#undef p
#undef single
#undef possibly_double
#undef possibly_eq
#undef possibly_double_or_eq
#undef possibly_double_or_eq_or_both

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
      fprintf(stderr, "integer ");
      infnum_print(tok.value.i, stderr);
      /* }}} */
      break;
    case TOK_REAL:
      /* {{{ */
      fprintf(stderr, "real %f", tok.value.f);
      /* }}} */
      break;
    case TOK_STRING:
      /* {{{ */
      fprintf(stderr, "string \"%s\"", tok.value.sp);
      /* }}} */
      break;
    case TOK_CHAR:
      /* {{{ */
      fprintf(stderr, "char '%c'", tok.value.c);
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
    case TOK_ACCUMULATOR:
      /* {{{ */
      fprintf(stderr, "accumulator \"%%%s\"", tok.value.s);
      /* }}} */
      break;
    case TOK_KEYWORD:
      /* {{{ */
      fprintf(stderr, "keyword \"%s\"", tok.value.s);
      /* }}} */
      break;
    case TOK_OPT:
      /* {{{ */
      fprintf(stderr, "option \"%s\"", tok.value.s);
      /* }}} */
      break;
    /* fall through */
    case TOK_EQ:
    case TOK_EQ_2:
    case TOK_SEMICOLON:
    case TOK_COMMA:
    case TOK_MINUS:
    case TOK_MINUS_2:
    case TOK_MINUS_EQ:
    case TOK_PLUS:
    case TOK_PLUS_2:
    case TOK_PLUS_EQ:
    case TOK_TIMES:
    case TOK_TIMES_EQ:
    case TOK_PERCENT:
    case TOK_PERCENT_EQ:
    case TOK_SLASH:
    case TOK_SLASH_EQ:
    case TOK_LPAREN:
    case TOK_RPAREN:
    case TOK_LMUSTASHE:
    case TOK_RMUSTASHE:
    case TOK_LBRACKET:
    case TOK_RBRACKET:
    case TOK_LCHEVRON:
    case TOK_LCHEVRON_EQ:
    case TOK_LCHEVRON_2:
    case TOK_LCHEVRON_2_EQ:
    case TOK_RCHEVRON:
    case TOK_RCHEVRON_EQ:
    case TOK_RCHEVRON_2:
    case TOK_RCHEVRON_2_EQ:
    case TOK_BANG:
    case TOK_COLON:
    case TOK_QUESTION:
    case TOK_AMPERSAND:
    case TOK_AMPERSAND_2:
    case TOK_AMPERSAND_EQ:
    case TOK_CARET:
    case TOK_CARET_EQ:
    case TOK_PIPE:
    case TOK_PIPE_2:
    case TOK_PIPE_EQ:
      /* {{{ */
      fprintf(stderr, "%s", tok_to_s(tok.type));
      /* }}} */
      break;
    case TOK_EOS:
      /* {{{ */
      fprintf(stderr, "<EOS>");
      /* }}} */
      break;
    default:
      fprintf(stderr, "##unknown##debug_print_token##");
      break;
  }
  /* }}} */
}
#endif

void skip(struct parser *parser, struct lexer *lex)
{
  /* {{{ skip body */
  struct token tok = fetch_token(parser, lex);

  if (tok.type == TOK_EOS){
    err(parser, lex, "unexpected <EOS>");
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
  struct token tok = fetch_token(parser, lex);

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
    skip(parser, lex);
    /* shut up, errors */
    return tok;
  }
  /* }}} */
}

bool force_keyword(struct parser *parser, struct lexer *lex, const char *name)
{
  /* {{{ force_keyword body */
  if (peek(parser, lex, TOK_KEYWORD)){
    if (!strcmp(name, lex->curr_tok.value.s)){
      skip(parser, lex);
      return true;
    }
  }

  parser->errorless = false;

  return false;
  /* }}} */
}

bool accept(struct parser *parser, struct lexer *lex, enum token_type type)
{
  /* {{{ accept body */
  struct token tok = fetch_token(parser, lex);

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

bool accept_keyword(struct parser *parser, struct lexer *lex, const char *name)
{
  /* {{{ accept_keyword body */
  if (peek(parser, lex, TOK_KEYWORD)){
    if (!strcmp(name, lex->curr_tok.value.s)){
      skip(parser, lex);
      return true;
    }
  }

  return false;
  /* }}} */
}

bool peek(struct parser *parser, struct lexer *lex, enum token_type type)
{
  /* {{{ peek_body */
  struct token tok = fetch_token(parser, lex);

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
    case TOK_INTEGER:       return "integer";
    case TOK_REAL:          return "real";
    case TOK_STRING:        return "string";
    case TOK_CHAR:          return "character";
    case TOK_NAME:          return "name";
    case TOK_ACCUMULATOR:   return "accumulator";
    case TOK_KEYWORD:       return "keyword";
    case TOK_TYPE:          return "type name";
    case TOK_OPT:           return "option";
    case TOK_EQ:            return "'='";
    case TOK_EQ_2:          return "'=='";
    case TOK_SEMICOLON:     return "';'";
    case TOK_COMMA:         return "','";
    case TOK_MINUS:         return "'--'";
    case TOK_MINUS_2:       return "'--'";
    case TOK_MINUS_EQ:      return "'-='";
    case TOK_PLUS:          return "'+'";
    case TOK_PLUS_2:        return "'++'";
    case TOK_PLUS_EQ:       return "'+='";
    case TOK_TIMES:         return "'*'";
    case TOK_TIMES_EQ:      return "'*='";
    case TOK_PERCENT:       return "'%'";
    case TOK_PERCENT_EQ:    return "'%='";
    case TOK_SLASH:         return "'/'";
    case TOK_SLASH_EQ:      return "'/='";
    case TOK_LPAREN:        return "'('";
    case TOK_RPAREN:        return "')'";
    case TOK_LMUSTASHE:     return "'{'";
    case TOK_RMUSTASHE:     return "'}'";
    case TOK_LBRACKET:      return "'['";
    case TOK_RBRACKET:      return "']'";
    case TOK_LCHEVRON:      return "'<'";
    case TOK_LCHEVRON_EQ:   return "'<='";
    case TOK_LCHEVRON_2:    return "'<<'";
    case TOK_LCHEVRON_2_EQ: return "'<<='";
    case TOK_RCHEVRON:      return "'>'";
    case TOK_RCHEVRON_EQ:   return "'>='";
    case TOK_RCHEVRON_2:    return "'>>'";
    case TOK_RCHEVRON_2_EQ: return "'>>='";
    case TOK_BANG:          return "'!'";
    case TOK_COLON:         return "':'";
    case TOK_QUESTION:      return "'?'";
    case TOK_AMPERSAND:     return "'&'";
    case TOK_AMPERSAND_2:   return "'&&'";
    case TOK_AMPERSAND_EQ:  return "'&='";
    case TOK_CARET:         return "'^'";
    case TOK_CARET_EQ:      return "'^='";
    case TOK_PIPE:          return "'|'";
    case TOK_PIPE_2:        return "'||'";
    case TOK_PIPE_EQ:       return "'|='";
    case TOK_EOS:           return "<EOS>";
    default:                return "##unknown##tok_to_s##";
  }
  /* }}} */
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

