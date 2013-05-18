/*
 *
 * builtin.c
 *
 * Created at:  Fri 17 May 2013 22:26:42 CEST 22:26:42
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
 * "Can't you see
 *  That in the world we live in
 *  Political lies
 *  Are just corporate decisions
 *  They'll take away
 *  All the hopes, not their promises
 *  They'll put an end to this
 *  Land of the livin"
 *
 *  Testament - Souls of Black
 */

#include <stdio.h>

#include "nemo.h"

static NmObject *builtin_print(Params *params)
{
  (void)params;
  printf("BULITIN PRINT CALLED\n");

  return NmObject_NewFromInt(1);
}

static NmModuleFuncs module_funcs[] = {
  { "print", builtin_print },
  { NULL, NULL }
};

void NmBuiltin_Init(Context *ctx)
{
  Nm_InsertFuncs(ctx, module_funcs);
}

