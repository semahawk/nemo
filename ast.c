/*
 *
 * ast.c
 *
 * Created at:  Fri 15 Nov 22:28:17 2013 22:28:17
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "ast.h"
#include "debug.h"
#include "mem.h"
#include "infnum.h"
#include "nob.h"
#include "lexer.h"
#include "util.h"
#include "utf8.h"

/* two handy macros */
#define EXEC(node) ((node)->execf(node))
#define COMP(node) ((node)->compf(node))
/* few more */
#define RETURN_NEXT return (NM_pc = nd->next, NM_pc)
#define RETURN return (NM_pc)
/* returns <node>'s id, or 0 if the node is none */
#define NDID(node) ((node) == NULL ? 0 : (node)->id)

/* program counter */
static struct node *NM_pc = NULL;

/* the argument stack */
static Nob  **NM_as      = NULL;
static Nob  **NM_as_curr = NULL;
static size_t NM_as_size = 16;

/* the current node's id */
static unsigned currid = 1;

/* the current generic label id */
static unsigned currlabelid = 0;

/* buffers for the assembly sections (the `out` functions writes into them) */
struct section text  = { { 0 }, 0 };
struct section data  = { { 0 }, 0 };
struct section bss   = { { 0 }, 0 };
struct section funcs = { { 0 }, 0 };
/* the current section we are writing to */
struct section *currsect = &text;

/* {{{ argument stack manipulation functions */
void arg_stack_init(void)
{
  NM_as = ncalloc(NM_as_size, sizeof(Nob *));
  NM_as_curr = NM_as;
}

void arg_stack_finish(void)
{
  nfree(NM_as);
}

void arg_stack_push(Nob *ob, const char *file, unsigned line)
{
  ptrdiff_t offset = NM_as_curr - NM_as;
  /* hmm, will they be ever used? */
  (void)file;
  (void)line;

  /* handle overflow */
  if (offset >= (signed)NM_as_size){
    NM_as_size *= 1.5;
    NM_as = nrealloc(NM_as, sizeof(int *) * NM_as_size);
    NM_as_curr = NM_as + offset; /* adjust the 'current' pointer */
  }

  *NM_as_curr = ob; /* set up the current `cell' */
  NM_as_curr++; /* move on to the next `cell' */
}

Nob *arg_stack_pop(const char *file, unsigned line)
{
  ptrdiff_t offset = NM_as_curr - NM_as;

  if (offset < 0){
    fprintf(stderr, "nemo: argument stack underflow! in %s line %u\n", file, line);
    exit(1);
  }

  NM_as_curr--;

  return *NM_as_curr;
}

Nob *arg_stack_top(void)
{
  return *(NM_as_curr - 1);
}

void arg_stack_dump(void)
{
  int i = 0;

  printf("\n## Stack dump:\n");
  for (; i < NM_as_curr - NM_as; i++){
    printf("  %x - %p (%s)", i, (void *)NM_as[i], nob_type_to_s(NM_as[i]->type->primitive));
    if (NM_as[i]->type->primitive == OT_INTEGER){
      putchar(' ');
      infnum_print(NOB_GET_INTEGER(NM_as[i]), stdout);
    }
    if (&NM_as[i] == NM_as_curr)
      printf(" <<<");
    printf("\n");
  }
  printf("## END\n\n");
}
/* }}} */

/* new_node
 *
 * Create a new node and append it to the <lex>'s `nodes` list.
 *
 */
static struct node *new_node(struct parser *parser, struct lexer *lex)
{
  /* {{{ */
  struct nodes_list *el = nmalloc(sizeof(struct nodes_list));
  struct node *new = nmalloc(sizeof(struct node));

  /* set the node's default values */
  new->id = currid++;
  new->next = NULL;
  new->result_type = NULL;
  new->scope = parser->curr_scope;
  /* associate the node with the list's element */
  el->node = new;
  /* append to the lexer's list */
  el->next = lex->nodes;
  lex->nodes = el;

  return new;
  /* }}} */
}

#if DEBUG
/* {{{ dump macros */
/* maintaining the proper amount of spaces before the node's dump */
static int nodes_sw = 3;
#define INDENT() do { nodes_sw += 2; } while (0)
#define DEDENT() do { nodes_sw -= 2; } while (0)
#define SPACES() do { \
  int i; \
  /* print the proper amount of spaces */ \
  for (i = 0; i < nodes_sw; putchar(' '), i++); \
} while (0)

#define DUMP(node) do { \
  SPACES(); \
  if ((node) == NULL) \
    printf("+ (null)\n"); \
  else \
    (node)->dumpf((node)); \
} while (0)

#define DUMPP(msg) do { \
  SPACES(); \
  /* meh, anonymous variadic macros were introduced in C99 */ \
  puts(msg); \
} while (0)
/* }}} */
/* {{{ dump_nodes */
void dump_nodes(struct node *node)
{
  if (NM_DEBUG_GET_FLAG(NM_DEBUG_AST)){
    printf("\n## AST Nodes Dump:\n\n");

    for (; node != NULL; node = node->next){
      DUMP(node);
    }

    printf("\n## End\n");
  }
}

void dump_nop(struct node *nd)
{
  printf("+ (#%u) nop\n", nd->id);
}

void dump_const(struct node *nd)
{
  if (nd->type == NT_INTEGER){
    printf("+ (#%u) const (integer ", nd->id);
    infnum_print(nd->in.i, stdout);
    printf(")\n");
  } else if (nd->type == NT_CHAR)
    printf("+ (#%u) const (char %lc)\n", nd->id, nd->in.c);
  else
    printf("+ (#%u) const\n", nd->id);
}

