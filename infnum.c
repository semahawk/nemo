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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "infnum.h"
#include "mem.h"
#include "util.h"

/*
 * Nemo's (lousy) implementation of arbitrary precision numbers.
 *
 * Some parts were taken from here[1], which makes it actualy a bit less lousy.
 *
 * [1] http://en.literateprograms.org/Arbitrary-precision_integer_arithmetic_(C)
 */

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

struct infnum infnum_raw(unsigned nmemb)
{
  struct infnum num;
  unsigned i;

  num.nmemb = nmemb;
  num.digits = nmalloc(sizeof(infnum_digit_t) * nmemb);
  num.sign = INFNUM_SIGN_POS;

  /* zero-out, d'uh */
  for (i = 0; i < nmemb; i++)
    num.digits[i] = 0;

  return num;
}

bool infnum_is_zero(struct infnum num)
{
  unsigned i;

  for (i = 0; i < num.nmemb; i++)
    if (num.digits[i] != 0)
      return false;

  return true;
}

struct infnum infnum_from_byte(uint8_t v)
{
  struct infnum ret;

  ret.digits = nmalloc(sizeof(infnum_digit_t) * 1);
  ret.digits[0] = (infnum_digit_t)v;
  ret.nmemb = 1;
  ret.sign = INFNUM_SIGN_POS;

  return ret;
}

struct infnum infnum_from_word(uint16_t v)
{
  struct infnum ret;

  ret.digits = nmalloc(sizeof(infnum_digit_t) * 1);
  ret.digits[0] = (infnum_digit_t)v;
  ret.nmemb = 1;
  ret.sign = INFNUM_SIGN_POS;

  return ret;
}

struct infnum infnum_from_dword(uint32_t v)
{
  struct infnum ret;

  ret.digits = nmalloc(sizeof(infnum_digit_t) * 1);
  ret.digits[0] = (infnum_digit_t)v;
  ret.nmemb = 1;
  ret.sign = INFNUM_SIGN_POS;

  return ret;
}

struct infnum infnum_from_qword(uint64_t v)
{
  struct infnum ret;

  ret.digits = nmalloc(sizeof(infnum_digit_t) * 2);
  ret.digits[0] = (INFNUM_MAX_DIGIT_VALUE & v);
  ret.digits[1] = (((infnum_double_digit_t)INFNUM_MAX_DIGIT_VALUE << INFNUM_DIGIT_BITS) & v) >> INFNUM_DIGIT_BITS;
  ret.nmemb = 2;
  ret.sign = INFNUM_SIGN_POS;

  return ret;
}

struct infnum infnum_from_str(char *s)
{
  struct infnum num;
  struct infnum digit = infnum_from_word(1);
  unsigned i = 0;
  bool negate = false;

  assert(s);

  num = infnum_raw((int)ceil(LOG_2_10 * strlen(s) /
        INFNUM_DIGIT_BITS));
  num.sign = INFNUM_SIGN_POS;

  if (s[0] == '-'){
    i++;
    negate = true;
  }

  for (; s[i] != '\0'; i++){
    infnum_mul_by_small_inline(num, 10, num);
    digit.digits[0] = s[i] - '0';
    infnum_add_inline(num, digit, num);
  }

  if (negate)
    num.sign = INFNUM_SIGN_NEG;

  free_infnum(digit);

  return num;
}

void infnum_print_hex(struct infnum num, FILE *fp)
{
  int i = num.nmemb - 1;

  if (num.sign == INFNUM_SIGN_NEG && !infnum_is_zero(num))
    fprintf(fp, "-");

  fprintf(fp, "0x");

  /* skip over the leading zeroes */
  while (num.digits[i] == 0 && i > 0)
    i--;

  /* the 'digits' are stored in reverse order */
  for (; i >= 0; i--)
    if (num.digits[i] == 0)
      fprintf(fp, "%08x", num.digits[i]);
    else
      fprintf(fp, "%x", num.digits[i]);
}

