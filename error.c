/*
 *
 * error.c
 *
 * Created at:  03/04/2013 05:42:39 PM
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License: the MIT license
 *
 */

/*
 * "You think your life's so grand
 *  You don't believe a word you say
 *  Your feet aren't on the ground
 *  You let your life just slip away
 *  Just so uncertain of your body and your soul
 *  The promises you make your mind go blank
 *  And then you lose control, then you lose control!"
 *
 *  Testament - Practice What You Preach
 */

#include <stdio.h>
#include <stdarg.h>

#include "nemo.h"

/*
 * TODO: append the errors into some kind of a list, then print them out at
 *       once, if one of them was fatal, or error, exit before
 *       executing/compiling
 */

/* current error message to be printed, changeable via NmError_SetString */
static char err[255] = "something went wrong";

char *NmError_GetCurr(void)
{
  return err;
}

void NmError_SetString(const char *msg, ...)
{
  va_list vl;
  va_start(vl, msg);
  vsprintf(err, msg, vl);
  va_end(vl);
}

void NmError_Fatal(const char *msg, ...){
  va_list vl;
  va_start(vl, msg);
  fprintf(stderr, "nemo: fatal: ");
  vfprintf(stderr, msg, vl);
  fprintf(stderr, "\n");
  va_end(vl);
}

void NmError_Error(const char *msg, ...){
  va_list vl;
  va_start(vl, msg);
  fprintf(stderr, "nemo: error: ");
  vfprintf(stderr, msg, vl);
  fprintf(stderr, "\n");
  va_end(vl);
}

void NmError_Lex(LexerState *lex, const char *msg, ...)
{
  va_list vl;
  va_start(vl, msg);

  if (lex->is_file){
    fprintf(stderr, "In file %s: ", lex->source);
  } else {
    fprintf(stderr, "In string \"%s\": ", lex->source);
  }

  vfprintf(stderr, msg, vl);
  fprintf(stderr, " at line %u in column %u", lex->current.pos.line, lex->current.pos.column);
  fprintf(stderr, "\n");
  va_end(vl);
}

void NmError_Parser(Node *n, const char *msg, ...)
{
  Namespace *namespace = NmNamespace_GetCurr();

  va_list vl;
  va_start(vl, msg);

  fprintf(stderr, "In file %s: ", namespace->name);

  vfprintf(stderr, msg, vl);
  fprintf(stderr, " at line %u in column %u", n->pos.line, n->pos.column);
  fprintf(stderr, "\n");
  va_end(vl);
}

