/*
 * nodes_exec.h
 *
 * Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
 *
 */

#ifndef NODES_EXEC_H
#define NODES_EXEC_H

#include "nodes_gen.h"

Value execTermExpression(struct Node *);
Value execBinExpression(struct Node *);
Value execUnExpression(struct Node *);
Value execDeclaration(struct Node *);
Value execAssignment(struct Node *);
Value execCall(struct Node *);
Value execWhile(struct Node *);
Value execIf(struct Node *);
Value execBlock(struct Node *);
Value execStatement(struct Node *);
Value execFuncDef(struct Node *);

void execNodes(struct Node *);
void freeNodes(struct Node *);

#endif // NODES_EXEC_H