void infnum_print(struct infnum num, FILE *fp)
{
  /* TODO make it more accurate (ideally to a one digit precision) */
  int up = (int)(LOG_10_2 * INFNUM_DIGIT_BITS * num.nmemb + 1);
  int i, len;
  char *buff = nmalloc(sizeof(char) * up);
  struct infnum copy = infnum_copy(num);
  struct infnum ten  = infnum_from_dword(10);

  if (infnum_is_zero(num)){
    buff[0]= '0';
    buff[1]= '\0';
  } else {
    for (i = 0; !infnum_is_zero(copy); i++){
      buff[i] = (char)infnum_mod_by_small(copy, 10) + '0';
      infnum_div_by_small_inline(copy, 10, copy);
    }

    buff[i] = '\0';
  }

  len = strlen(buff);
  /* reverse the string */
  for (i = 0; i < len / 2; i++){
    char tmp = buff[i];

    buff[i] = buff[len - i - 1];
    buff[len - i - 1] = tmp;
  }

  fprintf(fp, "%s", buff);

  free(buff);
  free_infnum(ten);
}

struct infnum infnum_copy(struct infnum num)
{
  struct infnum new;

  new.sign   = num.sign;
  new.nmemb  = num.nmemb;
  new.digits = nmalloc(sizeof(infnum_digit_t) * new.nmemb);

  /* copy the digits */
  for (unsigned i = 0; i < new.nmemb; i++)
    new.digits[i] = num.digits[i];

  return new;
}

uint8_t infnum_to_byte(struct infnum num)
{
  if (num.nmemb >= 1)
    return 0xff & num.digits[0];
  else
    /* this probably should be handled better */
    return 0;
}

uint16_t infnum_to_word(struct infnum num)
{
  if (num.nmemb >= 1)
    return 0xffff & num.digits[0];
  else
    /* this probably should be handled better */
    return 0;
}

uint32_t infnum_to_dword(struct infnum num)
{
  if (num.nmemb >= 1)
    return num.digits[0];
  else
    /* this probably should be handled better */
    return 0;
}

uint64_t infnum_to_qword(struct infnum num)
{
  if (num.nmemb >= 2)
    return ((infnum_double_digit_t)num.digits[1] << INFNUM_DIGIT_BITS) | num.digits[0];
  else if (num.nmemb == 1)
    return (infnum_digit_t)num.digits[0];
  else
    /* this probably should be handled better */
    return 0;
}

enum infnum_cmp infnum_cmp(struct infnum a, struct infnum b)
{
  /* {{{ */
  int i = MAX(a.nmemb - 1, b.nmemb - 1);

  for (; i >= 0; i--){
#define digit(n) ((((unsigned)i) < (n).nmemb) ? (n).digits[i] : 0)

    if (digit(a) < digit(b))
      return INFNUM_CMP_LT;
    else if (digit(a) > digit(b))
      return INFNUM_CMP_GT;

#undef digit
  }

  return INFNUM_CMP_EQ;
  /* }}} */
}

struct infnum infnum_add(struct infnum a, struct infnum b)
{
  /* {{{ */
  struct infnum sum;
  infnum_double_digit_t carry;
  unsigned i;

  sum.nmemb = MAX(a.nmemb, b.nmemb) + 2;
  sum.digits = nmalloc(sizeof(infnum_digit_t) * sum.nmemb);

  if (a.sign == b.sign) sum.sign = a.sign;
  else {
    if (a.sign == INFNUM_SIGN_NEG){
      /* -a + b becomes b - a */
      a.sign = INFNUM_SIGN_POS;
      sum = infnum_sub(b, a);
      return sum;
    } else {
      /* a + (-b) becomes a - b */
      b.sign = INFNUM_SIGN_POS;
      sum = infnum_sub(a, b);
      return sum;
    }
  }

  for (carry = 0, i = 0; i < sum.nmemb; i++){
#define digit(n) (((i) >= (n).nmemb) ? 0 : (n).digits[i])

    sum.digits[i] = (infnum_digit_t)(digit(a) + digit(b) + carry);
    carry = ((infnum_double_digit_t)digit(a) + digit(b)) >> (INFNUM_DIGIT_BITS);

#undef digit
  }

  return sum;
  /* }}} */
}

