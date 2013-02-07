/*
 * grammar.y
 *
 * Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
 *
 */

%token_type { YYSTYPE }

%extra_argument { struct Context *context }

%right ASSIGN EQ_ADD EQ_SUB EQ_MUL EQ_DIV EQ_MOD EQ_DOT.
%left EQ NE STR_EQ STR_NE.
%left GT LT GE LE STR_GT STR_GE STR_LT STR_LE.
%left ADD SUB DOT.
%left MUL DIV MOD.
%nonassoc PLUSPLUS MINUSMINUS.
%left UPLUS UMINUS.
%left LPAREN.

// dangling else
%nonassoc IF.
%nonassoc ELSE.

%type source { struct Node * }
%type gen_main_block { struct Node * }
%type stmts { struct Node * }
%type stmt { struct Node * }
%type comp_stmt { struct Node * }
%type change_comp_stmt_block { struct Node * }
%type gen_empty_iter { struct Node * }
%type iter_stmt { struct Node * }
%type select_stmt { struct Node * }
%type return_stmt { struct Node * }
%type funcdef_stmt { struct Node * }
%type gen_empty_funcdef { struct Node * }
%type use_stmt { struct Node * }
%type expr_stmt { struct Node * }
%type expr { struct Node * }
%type arg_list { struct ArgList * }
%type param_list { struct ParamList * }
%type INTEGER { YYSTYPE }
%type FLOATING { YYSTYPE }
%type STRING { YYSTYPE }
%type VAR_IDENT { YYSTYPE }

%include {

  #include "nemo.h"
  #include "nodes.h"
  #include "gen.h"
  #include "exec.h"
  #include "cast.h"
  #include "handy.h"
  #include "vars.h"
  #include "grammar.h"
  #include "yystype.h"

  struct Node *block = NULL;
  struct Node *funcdef = NULL;
  struct Node *iter = NULL;
  int argcount = 0, paramcount = 0;

}

%parse_accept {
  debug("", "parsing completed successfuly!");
}

%syntax_error {
  fprintf(stderr, "syntax error!\n");
  context->nodest = NULL;
}

source ::= gen_main_block(A) stmts . { void **p = context->nodest; *p = A; }
gen_main_block(A) ::= . { A = genEmptyBlock(NULL, block); block = A; }


stmts(A) ::= /* empty */ . { A = NULL; }
stmts ::= stmts stmt(stmt) . { appendToBlock(block, stmt); }


stmt(A) ::= comp_stmt(B) .    { A = B; }
stmt(A) ::= iter_stmt(B) .    { A = B; }
stmt(A) ::= select_stmt(B) .  { A = B; }
stmt(A) ::= funcdef_stmt(B) . { A = B; }
stmt(A) ::= expr_stmt(B) .    { A = B; }
stmt(A) ::= return_stmt(B) .  { A = B; }
stmt(A) ::= use_stmt(B) .     { A = B; }


use_stmt(A) ::= USE DQ_STRING(fname) SEMICOLON .
{
  // TODO: prevent from 'use'-ing the same file, or any other kind of infinitness
  void *tmp = block;
  block = A;
  A = parseFile(vtos(dispatchNode(genExpByDoubleString(fname.s, block))));
  block = tmp;
}
use_stmt(A) ::= USE SQ_STRING(fname) SEMICOLON .
{
  void *tmp = block;
  block = A;
  // we don't exec it and anything, because that is a best as it could get
  A = parseFile(fname.s);
  block = tmp;
}


change_comp_stmt_block(A) ::= .
                 { A = block; block = genEmptyBlock(block, funcdef); }
comp_stmt(A) ::= LMUSTASHE change_comp_stmt_block(B) stmts RMUSTASHE .
                 { A = block; block = B; }


iter_stmt(A) ::= FOR expr_stmt(init) expr_stmt(guard) expr(action) stmt(stmt) .
                 { A = genFor(init, guard, action, stmt, block); }
