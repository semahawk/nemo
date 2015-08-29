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
#include "scope.h"

struct parser {
  /* whether there were any errors or not */
  /* ie. if `true', it means the code can be safely executed (no error/warning
   * message was critical) */
  bool errorless;
  /* the current scope the 'compilation control' is in */
  struct scope *curr_scope;
  /* list of type variables currently 'in use' */
  /* whenever a type variable is created with a certain name, we want to refer
   * to the same underlaying structure, not a new one */
  struct type_variables_list *type_vars;
};

struct node *parse_file(char *file_name, struct scope *scope);
struct node *parse_string(char *name, char *string, struct scope *scope);

#endif /* PARSER_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

