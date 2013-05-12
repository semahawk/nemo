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
#include "object.h"
#include "mem.h"

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
 * @name - NmObject_NewFromInt
 * @desc - create an object that would contain an int
 */
NmObject *NmObject_NewFromInt(Nemo *NM, int i)
{
  FreeList *list = NmMem_Malloc(NM, sizeof(FreeList));
  NmIntObject *ob = NmMem_Malloc(NM, sizeof(NmIntObject));

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

void NmInt_Print(Nemo *NM, FILE *fp, NmObject *ob)
{
  assert(ob->type == OT_INTEGER);

  /* unused parameter */
  (void)NM;

  fprintf(fp, "%d", ((NmIntObject *)ob)->i);
}

void NmInt_Destroy(Nemo *NM, NmObject *ob)
{
  assert(ob->type == OT_INTEGER);

  NmMem_Free(NM, ob);
}

NmObject *NmObject_NewFromFloat(Nemo *NM, double f)
{
  FreeList *list = NmMem_Malloc(NM, sizeof(FreeList));
  NmFloatObject *ob = NmMem_Malloc(NM, sizeof(NmFloatObject));

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

void NmFloat_Print(Nemo *NM, FILE *fp, NmObject *ob)
{
  assert(ob->type == OT_FLOAT);

  /* unused parameter */
  (void)NM;

  fprintf(fp, "%f", ((NmFloatObject *)ob)->f);
}

void NmFloat_Destroy(Nemo *NM, NmObject *ob)
{
  assert(ob->type == OT_FLOAT);

  NmMem_Free(NM, ob);
}

NmObject *NmObject_NewFromString(Nemo *NM, char *s)
{
  FreeList *list = NmMem_Malloc(NM, sizeof(FreeList));
  NmStringObject *ob = NmMem_Malloc(NM, sizeof(NmStringObject));

  ob->type = OT_STRING;
  ob->s = NmMem_Strdup(NM, s);
  ob->fn.dstr = NmString_Destroy;
  ob->fn.print = NmString_Print;

  /* append to the free_list */
  list->ob = (NmObject *)ob;
  list->next = free_list;
  free_list = list;

  return (NmObject *)ob;
}

void NmString_Print(Nemo *NM, FILE *fp, NmObject *ob)
{
  assert(ob->type == OT_STRING);

  /* unused parameter */
  (void)NM;

  fprintf(fp, "%s", ((NmStringObject *)ob)->s);
}

void NmString_Destroy(Nemo *NM, NmObject *ob)
{
  assert(ob->type == OT_STRING);

  NmMem_Free(NM, ((NmStringObject *)ob)->s);
  NmMem_Free(NM, ob);
}

void NmObject_Destroy(Nemo *NM, NmObject *ob)
{
  ob->fn.dstr(NM, ob);
}

void NmObject_Tidyup(Nemo *NM)
{
  FreeList *list;
  FreeList *next;

  for (list = free_list; list != NULL; list = next){
    next = list->next;
    NmObject_Destroy(NM, (NmObject *)list->ob);
    NmMem_Free(NM, list);
  }
}

