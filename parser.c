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

/* <n> is of type { Node * } */
#define isLiteral(n) (n->type == NT_INTEGER || \
                      n->type == NT_FLOAT   || \
                      n->type == NT_STRING)

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
static Node **params_list(LexerState *lex);

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
  Node **arr_inside = NULL;

  /*
   * XXX INTEGER
   */
  if (NmLexer_Peek(lex, SYM_INTEGER)){
    new = NmAST_GenInt(lex->current->sym.pos, lex->current->sym.value.i);
    NmDebug_Parser("%d ", new->data.i);
    NmLexer_Skip(lex);
  }
  /*
   * XXX FLOAT
   */
  else if (NmLexer_Peek(lex, SYM_FLOAT)){
    new = NmAST_GenFloat(lex->current->sym.pos, lex->current->sym.value.f);
    NmDebug_Parser("%f ", new->data.f);
    NmLexer_Skip(lex);
  }
  /*
   * XXX STRING
   */
  else if (NmLexer_Peek(lex, SYM_STRING)){
    new = NmAST_GenString(lex->current->sym.pos, lex->current->sym.value.s);
    NmDebug_Parser("\"%s\" ", new->data.s);
    NmLexer_Skip(lex);
  }
  /*
   * XXX NAME
   */
  else if (NmLexer_Peek(lex, SYM_NAME)){
    new = NmAST_GenName(lex->current->sym.pos, lex->current->sym.value.s);
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
   * XXX '[' params_list ']'
   */
  else if (NmLexer_Accept(lex, SYM_LBRACKET)){
    NmDebug_Parser("[");
    arr_inside = params_list(lex);
    new = NmAST_GenArray(lex->current->sym.pos, arr_inside);
    NmLexer_Force(lex, SYM_RBRACKET);
    NmDebug_Parser("]");
  }
  /*
   * XXX EOS
   */
  else if (NmLexer_Accept(lex, SYM_EOS)){
    return NULL;
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
    NmDebug_Parser(", ");
    Node *next_expr = expr(lex);
    /*
     * Always make the array be one element bigger than supposed to, so the last
     * element is NULL, so traversing is easy
     */
    if (counter + 1 > nmemb - 1){
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
 *             | NAME '[' expr ']'
 *             | primary_expr postfix_op
 *             ;
 */
static Node *postfix_expr(LexerState *lex)
{
  Node *target = NULL;
  Node *ret = target = primary_expr(lex);
  Node **params;
  Node *index;
  char *name = NULL;

  /* I'm not checking if target is NULL because parenthesis, brackets, ++ and --
   * could belong to some prefix unary operation or just an expression */

  /*
   * XXX NAME '(' [params_list] ')'
   */
  if (NmLexer_Accept(lex, SYM_LPAREN)){
    /* store the function's name position */
    Pos pos = lex->current->prev->prev->sym.pos;
    if (target->type != NT_NAME){
      NmError_Lex(lex, "expected a name for a function call, not %s", symToS(lex->current->sym.type));
      /* FIXME */
      exit(EXIT_FAILURE);
    }
    name = target->data.s;
    NmDebug_Parser("(");
    params = params_list(lex);
    NmLexer_Force(lex, SYM_RPAREN);
    NmDebug_Parser(")");
    /* here, lex->current is the closing paren, so we have to use the stored
     * position */
    ret = NmAST_GenCall(pos, name, params);
    /* we only need the char *name, not the whole node, so free it */
    NmAST_Free(target);
  }
  /*
   * XXX NAME '[' expr ']'
   */
  else if (NmLexer_Accept(lex, SYM_LBRACKET)){
    /* store the position of the left bracket */
    Pos pos = lex->current->prev->sym.pos;
    if (target->type != NT_NAME && target->type != NT_STRING){
      NmError_Lex(lex, "expected a name or a string for an indexing, not %s", symToS(lex->current->sym.type));
      /* FIXME */
      exit(EXIT_FAILURE);
    }
    NmDebug_Parser("[");
    index = expr(lex);
    NmLexer_Force(lex, SYM_RBRACKET);
    NmDebug_Parser("]");
    ret = NmAST_GenBinop(pos, target, BINARY_INDEX, index);
  }
  /*
   * XXX NAME '++'
   */
  else if (NmLexer_Accept(lex, SYM_PLUSPLUS)){
    if (isLiteral(target)){
      /* using NmError_Error not NmError_Lex because lexer's state has gone
       * further in 'primary_expr' */
      NmError_Error("can't do the postfix increment on a literal in line %u at column %u", lex->current->prev->sym.pos.line, lex->current->prev->sym.pos.column);
      /* FIXME: shouldn't exit here */
      exit(EXIT_FAILURE);
    }
    NmDebug_Parser(":postfix++");
    ret = NmAST_GenUnop(lex->current->sym.pos, target, UNARY_POSTINC);
  }
  /*
   * XXX NAME '--'
   */
  else if (NmLexer_Accept(lex, SYM_MINUSMINUS)){
    if (isLiteral(target)){
      /* using NmError_Error not NmError_Lex because lexer's state has gone
       * further in 'primary_expr' */
      NmError_Error("can't do the postfix increment on a literal in line %u at column %u", lex->current->prev->sym.pos.line, lex->current->prev->sym.pos.column);
      /* FIXME: shouldn't exit here */
      exit(EXIT_FAILURE);
    }
    NmDebug_Parser(":postfix--");
    ret = NmAST_GenUnop(lex->current->sym.pos, target, UNARY_POSTDEC);
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
    /* if target is NULL it means something like that happend:
     *
     *    my var;
     *    var = !;
     *
     */
    if (!target){
      NmError_Lex(lex, "expected an expression for the unary negation");
      /* FIXME: probably shouldn't exit here */
      exit(EXIT_FAILURE);
    }
    ret = NmAST_GenUnop(lex->current->sym.pos, target, UNARY_NEGATE);
  }
  else if (NmLexer_Accept(lex, SYM_PLUS)){
    NmDebug_Parser("unary+ ");
    target = prefix_expr(lex);
    /* if target is NULL it means something like that happend:
     *
     *    my var;
     *    var = +;
     *
     */
    if (!target){
      NmError_Lex(lex, "expected an expression for the unary plus");
      /* FIXME: probably shouldn't exit here */
      exit(EXIT_FAILURE);
    }
    ret = NmAST_GenUnop(lex->current->sym.pos, target, UNARY_PLUS);
  }
  else if (NmLexer_Accept(lex, SYM_MINUS)){
    NmDebug_Parser("unary- ");
    target = prefix_expr(lex);
    /* if target is NULL it means something like that happend:
     *
     *    my var;
     *    var = -;
     *
     */
    if (!target){
      NmError_Lex(lex, "expected an expression for the unary minus");
      /* FIXME: probably shouldn't exit here */
      exit(EXIT_FAILURE);
    }
    ret = NmAST_GenUnop(lex->current->sym.pos, target, UNARY_MINUS);
  }
  else if (NmLexer_Accept(lex, SYM_PLUSPLUS)){
    NmDebug_Parser("prefix++:");
    target = prefix_expr(lex);
    /* if target is NULL it means something like that happend:
     *
     *    my var;
     *    var = ++;
     *
     */
    if (!target){
      NmError_Lex(lex, "expected an expression for the prefix incrementation");
      /* FIXME: probably shouldn't exit here */
      exit(EXIT_FAILURE);
    }
    ret = NmAST_GenUnop(lex->current->sym.pos, target, UNARY_PREINC);
  }
  else if (NmLexer_Accept(lex, SYM_MINUSMINUS)){
    NmDebug_Parser("prefix--:");
    target = prefix_expr(lex);
    /* if target is NULL it means something like that happend:
     *
     *    my var;
     *    var = --;
     *
     */
    if (!target){
      NmError_Lex(lex, "expected an expression for the prefix decrementation");
      /* FIXME: probably shouldn't exit here */
      exit(EXIT_FAILURE);
    }
    ret = NmAST_GenUnop(lex->current->sym.pos, target, UNARY_PREDEC);
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
    /* if left is NULL it means something like that happend:
     *
     *    my var;
     *    var = * 2;
     *
     */
    if (!left){
      NmError_Lex(lex, "expected an expression for the lhs of the binary %s operation", symToS(lex->current->sym.type));
      /* FIXME: probably shouldn't exit here */
      exit(EXIT_FAILURE);
    }
    if (NmLexer_Accept(lex, SYM_TIMES)){
      op = BINARY_MUL;
      NmDebug_Parser("* ");
    } else if (NmLexer_Accept(lex, SYM_SLASH)){
      op = BINARY_DIV;
      NmDebug_Parser("/ ");
    }
    right = prefix_expr(lex);
    /* if right is NULL it means something like that happend:
     *
     *    my var;
     *    var = 2 * ;
     *
     */
    if (!right){
      NmError_Lex(lex, "expected an expression for the rhs of the binary %s operation", binopToS(op));
      /* FIXME: probably shouldn't exit here */
      exit(EXIT_FAILURE);
    }
    ret = NmAST_GenBinop(lex->current->sym.pos, left, BINARY_MUL, right);
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
    /* we are not checking if letf is NULL here, because + and - are also a
     * prefix unary operations */
    if (NmLexer_Accept(lex, SYM_PLUS)){
      op = BINARY_ADD;
      NmDebug_Parser("+ ");
    } else if (NmLexer_Accept(lex, SYM_MINUS)){
      op = BINARY_SUB;
      NmDebug_Parser("- ");
    }
    right = mult_expr(lex);
    /* if right is NULL it means something like that happend:
     *
     *    my var;
     *    var = 2 + ;
     *
     */
    if (!right){
      NmError_Lex(lex, "expected an expression for the rhs of the binary %s operation", binopToS(op));
      /* FIXME: probably shouldn't exit here */
      exit(EXIT_FAILURE);
    }
    ret = NmAST_GenBinop(lex->current->sym.pos, left, op, right);
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
    /* if left is NULL it means something like that happend:
     *
     *    my var;
     *    var = > 2;
     *
     */
    if (!left){
      NmError_Lex(lex, "expected an expression for the lhs of the binary %s operation", symToS(lex->current->sym.type));
      /* FIXME: probably shouldn't exit here */
      exit(EXIT_FAILURE);
    }
    if (NmLexer_Accept(lex, SYM_GT)){
      op = BINARY_GT;
      NmDebug_Parser("> ");
    } else if (NmLexer_Accept(lex, SYM_LT)){
      op = BINARY_LT;
      NmDebug_Parser("< ");
    }
    right = add_expr(lex);
    /* if right is NULL it means something like that happend:
     *
     *    my var;
     *    var = 2 > ;
     *
     */
    if (!right){
      NmError_Lex(lex, "expected an expression for the rhs of the binary %s operation", binopToS(op));
      /* FIXME: probably shouldn't exit here */
      exit(EXIT_FAILURE);
    }
    ret = NmAST_GenBinop(lex->current->sym.pos, left, op, right);
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
    /* if left is NULL it means something like that happend:
     *
     *    my var;
     *    = 2;
     *
     */
    if (!left){
      NmError_Lex(lex, "expected an expression for the lhs of the binary %s operation", symToS(lex->current->sym.type));
      /* FIXME: probably shouldn't exit here */
      exit(EXIT_FAILURE);
    }
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
    /* if right is NULL it means something like that happend:
     *
     *    my var;
     *    var = ;
     *
     */
    if (!right){
      NmError_Lex(lex, "expected an expression");
      /* FIXME: probably shouldn't exit here */
      exit(EXIT_FAILURE);
    }
    ret = NmAST_GenBinop(lex->current->sym.pos, left, op, right);
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
    uint8_t flags = 0;
    NmDebug_Parser("my ");
    if (NmLexer_Accept(lex, SYM_CONST)){
      flags |= (1 << NMVAR_FLAG_CONST);
    }
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
    /* if value is NULL, then something like this happend:
     *
     *    my var = ;
     *
     */
    if (!value){
      NmError_Lex(lex, "nothing was initialized");
    }
    ret = NmAST_GenDecl(lex->current->sym.pos, name, value, flags);
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
    ret = NmAST_GenCall(lex->current->sym.pos, "print", params);
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
 *     | INLCLUDE NAME ';'
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
    ret = NmAST_GenNop(lex->current->sym.pos);
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
    if (strcmp(name, lex->source)){
      Nm_UseModule(tmp);
      ret = NmAST_GenInt(lex->current->prev->prev->sym.pos, 1);
    }
    endStmt(lex);
    NmMem_Free(name);
  }
  /*
   * XXX INCLUDE NAME ';'
   */
  else if (NmLexer_Accept(lex, SYM_INCLUDE)){
    char *tmp;
    NmDebug_Parser("include ");
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
    if (strcmp(name, lex->source)){
      Nm_IncludeModule(tmp);
      ret = NmAST_GenInt(lex->current->prev->prev->sym.pos, 1);
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
      ret = NmAST_GenFuncDef(lex->current->sym.pos, name, NULL);
    }
    /*
     * XXX FN NAME ... block
     */
    else {
      NmLexer_Force(lex, SYM_LMUSTASHE);
      NmDebug_Parser("{\n");
      body = stmt(lex);
      ret = NmAST_GenFuncDef(lex->current->sym.pos, name, body);
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
    ret = NmAST_GenIf(lex->current->sym.pos, guard, body, elsee);
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
    ret = NmAST_GenWhile(lex->current->sym.pos, guard, body, elsee);
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
      ret = NmAST_GenIf(lex->current->sym.pos, ret, guard, NULL);
    }
    /*
     * XXX expr WHILE stmt
     */
    else if (NmLexer_Accept(lex, SYM_WHILE)){
      NmDebug_Parser("while ");
      guard = stmt(lex);
      ret = NmAST_GenWhile(lex->current->sym.pos, ret, guard, NULL);
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

