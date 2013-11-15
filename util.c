/*
 *
 * util.c
 *
 * Created at:  Fri 15 Nov 15:49:21 2013 15:49:21
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdio.h>
#include <stdlib.h>

/*
 * A small little function to print a byte in it's binary form (8 bits long).
 */
const char *itob(int num)
{
  static char str[9] = { 0 };
  int i;

  for (i = 7; i >= 0; i--){
    str[i] = (num & 1) ? '1' : '0';
    num >>= 1;
  }

  return str;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */


