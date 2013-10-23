/*
 *
 * object.c
 *
 * Created at:  Sat 11 May 2013 20:27:13 CEST 20:27:13
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
 * "False sense of pride, satisfies
 *  There's no reason for suicide
 *  Use your mind, and hope to find
 *  Find the meaning of existence..."
 *
 *  Testament - Sins of Omission
 */

#include "nemo.h"

BinaryFunc nm_obj_has_binary_func(Nob *ob, BinaryOp op)
{
  switch (ob->type){
    case OT_INTEGER:
      switch (op){
        case BINARY_ADD:
          return nm_int_add;
        case BINARY_SUB:
          return nm_int_sub;
        case BINARY_MUL:
          return nm_int_mul;
        case BINARY_DIV:
          return nm_int_div;
        case BINARY_MOD:
          return nm_int_mod;
        default:
          return NULL;
      }
      break;
    case OT_FLOAT:
      switch (op){
        case BINARY_ADD:
          return nm_float_add;
        case BINARY_SUB:
          return nm_float_sub;
        case BINARY_MUL:
          return nm_float_mul;
        case BINARY_DIV:
          return nm_float_div;
        default:
          return NULL;
      }
      break;
    case OT_STRING:
      switch (op){
        case BINARY_ADD:
          return nm_str_add;
        case BINARY_INDEX:
          return nm_str_index;
        default:
          return NULL;
      }
      break;
    case OT_ARRAY:
      switch (op){
        case BINARY_ADD:
          return nm_arr_add;
        case BINARY_INDEX:
          return nm_arr_index;
        default:
          return NULL;
      }
      break;
    case OT_FILE:
      switch (op){
        default:
          return NULL;
      }
      break;
    case OT_NULL:
      switch (op){
        default:
          return NULL;
      }
      break;
    case OT_ANY:
      /* should never get here */
      return NULL;
    default:
      nm_error("unknown object type at %s line %d", __FILE__, __LINE__);
      nexit();
      break;
  }

  return NULL;
}

UnaryFunc nm_obj_has_unary_func(Nob *ob, UnaryOp op)
{
  assert(ob);

  switch (ob->type){
    case OT_INTEGER:
      switch (op){
        case UNARY_PLUS:
          return nm_int_plus;
        case UNARY_MINUS:
          return nm_int_minus;
        case UNARY_NEGATE:
          return nm_int_negate;
        case UNARY_PREINC:  /* fall-through */
        case UNARY_POSTINC:
          return nm_int_incr;
        case UNARY_PREDEC:  /* fall-through */
        case UNARY_POSTDEC:
          return nm_int_decr;
        default:
          return NULL;
      }
      break;
    case OT_FLOAT:
      switch (op){
        case UNARY_PREINC:  /* fall-through */
        case UNARY_POSTINC:
          return nm_float_incr;
        case UNARY_PREDEC:  /* fall-through */
        case UNARY_POSTDEC:
          return nm_float_decr;
        default:
          return NULL;
      }
      break;
    case OT_STRING:
      return NULL;
    case OT_ARRAY:
      return NULL;
    case OT_FILE:
      return NULL;
    case OT_NULL:
      return NULL;
    case OT_ANY:
      return NULL;
  }

  return NULL;
}

CmpFunc nm_obj_has_cmp_func(Nob *ob, BinaryOp op)
{
  /* unused paremeter */
  (void)op;

  assert(ob);

  switch (ob->type){
    case OT_INTEGER:
      return nm_int_cmp;
    case OT_FLOAT:
      return nm_float_cmp;
    case OT_STRING:
      return nm_str_cmp;
    case OT_ARRAY:
      return NULL;
    case OT_FILE:
      return NULL;
    case OT_NULL:
      return NULL;
    case OT_ANY:
      /* should never get here */
      return NULL;
  }

  return NULL;
}

void nm_obj_destroy(Nob *ob)
{
  assert(ob);

  switch (ob->type){
    case OT_INTEGER:
      nm_int_destroy(ob);
      break;
    case OT_FLOAT:
      nm_float_destroy(ob);
      break;
    case OT_STRING:
      nm_str_destroy(ob);
      break;
    case OT_ARRAY:
      nm_arr_destroy(ob);
      break;
    case OT_FILE:
      nm_file_destroy(ob);
      break;
    case OT_NULL:
      /* well, null is defined statically, so there's no need to free it */
      /* heck, it wasn't even malloced */
    case OT_ANY:
      /* that's strange and shouldn't happen */
      break;
    default:
      nm_error("unknown object type at %s line %d", __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      break;
  }
}

/*
 * Duplicates given object and returns the copy.
 */
Nob *nm_obj_dup(Nob *ob)
{
  Nob *new = nmalloc(sizeof(*ob));

  switch (ob->type){
    case OT_INTEGER:
      *(Niob *)new = *(Niob *)ob;
      break;
    case OT_FLOAT:
      *(Nfob *)new = *(Nfob *)ob;
      break;
    case OT_STRING:
      *(Nsob *)new = *(Nsob *)ob;
      break;
    case OT_ARRAY:
      *(Naob *)new = *(Naob *)ob;
      break;
    case OT_NULL:
      *(Nnob *)new = *(Nnob *)ob;
      break;
    case OT_FILE:
      *(Nfhob *)new = *(Nfhob *)ob;
      break;
    case OT_ANY:
      /* suspress the warnings */
      break;
  }

  gc_push(new);

  return (Nob *)new;
}

/*
 * Translates given <type> to a string.
 */
Nob *nm_obj_typetos(Nob *ob)
{
  assert(ob);

  Nob *ret = nm_new_str("");
  bool first = true;

  unsigned num = 7;
  for (unsigned i = 0; i < 7; i++)
    if (ob->type & (1 << i))
      num++;

  unsigned j = 0;
  for (unsigned i = 0; i < 7; i++){
    if (ob->type & (1 << i)){
      if (!first)
        ret = nm_str_add(ret, nm_new_str(" or "));
      first = false;
      switch (ob->type & (1 << i)){
        case OT_NULL:
          ret = nm_str_add(ret, nm_new_str("null"));
          j++;
          break;
        case OT_INTEGER:
          ret = nm_str_add(ret, nm_new_str("integer"));
          j++;
          break;
        case OT_FLOAT:
          ret = nm_str_add(ret, nm_new_str("float"));
          j++;
          break;
        case OT_STRING:
          ret = nm_str_add(ret, nm_new_str("string"));
          j++;
          break;
        case OT_ARRAY:
          ret = nm_str_add(ret, nm_new_str("array"));
          j++;
          break;
        case OT_FILE:
          ret = nm_str_add(ret, nm_new_str("file handle"));
          j++;
          break;
      }
    }
  }

  return ret;
}

/*
 * Check if given value is a true/false boolean-wise.
 *
 * In Nemo there is no "bool" type as is.
 */
bool nm_obj_boolish(Nob *ob)
{
  /*
   * null, 0, 0.0, empty string ("") and an empty array ([]) are false
   * everything else is true
   */
  switch (ob->type){
    case OT_NULL:
      return false;
    case OT_INTEGER:
      if (nm_int_value(ob) == 0)
        return false;
      else
        return true;
    case OT_FLOAT:
      if (nm_float_value(ob) == 0.0f)
        return false;
      else
        return true;
    case OT_STRING:
      if (!strcmp(nm_str_value(ob), ""))
        return false;
      else
        return true;
    case OT_ARRAY:
      if (nm_arr_nmemb(ob) == 0)
        return false;
      else
        return true;
    case OT_FILE:
      return true;
    default:
      return false;
  }
}

