/*
 *
 * debug.c
 *
 * Created at:  Mon 30 Dec 18:20:07 2013 18:20:07
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>

#include "ast.h"
#include "debug.h"

/* the bit pattern of the enabled debug flags */
/* see debug.h for more details */
uint32_t NM_debug_flags = 0;

void debug_ast_new(struct node *nd, const char *fmt, ...)
{
  if (NM_DEBUG_GET_FLAG(NM_DEBUG_AST)){
    va_list vl;

    va_start(vl, fmt);
    fprintf(stderr, "%p:  new node: ", (void *)nd);
    vfprintf(stderr, fmt, vl);
    fprintf(stderr, "\n");
    va_end(vl);
  }
}

void debug_ast_exec(struct node *nd, const char *fmt, ...)
{
  if (NM_DEBUG_GET_FLAG(NM_DEBUG_AST)){
    va_list vl;

    va_start(vl, fmt);
    fprintf(stderr, "%p: exec node: ", (void *)nd);
    vfprintf(stderr, fmt, vl);
    fprintf(stderr, "\n");
    va_end(vl);
  }
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

