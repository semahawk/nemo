/*
 *
 * infer.c
 *
 * Created at:  Mon Aug 24 14:48:17 2015 14:48:17
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include "mem.h"
#include "scope.h"
#include "infer.h"
#include "ast.h"
#include "nob.h"

static jmp_buf infer_jmp_buf;

static struct nob_type *infer_type_internal(struct scope *scope, struct node *node, struct ng *nongen);
static bool types_are_equal(struct nob_type *one, struct nob_type *two);
static bool occurs_in_type(struct nob_type *v, struct nob_type *type2);
static bool occurs_in(struct nob_type *v, struct nob_type *type2);
static struct nob_type *prune(struct nob_type *type);
static void unify(struct nob_type *one, struct nob_type *two);

typedef struct {
  struct nob_type *from;
  struct nob_type *to;
} mapping_t;

struct ng *new_nongen(void)
{
  struct ng *ng = nmalloc(sizeof(struct ng));

  ng->current_slot = ng->slots;

  return ng;
}

struct ng *copy_nongen(struct ng *ng)
{
  struct ng *new_ng = nmalloc(sizeof(struct ng));

  memcpy(new_ng, ng, sizeof(struct ng));
  new_ng->current_slot = new_ng->slots + ng->slots_num;

  return new_ng;
}

void add_to_nongen(struct ng *ng, struct nob_type *type)
{
  *ng->current_slot = type;
  ng->current_slot++;
  ng->slots_num++;
}

static bool is_generic(struct nob_type *type, struct ng *nongen)
{
  unsigned i;

  for (i = 0; i < nongen->slots_num; i++){
    if (occurs_in_type(type, nongen->slots[i]))
      return false;
  }

  return true;
}

static struct nob_type *freshrec(struct nob_type *type, struct ng *nongen, mapping_t *mappings, unsigned *current_mapping, unsigned *mappings_num)
{
  struct nob_type *pruned = prune(type);

  if (is_type_variable(pruned)){
    if (is_generic(pruned, nongen)){
      unsigned i;
      mapping_t *ptr;
      bool found = false;

      for (i = 0; i < *mappings_num; i++){
        ptr = &mappings[i];

        if (types_are_equal(ptr->from, pruned)){
          found = true;
          break;
        }
      }

      if (!found){
        ptr = &mappings[*current_mapping];
        ptr->from = pruned;
        ptr->to = new_type(OT_TYPE_VARIABLE);

        (*current_mapping)++;
        (*mappings_num)++;
      }

      return ptr->to;
    } else {
      /* not generic */
      return pruned;
    }
  } else {
    struct types_list *new_types_list = NULL;
    struct types_list *in_pruned;
    struct types_list *new_type_elem;

    for (in_pruned = pruned->types; in_pruned != NULL; in_pruned = in_pruned->next){
      new_type_elem = nmalloc(sizeof(struct types_list));
      new_type_elem->type = freshrec(in_pruned->type, nongen, mappings, current_mapping, mappings_num);
      /* append to the new types list */
      new_type_elem->next = new_types_list;
      new_types_list = new_type_elem;
    }

    new_types_list = reverse_types_list(new_types_list);

    switch (pruned->primitive){
      case OT_CUSTOM:
        return new_type(OT_CUSTOM, pruned->info.custom.name, pruned->info.custom.var);
      case OT_FUN:
        return new_type(OT_FUN, pruned->info.func.return_type, new_types_list);
      case OT_TUPLE:
        return new_type(OT_TUPLE, new_types_list);

      /* silence warnings, we won't use these here */
      case OT_TYPE_VARIABLE:
      case OT_INT:
      case OT_REAL:
      case OT_CHAR:
      case OT_STRING:
      case OT_INFNUM:
      default:
        /* hmmmmm... FIXME? */
        return pruned;
    }
  }
}

struct nob_type *fresh(struct nob_type *type, struct ng *nongen)
{
  mapping_t mappings[256] = { { NULL, NULL } };
  unsigned current_mapping = 0;
  unsigned mappings_num = 0;