void dump_list(struct node *nd)
{
  printf("+ (#%u) list\n", NDID(nd));

  for (struct nodes_list *p = nd->in.list.elems; p != NULL; p = p->next){
    INDENT();
    DUMPP("- elem:");
    INDENT();
    DUMP(p->node);
    DEDENT();
    DEDENT();
  }
}

void dump_tuple(struct node *nd)
{
  printf("+ (#%u) tuple\n", NDID(nd));

  for (struct nodes_list *p = nd->in.list.elems; p != NULL; p = p->next){
    INDENT();
    DUMPP("- elem:");
    INDENT();
    DUMP(p->node);
    DEDENT();
    DEDENT();
  }
}

void dump_name(struct node *nd)
{
  printf("+ (#%u) name (%s)\n", NDID(nd), nd->in.s);
}

void dump_decl(struct node *nd)
{
  printf("+ (#%u) declaration (%s, 0x%02x)\n", nd->id, nd->in.decl.var->name,
      nd->in.decl.var->flags);

  if (nd->in.decl.var->value){
    INDENT();
    DUMPP("- value:");
    INDENT();
    DUMP(nd->in.decl.var->value);
    DEDENT();
    DEDENT();
  }
}

void dump_unop(struct node *nd)
{
  printf("+ (#%u) unop ", nd->id);

  switch (nd->in.unop.type){
    case UNARY_PLUS:
      printf("'+'");
      break;
    case UNARY_MINUS:
      printf("'-'");
      break;
    case UNARY_LOGNEG:
      printf("'!'");
      break;
    case UNARY_BITNEG:
      printf("'~'");
      break;
    case UNARY_PREINC:
      printf("prefix '++'");
      break;
    case UNARY_PREDEC:
      printf("prefix '--'");
      break;
    case UNARY_POSTINC:
      printf("postfix '++'");
      break;
    case UNARY_POSTDEC:
      printf("postfix '--'");
      break;
    default:
      printf(" -- unknown");
      break;
  }
  printf("\n");

  INDENT();
  DUMP(nd->in.unop.target);
  DEDENT();
}

void dump_binop(struct node *nd)
{
  printf("+ (#%u) binop '%s'\n", nd->id, binop_to_s(nd->in.binop.type));
  INDENT();
  DUMP(nd->in.binop.left);
  DUMP(nd->in.binop.right);
  DEDENT();
}

void dump_ternop(struct node *nd)
{
  printf("+ (#%u) ternop\n", nd->id);
  INDENT();
  DUMPP("- predicate:");
  INDENT();
  DUMP(nd->in.ternop.predicate);
  DEDENT();
  DUMPP("- 'true' branch:");
  INDENT();
  DUMP(nd->in.ternop.yes);
  DEDENT();
  DUMPP("- 'false' branch:");
  INDENT();
  DUMP(nd->in.ternop.no);
  DEDENT();
}

void dump_if(struct node *nd)
{
  printf("+ (#%u) if\n", nd->id);
  INDENT();
  DUMPP("- guard:");
  INDENT();
  DUMP(nd->in.iff.guard);
  DEDENT();
  DUMPP("- body:");
  INDENT();
  DUMP(nd->in.iff.body);
  DEDENT();
  DUMPP("- else:");
  INDENT();
  DUMP(nd->in.iff.elsee);
  DEDENT();
  DEDENT();
}

void dump_fun(struct node *nd)
{
  struct node *d = nd->in.fun.body;
  struct types_list *p = nd->result_type->info.func.params;

  if (nd->in.fun.name)
    printf("+ (#%u) fun (%s)\n", nd->id, nd->in.fun.name);
  else
    printf("+ (#%u) lambda\n", nd->id);

  INDENT();
  DUMPP("- body:");
  INDENT();

  do {
    DUMP(d);
  } while ((d = d->next) != NULL);

  DEDENT();

  if (p){
    DUMPP("- params:");
    INDENT();

    while (p != NULL){
      SPACES();
      /*print_type(p->type);*/
      printf("\n");
      p = p->next;
    }
  } else {
    DUMPP("- no params");
  }

  DEDENT();
  DEDENT();
}

void dump_call(struct node *nd)
{
  printf("+ (#%u) call\n", nd->id);

  INDENT();
  DUMPP("- target function:");
  INDENT();
  SPACES();
  if (nd->in.call.fun->in.fun.name)
    printf("+ (#%u) fun (%s)\n", NDID(nd->in.call.fun), nd->in.call.fun->in.fun.name);
  else
    printf("+ (#%u) lambda\n", NDID(nd->in.call.fun));

  DEDENT();
  DUMPP("- args:");
  INDENT();
  DUMPP("(not yet implemented)");
  DEDENT();
  DEDENT();
}

void dump_print(struct node *nd)
{
  struct nodes_list *expr;

  printf("+ (#%u) print\n", nd->id);
  INDENT();

  for (expr = nd->in.print.exprs; expr != NULL; expr = expr->next){
    DUMPP("- expr:");
    INDENT();
    DUMP(expr->node);
    DEDENT();
  }

  DEDENT();
}
/* }}} */
#endif /* DEBUG */

/* {{{ exec_nodes */
void exec_nodes(struct node *node)
{
  NM_pc = node;

  if (NM_pc)
    while (EXEC(NM_pc))
      ;
}

struct node *exec_nop(struct node *nd)
{
  /* {{{ */
  debug_ast_exec(nd, "nop");

  RETURN_NEXT;
  /* }}} */
}

