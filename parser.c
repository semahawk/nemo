/*
 *
 * parser.c
 *
 * Created at:  Tue Apr  9 19:37:33 2013 19:37:33
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
 * "We live inside society, our presence is unknown
 *  We plot and we manipulate you to which way to go
 *  Illumination comes across the world to bring an end
 *  Intimidating silent force that puts us in command"
 *
 *  Gamma Ray - New World Order
 */

#include <stdio.h>
#include <stdlib.h>

#include "nemo.h"
#include "error.h"
#include "lexer.h"

#define isLiteral(s) (s == SYM_INTEGER ||\
                      s == SYM_FLOAT)

static void expr(LexerState *lex);
static void block(LexerState *lex);

static SymbolType factor(LexerState *lex)
{
  SymbolType ret;

  if (lexPeek(lex, SYM_INTEGER)){
    printf("%d ", lex->current->sym.value.i);
    lexSkip(lex);
    ret = SYM_INTEGER;
  }
  else if (lexPeek(lex, SYM_FLOAT)){
    printf("%f ", lex->current->sym.value.f);
    lexSkip(lex);
    ret = SYM_FLOAT;
  }
  else if (lexPeek(lex, SYM_NAME)){
    printf("%s ", lex->current->sym.value.s);
    lexSkip(lex);
    ret = SYM_NAME;
  }
  else if (lexAccept(lex, SYM_LPAREN)){
    printf("( ");
    expr(lex);
    lexForce(lex, SYM_RPAREN);
    printf(") ");
  } else {
    nmError("factor, we've got a problem (%s)", symToS(lex->current->sym.type));
    lexSkip(lex);
  }

  return ret;
}

static void postfix(LexerState *lex)
{
  SymbolType op = factor(lex);

  /*
   * XXX NAME "(" [expr] ")"
   */
  if (lexAccept(lex, SYM_LPAREN)){
    if (op != SYM_NAME){
      nmError("expected a name for a function call");
      exit(EXIT_FAILURE);
    }
    printf("(");
    if (!lexPeek(lex, SYM_RPAREN)){
      /* FIXME: params list */
      expr(lex);
    }
    lexForce(lex, SYM_RPAREN);
    printf(")");
  }
  /*
   * XXX NAME "++"
   */
  else if (lexAccept(lex, SYM_PLUSPLUS)){
    if (isLiteral(op)){
      nmError("can't do the postfix increment on a literal %s in line %u at column %u", symToS(op), lex->current->prev->sym.line, lex->current->prev->sym.column);
      exit(EXIT_FAILURE);
    }
    printf(":postfix++");
  }
  /*
   * XXX NAME "--"
   */
  else if (lexAccept(lex, SYM_MINUSMINUS)){
    if (isLiteral(op)){
      nmError("can't do the postfix increment on a literal %s in line %u at column %u", symToS(op), lex->current->prev->sym.line, lex->current->prev->sym.column);
      exit(EXIT_FAILURE);
    }
    printf(":postfix--");
  }
}

static void prefix(LexerState *lex)
{
  if (lexAccept(lex, SYM_BANG)){
    printf("unary! ");
    prefix(lex);
  }
  else if (lexAccept(lex, SYM_PLUS)){
    printf("unary+ ");
    prefix(lex);
  }
  else if (lexAccept(lex, SYM_MINUS)){
    printf("unary- ");
    prefix(lex);
  }
  else if (lexAccept(lex, SYM_PLUSPLUS)){
    printf("prefix++:");
    prefix(lex);
  }
  else if (lexAccept(lex, SYM_MINUSMINUS)){
    printf("prefix--:");
    prefix(lex);
  }
  else {
    postfix(lex);
  }
}

static void mult(LexerState *lex)
{
  prefix(lex);

  while (lexPeek(lex, SYM_TIMES) || lexPeek(lex, SYM_SLASH) || lexPeek(lex, SYM_MODULO)){
    if (lexAccept(lex, SYM_TIMES)){
      printf("* ");
    } else if (lexAccept(lex, SYM_SLASH)){
      printf("/ ");
    } else if (lexAccept(lex, SYM_MODULO)){
      printf("%% ");
    }
    prefix(lex);
  }
}

