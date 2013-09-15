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

#include <stdint.h>

#include "nemo.h"
#include "object.h"

enum NodeType {
  NT_NOP,
  NT_INTEGER,
  NT_FLOAT,
  NT_STRING,
  NT_ARRAY,
  NT_NAME,
  NT_BINOP,
  NT_UNOP,
  NT_IF,
  NT_WHILE,
  NT_DECL,
  NT_CALL,
  NT_STMT,
  NT_BLOCK,
  NT_FUNCDEF,
  NT_USE
};

enum BinaryOp {
  BINARY_GT,
  BINARY_LT,
  BINARY_GE,
  BINARY_LE,
  BINARY_EQ,
  BINARY_NE,
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
  BINARY_ASSIGN_MOD,
  BINARY_INDEX,
  BINARY_COMMA
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

/* forward */
typedef struct Node Node;

#define NMNODE_HEAD \
  /* pointer to the next node to be executed */ \
  struct Node *next; \
  /* type of the node */ \
  enum NodeType type; \
  /* it's position in the code */ \
  Pos pos

struct Node {
  NMNODE_HEAD;
};

typedef struct {
  NMNODE_HEAD;
  int i;
} Node_Int;

typedef struct {
  NMNODE_HEAD;
  float f;
} Node_Float;

typedef struct {
  NMNODE_HEAD;
  char *s;
} Node_String;

typedef struct {
  NMNODE_HEAD;
  char *name;
  struct Namespace *namespace;
} Node_Name;

typedef struct {
  NMNODE_HEAD;
  size_t nmemb;
  struct Node **a;
} Node_Array;

typedef struct {
  NMNODE_HEAD;
  enum BinaryOp op;
  struct Node *left;
  struct Node *right;
} Node_Binop;

typedef struct {
  NMNODE_HEAD;
  enum UnaryOp op;
  struct Node *target;
} Node_Unop;

typedef struct {
  NMNODE_HEAD;
  char *name;
  uint8_t flags;
  struct Node *value;
} Node_Decl;

typedef struct {
  NMNODE_HEAD;
  char *name;
  struct Node **params;
  char *opts;
  struct Namespace *namespace;
} Node_Call;

typedef struct {
  NMNODE_HEAD;
  struct Node *guard;
  struct Node *body;
  struct Node *elsee;
  /* ifs and unlesses are pretty much identical, and this is what
   * distinguishes them */
  bool unless;
} Node_If;

typedef struct {
  NMNODE_HEAD;
  struct Node *guard;
  struct Node *body;
  struct Node *elsee;
  /* whiles and untils are pretty much identical, and this is what
   * distinguishes them */
  bool until;
} Node_While;

typedef struct {
  NMNODE_HEAD;
  char *name;
  struct Node *body;
  unsigned argc;
  unsigned optc;
  NmObjectType *argv;
  char *opts;
} Node_Funcdef;

typedef struct {
  NMNODE_HEAD;
  char *fname;
  char *custom_path;
} Node_Use;

typedef struct {
  NMNODE_HEAD;
  /* number of expressions */
  size_t nmemb;
  /* the expression the statement is holding */
  struct Node *expr;
} Node_Stmt;

typedef struct {
  NMNODE_HEAD;
  struct Statement *head;
  struct Statement *tail;
} Node_Block;

typedef enum   BinaryOp  BinaryOp;
typedef enum   UnaryOp   UnaryOp;
typedef enum   NodeType  NodeType;
typedef struct Statement Statement;

/* defined in namespace.h */
struct Namespace;

Node *nm_ast_gen_int(Pos, int);
Node *nm_ast_gen_float(Pos, float);
Node *nm_ast_gen_str(Pos, char *);
Node *nm_ast_gen_arr(Pos, Node **);
Node *nm_ast_gen_name(Pos, char *, struct Namespace *);
Node *nm_ast_gen_binop(Pos, Node *, BinaryOp, Node *);
Node *nm_ast_gen_unop(Pos, Node *, UnaryOp);
Node *nm_ast_gen_if(Pos, Node *, Node *, Node *, bool);
Node *nm_ast_gen_while(Pos, Node *, Node *, Node *, bool);
Node *nm_ast_gen_decl(Pos, char *, Node *, uint8_t);
Node *nm_ast_gen_call(Pos, char *, Node **, char *, struct Namespace *);
Node *nm_ast_gen_stmt(Pos, Node *);
Node *nm_ast_gen_funcdef(Pos, char *, Node *, unsigned, unsigned, NmObjectType *, char *);
Node *nm_ast_gen_use(Pos, char *);
Node *nm_ast_gen_nop(Pos);

NmObject *nm_ast_exec_int(Node *);
NmObject *nm_ast_exec_float(Node *);
NmObject *nm_ast_exec_str(Node *);
NmObject *nm_ast_exec_arr(Node *);
NmObject *nm_ast_exec_name(Node *);
NmObject *nm_ast_exec_binop(Node *);
NmObject *nm_ast_exec_unop(Node *);
NmObject *nm_ast_exec_if(Node *);
NmObject *nm_ast_exec_while(Node *);
NmObject *nm_ast_exec_decl(Node *);
NmObject *nm_ast_exec_call(Node *);
NmObject *nm_ast_exec_stmt(Node *);
NmObject *nm_ast_exec_block(Node *);
NmObject *nm_ast_exec_funcdef(Node *);
NmObject *nm_ast_exec_use(Node *);
NmObject *nm_ast_exec_nop(Node *);

void nm_ast_free_int(Node *);
void nm_ast_free_float(Node *);
void nm_ast_free_str(Node *);
void nm_ast_free_arr(Node *);
void nm_ast_free_name(Node *);
void nm_ast_free_binop(Node *);
void nm_ast_free_unop(Node *);
void nm_ast_free_if(Node *);
void nm_ast_free_while(Node *);
void nm_ast_free_decl(Node *);
void nm_ast_free_call(Node *);
void nm_ast_free_stmt(Node *);
void nm_ast_free_block(Node *);
void nm_ast_free_nop(Node *);
void nm_ast_free_funcdef(Node *);
void nm_ast_free_use(Node *);
void nm_ast_free(Node *);

const char *binopToS(BinaryOp);
const char *unopToS(UnaryOp);

void nm_ast_stmt_append_expr(Node *, Node *);

#endif /* AST_H */

