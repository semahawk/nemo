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
#include "mem.h"

#define isLiteral(n) (n->type == NT_INTEGER ||\
                      n->type == NT_FLOAT)

static Node *expr(Nemo *NM, LexerState *lex);
static Node *block(Nemo *NM, LexerState *lex);

/*
 * primary_expr: INTEGER
 *             | FLOAT
 *             | NAME
 *             | '(' expr ')'
 *             ;
 */
static Node *primary_expr(Nemo *NM, LexerState *lex)
{
  Node *new = NULL;

  if (lexPeek(lex, SYM_INTEGER)){
    new = genIntNode(NM, lex->current->sym.value.i);
    debugParser(NM, "%d ", new->data.i);
    lexSkip(lex);
  }
  else if (lexPeek(lex, SYM_FLOAT)){
    new = genFloatNode(NM, lex->current->sym.value.f);
    debugParser(NM, "%f ", new->data.f);
    lexSkip(lex);
  }
  else if (lexPeek(lex, SYM_NAME)){
    new = genNameNode(NM, lex->current->sym.value.s);
    debugParser(NM, "%s ", new->data.s);
    lexSkip(lex);
  }
  else if (lexAccept(lex, SYM_LPAREN)){
    debugParser(NM, "(");
    new = expr(NM, lex);
    lexForce(lex, SYM_RPAREN);
    debugParser(NM, ")");
  }
  else if (lexAccept(lex, SYM_EOS)){
    lexError(lex, "unexpected end of file");
    exit(EXIT_FAILURE);
  }
  else {
    lexError(lex, "factor, we've got a problem with \"%s\"", symToS(lex->current->sym.type));
    lexSkip(lex);
  }

  return new;
}

/*
 *   postfix_op: '++'
 *             | '--'
 *             ;
 *
 * postfix_expr: NAME '(' [expr] ')'
 *             | primary_expr postfix_op
 *             ;
 */
static Node *postfix_expr(Nemo *NM, LexerState *lex)
{
  Node *target = NULL;
  Node *ret = target = primary_expr(NM, lex);

  /*
   * XXX NAME '(' [expr] ')'
   */
  if (lexAccept(lex, SYM_LPAREN)){
    if (target->type != NT_NAME){
      lexError(lex, "expected a name for a function call");
      exit(EXIT_FAILURE);
    }
    debugParser(NM, "(");
    if (!lexPeek(lex, SYM_RPAREN)){
      /* FIXME: params list */
      expr(NM, lex);
    }
    lexForce(lex, SYM_RPAREN);
    debugParser(NM, ")");
  }
  /*
   * XXX NAME '++'
   */
  else if (lexAccept(lex, SYM_PLUSPLUS)){
    if (isLiteral(target)){
      nmError("can't do the postfix increment on a literal in line %u at column %u", lex->current->prev->sym.line, lex->current->prev->sym.column);
      exit(EXIT_FAILURE);
    }
    debugParser(NM, ":postfix++");
    ret = genUnopNode(NM, target, UNARY_POSTINC);
  }
  /*
   * XXX NAME '--'
   */
  else if (lexAccept(lex, SYM_MINUSMINUS)){
    if (isLiteral(target)){
      nmError("can't do the postfix increment on a literal in line %u at column %u", lex->current->prev->sym.line, lex->current->prev->sym.column);
      exit(EXIT_FAILURE);
    }
    debugParser(NM, ":postfix--");
    ret = genUnopNode(NM, target, UNARY_POSTDEC);
  }

  return ret;
}

/*
 *   prefix_op: '+'
 *            | '-'
 *            | '!'
 *            | '++'
 *            | '--'
 *            ;
 *
 * prefix_expr: postfix_expr
 *            | prefix_op prefix_expr
 *            ;
 */
