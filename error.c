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

#include <stdio.h>
#include <stdarg.h>

void NM_fatal(const char *msg, ...){
  va_list vl;
  va_start(vl, msg);
  fprintf(stderr, "nemo: fatal: ");
  vfprintf(stderr, msg, vl);
  fprintf(stderr, "\n");
  va_end(vl);
}

