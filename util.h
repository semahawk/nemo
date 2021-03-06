/*
 *
 * util.h
 *
 * Created at:  Fri Nov  8 20:07:13 2013 20:07:13
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License: please visit the LICENSE file for details.
 *
 */

#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "ast.h"
#include "config.h"

#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))

#ifndef HAVE_STRDUP
#define strdup(p) strdup_(p, __FILE__, __LINE__)
char *strdup_(char *p, const char *file, unsigned line)
{
  if (p == NULL)
    return NULL;

  char *np = nmalloc(strlen(p) + 1);

  return np ? strcpy(np, p) : np;
}
#else /* HAVE_STRDUP */
char *strdup(const char *);
#endif /* HAVE_STRDUP */

const char *itob4(int number);
const char *itob8(int number);
const char *itob64(int64_t number);

void out(const char *fmt, ...);

#endif /* UTIL_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

