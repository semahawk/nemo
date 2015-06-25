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

#include <stdio.h>
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

/* the file into which all the assembly will go */
extern FILE *outfile;

#endif /* NEMO_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