struct node *exec_const(struct node *nd)
{
  /* {{{  */
  if (nd->type == NT_INTEGER){
    debug_ast_exec(nd, "integer");
    PUSH(new_nob(T_INT, nd->in.i));
  } else if (nd->type == NT_REAL){
    debug_ast_exec(nd, "real (%g)", nd->in.f);
    PUSH(new_nob(T_REAL, nd->in.f));
  } else if (nd->type == NT_CHAR){
    debug_ast_exec(nd, "char (%lc)", nd->in.c);
    PUSH(new_nob(T_CHAR, nd->in.c));
  }

  RETURN_NEXT;
  /* }}} */
}

struct node *exec_list(struct node *nd)
{
  /* {{{ */
  struct nobs_list *nobs = NULL;
  struct nodes_list *nodes;
  struct nobs_list *el;

  /* transform the nodes_list into nobs_list */
  for (nodes = nd->in.list.elems; nodes != NULL; nodes = nodes->next){
    el = nmalloc(sizeof(struct nobs_list));

    EXEC(nodes->node);
    el->nob = POP();
    el->next = nobs;
    nobs = el;
  }

  PUSH(new_nob(new_type(NULL, OT_LIST, /* FIXME  */ T_INT), reverse_nodes_list(nobs)));

  RETURN_NEXT;
  /* }}} */
}

struct node *exec_tuple(struct node *nd)
{
  /* {{{ */
  struct nobs_list *nobs = NULL;
  struct nodes_list *nodes;
  struct nobs_list *el;

  /* transform the nodes_list into nobs_list */
  for (nodes = nd->in.list.elems; nodes != NULL; nodes = nodes->next){
    el = nmalloc(sizeof(struct nobs_list));

    EXEC(nodes->node);
    el->nob = POP();
    el->next = nobs;
    nobs = el;
  }

  /* TODO */
  PUSH(new_nob(/* FIXME */ T_INT, reverse_nodes_list(nobs)));

  RETURN_NEXT;
  /* }}} */
}

struct node *exec_name(struct node *nd)
{
  /* {{{  */
  struct var *var = var_lookup(nd->in.s, nd->scope);

  if (var)
    EXEC(var->value);
  else {
    fprintf(stderr, "variable '%s' not found! runtime!!\n", nd->in.s);
    exit(1);
  }

  RETURN_NEXT;
  /* }}} */
}

struct node *exec_decl(struct node *nd)
{
  /* {{{ */
  if (nd->in.decl.var->value)
    debug_ast_exec(nd, "declaration (%s, 0x%02x, #%u)", nd->in.decl.var->name,
        nd->in.decl.var->flags, nd->in.decl.var->value->id);
  else
    debug_ast_exec(nd, "declaration (%s, 0x%02x, #--)", nd->in.decl.var->name,
        nd->in.decl.var->flags);

  if (nd->in.decl.var->value)
    EXEC(nd->in.decl.var->value);

  /* TODO associate the value with the variable */

  RETURN_NEXT;
  /* }}} */
}

struct node *exec_unop(struct node *nd)
{
  /* {{{ */
  EXEC(nd->in.unop.target);

  debug_ast_exec(nd, "unop ('op?', #%u)", NDID(nd->in.unop.target));

  switch (nd->in.unop.type){
    case UNARY_MINUS:
      /* simply set the top argument's sign to 'minus' */
      NOB_GET_INTEGER(TOP()).sign = INFNUM_SIGN_NEG;
      break;
    default: /* WIP */;
  }

  RETURN_NEXT;
  /* }}} */
}

struct node *exec_binop(struct node *nd)
{
  /* {{{ */
  Nob *left, *right;

  EXEC(nd->in.binop.left);
  EXEC(nd->in.binop.right);

  debug_ast_exec(nd, "binop ('%s', #%u, #%u)", binop_to_s(nd->in.binop.type),
      nd->in.binop.left->id, nd->in.binop.right->id);

  right = POP();
  left  = POP();

  assert(left->ptr != NULL);
  assert(right->ptr != NULL);

  switch (nd->in.binop.type){
    case BINARY_ADD:
      PUSH(new_nob(T_INT, infnum_add(NOB_GET_INTEGER(left), NOB_GET_INTEGER(right))));
      break;
    case BINARY_SUB:
      PUSH(new_nob(T_INT, infnum_sub(NOB_GET_INTEGER(left), NOB_GET_INTEGER(right))));
      break;
    case BINARY_MUL:
      PUSH(new_nob(T_INT, infnum_mul(NOB_GET_INTEGER(left), NOB_GET_INTEGER(right))));
      break;
    case BINARY_DIV:
      PUSH(new_nob(T_INT, infnum_div_by_small(NOB_GET_INTEGER(left), NOB_GET_INTEGER(right).digits[0])));
      break;
    case BINARY_MOD:
      PUSH(new_nob(T_INT, infnum_mod(NOB_GET_INTEGER(left), NOB_GET_INTEGER(right))));
      break;
    case BINARY_SHL:
      PUSH(new_nob(T_INT, infnum_shl_by_small(NOB_GET_INTEGER(left), NOB_GET_INTEGER(right).digits[0])));
      break;
    case BINARY_SHR:
      PUSH(new_nob(T_INT, infnum_shr_by_small(NOB_GET_INTEGER(left), NOB_GET_INTEGER(right).digits[0])));
      break;
    case BINARY_BITAND:
      PUSH(new_nob(T_INT, infnum_and(NOB_GET_INTEGER(left), NOB_GET_INTEGER(right))));
      break;
    case BINARY_BITXOR:
      PUSH(new_nob(T_INT, infnum_xor(NOB_GET_INTEGER(left), NOB_GET_INTEGER(right))));
      break;
    case BINARY_BITOR:
      PUSH(new_nob(T_INT, infnum_or(NOB_GET_INTEGER(left), NOB_GET_INTEGER(right))));
      break;

    /* fall through */
    case BINARY_GT:
    case BINARY_LT:
    case BINARY_GE:
    case BINARY_LE:
    case BINARY_EQ:
    case BINARY_NE:
    case BINARY_ASSIGN:
    case BINARY_ASSIGN_ADD:
    case BINARY_ASSIGN_SUB:
    case BINARY_ASSIGN_MUL:
    case BINARY_ASSIGN_DIV:
    case BINARY_ASSIGN_MOD:
    case BINARY_COMMA:
      PUSH(new_nob(T_INT, infnum_from_dword(0)));
      break;
    default: /* meh */;
  }

