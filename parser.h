/*
 *
 * parser.h
 *
 * Created at:  Thu Nov  7 20:09:55 2013 20:09:55
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License: please visit the LICENSE file for details.
 *
 */

#ifndef PARSER_H
#define PARSER_H

#include "nemo.h"

struct parser {
  /* whether there were any errors or not */
  /* ie. if `true', it means the code can be safely executed (no error/warning
   * message was critical) */
  bool errorless;
};

int parse_file(char *file_name);
int parse_string(char *string);

#endif /* PARSER_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

