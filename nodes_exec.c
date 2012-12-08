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

extern int varscount;
int functionscount = 0;

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
  return execBlock(nodest);
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

  debug("executing integer node <val: %d> at %p", n->data.value.i, n);

  return n->data.value;
}

Value execBinExpression(struct Node *n)
{
  assert(nt_BINARYOP == n->kind);

  const Value left = dispatchNode(n->data.binaryop.left);
  const Value right = dispatchNode(n->data.binaryop.right);
  Value ret;

  debug("executing binary operation node <op: '%c'> at %p", n->data.binaryop.op, n);

  switch (n->data.binaryop.op){
    case '+': ret.i = left.i + right.i;
              break;
    case '-': ret.i = left.i - right.i;
              break;
    case '*': ret.i = left.i * right.i;
              break;
    case '/': if (right.i == 0){
                cerror("zero division!");
                exit(1);
              } else {
                ret.i = left.i / right.i;
              }
    case '%': ret.i = left.i % right.i;
              break;
    case '>': ret.i = left.i > right.i;
              break;
    case '<': ret.i = left.i < right.i;
              break;

    default: cerror("unknown operator '%c'", n->data.binaryop.op);
             exit(1);
  }

  return ret;
}

Value execUnExpression(struct Node *n)
{
  assert(nt_UNARYOP == n->kind);

  debug("executing unary operation node at %p", n);

  const Value currval = getVariableValue(n->data.unaryop.expression->data.s, n->block);
  Value ret;

  switch (n->data.unaryop.op){
    case UNARY_POSTINC:
      ret.i = currval.i + 1;
      setVariableValue(n->data.unaryop.expression->data.s, ret, n->block);
      return ret;
    case UNARY_POSTDEC:
      ret.i = currval.i - 1;
      setVariableValue(n->data.unaryop.expression->data.s, ret, n->block);
      return ret;
    case UNARY_PREINC:
      ret.i = currval.i + 1;
      setVariableValue(n->data.unaryop.expression->data.s, ret, n->block);
      return currval;
    case UNARY_PREDEC:
      ret.i = currval.i - 1;
      setVariableValue(n->data.unaryop.expression->data.s, ret, n->block);
      return currval;
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

  Value val;

  val.i = 0;

  varscount++;

  varlist->var = var;
  varlist->var->type = n->data.declaration.type;
  varlist->var->name = n->data.declaration.name;

  if (n->data.declaration.right == NULL){
    varlist->var->value.i = 0;
  } else {
    varlist->var->value = dispatchNode(r);
  }

  varlist->next = n->block->data.block.vars;
  n->block->data.block.vars = varlist;

  return val;
}

Value execAssignment(struct Node *n)
{
  assert(n);
  assert(nt_ASSIGNMENT == n->kind);

  debug("executing assignment node <name: %s> at %p", n->data.s, n);

  Value val;

  if (!variableAlreadySet(n->data.s, n->block)){
    cerror("tried to change value of variable '%s' without declaring it first", n->data.s);
    exit(1);
  }

  struct Node *r = n->data.assignment.right;

  setVariableValue(n->data.s, dispatchNode(r), n->block);

  val.i = 0;

  return val;
}

Value execBlock(struct Node *n)
{
  assert(n);
  assert(nt_BLOCK == n->kind);

  debug("executing block node at %p", n);

  Value val;
  val.i = 0;

  for (int i = 0; i < n->data.block.count; i++){
    if (n->data.block.statements[i]->kind == nt_RETURN){
      val = execReturn(n->data.block.statements[i]);
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

  return val;
}

Value execStatement(struct Node *n)
{
  assert(n);
  assert(nt_STATEMENT == n->kind);

  debug("executing statement node at %p", n);

  Value val;

  for (int i = 0; i < n->data.statement.count; i++){
    dispatchNode(n->data.statement.nodes[i]);
  }

  val.i = 0;

  return val;
}

Value execCall(struct Node *n)
{
  assert(n);
  assert(nt_CALL == n->kind);

  debug("executing call node <name: %s> at %p", n->data.call.name, n);

  Value val;

  struct FunctionTable *t;

  if (!strcmp(n->data.call.name, "out")){
    val.i = 0;
    if (n->data.call.params){
      for (int i = 0; i < n->data.call.paramcount; i++){
        for (struct ParamList *p = n->data.call.params; p != NULL; p = p->next){
          if (p->pos == i){
            if (p->pos == n->data.call.paramcount - 1)
              printf("%d\n", dispatchNode(p->param).i);
            else
              printf("%d, ", dispatchNode(p->param).i);
            val.i = dispatchNode(p->param).i;
          }
        }
      }
    }
    return val;
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

          return dispatchNode(t->function->data.funcdef.body);
        }
      }
    }
  }

  cerror("couldn't find a function called '%s'", n->data.call.name);
  exit(1);
}

Value execReturn(struct Node *n)
{
  Value ret;
  ret.i = 0;

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

  Value val;

  struct Node * const c = n->data.whilee.cond;
  struct Node * const s = n->data.whilee.statements;

  assert(c);
  assert(s);

  while (dispatchNode(c).i){
    dispatchNode(s);
  }

  val.i = 0;

  return val;
}

Value execIf(struct Node *n)
{
  assert(n);
  assert(nt_IF == n->kind);

  debug("executing if node at %p", n);

  Value val;

  struct Node * const c = n->data.iff.cond;
  struct Node * const s = n->data.iff.stmt;
  struct Node * const e = n->data.iff.elsestmt;

  assert(c);
  assert(s);

  if (dispatchNode(c).i){
    dispatchNode(s);
  } else {
    if (n->data.iff.elsestmt){
      dispatchNode(e);
    }
  }

  val.i = 0;

  return val;
}

Value execFor(struct Node *n)
{
  assert(n);
  assert(nt_FOR == n->kind);

  debug("executing for node at %p", n);

  Value ret;

  ret.i = 1;

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

  while (c ? dispatchNode(c).i : 1){
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

  Value val;

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

  val.i = 0;

  return val;
}

