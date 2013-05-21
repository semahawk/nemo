/*
 *
 * error.h
 *
 * Created at:  Sun Apr  7 12:17:06 2013 12:17:06
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License: the MIT license
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

#ifndef ERROR_H
#define ERROR_H

#include <stdarg.h>

#include "lexer.h"

/* not yet used, but I don't want to rewrite it later */
#if 0
enum ErrorType {
  NOTE,
  WARNING,
  ERROR
};

struct Error {
  enum ErrorType type;
  char *message;
  /* singly linked list */
  struct Error *next;
};

struct ErrorState {
  /* wheater one of the errors would cause the program to not be able to
   * continue execution/compilation;
   * yeah, errors would cause it */
  BOOL should_exit;
  /* pointer to the first error on the list */
  struct Error *errors;
};

typedef enum   ErrorType  ErrorType;
typedef struct Error      Error;
typedef struct ErrorState ErrorState;
#endif

void NmError_Fatal(const char *msg, ...);
void NmError_Error(const char *msg, ...);
void NmError_Lex(LexerState *, const char *msg, ...);
void NmError_Parser(Node *, const char *msg, ...);

#endif /* ERROR_H */

