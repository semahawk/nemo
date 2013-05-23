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

/* defines which characters are a valid character for eg. name */
#define validForNameHead(c) (isalpha((c)) || (c) == '_')
#define validForNameTail(c) (isalpha((c)) || isdigit(c) || (c) == '_')

static struct Keyword {
  const char * const name;
  SymbolType sym;
} keywords[] =
{
  { "my",      SYM_MY      },
  { "const",   SYM_CONST   },
  { "if",      SYM_IF      },
  { "while",   SYM_WHILE   },
  { "else",    SYM_ELSE    },
  { "print",   SYM_PRINT   },
  { "fn",      SYM_FN      },
  { "use",     SYM_USE     },
  { "include", SYM_INCLUDE },
  { 0, 0 }
};
typedef struct Keyword Keyword;

/*
 * append a symbol of a given <type>
 */
static void append(LexerState *lex, SymbolType type)
{
  SymbolsList *new = NmMem_Malloc(sizeof(SymbolsList));
  NmDebug_Lexer(lex, type);
  /* initialize */
  new->sym.type = type;
  new->sym.line = lex->line;
  new->sym.column = lex->column;
  /* append it */
  /*   the list is empty */
  if (!lex->head && !lex->tail){
    new->next = lex->head;
    new->prev = lex->tail;
    lex->head = new;
    lex->tail = new;
    lex->current = new;
  /*   its not empty */
  } else {
    new->next = lex->head->next;
    lex->head->next = new;
    new->prev = lex->head;
    lex->head = new;
  }
}

static void appendInt(LexerState *lex, int i)
{
  SymbolsList *new = NmMem_Malloc(sizeof(SymbolsList));
  NmDebug_LexerInt(lex, SYM_INTEGER, i);
  /* initialize */
  new->sym.type = SYM_INTEGER;
  new->sym.line = lex->line;
  new->sym.column = lex->column;
  new->sym.value.i = i;
  /* append it */
  /*   the list is empty */
  if (!lex->head && !lex->tail){
    new->next = lex->head;
    new->prev = lex->tail;
    lex->head = new;
    lex->tail = new;
    lex->current = new;
  /*   its not empty */
  } else {
    new->next = lex->head->next;
    lex->head->next = new;
    new->prev = lex->head;
    lex->head = new;
  }
}

static void appendFloat(LexerState *lex, double f)
{
  SymbolsList *new = NmMem_Malloc(sizeof(SymbolsList));
  NmDebug_LexerFloat(lex, SYM_FLOAT, f);
  /* initialize */
  new->sym.type = SYM_FLOAT;
  new->sym.line = lex->line;
  new->sym.column = lex->column;
  new->sym.value.f = f;
  /* append it */
  /*   the list is empty */
  if (!lex->head && !lex->tail){
    new->next = lex->head;
    new->prev = lex->tail;
    lex->head = new;
    lex->tail = new;
    lex->current = new;
  /*   its not empty */
  } else {
    new->next = lex->head->next;
    lex->head->next = new;
    new->prev = lex->head;
    lex->head = new;
  }
}

static void appendStr(LexerState *lex, SymbolType type, char *s)
{
  SymbolsList *new = NmMem_Malloc(sizeof(SymbolsList));
  NmDebug_LexerStr(lex, type, s);
  /* initialize */
  new->sym.type = type;
  new->sym.line = lex->line;
  new->sym.column = lex->column;
  new->sym.value.s = NmMem_Strdup(s);
  /* append it */
  /*   the list is empty */
  if (!lex->head && !lex->tail){
    new->next = lex->head;
    new->prev = lex->tail;
    lex->head = new;
    lex->tail = new;
    lex->current = new;
  /*   its not empty */
  } else {
    new->next = lex->head->next;
    lex->head->next = new;
    new->prev = lex->head;
    lex->head = new;
  }
}

static void NmLexer_Init(LexerState *lex)
{
  lex->line    = 1;
  lex->column  = 1;
  lex->head    = NULL;
  lex->tail    = NULL;
  lex->current = NULL;
}

/*
 * clean after the lexers work
 */
void NmLexer_Destroy(LexerState *lex)
{
  SymbolsList *p;
  SymbolsList *next;

  for (p = lex->tail; p != NULL; p = next){
    next = p->next;
    /* TODO: debug */
    if (p->sym.type == SYM_NAME ||
        p->sym.type == SYM_STRING){
      NmMem_Free(p->sym.value.s);
    }
    NmMem_Free(p);
  }
}

