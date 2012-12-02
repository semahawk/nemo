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
static Value execUnExpression(struct Node *);
static Value execDeclaration(struct Node *);
static Value execAssignment(struct Node *);
static Value execCall(struct Node *);
static Value execWhilst(struct Node *);
static Value execAn(struct Node *);
static Value execBlock(struct Node *);
static Value execStatement(struct Node *);

static void freeTermExpression(struct Node *);
static void freeBinExpression(struct Node *);
static void freeUnExpression(struct Node *);
static void freeDeclaration(struct Node *);
static void freeAssignment(struct Node *);
static void freeCall(struct Node *);
static void freeWhilst(struct Node *);
static void freeAn(struct Node *);
static void freeBlock(struct Node *);
static void freeStatement(struct Node *);

static Value(*nodeExecs[])(struct Node *) =
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
  execWhilst,
  execAn
};

static void(*nodeFrees[])(struct Node *) =
{
  freeTermExpression,
  freeTermExpression,
  freeBinExpression,
  freeUnExpression,
  freeDeclaration,
  freeAssignment,
  freeBlock,
  freeStatement,
  freeCall,
  freeWhilst,
  freeAn
};

static Value dispatchNode(struct Node *n)
{
  assert(n);
  assert(nodeExecs[n->kind]);

  return nodeExecs[n->kind](n);
}

static void freeNode(struct Node *n)
{
  assert(n);
  assert(nodeFrees[n->kind]);

  nodeFrees[n->kind](n);
  n = NULL;
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

  const Value left = dispatchNode(n->data.binaryop.left);
  const Value right = dispatchNode(n->data.binaryop.right);
  Value ret;

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

static Value execUnExpression(struct Node *n)
{
  assert(nt_UNARYOP == n->kind);

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

void execNodes(struct Node *nodest)
{
  execBlock(nodest);
}

void freeNodes(struct Node *nodest)
{
  assert(nodest);
  assert(nt_BLOCK == nodest->kind);

  freeNode(nodest);

  nodest = NULL;
}

void freeTermExpression(struct Node *n)
{
  assert(n);

  debug("freeing id/integer node at %p", n);

  free(n);
}

void freeBinExpression(struct Node *n)
{
  assert(n);
  assert(nt_BINARYOP == n->kind);

  debug("freeing binary operation node at %p", n);

  freeNode(n->data.binaryop.left);
  freeNode(n->data.binaryop.right);

  free(n);
}

void freeUnExpression(struct Node *n)
{
  assert(n);
  assert(nt_UNARYOP == n->kind);

  debug("freeing unary operation node at %p", n);

  freeNode(n->data.unaryop.expression);

  free(n);
}

void freeDeclaration(struct Node *n)
{
  assert(n);
  assert(nt_DECLARATION == n->kind);

  debug("freeing declaration node at %p", n);

  freeNode(n->data.declaration.right);

  free(n);
}

void freeAssignment(struct Node *n)
{
  assert(n);
  assert(nt_ASSIGNMENT == n->kind);

  debug("freeing assignment node at %p", n);

  freeNode(n->data.assignment.right);

  free(n);
}

void freeCall(struct Node *n)
{
  assert(n);
  assert(nt_CALL == n->kind);

  debug("freeing call node at %p", n);

  freeNode(n->data.call.param);

  free(n);
}

void freeWhilst(struct Node *n)
{
  assert(n);
  assert(nt_WHILST == n->kind);

  debug("freeing whilst node at %p", n);

  freeNode(n->data.whilst.cond);
  freeNode(n->data.whilst.statements);

  free(n);
}

void freeAn(struct Node *n)
{
  assert(n);
  assert(nt_AN == n->kind);

  debug("freeing an node at %p", n);

  freeNode(n->data.an.cond);
  freeNode(n->data.an.statements);

  free(n);
}

void freeBlock(struct Node *n)
{
  assert(n);
  assert(nt_BLOCK == n->kind);

  debug("freeing block node at %p", n);

  for (int i = 0; i < n->data.block.count; i++){
    freeNode(n->data.block.statements[i]);
  }

  free(n);
}

void freeStatement(struct Node *n)
{
  assert(n);
  assert(nt_STATEMENT == n->kind);

  debug("freeing statement node at %p", n);

  for (int i = 0; i < n->data.statement.count; i++){
    freeNode(n->data.statement.nodes[i]);
  }

  free(n);
}

