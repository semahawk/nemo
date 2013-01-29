/*
 * exec.c
 *
 * Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
 *
 */

#include "nemo.h"
#include "handy.h"
#include "exec.h"
#include "nodes.h"
#include "vars.h"
#include "cast.h"
#include "predef.h"
#include "userdef.h"

Value(*nodeExecs[])(struct Node *) =
{
  execID,
  execConstant,
  execConstant,
  execConstant,
  execBinExpression,
  execUnExpression,
  execAssignment,
  execBlock,
  execStatement,
  execCall,
  execReturn,
  execWhile,
  execIf,
  execFor,
  execFuncDef,
  execIter,
  execNoop
};

Value dispatchNode(struct Node *n)
{
  assert(n);
  assert(nodeExecs[n->kind]);

  return nodeExecs[n->kind](n);
}

Value execNodes(struct Node *nodest)
{
  Value ret;

  ret.v.i = vtoi(execBlock(nodest));
  // we probably don't even need that but screw
  ret.type = TYPE_INTEGER;

  return ret;
}

Value execID(struct Node *n)
{
  assert(n);
  assert(nt_ID == n->kind);

  debug("exec", "id node <name: %s> at %p", n->data.s, n);

  if (variableAlreadySet(n->data.s, n->block)){
    return getVariableValue(n->data.s, n->block);
  } else {
    cerror("variable '%s' was not found", n->data.s);
    exit(1);
  }
}

Value execConstant(struct Node *n)
{
  assert(n);
  assert(nt_INTEGER  == n->kind ||
         nt_FLOATING == n->kind ||
         nt_STRING   == n->kind);

  debug("exec", "constant node <val: %d> at %p", vtoi(n->data.value), n);

  return n->data.value;
}

