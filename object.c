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

void NmObject_Destroy(NmObject *ob)
{
  ob->fn.dstr(ob);
}

void NmObject_Tidyup(void)
{
  NmInt_Tidyup();
  NmFloat_Tidyup();
  NmString_Tidyup();
  NmArray_Tidyup();
}

/*
 * Duplicates given object and returns the copy.
 */
NmObject *NmObject_Dup(NmObject *ob)
{
  NmObject *new = NmMem_Malloc(sizeof(*ob));

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
    case OT_ANY:
      /* suspress the warnings */
      break;
  }

  return (NmObject *)new;
}

/*
 * Translates given <type> to a string.
 */
NmObject *NmObject_TypeToS(NmObjectType type)
{
  NmObject *ret = NmString_New("");
  bool first = true;

  unsigned num = 7;
  for (unsigned i = 0; i < 7; i++)
    if (type & (1 << i))
      num++;

  unsigned j = 0;
  for (unsigned i = 0; i < 7; i++){
    if (type & (1 << i)){
      if (!first)
        ret = NmString_Add(ret, NmString_New(" or "));
      first = false;
      switch (type & (1 << i)){
        case OT_NULL:
          ret = NmString_Add(ret, NmString_New("null"));
          j++;
          break;
        case OT_INTEGER:
          ret = NmString_Add(ret, NmString_New("integer"));
          j++;
          break;
        case OT_FLOAT:
          ret = NmString_Add(ret, NmString_New("float"));
          j++;
          break;
        case OT_STRING:
          ret = NmString_Add(ret, NmString_New("string"));
          j++;
          break;
        case OT_ARRAY:
          ret = NmString_Add(ret, NmString_New("array"));
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
bool NmObject_Boolish(NmObject *ob)
{
  /*
   * null, 0, 0.0, empty string ("") and an empty array ([]) are false
   * everything else is true
   */
  switch (ob->type){
    case OT_NULL:
      return false;
    case OT_INTEGER:
      if (NmInt_VAL(ob) == 0)
        return false;
      else
        return true;
    case OT_FLOAT:
      if (NmFloat_VAL(ob) == 0.0f)
        return false;
      else
        return true;
    case OT_STRING:
      if (!strcmp(NmString_VAL(ob), ""))
        return false;
      else
        return true;
    case OT_ARRAY:
      if (NmArray_NMEMB(ob) == 0)
        return false;
      else
        return true;
    default:
      return false;
  }
}

