/*
 *
 * debug.h
 *
 * Created at:  Mon 30 Dec 18:16:35 2013 18:16:35
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>
#include <stdarg.h>

#include "ast.h"

/* globalize the flags (defined in debug.c) */
extern uint32_t NM_debug_flags;
/* the debug flags */
#define NM_DEBUG_AST    (1 << 0)  /* -da */
#define NM_DEBUG_LEXER  (1 << 1)  /* -dl */
#define NM_DEBUG_MEM    (1 << 2)  /* -dm */
#define NM_DEBUG_PARSER (1 << 3)  /* -dp */
#define NM_DEBUG_TYPES  (1 << 4)  /* -dt */
/* there are, obviously, more to come :) */
/* few more handy macros to set/get certain debug flags */
#define NM_DEBUG_SET_FLAG(f) (NM_debug_flags |= (f))
#define NM_DEBUG_GET_FLAG(f) (NM_debug_flags &  (f))

void debug_ast_new (struct node *node, const char *fmt, ...);
void debug_ast_exec(struct node *node, const char *fmt, ...);
void debug_ast_comp(struct node *node, const char *fmt, ...);

#endif /* DEBUG_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

