/*
 *
 * types.h
 *
 * Created at:  Tue Jul 21 23:43:45 2015 23:43:45
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef COUNT_PARAMS_H
#define COUNT_PARAMS_H

#include <string.h>

#include "ast.h"
#include "nob.h"

/*
 * A structure that holds information about the number of (implicit) params in a
 * node / expression. It's used by the `count_params_*` set of functions which
 * infer the number of (implicit) params a function takes.
 */
struct params_info {
  enum {
    PARAMS_INFO_BITMAP,
    PARAMS_INFO_DYNAMIC
  } tag;

  /*
   * Each set bit indicates a (implicit) param being used by a function. So,
   * given a function:
   *
   *     my f = { %1 + %3 };
   *
   * the bitmap would be equal to 0x5, 0b101 (bits #0 and #2 are set). This
   * should also issue some warning (or a note) that param %2 is being unused
   * but that's off the point. The number of (implicit) params a function takes
   * is then simply the number of bits set in the bitmap, and any unused
   * (implicit) params are indicated by the unset bits to the right of the
   * leftmost set bit.
   *
   *
   * If a function takes more than 32 (implicit) params (which is quite
   * unlikely, but still) then an array of uint32_t's is allocated on the
   * heap, and <value> shall be treated as such (that is, a pointer to an array
   * of uint32_t's). The same rules apply considering how many (implicit)
   * params a function takes etc. as if there were less than 33 of them; it's
   * just not as straight-forward.
   */

  uint32_t value;
};

/* <info> can be NULL */
static inline struct params_info count_params(struct node *node, struct params_info *info);

static inline void count_params_nop(struct node *node, struct params_info *info)
{
  /* supress warnings */
  (void)node;
  (void)info;

  return;
}

static inline void count_params_const(struct node *node, struct params_info *info)
{
  /* supress warnings */
  (void)node;
  (void)info;

  return;
}

static inline void count_params_tuple(struct node *node, struct params_info *info)
{
  struct nodes_list *l;

  for (l = node->in.tuple.elems; l != NULL; l = l->next)
    count_params(l->node, info);
}

static inline void count_params_decl(struct node *node, struct params_info *info)
{
  count_params(node->in.decl.var->value, info);
}

static inline void count_params_name(struct node *node, struct params_info *info)
{
  /* see if the name starts with a percent sign */
  if (!strncmp(node->in.s, "%", 1)){
    int params_index = atoi(node->in.s + 1);

    /* set the (params_index-1) nth bit (the (implicit) params start at 1) */
    info->value |= 1 << (params_index - 1);

    /* TODO check for the 32-bit overflow */
  }
}

static inline void count_params_unop(struct node *node, struct params_info *info)
{
  count_params(node->in.unop.target, info);
}

static inline void count_params_binop(struct node *node, struct params_info *info)
{
  count_params(node->in.binop.left, info);
  count_params(node->in.binop.right, info);
}

static inline void count_params_ternop(struct node *node, struct params_info *info)
{
  count_params(node->in.ternop.predicate, info);
  count_params(node->in.ternop.yes, info);
  count_params(node->in.ternop.no, info);
}

static inline void count_params_if(struct node *node, struct params_info *info)
{
  count_params(node->in.iff.guard, info);
  count_params(node->in.iff.body,  info);
  count_params(node->in.iff.elsee, info);
}

static inline void count_params_while(struct node *node, struct params_info *info)
{
  count_params(node->in.whilee.guard, info);
  count_params(node->in.whilee.body,  info);
  count_params(node->in.whilee.elsee, info);
}

static inline void count_params_fun(struct node *node, struct params_info *info)
{
  struct node *expr;

  for (expr = node->in.fun.body; expr != NULL; expr = expr->next)
    count_params(expr, info);
}

static inline void count_params_call(struct node *node, struct params_info *info)
{
  struct nodes_list *arg;

  for (arg = node->in.call.args; arg != NULL; arg = arg->next)
    count_params(arg->node, info);
}

static inline void count_params_use(struct node *node, struct params_info *info)
{
  /* supress warnings */
  (void)node;
  (void)info;

  return;
}

static inline void count_params_print(struct node *node, struct params_info *info)
{
  struct nodes_list *expr;

  for (expr = node->in.print.exprs; expr != NULL; expr = expr->next)
    count_params(expr->node, info);
}

static inline struct params_info count_params(struct node *node, struct params_info *info)
{
  static void (*func_jump_table[])(struct node *, struct params_info *) = {
    [NT_NOP]     = count_params_nop,
    [NT_INTEGER] = count_params_const,
    [NT_REAL]    = count_params_const,
    [NT_STRING]  = count_params_const,
    [NT_CHAR]    = count_params_const,
    [NT_TUPLE]   = count_params_tuple,
    [NT_NAME]    = count_params_name,
    [NT_UNOP]    = count_params_unop,
    [NT_BINOP]   = count_params_binop,
    [NT_TERNOP]  = count_params_ternop,
    [NT_IF]      = count_params_if,
    [NT_WHILE]   = count_params_while,
    [NT_DECL]    = count_params_decl,
    [NT_CALL]    = count_params_call,
    [NT_FUN]     = count_params_fun,
    [NT_USE]     = count_params_use,
    [NT_PRINT]   = count_params_print,
  };

  /* start with no params at all (makes sense, all right) */
  struct params_info ret = { PARAMS_INFO_BITMAP, 0x0 };

  /* use the supplied info if any was provided */
  if (info){
    ret.tag   = info->tag;
    ret.value = info->value;
  } else {
    info = &ret;
  }

  /* execute the appropriate function */
  func_jump_table[node->type](node, info);

  return ret;
}

#endif /* COUNT_PARAMS_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

