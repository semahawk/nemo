/*
 *
 * null.c
 *
 * Created at:  Sat 18 May 2013 16:31:21 CEST 16:31:21
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

/*
 * "[Instrumental]"
 *
 *  Racer X - Technical Difficulties
 */

#include "nemo.h"

/*
 * Creating the "null" object
 */
Nob *null = &(Nob){
  .type = OT_NULL,
  .fn = {
    .print = nm_null_print,
    .binary = {
      .add = NULL,
      .index = NULL,
      .cmp = NULL
    },
    .unary = {
      .plus = NULL,
      .minus = NULL,
      .negate = NULL
    }
  }
};

Nob *nm_null_repr(void)
{
  return nm_new_str("null");
}

/*
 * @name - nm_null_print
 * @desc - print the "null"
 */
void nm_null_print(FILE *fp, Nob *ob)
{
  assert(ob->type == OT_NULL);

  fprintf(fp, "(null)");
}

