//
// vars.h
//
// Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
//

#ifndef VARS_H
#define VARS_H

#include "nemo.h"
#include "nodes_gen.h"
#include "nodes_exec.h"

struct Variable {
  Value value;
  char *name;
  Type type;
};

struct VariableList {
  struct Variable *var;
  struct VariableList *next;
};

Value getVariableValue(const char *, struct Node *);
void setVariableValue(const char *, Value, struct Node *);
bool variableAlreadySet(const char *, struct Node *);

#endif // VARS_H