void NmLexer_LexFile(LexerState *lex, char *fname)
{
  FILE *fp;
  char *fbuffer = NULL;
  size_t flen = 0;

  if ((fp = fopen(fname, "r")) == NULL){
    NmError_Fatal("cannot open file '%s'", fname);
    exit(EXIT_FAILURE);
  }
  /* get the files length */
  fseek(fp, 0, SEEK_END);
  flen = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  /* make room for the contents */
  fbuffer = NmMem_Malloc(flen);
  /* store the files contents in the fbuffer */
  if (fread(fbuffer, 1, flen, fp) != flen){
    NmError_Fatal("fread failed in " __FILE__ " at line %d", __LINE__);
    exit(EXIT_FAILURE);
  }
  fbuffer[flen - 1] = '\0';
  /* now, treat the source as a string */
  NmLexer_LexString(lex, fbuffer);
  /* free the buffer */
  NmMem_Free(fbuffer);

  fclose(fp);
}

void NmLexer_LexString(LexerState *lex, char *string)
{
  char *p, *tmp;
  int i = 0, found = 0;
  Keyword *keyword;
  NmLexer_Init(lex);
  /* iterate through the string, and append symbols to the symbols list */
  for (p = string; *p != '\0'; p++, i = 0){
    /*
     * XXX name / keyword
     */
    if (validForNameHead(*p)){
      tmp = NmMem_Strdup(p);
      /* fetch the name */
      while (validForNameTail(*p)){
        p++; i++;
      }
      p--;
      *(tmp + i) = '\0';
      /* see if its a keyword */
      found = 0;
      for (keyword = keywords; keyword->name != NULL; keyword++){
        if (!strcmp(keyword->name, tmp)){
          append(lex, keyword->sym);
          found = 1;
          break;
        }
      }
      /* it's not a keyword */
      if (!found){
        appendStr(lex, SYM_NAME, tmp);
      }
      NmMem_Free(tmp);
      /* i is the length of the name */
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
          appendFloat(lex, atof(tmp));
        /*
         * XXX it's just "2.", let's make it be 2.0
         */
        } else {
          *(tmp + i) = '\0';
          appendFloat(lex, atof(tmp));
        }
      } else {
        appendInt(lex, atoi(tmp));
      }
      NmMem_Free(tmp);
      p--;
      lex->column += i;
    }
    else if (*p == '='){
      append(lex, SYM_EQ);
      lex->column++;
    }
    else if (*p == ' '){
      lex->column++;
    }
    else if (*p == ';'){
      append(lex, SYM_SEMICOLON);
      lex->column++;
    }
    else if (*p == '+'){
      /*
       * XXX ++
       */
      if (*(p + 1) == '+'){
        append(lex, SYM_PLUSPLUS);
        lex->column += 2;
        p++;
      /*
       * XXX +=
       */
      } else if (*(p + 1) == '='){
        append(lex, SYM_PLUSEQ);
        lex->column += 2;
        p++;
      /*
       * XXX +
       */
      } else {
        append(lex, SYM_PLUS);
        lex->column++;
      }
    }
    else if (*p == '-'){
      /*
       * XXX --
       */
      if (*(p + 1) == '-'){
        append(lex, SYM_MINUSMINUS);
        lex->column += 2;
        p++;
      /*
       * XXX -=
       */
      } else if (*(p + 1) == '='){
        append(lex, SYM_MINUSEQ);
        lex->column += 2;
        p++;
      /*
       * XXX -
       */
      } else {
        append(lex, SYM_MINUS);
        lex->column++;
      }
    }
    else if (*p == '*'){
      /*
       * XXX *=
       */
      if (*(p + 1) == '='){
        append(lex, SYM_TIMESEQ);
        lex->column += 2;
        p++;
      /*
       * XXX *
       */
      } else {
        append(lex, SYM_TIMES);
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
        p--;
      /*
       * XXX /=
       */
      } else if (*(p + 1) == '='){
        append(lex, SYM_SLASHEQ);
        lex->column += 2;
        p++;
      /*
       * XXX /
       */
      } else {
        append(lex, SYM_SLASH);
        lex->column++;
      }
    }
    else if (*p == '%'){
      /*
       * XXX %=
       */
      if (*(p + 1) == '='){
        append(lex, SYM_MODULOEQ);
        lex->column += 2;
        p++;
      /*
       * XXX %
       */
      } else {
        append(lex, SYM_MODULO);
        lex->column++;
      }
    }
    else if (*p == ';'){
      append(lex, SYM_SEMICOLON);
      lex->column++;
    }
    else if (*p == ','){
      append(lex, SYM_COMMA);
      lex->column++;
    }
    else if (*p == '('){
      append(lex, SYM_LPAREN);
      lex->column++;
    }
    else if (*p == ')'){
      append(lex, SYM_RPAREN);
      lex->column++;
    }
    else if (*p == '{'){
      append(lex, SYM_LMUSTASHE);
      lex->column++;
    }
    else if (*p == '}'){
      append(lex, SYM_RMUSTASHE);
      lex->column++;
    }
    else if (*p == '['){
      append(lex, SYM_LBRACKET);
      lex->column++;
    }
    else if (*p == ']'){
      append(lex, SYM_RBRACKET);
      lex->column++;
    }
    else if (*p == '<'){
      append(lex, SYM_LT);
      lex->column++;
    }
    else if (*p == '>'){
      append(lex, SYM_GT);
      lex->column++;
    }
    else if (*p == '!'){
      append(lex, SYM_BANG);
      lex->column++;
    }
    else if (*p == '?'){
      append(lex, SYM_QUESTION);
      lex->column++;
    }
    else if (*p == ':'){
      append(lex, SYM_COLON);
      lex->column++;
    }
    else if (*p == '#'){
      /* skip over the whole thing */
      while (*p++ != '\n');
      p--;
      lex->line++;
      lex->column = 1;
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
      appendStr(lex, SYM_STRING, tmp);
      NmMem_Free(tmp);
    }
    else if (*p == '\n'){
      lex->line++;
      lex->column = 1;
    }
    else {
      printf("unknown character %c\n", *p);
      lex->column++;
    }
  }
  append(lex, SYM_EOS);
}

