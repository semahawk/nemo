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
  NmDebug_Parser("ENDSTMT;\n"); \
} while (0);

/* forward */
static Node *expr(LexerState *lex);
static Node *assign_expr(LexerState *lex);
static Node *block(LexerState *lex);
static Node **params_list(LexerState *lex, int num);

/* pointer to the previous statement */
static Node *prev_stmt = NULL;

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
  char *name = NULL;

  /*
   * XXX INTEGER
   */
  if (NmLexer_Peek(lex, SYM_INTEGER)){
    new = NmAST_GenInt(lex->current.pos, lex->current.value.i);
    NmDebug_Parser("%d ", ((Node_Int *)new)->i);
    NmLexer_Skip(lex);
  }
  /*
   * XXX FLOAT
   */
  else if (NmLexer_Peek(lex, SYM_FLOAT)){
    new = NmAST_GenFloat(lex->current.pos, lex->current.value.f);
    NmDebug_Parser("%f ", ((Node_Float *)new)->f);
    NmLexer_Skip(lex);
  }
  /*
   * XXX STRING
   */
  else if (NmLexer_Peek(lex, SYM_STRING)){
    new = NmAST_GenString(lex->current.pos, lex->current.value.s);
    NmDebug_Parser("\"%s\" ", ((Node_String *)new)->s);
    NmLexer_Skip(lex);
  }
  /*
   * XXX NAME
   */
  else if (NmLexer_Peek(lex, SYM_NAME)){
    int argc;
    unsigned optc;
    char *opts;
    CFunc *cfunc;
    Func *func;
    Namespace *namespace = NmNamespace_GetCurr();
    Node **params = NULL;
    Symbol namesym = NmLexer_Force(lex, SYM_NAME);
    name = namesym.value.s;
    bool isafunc = false;
    /* it could be a function's name, so let's check it out */
    /* search the C functions */
    for (CFuncsList *cfuncs = namespace->cfuncs; cfuncs != NULL; cfuncs = cfuncs->next){
      if (!strcmp(cfuncs->func->name, name)){
        cfunc = cfuncs->func;
        argc = cfunc->argc;
        optc = strlen(cfunc->opts);
        opts = cfunc->opts;
        isafunc = true;
        break;
      }
    }
    /* search the user defined functions */
    for (FuncsList *funcs = namespace->funcs; funcs != NULL; funcs = funcs->next){
      if (!strcmp(funcs->func->name, name)){
        func = funcs->func;
        argc = func->argc;
        optc = strlen(func->opts);
        opts = func->opts;
        isafunc = true;
        break;
      }
    }

    NmDebug_Parser("%s ", name);

    if (isafunc){
      /* options are stored in a char array, for instance this:
       *
       *   function -e -g -p 6;
       *
       * would have options translated to
       *
       *   { 'e', 'g', 'p', '\0' }
       *
       */
      /* C99 ROCKS */
      char call_opts[optc + 1];
      /* this guard makes things like:
       *
       *   five-4
       *
       * possible (given that "five" is a function returning 5 and not taking
       * any arguments or options).
       * Although, if it was taking one argument, -4 would be given to it */
      if (optc > 0){
        lex->right_after_funname = true;
        if (NmLexer_Peek(lex, SYM_OPT)){
          Symbol optsym = NmLexer_Force(lex, SYM_OPT);
          strcpy(call_opts, optsym.value.s);
          NmDebug_Parser("-%s ", call_opts);
          /* check if not too many options were given */
          if (strlen(call_opts) > optc){
            NmError_Lex(lex, "too many options given for the function '%s', supported options are '%s'", name, opts);
            Nm_Exit();
            return NULL;
          }
          /* check if the function supports given options */
          for (unsigned i = 0; i < strlen(call_opts); i++){
            if (!strchr(opts, call_opts[i])){
              NmError_Lex(lex, "function '%s' doesn't support the '%c' option", name, call_opts[i]);
              Nm_Exit();
              return NULL;
            }
          }
        }
        lex->right_after_funname = false;
      }
      NmDebug_Parser("(");
      lex->right_after_funname = false;
      if (NmLexer_Peek(lex, SYM_LPAREN)){
        /* FIXME: if there are parenthesis there mustn't be more parameters
         *        passed than requested */
        NmLexer_Skip(lex);
        params = params_list(lex, argc);
        NmLexer_Force(lex, SYM_RPAREN);
      } else {
        params = params_list(lex, argc);
      }
      NmDebug_Parser(")");
      /* if 'params' returns NULL it means something bad happend */
      if (!params){
        NmError_Lex(lex, "wrong number of arguments for function '%s' %s", name, NmError_GetCurr());
        Nm_Exit();
        return NULL;
      }
      new = NmAST_GenCall(lex->current.pos, name, params, call_opts);
    } else {
      new = NmAST_GenName(lex->current.pos, name);
    }
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
    arr_inside = params_list(lex, -1);
    new = NmAST_GenArray(lex->current.pos, arr_inside);
    NmLexer_Force(lex, SYM_RBRACKET);
    NmDebug_Parser("]");
  }
  /*
   * XXX EOS
   */
  else if (NmLexer_Accept(lex, SYM_EOS)){
    lex->eos = true;
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
/*
 * This function fetches the parameters list, which are no more than <num>.
 *
 * If <num> is negative, it will take as many as possible.
 */
static Node **params_list(LexerState *lex, int num)
{
  unsigned nmemb = 5;
  unsigned counter = 0;
  Node **params = NmMem_Calloc(nmemb, sizeof(Node));
  /* using assign_expr instead of expr because comma_expr would screw things up
   * here*/
  Node *first_expr = assign_expr(lex);

  if (num == 0){
    if (first_expr){
      NmError_SetString("(1 when 0 expected)");
      NmAST_Free(first_expr);
      NmMem_Free(params);
      return NULL;
    } else {
      return params;
    }
  }
  /* Nasty hack */
  else if (num < 0){
    /* If anyone overcomes this, is an idiot.
     *
     * I don't want idiots using this language.
     */
    num = 1 << 15;
  }

  NmDebug_AST(params, "create params list");

  /* if 'first_expr' is NULL it means no params were fetched at all */
  if (!first_expr && num != 1 << 15){
    NmError_SetString("(0 when %d expected)", (unsigned)num);
    NmMem_Free(params);
    return NULL;
  }

  params[counter++] = first_expr;

  if (num == 1){
    return params;
  }

  while (NmLexer_Accept(lex, SYM_COMMA) && counter <= (unsigned)num){
    NmDebug_Parser(", ");
    Node *next_expr = assign_expr(lex);
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

  return params;
}

/*
 *   postfix_op: '++'
 *             | '--'
 *             ;
 *
 * postfix_expr: NAME '[' expr ']'
 *             | primary_expr postfix_op
 *             ;
 */
static Node *postfix_expr(LexerState *lex)
{
  Node *target = NULL;
  Node *ret = target = primary_expr(lex);
  Node *index;

  /* I'm not checking if target is NULL because parenthesis, brackets, ++ and --
   * could belong to some prefix unary operation or just an expression */

  /*
   * XXX NAME '[' expr ']'
   */
  if (NmLexer_Accept(lex, SYM_LBRACKET)){
    /* TODO: store the position of the left bracket
     *       right now it's the position of the very first thing after the left
     *       bracket */
    Pos pos = lex->current.pos;
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
      /* FIXME: store the target's position */
      NmError_Error("can't do the postfix increment on a literal in line %u at column %u", lex->current.pos.line, lex->current.pos.column);
      Nm_Exit();
      return NULL;
    }
    NmDebug_Parser(":postfix++");
    ret = NmAST_GenUnop(lex->current.pos, target, UNARY_POSTINC);
  }
  /*
   * XXX NAME '--'
   */
  else if (NmLexer_Accept(lex, SYM_MINUSMINUS)){
    if (isLiteral(target)){
      /* using NmError_Error not NmError_Lex because lexer's state has gone
       * further in 'primary_expr' */
      /* FIXME: store the target's position */
      NmError_Error("can't do the postfix increment on a literal in line %u at column %u", lex->current.pos.line, lex->current.pos.column);
      Nm_Exit();
      return NULL;
    }
    NmDebug_Parser(":postfix--");
    ret = NmAST_GenUnop(lex->current.pos, target, UNARY_POSTDEC);
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
      Nm_Exit();
      return NULL;
    }
    ret = NmAST_GenUnop(lex->current.pos, target, UNARY_NEGATE);
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
      Nm_Exit();
      return NULL;
    }
    ret = NmAST_GenUnop(lex->current.pos, target, UNARY_PLUS);
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
      Nm_Exit();
      return NULL;
    }
    ret = NmAST_GenUnop(lex->current.pos, target, UNARY_MINUS);
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
      Nm_Exit();
      return NULL;
    }
    ret = NmAST_GenUnop(lex->current.pos, target, UNARY_PREINC);
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
      Nm_Exit();
      return NULL;
    }
    ret = NmAST_GenUnop(lex->current.pos, target, UNARY_PREDEC);
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

  while (NmLexer_Peek(lex, SYM_TIMES) ||
         NmLexer_Peek(lex, SYM_SLASH) ||
         NmLexer_Peek(lex, SYM_PERCENT)){
    /* if left is NULL it means something like that happend:
     *
     *    my var;
     *    var = * 2;
     *
     */
    if (!left){
      NmError_Lex(lex, "expected an expression for the lhs of the binary %s operation", symToS(lex->current.type));
      Nm_Exit();
      return NULL;
    }
    if (NmLexer_Accept(lex, SYM_TIMES)){
      op = BINARY_MUL;
      NmDebug_Parser("* ");
    } else if (NmLexer_Accept(lex, SYM_SLASH)){
      op = BINARY_DIV;
      NmDebug_Parser("/ ");
    } else if (NmLexer_Accept(lex, SYM_PERCENT)){
      op = BINARY_MOD;
      NmDebug_Parser("% ");
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
      Nm_Exit();
      return NULL;
    }
    ret = NmAST_GenBinop(lex->current.pos, left, op, right);
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
      Nm_Exit();
      return NULL;
    }
    ret = NmAST_GenBinop(lex->current.pos, left, op, right);
    left = ret;
  }

  return ret;
}

