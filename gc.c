/*
 *
 * gc.c
 *
 * Created at:  Thu 26 Sep 2013 15:51:08 CEST 15:51:08
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

/* a malloced array of Nob pointers */
Nob **NM_gc_pool;
/* size of the above */
size_t NM_gc_pool_sz;
/* 'pointer' to the current element */
size_t NM_gc_pool_curr;

void gc_init(void)
{
  NM_gc_pool = ncalloc(GC_INITIAL_STACK_SIZE, sizeof(Nob *));
  NM_gc_pool_sz = GC_INITIAL_STACK_SIZE;
  NM_gc_pool_curr = 0;
}

void gc_dump(void)
{
  printf("## GC stack dump\n");
  for (unsigned i = 0; i < NM_gc_pool_curr; i++){
    printf("   %u - %p (%d)\n", i, NM_gc_pool[i], NM_gc_pool[i]->type);
  }
  printf("##\n");
}

void gc_sweep(void)
{
  for (unsigned i = 0; i < NM_gc_pool_curr; i++){
    /* NOP */;
  }
}

void gc_sweepall(void)
{
  for (unsigned i = 0; i < NM_gc_pool_curr; i++){
    nm_obj_destroy(NM_gc_pool[i]);
  }

  nfree(NM_gc_pool);
}