static Node *prefix_expr(Nemo *NM, LexerState *lex)
{
  Node *ret = NULL;
  Node *target = NULL;

  if (lexAccept(lex, SYM_BANG)){
    debugParser(NM, "unary! ");
    target = prefix_expr(NM, lex);
    ret = genUnopNode(NM, target, UNARY_NEGATE);
  }
  else if (lexAccept(lex, SYM_PLUS)){
    debugParser(NM, "unary+ ");
    target = prefix_expr(NM, lex);
    ret = genUnopNode(NM, target, UNARY_PLUS);
  }
  else if (lexAccept(lex, SYM_MINUS)){
    debugParser(NM, "unary- ");
    target = prefix_expr(NM, lex);
    ret = genUnopNode(NM, target, UNARY_MINUS);
  }
  else if (lexAccept(lex, SYM_PLUSPLUS)){
    debugParser(NM, "prefix++:");
    target = prefix_expr(NM, lex);
    ret = genUnopNode(NM, target, UNARY_PREINC);
  }
  else if (lexAccept(lex, SYM_MINUSMINUS)){
    debugParser(NM, "prefix--:");
    target = prefix_expr(NM, lex);
    ret = genUnopNode(NM, target, UNARY_PREDEC);
  }
  else {
    ret = postfix_expr(NM, lex);
  }

  return ret;
}

/*
 *   mult_op: '*'
 *          | '/'
 *          | '%'
 *          ;
 *
 * mult_expr: prefix_expr [mult_op prefix_expr]*
 *          ;
 */
static Node *mult_expr(Nemo *NM, LexerState *lex)
{
  Node *ret;
  Node *left;
  Node *right;
  BinaryOp op = 0;

  ret = left = prefix_expr(NM, lex);

  while (lexPeek(lex, SYM_TIMES) || lexPeek(lex, SYM_SLASH)){
    if (lexAccept(lex, SYM_TIMES)){
      op = BINARY_MUL;
      debugParser(NM, "* ");
    } else if (lexAccept(lex, SYM_SLASH)){
      op = BINARY_DIV;
      debugParser(NM, "/ ");
    }
    right = prefix_expr(NM, lex);
    ret = genBinopNode(NM, left, BINARY_MUL, right);
    left = ret;
  }

  return ret;
}

/*
 *   add_op: '+'
 *         | '-'
 *         ;
 *
 * add_expr: mult_expr [add_op mult_expr]*
 *         ;
 */
static Node *add_expr(Nemo *NM, LexerState *lex)
{
  Node *ret;
  Node *left;
  Node *right;
  BinaryOp op = 0;

  ret = left = mult_expr(NM, lex);

  while (lexPeek(lex, SYM_PLUS) || lexPeek(lex, SYM_MINUS)){
    if (lexAccept(lex, SYM_PLUS)){
      op = BINARY_ADD;
      debugParser(NM, "+ ");
    } else if (lexAccept(lex, SYM_MINUS)){
      op = BINARY_SUB;
      debugParser(NM, "- ");
    }
    right = mult_expr(NM, lex);
    ret = genBinopNode(NM, left, op, right);
    left = ret;
  }

  return ret;
}

/*
 *   cond_op: '>'
 *          | '<'
 *          ;
 *
 * cond_expr: add_expr [cond_op add_expr]*
 *          ;
 */
static Node *cond_expr(Nemo *NM, LexerState *lex)
{
  Node *ret;
  Node *left;
  Node *right;
  BinaryOp op = 0;

  ret = left = add_expr(NM, lex);

  while (lexPeek(lex, SYM_GT) || lexPeek(lex, SYM_LT)){
    if (lexAccept(lex, SYM_GT)){
      op = BINARY_GT;
      debugParser(NM, "> ");
    } else if (lexAccept(lex, SYM_LT)){
      op = BINARY_LT;
      debugParser(NM, "< ");
    }
    right = add_expr(NM, lex);
    ret = genBinopNode(NM, left, op, right);
    left = ret;
  }

  return ret;
}

/*
 *   assign_op:  '='
 *            | '+='
 *            | '-='
 *            | '*='
 *            | '/='
 *            | '%='
 *            ;
 *
 * assign_expr: cond_expr [assign_op assign_expr]*
 *            ;
 */
