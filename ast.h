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
#include "object.h"

enum NodeType {
  NT_NOP,
  NT_INTEGER,
  NT_FLOAT,
  NT_STRING,
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
  BINARY_ASSIGN,
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

Node *NmAST_GenInt(Context *, int);
Node *NmAST_GenFloat(Context *, float);
Node *NmAST_GenString(Context *, char *);
Node *NmAST_GenName(Context *, char *);
Node *NmAST_GenBinop(Context *, Node *, BinaryOp, Node *);
Node *NmAST_GenUnop(Context *, Node *, UnaryOp);
Node *NmAST_GenIf(Context *, Node *, Node *, Node *);
Node *NmAST_GenWhile(Context *, Node *, Node *, Node *);
Node *NmAST_GenDecl(Context *, char *, Node *);
Node *NmAST_GenCall(Context *, char *, Node **);
Node *NmAST_GenFuncDef(Context *, char *, Node *);
Node *NmAST_GenNop(Context *);

NmObject *NmAST_ExecInt(Context *, Node *);
NmObject *NmAST_ExecFloat(Context *, Node *);
NmObject *NmAST_ExecString(Context *, Node *);
NmObject *NmAST_ExecName(Context *, Node *);
NmObject *NmAST_ExecBinop(Context *, Node *);
NmObject *NmAST_ExecUnop(Context *, Node *);
NmObject *NmAST_ExecIf(Context *, Node *);
NmObject *NmAST_ExecWhile(Context *, Node *);
NmObject *NmAST_ExecDecl(Context *, Node *);
NmObject *NmAST_ExecCall(Context *, Node *);
NmObject *NmAST_ExecBlock(Context *, Node *);
NmObject *NmAST_ExecFuncDef(Context *, Node *);
NmObject *NmAST_ExecNop(Context *, Node *);
NmObject *NmAST_Exec(Context *, Node *);

void  NmAST_FreeInt(Node *);
void  NmAST_FreeFloat(Node *);
void  NmAST_FreeString(Node *);
void  NmAST_FreeName(Node *);
void  NmAST_FreeBinop(Node *);
void  NmAST_FreeUnop(Node *);
void  NmAST_FreeIf(Node *);
void  NmAST_FreeWhile(Node *);
void  NmAST_FreeDecl(Node *);
void  NmAST_FreeCall(Node *);
void  NmAST_FreeBlock(Node *);
void  NmAST_FreeNop(Node *);
void  NmAST_FreeFuncDef(Node *);
void  NmAST_Free(Node *);

#endif /* AST_H */

