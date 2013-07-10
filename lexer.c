/*
 *
 * lexer.c
 *
 * Created at:  Sat Apr  6 16:46:24 2013 16:46:24
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License: the MIT license
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

/*
 * "Face in the wind, we're riding the storm
 *  We'll stay our course whatever will come
 *  Wandering souls in the sea of the damned
 *  Death or glory, oh, oh, we're riding the
 *  storm"
 *
 *  Running Wild - Riding the Storm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "nemo.h"

static struct Keyword {
  const char * const name;
  SymbolType sym;
} keywords[] =
{
  { "my",      SYM_MY      },
  { "const",   SYM_CONST   },
  { "if",      SYM_IF      },
  { "unless",  SYM_UNLESS  },
  { "while",   SYM_WHILE   },
  { "until",   SYM_UNTIL   },
  { "else",    SYM_ELSE    },
  { "fun",     SYM_FUN     },
  { "use",     SYM_USE     },
  { "include", SYM_INCLUDE },
  { "goto",    SYM_GOTO    },
  { 0, 0 }
};
typedef struct Keyword Keyword;

void NmLexer_Tidyup(LexerState *lex)
{
  struct lexgc *curr;
  struct lexgc *next;

  for (curr = lex->gc_head; curr != NULL; curr = next){
    next = curr->next;
    NmMem_Free(curr->p);
    NmMem_Free(curr);
  }
}

static inline void new(LexerState *lex, Symbol *new, SymbolType type)
{
  /* initialize */
  new->type = type;
  new->pos.line = lex->line;
  new->pos.column = lex->column;
}

static inline void newInt(LexerState *lex, Symbol *new, int i)
{
  /* initialize */
  new->type = SYM_INTEGER;
  new->pos.line = lex->line;
  new->pos.column = lex->column;
  new->value.i = i;
}

static inline void newFloat(LexerState *lex, Symbol *new, double f)
{
  /* initialize */
  new->type = SYM_FLOAT;
  new->pos.line = lex->line;
  new->pos.column = lex->column;
  new->value.f = f;
}

static inline void newStr(LexerState *lex, Symbol *new, SymbolType type, char *s)
{
  struct lexgc *newgc = NmMem_Malloc(sizeof(struct lexgc));
  /* initialize */
  new->type = type;
  new->pos.line = lex->line;
  new->pos.column = lex->column;
  new->value.s = NmMem_Strdup(s);
  /* append that string to the GC list */
  newgc->p = new->value.s;
  newgc->next = lex->gc_head;
  lex->gc_head = newgc;
}

static inline void newChar(LexerState *lex, Symbol *new, SymbolType type, char c)
{
  /* initialize */
  new->type = type;
  new->pos.line = lex->line;
  new->pos.column = lex->column;
  new->value.c = c;
}