static Node *assign_expr(Nemo *NM, LexerState *lex)
{
  Node *ret;
  Node *left;
  Node *right;
  BinaryOp op = 0;

  ret = left = cond_expr(NM, lex);

  while (lexPeek(lex, SYM_EQ)      ||
         lexPeek(lex, SYM_PLUSEQ)  ||
         lexPeek(lex, SYM_MINUSEQ) ||
         lexPeek(lex, SYM_TIMESEQ) ||
         lexPeek(lex, SYM_SLASHEQ) ||
         lexPeek(lex, SYM_MODULOEQ)){
    if (lexAccept(lex, SYM_EQ)){
      op = BINARY_ADD;
      debugParser(NM, "= ");
    }
    else if (lexAccept(lex, SYM_PLUSEQ)){
      op = BINARY_ASSIGN_ADD;
      debugParser(NM, "+= ");
    }
    else if (lexAccept(lex, SYM_MINUSEQ)){
      op = BINARY_ASSIGN_SUB;
      debugParser(NM, "-= ");
    }
    else if (lexAccept(lex, SYM_TIMESEQ)){
      op = BINARY_ASSIGN_MUL;
      debugParser(NM, "*= ");
    }
    else if (lexAccept(lex, SYM_SLASHEQ)){
      op = BINARY_ASSIGN_DIV;
      debugParser(NM, "/= ");
    }
    else if (lexAccept(lex, SYM_MODULOEQ)){
      op = BINARY_ASSIGN_MOD;
      debugParser(NM, "%= ");
    }
    right = assign_expr(NM, lex);
    ret = genBinopNode(NM, left, op, right);
    left = ret;
  }

  return ret;
}

/*
 * expr: MY NAME [= assign_expr]
 *     | assign_expr
 *     ;
 */
static Node *expr(Nemo *NM, LexerState *lex)
{
  Node *ret = NULL;
  Node *value = NULL;
  char *name = NULL;

  if (lexAccept(lex, SYM_MY)){
    debugParser(NM, "my ");
    lexForce(lex, SYM_NAME);
    /* lexForce skips the symbol so we have to get to the previous one */
    name = lex->current->prev->sym.value.s;
    debugParser(NM, "%s", name);
    if (lexAccept(lex, SYM_EQ)){
      debugParser(NM, " = ");
      value = assign_expr(NM, lex);
    }
    ret = genDeclNode(NM, name, value);
  } else {
    ret = assign_expr(NM, lex);
  }

  return ret;
}

/*
 * stmt: ';'
 *     | block
 *     | IF stmt stmt
 *     | WHILE stmt stmt
 *     | expr IF stmt
 *     | expr WHILE stmt
 *     | expr ';'
 *     ;
 */
