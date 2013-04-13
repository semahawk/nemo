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

/*
 * TODO: append the errors into some kind of a list, then print them out at
 *       once, if one of them was fatal, or error, exit before
 *       executing/compiling
 */

void nmFatal(const char *msg, ...){
  va_list vl;
  va_start(vl, msg);
  fprintf(stderr, "nemo: fatal: ");
  vfprintf(stderr, msg, vl);
  fprintf(stderr, "\n");
  va_end(vl);
}

void nmError(const char *msg, ...){
  va_list vl;
  va_start(vl, msg);
  fprintf(stderr, "nemo: error: ");
  vfprintf(stderr, msg, vl);
  fprintf(stderr, "\n");
  va_end(vl);
}