void infnum_add_inline(struct infnum a, struct infnum b, struct infnum result)
{
  /* {{{ */
  infnum_double_digit_t carry;
  unsigned i;

  if (a.sign == b.sign) result.sign = a.sign;
  else {
    if (a.sign == INFNUM_SIGN_NEG){
      /* -a + b becomes b - a */
      a.sign = INFNUM_SIGN_POS;
      infnum_sub_inline(b, a, result);
      return;
    } else {
      /* a + (-b) becomes a - b */
      b.sign = INFNUM_SIGN_POS;
      infnum_sub_inline(a, b, result);
      return;
    }
  }

  for (carry = 0, i = 0; i < result.nmemb; i++){
#define digit(n) (((i) >= (n).nmemb) ? 0 : (n).digits[i])

    result.digits[i] = (infnum_digit_t)(digit(a) + digit(b) + carry);
    carry = ((infnum_double_digit_t)digit(a) + digit(b)) >> (INFNUM_DIGIT_BITS);

#undef digit
  }

  return;
  /* }}} */
}

struct infnum infnum_sub(struct infnum a, struct infnum b)
{
  /* {{{ */
  struct infnum diff;
  unsigned i;
  bool borrow;

  if (a.sign == b.sign) diff.sign = a.sign;
  else {
    if (a.sign == INFNUM_SIGN_NEG){
      /* -a - b becomes -(a + b) */
      a.sign = INFNUM_SIGN_POS;
      diff = infnum_add(a, b);
      diff.sign = INFNUM_SIGN_NEG;
      return diff;
    } else {
      /* a - (-b) becomes a + b */
      b.sign = INFNUM_SIGN_POS;
      diff = infnum_add(a, b);
      diff.sign = INFNUM_SIGN_POS;
      return diff;
    }
  }

  /* switch the values in place if necessary, so that <a> is the bigger one */
  if (infnum_cmp(a, b) == INFNUM_CMP_LT){
    /* small - big becomes -(big - small) */
    struct infnum temp = a;

    a = b; b = temp;
    diff.sign = INFNUM_SIGN_NEG;
  }

  diff.nmemb = MAX(a.nmemb, b.nmemb);
  diff.digits = nmalloc(sizeof(infnum_digit_t) * diff.nmemb);

  for (borrow = false, i = 0; i < a.nmemb; i++){
    infnum_double_digit_t lhs = a.digits[i];
    infnum_double_digit_t rhs = i < b.nmemb ? b.digits[i] : 0;

    if (borrow){
      if (lhs <= rhs){
        /* leave borrow set */
        lhs += (INFNUM_MAX_DIGIT_VALUE + 1) - 1;
      } else {
        borrow = false;
        lhs--;
      }
    }

    if (lhs < rhs){
      borrow = true;
      lhs += INFNUM_MAX_DIGIT_VALUE + 1;
    }

    diff.digits[i] = lhs - rhs;
  }

  for (; i < diff.nmemb; i++)
    diff.digits[i] = 0;

  return diff;
  /* }}} */
}

void infnum_sub_inline(struct infnum a, struct infnum b, struct infnum result)
{
  /* {{{ */
  unsigned i;
  bool borrow;

  if (a.sign == b.sign) result.sign = a.sign;
  else {
    if (a.sign == INFNUM_SIGN_NEG){
      /* -a - b becomes -(a + b) */
      a.sign = INFNUM_SIGN_POS;
      infnum_add_inline(a, b, result);
      result.sign = INFNUM_SIGN_NEG;
      return;
    } else {
      /* a - (-b) becomes a + b */
      b.sign = INFNUM_SIGN_POS;
      infnum_add_inline(a, b, result);
      result.sign = INFNUM_SIGN_POS;
      return;
    }
  }

  /* switch the values in place if necessary, so that <a> is the bigger one */
  if (infnum_cmp(a, b) == INFNUM_CMP_LT){
    /* small - big = -(big - small) */
    struct infnum temp = a;

    a = b; b = temp;
    result.sign = INFNUM_SIGN_NEG;
  }

  for (borrow = false, i = 0; i < a.nmemb; i++){
    infnum_double_digit_t lhs = a.digits[i];
    infnum_double_digit_t rhs = i < b.nmemb ? b.digits[i] : 0;

    if (borrow){
      if (lhs <= rhs){
        /* leave borrow set */
        lhs += (INFNUM_MAX_DIGIT_VALUE + 1) - 1;
      } else {
        borrow = false;
        lhs--;
      }
    }

    if (lhs < rhs){
      borrow = true;
      lhs += INFNUM_MAX_DIGIT_VALUE + 1;
    }

    result.digits[i] = lhs - rhs;
  }

  for (; i < result.nmemb; i++)
    result.digits[i] = 0;

  /* }}} */
}

