/*
 * nodes_exec.h
 *
 * Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
 *
 */

#ifndef NODES_EXEC_H
#define NODES_EXEC_H

struct Node;

// executes the nodes
void execNodes(struct Node *);
// frees the nodes
void freeNodes(struct Node *);

#endif // NODES_EXEC_H