Value execBinExpression(struct Node *n)
{
  assert(n);
  assert(nt_BINARYOP == n->kind);

  const Value left = dispatchNode(n->data.binaryop.left);
  const Value right = dispatchNode(n->data.binaryop.right);
  Value ret;

  // we're reallocing it later
  char  *new_str = myalloc(1);
  size_t new_size;

  // used with BINARY_STR_* operators
  int str_flag;
  int str_flag2;
  char *str_left;
  char *str_right;
  size_t longer;

  debug("exec", "binary operation node <op: '%s'> at %p", binarytos(n->data.binaryop.op), n);

  switch (left.type)
  {
    // INTEGER {{{
    case TYPE_INTEGER:
      switch (right.type)
      {
        // INTEGER and INTEGER {{{
        case TYPE_INTEGER:
          switch (n->data.binaryop.op)
          {
            // XXX int + int
            case BINARY_ADD:
              ret.v.i = left.v.i + right.v.i;
              ret.type = TYPE_INTEGER;
              break;
            // XXX int - int
            case BINARY_SUB:
              ret.v.i = left.v.i - right.v.i;
              ret.type = TYPE_INTEGER;
              break;
            // XXX int * int
            case BINARY_MUL:
              ret.v.i = left.v.i * right.v.i;
              ret.type = TYPE_INTEGER;
              break;
            // XXX int / int
            case BINARY_DIV:
              if (right.v.i == 0){
                cerror("zero division!");
                exit(1);
              }
              ret.v.f = vtof(left) / vtof(right);
              ret.type = TYPE_FLOATING;
              break;
            // XXX int % int
            case BINARY_MOD:
              ret.v.i = vtoi(left) % vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX int > int
            case BINARY_GT:
              ret.v.i = left.v.i > right.v.i;
              ret.type = TYPE_INTEGER;
              break;
            // XXX int < int
            case BINARY_LT:
              ret.v.i = left.v.i < right.v.i;
              ret.type = TYPE_INTEGER;
              break;
            // XXX int >= int
            case BINARY_GE:
              ret.v.i = left.v.i >= right.v.i;
              ret.type = TYPE_INTEGER;
              break;
            // XXX int <= int
            case BINARY_LE:
              ret.v.i = left.v.i <= right.v.i;
              ret.type = TYPE_INTEGER;
              break;
            // XXX int != int
            case BINARY_NE:
              ret.v.i = left.v.i != right.v.i;
              ret.type = TYPE_INTEGER;
              break;
            // XXX int == int
            case BINARY_EQ:
              ret.v.i = left.v.i == right.v.i;
              ret.type = TYPE_INTEGER;
              break;
            // XXX int += int
            case BINARY_EQ_ADD:
              ret.v.i = left.v.i + right.v.i;
              ret.type = TYPE_INTEGER;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX int -= int
            case BINARY_EQ_SUB:
              ret.v.i = left.v.i - right.v.i;
              ret.type = TYPE_INTEGER;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX int *= int
            case BINARY_EQ_MUL:
              ret.v.i = left.v.i * right.v.i;
              ret.type = TYPE_INTEGER;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX int /= int
            case BINARY_EQ_DIV:
              if (right.v.i == 0){
                cerror("zero division!");
                exit(1);
              }
              ret.v.f = vtof(left) / vtof(right);
              ret.type = TYPE_FLOATING;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX int %= int
            case BINARY_EQ_MOD:
              ret.v.i = vtoi(left) % vtoi(right);
              ret.type = TYPE_INTEGER;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX int . int
            case BINARY_CON:
              new_size = strlen(vtos(left)) + strlen(vtos(right)) + 1;
              myrealloc(new_str, new_size);
              snprintf(new_str, new_size, "%d%d", left.v.i, right.v.i);
              ret.v.s = new_str;
              ret.type = TYPE_STRING;
              break;
            // XXX int eq int
            case BINARY_STR_EQ:
              str_flag = 1;
              str_left = vtos(left);
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag = 0;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX int ne int
            case BINARY_STR_NE:
              str_flag = 0;
              str_left = vtos(left);
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX int gt int
            case BINARY_STR_GT:
              str_flag = 0;
              str_left = vtos(left);
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] > str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX int lt int
            case BINARY_STR_LT:
              str_flag = 0;
              str_left = vtos(left);
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] < str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX int ge int
            case BINARY_STR_GE:
              str_flag = 0;
              str_flag2 = 1;
              str_left = vtos(left);
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] > str_right[i]){
                  str_flag = 1;
                  break;
                }
              }

              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag2 = 0;
                  break;
                }
              }

              if (str_flag || str_flag2)
                ret.v.i = 1;
              else
                ret.v.i = 0;

              ret.type = TYPE_INTEGER;
              break;
            // XXX int le int
            case BINARY_STR_LE:
              str_flag = 0;
              str_flag2 = 1;
              str_left = vtos(left);
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] < str_right[i]){
                  str_flag = 1;
                  break;
                }
              }

              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag2 = 0;
                  break;
                }
              }

              if (str_flag || str_flag2)
                ret.v.i = 1;
              else
                ret.v.i = 0;

              ret.type = TYPE_INTEGER;
              break;
          }
          break;
        // }}}
        // INTEGER and FLOATING {{{
        case TYPE_FLOATING:
          switch (n->data.binaryop.op)
          {
            // XXX int + float
            case BINARY_ADD:
              ret.v.f = left.v.i + right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX int - float
            case BINARY_SUB:
              ret.v.f = left.v.i - right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX int * float
            case BINARY_MUL:
              ret.v.f = left.v.i * right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX int / float
            case BINARY_DIV:
              if (right.v.f == 0){
                cerror("zero division!");
                exit(1);
              }
              ret.v.f = vtof(left) / right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX int % float
            case BINARY_MOD:
              ret.v.i = left.v.i % vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX int > float
            case BINARY_GT:
              ret.v.f = left.v.i > right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX int < float
            case BINARY_LT:
              ret.v.f = left.v.i < right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX int >= float
            case BINARY_GE:
              ret.v.f = left.v.i >= right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX int <= float
            case BINARY_LE:
              ret.v.f = left.v.i <= right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX int != float
            case BINARY_NE:
              ret.v.f = left.v.i != right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX int == float
            case BINARY_EQ:
              ret.v.f = left.v.i == right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX int += float
            case BINARY_EQ_ADD:
              ret.v.f = left.v.i + right.v.f;
              ret.type = TYPE_FLOATING;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX int -= float
            case BINARY_EQ_SUB:
              ret.v.f = left.v.i - right.v.f;
              ret.type = TYPE_FLOATING;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX int *= float
            case BINARY_EQ_MUL:
              ret.v.f = left.v.i * right.v.f;
              ret.type = TYPE_FLOATING;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX int /= float
            case BINARY_EQ_DIV:
              if (right.v.f == 0){
                cerror("zero division!");
                exit(1);
              }
              ret.v.f = vtof(left) / right.v.f;
              ret.type = TYPE_FLOATING;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX int %= float
            case BINARY_EQ_MOD:
              ret.v.i = left.v.i % vtoi(right);
              ret.type = TYPE_INTEGER;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX int . float
            case BINARY_CON:
              new_size = strlen(vtos(left)) + strlen(vtos(right)) + 1;
              myrealloc(new_str, new_size);
              snprintf(new_str, new_size, "%d%.2f", left.v.i, right.v.f);
              ret.v.s = new_str;
              ret.type = TYPE_STRING;
              break;
            // XXX int eq float
            case BINARY_STR_EQ:
              str_flag = 1;
              str_left = vtos(left);
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag = 0;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX int ne float
            case BINARY_STR_NE:
              str_flag = 0;
              str_left = vtos(left);
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX int gt float
            case BINARY_STR_GT:
              str_flag = 0;
              str_left = vtos(left);
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] > str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX int lt float
            case BINARY_STR_LT:
              str_flag = 0;
              str_left = vtos(left);
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] < str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX int ge float
            case BINARY_STR_GE:
              str_flag = 0;
              str_flag2 = 1;
              str_left = vtos(left);
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] > str_right[i]){
                  str_flag = 1;
                  break;
                }
              }

              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag2 = 0;
                  break;
                }
              }

              if (str_flag || str_flag2)
                ret.v.i = 1;
              else
                ret.v.i = 0;

              ret.type = TYPE_INTEGER;
              break;
            // XXX int le float
            case BINARY_STR_LE:
              str_flag = 0;
              str_flag2 = 1;
              str_left = vtos(left);
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] < str_right[i]){
                  str_flag = 1;
                  break;
                }
              }

              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag2 = 0;
                  break;
                }
              }

              if (str_flag || str_flag2)
                ret.v.i = 1;
              else
                ret.v.i = 0;

              ret.type = TYPE_INTEGER;
              break;
          }
          break;
        // }}}
        // INTEGER and STRING {{{
        case TYPE_STRING:
          switch (n->data.binaryop.op)
          {
            // XXX int + string
            case BINARY_ADD:
              ret.v.i = left.v.i + vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX int - string
            case BINARY_SUB:
              ret.v.i = left.v.i - vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX int * string
            case BINARY_MUL:
              ret.v.i = left.v.i * vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX int / string
            case BINARY_DIV:
              if (vtoi(right) == 0){
                cerror("zero division!");
                exit(1);
              }
              ret.v.f = vtof(left) / vtoi(right);
              ret.type = TYPE_FLOATING;
              break;
            // XXX int % string
            case BINARY_MOD:
              ret.v.i = left.v.i % vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX int > string
            case BINARY_GT:
              ret.v.i = left.v.i > vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX int < string
            case BINARY_LT:
              ret.v.i = left.v.i < vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX int >= string
            case BINARY_GE:
              ret.v.i = left.v.i >= vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX int <= string
            case BINARY_LE:
              ret.v.i = left.v.i <= vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX int != string
            case BINARY_NE:
              ret.v.i = left.v.i != vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX int == string
            case BINARY_EQ:
              ret.v.i = left.v.i == vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX int += string
            case BINARY_EQ_ADD:
              ret.v.i = left.v.i + vtoi(right);
              ret.type = TYPE_INTEGER;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX int -= string
            case BINARY_EQ_SUB:
              ret.v.i = left.v.i - vtoi(right);
              ret.type = TYPE_INTEGER;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX int *= string
            case BINARY_EQ_MUL:
              ret.v.i = left.v.i * vtoi(right);
              ret.type = TYPE_INTEGER;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX int /= string
            case BINARY_EQ_DIV:
              if (vtoi(right) == 0){
                cerror("zero division!");
                exit(1);
              }
              ret.v.f = vtof(left) / vtoi(right);
              ret.type = TYPE_FLOATING;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX int %= string
            case BINARY_EQ_MOD:
              ret.v.i = left.v.i % vtoi(right);
              ret.type = TYPE_INTEGER;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX int . string
            case BINARY_CON:
              new_size = strlen(vtos(left)) + strlen(right.v.s) + 1;
              myrealloc(new_str, new_size);
              snprintf(new_str, new_size, "%d%s", left.v.i, right.v.s);
              ret.v.s = new_str;
              ret.type = TYPE_STRING;
              break;
            // XXX int eq string
            case BINARY_STR_EQ:
              str_flag = 1;
              str_left = vtos(left);
              str_right = right.v.s;
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag = 0;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX int ne string
            case BINARY_STR_NE:
              str_flag = 0;
              str_left = vtos(left);
              str_right = right.v.s;
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX int gt string
            case BINARY_STR_GT:
              str_flag = 0;
              str_left = vtos(left);
              str_right = right.v.s;
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] > str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX int lt string
            case BINARY_STR_LT:
              str_flag = 0;
              str_left = vtos(left);
              str_right = right.v.s;
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] < str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX int ge string
            case BINARY_STR_GE:
              str_flag = 0;
              str_flag2 = 1;
              str_left = vtos(left);
              str_right = right.v.s;
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] > str_right[i]){
                  str_flag = 1;
                  break;
                }
              }

              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag2 = 0;
                  break;
                }
              }

              if (str_flag || str_flag2)
                ret.v.i = 1;
              else
                ret.v.i = 0;

              ret.type = TYPE_INTEGER;
              break;
            // XXX int le string
            case BINARY_STR_LE:
              str_flag = 0;
              str_flag2 = 1;
              str_left = vtos(left);
              str_right = right.v.s;
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] < str_right[i]){
                  str_flag = 1;
                  break;
                }
              }

              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag2 = 0;
                  break;
                }
              }

              if (str_flag || str_flag2)
                ret.v.i = 1;
              else
                ret.v.i = 0;

              ret.type = TYPE_INTEGER;
              break;
          }
          break;
      }
      break;
      // }}}
    // }}}
    // FLOATING {{{
    case TYPE_FLOATING:
      switch (right.type)
      {
        // FLOATING and INTEGER {{{
        case TYPE_INTEGER:
          switch (n->data.binaryop.op)
          {
            // XXX float + int
            case BINARY_ADD:
              ret.v.f = left.v.f + right.v.i;
              ret.type = TYPE_FLOATING;
              break;
            // XXX float - int
            case BINARY_SUB:
              ret.v.f = left.v.f - right.v.i;
              ret.type = TYPE_FLOATING;
              break;
            // XXX float * int
            case BINARY_MUL:
              ret.v.f = left.v.f * right.v.i;
              ret.type = TYPE_FLOATING;
              break;
            // XXX float / int
            case BINARY_DIV:
              if (right.v.i == 0){
                cerror("zero division!");
                exit(1);
              }
              ret.v.f = left.v.f / vtof(right);
              ret.type = TYPE_FLOATING;
              break;
            // XXX float % int
            case BINARY_MOD:
              ret.v.i = vtoi(left) % right.v.i;
              ret.type = TYPE_INTEGER;
              break;
            // XXX float > int
            case BINARY_GT:
              ret.v.f = left.v.f > right.v.i;
              ret.type = TYPE_FLOATING;
              break;
            // XXX float < int
            case BINARY_LT:
              ret.v.f = left.v.f < right.v.i;
              ret.type = TYPE_FLOATING;
              break;
            // XXX float >= int
            case BINARY_GE:
              ret.v.f = left.v.f >= right.v.i;
              ret.type = TYPE_FLOATING;
              break;
            // XXX float <= int
            case BINARY_LE:
              ret.v.f = left.v.f <= right.v.i;
              ret.type = TYPE_FLOATING;
              break;
            // XXX float != int
            case BINARY_NE:
              ret.v.f = left.v.f != right.v.i;
              ret.type = TYPE_FLOATING;
              break;
            // XXX float == int
            case BINARY_EQ:
              ret.v.f = left.v.f == right.v.i;
              ret.type = TYPE_FLOATING;
              break;
            // XXX float += int
            case BINARY_EQ_ADD:
              ret.v.f = left.v.f + right.v.i;
              ret.type = TYPE_FLOATING;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX float -= int
            case BINARY_EQ_SUB:
              ret.v.f = left.v.f - right.v.i;
              ret.type = TYPE_FLOATING;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX float *= int
            case BINARY_EQ_MUL:
              ret.v.f = left.v.f * right.v.i;
              ret.type = TYPE_FLOATING;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX float /= int
            case BINARY_EQ_DIV:
              if (right.v.i == 0){
                cerror("zero division!");
                exit(1);
              }
              ret.v.f = left.v.f / vtof(right);
              ret.type = TYPE_FLOATING;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX float %= int
            case BINARY_EQ_MOD:
              ret.v.i = vtoi(left) % right.v.i;
              ret.type = TYPE_INTEGER;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX float . int
            case BINARY_CON:
              new_size = strlen(vtos(left)) + strlen(vtos(right)) + 1;
              myrealloc(new_str, new_size);
              snprintf(new_str, new_size, "%.2f%d", left.v.f, right.v.i);
              ret.v.s = new_str;
              ret.type = TYPE_STRING;
              break;
            // XXX float eq int
            case BINARY_STR_EQ:
              str_flag = 1;
              str_left = vtos(left);
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag = 0;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX float ne int
            case BINARY_STR_NE:
              str_flag = 0;
              str_left = vtos(left);
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX float gt int
            case BINARY_STR_GT:
              str_flag = 0;
              str_left = vtos(left);
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] > str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX float lt int
            case BINARY_STR_LT:
              str_flag = 0;
              str_left = vtos(left);
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] < str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX float ge int
            case BINARY_STR_GE:
              str_flag = 0;
              str_flag2 = 1;
              str_left = vtos(left);
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] > str_right[i]){
                  str_flag = 1;
                  break;
                }
              }

              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag2 = 0;
                  break;
                }
              }

              if (str_flag || str_flag2)
                ret.v.i = 1;
              else
                ret.v.i = 0;

              ret.type = TYPE_INTEGER;
              break;
            // XXX float le int
            case BINARY_STR_LE:
              str_flag = 0;
              str_flag2 = 1;
              str_left = vtos(left);
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] < str_right[i]){
                  str_flag = 1;
                  break;
                }
              }

              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag2 = 0;
                  break;
                }
              }

              if (str_flag || str_flag2)
                ret.v.i = 1;
              else
                ret.v.i = 0;

              ret.type = TYPE_INTEGER;
              break;
          }
          break;
        // }}}
        // FLOATING and FLOATING {{{
        case TYPE_FLOATING:
          switch (n->data.binaryop.op)
          {
            // XXX float + float
            case BINARY_ADD:
              ret.v.f = left.v.f + right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX float - float
            case BINARY_SUB:
              ret.v.f = left.v.f - right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX float * float
            case BINARY_MUL:
              ret.v.f = left.v.f * right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX float / float
            case BINARY_DIV:
              if (right.v.f == 0){
                cerror("zero division!");
                exit(1);
              }
              ret.v.f = left.v.f / right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX float % float
            case BINARY_MOD:
              ret.v.i = vtoi(left) % vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX float > float
            case BINARY_GT:
              ret.v.f = left.v.f > right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX float < float
            case BINARY_LT:
              ret.v.f = left.v.f < right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX float >= float
            case BINARY_GE:
              ret.v.f = left.v.f >= right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX float <= float
            case BINARY_LE:
              ret.v.f = left.v.f <= right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX float != float
            case BINARY_NE:
              ret.v.f = left.v.f != right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX float == float
            case BINARY_EQ:
              ret.v.f = left.v.f == right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX float += float
            case BINARY_EQ_ADD:
              ret.v.f = left.v.f + right.v.f;
              ret.type = TYPE_FLOATING;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX float -= float
            case BINARY_EQ_SUB:
              ret.v.f = left.v.f - right.v.f;
              ret.type = TYPE_FLOATING;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX float *= float
            case BINARY_EQ_MUL:
              ret.v.f = left.v.f * right.v.f;
              ret.type = TYPE_FLOATING;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX float /= float
            case BINARY_EQ_DIV:
              if (right.v.f == 0){
                cerror("zero division!");
                exit(1);
              }
              ret.v.f = left.v.f / right.v.f;
              ret.type = TYPE_FLOATING;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX float %= float
            case BINARY_EQ_MOD:
              ret.v.i = vtoi(left) % vtoi(right);
              ret.type = TYPE_INTEGER;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX float . float
            case BINARY_CON:
              new_size = strlen(vtos(left)) + strlen(vtos(right)) + 1;
              myrealloc(new_str, new_size);
              snprintf(new_str, new_size, "%.2f%.2f", left.v.f, right.v.f);
              ret.v.s = new_str;
              ret.type = TYPE_STRING;
              break;
            // XXX float eq float
            case BINARY_STR_EQ:
              str_flag = 1;
              str_left = vtos(left);
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag = 0;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX float ne float
            case BINARY_STR_NE:
              str_flag = 0;
              str_left = vtos(left);
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX float gt float
            case BINARY_STR_GT:
              str_flag = 0;
              str_left = vtos(left);
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] > str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX float lt float
            case BINARY_STR_LT:
              str_flag = 0;
              str_left = vtos(left);
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] < str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX float ge float
            case BINARY_STR_GE:
              str_flag = 0;
              str_flag2 = 1;
              str_left = vtos(left);
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] > str_right[i]){
                  str_flag = 1;
                  break;
                }
              }

              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag2 = 0;
                  break;
                }
              }

              if (str_flag || str_flag2)
                ret.v.i = 1;
              else
                ret.v.i = 0;

              ret.type = TYPE_INTEGER;
              break;
            // XXX float le float
            case BINARY_STR_LE:
              str_flag = 0;
              str_flag2 = 1;
              str_left = vtos(left);
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] < str_right[i]){
                  str_flag = 1;
                  break;
                }
              }

              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag2 = 0;
                  break;
                }
              }

              if (str_flag || str_flag2)
                ret.v.i = 1;
              else
                ret.v.i = 0;

              ret.type = TYPE_INTEGER;
              break;
          }
          break;
        // }}}
        // FLOATING and STRING {{{
        case TYPE_STRING:
          switch (n->data.binaryop.op)
          {
            // XXX float + string
            case BINARY_ADD:
              ret.v.f = left.v.f + vtoi(right);
              ret.type = TYPE_FLOATING;
              break;
            // XXX float - string
            case BINARY_SUB:
              ret.v.f = left.v.f - vtoi(right);
              ret.type = TYPE_FLOATING;
              break;
            // XXX float * string
            case BINARY_MUL:
              ret.v.f = left.v.f * vtoi(right);
              ret.type = TYPE_FLOATING;
              break;
            // XXX float / string
            case BINARY_DIV:
              if (vtoi(right) == 0){
                cerror("zero division!");
                exit(1);
              }
              ret.v.f = left.v.f / vtoi(right);
              ret.type = TYPE_FLOATING;
              break;
            // XXX float % string
            case BINARY_MOD:
              ret.v.i = vtoi(left) % vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX float > string
            case BINARY_GT:
              ret.v.f = left.v.f > vtoi(right);
              ret.type = TYPE_FLOATING;
              break;
            // XXX float < string
            case BINARY_LT:
              ret.v.f = left.v.f < vtoi(right);
              ret.type = TYPE_FLOATING;
              break;
            // XXX float >= string
            case BINARY_GE:
              ret.v.f = left.v.f >= vtoi(right);
              ret.type = TYPE_FLOATING;
              break;
            // XXX float <= string
            case BINARY_LE:
              ret.v.f = left.v.f <= vtoi(right);
              ret.type = TYPE_FLOATING;
              break;
            // XXX float != string
            case BINARY_NE:
              ret.v.f = left.v.f != vtoi(right);
              ret.type = TYPE_FLOATING;
              break;
            // XXX float == string
            case BINARY_EQ:
              ret.v.f = left.v.f == vtoi(right);
              ret.type = TYPE_FLOATING;
              break;
            // XXX float += string
            case BINARY_EQ_ADD:
              ret.v.f = left.v.f + vtoi(right);
              ret.type = TYPE_FLOATING;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX float -= string
            case BINARY_EQ_SUB:
              ret.v.f = left.v.f - vtoi(right);
              ret.type = TYPE_FLOATING;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX float *= string
            case BINARY_EQ_MUL:
              ret.v.f = left.v.f * vtoi(right);
              ret.type = TYPE_FLOATING;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX float /= string
            case BINARY_EQ_DIV:
              if (vtoi(right) == 0){
                cerror("zero division!");
                exit(1);
              }
              ret.v.f = left.v.f / vtoi(right);
              ret.type = TYPE_FLOATING;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX float %= string
            case BINARY_EQ_MOD:
              ret.v.i = vtoi(left) % vtoi(right);
              ret.type = TYPE_INTEGER;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX float . string
            case BINARY_CON:
              new_size = strlen(vtos(left)) + strlen(right.v.s) + 1;
              myrealloc(new_str, new_size);
              snprintf(new_str, new_size, "%.2f%s", left.v.f, right.v.s);
              ret.v.s = new_str;
              ret.type = TYPE_STRING;
              break;
            // XXX float eq string
            case BINARY_STR_EQ:
              str_flag = 1;
              str_left = vtos(left);
              str_right = right.v.s;
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag = 0;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX float ne string
            case BINARY_STR_NE:
              str_flag = 0;
              str_left = vtos(left);
              str_right = right.v.s;
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX float gt string
            case BINARY_STR_GT:
              str_flag = 0;
              str_left = vtos(left);
              str_right = right.v.s;
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] > str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX float lt string
            case BINARY_STR_LT:
              str_flag = 0;
              str_left = vtos(left);
              str_right = right.v.s;
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] < str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX float ge string
            case BINARY_STR_GE:
              str_flag = 0;
              str_flag2 = 1;
              str_left = vtos(left);
              str_right = right.v.s;
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] > str_right[i]){
                  str_flag = 1;
                  break;
                }
              }

              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag2 = 0;
                  break;
                }
              }

              if (str_flag || str_flag2)
                ret.v.i = 1;
              else
                ret.v.i = 0;

              ret.type = TYPE_INTEGER;
              break;
            // XXX float le string
            case BINARY_STR_LE:
              str_flag = 0;
              str_flag2 = 1;
              str_left = left.v.s;
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] < str_right[i]){
                  str_flag = 1;
                  break;
                }
              }

              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag2 = 0;
                  break;
                }
              }

              if (str_flag || str_flag2)
                ret.v.i = 1;
              else
                ret.v.i = 0;

              ret.type = TYPE_INTEGER;
              break;
          }
          break;
          // }}}
      }
      break;
    // }}}
    // STRING {{{
    case TYPE_STRING:
      switch (right.type)
      {
        // STRING and INTEGER {{{
        case TYPE_INTEGER:
          switch (n->data.binaryop.op)
          {
            // XXX string + int
            case BINARY_ADD:
              ret.v.i = vtoi(left) + right.v.i;
              ret.type = TYPE_INTEGER;
              break;
            // XXX string - int
            case BINARY_SUB:
              ret.v.i = vtoi(left) - right.v.i;
              ret.type = TYPE_INTEGER;
              break;
            // XXX string * int
            case BINARY_MUL:
              ret.v.i = vtoi(left) * right.v.i;
              ret.type = TYPE_INTEGER;
              break;
            // XXX string / int
            case BINARY_DIV:
              if (vtoi(right) == 0){
                cerror("zero division!");
                exit(1);
              }
              ret.v.f = vtoi(left) / vtof(right);
              ret.type = TYPE_FLOATING;
              break;
            // XXX string % int
            case BINARY_MOD:
              ret.v.i = vtoi(left) % right.v.i;
              ret.type = TYPE_INTEGER;
              break;
            // XXX string > int
            case BINARY_GT:
              ret.v.i = vtoi(left) > right.v.i;
              ret.type = TYPE_INTEGER;
              break;
            // XXX string < int
            case BINARY_LT:
              ret.v.i = vtoi(left) < right.v.i;
              ret.type = TYPE_INTEGER;
              break;
            // XXX string >= int
            case BINARY_GE:
              ret.v.i = vtoi(left) >= right.v.i;
              ret.type = TYPE_INTEGER;
              break;
            // XXX string <= int
            case BINARY_LE:
              ret.v.i = vtoi(left) <= right.v.i;
              ret.type = TYPE_INTEGER;
              break;
            // XXX string != int
            case BINARY_NE:
              ret.v.i = vtoi(left) != right.v.i;
              ret.type = TYPE_INTEGER;
              break;
            // XXX string == int
            case BINARY_EQ:
              ret.v.i = vtoi(left) == right.v.i;
              ret.type = TYPE_INTEGER;
              break;
            // XXX string += int
            case BINARY_EQ_ADD:
              ret.v.i = vtoi(left) + right.v.i;
              ret.type = TYPE_INTEGER;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX string -= int
            case BINARY_EQ_SUB:
              ret.v.i = vtoi(left) - right.v.i;
              ret.type = TYPE_INTEGER;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX string *= int
            case BINARY_EQ_MUL:
              ret.v.i = vtoi(left) * right.v.i;
              ret.type = TYPE_INTEGER;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX string /= int
            case BINARY_EQ_DIV:
              if (vtoi(right) == 0){
                cerror("zero division!");
                exit(1);
              }
              ret.v.i = vtoi(left) / vtof(right);
              ret.type = TYPE_INTEGER;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX string %= int
            case BINARY_EQ_MOD:
              ret.v.i = vtoi(left) % right.v.i;
              ret.type = TYPE_INTEGER;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX string . int
            case BINARY_CON:
              new_size = strlen(left.v.s) + strlen(vtos(right)) + 1;
              myrealloc(new_str, new_size);
              snprintf(new_str, new_size, "%s%d", left.v.s, right.v.i);
              ret.v.s = new_str;
              ret.type = TYPE_STRING;
              break;
            // XXX string eq int
            case BINARY_STR_EQ:
              str_flag = 1;
              str_left = left.v.s;
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag = 0;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX string ne int
            case BINARY_STR_NE:
              str_flag = 0;
              str_left = left.v.s;
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX string gt int
            case BINARY_STR_GT:
              str_flag = 0;
              str_left = left.v.s;
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] > str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX string lt int
            case BINARY_STR_LT:
              str_flag = 0;
              str_left = left.v.s;
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] < str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX string ge int
            case BINARY_STR_GE:
              str_flag = 0;
              str_flag2 = 1;
              str_left = left.v.s;
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] > str_right[i]){
                  str_flag = 1;
                  break;
                }
              }

              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag2 = 0;
                  break;
                }
              }

              if (str_flag || str_flag2)
                ret.v.i = 1;
              else
                ret.v.i = 0;

              ret.type = TYPE_INTEGER;
              break;
            // XXX string le int
            case BINARY_STR_LE:
              str_flag = 0;
              str_flag2 = 1;
              str_left = left.v.s;
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] < str_right[i]){
                  str_flag = 1;
                  break;
                }
              }

              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag2 = 0;
                  break;
                }
              }

              if (str_flag || str_flag2)
                ret.v.i = 1;
              else
                ret.v.i = 0;

              ret.type = TYPE_INTEGER;
              break;
          }
          break;
        // }}}
        // STRING and FLOATING {{{
        case TYPE_FLOATING:
          switch (n->data.binaryop.op)
          {
            // XXX string + float
            case BINARY_ADD:
              ret.v.f = vtoi(left) + right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX string - float
            case BINARY_SUB:
              ret.v.f = vtoi(left) - right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX string * float
            case BINARY_MUL:
              ret.v.f = vtoi(left) * right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX string / float
            case BINARY_DIV:
              if (right.v.f == 0){
                cerror("zero division!");
                exit(1);
              }
              ret.v.f = vtoi(left) / right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX string % float
            case BINARY_MOD:
              ret.v.i = vtoi(left) % vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX string > float
            case BINARY_GT:
              ret.v.f = vtoi(left) > right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX string < float
            case BINARY_LT:
              ret.v.f = vtoi(left) < right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX string >= float
            case BINARY_GE:
              ret.v.f = vtoi(left) >= right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX string <= float
            case BINARY_LE:
              ret.v.f = vtoi(left) <= right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX string != float
            case BINARY_NE:
              ret.v.f = vtoi(left) != right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX string == float
            case BINARY_EQ:
              ret.v.f = vtoi(left) == right.v.f;
              ret.type = TYPE_FLOATING;
              break;
            // XXX string += float
            case BINARY_EQ_ADD:
              ret.v.f = vtoi(left) + right.v.f;
              ret.type = TYPE_FLOATING;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX string -= float
            case BINARY_EQ_SUB:
              ret.v.f = vtoi(left) - right.v.f;
              ret.type = TYPE_FLOATING;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX string *= float
            case BINARY_EQ_MUL:
              ret.v.f = vtoi(left) * right.v.f;
              ret.type = TYPE_FLOATING;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX string /= float
            case BINARY_EQ_DIV:
              if (right.v.f == 0){
                cerror("zero division!");
                exit(1);
              }
              ret.v.f = vtoi(left) / right.v.f;
              ret.type = TYPE_FLOATING;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX string %= float
            case BINARY_EQ_MOD:
              ret.v.i = vtoi(left) % vtoi(right);
              ret.type = TYPE_INTEGER;
              setVariableValue(n->data.binaryop.left->data.s, ret, n->block);
              break;
            // XXX string . float
            case BINARY_CON:
              new_size = strlen(left.v.s) + strlen(vtos(right)) + 1;
              myrealloc(new_str, new_size);
              snprintf(new_str, new_size, "%s%.2f", left.v.s, right.v.f);
              ret.v.s = new_str;
              ret.type = TYPE_STRING;
              break;
            // XXX string eq float
            case BINARY_STR_EQ:
              str_flag = 1;
              str_left = left.v.s;
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag = 0;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX string ne float
            case BINARY_STR_NE:
              str_flag = 0;
              str_left = left.v.s;
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX string gt float
            case BINARY_STR_GT:
              str_flag = 0;
              str_left = left.v.s;
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] > str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX string lt float
            case BINARY_STR_LT:
              str_flag = 0;
              str_left = left.v.s;
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] < str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX string ge float
            case BINARY_STR_GE:
              str_flag = 0;
              str_flag2 = 1;
              str_left = left.v.s;
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] > str_right[i]){
                  str_flag = 1;
                  break;
                }
              }

              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag2 = 0;
                  break;
                }
              }

              if (str_flag || str_flag2)
                ret.v.i = 1;
              else
                ret.v.i = 0;

              ret.type = TYPE_INTEGER;
              break;
            // XXX string le float
            case BINARY_STR_LE:
              str_flag = 0;
              str_flag2 = 1;
              str_left = left.v.s;
              str_right = vtos(right);
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] < str_right[i]){
                  str_flag = 1;
                  break;
                }
              }

              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag2 = 0;
                  break;
                }
              }

              if (str_flag || str_flag2)
                ret.v.i = 1;
              else
                ret.v.i = 0;

              ret.type = TYPE_INTEGER;
              break;
          }
          break;
        // }}}
        // STRING and STRING {{{
        case TYPE_STRING:
          switch (n->data.binaryop.op)
          {
            // XXX string + string
            case BINARY_ADD:
              ret.v.i = vtoi(left) + vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX string - string
            case BINARY_SUB:
              ret.v.i = vtoi(left) - vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX string * string
            case BINARY_MUL:
              ret.v.i = vtoi(left) * vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX string / string
            case BINARY_DIV:
              if (vtoi(right) == 0){
                cerror("zero division!");
                exit(1);
              }
              ret.v.f = vtoi(left) / vtoi(right);
              ret.type = TYPE_FLOATING;
              break;
            // XXX string % string
            case BINARY_MOD:
              ret.v.i = vtoi(left) % vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX string > string
            case BINARY_GT:
              ret.v.i = vtoi(left) > vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX string < string
            case BINARY_LT:
              ret.v.i = vtoi(left) < vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX string >= string
            case BINARY_GE:
              ret.v.i = vtoi(left) >= vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX string <= string
            case BINARY_LE:
              ret.v.i = vtoi(left) <= vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX string != string
            case BINARY_NE:
              ret.v.i = vtoi(left) != vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX string == string
            case BINARY_EQ:
              ret.v.i = vtoi(left) == vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX string += string
            case BINARY_EQ_ADD:
              ret.v.i = vtoi(left) + vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX string -= string
            case BINARY_EQ_SUB:
              ret.v.i = vtoi(left) - vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX string *= string
            case BINARY_EQ_MUL:
              ret.v.i = vtoi(left) * vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX string /= string
            case BINARY_EQ_DIV:
              if (vtoi(right) == 0){
                cerror("zero division!");
                exit(1);
              }
              ret.v.f = vtoi(left) / vtoi(right);
              ret.type = TYPE_FLOATING;
              break;
            // XXX string %= string
            case BINARY_EQ_MOD:
              ret.v.i = vtoi(left) % vtoi(right);
              ret.type = TYPE_INTEGER;
              break;
            // XXX string . string
            case BINARY_CON:
              new_size = strlen(left.v.s) + strlen(right.v.s) + 1;
              myrealloc(new_str, new_size);
              snprintf(new_str, new_size, "%s%s", left.v.s, right.v.s);
              ret.v.s = new_str;
              ret.type = TYPE_STRING;
              break;
            // XXX string eq string
            case BINARY_STR_EQ:
              str_flag = 1;
              str_left = left.v.s;
              str_right = right.v.s;
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag = 0;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX string ne string
            case BINARY_STR_NE:
              str_flag = 0;
              str_left = left.v.s;
              str_right = right.v.s;
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX string gt string
            case BINARY_STR_GT:
              str_flag = 0;
              str_left = left.v.s;
              str_right = right.v.s;
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] > str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX string lt string
            case BINARY_STR_LT:
              str_flag = 0;
              str_left = left.v.s;
              str_right = right.v.s;
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] < str_right[i]){
                  str_flag = 1;
                  break;
                }
              }
              ret.v.i = str_flag;
              ret.type = TYPE_INTEGER;
              break;
            // XXX string ge string
            case BINARY_STR_GE:
              str_flag = 0;
              str_flag2 = 1;
              str_left = left.v.s;
              str_right = right.v.s;
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] > str_right[i]){
                  str_flag = 1;
                  break;
                }
              }

              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag2 = 0;
                  break;
                }
              }

              if (str_flag || str_flag2)
                ret.v.i = 1;
              else
                ret.v.i = 0;

              ret.type = TYPE_INTEGER;
              break;
            // XXX string le string
            case BINARY_STR_LE:
              str_flag = 0;
              str_flag2 = 1;
              str_left = left.v.s;
              str_right = right.v.s;
              longer = (strlen(str_left) > strlen(str_right)) ? strlen(str_left) : strlen(str_right);
              for (size_t i = 0; i < longer; i++){
                if (str_left[i] < str_right[i]){
                  str_flag = 1;
                  break;
                }
              }

              for (size_t i = 0; i < longer; i++){
                if (str_left[i] != str_right[i]){
                  str_flag2 = 0;
                  break;
                }
              }

              if (str_flag || str_flag2)
                ret.v.i = 1;
              else
                ret.v.i = 0;

              ret.type = TYPE_INTEGER;
              break;
          }
          break;
          // }}}
      }
      break;
      // }}}
  }

  return ret;
}

