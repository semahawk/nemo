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

#define NmDebug_FREE(p)       NmDebug_Memory(NM, p, "free")
#define NmDebug_MALLOC(p,i)   NmDebug_Memory(NM, p, "malloc %lu bytes", i)
#define NmDebug_CALLOC(p,i,j) NmDebug_Memory(NM, p, "calloc %lu x %lu bytes", i, j)
#define NmDebug_REALLOC(p,i)  NmDebug_Memory(NM, p, "realloc %lu bytes", i)

void NmDebug_Memory(Nemo *, void *, const char *, ...);

void NmDebug_Lexer(Nemo *, LexerState *, SymbolType);
void NmDebug_LexerInt(Nemo *, LexerState *, SymbolType, int);
void NmDebug_LexerFloat(Nemo *, LexerState *, SymbolType, double);
void NmDebug_LexerStr(Nemo *, LexerState *, SymbolType, char *);

void NmDebug_Parser(Nemo *, const char *, ...);
void NmDebug_ParserIndent(void);
void NmDebug_ParserDedent(void);

void NmDebug_AST(Nemo *, void *, const char *, ...);

#endif /* DEBUG_H */

