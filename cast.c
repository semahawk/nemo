/*
 * cast.c
 *
 * Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
 *
 */

#include "cast.h"
#include "nodes_gen.h"
#include "handy.h"

inline int vtoi(Value value)
{
  if (value.type == TYPE_INTEGER)
    return value.v.i;
  else if (value.type == TYPE_FLOATING)
    return (int)value.v.f;
  else
    return 0;
}

inline float vtof(Value value)
{
  if (value.type == TYPE_INTEGER)
    return (float)value.v.i;
  else if (value.type == TYPE_FLOATING)
    return value.v.f;
  else
    return 0.0f;
}

inline char *vtos(Value value)
{
  char *str = myalloc(31);

  if (value.type == TYPE_INTEGER)
    snprintf(str, 30, "%d", value.v.i);
  else if (value.type == TYPE_FLOATING)
    snprintf(str, 30, "%.2f", value.v.f);
  else
    snprintf(str, 30, "#unknowntype#vtos#");

  return str;
}

inline bool vtob(Value value)
{
  if (value.type == TYPE_INTEGER){
    if (value.v.i != 0){
      return true;
    } else {
      return false;
    }
  } else if (value.type == TYPE_FLOATING){
    if (value.v.f != 0){
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

inline Value vtov(Value value, Type type)
{
  Value ret;

  switch (type){
    case TYPE_INTEGER:
      ret.v.i = vtoi(value);
      ret.type = TYPE_INTEGER;
      break;
    case TYPE_FLOATING:
      ret.v.f = vtof(value);
      ret.type = TYPE_FLOATING;
      break;
  }

  return ret;
}

