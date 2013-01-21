/*
 *
 * userdef.h
 *
 * Created at:  01/20/2013 09:32:45 PM
 *
 * Author:  Szymon Urbas <szymon.urbas@aol.com>
 *
 * License: the MIT license
 *
 */

#ifndef USERDEF_H
#define USERDEF_H

#include "nodes.h"

struct UserdefFunction {
  struct Node *function;
  struct UserdefFunction *next;
};

bool functionAlreadyExists(char *name);
void addFunction(struct Node *);

extern struct UserdefFunction *userdefs;

#endif // USERDEF_H