Value execUnExpression(struct Node *n)
{
  assert(n);
  assert(nt_UNARYOP == n->kind);

  debug("exec", "unary operation node <op: '%s'> at %p", unarytos(n->data.unaryop.op), n);

  Value currval;
  Value ret;

  // if that's a variable, get it's value
  if (n->data.unaryop.expression->kind == nt_ID)
    currval = getVariableValue(n->data.unaryop.expression->data.s, n->block);
  // otherwise it's just a constant (or any other expression)
  else
    currval = dispatchNode(n->data.unaryop.expression);

  switch (n->data.unaryop.op){
    case UNARY_POSTINC:
      switch (currval.type){
        case TYPE_INTEGER:
          ret.v.i = currval.v.i + 1;
          ret.type = currval.type;
          setVariableValue(n->data.unaryop.expression->data.s, ret, n->block);
          return currval;
        case TYPE_FLOATING:
          ret.v.f = currval.v.f + 1.0f;
          ret.type = currval.type;
          setVariableValue(n->data.unaryop.expression->data.s, ret, n->block);
          return currval;
        case TYPE_STRING:
          cerror("cannot change value of a string");
          exit(1);
      }
    case UNARY_POSTDEC:
      switch (currval.type){
        case TYPE_INTEGER:
          ret.v.i = currval.v.i - 1;
          ret.type = currval.type;
          setVariableValue(n->data.unaryop.expression->data.s, ret, n->block);
          return currval;
        case TYPE_FLOATING:
          ret.v.f = currval.v.f - 1.0f;
          ret.type = currval.type;
          setVariableValue(n->data.unaryop.expression->data.s, ret, n->block);
          return currval;
        case TYPE_STRING:
          cerror("cannot change value of a string");
          exit(1);
      }
    case UNARY_PREINC:
      switch (currval.type){
        case TYPE_INTEGER:
          ret.v.i = currval.v.i + 1;
          ret.type = currval.type;
          setVariableValue(n->data.unaryop.expression->data.s, ret, n->block);
          return ret;
        case TYPE_FLOATING:
          ret.v.f = currval.v.f + 1.0f;
          ret.type = currval.type;
          setVariableValue(n->data.unaryop.expression->data.s, ret, n->block);
          return ret;
        case TYPE_STRING:
          cerror("cannot change value of a string");
          exit(1);
      }
    case UNARY_PREDEC:
      switch (currval.type){
        case TYPE_INTEGER:
          ret.v.i = currval.v.i - 1;
          ret.type = currval.type;
          setVariableValue(n->data.unaryop.expression->data.s, ret, n->block);
          return ret;
        case TYPE_FLOATING:
          ret.v.f = currval.v.f - 1.0f;
          ret.type = currval.type;
          setVariableValue(n->data.unaryop.expression->data.s, ret, n->block);
          return ret;
        case TYPE_STRING:
          cerror("cannot change value of a string");
          exit(1);
      }
    case UNARY_PLUS:
      switch (currval.type){
        case TYPE_INTEGER:
          ret.v.i = +currval.v.i;
          ret.type = currval.type;
          return ret;
        case TYPE_FLOATING:
          ret.v.f = +currval.v.f;
          ret.type = currval.type;
          return ret;
        case TYPE_STRING:
          cerror("cannot change value of a string");
          exit(1);
      }
    case UNARY_MINUS:
      switch (currval.type){
        case TYPE_INTEGER:
          ret.v.i = -currval.v.i;
          ret.type = currval.type;
          return ret;
        case TYPE_FLOATING:
          ret.v.f = -currval.v.f;
          ret.type = currval.type;
          return ret;
        case TYPE_STRING:
          cerror("cannot change value of a string");
          exit(1);
      }
    default: cerror("unknown unary expression");
             exit(1);
  }
}

