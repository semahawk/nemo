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

struct ExecEnv {
  // pointer to our first element
  struct VariableList *vars;
};

static Value execTermExpression(struct ExecEnv *, struct Node *);
static Value execBinExpression(struct ExecEnv *, struct Node *);
static Value execDeclaration(struct ExecEnv *, struct Node *);
static Value execAssignment(struct ExecEnv *, struct Node *);
static void execStatement(struct ExecEnv *, struct Node *);
static Value execCall(struct ExecEnv *, struct Node *);
static Value execWhilst(struct ExecEnv *, struct Node *);
static Value execAn(struct ExecEnv *, struct Node *);

static Value(*valExecs[])(struct ExecEnv *, struct Node *) =
{
  execTermExpression,
  execTermExpression,
  execBinExpression,
  execDeclaration,
  execAssignment,
  NULL,
  execCall,
  execWhilst,
  execAn
};

static void(*runExecs[])(struct ExecEnv *, struct Node *) =
{
  NULL, // ID and numbers are canonical and
  NULL, // don't need to be executed
  NULL, // so is not a binary op
  NULL,
  NULL,
  execStatement,
  NULL,
  NULL,
  NULL,
};

static Value dispatchExpression(struct ExecEnv *e, struct Node *n)
{
  assert(n);
  assert(valExecs[n->kind]);

  return valExecs[n->kind](e, n);
}

static void dispatchStatement(struct ExecEnv *e, struct Node *n)
{
  assert(n);
  assert(runExecs[n->kind]);

  runExecs[n->kind](e, n);
}

static void onlyName(const char *name, const char *ref, const char *kind)
{
  if (strcmp(ref, name)){
    cerror("Nemo only knows the %s '%s', not '%s' ", kind, ref, name);
    exit(1);
  }
}

static Value getVariableValue(struct ExecEnv *e, const char *name)
{
  struct VariableList *p;

  for (p = e->vars; p != NULL; p = p->next){
    if (!strcmp(name, p->var->name)){
      return p->var->value;
    }
  }

  cerror("variable '%s' was not found", name);
  exit(1);
}

static void setVariableValue(struct ExecEnv *e, const char *name, Value value)
{
  struct VariableList *p;

  for (p = e->vars; p != NULL; p = p->next){
    if (!strcmp(name, p->var->name)){
      if (p->var->type == TYPE_INTEGER)
        p->var->value = value;
    }
  }
}

static bool variableAlreadySet(struct ExecEnv *e, const char *name)
{
  struct VariableList *p;

  for (p = e->vars; p != NULL; p = p->next){
    if (!strcmp(name, p->var->name)){
      return true;
    }
  }

  return false;
}

static void onlyOut(const char *name)
{
  onlyName(name, "out", "function");
}

static Value execTermExpression(struct ExecEnv *e, struct Node *n)
{
  // TODO: refactor to an execNameExp and execVal functions
  assert(n);

  if (nt_INTEGER == n->kind){
    return n->data.value;
  } else {
    if (nt_ID == n->kind){
      assert(e);
      return getVariableValue(e, n->data.s);
    } else {
      cerror("ough: tried to get the value of a non-expression(%d)", n->kind);
      exit(1);
     }
  }
}

static Value execBinExpression(struct ExecEnv *e, struct Node *n)
{
  assert(nt_BINARYOP == n->kind);

  const Value left = dispatchExpression(e, n->data.expression.left);
  const Value right = dispatchExpression(e, n->data.expression.right);
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

static Value execDeclaration(struct ExecEnv *e, struct Node *n)
{
  assert(n);
  assert(nt_DECLARATION == n->kind);
  assert(e);

  if (variableAlreadySet(e, n->data.declaration.name)){
    cerror("variable '%s' already declared", n->data.declaration.name);
    exit(1);
  }

  struct VariableList *varlist = myalloc(sizeof(struct VariableList));
  struct Variable *var = myalloc(sizeof(struct Variable));

  struct Node *r = n->data.declaration.right;

  varscount++;

  varlist->var = var;
  varlist->var->type = n->data.declaration.type;
  varlist->var->name = n->data.declaration.name;

  if (n->data.declaration.right == NULL){
    varlist->var->value.i = 0;
  } else {
    varlist->var->value = dispatchExpression(e, r);
  }

  varlist->next = e->vars;
  e->vars = varlist;
}

static Value execAssignment(struct ExecEnv *e, struct Node *n)
{
  assert(n);
  assert(nt_ASSIGNMENT == n->kind);
  assert(e);

  Value val;

  if (!variableAlreadySet(e, n->data.s)){
    cerror("tried to change value of variable '%s' without declaring it first", n->data.s);
    exit(1);
  }

  struct Node *r = n->data.assignment.right;

  setVariableValue(e, n->data.s, dispatchExpression(e, r));

  val.i = 0;

  return val;
}

static void execStatement(struct ExecEnv *e, struct Node *n)
{
  assert(n);
  assert(nt_STATEMENTS == n->kind);

  for (int i = 0; i < n->data.statements.count; i++){
    dispatchExpression(e, n->data.statements.statements[i]);
  }
}

static Value execCall(struct ExecEnv *e, struct Node *n)
{
  assert(n);
  assert(nt_CALL == n->kind);

  Value val;

  onlyOut(n->data.call.name);
  printf("%d\n", dispatchExpression(e, n->data.call.param).i);

  val.i = 0;

  return val;
}

void execNodes(struct ExecEnv *e, struct Node *n)
{
  dispatchStatement(e, n);
}

static Value execWhilst(struct ExecEnv *e, struct Node *n)
{
  assert(n);
  assert(nt_WHILST == n->kind);

  Value val;

  struct Node * const c = n->data.whilst.cond;
  struct Node * const s = n->data.whilst.statements;

  assert(c);
  assert(s);

  while (dispatchExpression(e, c).i){
    dispatchStatement(e, s);
  }

  val.i = 0;

  return val;
}

static Value execAn(struct ExecEnv *e, struct Node *n)
{
  assert(n);
  assert(nt_AN == n->kind);

  Value val;

  struct Node * const c = n->data.an.cond;
  struct Node * const s = n->data.an.statements;

  assert(c);
  assert(s);

  if (dispatchExpression(e, c).i){
    dispatchStatement(e, s);
  }

  val.i = 0;

  return val;
}

struct ExecEnv *createEnv(void)
{
  assert(nt_LASTELEMENT == (ARRAY_SIZE(valExecs)));
  assert(nt_LASTELEMENT == (ARRAY_SIZE(runExecs)));

  struct ExecEnv *new = calloc(1, sizeof(struct ExecEnv));

  new->vars = NULL;
  
  return new;
}

void freeEnv(struct ExecEnv *e)
{
  free(e);
}

