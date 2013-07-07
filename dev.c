/*
 *
 * dev.c
 *
 * Created at:  Sat 06 Jul 2013 10:21:12 CEST 10:21:12
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
 * Library only for developing/testing purposes.
 */

#include "nemo.h"

static NmObject *dev_retarr(NmObject *args, bool *opts)
{
  NmObject *ret = NmArray_New(3);

  NmArray_SETELEM(ret, 0, NmInt_New(NmInt_VAL(NmArray_GETELEM(args, 0))));
  NmArray_SETELEM(ret, 1, NmInt_New(7));
  NmArray_SETELEM(ret, 2, NmInt_New(1));

  return ret;
}

static NmObject *dev_retstr(NmObject *args, bool *opts)
{
  /* unused parameter */
  (void)args;

  return NmString_New("Hello");
}

static NmObject *dev_optfun(NmObject *args, bool *opts)
{
  /* unused parameter */
  (void)args;

  return NmString_New("optfun called.");
}

static NmModuleFuncs module_funcs[] =
{
  { "retarr", dev_retarr, 1, { OT_INTEGER }, "" },
  { "retstr", dev_retstr, 0, { OT_NULL }, "" },
  { "optfun", dev_optfun, 0, { OT_NULL }, "t" },
  { NULL, NULL, 0, { 0 }, NULL }
};

void dev_init(void)
{
  Nm_InitModule(module_funcs);
}