  RETURN_NEXT;
  /* }}} */
}

struct node *exec_ternop(struct node *nd)
{
  /* {{{ */
  Nob *predicate;

  debug_ast_exec(nd, "ternop (#%u, #%u, #%u)", nd->in.ternop.predicate->id,
      nd->in.ternop.yes->id, nd->in.ternop.no->id);

  EXEC(nd->in.ternop.predicate);
  predicate = POP();

  if (nob_is_true(predicate)){
    EXEC(nd->in.ternop.yes);
  } else {
    EXEC(nd->in.ternop.no);
  }

  RETURN_NEXT;
  /* }}} */
}

struct node *exec_if(struct node *nd)
{
  /* {{{ */
  Nob *guard;

  if (nd->in.iff.elsee != NULL)
    debug_ast_exec(nd, "if (#%u, #%u, #%u)",
      nd->in.iff.guard->id,
      nd->in.iff.body->id,
      nd->in.iff.elsee->id);
  else
    debug_ast_exec(nd, "if (#%u, #%u, --)",
      nd->in.iff.guard->id,
      nd->in.iff.body->id);

  EXEC(nd->in.iff.guard);

  guard = TOP();

  if (guard)
    EXEC(nd->in.iff.body);
  else
    if (nd->in.iff.elsee != NULL)
      EXEC(nd->in.iff.elsee);

  RETURN_NEXT;
  /* }}} */
}

struct node *exec_fun(struct node *nd)
{
  /* {{{ */
  struct node *e;

  if (nd->in.fun.execute){
    debug_ast_exec(nd, "executing a function");

    for (e = nd->in.fun.body; e != NULL; e = e->next){
      EXEC(e);

      /* leave only the last expression on the stack */
      /* which effectively makes it the function's return value */
      if (e->next != NULL)
        POP();
    }
  }

  RETURN_NEXT;
  /* }}} */
}

struct node *exec_call(struct node *nd)
{
  /* {{{ */
  debug_ast_exec(nd, "function call (not yet implemented)");

  RETURN_NEXT;
  /* }}} */
}

/*
 * Prints a single <ob> to stdout.
 *
 * It's kind of for development purposes only.
 */
void print_nob(Nob *ob)
{
  /* {{{ */
  switch (ob->type->primitive){
    case OT_INTEGER:
      infnum_print(NOB_GET_INTEGER(ob), stdout);
      break;
    case OT_CHAR:
      printf("%lc", NOB_GET_CHAR(ob));
      break;
    case OT_REAL:
      printf("%g", NOB_GET_REAL(ob));
      break;
    case OT_LIST:
      printf("[");

      for (struct nobs_list *p = (struct nobs_list *)ob->ptr; p != NULL; p = p->next){
        print_nob(p->nob);

        if (p->next != NULL)
          printf(", ");
      }

      printf("]");
      break;
    case OT_TUPLE:
      printf("(");

      for (struct nobs_list *p = (struct nobs_list *)ob->ptr; p != NULL; p = p->next){
        print_nob(p->nob);

        if (p->next != NULL)
          printf(", ");
      }

      printf(")");
      break;

    /* fall through */
    case OT_STRING:
    case OT_FUN:
    case OT_TYPE_VARIABLE:
      break;
  }
  /* }}} */
}

struct node *exec_print(struct node *nd)
{
  /* {{{ */
  struct nodes_list *expr;

  debug_ast_exec(nd, "print");

  for (expr = nd->in.print.exprs; expr != NULL; expr = expr->next){
    EXEC(expr->node);
    print_nob(POP());
  }

  PUSH(new_nob(T_INT, infnum_from_dword(1)));

  RETURN_NEXT;
  /* }}} */
}
/* }}} */
/* {{{ comp_nodes */
void comp_nodes(struct node *node)
{
  struct node **func;
  unsigned vars_size = size_of_vars(node->scope);

  NM_pc = node;

  out("section .text");
  out("_kernel:");
  out("  int 0x80");
  out("  ret\n");
  out("global _start");
  out("_start:");

  if (vars_size > 0){
    out("  push ebp");
    out("  mov ebp, esp");
    out("  sub esp, %d", vars_size);
  }

  if (NM_pc)
    while (COMP(NM_pc))
      ;

  if (vars_size > 0)
    out("  leave", vars_size);

  out("  push dword eax");
  out("  mov eax, 1");
  out("  call _kernel\n");

  fprintf(outfile, "%s", text.buffer);
  fprintf(outfile, "%s", funcs.buffer);
  fprintf(outfile, "%s", bss.buffer);
  fprintf(outfile, "%s", data.buffer);

  /* suck it Emacs (: */
  fprintf(outfile, "; vim: ft=nasm:ts=2:sw=2 expandtab\n\n");
}

struct node *comp_nop(struct node *nd)
{
  /* {{{ */
  debug_ast_comp(nd, "nop");

  RETURN_NEXT;
  /* }}} */
}

