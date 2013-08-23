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
#if DEBUG
#define endStmt(lex) do { \
    if (!NmLexer_Peek(lex, SYM_LMUSTASHE) && \
        !NmLexer_Peek(lex, SYM_RMUSTASHE) && \
        !NmLexer_Peek(lex, SYM_EOS)){ \
      NmLexer_Force(lex, SYM_SEMICOLON); \
    } \
    NmDebug_Parser("ENDSTMT;\n"); \
  } while (0);
#else
#define endStmt(lex) do { \
    if (!NmLexer_Peek(lex, SYM_LMUSTASHE) && \
        !NmLexer_Peek(lex, SYM_RMUSTASHE) && \
        !NmLexer_Peek(lex, SYM_EOS)){ \
      NmLexer_Force(lex, SYM_SEMICOLON); \
    } \
  } while (0);
#endif

/* forward */
static Node *expr(LexerState *lex);
static Node *assign_expr(LexerState *lex);
static Node *block(LexerState *lex);
static Node **params_list(LexerState *lex, int num, bool inparens);

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
#if DEBUG
    NmDebug_Parser("%d ", ((Node_Int *)new)->i);
#endif
    NmLexer_Skip(lex);
  }
  /*
   * XXX FLOAT
   */
  else if (NmLexer_Peek(lex, SYM_FLOAT)){
    new = NmAST_GenFloat(lex->current.pos, lex->current.value.f);
#if DEBUG
    NmDebug_Parser("%f ", ((Node_Float *)new)->f);
#endif
    NmLexer_Skip(lex);
  }
  /*
   * XXX STRING
   */
  else if (NmLexer_Peek(lex, SYM_STRING)){
    new = NmAST_GenString(lex->current.pos, lex->current.value.s);
#if DEBUG
    NmDebug_Parser("\"%s\" ", ((Node_String *)new)->s);
#endif
    NmLexer_Skip(lex);
  }
  /*
   * XXX NAME
   */
  else if (NmLexer_Peek(lex, SYM_NAME)){
    int argc = 0;
    unsigned optc = 0;
    char *opts = NULL;
    CFunc *cfunc;
    Func *func;
    Namespace *namespace;
    Node **params = NULL;
    Symbol namesym = NmLexer_Force(lex, SYM_NAME);
    name = namesym.value.s;
    bool isafunc = false;
    /* XXX fetch the actual name, eg. when the name here is
     *     "Foo.Bar.baz", only the name "baz" would be retrieved */
    /* position of the last dot in the name */
    char *lastdot = name;
    /* 'where' is the part before the dots, including the previous dots, like in
     * the "Foo.Bar.baz" it would point to "Foo.Bar" */
    char *where = name;
    for (char *p = name; *p != '\0'; p++)
      if (*p == '.') lastdot = p + 1;

    if (lastdot != name){
      name = lastdot;
      *(lastdot - 1) = '\0';
    } else {
      where = NmNamespace_GetCurr()->name;
    }
    /* get the appropriate namespace */
    if (!(namespace = NmNamespace_GetByName(where))){
      NmError_Lex(lex, NmError_GetCurr());
      return NULL;
    }
    /* let's see, if the name is actually a something */
    if (!name_lookup(name, namespace)){
      NmError_Lex(lex, "name '%s.%s' not found", namespace->name, name);
      Nm_Exit();
      return NULL;
    }
    /* it could be a function's name, so let's check it out */
    Namespace *namespaces[3];
    namespaces[0] = NmNamespace_GetByName("core");
    namespaces[1] = namespace;
    namespaces[2] = NULL;

    for (int i = 0; namespaces[i] != NULL; i++){
      namespace = namespaces[i];
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
    }

#if DEBUG
    NmDebug_Parser("%s ", name);
#endif

    if (isafunc){
      /* options are stored in a char array, for instance this:
       *
       *   function -egp 6;
       *
       * would have options translated to
       *
       *   { 'e', 'g', 'p', '\0' }
       *
       */
      /* C99 ROCKS */
      char call_opts[optc + 1];
      memset(call_opts, 0, sizeof(call_opts));
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
#if DEBUG
          NmDebug_Parser("-%s ", call_opts);
#endif
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
#if DEBUG
      NmDebug_Parser("(");
#endif
      lex->right_after_funname = false;
      if (NmLexer_Peek(lex, SYM_LPAREN)){
        NmLexer_Skip(lex);
        params = params_list(lex, argc, true);
        NmLexer_Force(lex, SYM_RPAREN);
      } else {
        params = params_list(lex, argc, false);
      }
#if DEBUG
      NmDebug_Parser(")");
#endif
      /* if 'params' returns NULL it means something bad happend */
      if (!params){
        NmError_Lex(lex, "wrong number of arguments for function '%s' %s", name, NmError_GetCurr());
        Nm_Exit();
        return NULL;
      }
      new = NmAST_GenCall(lex->current.pos, name, params, call_opts, namespace);
    } else {
      new = NmAST_GenName(lex->current.pos, name, namespace);
    }
  }
  /*
   * XXX '(' expr ')'
   */
  else if (NmLexer_Accept(lex, SYM_LPAREN)){
#if DEBUG
    NmDebug_Parser("(");
#endif
    new = expr(lex);
    NmLexer_Force(lex, SYM_RPAREN);
#if DEBUG
    NmDebug_Parser(")");
#endif
  }
  /*
   * XXX '[' params_list ']'
   */
  else if (NmLexer_Accept(lex, SYM_LBRACKET)){
#if DEBUG
    NmDebug_Parser("[");
#endif
    arr_inside = params_list(lex, -1, true);
    new = NmAST_GenArray(lex->current.pos, arr_inside);
    NmLexer_Force(lex, SYM_RBRACKET);
#if DEBUG
    NmDebug_Parser("]");
#endif
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
static Node **params_list(LexerState *lex, int num, bool inparens)
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

#if DEBUG
  NmDebug_AST(params, "create params list");
#endif

  /* if 'first_expr' is NULL it means no params were fetched at all */
  if (!first_expr && num != 1 << 15){
    NmError_SetString("(0 when %d expected)", (unsigned)num);
    NmMem_Free(params);
    return NULL;
  }

  params[counter++] = first_expr;

  /* if we are inside a parens then we fetch all of the arguments
   * if we are not, then we can as well finish just now */
  if (!inparens){
    if (num == 1){
      return params;
    }
  }

  /*
   * here ---------------------------------------------+
   *                                                   |
   *   if we are inside a parens then we fetch as many |
   *   args as we can, but I don't want to modify the  |
   *   'num' variable, as it is going to be used later |
   *   on if too many args were supplied               v
   */
  while (NmLexer_Accept(lex, SYM_COMMA) && (counter <= (inparens ? 1 << 15 : (unsigned)num))){
#if DEBUG
    NmDebug_Parser(", ");
#endif
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

  /* we should get here only if 'inparens' is true */
  if (counter > (unsigned)num){
    NmError_SetString("(%u when %d expected)", counter, (unsigned)num);
    NmAST_Free(first_expr);
    /* TODO: we should free also the args' nodes, I guess */
    NmMem_Free(params);
    return NULL;
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
#if DEBUG
    NmDebug_Parser("[");
#endif
    index = expr(lex);
    NmLexer_Force(lex, SYM_RBRACKET);
#if DEBUG
    NmDebug_Parser("]");
#endif
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
      return NULL;
    }
#if DEBUG
    NmDebug_Parser(":postfix++");
#endif
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
      return NULL;
    }
#if DEBUG
    NmDebug_Parser(":postfix--");
#endif
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
#if DEBUG
    NmDebug_Parser("unary! ");
#endif
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
#if DEBUG
    NmDebug_Parser("unary+ ");
#endif
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
#if DEBUG
    NmDebug_Parser("unary- ");
#endif
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
#if DEBUG
    NmDebug_Parser("prefix++:");
#endif
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
#if DEBUG
    NmDebug_Parser("prefix--:");
#endif
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
#if DEBUG
      NmDebug_Parser("* ");
#endif
    } else if (NmLexer_Accept(lex, SYM_SLASH)){
      op = BINARY_DIV;
#if DEBUG
      NmDebug_Parser("/ ");
#endif
    } else if (NmLexer_Accept(lex, SYM_PERCENT)){
      op = BINARY_MOD;
#if DEBUG
      NmDebug_Parser("% ");
#endif
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
#if DEBUG
      NmDebug_Parser("+ ");
#endif
    } else if (NmLexer_Accept(lex, SYM_MINUS)){
      op = BINARY_SUB;
#if DEBUG
      NmDebug_Parser("- ");
#endif
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
#if DEBUG
      NmDebug_Parser("> ");
#endif
    } else if (NmLexer_Accept(lex, SYM_LCHEVRON)){
      op = BINARY_LT;
#if DEBUG
      NmDebug_Parser("< ");
#endif
    } else if (NmLexer_Accept(lex, SYM_RCHEVRONEQ)){
      op = BINARY_GE;
#if DEBUG
      NmDebug_Parser(">= ");
#endif
    } else if (NmLexer_Accept(lex, SYM_LCHEVRONEQ)){
      op = BINARY_LE;
#if DEBUG
      NmDebug_Parser("<= ");
#endif
    } else if (NmLexer_Accept(lex, SYM_EQEQ)){
      op = BINARY_EQ;
#if DEBUG
      NmDebug_Parser("== ");
#endif
    } else if (NmLexer_Accept(lex, SYM_BANGEQ)){
      op = BINARY_NE;
#if DEBUG
      NmDebug_Parser("!= ");
#endif
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
#if DEBUG
      NmDebug_Parser("= ");
#endif
    }
    else if (NmLexer_Accept(lex, SYM_PLUSEQ)){
      op = BINARY_ASSIGN_ADD;
#if DEBUG
      NmDebug_Parser("+= ");
#endif
    }
    else if (NmLexer_Accept(lex, SYM_MINUSEQ)){
      op = BINARY_ASSIGN_SUB;
#if DEBUG
      NmDebug_Parser("-= ");
#endif
    }
    else if (NmLexer_Accept(lex, SYM_TIMESEQ)){
      op = BINARY_ASSIGN_MUL;
#if DEBUG
      NmDebug_Parser("*= ");
#endif
    }
    else if (NmLexer_Accept(lex, SYM_SLASHEQ)){
      op = BINARY_ASSIGN_DIV;
#if DEBUG
      NmDebug_Parser("/= ");
#endif
    }
    else if (NmLexer_Accept(lex, SYM_MODULOEQ)){
      op = BINARY_ASSIGN_MOD;
#if DEBUG
      NmDebug_Parser("%= ");
#endif
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
#if DEBUG
    NmDebug_Parser(", ");
#endif
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
 *     | [MY|OUR] NAME '=' expr ';'
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
  Pos savedpos = lex->current.pos;

  /*
   * XXX ';'
   */
  if (NmLexer_Accept(lex, SYM_SEMICOLON)){
    /* that's NOP */
#if DEBUG
    NmDebug_Parser("NOP;\n");
#endif
    ret = NmAST_GenNop(lex->current.pos);
  }
  /*
   * XXX [MY|OUR]
   */
  else if (NmLexer_Peek(lex, SYM_MY) ||
           NmLexer_Peek(lex, SYM_OUR)){
    uint8_t flags = 0;
    if (NmLexer_Accept(lex, SYM_MY)){
#if DEBUG
      NmDebug_Parser("my ");
#endif
      flags |= NMVAR_FLAG_PRIVATE;
    } else {
#if DEBUG
      NmDebug_Parser("our ");
#endif
      NmLexer_Skip(lex);
    }
    /*
     * XXX [MY|OUR] CONST ...
     */
    if (NmLexer_Accept(lex, SYM_CONST)){
      flags |= NMVAR_FLAG_CONST;
    }
    Symbol namesym = NmLexer_Force(lex, SYM_NAME);
    /* NmLexer_Force skips the symbol so we have to get to the previous one */
    name = namesym.value.s;
#if DEBUG
    NmDebug_Parser("%s", name);
#endif
    /*
     * XXX [MY|OUR] NAME = expr
     */
    if (NmLexer_Accept(lex, SYM_EQ)){
#if DEBUG
      NmDebug_Parser(" = ");
#endif
      value = expr(lex);
      /* if value is NULL, then something like this happend:
       *
       *    my var = ;
       *
       */
      if (!value){
        /* it actually is not an error, so we don't return NULL here or anything
         * Nemo just lacks of "warning"s */
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
   *     (current state of the lexer makes that piece of grammar impossible to
   *     achieve)
   */
  /*else if (lex->current.type == SYM_NAME &&*/
           /*NmLexer_Peek(lex, SYM_COLON)){*/
    /*name = lex->current.value.s;*/
    /*#if DEBUG
NmDebug_Parser("%s", name);*/
/*#endif*/
    /*NmLexer_Skip(lex);*/
    /*#if DEBUG
NmDebug_Parser(":", name);*/
/*#endif*/
    /*NmLexer_Skip(lex);*/
    /*body = stmt(lex);*/
    /*ret = body;*/
    /*NmNamespace_NewLabel(name, body);*/
  /*}*/
  /*
   * XXX GOTO NAME ';'
   */
  else if (NmLexer_Accept(lex, SYM_GOTO)){
#if DEBUG
    NmDebug_Parser("goto ");
#endif
    Symbol namesym = NmLexer_Force(lex, SYM_NAME);
    name = namesym.value.s;
#if DEBUG
    NmDebug_Parser("%s", name);
#endif
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
   */
  else if (NmLexer_Accept(lex, SYM_USE)){
    Pos savepos = lex->current.pos;
#if DEBUG
      NmDebug_Parser("use ");
#endif
    Symbol namesym = NmLexer_Force(lex, SYM_NAME);
    name = namesym.value.s;
#if DEBUG
    NmDebug_Parser("%s ", name);
#endif
    ret = NmAST_GenUse(savepos, name);
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
#if DEBUG
    NmDebug_Parser("fun ");
#endif
    Symbol namesym = NmLexer_Force(lex, SYM_NAME);
    name = namesym.value.s;
    /* let's check if the function hasn't already been declared/defined */
    /* 6 is 2 | 4, which are the two things name_lookup returns when found a
     * function (either C or Nemo function) */
    int a = name_lookup(name, NULL);
    /*printf("a: %d, %x\n", a, a);*/
    if (6 & name_lookup(name, NULL)){
      NmError_Lex(lex, "cannot redefine function '%s'", name);
      Nm_Exit();
      return NULL;
    }
#if DEBUG
    NmDebug_Parser("%s ", namesym.value.s);
#endif
    while (NmLexer_Peek(lex, SYM_NAME) || NmLexer_Peek(lex, SYM_OPT)){
      if (NmLexer_Peek(lex, SYM_NAME)){
        Symbol namesym = NmLexer_Force(lex, SYM_NAME);
#if DEBUG
        NmDebug_Parser("%s ", namesym.value.s);
#endif
        argv = NmMem_Realloc(argv, (argc + 1) * sizeof(char *) + 1);
        argv[argc] = NmMem_Strdup(namesym.value.s);
        argc++;
      } else if (NmLexer_Peek(lex, SYM_OPT)){
        Symbol optsym = NmLexer_Force(lex, SYM_OPT);
#if DEBUG
        NmDebug_Parser("-%c ", optsym.value.c);
#endif
        if (strchr(optv, optsym.value.c)){
          NmError_Lex(lex, "warning: option '%c' already defined", optsym.value.c);
          Nm_Exit();
          return NULL;
        } else {
          optv = NmMem_Realloc(optv, (optc + 1) * sizeof(char));
          optv[optc] = optsym.value.c;
          optc++;
        }
      }
    }
    optv[optc] = '\0';

    if (NmLexer_Accept(lex, SYM_SEMICOLON)){
      lex->right_after_fun = false;
#if DEBUG
      NmDebug_Parser(";\n");
#endif
      body = NULL;
    } else {
      NmLexer_Force(lex, SYM_LMUSTASHE);
      lex->right_after_fun = false;
#if DEBUG
      NmDebug_Parser("{\n");
#endif
      body = block(lex);
      NmLexer_Force(lex, SYM_RMUSTASHE);
#if DEBUG
      NmDebug_Parser("}\n");
#endif
    }
    ret = NmAST_GenFuncDef(pos, name, body, argc, optc, argv, optv);
  }
  /*
   * XXX IF stmt stmt
   */
  else if (NmLexer_Accept(lex, SYM_IF)){
#if DEBUG
    NmDebug_Parser("if ");
#endif
    guard = stmt(lex);
    body = stmt(lex);
    /*
     * XXX IF stmt stmt ELSE stmt
     */
    if (NmLexer_Accept(lex, SYM_ELSE)){
#if DEBUG
      NmDebug_Parser("else ");
#endif
      elsee = stmt(lex);
    }
    ret = NmAST_GenIf(lex->current.pos, guard, body, elsee, false);
  }
  /*
   * XXX UNLESS stmt stmt
   */
  else if (NmLexer_Accept(lex, SYM_UNLESS)){
#if DEBUG
    NmDebug_Parser("unless ");
#endif
    guard = stmt(lex);
    body = stmt(lex);
    /*
     * XXX UNLESS stmt stmt ELSE stmt
     */
    if (NmLexer_Accept(lex, SYM_ELSE)){
#if DEBUG
      NmDebug_Parser("else ");
#endif
      elsee = stmt(lex);
    }
    ret = NmAST_GenIf(lex->current.pos, guard, body, elsee, true);
  }
  /*
   * XXX WHILE stmt stmt
   */
  else if (NmLexer_Accept(lex, SYM_WHILE)){
#if DEBUG
    NmDebug_Parser("while ");
#endif
    guard = stmt(lex);
    body = stmt(lex);
    /*
     * XXX WHILE stmt stmt ELSE stmt
     */
    if (NmLexer_Accept(lex, SYM_ELSE)){
#if DEBUG
      NmDebug_Parser("else ");
#endif
      elsee = stmt(lex);
    }
    ret = NmAST_GenWhile(lex->current.pos, guard, body, elsee, false);
  }
  /*
   * XXX UNTIL stmt stmt
   */
  else if (NmLexer_Accept(lex, SYM_UNTIL)){
#if DEBUG
    NmDebug_Parser("until ");
#endif
    guard = stmt(lex);
    body = stmt(lex);
    /*
     * XXX UNTIL stmt stmt ELSE stmt
     */
    if (NmLexer_Accept(lex, SYM_ELSE)){
#if DEBUG
      NmDebug_Parser("else ");
#endif
      elsee = stmt(lex);
    }
    ret = NmAST_GenWhile(lex->current.pos, guard, body, elsee, true);
  }
  /*
   * XXX '{' block '}'
   */
  else if (NmLexer_Accept(lex, SYM_LMUSTASHE)){
#if DEBUG
    NmDebug_Parser("{\n");
#endif
    ret = block(lex);
    NmLexer_Force(lex, SYM_RMUSTASHE);
#if DEBUG
    NmDebug_Parser("}\n");
#endif
  } else {
    body = ret = expr(lex);
    /*
     * XXX expr IF stmt
     */
    if (NmLexer_Accept(lex, SYM_IF)){
#if DEBUG
      NmDebug_Parser("if ");
#endif
      guard = stmt(lex);
      ret = NmAST_GenIf(lex->current.pos, ret, guard, NULL, false);
    }
    /*
     * XXX expr UNLESS stmt
     */
    else if (NmLexer_Accept(lex, SYM_UNLESS)){
#if DEBUG
      NmDebug_Parser("unless ");
#endif
      guard = stmt(lex);
      ret = NmAST_GenIf(lex->current.pos, ret, guard, NULL, true);
    }
    /*
     * XXX expr WHILE stmt
     */
    else if (NmLexer_Accept(lex, SYM_WHILE)){
#if DEBUG
      NmDebug_Parser("while ");
#endif
      guard = stmt(lex);
      ret = NmAST_GenWhile(lex->current.pos, ret, guard, NULL, false);
    }
    /*
     * XXX expr UNTIL stmt
     */
    else if (NmLexer_Accept(lex, SYM_UNTIL)){
#if DEBUG
      NmDebug_Parser("until ");
#endif
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

#if DEBUG
  NmDebug_AST(new_block, "create block node");
#endif

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

