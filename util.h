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

#include "config.h"

#ifndef HAVE_STRDUP
#define strdup(p) strdup_(p, __FILE__, __LINE__)
char *strdup_(char *p, const char *file, unsigned line)
{
  if (p == NULL)
    return NULL;

  char *np = malloc(strlen(p) + 1);

  if (np == NULL){
    fprintf(stderr, "malloc: %s in %s line %u\n", strerror(errno), file, line);
    exit(1);
  }

  return np ? strcpy(np, p) : np;
}
#endif

#endif /* UTIL_H */

