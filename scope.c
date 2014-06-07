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

  if (name != NULL)
    scope->name = strdup(name);
  else
    scope->name = NULL;

  scope->parent = parent;
  scope->vars   = NULL;

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
    struct nob_type *type, struct scope *scope)
{
  struct var *var = nmalloc(sizeof(struct var));
  struct vars_list *list = nmalloc(sizeof(struct vars_list));

  var->name = strdup(name);
  var->flags = flags;
  var->value = value;
  var->type = type;

  assert(scope);

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

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

