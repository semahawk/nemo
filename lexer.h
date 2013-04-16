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

enum SymbolType {
  SYM_MY,             /*  0 "my"    */
  SYM_INTEGER,        /*  1         */
  SYM_FLOAT,          /*  2         */
  SYM_STRING,         /*  3         */
  SYM_NAME,           /*  4         */
  SYM_EQ,             /*  5 "="     */
  SYM_SEMICOLON,      /*  6 ";"     */
  SYM_COMMA,          /*  7 ","     */
  SYM_MINUS,          /*  8 "-"     */
  SYM_MINUSMINUS,     /*  9 "--"    */
  SYM_PLUS,           /* 10 "+"     */
  SYM_PLUSPLUS,       /* 11 "++"    */
  SYM_TIMES,          /* 12 "*"     */
  SYM_MODULO,         /* 13 "%"     */
  SYM_SLASH,          /* 14 "/"     */
  SYM_LPAREN,         /* 15 "("     */
  SYM_RPAREN,         /* 16 ")"     */
  SYM_LMUSTASHE,      /* 17 "{"     */
  SYM_RMUSTASHE,      /* 18 "}"     */
  SYM_LBRACKET,       /* 19 "["     */
  SYM_RBRACKET,       /* 20 "]"     */
  SYM_LT,             /* 20 "<"     */
  SYM_GT,             /* 20 ">"     */
  SYM_BANG,           /* 20 "!"     */
  SYM_QUESTION,       /* 20 "?"     */
  SYM_COLON,          /* 20 ":"     */
  SYM_NL,             /* 21 "\n"    */
  SYM_IF,             /* 22 "if"    */
  SYM_WHILE           /* 23 "while" */
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

void lexFile(Nemo *, LexerState *, char *);
void lexString(Nemo *, LexerState *, char *);
void lexerDestroy(Nemo *, LexerState *);

BOOL lexLast(LexerState *lex);
BOOL lexPeek(LexerState *lex, SymbolType);
BOOL lexAccept(LexerState *lex, SymbolType);
void lexForce(LexerState *lex, SymbolType);
void lexSkip(LexerState *lex);

const char *symToS(SymbolType type);

#endif /* LEXER_H */

