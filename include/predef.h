/*
 *
 * predef.h
 *
 * Created at:  01/17/2013 07:21:12 PM
 *
 * Author:  Szymon Urbas <szymon.urbas@aol.com>
 *
 * License: the MIT license
 *
 */

#include "nemo.h"
#include "nodes.h"

struct PredefFunction {
  char *name;
  Value (*fn)(struct ParamList *, int);
};

Value predef_print(struct ParamList *, int);
Value predef_assert(struct ParamList *, int);

extern struct PredefFunction predefs[];
extern unsigned int predefs_size;

