/*
 *
 * debug.h
 *
 * Created at:  Sat Apr 13 15:03:06 2013 15:03:06
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

#ifndef DEBUG_H
#define DEBUG_H

/*
 * The whole debugging thing only works if --enable-debug was supplied
 */
#if DEBUG

#include "lexer.h"

#define nm_debug_free(p)       nm_debug_mem(p, "free")
#define nm_debug_malloc(p,i)   nm_debug_mem(p, "malloc %lu bytes", i)
#define nm_debug_calloc(p,i,j) nm_debug_mem(p, "calloc %lu x %lu bytes", i, j)
#define nm_debug_realloc(p,i)  nm_debug_mem(p, "realloc %lu bytes", i)

/*
 * Definitions for debug flags (-d?)
 *
 * The number assigned to definition means the bit at which the flags is stored
 */
#define DEBUG_FLAG_AST    0
#define DEBUG_FLAG_LEXER  1
#define DEBUG_FLAG_MEMORY 2
#define DEBUG_FLAG_PARSER 3

void nm_debug_set_flag(unsigned);

void nm_debug_mem(void *, const char *, ...);

void nm_debug_lex(LexerState *, SymbolType);
void nm_debug_lexInt(LexerState *, SymbolType, int);
void nm_debug_lexFloat(LexerState *, SymbolType, double);
void nm_debug_lexStr(LexerState *, SymbolType, char *);
void nm_debug_lexChar(LexerState *, SymbolType, char);

void nm_debug_parser(const char *, ...);

void nm_debug_ast(void *, const char *, ...);
#endif /* #if DEBUG */

#endif /* DEBUG_H */

