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
#include <stdint.h>

/*
 * A small little function to print a byte in it's binary form (4 bits long).
 */
const char *itob4(int num)
{
  static char str[5] = { 0 };
  int i;

  for (i = 3; i >= 0; i--){
    str[i] = (num & 1) ? '1' : '0';
    num >>= 1;
  }

  return str;
}

/*
 * A small little function to print a byte in it's binary form (8 bits long).
 */
const char *itob8(int num)
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
 * A small little function to print a byte in it's binary form (64 bits long).
 */
const char *itob64(int64_t num)
{
  static char str[65] = { 0 };
  int i;

  for (i = 63; i >= 0; i--){
    str[i] = (num & 1) ? '1' : '0';
    num >>= 1;
  }

  return str;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */


