/*
 *
 * mem.h
 *
 * Created at:  Sat 16 Nov 15:10:05 2013 15:10:05
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef MEM_H
#define MEM_H

#include <stdio.h>
#include <stdlib.h>

#define nmalloc(s) nmalloc_(s, __FILE__, __LINE__)
void *nmalloc_(size_t size, const char *file, unsigned line);

#define ncalloc(n,s) ncalloc_(n, s, __FILE__, __LINE__)
void *ncalloc_(size_t number, size_t size, const char *file, unsigned line);

#define nrealloc(s,n) nrealloc_(s, n, __FILE__, __LINE__)
void *nrealloc_(void *ptr, size_t size, const char *file, unsigned line);

#define nfree(p) nfree_(p, __FILE__, __LINE__)
void nfree_(void *ptr, const char *file, unsigned line);

#endif /* MEM_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

