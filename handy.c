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

void *myrealloc(void *ptr, size_t size)
{
  void *p = realloc(ptr, size);

  if (p == NULL){
    error("realloc: failed to allocate %d bytes in file %s at line %d", size, __FILE__, __LINE__);
    exit(1);
  }

  return p;
}

char *strdup(const char *p)
{
  char *np = myalloc(strlen(p) + 1);

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
    case UNARY_PLUS:
      return "unary +";
    case UNARY_MINUS:
      return "unary -";
  }

  return "#UNKUT#";
}

const char *binarytos(Binary type)
{
  switch (type){
    case BINARY_ADD:
      return "+";
    case BINARY_SUB:
      return "-";
    case BINARY_MUL:
      return "*";
    case BINARY_DIV:
      return "/";
    case BINARY_MOD:
      return "%";
    case BINARY_GT:
      return ">";
    case BINARY_LT:
      return "<";
    case BINARY_GE:
      return ">=";
    case BINARY_LE:
      return "<=";
    case BINARY_NE:
      return "!=";
    case BINARY_EQ:
      return "==";
    case BINARY_EQ_ADD:
      return "+=";
    case BINARY_EQ_SUB:
      return "-=";
    case BINARY_EQ_MUL:
      return "*=";
    case BINARY_EQ_DIV:
      return "/=";
    case BINARY_EQ_MOD:
      return "%=";
    case BINARY_CON:
      return ".";
  }

  return "#UNKBT#";
}