struct infnum infnum_mul(struct infnum a, struct infnum b)
{
  /* {{{ */
  struct infnum prod;
  int i, lidx, ridx;
  infnum_double_digit_t carry = 0;
  int max_size_no_carry;
  int a_max_digit = a.nmemb - 1;
  int b_max_digit = b.nmemb - 1;

  while (a.digits[a_max_digit] == 0) a_max_digit--;
  while (b.digits[b_max_digit] == 0) b_max_digit--;

  max_size_no_carry = a_max_digit + b_max_digit;

  prod.nmemb = a.nmemb + b.nmemb;
  prod.digits = nmalloc(sizeof(infnum_digit_t) * prod.nmemb);

  if (a.sign == b.sign) prod.sign = INFNUM_SIGN_POS;
  else                  prod.sign = INFNUM_SIGN_NEG;

  for (i = 0; i <= max_size_no_carry || carry != 0; i++){
    infnum_double_digit_t partial_sum = carry;
    carry = 0;
    lidx = MIN(i, a_max_digit);
    ridx = i - lidx;

    while (lidx >= 0 && ridx <= b_max_digit){
      partial_sum += ((infnum_double_digit_t)a.digits[lidx]) * b.digits[ridx];
      carry += partial_sum >> INFNUM_DIGIT_BITS;
      partial_sum &= INFNUM_MAX_DIGIT_VALUE;

      lidx--;
      ridx++;
    }

    prod.digits[i] = partial_sum;
  }

  for (; (unsigned)i < prod.nmemb; i++)
    prod.digits[i] = 0;

  return prod;
  /* }}} */
}

void infnum_mul_inline(struct infnum a, struct infnum b, struct infnum result)
{
  /* {{{ */
  int i, lidx, ridx;
  infnum_double_digit_t carry = 0;
  int max_size_no_carry;
  int a_max_digit = a.nmemb - 1;
  int b_max_digit = b.nmemb - 1;

  while (a.digits[a_max_digit] == 0) a_max_digit--;
  while (b.digits[b_max_digit] == 0) b_max_digit--;

  max_size_no_carry = a_max_digit + b_max_digit;

  if (a.sign == b.sign) result.sign = INFNUM_SIGN_POS;
  else                  result.sign = INFNUM_SIGN_NEG;

  for (i = 0; i <= max_size_no_carry || carry != 0; i++){
    infnum_double_digit_t partial_sum = carry;
    carry = 0;
    lidx = MIN(i, a_max_digit);
    ridx = i - lidx;

    while (lidx >= 0 && ridx <= b_max_digit){
      partial_sum += ((infnum_double_digit_t)a.digits[lidx]) * b.digits[ridx];
      carry += partial_sum >> INFNUM_DIGIT_BITS;
      partial_sum &= INFNUM_MAX_DIGIT_VALUE;

      lidx--;
      ridx++;
    }

    result.digits[i] = partial_sum;
  }

  for (; (unsigned)i < result.nmemb; i++)
    result.digits[i] = 0;

  /* }}} */
}

struct infnum infnum_mul_by_small(struct infnum a, infnum_digit_t b)
{
  /* {{{ */
  struct infnum prod;
  unsigned i;
  infnum_double_digit_t carry;

  prod.nmemb = a.nmemb + 1; /* hmm.. */
  prod.digits = nmalloc(sizeof(infnum_digit_t) * prod.nmemb);

  /* hmm, a probable FIXME */
  prod.sign = INFNUM_SIGN_POS;

