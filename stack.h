/*
 *
 * stack.h
 *
 * Created at:  Mon 24 Jun 2013 18:04:07 CEST 18:04:07
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

#ifndef STACK_H
#define STACK_H

#include "mem.h"
#include "object.h"

/* THE Arguments Stack */
struct arg_stack {
  Nob *arg;
  struct arg_stack *next;
};
/* defined in ast.c */
extern struct arg_stack *Nemo_AS;
static inline void arg_stack_print(void)
{
  printf("## stack print\n");
  for (struct arg_stack *p = Nemo_AS; p != NULL; p = p->next){
    printf("   p:%p arg:%p => ", (void*)p, (void*)p->arg);
    nm_obj_print(stdout, p->arg);
    printf("\n");
  }
  printf("##\n");
}
/* the push-ing function */
static inline void arg_stack_push(Nob *arg)
{
  //printf("pushing\n");
  struct arg_stack *el = nmalloc(sizeof(struct arg_stack));
  /* init */
  el->arg = arg;
  /* append */
  el->next = Nemo_AS;
  Nemo_AS = el;
  //arg_stack_dump();
}
/* the pop-ing function */
static inline Nob *arg_stack_pop(void)
{
  //printf("popping\n");
  if (Nemo_AS == NULL){
    /* the stack is empty */
    printf("THE STACK BE EMPTY\n");
    return null;
  }
  /* save the first elements value */
  Nob *ret = Nemo_AS->arg;
  /* save the first elements address */
  struct arg_stack *tmp = Nemo_AS;
  /* update the pointer */
  Nemo_AS = Nemo_AS->next;
  /* freeee */
  nfree(tmp);
  //arg_stack_dump();

  return ret;
}

#endif /* STACK_H */

