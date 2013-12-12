/*
 *
 * nob.c
 *
 * Created at:  Sun 24 Nov 16:28:31 2013 16:28:31
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "nob.h"

/* that's not working, obviously */
Nob nob_new_int(int64_t value)
{
  Nob ret;
  uint64_t magic_nibble;

  if (value > NOB_IMMIDIATE_MAX){
    fprintf(stderr, "warning: requested value is too big!\n");
    fprintf(stderr, "warning: %lu vs maximum %u\n", value, NOB_IMMIDIATE_MAX);
    fprintf(stderr, "warning: using the maximum instead!\n");

    value = NOB_IMMIDIATE_MAX;
  }

  if (value < 0){
    magic_nibble = NOB_IMMIDIATE_NEG;
    /* hmm.. that seems to work */
    value = ~value + 1;
  } else {
    magic_nibble = NOB_IMMIDIATE_POS;
  }

  ret = (magic_nibble << 60) | value;

  return ret;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

