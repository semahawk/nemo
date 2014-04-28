/*
 *
 * infnum.c
 *
 * Created at:  Wed 23 Apr 08:27:21 2014 08:27:21
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License:  please visit the LICENSE file for details.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "infnum.h"
#include "mem.h"
#include "util.h"

/*
 * Nemo's implementation of arbitrary precision numbers.
 */

#define MAX(a, b) ((a) > (b) ? (a) : (b))

struct infnum infnum_from_str(char *s)
{
  struct infnum num;

  /* see if the first digit is a sign 'digit' */
  if (*s == '+' || *s == '-'){
    if (*s == '+')
      num.sign = INFNUM_SIGN_POS;
    else if (*s == '-')
      num.sign = INFNUM_SIGN_NEG;
    s++;
  } else {
    /* positive is the default */
    num.sign = INFNUM_SIGN_POS;
  }

  num.digits = nmalloc(/* sizeof(char) times */ strlen(s) + 2);
  num.digits++;
  strncpy(num.digits, s, strlen(s));

  return num;
}

struct infnum infnum_from_int(int v)
{
  struct infnum num;
  unsigned digits_count; /* number of digits, d'uh */
  int temp, i;

  if (v >= 0) num.sign = INFNUM_SIGN_POS;
  else        num.sign = INFNUM_SIGN_NEG;

  v = abs(v);

  /* calculate the number of digits in <i> */
  for (temp = v, digits_count = 0; temp > 0; temp /= 10, digits_count++)
    ;

  num.digits = nmalloc(/* sizeof(char) times */ digits_count + 2);
  /* leave a space for the sign bit (this space is actually only used by
   * `infnum_to_str') */
  num.digits++;

  for (i = digits_count - 1, temp = v; temp > 0 && i >= 0; temp /= 10, i--)
    num.digits[i] = temp % 10 + '0';

  num.digits[digits_count] = '\0';

  return num;
}

char *infnum_to_str(struct infnum num)
{
  if (num.sign == INFNUM_SIGN_POS)
    return num.digits;
  else {
    /* ugly (but very handy) hack */
    *(num.digits - 1) = '-';
    return num.digits - 1;
  }
}

int64_t infnum_to_qword(struct infnum num)
{
  return 0xdeadcafe;
}

enum infnum_cmp infnum_cmp(struct infnum a, struct infnum b)
{
  size_t lenof_a, lenof_b;

  if (a.sign == INFNUM_SIGN_POS && b.sign == INFNUM_SIGN_NEG) return INFNUM_CMP_GT;
  if (a.sign == INFNUM_SIGN_NEG && b.sign == INFNUM_SIGN_POS) return INFNUM_CMP_LT;

  lenof_a = strlen(a.digits);
  lenof_b = strlen(b.digits);

  if (lenof_a > lenof_b) return INFNUM_CMP_GT; else
  if (lenof_a < lenof_b) return INFNUM_CMP_LT;
  else {
    size_t i;

    for (i = 0; i < lenof_a; i++)
      if (a.digits[i] != b.digits[i])
        return INFNUM_CMP_NE;

    return INFNUM_CMP_EQ;
  }
}

struct infnum infnum_add(struct infnum a, struct infnum b)
{
  struct infnum sum;
  size_t lenof_a = strlen(a.digits);
  size_t lenof_b = strlen(b.digits);
  char *ap, *bp;
  unsigned i;
  int carry;
  size_t max = MAX(lenof_a, lenof_b);

  if (a.sign == b.sign) sum.sign = a.sign;
  else {
    if (a.sign == INFNUM_SIGN_NEG){
      /* -a + b = b - a */
      a.sign = INFNUM_SIGN_POS;
      sum = infnum_sub(b, a);
      return sum;
    } else {
      /* a + (-b) = a - b */
      b.sign = INFNUM_SIGN_POS;
      sum = infnum_sub(a, b);
      return sum;
    }
  }

  /* assume there is always going to be 'overflow' (hence +2) */
  /* additional +1 (for the sign just before the actual number) */
  sum.digits = nmalloc(/* sizeof(char) times */ max + 3);
  sum.digits++;

  for (carry = 0, i = 0; i < max + 2; i++){
    /* current `a' character */
    char ac = i >= lenof_a ? 0 : a.digits[lenof_a - i - 1] - '0';
    /* current `b' character */
    char bc = i >= lenof_b ? 0 : b.digits[lenof_b - i - 1] - '0';

    sum.digits[/* max + 2 - i - 1 */ max - i + 1] = ((carry + ac + bc) % 10) + '0';
    carry = (carry + ac + bc) / 10;
  }

  /* nul-terminate */
  sum.digits[i] = '\0';

  return sum;
}

struct infnum infnum_sub(struct infnum a, struct infnum b)
{
  struct infnum diff;
  size_t lenof_a = strlen(a.digits);
  size_t lenof_b = strlen(b.digits);
  char *ap, *bp;
  unsigned i;
  size_t max = MAX(lenof_a, lenof_b);

  if (a.sign == b.sign) diff.sign = a.sign;
  else {
    if (a.sign == INFNUM_SIGN_NEG){
      /* -a - b = -(a + b) */
      a.sign = INFNUM_SIGN_POS;
      diff = infnum_add(b, a);
      diff.sign = INFNUM_SIGN_NEG;
      return diff;
    } else {
      /* a - (-b) = a + b */
      b.sign = INFNUM_SIGN_POS;
      diff = infnum_add(a, b);
      return diff;
    }
  }

  /* switch the numbers (if necessary) to always subtract the smaller from the bigger */
  if (infnum_cmp(b, a) == INFNUM_CMP_GT){
    /* small - big = -(big - small) */
    struct infnum temp = a;
    size_t lentemp = lenof_a;

    a = b; b = temp;
    lenof_a = lenof_b; lenof_b = lentemp;
    diff.sign = INFNUM_SIGN_NEG;
  }

  /* +2 (for nul and for the sign just before the actual number */
  diff.digits = nmalloc(/* sizeof(char) times */ lenof_a + 2);
  diff.digits++;

  strcpy(diff.digits, a.digits);

  for (i = 0; i < lenof_a; i++){
    /* current `diff' character */
    char dc = i >= lenof_a ? 0 : diff.digits[lenof_a - i - 1] - '0';
    /* current `b' character */
    char bc = i >= lenof_b ? 0 :    b.digits[lenof_b - i - 1] - '0';

    if (dc >= bc){
      diff.digits[lenof_a - i - 1] = dc - bc + '0';
    } else {
      diff.digits[lenof_a - i - 2] -= 1;
      diff.digits[lenof_a - i - 1] += 10 - bc;
    }
  }

  /* nul-terminate */
  diff.digits[i] = '\0';

  return diff;
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

