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
  int value;
  char *name;
  var_t type;
};

struct VariableList {
  struct Variable *var;
  struct VariableList *next;
};

struct ExecEnv {
  // pointer to our first element
  struct VariableList *vars;
};

static int execTermExpression(struct ExecEnv *, struct Node *);
static int execBinExpression(struct ExecEnv *, struct Node *);
static void execDeclaration(struct ExecEnv *, struct Node *);
static void execAssignment(struct ExecEnv *, struct Node *);
static void execStatement(struct ExecEnv *, struct Node *);
static void execCall(struct ExecEnv *, struct Node *);
static void execWhilst(struct ExecEnv *, struct Node *);
static void execAn(struct ExecEnv *, struct Node *);

static int(*valExecs[])(struct ExecEnv *, struct Node *) =
{
  execTermExpression,
  execTermExpression,
  execBinExpression,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

static void(*runExecs[])(struct ExecEnv *, struct Node *) =
{
  NULL, // ID and numbers are canonical and
  NULL, // don't need to be executed
  NULL, // so is not a binary op
  execDeclaration,
  execAssignment,
  execStatement,
  execCall,
  execWhilst,
  execAn
};

static int dispatchExpression(struct ExecEnv *e, struct Node *n)
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

static int getVariableValue(struct ExecEnv *e, const char *name)
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

static void setVariableValue(struct ExecEnv *e, const char *name, int value)
{
  struct VariableList *p;

  for (p = e->vars; p != NULL; p = p->next){
    if (!strcmp(name, p->var->name)){
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

static int execTermExpression(struct ExecEnv *e, struct Node *n)
{
  // TODO: refactor to an execNameExp and execVal functions
  assert(n);

  if (nt_INTEGER == n->kind){
    return n->data.i;
  } else {
    if (nt_ID == n->kind){
      assert(e);
      return getVariableValue(e, n->data.s);
    } else {
      cerror("ough: tried to get the value of a non-expression(%d)", n->kind);
      exit(1);
     }
  }

  return -1;
}

static int execBinExpression(struct ExecEnv *e, struct Node *n)
{
  assert(nt_BINARYOP == n->kind);

  const int left = dispatchExpression(e, n->data.expression.left);
  const int right = dispatchExpression(e, n->data.expression.right);

  switch (n->data.expression.op){
    case '+': return left + right;
    case '-': return left - right;
    case '*': return left * right;
    case '/': if (right == 0){
                cerror("zero division!");
                exit(1);
              } else {
                return left / right;
              }
    case '%': return left % right;
    case '>': return left > right;
    case '<': return left < right;

    default: cerror("unknown operator '%c'", n->data.expression.op);
             exit(1);
  }

  return -1;
}

static void execDeclaration(struct ExecEnv *e, struct Node *n)
{
  assert(n);
  assert(nt_DECLARATION == n->kind);
  assert(e);

  if (variableAlreadySet(e, n->data.declaration.name)){
    cerror("variable '%s' already declared");
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
    varlist->var->value = 0;
  } else {
    varlist->var->value = dispatchExpression(e, r);
  }

  varlist->next = e->vars;
  e->vars = varlist;
}

static void execAssignment(struct ExecEnv *e, struct Node *n)
{
  assert(n);
  assert(nt_ASSIGNMENT == n->kind);
  assert(e);

  if (!variableAlreadySet(e, n->data.s)){
    cerror("tried to change value of variable '%s' without declaring it first", n->data.s);
    exit(1);
  }

  struct Node *r = n->data.assignment.right;

  setVariableValue(e, n->data.s, dispatchExpression(e, r));
}

static void execStatement(struct ExecEnv *e, struct Node *n)
{
  assert(n);
  assert(nt_STATEMENTS == n->kind);

  for (int i = 0; i < n->data.statements.count; i++){
    dispatchStatement(e, n->data.statements.statements[i]);
  }
}

static void execCall(struct ExecEnv *e, struct Node *n)
{
  assert(n);
  assert(nt_CALL == n->kind);

  onlyOut(n->data.call.name);
  printf("%d\n", dispatchExpression(e, n->data.call.param));
}

void execNodes(struct ExecEnv *e, struct Node *n)
{
  dispatchStatement(e, n);
}

static void execWhilst(struct ExecEnv *e, struct Node *n)
{
  assert(n);
  assert(nt_WHILST == n->kind);

  struct Node * const c = n->data.whilst.cond;
  struct Node * const s = n->data.whilst.statements;

  assert(c);
  assert(s);

  while (dispatchExpression(e, c)){
    dispatchStatement(e, s);
  }
}

static void execAn(struct ExecEnv *e, struct Node *n)
{
  assert(n);
  assert(nt_AN == n->kind);

  struct Node * const c = n->data.an.cond;
  struct Node * const s = n->data.an.statements;

  assert(c);
  assert(s);

  if (dispatchExpression(e, c)){
    dispatchStatement(e, s);
  }
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

