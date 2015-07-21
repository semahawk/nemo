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

/* forward declaration */
struct scope;

/* stuff related to 'accumulators' */
/* each scope gets a list of accumulators */
struct accs_list {
  unsigned id; /* eg. id 5 would correspond to '%5' */
  struct node *node;
  struct accs_list *next;
};

struct accs_list *accs_new_list(void);
void accs_finish(struct accs_list *list);

void         acc_set_value(struct scope *, unsigned id, struct node *node);
struct node *acc_get_value(struct scope *, unsigned id);

#define VAR_FLAG_

/* stuff related to variables and scopes */
struct var {
  char *name;
  uint8_t flags;
  bool param; /* is it a parameter or a variable? (matters with the assembly) */
  struct node *value;
  struct node *decl; /* reference to the declaration that did the variable */
  struct nob_type *type;
  unsigned offset; /* the var's place on the stack */
};

struct vars_list {
  struct var *var;
  struct vars_list *next;
};

struct scope {
  /* can be null */
  char *name;
  /* can be null */
  struct scope *parent;
  /* head of the variables list */
  struct vars_list *vars;
  /* base offset the variables are from (increases along with "nestiness") */
  unsigned base_offset;
  /* offset the next variable will get */
  int curr_var_offset;
  /* offset the next parameter will get (FIXME?) */
  int curr_param_offset;
  /* head of the accumulators list */
  struct accs_list *accs;
};

struct scopes_list {
  struct scope *scope;
  struct scopes_list *next;
};

struct scope *new_scope(char *name, struct scope *parent);
void free_scope(struct scope *scope);
void scopes_finish(void);

struct var *new_var(char *name, uint8_t flags, struct node *value,
    struct nob_type *type, struct scope *scope, bool param, int offset);

struct var *var_lookup(char *name, struct scope *scope);

unsigned size_of_vars(struct scope *scope);

/* a list of global scopes */
extern struct scopes_list *NM_scopes;

#endif /* SCOPE_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */


