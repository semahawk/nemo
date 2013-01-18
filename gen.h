//
// gen.h
//
// Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
//

#ifndef GEN_H
#define GEN_H

#include "nemo.h"

struct Node *genDeclaration(Type, char *, struct Node *, struct Node *);
struct Node *genAssignment(char *, struct Node *, struct Node *);
struct Node *genEmptyBlock(struct Node *);
       void  blockappend(struct Node *, struct Node *);
struct Node *genBinaryop(struct Node *, struct Node *, char);
struct Node *genUnaryop(struct Node *, Unary, struct Node *);
struct Node *genCall(char *, struct ParamList *, int);
struct Node *genReturn(struct Node *);
struct Node *genWhile(struct Node *, struct Node *);
struct Node *genIf(struct Node *, struct Node *, struct Node *);
struct Node *genFor(struct Node *, struct Node *, struct Node *, struct Node *, struct Node *);
struct Node *genFuncDef(Type, char *, struct ArgList *, int, struct Node *);
struct Node *genExpByInt(int);
struct Node *genExpByFloat(float);
struct Node *genExpByName(char *, struct Node *);
struct Node *genIter(char *, struct Node *, struct Node *, struct Node *);

struct ArgList *genArgList(Type, char *, struct ArgList *, int);
struct ParamList *genParamList(struct Node *, struct ParamList *, int);

#endif // GEN_H