struct node *comp_const(struct node *nd)
{
  /* {{{  */
  if (nd->type == NT_INTEGER){
    debug_ast_comp(nd, "integer");
    out("  mov eax, %d", nd->in.i.digits[0]);
    PUSH(new_nob(T_INT, nd->in.i));
  } else if (nd->type == NT_REAL){
    debug_ast_comp(nd, "real (%g)", nd->in.f);
    /*PUSH(new_nob(T_REAL, nd->in.f));*/
  } else if (nd->type == NT_CHAR){
    debug_ast_comp(nd, "char (%lc)", nd->in.c);
    out("  mov eax, %d", nd->in.c);
    PUSH(new_nob(T_CHAR, nd->in.c));
  }

  RETURN_NEXT;
  /* }}} */
}

struct node *comp_list(struct node *nd)
{
  /* {{{ */
  printf("lists not yet fully implemented\n");

  RETURN_NEXT;
  /* }}} */
}

struct node *comp_tuple(struct node *nd)
{
  /* {{{ */
  printf("tuples not yet implemented\n");

  RETURN_NEXT;
  /* }}} */
}

struct node *comp_name(struct node *nd)
{
  /* {{{  */
  struct var *var = var_lookup(nd->in.s, nd->scope);
  char *base;

  if (!var){
    fprintf(stderr, "variable '%s' not found! compile time!\n", nd->in.s);
    exit(1);
  }

  if (var->decl != NULL && nd->scope != var->decl->scope){
    base = "ecx";
  } else {
    base = "ebp";
  }

  if (var->offset == 0)
    out("  mov eax, [%s] ; loading %s", base, var->name);
  else
    out("  mov eax, [%s %+d] ; loading %s", base, var->offset, var->name);

  RETURN_NEXT;
  /* }}} */
}

struct node *comp_decl(struct node *nd)
{
  /* {{{ */
  if (nd->in.decl.var->value)
    debug_ast_comp(nd, "declaration (%s, 0x%02x, #%u)", nd->in.decl.var->name,
        nd->in.decl.var->flags, nd->in.decl.var->value->id);
  else
    debug_ast_comp(nd, "declaration (%s, 0x%02x, #--)", nd->in.decl.var->name,
        nd->in.decl.var->flags);

  /*printf("var's offset: %u\n", nd->in.decl.var->offset);*/

  if (nd->in.decl.var->value)
    COMP(nd->in.decl.var->value);

  if (nd->in.decl.var->offset == 0)
    out("  mov [ebp], eax ; declaring %s", nd->in.decl.var->name);
  else
    out("  mov [ebp %+d], eax ; declaring %s", nd->in.decl.var->offset, nd->in.decl.var->name);

  RETURN_NEXT;
  /* }}} */
}

struct node *comp_unop(struct node *nd)
{
  /* {{{ */
  COMP(nd->in.unop.target);

  debug_ast_comp(nd, "unop ('op?', #%u)", NDID(nd->in.unop.target));

  switch (nd->in.unop.type){
    case UNARY_MINUS:
      out("  neg eax");
      break;
    case UNARY_PREINC:
    case UNARY_POSTINC: /* FIXME */
      out("  inc eax");
      break;
    case UNARY_PREDEC:
    case UNARY_POSTDEC: /* FIXME */
      out("  dec eax");
      break;
    default: /* WIP */;
  }

  RETURN_NEXT;
  /* }}} */
}

struct node *comp_binop(struct node *nd)
{
  /* {{{ */
  Nob *value;

  debug_ast_comp(nd, "binop ('%s', #%u, #%u)", binop_to_s(nd->in.binop.type),
      nd->in.binop.left->id, nd->in.binop.right->id);

#define PRIMITIVE_BINOP(func)   \
  out("  push ebx");            \
  COMP(nd->in.binop.right);     \
  out("  mov ebx, eax");        \
  COMP(nd->in.binop.left);      \
  out("  " func " eax, ebx");   \
  out("  pop ebx\n");

#define PRIMITIVE_COMPARE(func) \
  out("  push ebx");            \
  COMP(nd->in.binop.right);     \
  out("  mov ebx, eax");        \
  COMP(nd->in.binop.left);      \
  out("  cmp eax, ebx");        \
  out("  " func " al");         \
  out("  movsx eax, al");       \
  out("  pop ebx\n");

  switch (nd->in.binop.type){
    case BINARY_ADD:
      PRIMITIVE_BINOP("add");
      break;
    case BINARY_SUB:
      PRIMITIVE_BINOP("sub");
      break;
    case BINARY_MUL:
      PRIMITIVE_BINOP("imul");
      break;
    case BINARY_DIV:
      out("  push edx");
      out("  xor edx, edx");
      COMP(nd->in.binop.right);
      out("  mov ebx, eax");
      COMP(nd->in.binop.left);
      out("  idiv ebx");
      out("  pop edx\n");
      break;
    case BINARY_MOD:
      out("  push edx");
      out("  xor edx, edx");
      COMP(nd->in.binop.right);
      out("  mov ebx, eax");
      COMP(nd->in.binop.left);
      out("  idiv ebx");
      out("  mov eax, edx");
      out("  pop edx\n");
      break;
    case BINARY_BITAND:
      PRIMITIVE_BINOP("and");
      break;
    case BINARY_BITXOR:
      PRIMITIVE_BINOP("xor");
      break;
    case BINARY_BITOR:
      PRIMITIVE_BINOP("or");
      break;
    case BINARY_EQ:
      PRIMITIVE_COMPARE("sete");
      break;
    case BINARY_NE:
      PRIMITIVE_COMPARE("setne");
      break;
    case BINARY_LT:
      PRIMITIVE_COMPARE("setl");
      break;
    case BINARY_LE:
      PRIMITIVE_COMPARE("setle");
      break;
    case BINARY_GT:
      PRIMITIVE_COMPARE("setg");
      break;
    case BINARY_GE:
      PRIMITIVE_COMPARE("setge");
      break;
    case BINARY_SHL:
      COMP(nd->in.binop.right);
      value = POP();
      COMP(nd->in.binop.left);
      /* MIN(255, ...) because `shl` and `shr` accept either the `cl` register
       * or a __8-bit value__ */
      out("  shl eax, %d", MIN(0xff, NOB_GET_INTEGER(value).digits[0]));
      break;
    case BINARY_SHR:
      COMP(nd->in.binop.right);
      value = POP();
      COMP(nd->in.binop.left);
      out("  shr eax, %d", MIN(0xff, NOB_GET_INTEGER(value).digits[0]));
      break;

    /* fall through */
    case BINARY_ASSIGN:
    case BINARY_ASSIGN_ADD:
    case BINARY_ASSIGN_SUB:
    case BINARY_ASSIGN_MUL:
    case BINARY_ASSIGN_DIV:
    case BINARY_ASSIGN_MOD:
    case BINARY_COMMA:
      printf("nop (not implemented yet)\n");
      PUSH(new_nob(T_INT, infnum_from_dword(0)));
      break;
    default: /* meh */;
  }

