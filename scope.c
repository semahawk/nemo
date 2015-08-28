/*
 *
 * scope.c
 *
 * Created at:  Sat  7 Jun 20:24:21 2014 20:24:21
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#include "mem.h"
#include "scope.h"
#include "util.h"

/* a list of all the scopes ever declared */
struct scopes_list *NM_scopes = NULL;

struct scope *new_scope(char *name, struct scope *parent)
{
  struct scope *scope = nmalloc(sizeof(struct scope));
  struct scopes_list *list = nmalloc(sizeof(struct scopes_list));
  struct scope *p;

  if (name != NULL)
    scope->name = strdup(name);
  else
    scope->name = NULL;

  scope->parent = parent;
  scope->vars   = NULL;
  scope->accs   = accs_new_list();
  scope->curr_var_offset = -4;
  scope->curr_param_offset = 8;
  scope->base_offset = 0;

  for (p = scope; p != NULL; p = p->parent){
    scope->base_offset += size_of_vars(p);
  }

  list->scope = scope;
  /* append to the `NM_scopes' list */
  list->next = NM_scopes;
  NM_scopes = list;

  return scope;
}

void free_scope(struct scope *scope)
{
  struct vars_list *curr, *next;

  assert(scope);

  nfree(scope->name);
  /* free all the variables */
  for (curr = scope->vars; curr != NULL; curr = next){
    next = curr->next;
    nfree(curr->var->name);
    nfree(curr->var);
    nfree(curr);
    /* we don't have to worry about variables' types, as they are handled
     * somewhere else */
    /* same goes for the objects the variables hold, actualy */
  }

  /* free the accumulators */
  accs_finish(scope->accs);
}

void scopes_finish(void)
{
  struct scopes_list *curr, *next;

  for (curr = NM_scopes; curr != NULL; curr = next){
    next = curr->next;
    free_scope(curr->scope);
    nfree(curr);
  }
}

struct var *new_var(char *name, uint8_t flags, struct node *value,
    struct nob_type *type, struct scope *scope, bool param, int offset)
{
  struct var *var = nmalloc(sizeof(struct var));
  struct vars_list *list = nmalloc(sizeof(struct vars_list));

  var->name = strdup(name);
  var->flags = flags;
  var->value = value;
  var->type = type;
  var->param = param;

  assert(scope);

  /* assembly stuff */
  if (param){
    if (offset != 0)
      var->offset = offset + 8;
    else {
      var->offset = scope->curr_param_offset;

      if (type)
        scope->curr_param_offset += type->size;
      else /* FIXME FIXME */
        scope->curr_param_offset += 4;
    }
  } else {
    if (offset != 0)
      var->offset = offset - 4;
    else {
      var->offset = scope->curr_var_offset;

      scope->curr_var_offset -= type->size;
    }
  }

  /*printf("created new variable (%d) called %s at offset %d (orig %d)\n", param, name, var->offset, offset);*/

  list->var = var;
  /* append the new element into the `scope`s `vars` list */
  list->next = scope->vars;
  scope->vars = list;

  return var;
}

struct var *var_lookup(char *name, struct scope *scope)
{
  struct scope *s;
  struct vars_list *v;

  assert(scope);

  for (s = scope; s != NULL; s = s->parent)
    for (v = s->vars; v != NULL; v = v->next)
      if (!strcmp(v->var->name, name))
        return v->var;

  return NULL;
}

unsigned size_of_vars(struct scope *scope)
{
  struct vars_list *v;
  unsigned size = 0;

  assert(scope);

  for (v = scope->vars; v != NULL; v = v->next){
    if (v->var->param)
      continue;

    /* FIXME FIXME */
    if (v->var->type)
      size += v->var->type->size;
    else
      size += 4;
  }

  return size;
}

struct accs_list *accs_new_list(void)
{
  struct accs_list *head = NULL;
  unsigned i;

  /* create 2 'startup' accumulators by default */
  for (i = 0; i < 2; i++){
    struct accs_list *acc = nmalloc(sizeof(struct accs_list));
    /* set up */
    acc->id = i;
    acc->node = NULL; /* hmm.. */
    /* append */
    acc->next = head;
    head = acc;
  }

  return head;
}

void accs_finish(struct accs_list *accs)
{
  struct accs_list *curr, *next;

  for (curr = accs; curr != NULL; curr = next){
    next = curr->next;

    /* don't free curr->node */
    nfree(curr);
  }
}

void acc_set_value(struct scope *scope, unsigned id, struct node *node)
{
  bool found = false;

  for (; scope != NULL; scope = scope->parent)
    for (struct accs_list *p = scope->accs; p != NULL; p = p->next)
      if (p->id == id)
        p->node = node, found = true;

  if (!found){
    /* if the accumulator was not found - let's create it */
    struct accs_list *new = nmalloc(sizeof(struct accs_list));
    /* setup */
    new->id = id;
    new->node = node;
    /* append */
    new->next = scope->accs;
    scope->accs = new;
  }
}

struct node *acc_get_value(struct scope *scope, unsigned id)
{
  for (; scope != NULL; scope = scope->parent)
    for (struct accs_list *p = scope->accs; p != NULL; p = p->next)
      if (p->id == id)
        return p->node;

  return NULL;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

