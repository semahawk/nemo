/*
 *
 * infnum.h
 *
 * Created at:  Wed 23 Apr 08:27:36 2014 08:27:36
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#ifndef INFNUM_H
#define INFNUM_H

#include "nemo.h"

#define INFNUM_SIGN_POS 1
#define INFNUM_SIGN_NEG 0

struct infnum {
  char *digits;
  unsigned char sign;
};

enum infnum_cmp {
  INFNUM_CMP_EQ,
  INFNUM_CMP_NE,
  INFNUM_CMP_LT,
  INFNUM_CMP_LE,
  INFNUM_CMP_GT,
  INFNUM_CMP_GE
};

struct infnum infnum_from_str(char *s);
struct infnum infnum_from_int(int);
char *infnum_to_str(struct infnum);
int64_t infnum_to_qword(struct infnum);
enum infnum_cmp infnum_cmp(struct infnum, struct infnum);

struct infnum infnum_add(struct infnum, struct infnum);
struct infnum infnum_sub(struct infnum, struct infnum);

#endif /* INFNUM_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

