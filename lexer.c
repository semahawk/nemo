/*
 * lexer.c
 *
 * Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
 *
 */

#include "nemo.h"
#include "parser.h"
#include "lexer.h"
#include "handy.h"

static struct Symbol **symbols;
static unsigned int count_symbols = 0;

void append(SymbolType type)
{
  struct Symbol *new = myalloc(sizeof(struct Symbol));

  new->line = 0;
  new->column = 0;
  new->type = type;

  count_symbols++;
  symbols = realloc(symbols, count_symbols * sizeof(struct Symbol));
  symbols[count_symbols - 1] = new;
}

void appendInt(SymbolType type, int value)
{
  struct Symbol *new = myalloc(sizeof(struct Symbol));

  new->line = 0;
  new->column = 0;
  new->type = type;
  new->value.i = value;

  count_symbols++;
  symbols = realloc(symbols, count_symbols * sizeof(struct Symbol));
  symbols[count_symbols - 1] = new;
}

void appendFloat(SymbolType type, float value)
{
  struct Symbol *new = myalloc(sizeof(struct Symbol));

  new->line = 0;
  new->column = 0;
  new->type = type;
  new->value.f = value;

  count_symbols++;
  symbols = realloc(symbols, count_symbols * sizeof(struct Symbol));
  symbols[count_symbols - 1] = new;
}

void appendStr(SymbolType type, char *value)
{
  struct Symbol *new = myalloc(sizeof(struct Symbol));

  new->line = 0;
  new->column = 0;
  new->type = type;
  new->value.s = strdup(value);

  count_symbols++;
  symbols = realloc(symbols, count_symbols * sizeof(struct Symbol));
  symbols[count_symbols - 1] = new;
}

struct Symbol **lexFile(const char *filename)
{
  const char *keywords[] = {
    "int", "float", "none", "while", "if", "for", "else", "use", "return"
  };

  SymbolType keyword_syms[] = {
    SYM_TYPE_INTEGER, SYM_TYPE_FLOATING, SYM_NONE, SYM_WHILE, SYM_IF, SYM_FOR, SYM_ELSE, SYM_USE, SYM_RETURN
  };

  FILE *fp;
  int size = fsize(filename);
  char *code = myalloc(sizeof(char) * size);
  symbols = myalloc(sizeof(struct Symbol));

  if ((fp = fopen(filename, "r")) == NULL){
    fprintf(stderr, "nemo: error: ");
    perror(filename);
    exit(1);
  }

  fread(code, 1, size, fp);

