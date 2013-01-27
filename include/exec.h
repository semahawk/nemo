/*
 * nodes_exec.h
 *
 * Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
 *
 */

#ifndef EXEC_H
#define EXEC_H

#include "nodes.h"

Value execID(struct Node *);
Value execConstant(struct Node *);
Value execBinExpression(struct Node *);
Value execUnExpression(struct Node *);
Value execAssignment(struct Node *);
Value execCall(struct Node *);
Value execReturn(struct Node *);
Value execWhile(struct Node *);
Value execIf(struct Node *);
Value execFor(struct Node *);
Value execBlock(struct Node *);
Value execStatement(struct Node *);
Value execFuncDef(struct Node *);
Value execIter(struct Node *);
Value execNoop(struct Node *);

Value dispatchNode(struct Node *);

Value execNodes(struct Node *);
void freeNodes(struct Node *);

#endif // EXEC_H
