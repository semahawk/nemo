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
  execTermExpression,
  execTermExpression,
  execBinExpression,
  execUnExpression,
  execDeclaration,
  execAssignment,
  execBlock,
  execStatement,
  execCall,
  execWhile,
  execIf,
  execFuncDef
};

Value dispatchNode(struct Node *n)
{
  assert(n);
  assert(nodeExecs[n->kind]);

  return nodeExecs[n->kind](n);
}

void execNodes(struct Node *nodest)
{
  execBlock(nodest);
}

Value execTermExpression(struct Node *n)
{
  // TODO: refactor to an execNameExp and execVal functions
  assert(n);

  debug("executing id/integer expression node at %p", n);

  if (nt_INTEGER == n->kind){
    return n->data.value;
  } else {
    if (nt_ID == n->kind){
      if (variableAlreadySet(n->data.s, n->block))
        return getVariableValue(n->data.s, n->block);
      else
        cerror("variable '%s' doesn't exist", n->data.s);
        exit(1);
    } else {
      cerror("ough: tried to get the value of a non-expression(%d)", n->kind);
      exit(1);
     }
  }
}

Value execBinExpression(struct Node *n)
{
  assert(nt_BINARYOP == n->kind);

  const Value left = dispatchNode(n->data.binaryop.left);
  const Value right = dispatchNode(n->data.binaryop.right);
  Value ret;

  debug("executing binary operation node at %p", n);

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

  debug("executing declaration node at %p", n);

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

  debug("executing assignment node at %p", n);

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

  for (int i = 0; i < n->data.block.count; i++){
    dispatchNode(n->data.block.statements[i]);
  }

  for (int i = 0; i < n->data.block.count; i++){
    if (n->data.block.vars){
      for (struct VariableList *v = n->data.block.vars; v != NULL; v = v->next){
        free(v->var);
        v->var = NULL;
      }
      free(n->data.block.vars);
      n->data.block.vars = NULL;
    }
  }

  val.i = 0;

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

  debug("executing call node at %p", n);

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
  struct Node * const s = n->data.iff.statements;

  assert(c);
  assert(s);

  if (dispatchNode(c).i){
    dispatchNode(s);
  }

  val.i = 0;

  return val;
}

Value execFuncDef(struct Node *n)
{
  assert(n);
  assert(nt_FUNCDEF == n->kind);

  debug("executing function definiton node at %p", n);

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

