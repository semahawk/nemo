/*
 *
 * utf8.h
 *
 * Created at:  Mon Nov 11 19:15:57 2013 19:15:57
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef UTF8_H
#define UTF8_H

#include <stdint.h>

/* wchar_t's width seems quite varying */
typedef uint32_t nchar_t;

unsigned u8_strlen(char *string);
nchar_t u8_fetch_char(char **ptr);

#endif /* UTF8_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

