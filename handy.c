//
// utils.c
//
// Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
//

#include "nemo.h"
#include "handy.h"
#include "nodes.h"

extern bool debugflag;

void debug(const char *type, char *string, ...)
{
  if (debugflag){
    va_list ap;

    fprintf(stderr, " * ");

    if (!strcmp(type, "free")){
      fprintf(stderr, "\e[1;34mFREE:\e[0;0m ");
    } else if (!strcmp(type, "create")){
      fprintf(stderr, "\e[1;32mCREATE:\e[0;0m ");
    } else if (!strcmp(type, "exec")){
      fprintf(stderr, "\e[1;33mEXEC:\e[0;0m ");
    } else if (!strcmp(type, "append")){
      fprintf(stderr, "\e[1;32mAPPEND:\e[0;0m ");
    } else {
      fprintf(stderr, "\e[1;35mDEBUG:\e[0;0m ");
    }

    va_start(ap, string);
    vfprintf(stderr, string, ap);
    fprintf(stderr, "\n");
    va_end(ap);
  }
}

void error(char *string, ...)
{
  va_list ap;
  static int error_num = 1;

  fprintf(stderr, "nemo: error");
  if (debugflag)
    fprintf(stderr, " #%d", error_num);
  fprintf(stderr, ": ");
  va_start(ap, string);
  vfprintf(stderr, string, ap);
  fprintf(stderr, "\n");
  va_end(ap);
}

void cerror(char *string, ...)
{
  va_list ap;
  extern char source[];
  extern int linenum;
  extern int column;

  fprintf(stderr, "%s:%d:%d: error: ", source, linenum, column);
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

const char *unarytos(Unary type)
{
  switch (type){
    case UNARY_POSTINC:
      return "$v++";
    case UNARY_POSTDEC:
      return "$v--";
    case UNARY_PREINC:
      return "++$v";
    case UNARY_PREDEC:
      return "--$v";
  }

  return "#UNKUT#";
}

