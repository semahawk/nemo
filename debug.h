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

#include "lexer.h"

#define NmDebug_FREE(p)       NmDebug_Memory(p, "free")
#define NmDebug_MALLOC(p,i)   NmDebug_Memory(p, "malloc %lu bytes", i)
#define NmDebug_CALLOC(p,i,j) NmDebug_Memory(p, "calloc %lu x %lu bytes", i, j)
#define NmDebug_REALLOC(p,i)  NmDebug_Memory(p, "realloc %lu bytes", i)

/*
 * Definitions for debug flags (-d?)
 *
 * The number assigned to definition means the bit at which the flags is stored
 */
#define DEBUG_FLAG_AST    0
#define DEBUG_FLAG_LEXER  1
#define DEBUG_FLAG_MEMORY 2
#define DEBUG_FLAG_PARSER 3

void NmDebug_SetFlag(unsigned);

void NmDebug_Memory(void *, const char *, ...);

void NmDebug_Lexer(LexerState *, SymbolType);
void NmDebug_LexerInt(LexerState *, SymbolType, int);
void NmDebug_LexerFloat(LexerState *, SymbolType, double);
void NmDebug_LexerStr(LexerState *, SymbolType, char *);

void NmDebug_Parser(const char *, ...);

void NmDebug_AST(void *, const char *, ...);

#endif /* DEBUG_H */

