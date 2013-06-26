/*
 *
 * stack.h
 *
 * Created at:  Mon 24 Jun 2013 18:04:07 CEST 18:04:07
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

#ifndef STACK_H
#define STACK_H

#include "nemo.h"

#include <math.h>

/* the initial size of any created stack */
#define INITIAL_STACK_SIZE 10
/* the number by which an nearly-overflowed stack will grow */
#define STACK_GROW_RATIO 1.5

/*
 * This is Nemo's basic stack structure.
 *
 * Creating:
 *
 *     stack_new(name, type);
 *
 *   This would create a new stack, which values would be of the given type.
 *   For instance:
 *
 *     stack_new(arg, NmObject *);
 *
 *   Note: this stack and it's belongings are declared as "static" (functions
 *         are also "inline")!
 *
 * Pushing:
 *
 *     stack_push(name, value);
 *
 *   This would push a given value to the stack named ##name_stack, and the
 *   stack's internal "pointer" would be incremented.
 *   If the stack's size isn't sufficent, it would be grown
 *   STACK_GROW_RATIO times.
 *
 * Popping:
 *
 *      stack_pop(name);
 *
 *   This would return the top-most value from the stack and decrement the
 *   stack's internal "pointer".
 *
 */

/* creates a set of required functions for the stack to operate */
#define stack_init(_name, _type) \
  static inline void _name##_stack_push(_type value){ \
    if (_name##_stack.ptr >= _name##_stack.nmemb){ \
      if (_name##_stack.nmemb == 0){ \
        _name##_stack.nmemb = INITIAL_STACK_SIZE; \
      } else { \
        _name##_stack.nmemb = floor(_name##_stack.nmemb * STACK_GROW_RATIO); \
      } \
      _name##_stack.it = NmMem_Realloc(_name##_stack.it, sizeof(_type) * _name##_stack.nmemb); \
    } \
\
    _name##_stack.it[_name##_stack.ptr++] = value; \
  } \
\
  static inline _type _name##_stack_pop(void){ \
    if (_name##_stack.ptr <= 0){ \
      NmError_Error("popping from the '" #_name "' stack, but it's empty!"); \
      NmError_Error("returning zero"); \
      return (_type)0; \
    } \
\
    return _name##_stack.it[--_name##_stack.ptr]; \
  } \
\
  static inline BOOL _name##_stack_is_empty(void){ \
    return _name##_stack.ptr < 0 ? TRUE : FALSE; \
  }

/* defines the stack */
#define stack_new(_name, _type) \
  static struct { \
    /* the stack itself */ \
    _type *it; \
    /* number of members */ \
    int nmemb; \
    /* "pointer" to the current member */ \
    int ptr; \
  } _name##_stack = { \
    NULL, 0, 0 \
  }; \
\
  stack_init(_name, _type)

#define stack_push(_name, _value) \
  _name##_stack_push((_value));

#endif /* STACK_H */

