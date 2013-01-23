//
// vars.h
//
// Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
//

#ifndef VARS_H
#define VARS_H

#include "nemo.h"
#include "nodes.h"
#include "exec.h"

struct Variable {
  Value value;
  char *name;
  Type type;
  // used for read-only variables
  bool first_use;
};

struct VariableList {
  struct Variable *var;
  struct VariableList *next;
};

Value getVariableValue(const char *, struct Node *);
void setVariableValue(const char *, Value, struct Node *);
bool variableAlreadySet(const char *, struct Node *);
void addVariableToBlock(char *, struct Node *);

#endif // VARS_H
