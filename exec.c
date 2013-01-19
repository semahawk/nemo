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

struct FunctionTable {
  struct Node *function;
  struct FunctionTable *next;
};
// pointer to first element in FunctionTable
struct FunctionTable *funchead = NULL;

Value(*nodeExecs[])(struct Node *) =
{
  execID,
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
  execIter
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
         nt_FLOATING == n->kind);

  debug("exec", "constant node <val: %s> at %p", vtos(n->data.value), n);

  return n->data.value;
}

Value execBinExpression(struct Node *n)
{
  assert(n);
  assert(nt_BINARYOP == n->kind);

  const Value left = dispatchNode(n->data.binaryop.left);
  const Value right = dispatchNode(n->data.binaryop.right);
  Value ret;
  Value new_value;

  debug("exec", "binary operation node <op: '%s'> at %p", binarytos(n->data.binaryop.op), n);

  switch (n->data.binaryop.op){
    //
    // XXX PLUS
    //
    case BINARY_ADD:
      if (left.type == TYPE_INTEGER){
        // int + int
        if (right.type == TYPE_INTEGER){
          ret.v.i = left.v.i + right.v.i;
          ret.type = TYPE_INTEGER;
        // int + float
        } else if (right.type == TYPE_FLOATING){
          ret.v.f = left.v.i + right.v.f;
          ret.type = TYPE_FLOATING;
        } else {
          cerror("right operand in binary is of unknown type");
          exit(1);
        }
      } else if (left.type == TYPE_FLOATING){
        // float + int
        if (right.type == TYPE_INTEGER){
          ret.v.f = left.v.f + right.v.i;
          ret.type = TYPE_FLOATING;
        // float + float
        } else if (right.type == TYPE_FLOATING){
          ret.v.f = left.v.f + right.v.f;
          ret.type = TYPE_FLOATING;
        }
      } else {
        cerror("right operand in binary is of unknown type");
        exit(1);
      }
      break;

    //
    // XXX MINUS
    //
    case BINARY_SUB:
      if (left.type == TYPE_INTEGER){
        // int - int
        if (right.type == TYPE_INTEGER){
          ret.v.i = left.v.i - right.v.i;
          ret.type = TYPE_INTEGER;
        // int - float
        } else if (right.type == TYPE_FLOATING){
          ret.v.f = left.v.i - right.v.f;
          ret.type = TYPE_FLOATING;
        } else {
          cerror("right operand in binary is of unknown type");
          exit(1);
        }
      } else if (left.type == TYPE_FLOATING){
        // float - int
        if (right.type == TYPE_INTEGER){
          ret.v.f = left.v.f - right.v.i;
          ret.type = TYPE_FLOATING;
        // float - float
        } else if (right.type == TYPE_FLOATING){
          ret.v.f = left.v.f - right.v.f;
          ret.type = TYPE_FLOATING;
        }
      } else {
        cerror("right operand in binary is of unknown type");
        exit(1);
      }
      break;

    //
    // XXX MULTIPLE
    //
    case BINARY_MUL:
      if (left.type == TYPE_INTEGER){
        // int * int
        if (right.type == TYPE_INTEGER){
          ret.v.i = left.v.i * right.v.i;
          ret.type = TYPE_INTEGER;
        // int * float
        } else if (right.type == TYPE_FLOATING){
          ret.v.f = left.v.i * right.v.f;
          ret.type = TYPE_FLOATING;
        } else {
          cerror("right operand in binary is of unknown type");
          exit(1);
        }
      } else if (left.type == TYPE_FLOATING){
        // float * int
        if (right.type == TYPE_INTEGER){
          ret.v.f = left.v.f * right.v.i;
          ret.type = TYPE_FLOATING;
        // float * float
        } else if (right.type == TYPE_FLOATING){
          ret.v.f = left.v.f * right.v.f;
          ret.type = TYPE_FLOATING;
        }
      } else {
        cerror("right operand in binary is of unknown type");
        exit(1);
      }
      break;

    //
    // XXX DIVIDE
    //
    case BINARY_DIV:
      if (left.type == TYPE_INTEGER){
        // int / int
        if (right.type == TYPE_INTEGER){
          if (right.v.i == 0){
            cerror("zero divison!");
            exit(1);
          }
          ret.v.f = vtof(left) / vtof(right);
          ret.type = TYPE_FLOATING;
        // int / float
        } else if (right.type == TYPE_FLOATING){
          if (right.v.f == 0.0f){
            cerror("zero divison!");
            exit(1);
          }
          ret.v.f = vtof(left) / right.v.f;
          ret.type = TYPE_FLOATING;
        } else {
          cerror("right operand in binary is of unknown type");
          exit(1);
        }
      } else if (left.type == TYPE_FLOATING){
        // float / int
        if (right.type == TYPE_INTEGER){
          if (right.v.i == 0){
            cerror("zero divison!");
            exit(1);
          }
          ret.v.f = left.v.f / vtof(right);
          ret.type = TYPE_FLOATING;
        // float / float
        } else if (right.type == TYPE_FLOATING){
          if (right.v.f == 0.0f){
            cerror("zero divison!");
            exit(1);
          }
          ret.v.f = left.v.f / right.v.f;
          ret.type = TYPE_FLOATING;
        }
      } else {
        cerror("right operand in binary is of unknown type");
        exit(1);
      }
      break;

    //
    // XXX MODULO
    //
    case BINARY_MOD:
      if (left.type == TYPE_INTEGER){
        // int % int
        if (right.type == TYPE_INTEGER){
          ret.v.i = (int)(left.v.i % right.v.i);
          ret.type = TYPE_INTEGER;
        // int % float
        } else if (right.type == TYPE_FLOATING){
          ret.v.i = (int)(left.v.i % (int)right.v.f);
          ret.type = TYPE_INTEGER;
        } else {
          cerror("right operand in binary is of unknown type");
          exit(1);
        }
      } else if (left.type == TYPE_FLOATING){
        // float % int
        if (right.type == TYPE_INTEGER){
          ret.v.i = (int)((int)left.v.f % right.v.i);
          ret.type = TYPE_INTEGER;
        // float % float
        } else if (right.type == TYPE_FLOATING){
          ret.v.i = (int)((int)left.v.f % (int)right.v.f);
          ret.type = TYPE_INTEGER;
        }
      } else {
        cerror("right operand in binary is of unknown type");
        exit(1);
      }
      break;

    //
    // XXX GT
    //
    case BINARY_GT:
      if (left.type == TYPE_INTEGER){
        // int > int
        if (right.type == TYPE_INTEGER){
          ret.v.i = (left.v.i > right.v.i) ? 1 : 0;
          ret.type = TYPE_INTEGER;
        // int > float
        } else if (right.type == TYPE_FLOATING){
          ret.v.i = (left.v.i > right.v.f) ? 1 : 0;
          ret.type = TYPE_INTEGER;
        } else {
          cerror("right operand in binary is of unknown type");
          exit(1);
        }
      } else if (left.type == TYPE_FLOATING){
        // float > int
        if (right.type == TYPE_INTEGER){
          ret.v.i = (left.v.f > right.v.i) ? 1 : 0;
          ret.type = TYPE_INTEGER;
        // float > float
        } else if (right.type == TYPE_FLOATING){
          ret.v.i = (left.v.f > right.v.f) ? 1 : 0;
          ret.type = TYPE_INTEGER;
        }
      } else {
        cerror("right operand in binary is of unknown type");
        exit(1);
      }
      break;

    //
    // XXX LT
    //
    case BINARY_LT:
      if (left.type == TYPE_INTEGER){
        // int < int
        if (right.type == TYPE_INTEGER){
          ret.v.i = (left.v.i < right.v.i) ? 1 : 0;
          ret.type = TYPE_INTEGER;
        // int < float
        } else if (right.type == TYPE_FLOATING){
          ret.v.i = (left.v.i < right.v.f) ? 1 : 0;
          ret.type = TYPE_INTEGER;
        } else {
          cerror("right operand in binary is of unknown type");
          exit(1);
        }
      } else if (left.type == TYPE_FLOATING){
        // float < int
        if (right.type == TYPE_INTEGER){
          ret.v.i = (left.v.f < right.v.i) ? 1 : 0;
          ret.type = TYPE_INTEGER;
        // float < float
        } else if (right.type == TYPE_FLOATING){
          ret.v.i = (left.v.f < right.v.f) ? 1 : 0;
          ret.type = TYPE_INTEGER;
        }
      } else {
        cerror("right operand in binary is of unknown type");
        exit(1);
      }
      break;

    //
    // XXX GE
    //
    case BINARY_GE:
      if (left.type == TYPE_INTEGER){
        // int >= int
        if (right.type == TYPE_INTEGER){
          ret.v.i = (left.v.i >= right.v.i) ? 1 : 0;
          ret.type = TYPE_INTEGER;
        // int >= float
        } else if (right.type == TYPE_FLOATING){
          ret.v.i = (left.v.i >= right.v.f) ? 1 : 0;
          ret.type = TYPE_INTEGER;
        }
      } else if (left.type == TYPE_FLOATING){
        // float >= int
        if (right.type == TYPE_INTEGER){
          ret.v.i = (left.v.f >= right.v.i) ? 1 : 0;
          ret.type = TYPE_INTEGER;
        // float >= float
        } else if (right.type == TYPE_FLOATING){
          ret.v.i = (left.v.f >= right.v.f) ? 1 : 0;
          ret.type = TYPE_INTEGER;
        }
      }
      break;

    //
    // XXX LE
    //
    case BINARY_LE:
      if (left.type == TYPE_INTEGER){
        // int <= int
        if (right.type == TYPE_INTEGER){
          ret.v.i = (left.v.i <= right.v.i) ? 1 : 0;
          ret.type = TYPE_INTEGER;
        // int <= float
        } else if (right.type == TYPE_FLOATING){
          ret.v.i = (left.v.i <= right.v.f) ? 1 : 0;
          ret.type = TYPE_INTEGER;
        }
      } else if (left.type == TYPE_FLOATING){
        // float <= int
        if (right.type == TYPE_INTEGER){
          ret.v.i = (left.v.f <= right.v.i) ? 1 : 0;
          ret.type = TYPE_INTEGER;
        // float <= float
        } else if (right.type == TYPE_FLOATING){
          ret.v.i = (left.v.f <= right.v.f) ? 1 : 0;
          ret.type = TYPE_INTEGER;
        }
      }
      break;

    //
    // XXX NE
    //
    case BINARY_NE:
      if (left.type == TYPE_INTEGER){
        // int != int
        if (right.type == TYPE_INTEGER){
          ret.v.i = (left.v.i != right.v.i) ? 1 : 0;
          ret.type = TYPE_INTEGER;
        // int != float
        } else if (right.type == TYPE_FLOATING){
          ret.v.i = (left.v.i != right.v.f) ? 1 : 0;
          ret.type = TYPE_INTEGER;
        }
      } else if (left.type == TYPE_FLOATING){
        // float != int
        if (right.type == TYPE_INTEGER){
          ret.v.i = (left.v.f != right.v.i) ? 1 : 0;
          ret.type = TYPE_INTEGER;
        // float != float
        } else if (right.type == TYPE_FLOATING){
          ret.v.i = (left.v.f != right.v.f) ? 1 : 0;
          ret.type = TYPE_INTEGER;
        }
      }
      break;

    //
    // XXX EQ
    //
    case BINARY_EQ:
      if (left.type == TYPE_INTEGER){
        // int == int
        if (right.type == TYPE_INTEGER){
          ret.v.i = (left.v.i == right.v.i) ? 1 : 0;
          ret.type = TYPE_INTEGER;
        // int == float
        } else if (right.type == TYPE_FLOATING){
          ret.v.i = (left.v.i == right.v.f) ? 1 : 0;
          ret.type = TYPE_INTEGER;
        }
      } else if (left.type == TYPE_FLOATING){
        // float == int
        if (right.type == TYPE_INTEGER){
          ret.v.i = (left.v.f == right.v.i) ? 1 : 0;
          ret.type = TYPE_INTEGER;
        // float == float
        } else if (right.type == TYPE_FLOATING){
          ret.v.i = (left.v.f == right.v.f) ? 1 : 0;
          ret.type = TYPE_INTEGER;
        }
      }
      break;

    //
    // XXX EQ_ADD
    //
    case BINARY_EQ_ADD:
      if (left.type == TYPE_INTEGER){
        // int += int
        if (right.type == TYPE_INTEGER){
          new_value.v.i = left.v.i + right.v.i;
          new_value.type = TYPE_INTEGER;
          ret = new_value;
          setVariableValue(n->data.binaryop.left->data.s, new_value, n->block);
        // int += float
        } else if (right.type == TYPE_FLOATING){
          new_value.v.f = left.v.i + right.v.f;
          new_value.type = TYPE_FLOATING;
          ret = new_value;
          setVariableValue(n->data.binaryop.left->data.s, new_value, n->block);
        }
      } else if (left.type == TYPE_FLOATING){
        // float += int
        if (right.type == TYPE_INTEGER){
          new_value.v.f = left.v.f + right.v.i;
          new_value.type = TYPE_FLOATING;
          ret = new_value;
          setVariableValue(n->data.binaryop.left->data.s, new_value, n->block);
        // float += float
        } else if (right.type == TYPE_FLOATING){
          new_value.v.f = left.v.f + right.v.f;
          new_value.type = TYPE_FLOATING;
          ret = new_value;
          setVariableValue(n->data.binaryop.left->data.s, new_value, n->block);
        }
      }
      break;

    //
    // XXX EQ_SUB
    //
    case BINARY_EQ_SUB:
      if (left.type == TYPE_INTEGER){
        // int -= int
        if (right.type == TYPE_INTEGER){
          new_value.v.i = left.v.i - right.v.i;
          new_value.type = TYPE_INTEGER;
          ret = new_value;
          setVariableValue(n->data.binaryop.left->data.s, new_value, n->block);
        // int -= float
        } else if (right.type == TYPE_FLOATING){
          new_value.v.f = left.v.i - right.v.f;
          new_value.type = TYPE_FLOATING;
          ret = new_value;
          setVariableValue(n->data.binaryop.left->data.s, new_value, n->block);
        }
      } else if (left.type == TYPE_FLOATING){
        // float -= int
        if (right.type == TYPE_INTEGER){
          new_value.v.f = left.v.f - right.v.i;
          new_value.type = TYPE_FLOATING;
          ret = new_value;
          setVariableValue(n->data.binaryop.left->data.s, new_value, n->block);
        // float -= float
        } else if (right.type == TYPE_FLOATING){
          new_value.v.f = left.v.f - right.v.f;
          new_value.type = TYPE_FLOATING;
          ret = new_value;
          setVariableValue(n->data.binaryop.left->data.s, new_value, n->block);
        }
      }
      break;

    //
    // XXX EQ_MUL
    //
    case BINARY_EQ_MUL:
      if (left.type == TYPE_INTEGER){
        // int *= int
        if (right.type == TYPE_INTEGER){
          new_value.v.i = left.v.i * right.v.i;
          new_value.type = TYPE_INTEGER;
          ret = new_value;
          setVariableValue(n->data.binaryop.left->data.s, new_value, n->block);
        // int *= float
        } else if (right.type == TYPE_FLOATING){
          new_value.v.f = left.v.i * right.v.f;
          new_value.type = TYPE_FLOATING;
          ret = new_value;
          setVariableValue(n->data.binaryop.left->data.s, new_value, n->block);
        }
      } else if (left.type == TYPE_FLOATING){
        // float *= int
        if (right.type == TYPE_INTEGER){
          new_value.v.f = left.v.f * right.v.i;
          new_value.type = TYPE_FLOATING;
          ret = new_value;
          setVariableValue(n->data.binaryop.left->data.s, new_value, n->block);
        // float *= float
        } else if (right.type == TYPE_FLOATING){
          new_value.v.f = left.v.f * right.v.f;
          new_value.type = TYPE_FLOATING;
          ret = new_value;
          setVariableValue(n->data.binaryop.left->data.s, new_value, n->block);
        }
      }
      break;

    //
    // XXX EQ_DIV
    //
    case BINARY_EQ_DIV:
      if (left.type == TYPE_INTEGER){
        // int /= int
        if (right.type == TYPE_INTEGER){
          new_value.v.f = vtof(left) / vtof(right);
          new_value.type = TYPE_FLOATING;
          ret = new_value;
          setVariableValue(n->data.binaryop.left->data.s, new_value, n->block);
        // int /= float
        } else if (right.type == TYPE_FLOATING){
          new_value.v.f = vtof(left) / right.v.f;
          new_value.type = TYPE_FLOATING;
          ret = new_value;
          setVariableValue(n->data.binaryop.left->data.s, new_value, n->block);
        }
      } else if (left.type == TYPE_FLOATING){
        // float /= int
        if (right.type == TYPE_INTEGER){
          new_value.v.f = left.v.f / vtof(right);
          new_value.type = TYPE_FLOATING;
          ret = new_value;
          setVariableValue(n->data.binaryop.left->data.s, new_value, n->block);
        // float /= float
        } else if (right.type == TYPE_FLOATING){
          new_value.v.f = left.v.f / right.v.f;
          new_value.type = TYPE_FLOATING;
          ret = new_value;
          setVariableValue(n->data.binaryop.left->data.s, new_value, n->block);
        }
      }
      break;

    //
    // XXX EQ_MOD
    //
    case BINARY_EQ_MOD:
      if (left.type == TYPE_INTEGER){
        // int %= int
        if (right.type == TYPE_INTEGER){
          if (right.v.i == 0){
            cerror("zero division!");
            exit(1);
          }
          new_value.v.i = left.v.i % vtoi(right);
          new_value.type = TYPE_INTEGER;
          ret = new_value;
          setVariableValue(n->data.binaryop.left->data.s, new_value, n->block);
        // int %= float
        } else if (right.type == TYPE_FLOATING){
          if (vtoi(right) == 0){
            cerror("zero division!");
            exit(1);
          }
          new_value.v.i = left.v.i % vtoi(right);
          new_value.type = TYPE_FLOATING;
          ret = new_value;
          setVariableValue(n->data.binaryop.left->data.s, new_value, n->block);
        }
      } else if (left.type == TYPE_FLOATING){
        // float %= int
        if (right.type == TYPE_INTEGER){
          if (right.v.i == 0){
            cerror("zero division!");
            exit(1);
          }
          new_value.v.i = vtoi(left) % right.v.i;
          new_value.type = TYPE_FLOATING;
          ret = new_value;
          setVariableValue(n->data.binaryop.left->data.s, new_value, n->block);
        // float %= float
        } else if (right.type == TYPE_FLOATING){
          if (vtoi(right) == 0){
            cerror("zero division!");
            exit(1);
          }
          new_value.v.i = vtoi(left) % vtoi(right);
          new_value.type = TYPE_FLOATING;
          ret = new_value;
          setVariableValue(n->data.binaryop.left->data.s, new_value, n->block);
        }
      }
      break;

    default: cerror("unknown operator '%s'", binarytos(n->data.binaryop.op));
             exit(1);
  }

  return ret;
}

Value execUnExpression(struct Node *n)
{
  assert(n);
  assert(nt_UNARYOP == n->kind);

  debug("exec", "unary operation node <op: '%s'> at %p", unarytos(n->data.unaryop.op), n);

  const Value currval = getVariableValue(n->data.unaryop.expression->data.s, n->block);
  Value ret;

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
  for (struct FunctionTable *t = funchead; t != NULL; t = t->next){
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
      // TODO: make it could be a declaration
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

  for (struct FunctionTable *t = funchead; t != NULL; t = t->next){
    if (!strcmp(t->function->data.funcdef.name, n->data.funcdef.name)){
      cerror("function '%s' already defined", n->data.funcdef.name);
      exit(1);
    }
  }

  struct FunctionTable *functable = myalloc(sizeof(struct FunctionTable));

  functable->function = n;
  functable->next = funchead;
  funchead = functable;

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

