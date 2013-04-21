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

#define debugFree(p)       debugMemory(NM, p, "free")
#define debugMalloc(p,i)   debugMemory(NM, p, "malloc %lu bytes", i)
#define debugCalloc(p,i,j) debugMemory(NM, p, "calloc %lu x %lu bytes", i, j)

void debugMemory(Nemo *, void *, const char *, ...);

void debugLexer(Nemo *, LexerState *, SymbolType);
void debugLexerInt(Nemo *, LexerState *, SymbolType, int);
void debugLexerFloat(Nemo *, LexerState *, SymbolType, double);
void debugLexerStr(Nemo *, LexerState *, SymbolType, char *);

void debugParser(Nemo *, const char *, ...);
void debugParserIndent(void);
void debugParserDedent(void);

#endif /* DEBUG_H */

