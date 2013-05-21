/*
 *
 * array.c
 *
 * Created at:  Sun 19 May 2013 15:27:38 CEST 15:27:38
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

NmObject *NmArray_New(size_t nmemb)
{
  ObFreeList *list = NmMem_Malloc(sizeof(ObFreeList));
  NmArrayObject *ob = NmMem_Malloc(sizeof(NmArrayObject));
  NmObject **arr = NmMem_Malloc(nmemb * sizeof(NmObject));

  ob->type = OT_ARRAY;
  ob->fn.dstr = NmArray_Destroy;
  ob->fn.type_repr = NmArray_TypeRepr;
  ob->fn.print = NmArray_Print;
  /*ob->fn.binary.add = NmArray_Add;*/
  ob->fn.binary.add = NULL;
  ob->fn.binary.index = NmArray_Index;
  ob->nmemb = nmemb;
  ob->a = arr;

  /* append to the free_list */
  list->ob = (NmObject *)ob;
  list->next = free_list;
  free_list = list;

  return (NmObject *)ob;
}

NmStringObject *NmArray_TypeRepr(void)
{
  return (NmStringObject *)NmString_New("array");
}

void NmArray_Print(FILE *fp, NmObject *ob)
{
  assert(ob);
  assert(ob->type == OT_ARRAY);

  NmArrayObject *arr = (NmArrayObject *)ob;

  fprintf(fp, "[");
  for (size_t i = 0; i < arr->nmemb; i++){
    NmObject_PRINT(fp, arr->a[i]);
    if (i < arr->nmemb - 1){
      fprintf(fp, ", ");
    }
  }
  fprintf(fp, "]");
}

NmObject *NmArray_Index(NmObject *array, NmObject *index)
{
  return NmArray_GETELEM(array, ((NmIntObject *)index)->i);
}

void NmArray_Destroy(NmObject *ob)
{
  NmMem_Free(((NmArrayObject *)ob)->a);
  NmMem_Free(ob);
}

void NmArray_Tidyup(void){
  ObFreeList *list;
  ObFreeList *next;

  for (list = free_list; list != NULL; list = next){
    next = list->next;
    NmObject_Destroy((NmObject *)list->ob);
    NmMem_Free(list);
  }
}

