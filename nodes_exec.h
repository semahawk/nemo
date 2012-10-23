/*
 * nodes_exec.h
 *
 * Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
 *
 */

#ifndef NODES_EXEC_H
#define NODES_EXEC_H

struct Node;
struct ExecEnv;

// creates the execution engine
struct ExecEnv *createEnv(void);

// removes the ExecEnviron
void freeEnv(struct ExecEnv *);

// executes the nodes
void execNodes(struct ExecEnv *, struct Node *);

#endif // NODES_EXEC_H
