/*
 *
 * userdef.c
 *
 * Created at:  01/21/2013 05:09:36 PM
 *
 * Author:  Szymon Urbas <szymon.urbas@aol.com>
 *
 * License: the MIT license
 *
 */

#include "handy.h"
#include "userdef.h"

struct UserdefFunction *userdefs = NULL;

bool functionAlreadyExists(char *name)
{
  for (struct UserdefFunction *t = userdefs; t != NULL; t = t->next){
    if (!strcmp(t->function->data.funcdef.name, name)){
      return true;
    }
  }

  return false;
}

void addFunction(struct Node *n)
{
  assert(n);
  assert(nt_FUNCDEF == n->kind);

  if (functionAlreadyExists(n->data.funcdef.name)){
    cerror("function '%s' already defined", n->data.funcdef.name);
    exit(1);
  }

  struct UserdefFunction *functable = myalloc(sizeof(struct UserdefFunction));

  functable->function = n;
  functable->next = userdefs;
  userdefs = functable;
}

