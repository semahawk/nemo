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

#include "config.h"
#include "debug.h"
#include "mem.h"

/* see how many mallocs/callocs/reallocs/frees there were */
static unsigned counter = 0;

void *nmalloc_(size_t size, const char *file, unsigned line)
{
  void *ptr;

  if ((ptr = malloc(size)) == NULL){
    fprintf(stderr, "malloc couldn't allocate %lu bytes in %s line %u\n", size, file, line);
    exit(1);
  }

#if DEBUG
  if (NM_DEBUG_GET_FLAG(NM_DEBUG_MEM)){
    counter++;
    fprintf(stderr, "(%05u) %p: malloc %lu bytes (%s:%u)\n", counter, ptr, size, file, line);
  }
#else /* DEBUG */
  /* suspress warnings */
  (void)counter;
  (void)file;
  (void)line;
#endif /* DEBUG */

  return ptr;
}

void *ncalloc_(size_t number, size_t size, const char *file, unsigned line)
{
  void *ptr;

  if ((ptr = calloc(number, size)) == NULL){
    fprintf(stderr, "calloc failed to allocate %lu (%lux%lu) bytes in %s line %u\n", number * size, number, size, file, line);
    exit(1);
  }

#if DEBUG
  if (NM_DEBUG_GET_FLAG(NM_DEBUG_MEM)){
    counter++;
    fprintf(stderr, "(%05u) %p: calloc %lu (%lux%lu) bytes (%s:%u)\n", counter, ptr, number * size, number, size, file, line);
  }
#else /* DEBUG */
  /* suspress warnings */
  (void)counter;
  (void)file;
  (void)line;
#endif /* DEBUG */

  return ptr;
}

void *nrealloc_(void *ptr, size_t size, const char *file, unsigned line)
{
  void *p;

  if ((p = realloc(ptr, size)) == NULL){
    fprintf(stderr, "realloc couldn't reallocate %lu bytes in %s line %u\n", size, file, line);
    exit(1);
  }

#if DEBUG
  if (NM_DEBUG_GET_FLAG(NM_DEBUG_MEM)){
    counter++;
    fprintf(stderr, "(%05u) %p: realloc %lu bytes (%s:%u)\n", counter, ptr, size, file, line);
  }
#else /* DEBUG */
  /* suspress warnings */
  (void)counter;
  (void)file;
  (void)line;
#endif /* DEBUG */

  return p;
}

void nfree_(void *ptr, const char *file, unsigned line)
{
#if DEBUG
  if (NM_DEBUG_GET_FLAG(NM_DEBUG_MEM)){
    counter++;
    fprintf(stderr, "(%05u) %p: free (%s:%u)\n", counter, ptr, file, line);
  }
#else /* DEBUG */
  /* suspress warnings */
  (void)counter;
  (void)file;
  (void)line;
#endif /* DEBUG */

  /* free what's 'under' the pointer */
  free(ptr);
  /* 'invalidate' it */
  ptr = (unsigned char *)0x1;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

