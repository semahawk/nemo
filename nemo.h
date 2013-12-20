/*
 *
 * nemo.h
 *
 * Created at:  Sat Nov  9 10:58:55 2013 10:58:55
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License: please visit the LICENSE file for details.
 *
 */

#ifndef NEMO_H
#define NEMO_H

#include <stdint.h>

#if HAVE_STDBOOL_H
# include <stdbool.h>
#else /* HAVE_STDBOOL_H */
# define bool short
# define true 1
# define false 0
#endif /* HAVE_STDBOOL_H */

/* one small, quite handy, typedef */
typedef unsigned char byte_t;

/* globalize the flags (defined in nemo.c) */
extern uint32_t NM_debug_flags;
/* the debug flags */
#define NM_DEBUG_MEM   (1 << 0)  /* -dm */
#define NM_DEBUG_LEXER (1 << 1)  /* -dl */
/* there are, obviously, more to come :) */
/* few more handy macros to set/get certain debug flags */
#define NM_DEBUG_SET_FLAG(f) (NM_debug_flags |= (f))
#define NM_DEBUG_GET_FLAG(f) (NM_debug_flags &  (f))

#endif /* NEMO_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

