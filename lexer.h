/*
 *
 * lexer.h
 *
 * Created at:  Sat Apr  6 18:00:06 2013 18:00:06
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

#ifndef LEXER_H
#define LEXER_H

#include "nemo.h"

enum SymbolType {
  SYM_MY,             /* "my"      */
  SYM_CONST,          /* "const"   */
  SYM_IF,             /* "if"      */
  SYM_UNLESS,         /* "unless"  */
  SYM_WHILE,          /* "while"   */
  SYM_UNTIL,          /* "until"   */
  SYM_ELSE,           /* "else"    */
  SYM_FUN,            /* "fun"     */
  SYM_USE,            /* "use"     */
  SYM_INCLUDE,        /* "include" */
  SYM_GOTO,           /* "goto"    */
  SYM_INTEGER,        /*           */
  SYM_FLOAT,          /*           */
  SYM_STRING,         /*           */
  SYM_OPT,            /* -[a-zA-Z] */
  SYM_NAME,           /*           */
  SYM_EQ,             /* "="       */
  SYM_SEMICOLON,      /* ";"       */
  SYM_COMMA,          /* ","       */
  SYM_MINUS,          /* "-"       */
  SYM_MINUSMINUS,     /* "--"      */
  SYM_PLUS,           /* "+"       */
  SYM_PLUSPLUS,       /* "++"      */
  SYM_TIMES,          /* "*"       */
  SYM_PERCENT,        /* "%"       */
  SYM_SLASH,          /* "/"       */
  SYM_LPAREN,         /* "("       */
  SYM_RPAREN,         /* ")"       */
  SYM_LMUSTASHE,      /* "{"       */
  SYM_RMUSTASHE,      /* "}"       */
  SYM_LBRACKET,       /* "["       */
  SYM_RBRACKET,       /* "]"       */
  SYM_LCHEVRON,       /* "<"       */
  SYM_RCHEVRON,       /* ">"       */
  SYM_LCHEVRONEQ,     /* "<="      */
  SYM_RCHEVRONEQ,     /* ">="      */
  SYM_EQEQ,           /* "=="      */
  SYM_BANGEQ,         /* "!="      */
  SYM_BANG,           /* "!"       */
  SYM_COLON,          /* ":"       */
  SYM_QUESTION,       /* "?"       */
  SYM_PLUSEQ,         /* "+="      */
  SYM_MINUSEQ,        /* "-="      */
  SYM_TIMESEQ,        /* "*="      */
  SYM_SLASHEQ,        /* "/="      */
  SYM_MODULOEQ,       /* "%="      */
  SYM_EOS             /* end of script */
};

/* Position in the code */
struct Pos {
  unsigned line;
  unsigned column;
};

struct Symbol {
  enum SymbolType type;
  union {
    int i;
    double f;
    char *s;
    char c;
  } value;
  struct Pos pos;
};

struct lexgc {
  void *p;
  struct lexgc *next;
};

struct LexerState {
  bool is_file; /* either file or a string */
  char *source; /* name of the files name */
  char *content; /* contents of the file */
  char *savecontent;
  unsigned line;
  unsigned column;
  unsigned saveline;
  unsigned savecolumn;
  struct Symbol current;
  bool eos;
  /* Indicates if the lexing things are just after the "fun" keyword.
   * It is set to true right after the keyword "fun" was encountered and set to
   * false after the first left mustache ('{') or the semicolon (';') was encountered.
   */
  bool right_after_fun;
  /* Indicates if the lexing things are after a function's name, preassumbly(?)
   * in a function call.
   * It is set to true right after that function name and set to false when the
   * first non-option symbol was encountered */
  bool right_after_funname;
  /* GC */
  struct lexgc *gc_head;
};

typedef struct Pos Pos;
typedef enum SymbolType SymbolType;
typedef struct Symbol Symbol;
typedef struct LexerState LexerState;

void NmLexer_Tidyup(LexerState *);
Symbol NmLexer_Fetch(LexerState *);
bool NmLexer_Peek(LexerState *, SymbolType);
bool NmLexer_Accept(LexerState *, SymbolType);
Symbol NmLexer_Force(LexerState *, SymbolType);
void NmLexer_Skip(LexerState *);

const char *symToS(SymbolType type);

#endif /* LEXER_H */

