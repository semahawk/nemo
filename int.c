/*
 *
 * int.c
 *
 * Created at:  Sat 18 May 2013 16:27:42 CEST 16:27:42
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

/*
 * Simple singly linked list that contains any object that was allocated and
 * needs to be freed.
 */
static ObFreeList *free_list = NULL;

/*
 * @name - NmObject_NewFromInt
 * @desc - create an object that would contain an int
 */
NmObject *NmInt_New(int i)
{
  ObFreeList *list = NmMem_Malloc(sizeof(ObFreeList));
  NmIntObject *ob = NmMem_Malloc(sizeof(NmIntObject));

  ob->type = OT_INTEGER;
  ob->i = i;
  ob->fn.dstr = NmInt_Destroy;
  ob->fn.print = NmInt_Print;
  ob->fn.binary_index = NULL;

  /* append to the free_list */
  list->ob = (NmObject *)ob;
  list->next = free_list;
  free_list = list;

  return (NmObject *)ob;
}

void NmInt_Print(FILE *fp, NmObject *ob)
{
  assert(ob->type == OT_INTEGER);

  fprintf(fp, "%d", ((NmIntObject *)ob)->i);
}

void NmInt_Destroy(NmObject *ob)
{
  assert(ob->type == OT_INTEGER);

  NmMem_Free(ob);
}

NmObject *NmInt_NewFromVoidPtr(void *p)
{
  return NmInt_New((int)p);
}

void NmInt_Tidyup(void){
  ObFreeList *list;
  ObFreeList *next;

  for (list = free_list; list != NULL; list = next){
    next = list->next;
    NmObject_Destroy((NmObject *)list->ob);
    NmMem_Free(list);
  }
}

