/*
 *
 * gc.h
 *
 * Created at:  Tue 24 Sep 2013 22:14:35 CEST 22:14:35
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

#ifndef GC_H
#define GC_H

#include "nemo.h"

#define GC_INITIAL_STACK_SIZE 50
#define GC_STACK_GROW_RATIO   1.5

#define GC_WHITE 0x00
#define GC_GRAY  0x01
#define GC_BLACK 0x02

/* All three defined in gc.c */
extern Nob **NM_gc_pool;
extern size_t NM_gc_pool_sz;
extern size_t NM_gc_pool_curr;

void gc_init(void);
void gc_dump(void);
void gc_sweep(void);
void gc_sweepall(void);

#define gc_push(ob) gc_push_((Nob *)ob)
static inline void gc_push_(Nob *ob)
{
  if (NM_gc_pool_curr > NM_gc_pool_sz){
    NM_gc_pool_sz += NM_gc_pool_sz * GC_STACK_GROW_RATIO;
    NM_gc_pool = nrealloc(NM_gc_pool, NM_gc_pool_sz);
  }

  NM_gc_pool[NM_gc_pool_curr++] = (Nob *)ob;
}

#endif /* GC_H */