Symbol NmLexer_Fetch(LexerState *lex)
{
  char *tmp;
  int i = 0;
  bool found = true;
  Keyword *keyword;
  Symbol sym;
  char *p = lex->content;

  /* watch out for the nulls */
  if (*p == '\0'){
    new(lex, &sym, SYM_EOS);
    return sym;
  }

tryagain:
  /* tokenizing {{{ */
  /*
   * XXX name / keyword
   */
  if (isalpha(*p) || *p == '_'){
    tmp = NmMem_Strdup(p);
    /* fetch the name */
    while (isalpha(*p) ||
           isdigit(*p) ||
           *p == '_'   ||
          (*p == '.' && (*(p + 1) != '.') && (*(p - 1) != '.'))){
      p++; i++;
    }
    p--;
    *(tmp + i) = '\0';
    /* see if its a keyword */
    found = 0;
    for (keyword = keywords; keyword->name != NULL; keyword++){
      if (!strcmp(keyword->name, tmp)){
        new(lex, &sym, keyword->sym);
        found = 1;
        break;
      }
    }
    /* it's not a keyword */
    if (!found){
      newStr(lex, &sym, SYM_NAME, tmp);
    }
    NmMem_Free(tmp);
    /* "i" is the length of the name */
    lex->column += i;
  }
  /*
   * XXX 2
   */
  else if (isdigit(*p)){
    tmp = NmMem_Strdup(p);
    p++; i++;
    /* fetch the number */
    while (isdigit(*p)){
      p++; i++;
    }
    /*
     * XXX 2.
     */
    if (*p == '.'){
      p++; i++;
      /*
       * XXX 2.71
       */
      if (isdigit(*p)){
        while (isdigit(*p)){
          p++; i++;
        }
        *(tmp + i) = '\0';
        newFloat(lex, &sym, atof(tmp));
      /*
       * XXX it's just "2.", let's make it be 2.0
       */
      } else {
        *(tmp + i) = '\0';
        newFloat(lex, &sym, atof(tmp));
      }
    } else {
      newInt(lex, &sym, atoi(tmp));
    }
    NmMem_Free(tmp);
    p--;
    lex->column += i;
  }
  else if (*p == '='){
    /*
     * XXX '=='
     */
    if (*(p + 1) == '='){
      new(lex, &sym, SYM_EQEQ);
      lex->column += 2;
      p++;
    }
    /*
     * XXX '='
     */
    else {
      new(lex, &sym, SYM_EQ);
      lex->column++;
    }
  }
  else if (*p == ' '){
    lex->column++;
    p++;
    goto tryagain;
  }
  else if (*p == ';'){
    new(lex, &sym, SYM_SEMICOLON);
    lex->column++;
  }
  else if (*p == '+'){
    /*
     * XXX ++
     */
    if (*(p + 1) == '+'){
      new(lex, &sym, SYM_PLUSPLUS);
      lex->column += 2;
      p++;
    /*
     * XXX +=
     */
    } else if (*(p + 1) == '='){
      new(lex, &sym, SYM_PLUSEQ);
      lex->column += 2;
      p++;
    /*
     * XXX +
     */
    } else {
      new(lex, &sym, SYM_PLUS);
      lex->column++;
    }
  }
  else if (*p == '-'){
    /*
     * XXX FUN -option
     */
    if (lex->right_after_fun){
      char name;
      p++; /* skip over the '-' */
      name = *p;
      p++;
      /* skip over the rest of options name */
      while (isalpha(*p) || isdigit(*p))
        lex->column++, p++;
      newChar(lex, &sym, SYM_OPT, name);
      /* skip over the '-' and the first character after that */
      lex->column += 2;
    }
    /*
     * XXX NAME -options
     */
    else if (lex->right_after_funname){
      p++; /* skip over the '-' */
      tmp = NmMem_Strdup(p);
      while (isalpha(*p) || isdigit(*p)){
        p++; i++;
      }
      p--;
      *(tmp + i) = '\0';
      newStr(lex, &sym, SYM_OPT, tmp);
      /* skip over the '-' and the first character after that */
      NmMem_Free(tmp);
    }
    else {
      /*
       * XXX --
       */
      if (*(p + 1) == '-'){
        new(lex, &sym, SYM_MINUSMINUS);
        lex->column += 2;
        p++;
      /*
       * XXX -=
       */
      } else if (*(p + 1) == '='){
        new(lex, &sym, SYM_MINUSEQ);
        lex->column += 2;
        p++;
      /*
       * XXX -
       */
      } else {
        new(lex, &sym, SYM_MINUS);
        lex->column++;
      }
    }
  }
  else if (*p == '*'){
    /*
     * XXX *=
     */
    if (*(p + 1) == '='){
      new(lex, &sym, SYM_TIMESEQ);
      lex->column += 2;
      p++;
    /*
     * XXX *
     */
    } else {
      new(lex, &sym, SYM_TIMES);
      lex->column++;
    }
  }
  else if (*p == '/'){
    /*
     * XXX /\* comment *\/
     */
    if (*(p + 1) == '*'){
      p += 2;
      for (;;){
        if (*p == '\n'){
          lex->line++;
          lex->column = 1;
        } else if (*p == '*' && *(p + 1) == '/'){
          p += 2;
          break;
        }
        p++;
      }
      /*p--;*/
      goto tryagain;
    /*
     * XXX /=
     */
    } else if (*(p + 1) == '='){
      new(lex, &sym, SYM_SLASHEQ);
      lex->column += 2;
      p++;
    /*
     * XXX /
     */
    } else {
      new(lex, &sym, SYM_SLASH);
      lex->column++;
    }
  }
  else if (*p == '%'){
    /*
     * XXX %=
     */
    if (*(p + 1) == '='){
      new(lex, &sym, SYM_MODULOEQ);
      lex->column += 2;
      p++;
    /*
     * XXX %
     */
    } else {
      new(lex, &sym, SYM_PERCENT);
      lex->column++;
    }
  }
  else if (*p == ';'){
    new(lex, &sym, SYM_SEMICOLON);
    lex->column++;
  }
  else if (*p == ','){
    new(lex, &sym, SYM_COMMA);
    lex->column++;
  }
  else if (*p == '('){
    new(lex, &sym, SYM_LPAREN);
    lex->column++;
  }
  else if (*p == ')'){
    new(lex, &sym, SYM_RPAREN);
    lex->column++;
  }
  else if (*p == '{'){
    new(lex, &sym, SYM_LMUSTASHE);
    lex->column++;
  }
  else if (*p == '}'){
    new(lex, &sym, SYM_RMUSTASHE);
    lex->column++;
  }
  else if (*p == '['){
    new(lex, &sym, SYM_LBRACKET);
    lex->column++;
  }
  else if (*p == ']'){
    new(lex, &sym, SYM_RBRACKET);
    lex->column++;
  }
  else if (*p == '<'){
    /*
     * XXX '<='
     */
    if (*(p + 1) == '='){
      new(lex, &sym, SYM_LCHEVRONEQ);
      lex->column += 2;
      p++;
    }
    /*
     * XXX '<'
     */
    else {
      new(lex, &sym, SYM_LCHEVRON);
      lex->column++;
    }
  }
  else if (*p == '>'){
    /*
     * XXX '>='
     */
    if (*(p + 1) == '='){
      new(lex, &sym, SYM_RCHEVRONEQ);
      lex->column += 2;
      p++;
    }
    /*
     * XXX '>'
     */
    else {
      new(lex, &sym, SYM_RCHEVRON);
      lex->column++;
    }
  }
  else if (*p == '!'){
    /*
     * XXX '!='
     */
    if (*(p + 1) == '='){
      new(lex, &sym, SYM_BANGEQ);
      lex->column += 2;
      p++;
    }
    /*
     * XXX '!'
     */
    else {
      new(lex, &sym, SYM_BANG);
      lex->column++;
    }
  }
  else if (*p == '?'){
    new(lex, &sym, SYM_QUESTION);
    lex->column++;
  }
  else if (*p == ':'){
    new(lex, &sym, SYM_COLON);
    lex->column++;
  }
  else if (*p == '#'){
    /* skip over the whole thing */
    while (*p != '\n')
      p++;
    p++;
    lex->line++;
    lex->column = 1;
    goto tryagain;
  }
  /*
   * XXX "string"
   */
  else if (*p == '"'){
    p++; i++;
    tmp = NmMem_Strdup(p);
    lex->column++;
    while (*p != '"'){
      if (*p == '\n'){
        lex->line++;
        lex->column = 1;
      }
      p++; i++;
      lex->column++;
    }
    p--; i--;
    *(tmp + i) = '\0';
    /* skip over the '"' */
    p++;
    newStr(lex, &sym, SYM_STRING, tmp);
    NmMem_Free(tmp);
  }
  else if (*p == '\n'){
    p++;
    lex->line++;
    lex->column = 1;
    goto tryagain;
  }
  else if (*p == '\0'){
    new(lex, &sym, SYM_EOS);
    return sym;
  }
  else {
    printf("unknown character 0x%02x %d '%c' in line %u, column %u\n", *p, *p, *p, lex->line, lex->column);
    lex->column++;
    goto tryagain;
  }
  /* }}} */

  lex->content = ++p;

  return sym;
}