/*
 *   cond_op: '>'
 *          | '<'
 *          | '>='
 *          | '<='
 *          | '=='
 *          | '!='
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

  while (NmLexer_Peek(lex, SYM_RCHEVRON) ||
         NmLexer_Peek(lex, SYM_LCHEVRON) ||
         NmLexer_Peek(lex, SYM_RCHEVRONEQ) ||
         NmLexer_Peek(lex, SYM_LCHEVRONEQ) ||
         NmLexer_Peek(lex, SYM_EQEQ) ||
         NmLexer_Peek(lex, SYM_BANGEQ)){
    /* if left is NULL it means something like that happend:
     *
     *    my var;
     *    var = > 2;
     *
     */
    if (!left){
      NmError_Lex(lex, "expected an expression for the lhs of the binary %s operation", symToS(lex->current.type));
      Nm_Exit();
      return NULL;
    }
    if (NmLexer_Accept(lex, SYM_RCHEVRON)){
      op = BINARY_GT;
      NmDebug_Parser("> ");
    } else if (NmLexer_Accept(lex, SYM_LCHEVRON)){
      op = BINARY_LT;
      NmDebug_Parser("< ");
    } else if (NmLexer_Accept(lex, SYM_RCHEVRONEQ)){
      op = BINARY_GE;
      NmDebug_Parser(">= ");
    } else if (NmLexer_Accept(lex, SYM_LCHEVRONEQ)){
      op = BINARY_LE;
      NmDebug_Parser("<= ");
    } else if (NmLexer_Accept(lex, SYM_EQEQ)){
      op = BINARY_EQ;
      NmDebug_Parser("== ");
    } else if (NmLexer_Accept(lex, SYM_BANGEQ)){
      op = BINARY_NE;
      NmDebug_Parser("!= ");
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
      Nm_Exit();
      return NULL;
    }
    ret = NmAST_GenBinop(lex->current.pos, left, op, right);
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
      NmError_Lex(lex, "expected an expression for the lhs of the binary %s operation", symToS(lex->current.type));
      Nm_Exit();
      return NULL;
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
      Nm_Exit();
      return NULL;
    }
    ret = NmAST_GenBinop(lex->current.pos, left, op, right);
    left = ret;
  }

  return ret;
}