  RETURN_NEXT;
  /* }}} */
}

struct node *comp_ternop(struct node *nd)
{
  /* {{{ */
  debug_ast_comp(nd, "ternop (#%u, #%u, #%u)", nd->in.ternop.predicate->id,
      nd->in.ternop.yes->id, nd->in.ternop.no->id);

  printf("ternary ops not yet implemented\n");

  RETURN_NEXT;
  /* }}} */
}

struct node *comp_if(struct node *nd)
{
  /* {{{ */
  debug_ast_comp(nd, "if (#%u, #%u, #%u)",
    nd->in.iff.guard->id,
    nd->in.iff.body->id,
    nd->in.iff.elsee->id);

  COMP(nd->in.iff.guard);

  out("  test eax, eax");
  out("  jz .l%d", currlabelid);
  out("  ; the 'true' branch");
  COMP(nd->in.iff.body);
  out("  jmp .l%d", currlabelid + 1);
  out(".l%d:", currlabelid);
  out("  ; the 'false' branch");
  COMP(nd->in.iff.elsee);

  out(".l%d:", currlabelid + 1);

  currlabelid++;

  RETURN_NEXT;
  /* }}} */
}

struct node *comp_fun(struct node *nd)
{
  /* {{{ */
  struct node *expr;
  unsigned vars_size = size_of_vars(nd->scope);

  debug_ast_comp(nd, "function");

  /* a buffer for the functions */
  struct node *functions[64] = { NULL };
  struct node **currfun = functions;
  struct node **fun;

  /*if (nd->in.fun.name)*/
    /*printf("function's %s scope is %s, %p\n", nd->in.fun.name, nd->scope->name, (void*)nd->scope);*/
  /*else*/
    /*printf("function's _f%d scope is %s, %p\n", NDID(nd), nd->scope->name, (void*)nd->scope);*/

  /*printf("size of vars: %u\n", vars_size);*/

  if (!nd->in.fun.compiled){
    currsect = &funcs;
    nd->in.fun.compiled = true;

    /* print out the function's name (or a generated handle for anonymous) */
    if (nd->in.fun.name)
      out("%s:", nd->in.fun.name);
    else
      out("_f%d:", NDID(nd));

    /* set up the stack frame for the function */
    out("  push ebp");
    out("  mov ebp, esp");

    if (vars_size > 0){
      out("  sub esp, %d", vars_size);
    }

    for (expr = nd->in.fun.body; expr != NULL; expr = expr->next){
      /* if the expression in the function's body is also a function we have to
       * wait with it a bit and write it to the assembly file after with dealt
       * with the current function */
      if (expr->type == NT_FUN){
        /* TODO: FIXME: check for overflow */
        *(currfun++) = expr;
      } else if (expr->type == NT_CALL){
        /* TODO: FIXME: check for overflow */
        *(currfun++) = expr->in.call.fun;
        /* set this to true so that the body of the function doesn't get
         * written in the midway */
        expr->in.call.fun->in.fun.compiled = true;
        /* we call this because we need the `call` to the function */
        COMP(expr);
        /* set this to false again so at the end of this function the body gets
         * generated */
        expr->in.call.fun->in.fun.compiled = false;
      } else {
        COMP(expr);

        /* leave only the last expression on the stack */
        /* which effectively makes it the function's return value */
        /* NOTE: hmmmm... is that right here? */
        if (expr->next != NULL)
          POP();
      }
    }

    out("  leave");
    out("  ret\n");

    currsect = &text;
  }

  if (nd->in.fun.name)
    out("  mov eax, %s", nd->in.fun.name);
  else
    out("  mov eax, _f%d", NDID(nd));

  /* compile all the functions that were defined/declared inside of the current
   * one */
  for (fun = functions; *fun != NULL; fun++)
    COMP(*fun);

  RETURN_NEXT;
  /* }}} */
}

struct node *comp_call(struct node *nd)
{
  /* {{{ */
  unsigned args_size = 0;

  assert(nd->in.call.fun);

  debug_ast_comp(nd, "compiling a function call (#%u)", NDID(nd->in.call.fun));

  /* calculate how many variables the function we're about to call has (well,
   * not how many but rather how much of them there is) */
  /*vars_size = size_of_vars(nd->in.call.fun->scope);*/

  for (struct nodes_list *p = nd->in.call.args; p != NULL; p = p->next){
    args_size += p->node->result_type->size;
    COMP(p->node);
    out("  push eax");
  }

