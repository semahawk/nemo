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
  SYM_MY,             /* "my"    */
  SYM_IF,             /* "if"    */
  SYM_WHILE,          /* "while" */
  SYM_ELSE,           /* "else"  */
  SYM_PRINT,          /* "print" */
  SYM_FN,             /* "fn"    */
  SYM_USE,            /* "use"   */
  SYM_INTEGER,        /*         */
  SYM_FLOAT,          /*         */
  SYM_STRING,         /*         */
  SYM_NAME,           /*         */
  SYM_EQ,             /* "="     */
  SYM_SEMICOLON,      /* ";"     */
  SYM_COMMA,          /* ","     */
  SYM_MINUS,          /* "-"     */
  SYM_MINUSMINUS,     /* "--"    */
  SYM_PLUS,           /* "+"     */
  SYM_PLUSPLUS,       /* "++"    */
  SYM_TIMES,          /* "*"     */
  SYM_MODULO,         /* "%"     */
  SYM_SLASH,          /* "/"     */
  SYM_LPAREN,         /* "("     */
  SYM_RPAREN,         /* ")"     */
  SYM_LMUSTASHE,      /* "{"     */
  SYM_RMUSTASHE,      /* "}"     */
  SYM_LBRACKET,       /* "["     */
  SYM_RBRACKET,       /* "]"     */
  SYM_LT,             /* "<"     */
  SYM_GT,             /* ">"     */
  SYM_BANG,           /* "!"     */
  SYM_COLON,          /* ":"     */
  SYM_QUESTION,       /* "?"     */
  SYM_PLUSEQ,         /* "+="    */
  SYM_MINUSEQ,        /* "-="    */
  SYM_TIMESEQ,        /* "*="    */
  SYM_SLASHEQ,        /* "/="    */
  SYM_MODULOEQ,       /* "%="    */
  SYM_EOS             /* end of script */
};

struct Symbol {
  enum SymbolType type;
  union {
    int i;
    double f;
    char *s;
  } value;
  unsigned line;
  unsigned column;
};

struct SymbolsList {
  struct Symbol sym;
  struct SymbolsList *next;
  struct SymbolsList *prev;
};

struct LexerState {
  BOOL is_file; /* either file or a string */
  char *source; /* name of the files name */
  unsigned line;
  unsigned column;
  struct SymbolsList *head;
  struct SymbolsList *tail;
  struct SymbolsList *current;
};

typedef enum SymbolType SymbolType;
typedef struct Symbol Symbol;
typedef struct SymbolsList SymbolsList;
typedef struct LexerState LexerState;

void NmLexer_LexFile(LexerState *, char *);
void NmLexer_LexString(LexerState *, char *);
void NmLexer_Destroy(LexerState *);

BOOL NmLexer_Peek(LexerState *lex, SymbolType);
BOOL NmLexer_Accept(LexerState *lex, SymbolType);
void NmLexer_Force(LexerState *lex, SymbolType);
void NmLexer_Skip(LexerState *lex);

const char *symToS(SymbolType type);

#endif /* LEXER_H */