/*
 *   comma_op: ','
 *           ;
 *
 * comma_expr: assign_expr [comma_op assign_expr]*
 *            ;
 */
static Node *comma_expr(LexerState *lex)
{
  Node *ret;
  Node *left;
  Node *right;

  ret = left = assign_expr(lex);

  while (NmLexer_Accept(lex, SYM_COMMA)){
    /* if left is NULL it means something like that happend:
     *
     *    my var;
     *    , 2;
     *
     */
    if (!left){
      NmError_Lex(lex, "expected an expression for the lhs of the binary ',' operation");
      Nm_Exit();
      return NULL;
    }
    NmDebug_Parser(", ");
    right = assign_expr(lex);
    /* if right is NULL it means something like that happend:
     *
     *    my var;
     *    var = 2, ;
     *
     */
    if (!right){
      NmError_Lex(lex, "expected an expression for the rhs of the binary ',' operation");
      Nm_Exit();
      return NULL;
    }
    ret = NmAST_GenBinop(lex->current.pos, left, BINARY_COMMA, right);
    left = ret;
  }

  return ret;
}

/*
 * expr: comma_expr
 *     ;
 */
static Node *expr(LexerState *lex)
{
  return comma_expr(lex);
}

/*
 * stmt: ';'
 *     | '{' block '}'
 *     | MY NAME '=' expr ';'
 *     | label
 *     | GOTO NAME ';'
 *     | function_prototype
 *     | USE NAME ';'
 *     | INLCLUDE NAME ';'
 *     | IF stmt stmt
 *     | UNLESS stmt stmt
 *     | WHILE stmt stmt
 *     | UNTIL stmt stmt
 *     | expr IF stmt
 *     | expr UNLESS stmt
 *     | expr WHILE stmt
 *     | expr UNTIL stmt
 *     | expr ';'
 *     ;
 *
 * label: NAME ':' stmt
 *      ;
 *
 * function_prototype: FUN NAME [OPT|NAME]* ';'
 *                   | FUN NAME [OPT|NAME]* block
 *                   ;
 */
