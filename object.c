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
 * Check if given value is a true/false boolean-wise.
 *
 * In Nemo there is no "bool" type as is.
 */
BOOL NmObject_Boolish(NmObject *o)
{
  /*
   * null, 0, 0.0 and empty string ("") are false
   * everything else is true
   */
  switch (o->type){
    case OT_NULL:
      return FALSE;
    case OT_INTEGER:
      if (((NmIntObject *)o)->i == 0)
        return FALSE;
      else
        return TRUE;
    case OT_FLOAT:
      if (((NmFloatObject *)o)->f == 0.0f)
        return FALSE;
      else
        return TRUE;
    case OT_STRING:
      if (!strcmp(((NmStringObject *)o)->s, ""))
        return FALSE;
      else
        return TRUE;
    default:
      return FALSE;
  }
}

