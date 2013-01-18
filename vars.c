//
// vars.c
//
// Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
//

#include "nemo.h"
#include "handy.h"
#include "vars.h"

Value getVariableValue(const char *name, struct Node *block)
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

void setVariableValue(const char *name, Value value, struct Node *block)
{
  struct Node *b;
  struct VariableList *p;

  for (b = block; b != NULL; b = b->data.block.parent){
    for (p = b->data.block.vars; p != NULL; p = p->next){
      if (!strcmp(name, p->var->name)){
        // !read-only variable
        // you set it only on the first use
        if (!strncmp(name, "!", 1)){
          // if that's it's first use, we can just change the value
          if (p->var->first_use){
            p->var->value = value;
            p->var->first_use = false;
            return;
          } else {
            cerror("cannot change value of a read-only variable '%s'", name);
            exit(1);
          }
        // just a regular variable
        } else {
          p->var->value = value;
          return;
        }
      }
    }
  }
}

bool variableAlreadySet(const char *name, struct Node *block)
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

void addVariableToBlock(char *name, struct Node *block)
{
  struct VariableList *varlist = myalloc(sizeof(struct VariableList));
  struct Variable *var = myalloc(sizeof(struct Variable));

  varlist->var = var;
  varlist->var->type = TYPE_INTEGER;
  varlist->var->name = name;
  varlist->var->first_use = true;

  varlist->next = block->data.block.vars;
  block->data.block.vars = varlist;
}

