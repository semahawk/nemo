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

#include "nemo.h"
#include "lexer.h"

#define PARSER_INDENT_STEP 2
static unsigned parser_debug_level = 0;

void NmDebug_Memory(Nemo *NM, void *pointer, const char *msg, ...)
{
  if (NM->flags.debug.memory){
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

void NmDebug_Lexer(Nemo *NM, LexerState *lex, SymbolType type)
{
  if (NM->flags.debug.lexer){
    fprintf(stderr, "lex: %05uL, %03uC: found %s\n", lex->line, lex->column, symToS(type));
  }
}

void NmDebug_LexerInt(Nemo *NM, LexerState *lex, SymbolType type, int i)
{
  if (NM->flags.debug.lexer){
    fprintf(stderr, "lex: %05uL, %03uC: found %s (%i)\n", lex->line, lex->column, symToS(type), i);
  }
}

void NmDebug_LexerFloat(Nemo *NM, LexerState *lex, SymbolType type, double f)
{
  if (NM->flags.debug.lexer){
    fprintf(stderr, "lex: %05uL, %03uC: found %s (%f)\n", lex->line, lex->column, symToS(type), f);
  }
}

void NmDebug_LexerStr(Nemo *NM, LexerState *lex, SymbolType type, char *s)
{
  if (NM->flags.debug.lexer){
    fprintf(stderr, "lex: %05uL, %03uC: found %s (%s)\n", lex->line, lex->column, symToS(type), s);
  }
}

void NmDebug_Parser(Nemo *NM, char const *msg, ...)
{
  if (NM->flags.debug.parser){
    va_list vl;
    /*unsigned i;*/
    va_start(vl, msg);
    /*for (i = 0; i < parser_debug_level; i++){*/
      /*fprintf(stderr, " ");*/
    /*}*/
    vfprintf(stderr, msg, vl);
    va_end(vl);
  }
}

void NmDebug_ParserIndent(void)
{
  parser_debug_level += PARSER_INDENT_STEP;
}

void NmDebug_ParserDedent(void)
{
  parser_debug_level -= PARSER_INDENT_STEP;
}

void NmDebug_AST(Nemo *NM, void *ptr, const char *msg, ...)
{
  if (NM->flags.debug.ast){
    va_list vl;
    va_start(vl, msg);
    fprintf(stderr, "%p: ", ptr);
    vfprintf(stderr, msg, vl);
    fprintf(stderr, "\n");
    va_end(vl);
  }
}

