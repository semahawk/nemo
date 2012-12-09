//
// utils.h
//
// Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
//

#ifndef HANDY_H
#define HANDY_H

#include "nodes_gen.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof *(x))

void debug(char *, ...);
void error(char *, ...);
void cerror(char *, ...);

void *myalloc(size_t);

char *strdup(const char *);

const char *unarytos(Unary);

#endif // HANDY_H
