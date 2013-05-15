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
#include <string.h>

#include "nemo.h"
#include "error.h"
#include "parser.h"
#include "debug.h"
#include "lexer.h"
#include "ast.h"
#include "mem.h"

/* <n> is of type { Node * } */
#define isLiteral(n) (n->type == NT_INTEGER || \
                      n->type == NT_FLOAT   || \
                      n->type == NT_STRING)

/* <lex> is of type { LexerState * } */
#define nextIsType(lex) (NmLexer_Peek(lex, SYM_TINT)   || \
                     NmLexer_Peek(lex, SYM_TFLOAT) || \
                     NmLexer_Peek(lex, SYM_TSTR))

/* <lex> is of type { LexerState * } */
#define endStmt(lex) do { \
  if (!NmLexer_Peek(lex, SYM_LMUSTASHE) && \
      !NmLexer_Peek(lex, SYM_RMUSTASHE) && \
      !NmLexer_Peek(lex, SYM_EOS)){ \
    NmLexer_Force(lex, SYM_SEMICOLON); \
  } \
  NmDebug_Parser(";\n"); \
} while (0);

static Node *expr(LexerState *lex);
static Node *block(LexerState *lex);

/*
 * primary_expr: INTEGER
 *             | FLOAT
 *             | STRING
 *             | NAME
 *             | '(' expr ')'
 *             ;
 */
static Node *primary_expr(LexerState *lex)
{
  Node *new = NULL;

  /*
   * XXX INTEGER
   */
  if (NmLexer_Peek(lex, SYM_INTEGER)){
    new = NmAST_GenInt(lex->current->sym.value.i);
    NmDebug_Parser("%d ", new->data.i);
    NmLexer_Skip(lex);
  }
  /*
   * XXX FLOAT
   */
  else if (NmLexer_Peek(lex, SYM_FLOAT)){
    new = NmAST_GenFloat(lex->current->sym.value.f);
    NmDebug_Parser("%f ", new->data.f);
    NmLexer_Skip(lex);
  }
  /*
   * XXX STRING
   */
  else if (NmLexer_Peek(lex, SYM_STRING)){
    new = NmAST_GenString(lex->current->sym.value.s);
    NmDebug_Parser("\"%s\" ", new->data.s);
    NmLexer_Skip(lex);
  }
  /*
   * XXX NAME
   */
  else if (NmLexer_Peek(lex, SYM_NAME)){
    new = NmAST_GenName(lex->current->sym.value.s);
    NmDebug_Parser("%s ", new->data.s);
    NmLexer_Skip(lex);
  }
  /*
   * XXX '(' expr ')'
   */
  else if (NmLexer_Accept(lex, SYM_LPAREN)){
    NmDebug_Parser("(");
    new = expr(lex);
    NmLexer_Force(lex, SYM_RPAREN);
    NmDebug_Parser(")");
  }
  /*
   * XXX EOS
   */
  else if (NmLexer_Accept(lex, SYM_EOS)){
    /*NmError_Lex(lex, "unexpected end of file");*/
    /*exit(EXIT_FAILURE);*/
  }
  /*
   * XXX nothing described above, returning NULL
   */
  else {
    return NULL;
  }

  return new;
}

/*
 * params_list: # empty
 *            | expr [',' expr]*
 *            ;
 */
