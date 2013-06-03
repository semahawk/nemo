/*
 *
 * mem.c
 *
 * Created at:  Sun Apr 14 13:30:14 2013 13:30:14
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
 * "Hiding in any doorway, in any shadow
 *  Any place where danger waits to kill my time
 *  there is one who loves and waits, to seal your fate
 *  As sure as you live and die"
 *
 *  Megadeth - Never Walk Alone
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nemo.h"

void *NmMem_Malloc(size_t size)
{
  void *p = malloc(size);

  if (!p){
    NmError_Fatal("malloc failed to allocate %lu bytes", size);
    exit(EXIT_FAILURE);
  }

  NmDebug_MALLOC(p, size);

  return p;
}

void *NmMem_Calloc(size_t nmemb, size_t size)
{
  void *p = calloc(nmemb, size);

  if (!p){
    NmError_Fatal("calloc failed to allocate %lu x %lu bytes", nmemb, size);
    exit(EXIT_FAILURE);
  }

  NmDebug_CALLOC(p, nmemb, size);

  return p;
}

void *NmMem_Realloc(void *ptr, size_t nmemb)
{
  void *p = realloc(ptr, nmemb);

  if (!p){
    NmError_Fatal("realloc failed to reallocate %lu bytes", nmemb);
    exit(EXIT_FAILURE);
  }

  NmDebug_REALLOC(p, nmemb);

  return p;
}

void NmMem_Free(void *p)
{
  NmDebug_FREE(p);

  free(p);
}

/*
 * Returns a copyied string or NULL if the string is NULL.
 */
char *NmMem_Strdup(char *p)
{
  if (!p)
    return NULL;

  char *np = NmMem_Malloc(strlen(p) + 1);
  return np ? strcpy(np, p) : np;
}

