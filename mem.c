/*
 *
 * mem.c
 *
 * Created at:  Sat 16 Nov 15:15:08 2013 15:15:08
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "mem.h"

void *nmalloc_(size_t size, const char *file, unsigned line)
{
  void *p;

  if ((p = malloc(size)) == NULL){
    fprintf(stderr, "malloc couldn't allocate %lu bytes in %s line %u\n", size, file, line);
    exit(1);
  }

  return p;
}

void *ncalloc_(size_t number, size_t size, const char *file, unsigned line)
{
  void *p;

  if ((p = calloc(number, size)) == NULL){
    fprintf(stderr, "calloc failed to allocate %lu (%lux%lu) bytes in %s line %u\n", number * size, number, size, file, line);
    exit(1);
  }

  return p;
}

void *nrealloc_(void *ptr, size_t size, const char *file, unsigned line)
{
  void *p;

  if ((p = realloc(ptr, size)) == NULL){
    fprintf(stderr, "realloc couldn't reallocate %lu bytes in %s line %u\n", size, file, line);
    exit(1);
  }

  return p;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

