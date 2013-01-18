/*
 *
 * predef.c
 *
 * Created at:  01/17/2013 07:20:24 PM
 *
 * Author:  Szymon Urbas <szymon.urbas@aol.com>
 *
 * License: the MIT license
 *
 */

#include "nemo.h"
#include "predef.h"
#include "exec.h"
#include "nodes.h"
#include "cast.h"

struct PredefFunction predefs[] =
{
  { "print", predef_print }
};

unsigned int predefs_size = 1;

Value predef_print(struct ParamList *params, int paramcount)
{
  Value ret;

  if (params){
    for (struct ParamList *p = params; p != NULL; p = p->next){
      if (p->pos == paramcount - 1)
        printf("%s\n", vtos(dispatchNode(p->param)));
      else
        printf("%s, ", vtos(dispatchNode(p->param)));
    }
  }

  ret.v.i = 1;
  ret.type = TYPE_INTEGER;

  return ret;
}