static void add(LexerState *lex)
{
  mult(lex);

  while (lexPeek(lex, SYM_PLUS) || lexPeek(lex, SYM_MINUS)){
    if (lexAccept(lex, SYM_PLUS)){
      printf("+ ");
    } else if (lexAccept(lex, SYM_MINUS)){
      printf("- ");
    }
    mult(lex);
  }
}

static void cond(LexerState *lex)
{
  add(lex);

  while (lexPeek(lex, SYM_GT) || lexPeek(lex, SYM_LT)){
    if (lexAccept(lex, SYM_GT)){
      printf("> ");
    } else if (lexAccept(lex, SYM_LT)){
      printf("< ");
    }
    add(lex);
  }
}

static void assign(LexerState *lex)
{
  cond(lex);

  while (lexPeek(lex, SYM_PLUSEQ)   ||
         lexPeek(lex, SYM_MINUSEQ)  ||
         lexPeek(lex, SYM_TIMESEQ)  ||
         lexPeek(lex, SYM_SLASHEQ)  ||
         lexPeek(lex, SYM_MODULOEQ)){
    if (lexAccept(lex, SYM_PLUSEQ)){
      printf("+= ");
    }
    else if (lexAccept(lex, SYM_MINUSEQ)){
      printf("-= ");
    }
    else if (lexAccept(lex, SYM_TIMESEQ)){
      printf("*= ");
    }
    else if (lexAccept(lex, SYM_SLASHEQ)){
      printf("/= ");
    }
    else if (lexAccept(lex, SYM_MODULOEQ)){
      printf("%%= ");
    }
    cond(lex);
  }
}

static void expr(LexerState *lex)
{
  if (lexAccept(lex, SYM_MY)){
    printf("my ");
    lexForce(lex, SYM_NAME);
    printf("name ");
    if (lexAccept(lex, SYM_EQ)){
      printf("= ");
      assign(lex);
    }
  } else {
    assign(lex);
  }
}

static void stmt(LexerState *lex)
{
  if (lexAccept(lex, SYM_IF)){
    printf("if ");
    stmt(lex);
    stmt(lex);
    printf("\n");
  }
  else if (lexAccept(lex, SYM_WHILE)){
    printf("while ");
    stmt(lex);
    stmt(lex);
    printf("\n");
  }
  else if (lexAccept(lex, SYM_LMUSTASHE)){
    printf("{\n");
    block(lex);
    lexForce(lex, SYM_RMUSTASHE);
    printf("}\n");
  } else {
    expr(lex);
    /* if the next symbol is "if", "while", '{', '}' or end-of-script, it
     * doesn't need the semicolon then, otherwise it is required */
    if (!lexPeek(lex, SYM_IF) &&
        !lexPeek(lex, SYM_WHILE) &&
        !lexPeek(lex, SYM_LMUSTASHE) &&
        !lexPeek(lex, SYM_RMUSTASHE) &&
        !lexLast(lex)){
      lexForce(lex, SYM_SEMICOLON);
    }
    printf(";\n");
  }
}

static void block(LexerState *lex)
{
  while (!lexPeek(lex, SYM_RMUSTASHE) && !lexLast(lex)){
    stmt(lex);
  }
}

void parseFile(Nemo *NM, char *fname)
{
  LexerState lex;
  lex.is_file = TRUE;
  lex.source = fname;
  lexFile(NM, &lex, fname);
  block(&lex);
  lexerDestroy(NM, &lex);
}

void parseString(Nemo *NM, char *string)
{
  LexerState lex;
  lex.is_file = FALSE;
  lex.source = string;
  lexString(NM, &lex, string);
  block(&lex);
  lexerDestroy(NM, &lex);
}

