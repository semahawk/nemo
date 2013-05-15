/*
 *
 * debug.c
 *
 * Created at:  Sat Apr 13 15:01:36 2013 15:01:36
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
 * "This is the end of the road; this is the end of the line
 *  This is the end of your life; this is the...
 *
 *  A society in a society, inside the fence life as you know it stops
 *  They got their rules of conduct and we got ours
 *  Be quick or be dead, you crumble up and die, the clock is
 *  Ticking so slowly and so much can happen in an hour"
 *
 *  Megadeth - Endgame
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

#include "nemo.h"
#include "lexer.h"
#include "debug.h"

/*
 * Every bit here sets on/off the debugging flags
 * Which bit means what is described in debug.h
 */
static uint8_t debug_flags = 0;

/*
 * Set the given <flag> to 1
 */
void NmDebug_SetFlag(unsigned flag)
{
  debug_flags |= 1 << flag;
}

/* Simple macro to get the given <flag> */
/* <flag> is of type { unsigned } */
#define NmDebug_GETFLAG(flag) (debug_flags & (1 << (flag)))

void NmDebug_AST(void *ptr, const char *msg, ...)
{
  if (NmDebug_GETFLAG(DEBUG_FLAG_AST)){
    va_list vl;
    va_start(vl, msg);
    fprintf(stderr, "%p: ", ptr);
    vfprintf(stderr, msg, vl);
    fprintf(stderr, "\n");
    va_end(vl);
  }
}

void NmDebug_Memory(void *pointer, const char *msg, ...)
{
  if (NmDebug_GETFLAG(DEBUG_FLAG_MEMORY)){
    static unsigned count = 1;
    va_list vl;
    va_start(vl, msg);
    fprintf(stderr, "%p: (%u) ", (void*)pointer, count);
    vfprintf(stderr, msg, vl);
    fprintf(stderr, "\n");
    va_end(vl);
    count++;
  }
}

void NmDebug_Lexer(LexerState *lex, SymbolType type)
{
  if (NmDebug_GETFLAG(DEBUG_FLAG_LEXER)){
    fprintf(stderr, "lex: %05uL, %03uC: found %s\n", lex->line, lex->column, symToS(type));
  }
}

void NmDebug_LexerInt(LexerState *lex, SymbolType type, int i)
{
  if (NmDebug_GETFLAG(DEBUG_FLAG_LEXER)){
    fprintf(stderr, "lex: %05uL, %03uC: found %s (%i)\n", lex->line, lex->column, symToS(type), i);
  }
}

void NmDebug_LexerFloat(LexerState *lex, SymbolType type, double f)
{
  if (NmDebug_GETFLAG(DEBUG_FLAG_LEXER)){
    fprintf(stderr, "lex: %05uL, %03uC: found %s (%f)\n", lex->line, lex->column, symToS(type), f);
  }
}

void NmDebug_LexerStr(LexerState *lex, SymbolType type, char *s)
{
  if (NmDebug_GETFLAG(DEBUG_FLAG_LEXER)){
    fprintf(stderr, "lex: %05uL, %03uC: found %s (%s)\n", lex->line, lex->column, symToS(type), s);
  }
}

void NmDebug_Parser(char const *msg, ...)
{
  if (NmDebug_GETFLAG(DEBUG_FLAG_PARSER)){
    va_list vl;
    va_start(vl, msg);
    vfprintf(stderr, msg, vl);
    va_end(vl);
  }
}

