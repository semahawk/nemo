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
                      n->type == NT_STRING  || \
                      n->type == NT_ARRAY)

/* <lex> is of type { LexerState * } */
#if DEBUG
#define endStmt(lex) do { \
    if (!nm_lex_peek(lex, SYM_LMUSTASHE) && \
        !nm_lex_peek(lex, SYM_RMUSTASHE) && \
        !nm_lex_peek(lex, SYM_EOS)){ \
      nm_lex_force(lex, SYM_SEMICOLON); \
    } \
    nm_debug_parser("ENDSTMT;\n"); \
  } while (0);
#else
#define endStmt(lex) do { \
    if (!nm_lex_peek(lex, SYM_LMUSTASHE) && \
        !nm_lex_peek(lex, SYM_RMUSTASHE) && \
        !nm_lex_peek(lex, SYM_EOS)){ \
      nm_lex_force(lex, SYM_SEMICOLON); \
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
  /* {{{ */
  Node *new = NULL;
  Node **arr_inside = NULL;
  char *name = NULL;

  /*
   * XXX INTEGER
   */
  if (nm_lex_peek(lex, SYM_INTEGER)){
    /* {{{ */
    new = nm_ast_gen_int(lex->current.pos, lex->current.value.i);
#if DEBUG
    nm_debug_parser("%d ", ((Node_Int *)new)->i);
#endif
    nm_lex_skip(lex);
    /* }}} */
  }
  /*
   * XXX FLOAT
   */
  else if (nm_lex_peek(lex, SYM_FLOAT)){
    /* {{{ */
    new = nm_ast_gen_float(lex->current.pos, lex->current.value.f);
#if DEBUG
    nm_debug_parser("%f ", ((Node_Float *)new)->f);
#endif
    nm_lex_skip(lex);
    /* }}} */
  }
  /*
   * XXX STRING
   */
  else if (nm_lex_peek(lex, SYM_STRING)){
    /* {{{ */
    new = nm_ast_gen_str(lex->current.pos, lex->current.value.s);
#if DEBUG
    nm_debug_parser("\"%s\" ", ((Node_String *)new)->s);
#endif
    nm_lex_skip(lex);
    /* }}} */
  }
  /*
   * XXX NAME
   */
  else if (nm_lex_peek(lex, SYM_NAME)){
    /* {{{ */
    int argc = 0;
    unsigned optc = 0;
    char *opts = NULL;
    CFunc *cfunc;
    Func *func;
    Namespace *namespace;
    Node **params = NULL;
    Symbol namesym = nm_lex_force(lex, SYM_NAME);
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
      where = nm_curr_namespace()->name;
    }
    /* get the appropriate namespace */
    if (!(namespace = nm_get_namespace_by_name(where))){
      nm_lex_error(lex, nm_curr_error());
      return NULL;
    }
    /* let's see, if the name is actually a something */
    int lookup = name_lookup(name, namespace);
    if (!lookup){
      nm_lex_error(lex, "name '%s.%s' not found", namespace->name, name);
      nexit();
      return NULL;
    }
    /* it could be a function's name, so let's check it out */
    Namespace *namespaces[3];
    namespaces[0] = nm_get_namespace_by_name("core");
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
    nm_debug_parser("%s ", name);
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
        if (nm_lex_peek(lex, SYM_OPT)){
          Symbol optsym = nm_lex_force(lex, SYM_OPT);
          strcpy(call_opts, optsym.value.s);
#if DEBUG
          nm_debug_parser("-%s ", call_opts);
#endif
          /* check if not too many options were given */
          if (strlen(call_opts) > optc){
            nm_lex_error(lex, "too many options given for the function '%s', supported options are '%s'", name, opts);
            nexit();
            return NULL;
          }
          /* check if the function supports given options */
          for (unsigned i = 0; i < strlen(call_opts); i++){
            if (!strchr(opts, call_opts[i])){
              nm_lex_error(lex, "function '%s' doesn't support the '%c' option", name, call_opts[i]);
              nexit();
              return NULL;
            }
          }
        }
        lex->right_after_funname = false;
      }
#if DEBUG
      nm_debug_parser("(");
#endif
      lex->right_after_funname = false;
      if (nm_lex_peek(lex, SYM_LPAREN)){
        nm_lex_skip(lex);
        params = params_list(lex, argc, true);
        nm_lex_force(lex, SYM_RPAREN);
      } else {
        params = params_list(lex, argc, false);
      }
#if DEBUG
      nm_debug_parser(")");
