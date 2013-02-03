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
#include "handy.h"
#include "free.h"

struct PredefFunction predefs[] =
{
  { "print",  predef_print  },
  { "assert", predef_assert },
  { "strlen", predef_strlen },
  { "eval",   predef_eval   },
};

unsigned int predefs_size = 4;

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

Value predef_assert(struct ParamList *params, int paramcount)
{
  if (paramcount > 2){
    cerror("too many arguments for function 'assert' (%d when 2 expected)", paramcount);
    exit(1);
  }

  Value ret;
  bool  res = false;

  // 2 arguments
  //
  // comparing them, if they are equal, that's good
  // if not, aborting
  if (paramcount == 2){
    Value first  = dispatchNode(params->param);
    params = params->next;
    Value second = dispatchNode(params->param);

    if (first.type == TYPE_INTEGER){
      if (second.type == TYPE_INTEGER){
        res = first.v.i == second.v.i;
      } else if (second.type == TYPE_FLOATING){
        res = vtof(first) == second.v.f;
      }
    } else if (first.type == TYPE_FLOATING){
      if (second.type == TYPE_INTEGER){
        res = first.v.f == vtof(second);
      } else if (second.type == TYPE_FLOATING){
        res = first.v.f == second.v.f;
      }
    } else if (first.type == TYPE_STRING){
      res = true;
      char *str_left = first.v.s;
      char *str_right = vtos(second);
      size_t longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
      for (size_t i = 0; i < longer; i++){
        if (str_left[i] != str_right[i]){
          res = false;
          break;
        }
      }
    }
  // 1 argument
  //
  // casting that to "bool", if 0, abort, else, that's good
  } else {
    Value first = dispatchNode(params->param);
    res = vtob(first);
  }

  if (!res){
    cerror("assertion failed");
    exit(1);
  }

  ret.v.i = 1;
  ret.type = TYPE_INTEGER;

  return ret;
}

Value predef_strlen(struct ParamList *params, int param_count)
{
  Value ret;

  if (param_count != 1){
    cerror("strlen: wrong number of arguments (%d when 1 expected)", param_count);
    exit(1);
  }

  ret.v.i = strlen(vtos(dispatchNode(params->param)));
  ret.type = TYPE_INTEGER;

  return ret;
}

Value predef_eval(struct ParamList *params, int param_count)
{
  Value ret;
  struct Node *nodest;

  if (param_count != 1){
    cerror("eval: wrong number of arguments (%d when 1 expected)", param_count);
    exit(1);
  }

  nodest = parseString(vtos(dispatchNode(params->param)));

  ret.v.i = execNodes(nodest).v.i;
  ret.type = TYPE_INTEGER;

  freeNodes(nodest);
  freeStack();

  return ret;
}

