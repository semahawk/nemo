/*
 *
 * predef.c
 *
 * Created at:  Fri 17 May 2013 22:26:42 CEST 22:26:42
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License: the MIT license
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

/*
 * "Can't you see
 *  That in the world we live in
 *  Political lies
 *  Are just corporate decisions
 *  They'll take away
 *  All the hopes, not their promises
 *  They'll put an end to this
 *  Land of the livin"
 *
 *  Testament - Souls of Black
 */

#include <stdio.h>
#include <stdlib.h>

#include "nemo.h"

static Nob *predef_len(Nob *args, bool *opts)
{
  /* unused parameter */
  (void)opts;

  Nob *ob = nm_arr_get_elem(args, 0);

  if (ob->type == OT_ARRAY)
    return nm_new_int(nm_arr_nmemb(ob));
  /* it can't be anything else than string down here */
  else
    return nm_new_int(strlen(nm_str_value(ob)));
}

static Nob *predef_assert(Nob *args, bool *opts)
{
  /* unused parameter */
  (void)opts;

  Nob *first  = nm_arr_get_elem(args, 0);
  Nob *second = nm_arr_get_elem(args, 1);

  /* both are of the same type */
  if (first->type == second->type){
    CmpFunc cmpfunc = nm_obj_has_cmp_func(first, BINARY_EQ);
    if (!cmpfunc){
      nm_set_error("can't compare types '%s' and '%s' in 'assert'", nm_str_value(nm_obj_typetos(first)), nm_str_value(nm_obj_typetos(second)));
      return NULL;
    }

    if (cmpfunc(first, second) != CMP_EQ){
      nm_set_error("assertion failed");
      nm_obj_print(stdout, first);
      putchar('\n');
      nm_obj_print(stdout, second);
      return NULL;
    }
  }
  /* they are of different types */
  else {
    /* XXX int and float */
    if (first->type == OT_INTEGER && second->type == OT_FLOAT){
      first = nm_new_float_from_int(nm_int_value(first));
      CmpFunc cmpfunc = nm_obj_has_cmp_func(first, BINARY_EQ);
      if (!cmpfunc){
        nm_set_error("can't compare types '%s' and '%s' in 'assert'", nm_str_value(nm_obj_typetos(first)), nm_str_value(nm_obj_typetos(second)));
        return NULL;
      }
    }
    /* XXX float and int */
    else if (first->type == OT_FLOAT && second->type == OT_INTEGER){
      second = nm_new_float_from_int(nm_int_value(second));
      CmpFunc cmpfunc = nm_obj_has_cmp_func(first, BINARY_EQ);
      if (!cmpfunc){
        nm_set_error("can't compare types '%s' and '%s' in 'assert'", nm_str_value(nm_obj_typetos(first)), nm_str_value(nm_obj_typetos(second)));
        return NULL;
      }
    }
    /* if anything else, the operation is simply not permitted */
    else {
      nm_set_error("can't compare types '%s' and '%s' in 'assert'", nm_str_value(nm_obj_typetos(first)), nm_str_value(nm_obj_typetos(second)));
      return NULL;
    }
  }

  return nm_new_int(1);
}