#endif
      /* if 'params' returns NULL it means something bad happend */
      if (!params){
        nm_lex_error(lex, "wrong number of arguments for function '%s' %s", name, nm_curr_error());
        nexit();
        return NULL;
      }
      new = nm_ast_gen_call(lex->current.pos, name, params, call_opts, namespace);
    } else {
      new = nm_ast_gen_name(lex->current.pos, name, namespace);
    }
    /* }}} */
  }
  /*
   * XXX '(' expr ')'
   */
  else if (nm_lex_accept(lex, SYM_LPAREN)){
    /* {{{ */
#if DEBUG
    nm_debug_parser("(");
#endif
    new = expr(lex);
    nm_lex_force(lex, SYM_RPAREN);
#if DEBUG
    nm_debug_parser(")");
#endif
    /* }}} */
  }
  /*
   * XXX '[' params_list ']'
   */
  else if (nm_lex_accept(lex, SYM_LBRACKET)){
    /* {{{ */
#if DEBUG
    nm_debug_parser("[");
#endif
    arr_inside = params_list(lex, -1, true);
    new = nm_ast_gen_arr(lex->current.pos, arr_inside);
    nm_lex_force(lex, SYM_RBRACKET);
#if DEBUG
    nm_debug_parser("]");
#endif
    /* }}} */
  }
  /*
   * XXX EOS
   */
  else if (nm_lex_accept(lex, SYM_EOS)){
    /* {{{ */
    lex->eos = true;
    return NULL;
    /* }}} */
  }
  /*
   * XXX nothing described above, returning NULL
   */
  else {
    return NULL;
  }

  return new;
  /* }}} */
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
  /* {{{ */
  unsigned nmemb = 5;
  unsigned counter = 0;
  Node **params = ncalloc(nmemb, sizeof(Node));
  /* using assign_expr instead of expr because comma_expr would screw things up
   * here*/
  Node *first_expr = assign_expr(lex);

  if (num == 0){
    if (first_expr){
      nm_set_error("(1 when 0 expected)");
      nm_ast_free(first_expr);
      nfree(params);
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
  nm_debug_ast(params, "create params list");
#endif

  /* if 'first_expr' is NULL it means no params were fetched at all */
  if (!first_expr && num != 1 << 15){
    nm_set_error("(0 when %d expected)", (unsigned)num);
    nfree(params);
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
  while (nm_lex_accept(lex, SYM_COMMA) && (counter <= (inparens ? 1 << 15 : (unsigned)num))){
#if DEBUG
    nm_debug_parser(", ");
#endif
    Node *next_expr = assign_expr(lex);
    /*
     * Always make the array be one element bigger than supposed to, so the last
     * element is NULL, so traversing is easy
     */
    if (counter + 1 > nmemb - 1){
      nmemb++;
      params = nrealloc(params, nmemb * sizeof(Node));
    }
    params[counter++] = next_expr;
  }

  /* we should get here only if 'inparens' is true */
  if (counter > (unsigned)num){
    nm_set_error("(%u when %d expected)", counter, (unsigned)num);
    nm_ast_free(first_expr);
    /* TODO: we should free also the args' nodes, I guess */
    nfree(params);
    return NULL;
  }

  return params;
  /* }}} */
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
  /* {{{ */
  Node *target = NULL;
  Node *ret = target = primary_expr(lex);
  Node *index;

  /* I'm not checking if target is NULL because parenthesis, brackets, ++ and --
   * could belong to some prefix unary operation or just an expression */

  /*
   * XXX NAME '[' expr ']'
   */
  if (nm_lex_accept(lex, SYM_LBRACKET)){
    /* {{{ */
    /* TODO: store the position of the left bracket
     *       right now it's the position of the very first thing after the left
     *       bracket */
    Pos pos = lex->current.pos;
#if DEBUG
    nm_debug_parser("[");
#endif
    index = expr(lex);
    nm_lex_force(lex, SYM_RBRACKET);
#if DEBUG
    nm_debug_parser("]");
#endif
    ret = nm_ast_gen_binop(pos, target, BINARY_INDEX, index);
    /* }}} */
  }
  /*
   * XXX NAME '++'
   */
  else if (nm_lex_accept(lex, SYM_PLUSPLUS)){
    /* {{{ */
    if (isLiteral(target)){
      /* using nm_error not nm_lex_error because lexer's state has gone
       * further in 'primary_expr' */
      /* FIXME: store the target's position */
      nm_error("can't do the postfix increment on a literal in line %u at column %u", lex->current.pos.line, lex->current.pos.column);
      return NULL;
    }
#if DEBUG
    nm_debug_parser(":postfix++");
#endif
    ret = nm_ast_gen_unop(lex->current.pos, target, UNARY_POSTINC);
    /* }}} */
  }
  /*
   * XXX NAME '--'
   */
  else if (nm_lex_accept(lex, SYM_MINUSMINUS)){
    /* {{{ */
    if (isLiteral(target)){
      /* using nm_error not nm_lex_error because lexer's state has gone
       * further in 'primary_expr' */
      /* FIXME: store the target's position */
      nm_error("can't do the postfix increment on a literal in line %u at column %u", lex->current.pos.line, lex->current.pos.column);
      return NULL;
    }
#if DEBUG
    nm_debug_parser(":postfix--");
#endif
    ret = nm_ast_gen_unop(lex->current.pos, target, UNARY_POSTDEC);
    /* }}} */
  }

  return ret;
  /* }}} */
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
  /* {{{ */
  Node *ret = NULL;
  Node *target = NULL;

  if (nm_lex_accept(lex, SYM_BANG)){
    /* {{{ */
#if DEBUG
    nm_debug_parser("unary! ");
#endif
    target = prefix_expr(lex);
    /* if target is NULL it means something like that happend:
     *
     *    my var;
     *    var = !;
     *
     */
    if (!target){
      nm_lex_error(lex, "expected an expression for the unary negation");
      nexit();
      return NULL;
    }
    ret = nm_ast_gen_unop(lex->current.pos, target, UNARY_NEGATE);
    /* }}} */
  }
  else if (nm_lex_accept(lex, SYM_PLUS)){
    /* {{{ */
#if DEBUG
    nm_debug_parser("unary+ ");
#endif
    target = prefix_expr(lex);
    /* if target is NULL it means something like that happend:
     *
     *    my var;
     *    var = +;
     *
     */
    if (!target){
      nm_lex_error(lex, "expected an expression for the unary plus");
      nexit();
      return NULL;
    }
    ret = nm_ast_gen_unop(lex->current.pos, target, UNARY_PLUS);
    /* }}} */
  }
  else if (nm_lex_accept(lex, SYM_MINUS)){
    /* {{{ */
#if DEBUG
    nm_debug_parser("unary- ");
#endif
    target = prefix_expr(lex);
    /* if target is NULL it means something like that happend:
     *
     *    my var;
     *    var = -;
     *
     */
    if (!target){
      nm_lex_error(lex, "expected an expression for the unary minus");
      nexit();
      return NULL;
    }
    ret = nm_ast_gen_unop(lex->current.pos, target, UNARY_MINUS);
    /* }}} */
  }
  else if (nm_lex_accept(lex, SYM_PLUSPLUS)){
    /* {{{ */
#if DEBUG
    nm_debug_parser("prefix++:");
#endif
    target = prefix_expr(lex);
    /* if target is NULL it means something like that happend:
     *
     *    my var;
     *    var = ++;
     *
     */
    if (!target){
      nm_lex_error(lex, "expected an expression for the prefix incrementation");
      nexit();
      return NULL;
    }
    ret = nm_ast_gen_unop(lex->current.pos, target, UNARY_PREINC);
    /* }}} */
  }
  else if (nm_lex_accept(lex, SYM_MINUSMINUS)){
    /* {{{ */
#if DEBUG
    nm_debug_parser("prefix--:");
#endif
    target = prefix_expr(lex);
    /* if target is NULL it means something like that happend:
     *
     *    my var;
     *    var = --;
     *
     */
    if (!target){
      nm_lex_error(lex, "expected an expression for the prefix decrementation");
      nexit();
      return NULL;
    }
    ret = nm_ast_gen_unop(lex->current.pos, target, UNARY_PREDEC);
    /* }}} */
  }
  else {
    ret = postfix_expr(lex);
  }

  return ret;
  /* }}} */
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
  /* {{{ */
  Node *ret;
  Node *left;
  Node *right;
  BinaryOp op = 0;

  ret = left = prefix_expr(lex);

  while (nm_lex_peek(lex, SYM_TIMES) ||
         nm_lex_peek(lex, SYM_SLASH) ||
         nm_lex_peek(lex, SYM_PERCENT)){
    /* if left is NULL it means something like that happend:
     *
     *    my var;
     *    var = * 2;
     *
     */
    if (!left){
      nm_lex_error(lex, "expected an expression for the lhs of the binary %s operation", symToS(lex->current.type));
      nexit();
      return NULL;
    }
    if (nm_lex_accept(lex, SYM_TIMES)){
      op = BINARY_MUL;
#if DEBUG
      nm_debug_parser("* ");
#endif
    } else if (nm_lex_accept(lex, SYM_SLASH)){
      op = BINARY_DIV;
#if DEBUG
      nm_debug_parser("/ ");
#endif
    } else if (nm_lex_accept(lex, SYM_PERCENT)){
      op = BINARY_MOD;
#if DEBUG
      nm_debug_parser("% ");
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
      nm_lex_error(lex, "expected an expression for the rhs of the binary %s operation", binopToS(op));
      nexit();
      return NULL;
    }
    ret = nm_ast_gen_binop(lex->current.pos, left, op, right);
    left = ret;
  }

  return ret;
  /* }}} */
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
  /* {{{ */
  Node *ret;
  Node *left;
  Node *right;
  BinaryOp op = 0;

  ret = left = mult_expr(lex);

  while (nm_lex_peek(lex, SYM_PLUS) || nm_lex_peek(lex, SYM_MINUS)){
    /* we are not checking if letf is NULL here, because + and - are also a
     * prefix unary operations */
    if (nm_lex_accept(lex, SYM_PLUS)){
      op = BINARY_ADD;
#if DEBUG
      nm_debug_parser("+ ");
#endif
    } else if (nm_lex_accept(lex, SYM_MINUS)){
      op = BINARY_SUB;
#if DEBUG
      nm_debug_parser("- ");
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
      nm_lex_error(lex, "expected an expression for the rhs of the binary %s operation", binopToS(op));
      nexit();
      return NULL;
    }
    ret = nm_ast_gen_binop(lex->current.pos, left, op, right);
    left = ret;
  }

  return ret;
  /* }}} */
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
  /* {{{ */
  Node *ret;
  Node *left;
  Node *right;
  BinaryOp op = 0;

  ret = left = add_expr(lex);

  while (nm_lex_peek(lex, SYM_RCHEVRON) ||
         nm_lex_peek(lex, SYM_LCHEVRON) ||
         nm_lex_peek(lex, SYM_RCHEVRONEQ) ||
         nm_lex_peek(lex, SYM_LCHEVRONEQ) ||
         nm_lex_peek(lex, SYM_EQEQ) ||
         nm_lex_peek(lex, SYM_BANGEQ)){
    /* if left is NULL it means something like that happend:
     *
     *    my var;
     *    var = > 2;
     *
     */
    if (!left){
      nm_lex_error(lex, "expected an expression for the lhs of the binary %s operation", symToS(lex->current.type));
      nexit();
      return NULL;
    }
    if (nm_lex_accept(lex, SYM_RCHEVRON)){
      op = BINARY_GT;
#if DEBUG
      nm_debug_parser("> ");
#endif
    } else if (nm_lex_accept(lex, SYM_LCHEVRON)){
      op = BINARY_LT;
#if DEBUG
      nm_debug_parser("< ");
#endif
    } else if (nm_lex_accept(lex, SYM_RCHEVRONEQ)){
      op = BINARY_GE;
#if DEBUG
      nm_debug_parser(">= ");
#endif
    } else if (nm_lex_accept(lex, SYM_LCHEVRONEQ)){
      op = BINARY_LE;
#if DEBUG
      nm_debug_parser("<= ");
#endif
    } else if (nm_lex_accept(lex, SYM_EQEQ)){
      op = BINARY_EQ;
#if DEBUG
      nm_debug_parser("== ");
#endif
    } else if (nm_lex_accept(lex, SYM_BANGEQ)){
      op = BINARY_NE;
#if DEBUG
      nm_debug_parser("!= ");
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
      nm_lex_error(lex, "expected an expression for the rhs of the binary %s operation", binopToS(op));
      nexit();
      return NULL;
    }
    ret = nm_ast_gen_binop(lex->current.pos, left, op, right);
    left = ret;
  }

  return ret;
  /* }}} */
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
  /* {{{ */
  Node *ret;
  Node *left;
  Node *right;
  BinaryOp op = 0;

  ret = left = cond_expr(lex);

  while (nm_lex_peek(lex, SYM_EQ)      ||
         nm_lex_peek(lex, SYM_PLUSEQ)  ||
         nm_lex_peek(lex, SYM_MINUSEQ) ||
         nm_lex_peek(lex, SYM_TIMESEQ) ||
         nm_lex_peek(lex, SYM_SLASHEQ) ||
         nm_lex_peek(lex, SYM_MODULOEQ)){
    /* if left is NULL it means something like that happend:
     *
     *    my var;
     *    = 2;
     *
     */
    if (!left){
      nm_lex_error(lex, "expected an expression for the lhs of the binary %s operation", symToS(lex->current.type));
      nexit();
      return NULL;
    }
    if (nm_lex_accept(lex, SYM_EQ)){
      op = BINARY_ASSIGN;
#if DEBUG
      nm_debug_parser("= ");
#endif
    }
    else if (nm_lex_accept(lex, SYM_PLUSEQ)){
      op = BINARY_ASSIGN_ADD;
#if DEBUG
      nm_debug_parser("+= ");
#endif
    }
    else if (nm_lex_accept(lex, SYM_MINUSEQ)){
      op = BINARY_ASSIGN_SUB;
#if DEBUG
      nm_debug_parser("-= ");
#endif
    }
    else if (nm_lex_accept(lex, SYM_TIMESEQ)){
      op = BINARY_ASSIGN_MUL;
#if DEBUG
      nm_debug_parser("*= ");
#endif
    }
    else if (nm_lex_accept(lex, SYM_SLASHEQ)){
      op = BINARY_ASSIGN_DIV;
#if DEBUG
      nm_debug_parser("/= ");
#endif
    }
    else if (nm_lex_accept(lex, SYM_MODULOEQ)){
      op = BINARY_ASSIGN_MOD;
#if DEBUG
      nm_debug_parser("%= ");
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
      nm_lex_error(lex, "expected an expression");
      nexit();
      return NULL;
    }
    ret = nm_ast_gen_binop(lex->current.pos, left, op, right);
    left = ret;
  }

  return ret;
  /* }}} */
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
  /* {{{ */
  Node *ret;
  Node *left;
  Node *right;

  ret = left = assign_expr(lex);

  while (nm_lex_accept(lex, SYM_COMMA)){
    /* if left is NULL it means something like that happend:
     *
     *    my var;
     *    , 2;
     *
     */
    if (!left){
      nm_lex_error(lex, "expected an expression for the lhs of the binary ',' operation");
      nexit();
      return NULL;
    }
#if DEBUG
    nm_debug_parser(", ");
#endif
    right = assign_expr(lex);
    /* if right is NULL it means something like that happend:
     *
     *    my var;
     *    var = 2, ;
     *
     */
    if (!right){
      nm_lex_error(lex, "expected an expression for the rhs of the binary ',' operation");
      nexit();
      return NULL;
    }
    ret = nm_ast_gen_binop(lex->current.pos, left, BINARY_COMMA, right);
    left = ret;
  }

  return ret;
  /* }}} */
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
 * function_prototype: FUN NAME args ';'
 *                   | FUN NAME args block
 *                   ;
 *
 * args: '(' [ifas]* ')'
 *     ;
 */
static Node *stmt(LexerState *lex)
{
  /* {{{ */
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
  if (nm_lex_accept(lex, SYM_SEMICOLON)){
    /* {{{ */
    /* that's NOP */
#if DEBUG
    nm_debug_parser("NOP;\n");
#endif
    ret = nm_ast_gen_nop(lex->current.pos);
    /* }}} */
  }
  /*
   * XXX [MY|OUR]
   */
  else if (nm_lex_peek(lex, SYM_MY) ||
           nm_lex_peek(lex, SYM_OUR)){
    /* {{{ */
    uint8_t flags = 0;
    if (nm_lex_accept(lex, SYM_MY)){
#if DEBUG
      nm_debug_parser("my ");
#endif
      flags |= NMVAR_FLAG_PRIVATE;
    } else {
#if DEBUG
      nm_debug_parser("our ");
#endif
      nm_lex_skip(lex);
    }
    /*
     * XXX [MY|OUR] CONST ...
     */
    if (nm_lex_accept(lex, SYM_CONST)){
      flags |= NMVAR_FLAG_CONST;
    }
    Symbol namesym = nm_lex_force(lex, SYM_NAME);
    /* nm_lex_force skips the symbol so we have to get to the previous one */
    name = namesym.value.s;
#if DEBUG
    nm_debug_parser("%s", name);
#endif
    /*
     * XXX [MY|OUR] NAME = expr
     */
    if (nm_lex_accept(lex, SYM_EQ)){
#if DEBUG
      nm_debug_parser(" = ");
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
        nm_lex_error(lex, "nothing was initialized");
      }
    }
    ret = nm_ast_gen_decl(lex->current.pos, name, value, flags);
    endStmt(lex);
    /* }}} */
  }
  /*
   * XXX NAME ':' stmt
   *
   *     basically just a label
   *     (current state of the lexer makes that piece of grammar impossible to
   *     achieve)
   */
  /*else if (lex->current.type == SYM_NAME &&*/
           /*nm_lex_peek(lex, SYM_COLON)){*/
    /* {{{ */
    /*name = lex->current.value.s;*/
    /*#if DEBUG
nm_debug_parser("%s", name);*/
/*#endif*/
    /*nm_lex_skip(lex);*/
    /*#if DEBUG
nm_debug_parser(":", name);*/
/*#endif*/
    /*nm_lex_skip(lex);*/
    /*body = stmt(lex);*/
    /*ret = body;*/
    /*nm_new_namespaceLabel(name, body);*/
    /* }}} */
  /*}*/
  /*
   * XXX GOTO NAME ';'
   */
  else if (nm_lex_accept(lex, SYM_GOTO)){
    /* {{{ */
#if DEBUG
    nm_debug_parser("goto ");
#endif
    Symbol namesym = nm_lex_force(lex, SYM_NAME);
    name = namesym.value.s;
#if DEBUG
    nm_debug_parser("%s", name);
#endif
    ret = nm_ast_gen_nop(namesym.pos);
    ret->next = nm_get_label(name);
    /* nm_get_label returns NULL if didn't find the label */
    if (!ret->next){
      nm_parser_error(ret, "label '%s' was not found", name);
      nexit();
      ret = NULL;
    }
    endStmt(lex);
    /* }}} */
  }
  /*
   * XXX USE NAME ';'
   */
  else if (nm_lex_accept(lex, SYM_USE)){
    /* {{{ */
    Pos savepos = lex->current.pos;
#if DEBUG
      nm_debug_parser("use ");
#endif
    Symbol namesym = nm_lex_force(lex, SYM_NAME);
    name = namesym.value.s;
#if DEBUG
    nm_debug_parser("%s ", name);
#endif
    ret = nm_ast_gen_use(savepos, name);
    endStmt(lex);
    /* }}} */
  }
  /*
   * XXX NAMESPACE NAME stmt
   */
  else if (nm_lex_accept(lex, SYM_NAMESPACE)){
    /* {{{ */
#if DEBUG
    nm_debug_parser("namespace ");
#endif
    Symbol namesym = nm_lex_force(lex, SYM_NAME);
    nm_switch_namespace(namesym.value.s);
#if DEBUG
    nm_debug_parser("%s ", namesym.value.s);
#endif
    ret = stmt(lex);
    nm_restore_namespace();
    /* }}} */
  }
  /*
   * XXX FUN NAME [OPT|NAME]* ';'
   * XXX FUN NAME [OPT|NAME]* block
   */
  else if (nm_lex_accept(lex, SYM_FUN)){
    /* {{{ */
    /* indicates whether a incorrect character was used in the args list */
    bool unknown_arg_char = false;
    lex->right_after_fun = true;
    unsigned argc = 0;
    unsigned optc = 0;
    NobType *argv = nmalloc(sizeof(NobType) * 1);
    char *optv = nmalloc(sizeof(char) * 1 + 1); /* I know sizeof(char) is 1 */
    /* FIXME: store the "fun" position (right now it's a position of the function's name) */
    Pos pos = lex->current.pos;
#if DEBUG
    nm_debug_parser("fun ");
#endif
    Symbol namesym = nm_lex_force(lex, SYM_NAME);
    name = namesym.value.s;
#if DEBUG
    nm_debug_parser("%s ", namesym.value.s);
#endif
    /* fetch the options */
    while (nm_lex_peek(lex, SYM_OPT)){
      Symbol optsym = nm_lex_force(lex, SYM_OPT);
#if DEBUG
      nm_debug_parser("-%c ", optsym.value.c);
#endif
      if (strchr(optv, optsym.value.c)){
        nm_lex_error(lex, "warning: option '%c' already defined", optsym.value.c);
        nexit();
        return NULL;
      } else {
        optv = nrealloc(optv, (optc + 1) * sizeof(char));
        optv[optc] = optsym.value.c;
        optc++;
      }
    }
    optv[optc] = '\0';

    /* fetch the arguments */
    nm_lex_force(lex, SYM_LPAREN);
#if DEBUG
    nm_debug_parser("(");
#endif
    if (nm_lex_accept(lex, SYM_NAME)){
      char *args = lex->current.value.s;
      /* let's see if correct characters were given
       * as of version 0.19.0 (24 Aug, 2013) the only correct letters are:
       *
       *   i - for an int
       *   f - for a float
       *   a - for an array
       *   s - for a string
       */
      for (char *p = args; *p != '\0' && argc < 32; p++){
        switch (*p){
          case 'i':
            argv = nrealloc(argv, (argc + 1) * sizeof(NobType) + 1);
            argv[argc++] = OT_INTEGER;
            break;
          case 'f':
            argv = nrealloc(argv, (argc + 1) * sizeof(NobType) + 1);
            argv[argc++] = OT_FLOAT;
            break;
          case 'a':
            argv = nrealloc(argv, (argc + 1) * sizeof(NobType) + 1);
            argv[argc++] = OT_ARRAY;
            break;
          case 's':
            argv = nrealloc(argv, (argc + 1) * sizeof(NobType) + 1);
            argv[argc++] = OT_STRING;
            break;
          case 'h':
            argv = nrealloc(argv, (argc + 1) * sizeof(NobType) + 1);
            argv[argc++] = OT_FILE;
            break;
          default:
            nm_lex_error(lex, "unknown argument character '%c'", *p);
            unknown_arg_char = true;
            break;
        }
      }
#if DEBUG
      nm_debug_parser("%s", args);
#endif
    }

    if (unknown_arg_char){
      nexit();
      return NULL;
    }

    nm_lex_force(lex, SYM_RPAREN);
#if DEBUG
    nm_debug_parser(")");
#endif

    if (nm_lex_accept(lex, SYM_SEMICOLON)){
      lex->right_after_fun = false;
#if DEBUG
      nm_debug_parser(";\n");
#endif
      body = NULL;
    } else {
      nm_lex_force(lex, SYM_LMUSTASHE);
      lex->right_after_fun = false;
#if DEBUG
      nm_debug_parser("{\n");
#endif
      body = block(lex);
      nm_lex_force(lex, SYM_RMUSTASHE);
#if DEBUG
      nm_debug_parser("}\n");
#endif
    }

    /* let's check if the function hasn't already been declared/defined */
    /* 6 is 2 | 4, which are the two things name_lookup returns when found a
     * function (either C or Nemo function) */
    int lookup = name_lookup(name, NULL);
    if (6 & lookup){
      nm_lex_error(lex, "cannot redefine function '%s'", name);
      nexit();
      return NULL;
    /* 8th bit set means there is a Nemo function, but not defined, only declared */
    } else if (8 & lookup) {
      /* if there is no a body then it's a declaration */
      if (!body){
        nm_lex_error(lex, "cannot redeclare function '%s'", name);
        nexit();
        return NULL;
      }
      /* no else needed, we are defining a function that has already been declared which is cool */
    }

    ret = nm_ast_gen_funcdef(pos, name, body, argc, optc, argv, optv);
    /* }}} */
  }
  /*
   * XXX IF stmt stmt
   */
  else if (nm_lex_accept(lex, SYM_IF)){
    /* {{{ */
#if DEBUG
    nm_debug_parser("if ");
#endif
    guard = stmt(lex);
    body = stmt(lex);
    /*
     * XXX IF stmt stmt ELSE stmt
     */
    if (nm_lex_accept(lex, SYM_ELSE)){
#if DEBUG
      nm_debug_parser("else ");
#endif
      elsee = stmt(lex);
    }
    ret = nm_ast_gen_if(lex->current.pos, guard, body, elsee, false);
    /* }}} */
  }
  /*
   * XXX UNLESS stmt stmt
   */
  else if (nm_lex_accept(lex, SYM_UNLESS)){
    /* {{{ */
#if DEBUG
    nm_debug_parser("unless ");
#endif
    guard = stmt(lex);
    body = stmt(lex);
    /*
     * XXX UNLESS stmt stmt ELSE stmt
     */
    if (nm_lex_accept(lex, SYM_ELSE)){
#if DEBUG
      nm_debug_parser("else ");
#endif
      elsee = stmt(lex);
    }
    ret = nm_ast_gen_if(lex->current.pos, guard, body, elsee, true);
    /* }}} */
  }
  /*
   * XXX WHILE stmt stmt
   */
  else if (nm_lex_accept(lex, SYM_WHILE)){
    /* {{{ */
#if DEBUG
    nm_debug_parser("while ");
#endif
    guard = stmt(lex);
    body = stmt(lex);
    /*
     * XXX WHILE stmt stmt ELSE stmt
     */
    if (nm_lex_accept(lex, SYM_ELSE)){
#if DEBUG
      nm_debug_parser("else ");
#endif
      elsee = stmt(lex);
    }
    ret = nm_ast_gen_while(lex->current.pos, guard, body, elsee, false);
    /* }}} */
  }
  /*
   * XXX UNTIL stmt stmt
   */
  else if (nm_lex_accept(lex, SYM_UNTIL)){
    /* {{{ */
#if DEBUG
    nm_debug_parser("until ");
#endif
    guard = stmt(lex);
    body = stmt(lex);
    /*
     * XXX UNTIL stmt stmt ELSE stmt
     */
    if (nm_lex_accept(lex, SYM_ELSE)){
#if DEBUG
      nm_debug_parser("else ");
#endif
      elsee = stmt(lex);
    }
    ret = nm_ast_gen_while(lex->current.pos, guard, body, elsee, true);
    /* }}} */
  }
  /*
   * XXX '{' block '}'
   */
  else if (nm_lex_accept(lex, SYM_LMUSTASHE)){
    /* {{{ */
#if DEBUG
    nm_debug_parser("{\n");
#endif
    ret = block(lex);
    nm_lex_force(lex, SYM_RMUSTASHE);
#if DEBUG
    nm_debug_parser("}\n");
#endif
    /* }}} */
  }
  /*
   * XXX expr
   */
  else {
    /* {{{ */
    body = ret = expr(lex);
    /*
     * XXX expr IF stmt
     */
    if (nm_lex_accept(lex, SYM_IF)){
      /* {{{ */
#if DEBUG
      nm_debug_parser("if ");
#endif
      guard = stmt(lex);
      ret = nm_ast_gen_if(lex->current.pos, guard, ret, NULL, false);
      /* }}} */
    }
    /*
     * XXX expr UNLESS stmt
     */
    else if (nm_lex_accept(lex, SYM_UNLESS)){
      /* {{{ */
#if DEBUG
      nm_debug_parser("unless ");
#endif
      guard = stmt(lex);
      ret = nm_ast_gen_if(lex->current.pos, guard, ret, NULL, true);
      /* }}} */
    }
    /*
     * XXX expr WHILE stmt
     */
    else if (nm_lex_accept(lex, SYM_WHILE)){
      /* {{{ */
#if DEBUG
      nm_debug_parser("while ");
#endif
      guard = stmt(lex);
      ret = nm_ast_gen_while(lex->current.pos, guard, ret, NULL, false);
      /* }}} */
    }
    /*
     * XXX expr UNTIL stmt
     */
    else if (nm_lex_accept(lex, SYM_UNTIL)){
      /* {{{ */
#if DEBUG
      nm_debug_parser("until ");
#endif
      guard = stmt(lex);
      ret = nm_ast_gen_while(lex->current.pos, guard, ret, NULL, true);
      /* }}} */
    }
    /*
     * XXX expr ';'
     */
    else if (!lex->eos) endStmt(lex);
    /* }}} */
  }

  /* this logic probably could've been done better */
  if (lex->eos && !ret){
    return NULL;
  } else {
    /* if ret is NULL right here, it means there was some error above
     * so let's just gently do nothing */
    if (!ret && !lex->eos)
      ret = nm_ast_gen_nop(savedpos);

    ret = nm_ast_gen_stmt(ret->pos, ret);
  }

  /* set the previous statement's "next", but only if it's previous value is
   * different than NULL */
  if (prev_stmt)
    if (prev_stmt->next == NULL)
      prev_stmt->next = ret;

  prev_stmt = ret;

  return ret;
  /* }}} */
}

/*
 * block: [stmt]*
 *      ;
 */
static Node *block(LexerState *lex)
{
  /* {{{ */
  Node_Block *new_block = nmalloc(sizeof(Node_Block));

  new_block->type = NT_BLOCK;
  new_block->head = NULL;
  new_block->tail = NULL;

#if DEBUG
  nm_debug_ast(new_block, "create block node");
#endif

  while (!nm_lex_peek(lex, SYM_RMUSTASHE) && !nm_lex_peek(lex, SYM_EOS)){
    Statement *new_stmt = nmalloc(sizeof(Statement));
    new_stmt->stmt = stmt(lex);
    if (new_stmt->stmt == NULL){
      nfree(new_stmt);
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
  /* }}} */
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
 * @name - nm_parse_file
 * @desc - parse file of a given <fname> and return a pointer to the node of a
 *         block that was parsed, the main block of the whole script
 * @return - {Node *} of .type = NT_BLOCK
 */
Node *nm_parse_file(char *fname)
{
  /* {{{ */
  Node *nodest = NULL;
  LexerState lex;
  FILE *fp;
  char *fbuffer = NULL;
  size_t flen = 0;

  if ((fp = fopen(fname, "r")) == NULL){
    nm_fatal("cannot open file '%s': %s", fname, strerror(errno));
    exit(EXIT_FAILURE);
  }
  /* get the files length */
  fseek(fp, 0, SEEK_END);
  flen = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  /* make room for the contents */
  fbuffer = nmalloc(flen);
  /* store the files contents in the fbuffer */
  if (fread(fbuffer, 1, flen, fp) != flen){
    nm_fatal("fread failed in " __FILE__ " at line %d", __LINE__);
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
  nm_lex_tidyup(&lex);
  /* free the buffer */
  nfree(fbuffer);
  /* close the file handle */
  fclose(fp);

  return nodest;
  /* }}} */
}

/*
 * @name - nm_parse_string
 * @desc - parse the given <string> and return a pointer to the node of a
 *         block that was parsed, the main block of the script
 *         (it probably should return something else)
 * @return - { Node * } of .type = NT_BLOCK
 */
Node *nm_parse_string(char *string)
{
  /* {{{ */
  Node *nodest = NULL;
  LexerState lex;
  lex.is_file = false;
  lex.source = string;
  lex.content = string;
  lex.savecontent = string;
  lexinitrest(lex);
  nodest = block(&lex);
  nm_lex_tidyup(&lex);

  return nodest;
  /* }}} */
}

