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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "nemo.h"

/*
 * Creating the "null" object
 */
NmObject *NmNull = &(NmObject){
  .type = OT_NULL,
  .fn = {
    .dstr  = NULL,
    .repr  = NULL,
    .print = NmNull_Print
  }
};

/*
 * Simple singly linked list that contains any object that was allocated and
 * needs to be freed.
 */
typedef struct FreeList FreeList;
static struct FreeList {
  NmObject *ob;
  FreeList *next;
} *free_list = NULL;

/*
 * @name - NmNull_Print
 * @desc - print the "null"
 */
void NmNull_Print(FILE *fp, NmObject *ob)
{
  assert(ob->type == OT_NULL);

  fprintf(fp, "(null)");
}

/*
 * @name - NmObject_NewFromInt
 * @desc - create an object that would contain an int
 */
NmObject *NmObject_NewFromInt(int i)
{
  FreeList *list = NmMem_Malloc(sizeof(FreeList));
  NmIntObject *ob = NmMem_Malloc(sizeof(NmIntObject));

  ob->type = OT_INTEGER;
  ob->i = i;
  ob->fn.dstr = NmInt_Destroy;
  ob->fn.print = NmInt_Print;

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
  return NmObject_NewFromInt((int)p);
}

NmObject *NmObject_NewFromFloat(double f)
{
  FreeList *list = NmMem_Malloc(sizeof(FreeList));
  NmFloatObject *ob = NmMem_Malloc(sizeof(NmFloatObject));

  ob->type = OT_FLOAT;
  ob->f = f;
  ob->fn.dstr = NmFloat_Destroy;
  ob->fn.print = NmFloat_Print;

  /* append to the free_list */
  list->ob = (NmObject *)ob;
  list->next = free_list;
  free_list = list;

  return (NmObject *)ob;
}

void NmFloat_Print(FILE *fp, NmObject *ob)
{
  assert(ob->type == OT_FLOAT);

  fprintf(fp, "%f", ((NmFloatObject *)ob)->f);
}

void NmFloat_Destroy(NmObject *ob)
{
  assert(ob->type == OT_FLOAT);

  NmMem_Free(ob);
}

NmObject *NmObject_NewFromString(char *s)
{
  FreeList *list = NmMem_Malloc(sizeof(FreeList));
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

void NmObject_Destroy(NmObject *ob)
{
  ob->fn.dstr(ob);
}

void NmObject_Tidyup(void)
{
  FreeList *list;
  FreeList *next;

  for (list = free_list; list != NULL; list = next){
    next = list->next;
    NmObject_Destroy((NmObject *)list->ob);
    NmMem_Free(list);
  }
}

