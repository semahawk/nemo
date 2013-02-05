//
// utils.c
//
// Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
//

#include "nemo.h"
#include "handy.h"
#include "nodes.h"
#include "free.h"

extern bool debug_flag;

void debug(const char *type, char *string, ...)
{
  if (debug_flag){
    va_list ap;

    /*fprintf(stderr, " * ");*/

    if (!strcmp(type, "free")){
      fprintf(stderr, "\e[1;34m *   FREE:\e[0;0m ");
    } else if (!strcmp(type, "gen")){
      fprintf(stderr, "\e[1;32m *    GEN:\e[0;0m ");
    } else if (!strcmp(type, "exec")){
      fprintf(stderr, "\e[1;33m *   EXEC:\e[0;0m ");
    } else if (!strcmp(type, "append")){
      fprintf(stderr, "\e[1;32m * APPEND:\e[0;0m ");
    } else {
      fprintf(stderr, "\e[1;35m *  DEBUG:\e[0;0m ");
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
  if (debug_flag)
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
  extern int line;
  extern int col;

  fprintf(stderr, "%d:%d: error: ", line, col);
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
  //addToStack(np);

  return np ? strcpy(np, p) : np;
}

char *strip_quotes(char *p)
{
  for (char *q = p; *(q + 2) != '\0'; q++){
    *q = *(q + 1);
  }

  *(p + strlen(p) - 2) = '\0';

  return p;
}

int fsize(const char *name)
{
  struct stat st;

  if (stat(name, &st) != 0){
    perror("could not stat");
    return -1;
  }

  return st.st_size;
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
    case BINARY_EQ_CON:
      return ".=";
    case BINARY_CON:
      return ".";
    case BINARY_STR_GT:
      return "gt";
    case BINARY_STR_LT:
      return "lt";
    case BINARY_STR_GE:
      return "ge";
    case BINARY_STR_LE:
      return "le";
    case BINARY_STR_NE:
      return "ne";
    case BINARY_STR_EQ:
      return "eq";
  }

  return "#UNKBT#";
}

