/*
 *
 * string.c
 *
 * Created at:  Sat 18 May 2013 16:30:05 CEST 16:30:05
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

#include "nemo.h"

static ObFreeList *free_list = NULL;

NmObject *NmString_New(char *s)
{
  ObFreeList *list = NmMem_Malloc(sizeof(ObFreeList));
  NmStringObject *ob = NmMem_Malloc(sizeof(NmStringObject));

  ob->type = OT_STRING;
  ob->s = NmMem_Strdup(s);
  ob->fn.dstr = NmString_Destroy;
  ob->fn.print = NmString_Print;

  /* append to the free_list */
  list->ob = (NmObject *)ob;
  list->next = free_list;
  free_list = list;

  return (NmObject *)ob;
}

void NmString_Print(FILE *fp, NmObject *ob)
{
  assert(ob->type == OT_STRING);

  fprintf(fp, "%s", ((NmStringObject *)ob)->s);
}

void NmString_Destroy(NmObject *ob)
{
  assert(ob->type == OT_STRING);

  NmMem_Free(((NmStringObject *)ob)->s);
  NmMem_Free(ob);
}

void NmString_Tidyup(void){
  ObFreeList *list;
  ObFreeList *next;

  for (list = free_list; list != NULL; list = next){
    next = list->next;
    NmObject_Destroy((NmObject *)list->ob);
    NmMem_Free(list);
  }
}