static Node *stmt(LexerState *lex)
{
  Node *ret   = NULL;
  Node *guard = NULL;
  Node *body  = NULL;
  Node *elsee = NULL;
  Node *value = NULL;
  char *name  = NULL;
  char *path  = NULL;
  Pos savedpos = lex->current.pos;

  /*
   * XXX ';'
   */
  if (NmLexer_Accept(lex, SYM_SEMICOLON)){
    /* that's NOP */
    NmDebug_Parser("NOP;\n");
    ret = NmAST_GenNop(lex->current.pos);
  }
  /*
   * XXX MY
   */
  else if (NmLexer_Accept(lex, SYM_MY)){
    uint8_t flags = 0;
    NmDebug_Parser("my ");
    if (NmLexer_Accept(lex, SYM_CONST)){
      flags |= (1 << NMVAR_FLAG_CONST);
    }
    Symbol namesym = NmLexer_Force(lex, SYM_NAME);
    /* NmLexer_Force skips the symbol so we have to get to the previous one */
    name = namesym.value.s;
    NmDebug_Parser("%s", name);
    /*
     * XXX MY NAME = expr
     */
    if (NmLexer_Accept(lex, SYM_EQ)){
      NmDebug_Parser(" = ");
      value = expr(lex);
      /* if value is NULL, then something like this happend:
       *
       *    my var = ;
       *
       */
      if (!value){
        NmError_Lex(lex, "nothing was initialized");
      }
    }
    ret = NmAST_GenDecl(lex->current.pos, name, value, flags);
    endStmt(lex);
  }
  /*
   * XXX NAME ':' stmt
   *
   *     basically just a label
   */
  /*else if (lex->current.type == SYM_NAME &&*/
           /*NmLexer_Peek(lex, SYM_COLON)){*/
    /*name = lex->current.value.s;*/
    /*NmDebug_Parser("%s", name);*/
    /*NmLexer_Skip(lex);*/
    /*NmDebug_Parser(":", name);*/
    /*NmLexer_Skip(lex);*/
    /*body = stmt(lex);*/
    /*ret = body;*/
    /*NmNamespace_NewLabel(name, body);*/
  /*}*/
  /*
   * XXX GOTO NAME ';'
   */
  else if (NmLexer_Accept(lex, SYM_GOTO)){
    NmDebug_Parser("goto ");
    Symbol namesym = NmLexer_Force(lex, SYM_NAME);
    name = namesym.value.s;
    NmDebug_Parser("%s", name);
    ret = NmAST_GenNop(namesym.pos);
    ret->next = NmNamespace_GetLabel(name);
    /* NmNamespace_GetLabel returns NULL if didn't find the label */
    if (!ret->next){
      NmError_Parser(ret, "label '%s' was not found", name);
      Nm_Exit();
      ret = NULL;
    }
    endStmt(lex);
  }
  /*
   * XXX USE NAME ';'
   * XXX INCLUDE NAME ';'
   */
  else if (NmLexer_Peek(lex, SYM_USE) ||
           NmLexer_Peek(lex, SYM_INCLUDE)){
    bool use;
    Pos savepos = lex->current.pos;
    if (NmLexer_Accept(lex, SYM_USE)){
      use = true;
      NmDebug_Parser("use ");
    } else {
      use = false;
      NmDebug_Parser("include ");
      NmLexer_Skip(lex);
    }
    Symbol namesym = NmLexer_Force(lex, SYM_NAME);
    name = namesym.value.s;
    NmDebug_Parser("%s ", name);
    /*
     * XXX USE NAME PATH ';'
     * XXX INCLUDE NAME PATH ';'
     */
    if (NmLexer_Peek(lex, SYM_STRING)){
      path = lex->current.value.s;
      NmLexer_Skip(lex);
    } else {
      path = NULL;
    }
    ret = NmAST_GenInclude(savepos, name, path, use);
    endStmt(lex);
  }
  /*
   * XXX FUN NAME [OPT|NAME]* ';'
   * XXX FUN NAME [OPT|NAME]* block
   */
  else if (NmLexer_Accept(lex, SYM_FUN)){
    lex->right_after_fun = true;
    unsigned argc = 0;
    unsigned optc = 0;
    char **argv = NmMem_Malloc(sizeof(char *) * 1);
    char *optv = NmMem_Malloc(sizeof(char) * 1 + 1); /* I know sizeof(char) is 1 */
    /* FIXME: store the "fun" position */
    Pos pos = lex->current.pos;
    NmDebug_Parser("fun ");
    Symbol namesym = NmLexer_Force(lex, SYM_NAME);
    name = namesym.value.s;
    NmDebug_Parser("%s ", namesym.value.s);
    while (NmLexer_Peek(lex, SYM_NAME) || NmLexer_Peek(lex, SYM_OPT)){
      if (NmLexer_Peek(lex, SYM_NAME)){
        Symbol namesym = NmLexer_Force(lex, SYM_NAME);
        NmDebug_Parser("%s ", namesym.value.s);
        argv = NmMem_Realloc(argv, (argc + 1) * sizeof(char *) + 1);
        argv[argc] = NmMem_Strdup(namesym.value.s);
        argc++;
      } else if (NmLexer_Peek(lex, SYM_OPT)){
        Symbol optsym = NmLexer_Force(lex, SYM_OPT);
        NmDebug_Parser("-%c ", optsym.value.c);
        optv = NmMem_Realloc(optv, (optc + 1) * sizeof(char));
        optv[optc] = optsym.value.c;
        optc++;
      }
    }
    optv[optc] = '\0';

    if (NmLexer_Accept(lex, SYM_SEMICOLON)){
      lex->right_after_fun = false;
      NmDebug_Parser(";\n");
      body = NULL;
    } else {
      NmLexer_Force(lex, SYM_LMUSTASHE);
      lex->right_after_fun = false;
      NmDebug_Parser("{\n");
      body = block(lex);
      NmLexer_Force(lex, SYM_RMUSTASHE);
      NmDebug_Parser("}\n");
    }
    ret = NmAST_GenFuncDef(pos, name, body, argc, optc, argv, optv);
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
    ret = NmAST_GenIf(lex->current.pos, guard, body, elsee, false);
  }
  /*
   * XXX UNLESS stmt stmt
   */
  else if (NmLexer_Accept(lex, SYM_UNLESS)){
    NmDebug_Parser("unless ");
    guard = stmt(lex);
    body = stmt(lex);
    /*
     * XXX UNLESS stmt stmt ELSE stmt
     */
    if (NmLexer_Accept(lex, SYM_ELSE)){
      NmDebug_Parser("else ");
      elsee = stmt(lex);
    }
    ret = NmAST_GenIf(lex->current.pos, guard, body, elsee, true);
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
    ret = NmAST_GenWhile(lex->current.pos, guard, body, elsee, false);
  }
  /*
   * XXX UNTIL stmt stmt
   */
  else if (NmLexer_Accept(lex, SYM_UNTIL)){
    NmDebug_Parser("until ");
    guard = stmt(lex);
    body = stmt(lex);
    /*
     * XXX UNTIL stmt stmt ELSE stmt
     */
    if (NmLexer_Accept(lex, SYM_ELSE)){
      NmDebug_Parser("else ");
      elsee = stmt(lex);
    }
    ret = NmAST_GenWhile(lex->current.pos, guard, body, elsee, true);
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
      ret = NmAST_GenIf(lex->current.pos, ret, guard, NULL, false);
    }
    /*
     * XXX expr UNLESS stmt
     */
    else if (NmLexer_Accept(lex, SYM_UNLESS)){
      NmDebug_Parser("unless ");
      guard = stmt(lex);
      ret = NmAST_GenIf(lex->current.pos, ret, guard, NULL, true);
    }
    /*
     * XXX expr WHILE stmt
     */
    else if (NmLexer_Accept(lex, SYM_WHILE)){
      NmDebug_Parser("while ");
      guard = stmt(lex);
      ret = NmAST_GenWhile(lex->current.pos, ret, guard, NULL, false);
    }
    /*
     * XXX expr UNTIL stmt
     */
    else if (NmLexer_Accept(lex, SYM_UNTIL)){
      NmDebug_Parser("until ");
      guard = stmt(lex);
      ret = NmAST_GenWhile(lex->current.pos, ret, guard, NULL, true);
    }
    /*
     * XXX expr ';'
     */
    else if (!lex->eos) endStmt(lex);
  }

  /* this logic probably could've been done better */
  if (lex->eos && !ret){
    return NULL;
  } else {
    /* if ret is NULL right here, it means there was some error above
     * so let's just gently do nothing */
    if (!ret && !lex->eos)
      ret = NmAST_GenNop(savedpos);

    ret = NmAST_GenStmt(ret->pos, ret);
  }

  /* set the previous statement's "next", but only if it's previous value is
   * different than NULL */
  if (prev_stmt)
    if (prev_stmt->next == NULL)
      prev_stmt->next = ret;

  prev_stmt = ret;

  return ret;
}

