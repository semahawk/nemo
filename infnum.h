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

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

#include "nemo.h"

#define INFNUM_SIGN_POS 1
#define INFNUM_SIGN_NEG 0

typedef uint32_t infnum_digit_t;
typedef uint64_t infnum_double_digit_t;

/* the number of bits in a single 'digit' */
#define INFNUM_DIGIT_BITS (sizeof(infnum_digit_t) * CHAR_BIT)
/* the number of bits in a single double 'digit' */
#define INFNUM_DOUBLE_DIGIT_BITS (sizeof(infnum_double_digit_t) * CHAR_BIT)

/* maximum value for a single 'digit' */
#define INFNUM_MAX_DIGIT_VALUE ((infnum_digit_t)(-1))

/* a one precomputed constant */
#define LOG_2_10 3.3219280948873623478703194294894

struct infnum {
  /* stored in reverse order (least significant 'digit' first) */
  infnum_digit_t *digits;
  /* number of 'digits' (a buttload tops, apparently) */
  uint64_t nmemb: 63;
  unsigned char sign: 1;
};

enum infnum_cmp {
  INFNUM_CMP_LT = 1,
  INFNUM_CMP_EQ = 2,
  INFNUM_CMP_GT = 4
};

struct infnum infnum_raw(unsigned nmemb);
struct infnum infnum_from_str(char *s);
struct infnum infnum_from_byte(uint8_t);
struct infnum infnum_from_word(uint16_t);
struct infnum infnum_from_dword(uint32_t);
struct infnum infnum_from_qword(uint64_t);

uint8_t  infnum_to_byte (struct infnum);
uint16_t infnum_to_word (struct infnum);
uint32_t infnum_to_dword(struct infnum);
uint64_t infnum_to_qword(struct infnum);

void infnum_print(struct infnum, FILE *);
enum infnum_cmp infnum_cmp(struct infnum, struct infnum);
bool infnum_is_zero(struct infnum);

struct infnum infnum_add(struct infnum, struct infnum);
struct infnum infnum_sub(struct infnum, struct infnum);
struct infnum infnum_mul(struct infnum, struct infnum);
struct infnum infnum_mul_by_small(struct infnum, infnum_digit_t);

void infnum_add_inline(struct infnum, struct infnum, struct infnum);
void infnum_sub_inline(struct infnum, struct infnum, struct infnum);
void infnum_mul_inline(struct infnum, struct infnum, struct infnum);
void infnum_mul_by_small_inline(struct infnum, infnum_digit_t, struct infnum);

void free_infnum(struct infnum);

#endif /* INFNUM_H */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

