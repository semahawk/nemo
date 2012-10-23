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

struct ExecEnv {
  // TODO: make there be moar variables
  //       that's the value of the 'x' variable
  int x;
};

static int execTermExpression(struct ExecEnv *, struct Node *);
static int execBinExpression(struct ExecEnv *, struct Node *);
static void execAssignment(struct ExecEnv *, struct Node *);
static void execStatement(struct ExecEnv *, struct Node *);
static void execExpression(struct ExecEnv *, struct Node *);
static void execCall(struct ExecEnv *, struct Node *);

static int(*valExecs[])(struct ExecEnv *, struct Node *) =
{
  execTermExpression,
  execTermExpression,
  execBinExpression,
  NULL,
  NULL,
  NULL
};

static void(*runExecs[])(struct ExecEnv *, struct Node *) =
{
  NULL, // ID and numbers are canonical and
  NULL, // don't need to be executed
  NULL, // so is not a binary op
  execAssignment,
  execStatement,
  execCall
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
    error("Nemo only knows the %s '%s', not %s ", kind, ref, name);
    exit(1);
  }
}

static void onlyX(const char *name)
{
  onlyName(name, "x", "variable");
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
      onlyX(n->data.s);
      assert(e);
      return e->x;
    } else {
      error("ough: tried to get the value of a non-expression(%d)", n->kind);
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
    case '%': return left % right;
    case '/': return left / right;
    case '>': return left > right;
    case '<': return left < right;

    default: error("unknown operator '%c'", n->data.expression.op);
             exit(1);
  }

  return -1;
}

static void execAssignment(struct ExecEnv *e, struct Node *n)
{
  assert(n);
  assert(nt_ASSIGNMENT == n->kind);

  onlyX(n->data.assignment.name);
  assert(e);

  struct Node *r = n->data.assignment.right;

  e->x = dispatchExpression(e, r);
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

struct ExecEnv *createEnv(void)
{
  assert(nt_LASTELEMENT == (ARRAY_SIZE(valExecs)));
  assert(nt_LASTELEMENT == (ARRAY_SIZE(runExecs)));

  return calloc(1, sizeof(struct ExecEnv));
}

void freeEnv(struct ExecEnv *e)
{
  free(e);
}