static inline void fallback(LexerState *lex)
{
  lex->line    = lex->saveline;
  lex->column  = lex->savecolumn;
  lex->content = lex->savecontent;
}

static inline void advance(LexerState *lex)
{
  lex->saveline    = lex->line;
  lex->savecolumn  = lex->column;
  lex->savecontent = lex->content;
}

/*
 * @name    NmLexer_Force
 * @desc    if the next symbol is not of given <type>, throw an error
 *          grammar is baad
 */
Symbol NmLexer_Force(LexerState *lex, SymbolType type)
{
  Symbol sym = NmLexer_Fetch(lex);

  if (sym.type != type){
    NmError_Lex(lex, "expected %s instead of %s", symToS(type), symToS(sym.type));
    exit(EXIT_FAILURE);
  } else {
    advance(lex);
    lex->current = sym;
    return sym;
  }
}

/*
 * @name    NmLexer_Accept
 * @desc    if the next symbol is of given <type> return true, and skip over it
 *          else return false
 */
bool NmLexer_Accept(LexerState *lex, SymbolType type)
{
  Symbol sym = NmLexer_Fetch(lex);

  if (sym.type == type){
    advance(lex);
    lex->current = sym;
    return true;
  }

  fallback(lex);

  return false;
}

/*
 * @name    NmLexer_Peek
 * @desc    check if the next symbol on the list is of a given <type>
 */