  /* make sure the function is actually defined in the assembly file */
  /* and that eax is loaded with the function's address */
  /*if (!nd->in.call.fun->in.fun.compiled)*/
    COMP(nd->in.call.fun);

  /* store the current stack frame for the function to use */
  /* if the function has a parent it means it's a nested function */
  /*if (nd->in.call.fun->scope->parent)*/
    out("  lea ecx, [ebp]");

  /* make the call */
  out("  call eax");

  /* remove arguments from the frame */
  if (args_size > 0)
    out("  add esp, %d", args_size);

  RETURN_NEXT;
  /* }}} */
}

struct node *comp_print(struct node *nd)
{
  /* {{{ */
  debug_ast_comp(nd, "print");

  PUSH(new_nob(T_INT, infnum_from_dword(1)));

  RETURN_NEXT;
  /* }}} */
}
/* }}} */
/* {{{ new_nodes */
struct node *new_nop(struct parser *parser, struct lexer *lex)
{
  /* {{{ */
  struct node *nd = new_node(parser, lex);

  nd->type = NT_NOP;
  nd->execf = exec_nop;
  nd->compf = comp_nop;
#if DEBUG
  nd->dumpf = dump_nop;
#endif

  debug_ast_new(nd, "nop");

  return nd;
  /* }}} */
}

struct node *new_int(struct parser *parser, struct lexer *lex, struct infnum value)
{
  /* {{{ */
  struct node *nd = new_node(parser, lex);

  nd->type = NT_INTEGER;
  nd->result_type = T_INT;
  nd->in.i = value;
  nd->execf = exec_const;
  nd->compf = comp_const;
#if DEBUG
  nd->dumpf = dump_const;
#endif
  nd->result_type = T_INT;

  debug_ast_new(nd, "integer");

  return nd;
  /* }}} */
}

struct node *new_char(struct parser *parser, struct lexer *lex, nchar_t value)
{
  /* {{{ */
  struct node *nd = new_node(parser, lex);

  nd->type = NT_CHAR;
  nd->result_type = T_CHAR;
  nd->in.c = value;
  nd->execf = exec_const;
  nd->compf = comp_const;
#if DEBUG
  nd->dumpf = dump_const;
#endif

  debug_ast_new(nd, "char (%lc) ", value);

  return nd;
  /* }}} */
}

struct node *new_real(struct parser *parser, struct lexer *lex, double value)
{
  /* {{{ */
  struct node *nd = new_node(parser, lex);

  nd->type = NT_REAL;
  nd->result_type = T_REAL;
  nd->in.f = value;
  nd->execf = exec_const;
  nd->compf = comp_const;
#if DEBUG
  nd->dumpf = dump_const;
#endif

  debug_ast_new(nd, "real (%g) ", value);

  return nd;
  /* }}} */
}

struct node *new_list(struct parser *parser, struct lexer *lex,
    struct nodes_list *elems)
{
  /* {{{ */
  struct node *nd = new_node(parser, lex);

  nd->type = NT_LIST;
  nd->execf = exec_list;
  nd->compf = comp_list;
  nd->in.list.elems = elems;
#if DEBUG
  nd->dumpf = dump_list;
#endif

  debug_ast_new(nd, "list");

  return nd;
  /* }}} */
}

struct node *new_tuple(struct parser *parser, struct lexer *lex,
    struct nodes_list *elems)
{
  /* {{{ */
  struct node *nd = new_node(parser, lex);

  nd->type = NT_TUPLE;
  nd->in.list.elems = elems;
  nd->execf = exec_tuple;
  nd->compf = comp_tuple;
#if DEBUG
  nd->dumpf = dump_tuple;
#endif

  debug_ast_new(nd, "tuple");

  return nd;
  /* }}} */
}

struct node *new_name(struct parser *parser, struct lexer *lex, char *name)
{
  /* {{{ */
  struct node *nd = new_node(parser, lex);

  nd->type = NT_NAME;
  nd->in.s = strdup(name);
  nd->execf = exec_name;
  nd->compf = comp_name;
#if DEBUG
  nd->dumpf = dump_name;
#endif

  debug_ast_new(nd, "name (%s)", name);

  return nd;
  /* }}} */
}

struct node *new_decl(struct parser *parser, struct lexer *lex, struct var *var)
{
  /* {{{ */
  struct node *nd = new_node(parser, lex);

  nd->type = NT_DECL;
  nd->in.decl.var = var;
  nd->execf = exec_decl;
  nd->compf = comp_decl;
#if DEBUG
  nd->dumpf = dump_decl;
#endif

  debug_ast_new(nd, "declaration (#%u, 0x%02x) ", NDID(var->value), var->flags);

  return nd;
  /* }}} */
}

struct node *new_unop(struct parser *parser, struct lexer *lex, enum unop_type type,
    struct node *target)
{
  /* {{{ */
  struct node *nd = new_node(parser, lex);

  nd->type = NT_UNOP;
  nd->in.unop.type = type;
  nd->in.unop.target = target;
  nd->execf = exec_unop;
  nd->compf = comp_unop;
#if DEBUG
  nd->dumpf = dump_unop;
#endif

  debug_ast_new(nd, "unop ('op?', #%u)", NDID(nd->in.unop.target));

  return nd;
  /* }}} */
}

struct node *new_binop(struct parser *parser, struct lexer *lex, enum binop_type type,
    struct node *left, struct node *right)
{
  /* {{{ */
  struct node *nd = new_node(parser, lex);

  nd->type = NT_BINOP;
  nd->in.binop.type = type;
  nd->in.binop.left = left;
  nd->in.binop.right = right;
  nd->execf = exec_binop;
  nd->compf = comp_binop;
#if DEBUG
  nd->dumpf = dump_binop;
#endif