iter_stmt(A) ::= FOR expr_stmt(init) expr_stmt(guard) stmt(stmt) .
                 { A = genFor(init, guard, NULL, stmt, block); }
iter_stmt(A) ::= WHILE expr(expr) stmt(stmt) .
                 { A = genWhile(expr, stmt); }
gen_empty_iter(B) ::= expr(expr) TIMES .
                      { B = iter; iter = genIter("times", expr, block); }
iter_stmt(A) ::= gen_empty_iter stmt(B) .
                 { A = iter; iter->data.iter.stmt = B; iter = NULL; }


select_stmt(A) ::= IF expr(expr) stmt(stmt) .
                   { A = genIf(expr, stmt, NULL); }
select_stmt(A) ::= IF expr(expr) stmt(stmt) ELSE stmt(else_stmt) .
                   { A = genIf(expr, stmt, else_stmt); }


gen_empty_funcdef(B) ::= IDENT(name) arg_list(args) .
                         { B = funcdef; funcdef = genFuncDef(TYPE_INTEGER, name.s, args, argcount); argcount = 0; }
funcdef_stmt(A) ::= FUN gen_empty_funcdef comp_stmt(B) .
                    { A = funcdef; funcdef->data.funcdef.body = B; funcdef = NULL; }


return_stmt(A) ::= RETURN SEMICOLON .
                   { A = genReturn(NULL); }
return_stmt(A) ::= RETURN expr(expr) SEMICOLON .
                   { A = genReturn(expr); }


expr_stmt(A) ::= expr(expr) SEMICOLON .
                 { A = expr; }
expr_stmt(A) ::= SEMICOLON .
                 { A = genNoop(); }


expr(A) ::= IDENT(B) LPAREN param_list(C) RPAREN .
            { A = genCall(genExpBySingleString(B.s), C, paramcount); paramcount = 0; }
expr(A) ::= VAR_IDENT(B) LPAREN param_list(C) RPAREN .
            { A = genCall(genExpByName(B.s, block), C, paramcount); paramcount = 0; }
expr(A) ::= DQ_STRING(B) LPAREN param_list(C) RPAREN .
            { A = genCall(genExpByDoubleString(B.s, block), C, paramcount); paramcount = 0; }
expr(A) ::= SQ_STRING(B) LPAREN param_list(C) RPAREN .
            { A = genCall(genExpBySingleString(B.s), C, paramcount); paramcount = 0; }

expr(A) ::= LPAREN expr(expr) RPAREN .
            { A = expr; }
expr(A) ::= INTEGER(val) .
            { A = genExpByInt(val.i); }
expr(A) ::= FLOATING(val) .
            { A = genExpByFloat(val.f); }
expr(A) ::= DQ_STRING(val) .
            { A = genExpByDoubleString(val.s, block); }
expr(A) ::= SQ_STRING(val) .
            { A = genExpBySingleString(val.s); }
expr(A) ::= VAR_IDENT(val) .
            { A = genExpByName(val.s, block); }

expr(A) ::= VAR_IDENT(left) ASSIGN expr(right) .
            { A = genAssignment(left.s, right, block); }
expr(A) ::= expr(left) ADD expr(right) .
            { A = genBinaryop(left, right, BINARY_ADD, block); }
expr(A) ::= expr(left) SUB expr(right) .
            { A = genBinaryop(left, right, BINARY_SUB, block); }
expr(A) ::= expr(left) MUL expr(right) .
            { A = genBinaryop(left, right, BINARY_MUL, block); }
expr(A) ::= expr(left) DIV expr(right) .
            { A = genBinaryop(left, right, BINARY_DIV, block); }
expr(A) ::= expr(left) MOD expr(right) .
            { A = genBinaryop(left, right, BINARY_MOD, block); }
expr(A) ::= expr(left) EQ_ADD expr(right) .
            { A = genBinaryop(left, right, BINARY_EQ_ADD, block); }
