/*
 *
 * ast.h
 *
 * Created at:  Wed Apr 17 20:12:19 2013 20:12:19
 *
 * Author:  Szymon Urbaś <szymon.urbas@aol.com>
 *
 * License: the MIT license
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

#ifndef AST_H
#define AST_H

#include "nemo.h"

enum NodeType {
  NT_NOP,
  NT_INTEGER,
  NT_FLOAT,
  NT_NAME,
  NT_BINOP,
  NT_UNOP,
  NT_IF,
  NT_WHILE,
  NT_DECL,
  NT_BLOCK,
  NT_CALL,
  NT_FUNCDEF
};

enum BinaryOp {
  BINARY_GT,
  BINARY_LT,
  BINARY_ADD,
  BINARY_SUB,
  BINARY_MUL,
  BINARY_DIV,
  BINARY_MOD,
  BINARY_ASSIGN_ADD,
  BINARY_ASSIGN_SUB,
  BINARY_ASSIGN_MUL,
  BINARY_ASSIGN_DIV,
  BINARY_ASSIGN_MOD
};

enum UnaryOp {
  UNARY_PLUS,
  UNARY_MINUS,
  UNARY_NEGATE,
  UNARY_PREINC,
  UNARY_PREDEC,
  UNARY_POSTINC,
  UNARY_POSTDEC
};

/*
 * handled as a doubly linked list so it's easy to manipulate them
 */
struct Statement {
  struct Node *stmt;
  struct Statement *next;
  struct Statement *prev;
};

struct Node {
  enum NodeType type;
  union {
    int i;
    float f;
    char *s;

    struct {
      enum BinaryOp op;
      struct Node *left;
      struct Node *right;
    } binop;

    struct {
      enum UnaryOp op;
      struct Node *expr;
    } unop;

    struct {
      struct Node *guard;
      struct Node *body;
      struct Node *elsee;
    } iff;

    struct {
      struct Node *guard;
      struct Node *body;
      struct Node *elsee;
    } whilee;

    struct {
      char *name;
      struct Node *value;
    } decl;

    struct {
      struct Statement *head;
      struct Statement *tail;
    } block;

    struct {
      char *name;
      struct Node **params;
    } call;

    struct {
      char *name;
      struct Node *body;
    } funcdef;
  } data;
};

typedef enum   BinaryOp  BinaryOp;
typedef enum   UnaryOp   UnaryOp;
typedef enum   NodeType  NodeType;
typedef struct Node      Node;
typedef struct Params    Params;
typedef struct Statement Statement;

Node *genIntNode(Nemo *, int);
Node *genFloatNode(Nemo *, float);
Node *genNameNode(Nemo *, char *);
Node *genBinopNode(Nemo *, Node *, BinaryOp, Node *);
Node *genUnopNode(Nemo *, Node *, UnaryOp);
Node *genIfNode(Nemo *, Node *, Node *, Node *);
Node *genWhileNode(Nemo *, Node *, Node *, Node *);
Node *genDeclNode(Nemo *, char *, Node *);
Node *genCallNode(Nemo *, char *, Node **);
Node *genFuncDefNode(Nemo *, char *, Node *);
Node *genNopNode(Nemo *);

Value execIntNode(Nemo *, Node *);
Value execFloatNode(Nemo *, Node *);
Value execNameNode(Nemo *, Node *);
Value execBinopNode(Nemo *, Node *);
Value execUnopNode(Nemo *, Node *);
Value execIfNode(Nemo *, Node *);
Value execWhileNode(Nemo *, Node *);
Value execDeclNode(Nemo *, Node *);
Value execCallNode(Nemo *, Node *);
Value execBlockNode(Nemo *, Node *);
Value execFuncDefNode(Nemo *, Node *);
Value execNopNode(Nemo *, Node *);
Value execNode(Nemo *, Node *);

void  freeIntNode(Nemo *, Node *);
void  freeFloatNode(Nemo *, Node *);
void  freeNameNode(Nemo *, Node *);
void  freeBinopNode(Nemo *, Node *);
void  freeUnopNode(Nemo *, Node *);
void  freeIfNode(Nemo *, Node *);
void  freeWhileNode(Nemo *, Node *);
void  freeDeclNode(Nemo *, Node *);
void  freeCallNode(Nemo *, Node *);
void  freeBlockNode(Nemo *, Node *);
void  freeNopNode(Nemo *, Node *);
void  freeFuncDefNode(Nemo *, Node *);
void  freeNode(Nemo *, Node *);

#endif /* AST_H */

