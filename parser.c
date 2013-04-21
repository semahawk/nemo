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
#include "debug.h"
#include "lexer.h"
#include "ast.h"

#define isLiteral(n) (n->type == NT_INTEGER ||\
                      n->type == NT_FLOAT)

static Node *expr(Nemo *NM, LexerState *lex);
static Node *block(Nemo *NM, LexerState *lex);

static Node *factor(Nemo *NM, LexerState *lex)
{
  Node *new = NULL;

  if (lexPeek(lex, SYM_INTEGER)){
    debugParser(NM, "%d ", lex->current->sym.value.i);
    new = genIntNode(NM, lex->current->sym.value.i);
    lexSkip(lex);
  }
  else if (lexPeek(lex, SYM_FLOAT)){
    debugParser(NM, "%f ", lex->current->sym.value.f);
    new = genFloatNode(NM, lex->current->sym.value.f);
    lexSkip(lex);
  }
  else if (lexPeek(lex, SYM_NAME)){
    debugParser(NM, "%s ", lex->current->sym.value.s);
    new = genNameNode(NM, lex->current->sym.value.s);
    lexSkip(lex);
  }
  else if (lexAccept(lex, SYM_LPAREN)){
    debugParser(NM, "( ");
    new = expr(NM, lex);
    lexForce(lex, SYM_RPAREN);
    debugParser(NM, ") ");
  } else {
    nmError("factor, we've got a problem (%s)", symToS(lex->current->sym.type));
    lexSkip(lex);
  }

  return new;
}

static Node *mult(Nemo *NM, LexerState *lex)
{
  Node *ret;
  Node *left;
  Node *right;
  BinaryOp op = 0;

  ret = left = factor(NM, lex);

  while (lexPeek(lex, SYM_TIMES) || lexPeek(lex, SYM_SLASH)){
    if (lexAccept(lex, SYM_TIMES)){
      op = BINARY_MUL;
    } else if (lexAccept(lex, SYM_SLASH)){
      op = BINARY_DIV;
    }
    right = factor(NM, lex);
    ret = genBinopNode(NM, left, BINARY_MUL, right);
    left = ret;
  }

  return ret;
}

static Node *add(Nemo *NM, LexerState *lex)
{
  Node *ret;
  Node *left;
  Node *right;
  BinaryOp op = 0;

  ret = left = mult(NM, lex);

  while (lexPeek(lex, SYM_PLUS) || lexPeek(lex, SYM_MINUS)){
    if (lexAccept(lex, SYM_PLUS)){
      op = BINARY_ADD;
    } else if (lexAccept(lex, SYM_MINUS)){
      op = BINARY_SUB;
    }
    right = mult(NM, lex);
    ret = genBinopNode(NM, left, op, right);
    left = ret;
  }

  return ret;
}

static Node *expr(Nemo *NM, LexerState *lex)
{
  Node *ret = NULL;
  Node *value = NULL;

  if (lexAccept(lex, SYM_MY)){
    debugParser(NM, "my ");
    lexForce(lex, SYM_NAME);
    /* lexForce skips the symbol so we have to get to the previous one */
    /*name = lex->current->prev->sym.value.s;*/
    debugParser(NM, "%s ", lex->current->prev->sym.value.s);
    if (lexAccept(lex, SYM_EQ)){
      debugParser(NM, "= ");
      value = add(NM, lex);
      /*printf("add: %d\n", add(NM, lex));*/
    }
    /*ret = genDeclNode(NM, name, value);*/
  } else {
    ret = add(NM, lex);
    /*printf("add: %d\n", add(NM, lex));*/
  }

  return ret;
}

static Node *stmt(Nemo *NM, LexerState *lex)
{
  Node *ret = NULL;
  Node *guard = NULL;
  Node *body = NULL;

  if (lexAccept(lex, SYM_IF)){
    debugParser(NM, "if ");
    guard = stmt(NM, lex);
    body = stmt(NM, lex);
    debugParser(NM, "\n");
    ret = genIfNode(NM, guard, body);
  }
  else if (lexAccept(lex, SYM_WHILE)){
    debugParser(NM, "while ");
    guard = stmt(NM, lex);
    body = stmt(NM, lex);
    debugParser(NM, "\n");
    ret = genWhileNode(NM, guard, body);
  }
  else if (lexAccept(lex, SYM_LMUSTASHE)){
    debugParser(NM, "{\n");
    block(NM, lex);
    lexForce(lex, SYM_RMUSTASHE);
    debugParser(NM, "}\n");
  } else {
    /*ret = */expr(NM, lex);
    ret = NULL;
    /* if the next symbol is "if", "while", '{', '}' or end-of-script, it
     * doesn't need the semicolon then, otherwise it is required */
    if (!lexPeek(lex, SYM_IF) &&
        !lexPeek(lex, SYM_WHILE) &&
        !lexPeek(lex, SYM_LMUSTASHE) &&
        !lexPeek(lex, SYM_RMUSTASHE) &&
        !lexLast(lex)){
      lexForce(lex, SYM_SEMICOLON);
    }
    debugParser(NM, ";\n");
  }

  return ret;
}

static Node *block(Nemo *NM, LexerState *lex)
{
  Node *ret = NULL;

  while (!lexPeek(lex, SYM_RMUSTASHE) && !lexLast(lex)){
    stmt(NM, lex);
  }

  return ret;
}

void parseFile(Nemo *NM, char *fname)
{
  Node *nodest = NULL;
  LexerState lex;
  lex.is_file = TRUE;
  lex.source = fname;
  lexFile(NM, &lex, fname);
  nodest = block(NM, &lex);
  lexerDestroy(NM, &lex);
}

void parseString(Nemo *NM, char *string)
{
  Node *nodest = NULL;
  LexerState lex;
  lex.is_file = FALSE;
  lex.source = string;
  lexString(NM, &lex, string);
  nodest = block(NM, &lex);
  lexerDestroy(NM, &lex);
}

