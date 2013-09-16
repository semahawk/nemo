/*
 *
 * vars.c
 *
 * Created at:  Sat 01 Jun 2013 18:08:13 CEST 18:08:13
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

#include "nemo.h"

/*
 * Creates a new variable of name <name> and value <value> in the current namespace.
 *
 * Note: One have to set flag manually for it (using nm_var_set_flag)
 *
 * Returns the variable that was created
 */
Variable *nm_new_var(char *name, Nob *value)
{
  Namespace *namespace = nm_curr_namespace();
  VariablesList *vars_list = nmalloc(sizeof(VariablesList));
  Variable *var = nmalloc(sizeof(Variable));

  /* add the newly created variable to the global namespace */
  var->name = nm_strdup(name);
  var->value = value;
  vars_list->var = var;
  vars_list->next = namespace->globals;
  namespace->globals = vars_list;

  return var;
}