/*
 * @name    NmLexer_Force
 * @desc    if the next symbol is not of given <type>, throw an error
 *          grammar is baad
 */
void NmLexer_Force(LexerState *lex, SymbolType type)
{
  if (lex->current == NULL){
    if (lex->is_file){
      NmError_Lex(lex, "unexpected end of file");
    } else {
      NmError_Lex(lex, "unexpected end of string");
    }
    exit(EXIT_FAILURE);
  }

  if (lex->current->sym.type != type){
    NmError_Lex(lex, "expected %s instead of %s", symToS(type), symToS(lex->current->sym.type));
    exit(EXIT_FAILURE);
  } else {
    lex->current = lex->current->next;
  }
}

/*
 * @name    NmLexer_Accept
 * @desc    if the next symbol is of given <type> return true, and skip over it
 *          else return false
 */
BOOL NmLexer_Accept(LexerState *lex, SymbolType type)
{
  if (lex->current == NULL){
    if (lex->is_file){
      NmError_Lex(lex, "unexpected end of file");
    } else {
      NmError_Lex(lex, "unexpected end of string");
    }
    exit(EXIT_FAILURE);
  }

  if (lex->current->sym.type == type){
    lex->current = lex->current->next;
    return TRUE;
  }

  return FALSE;
}

/*
 * @name    NmLexer_Peek
 * @desc    check if the next symbol on the list is of a given <type>
 */
BOOL NmLexer_Peek(LexerState *lex, SymbolType type)
{
  if ((lex->current != NULL) && (lex->current->sym.type == type))
    return TRUE;

  return FALSE;
}

/*
 * @name    NmLexer_Skip
 * @desc    simply skip over the current symbol
 */
void NmLexer_Skip(LexerState *lex)
{
  if (lex->current == NULL){
    if (lex->is_file){
      NmError_Lex(lex, "unexpected end of file");
    } else {
      NmError_Lex(lex, "unexpected end of string");
    }
    exit(EXIT_FAILURE);
  }

  lex->current = lex->current->next;
}

const char *symToS(SymbolType type)
{
  switch (type){
    case SYM_MY:         return "\"my\"";
    case SYM_IF:         return "\"if\"";
    case SYM_WHILE:      return "\"while\"";
    case SYM_ELSE:       return "\"else\"";
    case SYM_PRINT:      return "\"print\"";
    case SYM_USE:        return "\"use\"";
    case SYM_INCLUDE:    return "\"include\"";
    case SYM_FN:         return "\"fn\"";
    case SYM_INTEGER:    return "integer";
    case SYM_FLOAT:      return "float";
    case SYM_STRING:     return "string";
    case SYM_NAME:       return "name";
    case SYM_EQ:         return "'='";
    case SYM_SEMICOLON:  return "';'";
    case SYM_COMMA:      return "','";
    case SYM_MINUS:      return "'-'";
    case SYM_MINUSMINUS: return "'--'";
    case SYM_PLUS:       return "'+'";
    case SYM_PLUSPLUS:   return "'++'";
    case SYM_TIMES:      return "'*'";
    case SYM_MODULO:     return "'%'";
    case SYM_SLASH:      return "'/'";
    case SYM_LPAREN:     return "'('";
    case SYM_RPAREN:     return "')'";
    case SYM_LMUSTASHE:  return "'{'";
    case SYM_RMUSTASHE:  return "'}'";
    case SYM_LBRACKET:   return "'['";
    case SYM_RBRACKET:   return "']'";
    case SYM_LT:         return "'<'";
    case SYM_GT:         return "'>'";
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
 *
 * The Office
 *
 */

