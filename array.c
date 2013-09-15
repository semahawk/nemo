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

NmObject *nm_new_arr(size_t nmemb)
{
  ObFreeList *list = nmalloc(sizeof(ObFreeList));
  NmArrayObject *ob = ncalloc(1, sizeof(NmArrayObject));
  NmObject **arr = nmalloc(nmemb * sizeof(NmObject));

  ob->type = OT_ARRAY;
  ob->fn.dstr = nm_arr_destroy;
  ob->fn.type_repr = nm_arr_repr;
  ob->fn.print = nm_arr_print;
  ob->fn.binary.add = nm_arr_add;
  ob->fn.binary.index = nm_arr_index;
  ob->nmemb = nmemb;
  ob->a = arr;

  /* append to the free_list */
  list->ob = (NmObject *)ob;
  list->next = free_list;
  free_list = list;

  return (NmObject *)ob;
}

NmObject *nm_arr_repr(void)
{
  return nm_new_str("array");
}

NmObject *nm_arr_add(NmObject *left, NmObject *right)
{
  NmObject *new = nm_new_arr(nm_arr_nmemb(left) + nm_arr_nmemb(right));

  unsigned i;

  for (i = 0; i < nm_arr_nmemb(left); i++){
    nm_arr_set_elem(new, i, nm_arr_get_elem(left, i));
  }

  for (; i < nm_arr_nmemb(left) + nm_arr_nmemb(right); i++){
    nm_arr_set_elem(new, i, nm_arr_get_elem(right, i - nm_arr_nmemb(left)));
  }

  return new;
}

void nm_arr_print(FILE *fp, NmObject *ob)
{
  assert(ob);
  assert(ob->type == OT_ARRAY);

  NmArrayObject *arr = (NmArrayObject *)ob;

  fprintf(fp, "[");
  for (size_t i = 0; i < arr->nmemb; i++){
    nm_obj_print(fp, arr->a[i]);
    if (i < arr->nmemb - 1){
      fprintf(fp, ", ");
    }
  }
  fprintf(fp, "]");
}

NmObject *nm_arr_index(NmObject *array, NmObject *index)
{
  return nm_arr_get_elem(array, nm_int_value(index));
}

void nm_arr_destroy(NmObject *ob)
{
  nfree(nm_arr_value(ob));
  nfree(ob);
}

void nm_arr_cleanup(void){
  ObFreeList *list;
  ObFreeList *next;

  for (list = free_list; list != NULL; list = next){
    next = list->next;
    nm_obj_destroy((NmObject *)list->ob);
    nfree(list);
  }
}