static Nob *predef_printf(Nob *args, bool *opts)
{
  /* unused parameter */
  (void)opts;

  /* skip over the formatting string */
  size_t i = 1;
  /* number of arguments without the formatting string */
  size_t count = 0;
  char *p;

  Nob *format = nm_arr_get_elem(args, 0);

  for (p = nm_str_value(format); *p != '\0'; p++){
    if (*p == '%'){
      switch (*(p + 1)){
        case 'i': /* fall */
        case 'f': /* through */
        case 's':
        case 'a':
        case '%':
          count++;
          break;
        default: nm_set_error("unknown format '%c' in 'printf'", *(p + 1));
                 return NULL;
      }
    }
  }

  /* search if all the formats are valid */
  for (p = nm_str_value(format); *p != '\0'; p++){
    if (*p == '%'){
      if (count + 1 > nm_arr_nmemb(args)){
        nm_set_error("not enough formatting arguments for 'printf' (%d when %d expected)", nm_arr_nmemb(args) - 1, count);
        return NULL;
      }
      /* fetch the next argument */
      Nob *ob = nm_arr_get_elem(args, i);
      switch (*(p + 1)){
        case 'i':
          if (ob->type != OT_INTEGER){
            nm_set_error("wrong type '%s' for format 'i'", nm_str_value(nm_obj_typetos(ob)));
            return NULL;
          }
          i++; p++;
          break;
        case 'f':
          if (ob->type != OT_FLOAT){
            nm_set_error("wrong type '%s' for format 'f'", nm_str_value(nm_obj_typetos(ob)));
            return NULL;
          }
          i++; p++;
          break;
        case 's':
          if (ob->type != OT_STRING){
            nm_set_error("wrong type '%s' for format 's'", nm_str_value(nm_obj_typetos(ob)));
            return NULL;
          }
          i++; p++;
          break;
        case 'a':
          if (ob->type != OT_ARRAY){
            nm_set_error("wrong type '%s' for format 'a'", nm_str_value(nm_obj_typetos(ob)));
            return NULL;
          }
          i++; p++;
          break;
        case '%':
          p++;
          break;
        default: /* should never get here as it should've been already checked up there */
                 return NULL;
      }
    }
  }

  /* check if the number of arguments is correct */
  if (nm_arr_nmemb(args) > i){
    nm_set_error("too much formatting arguments for 'printf' (%d when %d expected)", nm_arr_nmemb(args) - 1, count);
    return NULL;
  }

  /* skip over the formatting string */
  i = 1;

  for (p = nm_str_value(format); *p != '\0'; p++){
    if (*p == '%'){
      /* fetch the next argument */
      Nob *ob = nm_arr_get_elem(args, i);
      switch (*(p + 1)){
        case 'i': /* fall */
        case 'f': /* through */
        case 's':
        case 'a':
          nm_obj_print(stdout, ob);
          p++;
          break;
        case '%': putchar('%');
                  break;
        default: /* should never get here as it should've been already checked up there */
                 return NULL;
      }
      i++;
    } else {
      putchar(*p);
    }
  }

  return nm_new_int(1);
}

static Nob *predef_print(Nob *args, bool *opts)
{
  for (size_t i = 0; i < nm_arr_nmemb(args); i++){
    nm_obj_print(stdout, nm_arr_get_elem(args, i));
  }

  if (opts[0])
    fputc('\n', stdout);

  return nm_new_int(1);
}

static Nob *predef_id(Nob *args, bool *opts)
{
  /* unused parameter */
  (void)opts;

  return nm_new_int_from_void_ptr((void *)nm_arr_get_elem(args, 0));
}

static Nob *predef_eval(Nob *args, bool *opts)
{
  /* unused parameter */
  (void)opts;

  Nob *ob = nm_arr_get_elem(args, 0);

  return nm_ast_exec_block(nm_parse_string(nm_str_value(ob)));
}

static Nob *predef_open(Nob *args, bool *opts)
{
  char *name = nm_str_value(nm_arr_get_elem(args, 0));
  char mode[4] = "r";
  Nfhob *new = ncalloc(1, sizeof(Nfhob));
  FILE *fp;

  if (opts[0])
    ;
  else if (opts[1])
    strcpy(mode, "w");
  else if (opts[2])
    strcpy(mode, "a");

  fp = fopen(name, mode);

  if (!fp){
    nm_set_error(strerror(errno));
    return null;
  }

  new->type = OT_FILE;
  new->fp = fp;
  new->name = nm_strdup(name);
  new->fn.print = nm_file_print;

  return (Nob *)new;
}

static Nob *predef_close(Nob *args, bool *opts)
{
  /* unused parameter */
  (void)opts;

  Nob *arg = nm_arr_get_elem(args, 0);

  fclose(((Nfhob *)arg)->fp);

  return nm_new_int(1);
}

static NmModuleFuncs module_funcs[] = {
  { "assert", predef_assert,  2, { OT_ANY, OT_ANY }, "" },
  { "close",  predef_close,   1, { OT_FILE }, "" },
  { "eval",   predef_eval,    1, { OT_STRING }, "" },
  { "id",     predef_id,      1, { OT_ANY }, "" },
  { "len",    predef_len,     1, { OT_STRING | OT_ARRAY }, "" },
  { "open",   predef_open,    1, { OT_STRING }, "rwa" },
  { "print",  predef_print,  -1, { OT_ANY }, "n" },
  { "printf", predef_printf, -1, { OT_ANY }, "" },
  { NULL, NULL, 0, { 0 }, NULL }
};

void predef_init(void)
{
  nm_init_module(module_funcs);
}

