//
// free.h
//
// Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
//

#ifndef FREE_H
#define FREE_H

#include "nodes.h"

void freeNode(struct Node *);
void freeNodes(struct Node *);

void freeTermExpression(struct Node *);
void freeBinExpression(struct Node *);
void freeUnExpression(struct Node *);
void freeAssignment(struct Node *);
void freeCall(struct Node *);
void freeReturn(struct Node *);
void freeWhile(struct Node *);
void freeIf(struct Node *);
void freeFor(struct Node *);
void freeFuncDef(struct Node *);
void freeBlock(struct Node *);
void freeStatement(struct Node *);

#endif // FREE_H