bool NmLexer_Peek(LexerState *lex, SymbolType type)
{
  Symbol sym = NmLexer_Fetch(lex);

  fallback(lex);

  if (sym.type == type){
    lex->current = sym;
    return true;
  }

  return false;
}

/*
 * @name    NmLexer_Skip
 * @desc    simply skip over the current symbol
 */
void NmLexer_Skip(LexerState *lex)
{
  Symbol sym = NmLexer_Fetch(lex);

  if (sym.type == SYM_EOS){
    NmError_Lex(lex, "unexpected <EOS> (skip)");
    Nm_Exit();
    lex->current.type = SYM_EOS;
  }

  advance(lex);
}

const char *symToS(SymbolType type)
{
  switch (type){
    case SYM_MY:         return "\"my\"";
    case SYM_IF:         return "\"if\"";
    case SYM_WHILE:      return "\"while\"";
    case SYM_ELSE:       return "\"else\"";
    case SYM_USE:        return "\"use\"";
    case SYM_INCLUDE:    return "\"include\"";
    case SYM_FUN:        return "\"fun\"";
    case SYM_INTEGER:    return "integer";
    case SYM_FLOAT:      return "float";
    case SYM_STRING:     return "string";
    case SYM_OPT:        return "option";
    case SYM_NAME:       return "name";
    case SYM_EQ:         return "'='";
    case SYM_SEMICOLON:  return "';'";
    case SYM_COMMA:      return "','";
    case SYM_MINUS:      return "'-'";
    case SYM_MINUSMINUS: return "'--'";
    case SYM_PLUS:       return "'+'";
    case SYM_PLUSPLUS:   return "'++'";
    case SYM_TIMES:      return "'*'";
    case SYM_PERCENT:    return "'%'";
    case SYM_SLASH:      return "'/'";
    case SYM_LPAREN:     return "'('";
    case SYM_RPAREN:     return "')'";
    case SYM_LMUSTASHE:  return "'{'";
    case SYM_RMUSTASHE:  return "'}'";
    case SYM_LBRACKET:   return "'['";
    case SYM_RBRACKET:   return "']'";
    case SYM_LCHEVRON:   return "'<'";
    case SYM_RCHEVRON:   return "'>'";
    case SYM_LCHEVRONEQ: return "'<='";
    case SYM_RCHEVRONEQ: return "'>='";
    case SYM_EQEQ:       return "'=='";
    case SYM_BANGEQ:     return "'!='";
    case SYM_BANG:       return "'!'";
    case SYM_QUESTION:   return "'?'";
    case SYM_COLON:      return "':'";
    case SYM_PLUSEQ:     return "'+='";
    case SYM_MINUSEQ:    return "'-='";
    case SYM_TIMESEQ:    return "'*='";
    case SYM_SLASHEQ:    return "'/='";
    case SYM_MODULOEQ:   return "'%='";
    case SYM_EOS:        return "<EOS>";
    default:             return "#unknown#symToS#";
  }
}

/*
 * Rhapsody, Steve Vai, Helloween, Ensiferum
 * Testament
 *
 * The Office
 *
 */

