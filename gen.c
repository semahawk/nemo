//
// gen.c
//
// Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
//

#include "nemo.h"
#include "handy.h"
#include "gen.h"
#include "vars.h"
#include "userdef.h"

struct Node *genAssignment(char *name, struct Node *val, struct Node *block)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("create", "assignment node <name: %s> at %p", name, new);

  if (!variableAlreadySet(name, block)){
    addVariableToBlock(name, block);
  }

  new->kind = nt_ASSIGNMENT;
  new->data.assignment.name = name;
  new->data.assignment.right = val;
  new->block = block;

  return new;
}

struct Node *genExpByInt(int val)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("create", "integer node <val: %d> at %p", val, new);

  new->kind = nt_INTEGER;
  new->data.value.type = TYPE_INTEGER;
  new->data.value.v.i = val;
  new->block = NULL;

  return new;
}

struct Node *genExpByFloat(float val)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("create", "float node <val: %f> at %p", val, new);

  new->kind = nt_FLOATING;
  new->data.value.type = TYPE_FLOATING;
  new->data.value.v.f = val;
  new->block = NULL;

  return new;
}

struct Node *genExpByName(char *name, struct Node *block)
{
  struct Node *new = myalloc(sizeof(struct Node));
  bool found = false;

  debug("create", "identifier node <name: %s> at %p", name, new);

  if (!variableAlreadySet(name, block)){
    // that variable is in a function, so let's check if one of arguments match
    // it
    if (block->data.block.funcdef){
      for (struct ArgList *a = block->data.block.funcdef->data.funcdef.args; a != NULL; a = a->next){
        // FOUND
        if (!strcmp(name, a->arg->name)){
          found = true;
        }
      }
    }

    if (!found){
      cerror("variable '%s' was not found", name);
      exit(1);
    }
  }

  new->kind = nt_ID;
  new->data.s = name;
  new->block = block;

  return new;
}

struct Node *genBinaryop(struct Node *left, struct Node *right, Binary op, struct Node *block)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("create", "binary operation node <op: '%c'> at %p", op, new);

  new->kind = nt_BINARYOP;
  new->data.binaryop.left = left;
  new->data.binaryop.right = right;
  new->data.binaryop.op = op;
  new->block = block;

  return new;
}

struct Node *genUnaryop(struct Node *left, Unary op, struct Node *currentblock)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("create", "unary operation node <op: '%s'> at %p", unarytos(op), new);

  if (left->kind != nt_ID){
    cerror("trying to change value of a constant object");
    exit(1);
  }

  new->kind = nt_UNARYOP;
  new->data.unaryop.expression = left;
  new->data.unaryop.op = op;
  new->block = currentblock;

  return new;
}

struct Node *genEmptyBlock(struct Node *parent, struct Node *funcdef)
{
  struct Node *new = myalloc(sizeof(struct Node));

  new->kind = nt_BLOCK;
  new->data.block.count = 0;
  new->data.block.statements = 0;
  new->data.block.vars = NULL;
  new->data.block.parent = parent;
  new->data.block.funcdef = funcdef;
  new->block = NULL;

  debug("create", "empty block node at %p with parent at %p", new, parent);

  return new;
}

void blockappend(struct Node *currentblock, struct Node *toappend)
{
  currentblock->data.block.count++;
  currentblock->data.block.statements = realloc(currentblock->data.block.statements, currentblock->data.block.count * sizeof(*currentblock->data.block.statements));
  currentblock->data.block.statements[currentblock->data.block.count - 1] = toappend;

  debug("append", "statement (%p) to block node (%p)", toappend, currentblock);
}

struct Node *genStatement(struct Node *new, struct Node *toappend)
{
  if (!new){
    new = myalloc(sizeof(struct Node));

    new->kind = nt_STATEMENT;
    new->data.statement.count = 0;
    new->data.statement.nodes = 0;
  }

  debug("create", "statement node at %p", new);
  assert(nt_STATEMENT == new->kind);

  new->data.statement.count++;
  new->data.statement.nodes = realloc(new->data.statement.nodes, new->data.statement.count * sizeof(*new->data.statement.nodes));
  new->data.statement.nodes[new->data.statement.count - 1] = toappend;

  return new;
}

struct Node *genWhile(struct Node *cond, struct Node *stmt)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("create", "while node at %p", new);

  new->kind = nt_WHILE;
  new->data.whilee.cond = cond;
  new->data.whilee.statements = stmt;
  new->block = NULL;

  return new;
}

struct Node *genIf(struct Node *cond, struct Node *stmt, struct Node *elsestmt)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("create", "if node at %p", new);

  new->kind = nt_IF;
  new->data.iff.cond = cond;
  new->data.iff.stmt = stmt;
  new->data.iff.elsestmt = elsestmt;
  new->block = NULL;

  return new;
}

struct Node *genFor(struct Node *init, struct Node *cond, struct Node *action, struct Node *stmt, struct Node *block)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("create", "for node at %p", new);

  new->kind = nt_FOR;
  new->data.forr.init = init;
  new->data.forr.cond = cond;
  new->data.forr.action = action;
  new->data.forr.stmt = stmt;
  new->block = block;

  return new;
}

struct Node *genFuncDef(Type returntype, char *name, struct ArgList *args, int argcount)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("create", "function definiton <name: %s> at %p", name, new);

  new->kind = nt_FUNCDEF;
  new->data.funcdef.returntype = returntype;
  new->data.funcdef.name = name;
  new->data.funcdef.body = NULL;
  new->data.funcdef.args = args;
  new->data.funcdef.argcount = argcount;

  return new;
}

struct ArgList *genArgList(Type type, char *name, struct ArgList *head, int pos)
{
  struct ArgList *arglist = myalloc(sizeof(struct ArgList));
  struct Arg *arg = myalloc(sizeof(struct Arg));

  debug("create", "argument list at %p", arglist);

  arglist->arg = arg;
  arglist->pos = pos;
  arglist->arg->type = type;
  arglist->arg->name = name;
  arglist->next = head;
  head = arglist;

  return arglist;
}

struct ParamList *genParamList(struct Node *param, struct ParamList *head, int pos)
{
  struct ParamList *new = myalloc(sizeof(struct ParamList));

  debug("create", "parameter list at %p", new);

  new->param = param;
  new->pos = pos;
  new->next = head;
  head = new;

  return new;
}

struct Node *genCall(char *name, struct ParamList *params, int paramcount)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("create", "call node <name: %s> at %p", name, new);

  // reversing the params list
  struct ParamList *next;
  struct ParamList *current = params;
  struct ParamList *reversed = NULL;

  while (current != NULL){
    next = current->next;
    current->next = reversed;
    reversed = current;
    current = next;
  }

  new->kind = nt_CALL;
  new->data.call.name = name;
  new->data.call.params = reversed;
  new->data.call.paramcount = paramcount;
  new->block = NULL;

  return new;
}

struct Node *genReturn(struct Node *expr)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("create", "return node at %p", new);

  new->kind = nt_RETURN;
  new->data.returnn.expr = expr;
  new->block = NULL;

  return new;
}

struct Node *genIter(char *type, struct Node *count, struct Node *stmt, struct Node *block)
{
  struct Node *new = myalloc(sizeof(struct Node));

  debug("create", "iter node <type: '%s'> at %p", type, new);

  new->kind = nt_ITER;
  new->data.iter.count = count;
  new->data.iter.stmt = stmt;
  new->block = block;

  return new;
}
