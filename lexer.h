/*
 * lexer.h
 *
 * Copyright: (c) 2012 by Szymon Urbaś
 *
 */

#ifndef LEXER_H
#define LEXER_H

typedef enum {
  SYM_VAR_IDENT,                // $varname     1
  SYM_IDENT,                    // name         2
  SYM_INTEGER,                  // 3            3
  SYM_FLOATING,                 // 2.71         4
  SYM_TYPE_INTEGER,             // int          5
  SYM_TYPE_FLOATING,            // float        6
  SYM_WHILE,                    // while        7
  SYM_IF,                       // if           8
  SYM_ELSE,                     // else         9
  SYM_FOR,                      // for         10
  SYM_NONE,                     // none        11
  SYM_RETURN,                   // return      12
  SYM_USE,                      // use         13
  SYM_PLUSPLUS,                 // ++          14
  SYM_MINUSMINUS,               // --          15
  SYM_EQ,                       // =           16
  SYM_LT, SYM_GT,               // < and >     17,18
  SYM_PLUS, SYM_MINUS,          // + and -     19,20
  SYM_TIMES, SYM_SLASH,         // * and /     21,22
  SYM_MODULO,                   // %           23
  SYM_LPAREN, SYM_RPAREN,       // ( and )     24,25
  SYM_LMUSTASHE, SYM_RMUSTASHE, // { and }     26,27
  SYM_COMMA, SYM_SEMICOLON,     // , and ;     28,29
  SYM_COLON, SYM_DOT,           // : and .     30,31
  SYM_QMARK, SYM_EMARK,         // ? and !     32,33
  SYM_NEWLINE,                  // \n          34
  SYM_EOF                       // C-d         35
} SymbolType;

typedef union {
  int i;
  float f;
  char *s;
} SymbolValue;

struct Symbol {
  int line;
  int column;
  SymbolType type;
  SymbolValue value;
};

struct Symbol **lexFile(const char *name);

#endif /* LEXER_H */
