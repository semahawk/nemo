//
// utils.h
//
// Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
//

#ifndef HANDY_H
#define HANDY_H

#include "nodes.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof *(x))

void debug(const char *, char *, ...);
void error(char *, ...);
void cerror(char *, ...);

void *myalloc(size_t);
void *myrealloc(void *, size_t);

char *strdup(const char *);
int fsize(const char *);

const char *unarytos(Unary);
const char *binarytos(Binary);

#endif // HANDY_H
