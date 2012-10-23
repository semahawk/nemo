//
// utils.c
//
// Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
//

#include "nemo.h"
#include "handy.h"

#if DEBUG
  void debug(char *string, ...)
  {
    va_list ap;

    fprintf(stderr, "** DEBUG: ");
    va_start(ap, string);
    vfprintf(stderr, string, ap);
    fprintf(stderr, "\n");
    va_end(ap);
  }
#else
  void debug(char *string, ...){
    /* do nothing, and do not complain about unused parameter */
    string;
  }
#endif

void error(char *string, ...)
{
  va_list ap;
#if DEBUG
  static int error_num = 1;
#endif

  fprintf(stderr, "nemo: error");
#if DEBUG
  fprintf(stderr, " #%d", error_num);
#endif
  fprintf(stderr, ": ");
  va_start(ap, string);
  vfprintf(stderr, string, ap);
  fprintf(stderr, "\n");
  va_end(ap);
}

void *myalloc(size_t size)
{
  void *p = malloc(size);
  if (p == NULL){
    error("malloc: failed to allocate %d bytes in file %s at line %d", size, __FILE__, __LINE__);
    exit(1);
  }

  return p;
}

char *strdup(const char *p)
{
  char *np = malloc(strlen(p) + 1);

  return np ? strcpy(np, p) : np;
}

