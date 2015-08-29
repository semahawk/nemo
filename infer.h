/*
 *
 * infer.h
 *
 * Created at:  Mon Aug 24 14:47:44 2015 14:47:44
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef INFER_H
#define INFER_H

#include "ast.h"
#include "nemo.h"
#include "nob.h"
#include "scope.h"

/* bleh.. */
struct ng {
  struct nob_type *slots[256];
  struct nob_type **current_slot;
  size_t slots_num;
};

struct nob_type *infer_node_type(struct scope *scope, struct node *node);
struct nob_type *fresh(struct nob_type *type, struct ng *non_generic);

struct ng *new_non_generic(void);
struct ng *copy_non_generic(struct ng *non_generic);
void add_to_non_generic(struct ng *non_generic, struct nob_type *type);

#endif /* INFER_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