Value execAssignment(struct Node *n)
{
  assert(n);
  assert(nt_ASSIGNMENT == n->kind);

  debug("exec", "assignment node <name: %s> at %p", n->data.s, n);

  Value ret;

  struct Node *r = n->data.assignment.right;
  Value r_val = dispatchNode(r);

  setVariableValue(n->data.s, r_val, n->block);

  ret = vtov(r_val, r_val.type);

  return ret;
}

Value execBlock(struct Node *n)
{
  assert(n);
  assert(nt_BLOCK == n->kind);

  debug("exec", "block node at %p", n);

  Value ret;
  ret.v.i = 0;
  ret.type = TYPE_INTEGER;

  for (int i = 0; i < n->data.block.count; i++){
    if (n->data.block.statements[i]->kind == nt_RETURN){
      ret = execReturn(n->data.block.statements[i]);
      break;
    } else {
      dispatchNode(n->data.block.statements[i]);
    }
  }

  for (int i = 0; i < n->data.block.count; i++){
    if (n->data.block.vars){
      for (struct VariableList *v = n->data.block.vars; v != NULL; v = v->next){
        debug("free", "variable at %p at the end executing block at %p", v->var, n);
        free(v->var);
        v->var = NULL;
      }
      free(n->data.block.vars);
      n->data.block.vars = NULL;
    }
  }

  return ret;
}