  return freshrec(type, nongen, mappings, &current_mapping, &mappings_num);
}

static bool types_are_equal(struct nob_type *type1, struct nob_type *type2)
{
  return type1 == type2;
}

static bool occurs_in_type(struct nob_type *type1, struct nob_type *type2)
{
  assert(type1 != NULL && type2 != NULL);

  struct nob_type *pruned_type2 = prune(type2);

  if (is_type_variable(pruned_type2))
    return types_are_equal(type1, pruned_type2);
  else if (is_type_operator(pruned_type2))
    return occurs_in(type1, pruned_type2);
  else {
    printf("error: neither a type operator nor a type variable");
    longjmp(infer_jmp_buf, 1);
  }
}

static bool occurs_in(struct nob_type *type1, struct nob_type *type2)
{
  assert(type1 != NULL && type2 != NULL);

  struct types_list *lptr;

  for (lptr = type2->types; lptr != NULL; lptr = lptr->next)
    if (occurs_in_type(type1, lptr->type))
      return true;

  return false;
}

static struct nob_type *prune(struct nob_type *type)
{
  assert(type != NULL);

  if (is_type_variable(type))
    if (type->info.var.instance){
      type->info.var.instance = prune(type->info.var.instance);
      return type->info.var.instance;
    }

  return type;
}

static void unify(struct nob_type *type1, struct nob_type *type2)
{
  assert(type1 != NULL);
  assert(type2 != NULL);

  struct nob_type *a = prune(type1);
  struct nob_type *b = prune(type2);

  assert(a != NULL && b != NULL);

  if (is_type_variable(a)){
    if (!types_are_equal(a, b)){
      if (occurs_in_type(a, b)){
        printf("recursive unification");
        longjmp(infer_jmp_buf, 1);
      } else {
        a->info.var.instance = b;
      }
    }
  } else if (is_type_operator(a) && is_type_variable(b)){
    unify(b, a);
  } else if (is_type_operator(a) && is_type_operator(b)){
    if ((a->primitive != b->primitive) || (types_list_length(a->types) != types_list_length(b->types))){
      printf("type mismatch: ");
      nob_print_type(a);
      printf(" != ");
      nob_print_type(b);

      longjmp(infer_jmp_buf, 1);
    } else {
      struct types_list *lptra = a->types,
                        *lptrb = b->types;

      for (; lptra != NULL; lptra = lptra->next, lptrb = lptrb->next)
        unify(lptra->type, lptrb->type);
    }
  } else {
    printf("couldn't unify the two types");
    longjmp(infer_jmp_buf, 1);
  }
}

static struct nob_type *infer_type_internal(struct scope *scope, struct node *node, struct ng *nongen)
{
  assert(node);

  if (node->result_type)
    return node->result_type;

  if (nongen == NULL)
    nongen = new_nongen();

  switch (node->type){
    case OT_INT:
      return T_INT;
    case OT_STRING:
      return T_STRING;
    case OT_TUPLE:
    {
      struct types_list *new_types_list = NULL;
      struct types_list *new_type_elem;
      struct nodes_list *lptr;

      for (lptr = node->in.tuple.elems; lptr != NULL; lptr = lptr->next){
        new_type_elem = nmalloc(sizeof(struct types_list));
        new_type_elem->type = infer_type_internal(scope, lptr->node, nongen);
        new_type_elem->next = new_types_list;
        new_types_list = new_type_elem;
      }

      new_types_list = reverse_types_list(new_types_list);

      return new_type(OT_TUPLE, new_types_list);
    }
    /* TODO implement the rest of nodes */
    default:
      printf("#unknown_node_type#infer_type_internal#");
      longjmp(infer_jmp_buf, 1);
  }
}

struct nob_type *infer_node_type(struct scope *scope, struct node *node)
{
  next_type_var_name = 'a';

  if (!setjmp(infer_jmp_buf))
    return infer_type_internal(scope, node, NULL);
  else
    return NULL;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

