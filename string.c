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

/*
 * Simple singly linked list that contains any object that was allocated and
 * needs to be freed.
 */
static ObFreeList *free_list = NULL;

NmObject *NmString_New(char *s)
{
  ObFreeList *list = NmMem_Malloc(sizeof(ObFreeList));
  NmStringObject *ob = NmMem_Calloc(1, sizeof(NmStringObject));

  ob->type = OT_STRING;
  ob->s = NmMem_Malloc(strlen(s) + 1);
  /* set the strings contents, char by char */
  unsigned i = 0;
  for (char *p = s; *p != '\0'; p++, i++){
    if (*p == '\\'){
      switch (*(p + 1)){
        case 'a':
          ob->s[i] = '\a';
          p++;
          break;
        case 'b':
          ob->s[i] = '\b';
          p++;
          break;
        case 'e':
          /* huh, my Vim highlights that as an error */
          ob->s[i] = '\e';
          p++;
          break;
        case 'f':
          ob->s[i] = '\f';
          p++;
          break;
        case 'n':
          ob->s[i] = '\n';
          p++;
          break;
        case 'r':
          ob->s[i] = '\r';
          p++;
          break;
        case 't':
          ob->s[i] = '\t';
          p++;
          break;
        case '\\':
          ob->s[i] = '\\';
          p++;
          break;
        case '"':
          ob->s[i] = '"';
          p++;
          break;
        default:
          NmError_SetString("unknown escape sequence '%c'", *(p + 1));
          return NULL;
      }
    } else {
      ob->s[i] = *p;
    }
  }
  ob->s[i] = '\0';
  /* set it's functions */
  ob->fn.dstr = NmString_Destroy;
  ob->fn.type_repr = NmString_TypeRepr;
  ob->fn.print = NmString_Print;
  ob->fn.binary.add = NmString_Add;
  ob->fn.binary.index = NmString_Index;

  /* append to the free_list */
  list->ob = (NmObject *)ob;
  list->next = free_list;
  free_list = list;

  return (NmObject *)ob;
}

NmObject *NmString_NewFromChar(char c)
{
  ObFreeList *list = NmMem_Malloc(sizeof(ObFreeList));
  NmStringObject *ob = NmMem_Calloc(1, sizeof(NmStringObject));

  ob->type = OT_STRING;
  /* create this tiny string */
  ob->s = NmMem_Malloc(2);
  ob->s[0] = c;
  ob->s[1] = '\0';
  ob->fn.dstr = NmString_Destroy;
  ob->fn.print = NmString_Print;
  /*ob->fn.binary.add = NmString_Add;*/
  ob->fn.binary.add = NULL;
  ob->fn.binary.index = NmString_Index;

  /* append to the free_list */
  list->ob = (NmObject *)ob;
  list->next = free_list;
  free_list = list;

  return (NmObject *)ob;
}

NmObject *NmString_Add(NmObject *left, NmObject *right)
{
  size_t leftlen = strlen(NmString_VAL(left));
  size_t rightlen = strlen(NmString_VAL(right));
  char *new = NmMem_Malloc(leftlen + rightlen);

  memcpy(new, NmString_VAL(left), leftlen);
  memcpy(new + leftlen, NmString_VAL(right), rightlen);

  return NmString_New(new);
}

NmObject *NmString_TypeRepr(void)
{
  return NmString_New("string");
}

void NmString_Print(FILE *fp, NmObject *ob)
{
  assert(ob->type == OT_STRING);

  fprintf(fp, "%s", NmString_VAL(ob));
}

NmObject *NmString_Index(NmObject *string, NmObject *index)
{
  return NmString_NewFromChar(NmString_VAL(string)[NmInt_VAL(index)]);
}

void NmString_Destroy(NmObject *ob)
{
  assert(ob->type == OT_STRING);

  NmMem_Free(NmString_VAL(ob));
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