  for (carry = 0, i = 0; i < prod.nmemb; i++){
#define digit(n) (((i) >= (n).nmemb) ? 0 : (n).digits[i])

    prod.digits[i] = digit(a) * b + carry;
    carry = ((infnum_double_digit_t)digit(a) + b) >> INFNUM_DIGIT_BITS;

#undef digit
  }

  return prod;
  /* }}} */
}

void infnum_mul_by_small_inline(struct infnum a, infnum_digit_t b, struct infnum result)
{
  /* {{{ */
  infnum_double_digit_t carry = 0;
  unsigned i;

  /* hmm, a probable FIXME */
  result.sign = INFNUM_SIGN_POS;

  for (i = 0; i < a.nmemb || carry != 0; i++){
    infnum_double_digit_t partial_sum = carry;
    carry = 0;

    if (i < a.nmemb) partial_sum += (infnum_double_digit_t)a.digits[i] * b;

    carry = partial_sum >> INFNUM_DIGIT_BITS;
    result.digits[i] = (infnum_digit_t)(partial_sum & INFNUM_MAX_DIGIT_VALUE);
  }

  for (; i < result.nmemb; i++)
    result.digits[i] = 0;

  /* }}} */
}

struct infnum infnum_div_by_small(struct infnum a, infnum_digit_t b)
{
  /* {{{ */
  struct infnum quot;
  int i;
  infnum_double_digit_t dividend = 0;

  quot.nmemb = a.nmemb + 1; /* hmm.. */
  quot.digits = nmalloc(sizeof(infnum_digit_t) * quot.nmemb);
  quot.sign = INFNUM_SIGN_POS; /* FIXME */

  for (i = (signed)a.nmemb - 1; i >= 0; i--){
    dividend |= a.digits[i];
    quot.digits[i] = dividend / b;
    dividend = (dividend % b) << INFNUM_DIGIT_BITS;
  }

  return quot;
  /* }}} */
}

void infnum_div_by_small_inline(struct infnum a, infnum_digit_t b, struct infnum res)
{
  /* {{{ */
  infnum_double_digit_t dividend = 0;
  int i;

  for (i = (signed)a.nmemb - 1; i >= 0; i--){
    dividend |= a.digits[i];
    res.digits[i] = dividend / b;
    dividend = (dividend % b) << INFNUM_DIGIT_BITS;
  }
  /* }}} */
}

struct infnum infnum_mod(struct infnum a, struct infnum b)
{
  /* {{{ */
  struct infnum rem = infnum_raw(b.nmemb);
  struct infnum twos_power = infnum_raw(b.nmemb + 1);
  unsigned i, bit;

  twos_power.digits[0] = 1;

  for (i = 0; i < a.nmemb; i++){
    for (bit = 0; bit < INFNUM_DIGIT_BITS; bit++){
      if ((a.digits[i] & (1 << bit)) != 0){
        infnum_add_inline(rem, twos_power, rem);

        if (infnum_cmp(rem, b) & (INFNUM_CMP_GT | INFNUM_CMP_EQ)){
          infnum_sub_inline(rem, b, rem);
        }
      }

      infnum_shl_by_one_inline(twos_power);

      if (infnum_cmp(twos_power, b) & (INFNUM_CMP_GT | INFNUM_CMP_EQ)){
        infnum_sub_inline(twos_power, b, twos_power);
      }
    }
  }

  free_infnum(twos_power);

  return rem;
  /* }}} */
}

infnum_digit_t infnum_mod_by_small(struct infnum a, infnum_digit_t b)
{
  /* {{{ */
  infnum_double_digit_t twos_power = 1;
  infnum_double_digit_t rem = 0;
  unsigned i, bit;

  for (i = 0; i < a.nmemb; i++){
    for (bit = 0; bit < INFNUM_DIGIT_BITS; bit++){
      if ((a.digits[i] & (1 << bit)) != 0){
        rem += twos_power;

        if (rem >= b){
          rem -= b;
        }
      }

      twos_power <<= 1;

      if (twos_power >= b){
        twos_power -= b;
      }
    }
  }

  return (infnum_digit_t)rem;
  /* }}} */
}