/*
 * block: [stmt]*
 *      ;
 */
static Node *block(LexerState *lex)
{
  Node_Block *new_block = NmMem_Malloc(sizeof(Node_Block));

  new_block->type = NT_BLOCK;
  new_block->head = NULL;
  new_block->tail = NULL;

  NmDebug_AST(new_block, "create block node");

  while (!NmLexer_Peek(lex, SYM_RMUSTASHE) && !NmLexer_Peek(lex, SYM_EOS)){
    Statement *new_stmt = NmMem_Malloc(sizeof(Statement));
    new_stmt->stmt = stmt(lex);
    if (new_stmt->stmt == NULL){
      NmMem_Free(new_stmt);
      break;
    }
    /* append that statement to the statements of the block */
    /*   the list is empty */
    if (!new_block->head && !new_block->tail){
      new_stmt->next = new_block->head;
      new_stmt->prev = new_block->tail;
      new_block->head = new_stmt;
      new_block->tail = new_stmt;
    /*   the list is NOT empty */
    } else {
      new_stmt->next = new_block->head->next;
      new_block->head->next = new_stmt;
      new_stmt->prev = new_block->head;
      new_block->head = new_stmt;
    }
  }

  /* set the previous statement's "next", but only if it's previous value is
   * different than NULL */
  if (prev_stmt)
    if (prev_stmt->next == NULL)
      prev_stmt->next = (Node *)new_block;

  return (Node *)new_block;
}

