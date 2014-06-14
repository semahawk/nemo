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

#include "utf8.h"

/* masks to retrieve the value from the first byte in a character */
static uint8_t first_byte_masks[6] =
{
  /* 1 byte   0x0.......  0x01111111 */ 0x7f,
  /* 2 bytes  0x110.....  0x00011111 */ 0x1f,
  /* 3 bytes  0x1110....  0x00001111 */ 0x0f,
  /* 4 bytes  0x11110...  0x00000111 */ 0x07,
  /* 5 bytes  0x111110..  0x00000011 */ 0x03,
  /* 6 bytes  0x1111110.  0x00000001 */ 0x01
};

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
 * Return the number of bytes needed for a codepoint, given the character's
 * first byte.
 *
 */
static inline unsigned u8_nbytes(uint8_t first_byte)
{
  /* 0x0....... */
  if      ((first_byte & 0x80) == 0x00)
    return 1;

  /* 0x110..... */
  else if ((first_byte & 0xe0) == 0xc0)
    return 2;

  /* 0x1110.... */
  else if ((first_byte & 0xf0) == 0xe0)
    return 3;

  /* 0x11110... */
  else if ((first_byte & 0xf8) == 0xf0)
    return 4;

  /* 0x111110.. */
  else if ((first_byte & 0xfc) == 0xf8)
    return 5;

  /* 0x1111110. */
  else if ((first_byte & 0xfe) == 0xfc)
    return 6;

  else
    /* that's gonna go noticed */
    /* it's _probably_ not a good idea, but now I only have to remember that
     * when something wierd-ass happens, that's it */
    return -1;
}

/*
 * This function starts reading bytes, at position indicated by *p,
 * fetches the correct utf-8 codepoint, and returns it.
 *
 * The parameter <p> gets updated to point right after the character's last
 * byte.
 *
 */
nchar_t u8_fetch_char(char **p)
{
  nchar_t ch = 0;
  /* number of bytes in the character */
  unsigned n = u8_nbytes(**p);

  if (n == 1){
    /* only 1 byte */
    ch = **p;
    (*p)++;
  } else {
    /* more than 1 byte */
    ch = **p & first_byte_masks[n - 1];
    (*p)++;

    while ((**p & 0xc0) == 0x80){
      ch = (ch << 6) | (**p & 0x3f);
      (*p)++;
    }
  }

  return ch;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */


