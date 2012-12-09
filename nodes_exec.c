/*
 * nodes_exec.c
 *
 * Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
 *
 */

#include "nemo.h"
#include "handy.h"
#include "nodes_exec.h"
#include "nodes_gen.h"
#include "vars.h"
#include "cast.h"

struct FunctionTable {
  struct Node *function;
  struct FunctionTable *next;
};
// pointer to first element in FunctionTable
struct FunctionTable *funchead = NULL;

Value(*nodeExecs[])(struct Node *) =
{
  execID,
  execInteger,
  execFloating,
  execBinExpression,
  execUnExpression,
  execDeclaration,
  execAssignment,
  execBlock,
  execStatement,
  execCall,
  execReturn,
  execWhile,
  execIf,
  execFor,
  execFuncDef
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

  debug("executing id node <name: %s> at %p", n->data.s, n);

  if (variableAlreadySet(n->data.s, n->block)){
    return getVariableValue(n->data.s, n->block);
  } else {
    cerror("variable '%s' was not found", n->data.s);
    exit(1);
  }
}

Value execInteger(struct Node *n)
{
  assert(n);
  assert(nt_INTEGER == n->kind);

  debug("executing integer node <val: %d> at %p", n->data.value.v.i, n);

  return n->data.value;
}

Value execFloating(struct Node *n)
{
  assert(n);
  assert(nt_FLOATING == n->kind);

  debug("executing floating node <val: %f> at %p", n->data.value.v.f, n);

  return n->data.value;
}

Value execBinExpression(struct Node *n)
{
  assert(n);
  assert(nt_BINARYOP == n->kind);

  const Value left = dispatchNode(n->data.binaryop.left);
  const Value right = dispatchNode(n->data.binaryop.right);
  Value ret;

  debug("executing binary operation node <op: '%c'> at %p", n->data.binaryop.op, n);

  switch (n->data.binaryop.op){
    // binary PLUS
    case '+': if (left.type == TYPE_INTEGER){
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

    // binary MINUS
    case '-': if (left.type == TYPE_INTEGER){
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

    // binary MULTIPLE
    case '*': if (left.type == TYPE_INTEGER){
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

    // binary DIVIDE
    case '/': if (left.type == TYPE_INTEGER){
                // int / int
                if (right.type == TYPE_INTEGER){
                  if (right.v.i == 0){
                    cerror("zero divison!");
                    exit(1);
                  }
                  ret.v.i = left.v.i / right.v.i;
                  ret.type = TYPE_FLOATING;
                // int / float
                } else if (right.type == TYPE_FLOATING){
                  if (right.v.f == 0){
                    cerror("zero divison!");
                    exit(1);
                  }
                  ret.v.f = left.v.i / right.v.f;
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
                  ret.v.f = left.v.f / right.v.i;
                  ret.type = TYPE_FLOATING;
                // float / float
                } else if (right.type == TYPE_FLOATING){
                  if (right.v.f == 0){
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

    // binary MODULO
    case '%': if (left.type == TYPE_INTEGER){
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

    // binary GT
    case '>': if (left.type == TYPE_INTEGER){
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

    // binary GT
    case '<': if (left.type == TYPE_INTEGER){
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

    default: cerror("unknown operator '%c'", n->data.binaryop.op);
             exit(1);
  }

  return ret;
}

Value execUnExpression(struct Node *n)
{
  assert(n);
  assert(nt_UNARYOP == n->kind);

  debug("executing unary operation node <op: '%s'> at %p", unarytos(n->data.unaryop.op), n);

  const Value currval = getVariableValue(n->data.unaryop.expression->data.s, n->block);
  Value ret;

  switch (n->data.unaryop.op){
    case UNARY_POSTINC:
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
    case UNARY_POSTDEC:
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
    case UNARY_PREINC:
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
    case UNARY_PREDEC:
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
    default: cerror("unknown unary expression");
             exit(1);
  }
}

Value execDeclaration(struct Node *n)
{
  assert(n);
  assert(nt_DECLARATION == n->kind);

  debug("executing declaration node <name: %s> at %p", n->data.declaration.name, n);

  if (variableAlreadySet(n->data.declaration.name, n->block)){
    cerror("variable '%s' already declared", n->data.declaration.name);
    exit(1);
  }

  struct VariableList *varlist = myalloc(sizeof(struct VariableList));
  struct Variable *var = myalloc(sizeof(struct Variable));

  struct Node *r = n->data.declaration.right;

  Value ret;

  ret.v.i = 1;
  ret.type = TYPE_INTEGER;

  varlist->var = var;
  varlist->var->type = n->data.declaration.type;
  varlist->var->name = n->data.declaration.name;

  if (n->data.declaration.right == NULL){
    if (n->data.declaration.type == TYPE_INTEGER){
      varlist->var->value.v.i = 0;
      varlist->var->value.type = TYPE_INTEGER;
    } else if (n->data.declaration.type == TYPE_FLOATING){
      varlist->var->value.v.f = 0.0f;
      varlist->var->value.type = TYPE_FLOATING;
    }
  } else {
    varlist->var->value = vtov(dispatchNode(r), n->data.declaration.type);
  }

  varlist->next = n->block->data.block.vars;
  n->block->data.block.vars = varlist;

  return ret;
}

Value execAssignment(struct Node *n)
{
  assert(n);
  assert(nt_ASSIGNMENT == n->kind);

  debug("executing assignment node <name: %s> at %p", n->data.s, n);

  Value ret;

  if (!variableAlreadySet(n->data.s, n->block)){
    cerror("tried to change value of variable '%s' without declaring it first", n->data.s);
    exit(1);
  }

  struct Node *r = n->data.assignment.right;

  setVariableValue(n->data.s, dispatchNode(r), n->block);

  ret.v.i = 1;
  ret.type = TYPE_INTEGER;

  return ret;
}

Value execBlock(struct Node *n)
{
  assert(n);
  assert(nt_BLOCK == n->kind);

  debug("executing block node at %p", n);

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
        debug("freeing variable at %p at the end executing block at %p", v->var, n);
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

  debug("executing statement node at %p", n);

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

  debug("executing call node <name: %s> at %p", n->data.call.name, n);

  Value ret;

  struct FunctionTable *t;

  if (!strcmp(n->data.call.name, "out")){
    ret.v.i = 1;
    ret.type = TYPE_INTEGER;
    if (n->data.call.params){
      for (int i = 0; i < n->data.call.paramcount; i++){
        for (struct ParamList *p = n->data.call.params; p != NULL; p = p->next){
          if (p->pos == i){
            if (p->pos == n->data.call.paramcount - 1)
              printf("%s\n", vtos(dispatchNode(p->param)));
            else
              printf("%s, ", vtos(dispatchNode(p->param)));
          }
        }
      }
    }
    return ret;
  } else {
    for (t = funchead; t != NULL; t = t->next){
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

          return vtov(dispatchNode(t->function->data.funcdef.body), t->function->data.funcdef.returntype);
        }
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

  debug("executing while node at %p", n);

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

  debug("executing if node at %p", n);

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

  debug("executing for node at %p", n);

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

  debug("executing function definiton node <name: %s> at %p", n->data.funcdef.name, n);

  Value ret;

  struct FunctionTable *t;

  for (t = funchead; t != NULL; t = t->next){
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