  int i = 0;
  char *tmp;
  for (char *p = code; *p != 0; p++, i = 0){
    /*putchar(*p);*/
    //
    // XXX ;
    //
    if (*p == ';'){
      append(SYM_SEMICOLON);
      debug("appending symbol: %s", symtos(SYM_SEMICOLON));
    //
    // XXX :
    //
    } else if (*p == ':'){
      append(SYM_COLON);
      debug("appending symbol: %s", symtos(SYM_COLON));
    //
    // XXX ,
    //
    } else if (*p == ','){
      append(SYM_COMMA);
      debug("appending symbol: %s", symtos(SYM_COMMA));
    //
    // XXX \n
    //
    } else if (*p == '\n'){
      //append(SYM_NEWLINE);
      //debug("appending symbol: %s", symtos(SYM_NEWLINE));
    //
    // XXX +
    //
    } else if (*p == '+'){
      //
      // XXX +
      //
      if (*(p + 1) == '+'){
        append(SYM_PLUSPLUS);
        debug("appending symbol: %s", symtos(SYM_PLUSPLUS));
      } else {
        append(SYM_PLUS);
        debug("appending symbol: %s", symtos(SYM_PLUS));
      }
    //
    // XXX -
    //
    } else if (*p == '-'){
      append(SYM_MINUS);
      debug("appending symbol: %s", symtos(SYM_MINUS));
    //
    // XXX *
    //
    } else if (*p == '*'){
      append(SYM_TIMES);
      debug("appending symbol: %s", symtos(SYM_TIMES));
    //
    // XXX /
    //
    } else if (*p == '/'){
      //
      // XXX // comment
      //
      if (*(p + 1) == '/'){
        p++;
        // eat up the whole line
        while (*p != '\n')
          p++;
        //append(SYM_NEWLINE);
      } else {
        append(SYM_SLASH);
        debug("appending symbol: %s", symtos(SYM_SLASH));
      }
    //
    // XXX %
    //
    } else if (*p == '%'){
      append(SYM_MODULO);
      debug("appending symbol: %s", symtos(SYM_MODULO));
    //
    // XXX =
    //
    } else if (*p == '='){
      append(SYM_EQ);
      debug("appending symbol: %s", symtos(SYM_EQ));
    //
    // XXX ?
    //
    } else if (*p == '?'){
      append(SYM_QMARK);
      debug("appending symbol: %s", symtos(SYM_QMARK));
    //
    // XXX !
    //
    } else if (*p == '!'){
      append(SYM_EMARK);
      debug("appending symbol: %s", symtos(SYM_EMARK));
    //
    // XXX <
    //
    } else if (*p == '<'){
      append(SYM_LT);
      debug("appending symbol: %s", symtos(SYM_LT));
    //
    // XXX >
    //
    } else if (*p == '>'){
      append(SYM_GT);
      debug("appending symbol: %s", symtos(SYM_GT));
    //
    // XXX (
    //
    } else if (*p == '('){
      append(SYM_LPAREN);
      debug("appending symbol: %s", symtos(SYM_LPAREN));
    //
    // XXX )
    //
    } else if (*p == ')'){
      append(SYM_RPAREN);
      debug("appending symbol: %s", symtos(SYM_RPAREN));
    //
    // XXX {
    //
    } else if (*p == '{'){
      append(SYM_LMUSTASHE);
      debug("appending symbol: %s", symtos(SYM_LMUSTASHE));
    //
    // XXX }
    //
    } else if (*p == '}'){
      append(SYM_RMUSTASHE);
      debug("appending symbol: %s", symtos(SYM_RMUSTASHE));
    //
    // XXX $var
    //
    } else if (*p == '$'){
      tmp = strdup(p);
      i++;
      p++;
      while (isalpha(*p) ||
             *p == '_'   ||
             *p == '!'   ||
             *p == '@'   ||
             *p == '#'   ||
             *p == '%'   ||
             *p == '^'   ||
             *p == '&'   ||
             *p == '*'   ||
             *p == '+'   ||
             *p == '|'){
        p++; i++;
      }
      p--;
      *(tmp + i) = '\0';
      i--;
      appendStr(SYM_VAR_IDENT, tmp);
      debug("appending symbol: %s (%s)", symtos(SYM_VAR_IDENT), tmp);
      free(tmp);
    //
    // XXX name
    //
    } else if (isalpha(*p)){
      tmp = strdup(p);
      p++;
      i++;
      while (isalpha(*p) || *p == '_'){
        p++; i++;
      }
      p--;
      *(tmp + i) = '\0';
      i--;

      bool found = false;
      for (unsigned short key = 0; key < ARRAY_SIZE(keywords); key++){
        if (!strcmp(keywords[key], tmp)){
          append(keyword_syms[key]);
          debug("appending symbol: %s", symtos(keyword_syms[key]));
          found = true;
        }
      }

      if (!found){
        appendStr(SYM_IDENT, tmp);
        debug("appending symbol: %s", symtos(SYM_IDENT));
      }

      free(tmp);
    //
    // XXX 123
    //
    } else if (isdigit(*p)){
      tmp = strdup(p);
      p++;
      i++;
      while (isdigit(*p)){
        p++; i++;
      }
      //
      // XXX 2.71
      //
      if (*p == '.'){
        p++;
        i++;
        if (!isdigit(*p)){
          // it's say "2.", make it be 2.0
        } else {
          while (isdigit(*p)){
            p++; i++;
          }
        }
        *(tmp + i) = '\0';
        appendFloat(SYM_FLOATING, atof(tmp));
        debug("appending symbol: %s (%f)", symtos(SYM_FLOATING), atof(tmp));
        free(tmp);
      } else {
        *(tmp + i) = '\0';
        appendInt(SYM_INTEGER, atoi(tmp));
        debug("appending symbol: %s (%d)", symtos(SYM_INTEGER), atoi(tmp));
        free(tmp);
      }
      p--;
    //
    // XXX .
    //
    } else if (*p == '.'){
      //
      //   .5
      //
      if (*(p + 1) >= '0' && *(p + 1) <= '9'){
        tmp = strdup(p);
        p++;
        i++;
        while (*p >= '0' && *p <= '9'){
          p++; i++;
        }
        *(tmp + i) = '\0';
        appendFloat(SYM_FLOATING, atof(tmp));
        debug("appending symbol: %s (%f)", symtos(SYM_FLOATING), atof(tmp));
        free(tmp);
      } else {
        append(SYM_DOT);
        debug("appending symbol: %s", symtos(SYM_DOT));
      }
      p--;
    //
    // XXX \s
    //
    } else if (*p == ' '){
      continue;
    } else {
      printf("nemo: warning: got an unknown character: %c\n", *p);
    }
  }
  //
  //   XXX C-d
  //
  append(SYM_EOF);
  debug("appending symbol: %s", symtos(SYM_EOF));

  free(code);
  fclose(fp);

  return symbols;
}