Value execStatement(struct Node *n)
{
  assert(n);
  assert(nt_STATEMENT == n->kind);

  debug("exec", "statement node at %p", n);

  Value ret;

  for (int i = 0; i < n->data.statement.count; i++){
    dispatchNode(n->data.statement.nodes[i]);
  }

  ret.v.i = 0;
  ret.type = TYPE_INTEGER;

  return ret;
}

Value execCall(struct Node *n)
{
  assert(n);
  assert(nt_CALL == n->kind);

  debug("exec", "call node <name: %s> at %p", n->data.call.name, n);

  Value ret;

  // check if that function is a predefined one
  // if so, execute it, and return value of that executed function
  for (unsigned int i = 0; i < predefs_size; i++){
    if (!strcmp(n->data.call.name, predefs[i].name)){
      ret = predefs[i].fn(n->data.call.params, n->data.call.paramcount);
      return ret;
    }
  }

  // if it's not a predefined function, we search for it
  // and exec
  for (struct UserdefFunction *t = userdefs; t != NULL; t = t->next){
    if (!strcmp(n->data.call.name, t->function->data.funcdef.name)){
      // checking for argument/param lenghts
      if (n->data.call.paramcount > t->function->data.funcdef.argcount){
        cerror("too many arguments for function '%s' (%d when %d expected)", t->function->data.funcdef.name, n->data.call.paramcount, t->function->data.funcdef.argcount);
        exit(1);
      } else if (n->data.call.paramcount < t->function->data.funcdef.argcount){
        cerror("too few arguments for function '%s' (%d when %d expected)", t->function->data.funcdef.name, n->data.call.paramcount, t->function->data.funcdef.argcount);
        exit(1);
      } else {
        for (struct ArgList *a = t->function->data.funcdef.args; a != NULL; a = a->next){
          struct VariableList *varlist = myalloc(sizeof(struct VariableList));
          struct Variable *var = myalloc(sizeof(struct Variable));

          varlist->var = var;
          varlist->var->type = a->arg->type;
          varlist->var->name = a->arg->name;

          for (struct ParamList *p = n->data.call.params; p != NULL; p = p->next){
            if (p->pos == a->pos)
              varlist->var->value = dispatchNode(p->param);
          }

          varlist->next = t->function->data.funcdef.body->data.block.vars;
          t->function->data.funcdef.body->data.block.vars = varlist;
        }

        return dispatchNode(t->function->data.funcdef.body);
      }
    }
  }

  cerror("couldn't find a function called '%s'", n->data.call.name);
  exit(1);
}

Value execReturn(struct Node *n)
{
  assert(n);
  assert(nt_RETURN == n->kind);

  debug("exec", "return node at %p", n);

  Value ret;

  ret.v.i = 0;
  ret.type = TYPE_INTEGER;

  if (n->data.returnn.expr)
    return dispatchNode(n->data.returnn.expr);
  else
    return ret;
}

Value execWhile(struct Node *n)
{
  assert(n);
  assert(nt_WHILE == n->kind);

  debug("exec", "while node at %p", n);

  Value ret;

  struct Node * const c = n->data.whilee.cond;
  struct Node * const s = n->data.whilee.statements;

  assert(c);
  assert(s);

  while (vtob(dispatchNode(c))){
    dispatchNode(s);
  }

  ret.v.i = 1;
  ret.type = TYPE_INTEGER;

  return ret;
}

Value execIf(struct Node *n)
{
  assert(n);
  assert(nt_IF == n->kind);

  debug("exec", "if node at %p", n);

  Value ret;

  struct Node * const c = n->data.iff.cond;
  struct Node * const s = n->data.iff.stmt;
  struct Node * const e = n->data.iff.elsestmt;

  assert(c);
  assert(s);

  if (vtob(dispatchNode(c))){
    dispatchNode(s);
  } else {
    if (n->data.iff.elsestmt){
      dispatchNode(e);
    }
  }

  ret.v.i = 1;
  ret.type = TYPE_INTEGER;

  return ret;
}