  debug_ast_new(nd, "binop ('%s', #%u, #%u)", binop_to_s(nd->in.binop.type),
      NDID(nd->in.binop.left), NDID(nd->in.binop.right));

  return nd;
  /* }}} */
}

struct node *new_ternop(struct parser *parser, struct lexer *lex, struct node *predicate,
    struct node *yes, struct node *no)
{
  /* {{{ */
  struct node *nd = new_node(parser, lex);

  nd->type = NT_TERNOP;
  nd->in.ternop.predicate = predicate;
  nd->in.ternop.yes = yes;
  nd->in.ternop.no = no;
  nd->execf = exec_ternop;
  nd->compf = comp_ternop;
#if DEBUG
  nd->dumpf = dump_ternop;
#endif

  debug_ast_new(nd, "ternop (#%u, #%u, #%u)", NDID(nd->in.ternop.predicate),
      NDID(nd->in.ternop.yes), NDID(nd->in.ternop.no));

  return nd;
  /* }}} */
}

struct node *new_if(struct parser *parser, struct lexer *lex, struct node *guard, struct node *body,
    struct node *elsee)
{
  /* {{{ */
  struct node *nd = new_node(parser, lex);

  nd->type = NT_IF;
  nd->in.iff.guard = guard;
  nd->in.iff.body  = body;
  nd->in.iff.elsee = elsee;
  nd->in.iff.unless = false;
  nd->execf = exec_if;
  nd->compf = comp_if;
#if DEBUG
  nd->dumpf = dump_if;
#endif

  debug_ast_new(nd, "if (#%u, #%u, #%u)", NDID(guard), NDID(body), NDID(elsee));

  return nd;
  /* }}} */
}

struct node *new_fun(struct parser *parser, struct lexer *lex, char *name,
    struct node *body, char *opts, bool execute)
{
  /* {{{ */
  struct node *nd = new_node(parser, lex);

  nd->type = NT_FUN;
  nd->in.fun.name = name;
  nd->in.fun.body = body;
  nd->in.fun.execute = execute;
  nd->in.fun.compiled = false;
  nd->execf = exec_fun;
  nd->compf = comp_fun;
#if DEBUG
  nd->dumpf = dump_fun;
#endif

  if (name)
    debug_ast_new(nd, "fun (%s, #%u, %d)", name, NDID(body), execute);
  else
    debug_ast_new(nd, "lambda (#%u, %d)", name, NDID(body), execute);

  return nd;
  /* }}} */
}

struct node *new_call(struct parser *parser, struct lexer *lex, struct node *fun,
    struct nodes_list *args, char *opts)
{
  /* {{{ */
  struct node *nd = new_node(parser, lex);

  nd->type = NT_CALL;
  nd->in.call.fun  = fun;
  nd->in.call.args = args;
  nd->in.call.opts = opts;
  nd->execf = exec_call;
  nd->compf = comp_call;
#if DEBUG
  nd->dumpf = dump_call;
#endif

  debug_ast_new(nd, "call (#%d)", NDID(fun));

  return nd;
  /* }}} */
}

struct node *new_print(struct parser *parser, struct lexer *lex, struct nodes_list *exprs)
{
  /* {{{ */
  struct node *nd = new_node(parser, lex);

  nd->type = NT_PRINT;
  nd->result_type = T_INT;
  nd->in.print.exprs = exprs;
  nd->execf = exec_print;
  nd->compf = comp_print;
#if DEBUG
  nd->dumpf = dump_print;
#endif

  debug_ast_new(nd, "print");

  return nd;
  /* }}} */
}
/* }}} */

/*
 * Modifies a { struct nodes_list } in place by reversing it's order.
 *
 * The function returns the head of the (now modified) list, or NULL if <list>
 * was already NULL.
 */
struct nodes_list *reverse_nodes_list(struct nodes_list *list)
{
  struct nodes_list *curr = list,
                    *prev = NULL,
                    *next;

  while (curr != NULL){
    next = curr->next;
    curr->next = prev;
    prev = curr;
    curr = next;
  }

  list = prev;

  return list;
}

const char *binop_to_s(enum binop_type type)
{
  switch (type){
    case BINARY_GT:         return ">";
    case BINARY_LT:         return "<";
    case BINARY_GE:         return ">=";
    case BINARY_LE:         return "<=";
    case BINARY_EQ:         return "==";
    case BINARY_NE:         return "!=";
    case BINARY_ADD:        return "+";
    case BINARY_SUB:        return "-";
    case BINARY_MUL:        return "*";
    case BINARY_DIV:        return "/";
    case BINARY_MOD:        return "%";
    case BINARY_SHL:        return "<<";
    case BINARY_SHR:        return ">>";
    case BINARY_BITAND:     return "&";
    case BINARY_BITXOR:     return "^";
    case BINARY_BITOR:      return "|";
    case BINARY_ASSIGN:     return "=";
    case BINARY_ASSIGN_ADD: return "+=";
    case BINARY_ASSIGN_SUB: return "-=";
    case BINARY_ASSIGN_MUL: return "*=";
    case BINARY_ASSIGN_DIV: return "/=";
    case BINARY_ASSIGN_MOD: return "%=";
    case BINARY_ASSIGN_AND: return "&=";
    case BINARY_ASSIGN_XOR: return "^=";
    case BINARY_ASSIGN_OR:  return "|=";
    case BINARY_ASSIGN_SHL: return "<<=";
    case BINARY_ASSIGN_SHR: return ">>=";
    case BINARY_COMMA:      return ",";
    default: return "#unknown#binop_to_s#";
  }
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