/* <lex> is of type { LexerState } */
#define lexinitrest(lex) \
  lex.line = 1; \
  lex.column = 1; \
  lex.saveline = 1; \
  lex.savecolumn = 1; \
  lex.current.pos.line  = 1; \
  lex.current.pos.column = 1; \
  lex.eos = false; \
  lex.right_after_fun = false; \
  lex.right_after_funname = false; \
  lex.gc_head = NULL;

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
  FILE *fp;
  char *fbuffer = NULL;
  size_t flen = 0;

  if ((fp = fopen(fname, "r")) == NULL){
    NmError_Fatal("cannot open file '%s': %s", fname, strerror(errno));
    exit(EXIT_FAILURE);
  }
  /* get the files length */
  fseek(fp, 0, SEEK_END);
  flen = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  /* make room for the contents */
  fbuffer = NmMem_Malloc(flen);
  /* store the files contents in the fbuffer */
  if (fread(fbuffer, 1, flen, fp) != flen){
    NmError_Fatal("fread failed in " __FILE__ " at line %d", __LINE__);
    exit(EXIT_FAILURE);
  }
  fbuffer[flen - 1] = '\0';
  /* initialize */
  lex.is_file = true;
  lex.source = fname;
  lex.content = fbuffer;
  lex.savecontent = fbuffer;
  lexinitrest(lex);
  nodest = block(&lex);
  NmLexer_Tidyup(&lex);
  /* free the buffer */
  NmMem_Free(fbuffer);
  /* close the file handle */
  fclose(fp);

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
  lex.is_file = false;
  lex.source = string;
  lex.content = string;
  lex.savecontent = string;
  lexinitrest(lex);
  nodest = block(&lex);
  NmLexer_Tidyup(&lex);

  return nodest;
}