Value execFor(struct Node *n)
{
  assert(n);
  assert(nt_FOR == n->kind);

  debug("exec", "for node at %p", n);

  Value ret;

  ret.v.i = 1;
  ret.type = TYPE_INTEGER;

  struct Node * const i = n->data.forr.init;
  struct Node * const c = n->data.forr.cond;
  struct Node * const a = n->data.forr.action;
  struct Node * const s = n->data.forr.stmt;

  if (i){
    if (i->kind == nt_ASSIGNMENT){
      setVariableValue(i->data.assignment.name, dispatchNode(i->data.assignment.right), n->block);
    } else {
      cerror("wrong expression kind at first fortion");
      exit(1);
    }
  }

  if (c){
    if (c->kind != nt_BINARYOP && c->kind != nt_UNARYOP){
      cerror("wrong expression kind at second forition");
      exit(1);
    }
  }

  if (a){
    if (a->kind != nt_ASSIGNMENT && a->kind != nt_UNARYOP){
      cerror("wrong expression kind at third fortion");
      exit(1);
    }
  }

  while (c ? vtob(dispatchNode(c)) : 1){
    if (s)
      dispatchNode(s);
    if (a)
      dispatchNode(a);
  }

  return ret;
}

Value execFuncDef(struct Node *n)
{
  assert(n);
  assert(nt_FUNCDEF == n->kind);

  debug("exec", "function definiton node <name: %s> at %p", n->data.funcdef.name, n);

  Value ret;

  addFunction(n);

  ret.v.i = 1;
  ret.type = TYPE_INTEGER;

  return ret;
}

Value execIter(struct Node *n)
{
  assert(n);
  assert(nt_ITER == n->kind);

  debug("exec", "'%s' iter node at %p", n->data.iter.type, n);

  Value ret;
  Value count = dispatchNode(n->data.iter.count);

  int times = vtoi(count);

  for (int i = 0; i < times; i++){
    if (nt_BLOCK == n->data.iter.stmt->kind){
      // adding two variables to that block
      //
      // $+  will get bigger as the loops go (say: 0->9)
      // $-  will get smaller (say: 9->0)
      //
      Value plus_val;
      plus_val.v.i = i;
      plus_val.type = TYPE_INTEGER;
      Value minus_val;
      minus_val.v.i = (times - 1) - i;
      minus_val.type = TYPE_INTEGER;
      addVariableToBlock("$+", n->data.iter.stmt);
      setVariableValue("$+", plus_val, n->data.iter.stmt);
      addVariableToBlock("$-", n->data.iter.stmt);
      setVariableValue("$-", minus_val, n->data.iter.stmt);
    }

    dispatchNode(n->data.iter.stmt);
  }

  ret.v.i = 1;
  ret.type = TYPE_INTEGER;

  return ret;
}

Value execNoop(struct Node *n)
{
  Value ret;

  debug("exec", "but not really, a noop node at %p", n);

  return ret;
}

