/*
 *
 * utf8.c
 *
 * Created at:  Mon Nov 11 19:14:49 2013 19:14:49
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdio.h>
#include <stdlib.h>

/* credit: http://canonical.org/~kragen/strlen-utf8.html */
unsigned u8_strlen(char *s)
{
  size_t i = 0, j = 0;

  while (s[i]){
    if ((s[i] & 0xc0) != 0x80)
      j++;
    i++;
  }

  return j;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */


