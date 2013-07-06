/*
 *
 * builtin.c
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

static NmObject *builtin_len(NmObject *args)
{
  NmObject *ob = NmArray_GETELEM(args, 0);

  if (ob->type != OT_ARRAY && ob->type != OT_STRING){
    NmError_SetString("wrong argument type for function 'len' (%s when 'string' or 'array' expected)", NmString_VAL(ob->fn.type_repr()));
    return NULL;
  }

  if (ob->type == OT_ARRAY)
    return NmInt_New(NmArray_NMEMB(ob));
  /* it can't be anything else than string down here */
  else
    return NmInt_New(strlen(NmString_VAL(ob)));
}

static NmObject *builtin_assert(NmObject *args)
{
  NmObject *first  = NmArray_GETELEM(args, 0);
  NmObject *second = NmArray_GETELEM(args, 1);

  /* both are of the same type */
  if (first->type == second->type){
    if (!first->fn.binary.cmp){
      NmError_SetString("can't compare types '%s' and '%s' in 'assert'", NmString_VAL(first->fn.type_repr()), NmString_VAL(second->fn.type_repr()));
      return NULL;
    }

    if (first->fn.binary.cmp(first, second) != CMP_EQ){
      NmError_SetString("assertion failed");
      return NULL;
    }
  }
  /* they are of different types */
  else {
    /* XXX int and float */
    if (first->type == OT_INTEGER && second->type == OT_FLOAT){
      first = NmFloat_NewFromInt(NmInt_VAL(first));
      if (!first->fn.binary.cmp){
        NmError_SetString("can't compare types '%s' and '%s' in 'assert'", NmString_VAL(first->fn.type_repr()), NmString_VAL(second->fn.type_repr()));
        return NULL;
      }
    }
    /* XXX float and int */
    else if (first->type == OT_FLOAT && second->type == OT_INTEGER){
      second = NmFloat_NewFromInt(NmInt_VAL(second));
      if (!first->fn.binary.cmp){
        NmError_SetString("can't compare types '%s' and '%s' in 'assert'", NmString_VAL(first->fn.type_repr()), NmString_VAL(second->fn.type_repr()));
        return NULL;
      }
    }
    /* if anything else, the operation is simply not permitted */
    else {
      NmError_SetString("can't compare types '%s' and '%s' in 'assert'", NmString_VAL(first->fn.type_repr()), NmString_VAL(second->fn.type_repr()));
      return NULL;
    }
  }

  return NmInt_New(1);
}

static NmObject *builtin_printf(NmObject *args)
{
  /* skip over the formatting string */
  size_t i = 1;
  /* number of arguments without the formatting string */
  size_t count = 0;
  char *p;

  NmObject *format = NmArray_GETELEM(args, 0);

  for (p = NmString_VAL(format); *p != '\0'; p++){
    if (*p == '%'){
      switch (*(p + 1)){
        case 'i': /* fall */
        case 'f': /* through */
        case 's':
        case 'a':
        case '%':
          count++;
          break;
        default: NmError_SetString("unknown format '%c' in 'printf'", *(p + 1));
                 return NULL;
      }
    }
  }

  /* search if all the formats are valid */
  for (p = NmString_VAL(format); *p != '\0'; p++){
    if (*p == '\\'){
      switch (*(p + 1)){
        case  'n': /* fall */
        case  't': /* through */
        case  'a':
        case  '%':
        case '\\':
          p++;
          break;
        default: NmError_SetString("unknown escaping sequence '%c%c' in 'printf'", '\\', *(p + 1));
                 return NULL;
      }
    } else if (*p == '%'){
      if (count + 1 > NmArray_NMEMB(args)){
        NmError_SetString("not enough formatting arguments for 'printf' (%d when %d expected)", NmArray_NMEMB(args) - 1, count);
        return NULL;
      }
      /* fetch the next argument */
      NmObject *ob = NmArray_GETELEM(args, i);
      switch (*(p + 1)){
        case 'i':
          if (ob->type != OT_INTEGER){
            NmError_SetString("wrong type '%s' for format 'i'", NmString_VAL(ob->fn.type_repr()));
            return NULL;
          }
          i++; p++;
          break;
        case 'f':
          if (ob->type != OT_FLOAT){
            NmError_SetString("wrong type '%s' for format 'f'", NmString_VAL(ob->fn.type_repr()));
            return NULL;
          }
          i++; p++;
          break;
        case 's':
          if (ob->type != OT_STRING){
            NmError_SetString("wrong type '%s' for format 's'", NmString_VAL(ob->fn.type_repr()));
            return NULL;
          }
          i++; p++;
          break;
        case 'a':
          if (ob->type != OT_ARRAY){
            NmError_SetString("wrong type '%s' for format 'a'", NmString_VAL(ob->fn.type_repr()));
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
  if (NmArray_NMEMB(args) > i){
    NmError_SetString("too much formatting arguments for 'printf' (%d when %d expected)", NmArray_NMEMB(args) - 1, count);
    return NULL;
  }

  /* skip over the formatting string */
  i = 1;

  for (p = NmString_VAL(format); *p != '\0'; p++){
    if (*p == '\\'){
      switch (*(p + 1)){
        case 'n':  putchar('\n');
                   p++;
                   break;
        case 't':  putchar('\t');
                   p++;
                   break;
        case 'a':  putchar('\a');
                   p++;
                   break;
        case '%':  putchar('%');
                   p++;
        case '\\': putchar('\\');
                   p++;
                   break;
        default: /* should never get here as it should've been already checked up there */
                   return NULL;
      }
    } else if (*p == '%'){
      /* fetch the next argument */
      NmObject *ob = NmArray_GETELEM(args, i);
      switch (*(p + 1)){
        case 'i': /* fall */
        case 'f': /* through */
        case 's':
        case 'a':
          NmObject_PRINT(stdout, ob);
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

  return NmInt_New(1);
}

static NmObject *builtin_print(NmObject *args)
{
  for (size_t i = 0; i < NmArray_NMEMB(args); i++){
    NmObject_PRINT(stdout, NmArray_GETELEM(args, i));
  }

  return NmInt_New(1);
}

static NmObject *builtin_id(NmObject *args)
{
  return NmInt_NewFromVoidPtr((void *)NmArray_GETELEM(args, 0));
}

static NmObject *builtin_eval(NmObject *args)
{
  NmObject *ob = NmArray_GETELEM(args, 0);

  if (ob->type != OT_STRING){
    NmError_SetString("wrong argument type for function 'len' (%s when 'string' expected)", NmString_VAL(ob->fn.type_repr()));
    return NULL;
  }

  return NmAST_Exec(NmParser_ParseString(NmString_VAL(ob)));
}

static NmModuleFuncs module_funcs[] = {
  { "assert", builtin_assert,  2, "" },
  { "eval",   builtin_eval,    1, "" },
  { "id",     builtin_id,      1, "" },
  { "len",    builtin_len,     1, "" },
  { "print",  builtin_print,  -1, "n" },
  { "printf", builtin_printf, -1, "" },
  { NULL, NULL, 0, NULL }
};

void NmBuiltin_Init(void)
{
  Nm_InitModule(module_funcs);
}

