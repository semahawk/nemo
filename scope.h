/*
 *
 * scope.h
 *
 * Created at:  Sat  7 Jun 19:51:07 2014 19:51:07
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef SCOPE_H
#define SCOPE_H

#include <stdint.h>

#include "ast.h"
#include "nob.h"

struct var {
  char *name;
  uint8_t flags;
  struct node *value;
  struct nob_type *type;
};

struct vars_list {
  struct var *var;
  struct vars_list *next;
};

struct scope {
  char *name; /* can be null */
  struct scope *parent;
  struct vars_list *vars;
};

struct scopes_list {
  struct scope *scope;
  struct scopes_list *next;
};

struct scope *new_scope(char *name, struct scope *parent);
void free_scope(struct scope *scope);
void scopes_finish(void);

struct var *new_var(char *name, uint8_t flags, struct node *value,
    struct nob_type *type, struct scope *scope);

struct var *var_lookup(char *name, struct scope *scope);

/* a list of global scopes */
extern struct scopes_list *NM_scopes;

#endif /* SCOPE_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */


