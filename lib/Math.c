/*
 *
 * Math.c
 *
 * Created at:  Sat 25 May 2013 09:50:28 CEST 09:50:28
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

#include <math.h>
#include <nemo.h>

static NmObject *Math_sin(NmObject *args)
{
  return NmFloat_New(sin(NmFloat_VAL(NmArray_GETELEM(args, 0))));
}

static NmObject *Math_sqrt(NmObject *args)
{
  return NmFloat_New(sqrt(NmInt_VAL(NmArray_GETELEM(args, 0))));
}

static NmModuleFuncs Math_funcs[] =
{
  { "sin",  Math_sin,  1, { OT_FLOAT }, "" },
  { "sqrt", Math_sqrt, 1, { OT_INTEGER }, "" },
  { NULL, NULL, 0, { 0 }, NULL }
};

void Math_init(void)
{
  Nm_InitModule(Math_funcs);
}