struct infnum infnum_shl_by_small(struct infnum a, infnum_digit_t b)
{
  /* {{{ */
  struct infnum res;
  unsigned i;
  infnum_double_digit_t mask;

  if (b == 0)
    return a;

  /* calculate the number of digits the result will have to have */
  res = infnum_raw(a.nmemb + (floor((b - 1) / INFNUM_DIGIT_BITS) + 1));
  res.sign = a.sign;

  for (i = 0; i < a.nmemb; i++){
    /* res_digit holds where the low 'half' of a digit <n> will end up (ie.
     * in which digit of the resulting number); the high 'half' will end up in
     * res_digit + 1 */
    unsigned res_digit_idx = floor(b / INFNUM_DIGIT_BITS) + i;

    mask = (infnum_double_digit_t)a.digits[i] << (b % INFNUM_DIGIT_BITS);
    res.digits[res_digit_idx] |= mask & INFNUM_MAX_DIGIT_VALUE;
    res.digits[res_digit_idx + 1] |= mask >> INFNUM_DIGIT_BITS;
  }

  return res;
  /* }}} */
}

void infnum_shl_by_one_inline(struct infnum a)
{
  /* {{{ */
  int i;

  /* TODO make room for the MSB */
  a.digits[a.nmemb - 1] <<= 1;

  for (i = (signed)a.nmemb - 2; i >= 0; i--){
    a.digits[i + 1] |= a.digits[i] >> (INFNUM_DIGIT_BITS - 1);
    a.digits[i] <<= 1;
  }
  /* }}} */
}

struct infnum infnum_shr_by_small(struct infnum a, infnum_digit_t b)
{
  /* {{{ */
  struct infnum res;
  unsigned i;
  infnum_double_digit_t mask;

  if (b == 0)
    return a;

  /* calculate the number of digits the result will have to have */
  res = infnum_raw(a.nmemb - floor(b / INFNUM_DIGIT_BITS));
  res.sign = a.sign;

  for (i = 0; i < res.nmemb; i++){
    /*
     * <a_digit_idx> holds where in a.digits is (part of) the digit that will
     * end up in res.digits[i]
     */
    unsigned a_digit_idx = floor(b / INFNUM_DIGIT_BITS) + i;
    infnum_double_digit_t mask_lowbits = a.digits[a_digit_idx];
    /* high_a_digit is the digit right next to a.digits[a_digit_idx] unless
     * there would be overflow, in which case it's simply zero */
    infnum_double_digit_t mask_highbits = ((a_digit_idx + 1) < a.nmemb) ? a.digits[a_digit_idx + 1] : 0;

    /* the mask consists of ORed a.digits[a_digit_idx] and the digit right next
     * to it (or zero) */
    mask = (mask_lowbits | (mask_highbits << INFNUM_DIGIT_BITS)) >> (b % INFNUM_DIGIT_BITS);
    res.digits[i] |= mask & INFNUM_MAX_DIGIT_VALUE;
  }

  return res;
  /* }}} */
}

struct infnum infnum_xor(struct infnum a, struct infnum b)
{
  /* {{{ */
  struct infnum res;
  unsigned i;

  res = infnum_raw(MAX(a.nmemb, b.nmemb));
  res.sign = INFNUM_SIGN_POS; /* possible FIXME */

  for (i = 0; i < MAX(a.nmemb, b.nmemb); i++){
#define digit(n) (((i) < (n).nmemb) ? (n).digits[i] : 0)

    res.digits[i] = digit(a) ^ digit(b);

#undef digit
  }

  return res;
  /* }}} */
}

struct infnum infnum_or(struct infnum a, struct infnum b)
{
  /* {{{ */
  struct infnum res;
  unsigned i;

  res = infnum_raw(MAX(a.nmemb, b.nmemb));
  res.sign = INFNUM_SIGN_POS; /* possible FIXME */

  for (i = 0; i < MAX(a.nmemb, b.nmemb); i++){
#define digit(n) (((i) < (n).nmemb) ? (n).digits[i] : 0)

    res.digits[i] = digit(a) | digit(b);

#undef digit
  }

  return res;
  /* }}} */
}

void free_infnum(struct infnum num)
{
  nfree(num.digits);
}

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

