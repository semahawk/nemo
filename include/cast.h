/*
 * cast.h
 *
 * Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
 *
 */

#ifndef CAST_H
#define CAST_H

#include <ctype.h>

#include "nodes.h"

int   vtoi(Value);
float vtof(Value);
char *vtos(Value);
bool  vtob(Value);
Value vtov(Value, Type);

#endif /* CAST_H */
