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

// keep track of how many vars we've declared
int varscount = 0;

struct Variable {
  Value value;
  char *name;
  Type type;
};

struct VariableList {
  struct Variable *var;
  struct VariableList *next;
};

static Value execTermExpression(struct Node *);
static Value execBinExpression(struct Node *);
static Value execDeclaration(struct Node *);
static Value execAssignment(struct Node *);
static Value execCall(struct Node *);
static Value execWhilst(struct Node *);
static Value execAn(struct Node *);
static Value execBlock(struct Node *);
static Value execStatement(struct Node *);

static Value(*nodeExecs[])(struct Node *) =
{
  execTermExpression,
  execTermExpression,
  execBinExpression,
  execDeclaration,
  execAssignment,
  execBlock,
  execStatement,
  execCall,
  execWhilst,
  execAn
};

static Value dispatchNode( struct Node *n)
{
  assert(n);
  assert(nodeExecs[n->kind]);

  return nodeExecs[n->kind](n);
}

static void onlyName(const char *name, const char *ref, const char *kind)
{
  if (strcmp(ref, name)){
    cerror("Nemo only knows the %s '%s', not '%s' ", kind, ref, name);
    exit(1);
  }
}

static Value getVariableValue(const char *name, struct Node *block)
{
  struct Node *b;
  struct VariableList *p;

  for (b = block; b != NULL; b = b->data.block.parent){
    for (p = b->data.block.vars; p != NULL; p = p->next){
      if (!strcmp(name, p->var->name)){
        return p->var->value;
      }
    }
  }

  cerror("variable '%s' was not found", name);
  exit(1);
}

static void setVariableValue(const char *name, Value value, struct Node *block)
{
  struct Node *b;
  struct VariableList *p;

  for (b = block; b != NULL; b = b->data.block.parent){
    for (p = b->data.block.vars; p != NULL; p = p->next){
      if (!strcmp(name, p->var->name)){
        p->var->value = value;
      }
    }
  }
}

static bool variableAlreadySet(const char *name, struct Node *block)
{
  struct Node *b;
  struct VariableList *p;

  for (b = block; b != NULL; b = b->data.block.parent){
    for (p = b->data.block.vars; p != NULL; p = p->next){
      if (!strcmp(name, p->var->name)){
        return true;
      }
    }
  }

  return false;
}

static void onlyOut(const char *name)
{
  onlyName(name, "out", "function");
}

static Value execTermExpression(struct Node *n)
{
  // TODO: refactor to an execNameExp and execVal functions
  assert(n);

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

static Value execBinExpression(struct Node *n)
{
  assert(nt_BINARYOP == n->kind);

  const Value left = dispatchNode(n->data.expression.left);
  const Value right = dispatchNode(n->data.expression.right);
  Value ret;

  switch (n->data.expression.op){
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

    default: cerror("unknown operator '%c'", n->data.expression.op);
             exit(1);
  }

  return ret;
}

static Value execDeclaration(struct Node *n)
{
  assert(n);
  assert(nt_DECLARATION == n->kind);

  if (variableAlreadySet(n->data.declaration.name, n->block)){
    cerror("variable '%s' already declared", n->data.declaration.name);
    exit(1);
  }

  struct VariableList *varlist = myalloc(sizeof(struct VariableList));
  struct Variable *var = myalloc(sizeof(struct Variable));

  struct Node *r = n->data.declaration.right;

  debug("declaration node is in block at %p", n->block);

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

static Value execAssignment(struct Node *n)
{
  assert(n);
  assert(nt_ASSIGNMENT == n->kind);

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

static Value execBlock(struct Node *n)
{
  assert(n);
  assert(nt_BLOCK == n->kind);

  Value val;

  for (int i = 0; i < n->data.block.count; i++){
    dispatchNode(n->data.block.statements[i]);
  }

  val.i = 0;

  return val;
}

static Value execStatement(struct Node *n)
{
  assert(n);
  assert(nt_STATEMENT == n->kind);

  Value val;

  for (int i = 0; i < n->data.statement.count; i++){
    dispatchNode(n->data.statement.nodes[i]);
  }
  
  val.i = 0;

  return val;
}

static Value execCall(struct Node *n)
{
  assert(n);
  assert(nt_CALL == n->kind);

  Value val;

  onlyOut(n->data.call.name);
  printf("%d\n", dispatchNode(n->data.call.param).i);

  val.i = 0;

  return val;
}

void execNodes(struct Node *n)
{
  execBlock(n);
}

static Value execWhilst(struct Node *n)
{
  assert(n);
  assert(nt_WHILST == n->kind);

  Value val;

  struct Node * const c = n->data.whilst.cond;
  struct Node * const s = n->data.whilst.statements;

  assert(c);
  assert(s);

  while (dispatchNode(c).i){
    dispatchNode(s);
  }

  val.i = 0;

  return val;
}

static Value execAn(struct Node *n)
{
  assert(n);
  assert(nt_AN == n->kind);

  Value val;

  struct Node * const c = n->data.an.cond;
  struct Node * const s = n->data.an.statements;

  assert(c);
  assert(s);

  if (dispatchNode(c).i){
    dispatchNode(s);
  }

  val.i = 0;

  return val;
}