expr(A) ::= expr(left) EQ_SUB expr(right) .
            { A = genBinaryop(left, right, BINARY_EQ_SUB, block); }
expr(A) ::= expr(left) EQ_MUL expr(right) .
            { A = genBinaryop(left, right, BINARY_EQ_MUL, block); }
expr(A) ::= expr(left) EQ_DIV expr(right) .
            { A = genBinaryop(left, right, BINARY_EQ_DIV, block); }
expr(A) ::= expr(left) EQ_MOD expr(right) .
            { A = genBinaryop(left, right, BINARY_EQ_MOD, block); }
expr(A) ::= expr(left) EQ_DOT expr(right) .
            { A = genBinaryop(left, right, BINARY_EQ_CON, block); }
expr(A) ::= expr(left) DOT expr(right) .
            { A = genBinaryop(left, right, BINARY_CON, block); }
expr(A) ::= expr(left) NE expr(right) .
            { A = genBinaryop(left, right, BINARY_NE, block); }
expr(A) ::= expr(left) EQ expr(right) .
            { A = genBinaryop(left, right, BINARY_EQ, block); }
expr(A) ::= expr(left) GT expr(right) .
            { A = genBinaryop(left, right, BINARY_GT, block); }
expr(A) ::= expr(left) LT expr(right) .
            { A = genBinaryop(left, right, BINARY_LT, block); }
expr(A) ::= expr(left) GE expr(right) .
            { A = genBinaryop(left, right, BINARY_GE, block); }
expr(A) ::= expr(left) LE expr(right) .
            { A = genBinaryop(left, right, BINARY_LE, block); }
expr(A) ::= expr(left) STR_NE expr(right) .
            { A = genBinaryop(left, right, BINARY_STR_NE, block); }
expr(A) ::= expr(left) STR_EQ expr(right) .
            { A = genBinaryop(left, right, BINARY_STR_EQ, block); }
expr(A) ::= expr(left) STR_GT expr(right) .
            { A = genBinaryop(left, right, BINARY_STR_GT, block); }
expr(A) ::= expr(left) STR_LT expr(right) .
            { A = genBinaryop(left, right, BINARY_STR_LT, block); }
expr(A) ::= expr(left) STR_GE expr(right) .
            { A = genBinaryop(left, right, BINARY_STR_GE, block); }
expr(A) ::= expr(left) STR_LE expr(right) .
            { A = genBinaryop(left, right, BINARY_STR_LE, block); }
expr(A) ::= ADD expr(right) . [UPLUS]
            { A = genUnaryop(right, UNARY_PLUS, block); }
expr(A) ::= SUB expr(right) . [UMINUS]
            { A = genUnaryop(right, UNARY_MINUS, block); }
expr(A) ::= expr(left) PLUSPLUS .
            { A = genUnaryop(left, UNARY_POSTINC, block); }
expr(A) ::= expr(left) MINUSMINUS .
            { A = genUnaryop(left, UNARY_POSTDEC, block); }
expr(A) ::= PLUSPLUS expr(right) .
            { A = genUnaryop(right, UNARY_PREINC, block); }
expr(A) ::= MINUSMINUS expr(right) .
            { A = genUnaryop(right, UNARY_PREDEC, block); }


arg_list(A) ::= VAR_IDENT(val) .
                { A = genArgList(TYPE_INTEGER, val.s, NULL, argcount); argcount = 1; }
arg_list(A) ::= arg_list(list) COMMA VAR_IDENT(val) .
                { A = genArgList(TYPE_INTEGER, val.s, list, argcount); argcount++; }
arg_list(A) ::= .
                { A = NULL; argcount = 0; }


param_list(A) ::= expr(B) .
                  { A = genParamList(B, NULL, paramcount); paramcount = 1; }
param_list(A) ::= param_list(B) COMMA expr(C) .
                  { A = genParamList(C, B, paramcount); paramcount++; }
param_list(A) ::= .
                  { A = NULL; paramcount = 0; }