static Node **params_list(LexerState *lex)
{
  unsigned nmemb = 5;
  unsigned counter = 0;
  Node **params = NmMem_Calloc(nmemb, sizeof(Node));
  Node *first_expr = expr(lex);

  if (!first_expr){
    NmMem_Free(params);
    return NULL;
  }

  params[counter++] = first_expr;
  while (NmLexer_Accept(lex, SYM_COMMA)){
    Node *next_expr = expr(lex);
    if (counter + 1 > nmemb){
      nmemb++;
      params = NmMem_Realloc(params, nmemb * sizeof(Node));
    }
    params[counter++] = next_expr;
  }

  NmDebug_AST(params, "create params list");

  return params;
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
static Node *postfix_expr(LexerState *lex)
{
  Node *target = NULL;
  Node *ret = target = primary_expr(lex);
  Node ** params;
  char *name = NULL;

  /*
   * XXX NAME '(' [params_list] ')'
   */
  if (NmLexer_Accept(lex, SYM_LPAREN)){
    if (target->type != NT_NAME){
      NmError_Lex(lex, "expected a name for a function call, not %s", symToS(lex->current->sym.type));
      exit(EXIT_FAILURE);
    }
    name = lex->current->prev->sym.value.s;
    NmDebug_Parser("%s ", name);
    NmDebug_Parser("(");
    /*if (!NmLexer_Peek(lex, SYM_RPAREN)){*/
      params = params_list(lex);
    /*}*/
    NmLexer_Force(lex, SYM_RPAREN);
    NmDebug_Parser(")");
    ret = NmAST_GenCall(name, params);
  }
  /*
   * XXX NAME '++'
   */
  else if (NmLexer_Accept(lex, SYM_PLUSPLUS)){
    if (isLiteral(target)){
      NmError_Error("can't do the postfix increment on a literal in line %u at column %u", lex->current->prev->sym.line, lex->current->prev->sym.column);
      exit(EXIT_FAILURE);
    }
    NmDebug_Parser(":postfix++");
    ret = NmAST_GenUnop(target, UNARY_POSTINC);
  }
  /*
   * XXX NAME '--'
   */
  else if (NmLexer_Accept(lex, SYM_MINUSMINUS)){
    if (isLiteral(target)){
      NmError_Error("can't do the postfix increment on a literal in line %u at column %u", lex->current->prev->sym.line, lex->current->prev->sym.column);
      exit(EXIT_FAILURE);
    }
    NmDebug_Parser(":postfix--");
    ret = NmAST_GenUnop(target, UNARY_POSTDEC);
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
static Node *prefix_expr(LexerState *lex)
{
  Node *ret = NULL;
  Node *target = NULL;

  if (NmLexer_Accept(lex, SYM_BANG)){
    NmDebug_Parser("unary! ");
    target = prefix_expr(lex);
    ret = NmAST_GenUnop(target, UNARY_NEGATE);
  }
  else if (NmLexer_Accept(lex, SYM_PLUS)){
    NmDebug_Parser("unary+ ");
    target = prefix_expr(lex);
    ret = NmAST_GenUnop(target, UNARY_PLUS);
  }
  else if (NmLexer_Accept(lex, SYM_MINUS)){
    NmDebug_Parser("unary- ");
    target = prefix_expr(lex);
    ret = NmAST_GenUnop(target, UNARY_MINUS);
  }
  else if (NmLexer_Accept(lex, SYM_PLUSPLUS)){
    NmDebug_Parser("prefix++:");
    target = prefix_expr(lex);
    ret = NmAST_GenUnop(target, UNARY_PREINC);
  }
  else if (NmLexer_Accept(lex, SYM_MINUSMINUS)){
    NmDebug_Parser("prefix--:");
    target = prefix_expr(lex);
    ret = NmAST_GenUnop(target, UNARY_PREDEC);
  }
  else {
    ret = postfix_expr(lex);
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
static Node *mult_expr(LexerState *lex)
{
  Node *ret;
  Node *left;
  Node *right;
  BinaryOp op = 0;

  ret = left = prefix_expr(lex);

  while (NmLexer_Peek(lex, SYM_TIMES) || NmLexer_Peek(lex, SYM_SLASH)){
    if (NmLexer_Accept(lex, SYM_TIMES)){
      op = BINARY_MUL;
      NmDebug_Parser("* ");
    } else if (NmLexer_Accept(lex, SYM_SLASH)){
      op = BINARY_DIV;
      NmDebug_Parser("/ ");
    }
    right = prefix_expr(lex);
    ret = NmAST_GenBinop(left, BINARY_MUL, right);
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
static Node *add_expr(LexerState *lex)
{
  Node *ret;
  Node *left;
  Node *right;
  BinaryOp op = 0;

  ret = left = mult_expr(lex);

  while (NmLexer_Peek(lex, SYM_PLUS) || NmLexer_Peek(lex, SYM_MINUS)){
    if (NmLexer_Accept(lex, SYM_PLUS)){
      op = BINARY_ADD;
      NmDebug_Parser("+ ");
    } else if (NmLexer_Accept(lex, SYM_MINUS)){
      op = BINARY_SUB;
      NmDebug_Parser("- ");
    }
    right = mult_expr(lex);
    ret = NmAST_GenBinop(left, op, right);
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
static Node *cond_expr(LexerState *lex)
{
  Node *ret;
  Node *left;
  Node *right;
  BinaryOp op = 0;

  ret = left = add_expr(lex);

  while (NmLexer_Peek(lex, SYM_GT) || NmLexer_Peek(lex, SYM_LT)){
    if (NmLexer_Accept(lex, SYM_GT)){
      op = BINARY_GT;
      NmDebug_Parser("> ");
    } else if (NmLexer_Accept(lex, SYM_LT)){
      op = BINARY_LT;
      NmDebug_Parser("< ");
    }
    right = add_expr(lex);
    ret = NmAST_GenBinop(left, op, right);
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
static Node *assign_expr(LexerState *lex)
{
  Node *ret;
  Node *left;
  Node *right;
  BinaryOp op = 0;

  ret = left = cond_expr(lex);

  while (NmLexer_Peek(lex, SYM_EQ)      ||
         NmLexer_Peek(lex, SYM_PLUSEQ)  ||
         NmLexer_Peek(lex, SYM_MINUSEQ) ||
         NmLexer_Peek(lex, SYM_TIMESEQ) ||
         NmLexer_Peek(lex, SYM_SLASHEQ) ||
         NmLexer_Peek(lex, SYM_MODULOEQ)){
    if (NmLexer_Accept(lex, SYM_EQ)){
      op = BINARY_ASSIGN;
      NmDebug_Parser("= ");
    }
    else if (NmLexer_Accept(lex, SYM_PLUSEQ)){
      op = BINARY_ASSIGN_ADD;
      NmDebug_Parser("+= ");
    }
    else if (NmLexer_Accept(lex, SYM_MINUSEQ)){
      op = BINARY_ASSIGN_SUB;
      NmDebug_Parser("-= ");
    }
    else if (NmLexer_Accept(lex, SYM_TIMESEQ)){
      op = BINARY_ASSIGN_MUL;
      NmDebug_Parser("*= ");
    }
    else if (NmLexer_Accept(lex, SYM_SLASHEQ)){
      op = BINARY_ASSIGN_DIV;
      NmDebug_Parser("/= ");
    }
    else if (NmLexer_Accept(lex, SYM_MODULOEQ)){
      op = BINARY_ASSIGN_MOD;
      NmDebug_Parser("%= ");
    }
    right = assign_expr(lex);
    ret = NmAST_GenBinop(left, op, right);
    left = ret;
  }

  return ret;
}

/*
 * expr: MY NAME [= assign_expr]
 *     | PRINT params_list
 *     | assign_expr
 *     ;
 */
static Node *expr(LexerState *lex)
{
  Node *ret = NULL;
  Node *value = NULL;
  Node **params;
  char *name = NULL;

  /*
   * XXX MY
   */
  if (NmLexer_Accept(lex, SYM_MY)){
    NmDebug_Parser("my ");
    NmLexer_Force(lex, SYM_NAME);
    /* NmLexer_Force skips the symbol so we have to get to the previous one */
    name = lex->current->prev->sym.value.s;
    NmDebug_Parser("%s", name);
    /*
     * XXX MY NAME = assign_expr
     */
    if (NmLexer_Accept(lex, SYM_EQ)){
      NmDebug_Parser(" = ");
      value = assign_expr(lex);
    }
    ret = NmAST_GenDecl(name, value);
  }
  /*
   * XXX PRINT
   */
  else if (NmLexer_Accept(lex, SYM_PRINT)){
    NmDebug_Parser("print ");
    /*
     * XXX PRINT '(' params_list ')'
     */
    if (NmLexer_Accept(lex, SYM_LPAREN)){
      params = params_list(lex);
      NmLexer_Force(lex, SYM_RPAREN);
    /*
     * XXX PRINT params_list
     */
    } else {
      params = params_list(lex);
    }
    ret = NmAST_GenCall("print", params);
  }
  else {
    ret = assign_expr(lex);
  }

  return ret;
}

/*
 * stmt: ';'
 *     | '{' block '}'
 *     | function_prototype
 *     | USE NAME ';'
 *     | IF stmt stmt
 *     | WHILE stmt stmt
 *     | expr IF stmt
 *     | expr WHILE stmt
 *     | expr ';'
 *     ;
 *
 * function_prototype: FN NAME '(' ')' block
 *                   | FN NAME '(' ')' ';'
 *                   | FN NAME '(' [NAME[',' NAME]*]+ ')' block
 *                   | FN NAME '(' [NAME[',' NAME]*]+ ')' ';'
 *                   ;
 */
static Node *stmt(LexerState *lex)
{
  Node *ret   = NULL;
  Node *guard = NULL;
  Node *body  = NULL;
  Node *elsee = NULL;
  char *name  = NULL;

  /*
   * XXX ';'
   */
  if (NmLexer_Accept(lex, SYM_SEMICOLON)){
    /* that's NOP */
    NmDebug_Parser(";\n");
    ret = NmAST_GenNop();
  }
  /*
   * XXX USE NAME ';'
   */
  else if (NmLexer_Accept(lex, SYM_USE)){
    char *tmp;
    NmDebug_Parser("use ");
    NmLexer_Force(lex, SYM_NAME);
    tmp = lex->current->prev->sym.value.s;
    NmDebug_Parser("%s ", tmp);
    name = NmMem_Malloc(strlen(tmp) + 4);
    strncpy(name, tmp, strlen(tmp));
    name[strlen(tmp)]     = '.';
    name[strlen(tmp) + 1] = 'n';
    name[strlen(tmp) + 2] = 'm';
    name[strlen(tmp) + 3] = '\0';
    /* return the block that was returned by parsing the file */
    /* only when the "used" name is different than the current source's name */
    if (!strcmp(name, lex->source)){
      ret = NmAST_GenNop();
    } else {
      ret = NmParser_ParseFile(name);
    }
    endStmt(lex);
    NmMem_Free(name);
  }
  /*
   * XXX FN NAME
   */
  else if (NmLexer_Accept(lex, SYM_FN)){
    NmDebug_Parser("fn ");
    NmDebug_Parser("%s ", symToS(lex->current->sym.type));
    NmLexer_Force(lex, SYM_NAME);
    name = lex->current->prev->sym.value.s;
    NmDebug_Parser("%s ", name);
    /*
     * XXX FN NAME '('
     */
    NmLexer_Force(lex, SYM_LPAREN);
    NmDebug_Parser("(");
    /*
     * XXX FN NAME '(' ')'
     */
    if (NmLexer_Accept(lex, SYM_RPAREN)){
      NmDebug_Parser(")");
    } else {
      /*
       * XXX FN NAME '(' [NAME[',' NAME]*]+ ')'
       */
      do {
         NmLexer_Force(lex, SYM_NAME);
         NmDebug_Parser("%s ", lex->current->prev->sym.value.s);
         if (NmLexer_Peek(lex, SYM_COMMA)){
           NmDebug_Parser(", ");
         }
      } while (NmLexer_Accept(lex, SYM_COMMA));
      NmLexer_Force(lex, SYM_RPAREN);
      NmDebug_Parser(")");
    }
    /*
     * XXX FN NAME ... ';'
     */
    if (NmLexer_Accept(lex, SYM_SEMICOLON)){
      NmDebug_Parser(";\n");
      ret = NmAST_GenFuncDef(name, NULL);
    }
    /*
     * XXX FN NAME ... block
     */
    else {
      NmLexer_Force(lex, SYM_LMUSTASHE);
      NmDebug_Parser("{\n");
      body = stmt(lex);
      ret = NmAST_GenFuncDef(name, body);
      NmLexer_Force(lex, SYM_RMUSTASHE);
      NmDebug_Parser("}\n");
    }
  }
  /*
   * XXX IF stmt stmt
   */
  else if (NmLexer_Accept(lex, SYM_IF)){
    NmDebug_Parser("if ");
    guard = stmt(lex);
    body = stmt(lex);
    /*
     * XXX IF stmt stmt ELSE stmt
     */
    if (NmLexer_Accept(lex, SYM_ELSE)){
      NmDebug_Parser("else ");
      elsee = stmt(lex);
    }
    ret = NmAST_GenIf(guard, body, elsee);
  }
  /*
   * XXX WHILE stmt stmt
   */
  else if (NmLexer_Accept(lex, SYM_WHILE)){
    NmDebug_Parser("while ");
    guard = stmt(lex);
    body = stmt(lex);
    /*
     * XXX WHILE stmt stmt ELSE stmt
     */
    if (NmLexer_Accept(lex, SYM_ELSE)){
      NmDebug_Parser("else ");
      elsee = stmt(lex);
    }
    ret = NmAST_GenWhile(guard, body, elsee);
  }
  /*
   * XXX '{' block '}'
   */
  else if (NmLexer_Accept(lex, SYM_LMUSTASHE)){
    NmDebug_Parser("{\n");
    ret = block(lex);
    NmLexer_Force(lex, SYM_RMUSTASHE);
    NmDebug_Parser("}\n");
  } else {
    body = ret = expr(lex);
    /*
     * XXX expr IF stmt
     */
    if (NmLexer_Accept(lex, SYM_IF)){
      NmDebug_Parser("if ");
      guard = stmt(lex);
      ret = NmAST_GenIf(ret, guard, NULL);
    }
    /*
     * XXX expr WHILE stmt
     */
    else if (NmLexer_Accept(lex, SYM_WHILE)){
      NmDebug_Parser("while ");
      guard = stmt(lex);
      ret = NmAST_GenWhile(ret, guard, NULL);
    }
    /*
     * XXX expr ';'
     */
    else endStmt(lex);
  }

  NmDebug_AST(ret, "create statement");

  return ret;
}

/*
 * block: [stmt]*
 *      ;
 */
static Node *block(LexerState *lex)
{
  Node *new_block = NmMem_Malloc(sizeof(Node));

  new_block->type = NT_BLOCK;
  new_block->data.block.head = NULL;
  new_block->data.block.tail = NULL;

  while (!NmLexer_Peek(lex, SYM_RMUSTASHE) && !NmLexer_Peek(lex, SYM_EOS)){
    Statement *new_stmt = NmMem_Malloc(sizeof(Statement));
    new_stmt->stmt = stmt(lex);
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

  NmDebug_AST(new_block, "create block node");

  return new_block;
}

/*
 * @name - NmParser_ParseFile
 * @desc - parse file of a given <fname> and return a pointer to the node of a
 *         block that was parsed, the main block of the whole script
 * @return - {Node *} of .type = NT_BLOCK
 */
Node *NmParser_ParseFile(char *fname)
{
  Node *nodest = NULL;
  LexerState lex;
  lex.is_file = TRUE;
  lex.source = fname;
  NmLexer_LexFile(&lex, fname);
  nodest = block(&lex);
  NmLexer_Destroy(&lex);

  return nodest;
}

/*
 * @name - NmParser_ParseString
 * @desc - parse the given <string> and return a pointer to the node of a
 *         block that was parsed, the main block of the script
 *         (it probably should return something else)
 * @return - { Node * } of .type = NT_BLOCK
 */
Node *NmParser_ParseString(char *string)
{
  Node *nodest = NULL;
  LexerState lex;
  lex.is_file = FALSE;
  lex.source = string;
  NmLexer_LexString(&lex, string);
  nodest = block(&lex);
  NmLexer_Destroy(&lex);

  return nodest;
}

