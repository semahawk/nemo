/*
 *
 * object.c
 *
 * Created at:  Sat 11 May 2013 20:27:13 CEST 20:27:13
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
 * "False sense of pride, satisfies
 *  There's no reason for suicide
 *  Use your mind, and hope to find
 *  Find the meaning of existence..."
 *
 *  Testament - Sins of Omission
 */

#include "nemo.h"

void nm_obj_destroy(NmObject *ob)
{
  ob->fn.dstr(ob);
}

void nm_obj_cleanup(void)
{
  nm_int_cleanup();
  nm_float_cleanup();
  nm_str_cleanup();
  nm_arr_cleanup();
}

/*
 * Duplicates given object and returns the copy.
 */
NmObject *nm_obj_dup(NmObject *ob)
{
  NmObject *new = nmalloc(sizeof(*ob));

  switch (ob->type){
    case OT_INTEGER:
      *(NmIntObject *)new = *(NmIntObject *)ob;
      break;
    case OT_FLOAT:
      *(NmFloatObject *)new = *(NmFloatObject *)ob;
      break;
    case OT_STRING:
      *(NmStringObject *)new = *(NmStringObject *)ob;
      break;
    case OT_ARRAY:
      *(NmArrayObject *)new = *(NmArrayObject *)ob;
      break;
    case OT_NULL:
      *(NmNullObject *)new = *(NmNullObject *)ob;
      break;
    case OT_FILE:
      *(NmFileObject *)new = *(NmFileObject *)ob;
      break;
    case OT_ANY:
      /* suspress the warnings */
      break;
  }

  return (NmObject *)new;
}

/*
 * Translates given <type> to a string.
 */
NmObject *nm_obj_typetos(NmObjectType type)
{
  NmObject *ret = nm_new_str("");
  bool first = true;

  unsigned num = 7;
  for (unsigned i = 0; i < 7; i++)
    if (type & (1 << i))
      num++;

  unsigned j = 0;
  for (unsigned i = 0; i < 7; i++){
    if (type & (1 << i)){
      if (!first)
        ret = nm_str_add(ret, nm_new_str(" or "));
      first = false;
      switch (type & (1 << i)){
        case OT_NULL:
          ret = nm_str_add(ret, nm_new_str("null"));
          j++;
          break;
        case OT_INTEGER:
          ret = nm_str_add(ret, nm_new_str("integer"));
          j++;
          break;
        case OT_FLOAT:
          ret = nm_str_add(ret, nm_new_str("float"));
          j++;
          break;
        case OT_STRING:
          ret = nm_str_add(ret, nm_new_str("string"));
          j++;
          break;
        case OT_ARRAY:
          ret = nm_str_add(ret, nm_new_str("array"));
          j++;
          break;
        case OT_FILE:
          ret = nm_str_add(ret, nm_new_str("file handle"));
          j++;
          break;
      }
    }
  }

  return ret;
}

/*
 * Check if given value is a true/false boolean-wise.
 *
 * In Nemo there is no "bool" type as is.
 */
bool nm_obj_boolish(NmObject *ob)
{
  /*
   * null, 0, 0.0, empty string ("") and an empty array ([]) are false
   * everything else is true
   */
  switch (ob->type){
    case OT_NULL:
      return false;
    case OT_INTEGER:
      if (nm_int_value(ob) == 0)
        return false;
      else
        return true;
    case OT_FLOAT:
      if (nm_float_value(ob) == 0.0f)
        return false;
      else
        return true;
    case OT_STRING:
      if (!strcmp(nm_str_value(ob), ""))
        return false;
      else
        return true;
    case OT_ARRAY:
      if (nm_arr_nmemb(ob) == 0)
        return false;
      else
        return true;
    case OT_FILE:
      return true;
    default:
      return false;
  }
}

