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

Nob *nm_new_str(char *s)
{
  ObFreeList *list = nmalloc(sizeof(ObFreeList));
  Nsob *ob = ncalloc(1, sizeof(Nsob));

  ob->type = OT_STRING;
  ob->s = nmalloc(strlen(s) + 1);
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
          nm_set_error("unknown escape sequence '%c'", *(p + 1));
          return NULL;
      }
    } else {
      ob->s[i] = *p;
    }
  }
  ob->s[i] = '\0';
  /* set it's functions */
  ob->fn.print = nm_str_print;
  ob->fn.binary.add = nm_str_add;
  ob->fn.binary.index = nm_str_index;
  ob->fn.binary.cmp = nm_str_cmp;

  /* append to the free_list */
  list->ob = (Nob *)ob;
  list->next = free_list;
  free_list = list;

  return (Nob *)ob;
}

Nob *nm_new_str_from_char(char c)
{
  ObFreeList *list = nmalloc(sizeof(ObFreeList));
  Nsob *ob = ncalloc(1, sizeof(Nsob));

  ob->type = OT_STRING;
  /* create this tiny string */
  ob->s = nmalloc(2);
  ob->s[0] = c;
  ob->s[1] = '\0';
  ob->fn.print = nm_str_print;
  ob->fn.binary.add = nm_str_add;
  ob->fn.binary.index = nm_str_index;
  ob->fn.binary.cmp = nm_str_cmp;

  /* append to the free_list */
  list->ob = (Nob *)ob;
  list->next = free_list;
  free_list = list;

  return (Nob *)ob;
}

Nob *nm_str_add(Nob *left, Nob *right)
{
  size_t leftlen = strlen(nm_str_value(left));
  size_t rightlen = strlen(nm_str_value(right));
  char *new = nmalloc(leftlen + rightlen);

  memcpy(new, nm_str_value(left), leftlen);
  memcpy(new + leftlen, nm_str_value(right), rightlen);

  return nm_new_str(new);
}

CmpRes nm_str_cmp(Nob *left, Nob *right)
{
  int res = strcmp(nm_str_value(left), nm_str_value(right));

  if (res > 0)
    return CMP_GT;
  else if (res < 0)
    return CMP_LT;
  else
    return CMP_EQ;
}

Nob *nm_str_repr(void)
{
  return nm_new_str("string");
}

void nm_str_print(FILE *fp, Nob *ob)
{
  assert(ob->type == OT_STRING);

  fprintf(fp, "%s", nm_str_value(ob));
}

Nob *nm_str_index(Nob *string, Nob *index)
{
  return nm_new_str_from_char(nm_str_value(string)[nm_int_value(index)]);
}

void nm_str_destroy(Nob *ob)
{
  assert(ob->type == OT_STRING);

  nfree(nm_str_value(ob));
  nfree(ob);
}

void nm_str_cleanup(void){
  ObFreeList *list;
  ObFreeList *next;

  for (list = free_list; list != NULL; list = next){
    next = list->next;
    nm_obj_destroy((Nob *)list->ob);
    nfree(list);
  }
}