static Node *stmt(Nemo *NM, LexerState *lex)
{
  Node *ret   = NULL;
  Node *guard = NULL;
  Node *body  = NULL;
  Node *elsee = NULL;

  /*
   * XXX ;
   */
  if (lexAccept(lex, SYM_SEMICOLON)){
    /* that's NOP
     * we're generating it anyway, because it would come in handy when we're
     * compiling into the bytecode as NOP opcode
     */
    debugParser(NM, ";\n");
    ret = genNopNode(NM);
  }
  /*
   * XXX IF stmt stmt
   */
  else if (lexAccept(lex, SYM_IF)){
    debugParser(NM, "if ");
    guard = stmt(NM, lex);
    body = stmt(NM, lex);
    /*
     * XXX IF stmt stmt ELSE stmt
     */
    if (lexAccept(lex, SYM_ELSE)){
      debugParser(NM, "else ");
      elsee = stmt(NM, lex);
    }
    ret = genIfNode(NM, guard, body, elsee);
  }
  /*
   * XXX WHILE stmt stmt
   */
  else if (lexAccept(lex, SYM_WHILE)){
    debugParser(NM, "while ");
    guard = stmt(NM, lex);
    body = stmt(NM, lex);
    /*
     * XXX WHILE stmt stmt ELSE stmt
     */
    if (lexAccept(lex, SYM_ELSE)){
      debugParser(NM, "else ");
      elsee = stmt(NM, lex);
    }
    ret = genWhileNode(NM, guard, body, elsee);
  }
  /*
   * XXX { block }
   */
  else if (lexAccept(lex, SYM_LMUSTASHE)){
    debugParser(NM, "{\n");
    debugParserIndent();
    ret = block(NM, lex);
    lexForce(lex, SYM_RMUSTASHE);
    debugParserDedent();
    debugParser(NM, "}\n");
  } else {
    body = ret = expr(NM, lex);
    /*
     * XXX expr IF stmt
     */
    if (lexAccept(lex, SYM_IF)){
      debugParser(NM, "if ");
      guard = stmt(NM, lex);
      ret = genIfNode(NM, ret, guard, NULL);
    }
    /*
     * XXX expr WHILE stmt
     */
    else if (lexAccept(lex, SYM_WHILE)){
      debugParser(NM, "while ");
      guard = stmt(NM, lex);
      ret = genWhileNode(NM, ret, guard, NULL);
    }
    /* if the next symbol is '{', '}' or end-of-script, it
     * doesn't need the semicolon then, otherwise it is required */
    else if (!lexPeek(lex, SYM_LMUSTASHE) &&
        !lexPeek(lex, SYM_RMUSTASHE) &&
        !lexPeek(lex, SYM_EOS)){
      lexForce(lex, SYM_SEMICOLON);
    }
    debugParser(NM, ";\n");
  }

  return ret;
}

/*
 * block: '{' [stmt]* '}'
 *      ;
 */
static Node *block(Nemo *NM, LexerState *lex)
{
  Node *new_block = nmMalloc(NM, sizeof(Node));

  new_block->type = NT_BLOCK;
  new_block->data.block.head = NULL;
  new_block->data.block.tail = NULL;

  while (!lexPeek(lex, SYM_RMUSTASHE) && !lexPeek(lex, SYM_EOS)){
    Statement *new_stmt = nmMalloc(NM, sizeof(Statement));
    new_stmt->stmt = stmt(NM, lex);
    /* append that statement to the statements of the block */
    /*   the list is empty */
    if (!new_block->data.block.head && !new_block->data.block.tail){
      new_stmt->next = new_block->data.block.head;
      new_stmt->prev = new_block->data.block.tail;
      new_block->data.block.head = new_stmt;
      new_block->data.block.tail = new_stmt;
    /*   the list is NOT empty */
    } else {
      new_stmt->next = new_block->data.block.head->next;
      new_block->data.block.head->next = new_stmt;
      new_stmt->prev = new_block->data.block.head;
      new_block->data.block.head = new_stmt;
    }
  }

  return new_block;
}

/*
 * @name - parseFile
 * @desc - parse file of a given <fname> and return a pointer to the node of a
 *         block that was parsed, the main block of the whole script
 * @return - {Node *} of .type = NT_BLOCK
 */
Node *parseFile(Nemo *NM, char *fname)
{
  Node *nodest = NULL;
  LexerState lex;
  lex.is_file = TRUE;
  lex.source = fname;
  lexFile(NM, &lex, fname);
  nodest = block(NM, &lex);
  lexerDestroy(NM, &lex);

  return nodest;
}

/*
 * @name - parseString
 * @desc - parse the given <string> and return a pointer to the node of a
 *         block that was parsed, the main block of the script
 *         (it probably should return something else)
 * @return - {Node *} of .type = NT_BLOCK
 */
Node *parseString(Nemo *NM, char *string)
{
  Node *nodest = NULL;
  LexerState lex;
  lex.is_file = FALSE;
  lex.source = string;
  lexString(NM, &lex, string);
  nodest = block(NM, &lex);
  lexerDestroy(NM, &lex);

  return nodest;
}

