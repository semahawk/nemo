/*
 * grammar.y
 *
 * Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
 *
 */

%output "y.tab.c"

%{

  #include "nemo.h"
  #include "nodes.h"
  #include "gen.h"
  #include "handy.h"

  #define YYERROR_VERBOSE

  void yyerror(const char *);
  int yylex();

  extern int linenum;
  extern int column;

  extern char source[255];

  extern struct Node *nodest;
  struct Node *currentblock;
  struct Node *funcdef = NULL;
  int argcount = 0, paramcount = 0;

%}

%union {
  int i;
  float f;
  char *s;
  char op;
  int type;
  struct Node *node;
  struct ArgList *arglist;
  struct ParamList *paramlist;
  Binary binary;
}

%token <i> INTEGER
%token <f> FLOAT
%token <s> VAR_IDENT IDENT
%type <node> stmts stmt
%type <node> expr_stmt iter_stmt select_stmt comp_stmt funcdef_stmt return_stmt
%type <node> expr assign_expr equ_expr cond_expr add_expr mult_expr prefix_expr postfix_expr primary_expr
%type <binary> assign_op add_op mult_op cond_op equ_op unary_op
%type <arglist> arg_list
%type <paramlist> param_list

%token WHILE IF ELSE FOR NONE RETURN
%token ASSIGN FUN TIMES
%token GE LE EQ NE
%token PLUSPLUS MINUSMINUS

%right ASSIGN EQ_ADD EQ_SUB EQ_MUL EQ_DIV EQ_MOD
%left  EQ NE
%left  GT LT GE LE
%left  '+' '-'
%left  '*' '/' '%'
%nonassoc PLUSPLUS MINUSMINUS
%left  UPLUS UMINUS
%left  '('

%nonassoc LOWERTHANELSE
%nonassoc ELSE

%start source

%%

source
    : { currentblock = nodest = genEmptyBlock(NULL, NULL); } stmts
    ;

stmts
    : /* empty */          { $$ = 0; }
    | stmts stmt           { blockappend(currentblock, $2);   }
    ;

stmt
    : expr_stmt            { $$ = $1; }
    | iter_stmt            { $$ = $1; }
    | select_stmt          { $$ = $1; }
    | comp_stmt            { $$ = $1; }
    | funcdef_stmt         { $$ = $1; }
    | return_stmt          { $$ = $1; }
    ;

comp_stmt
    : '{' { $<node>$ = currentblock; currentblock = genEmptyBlock(currentblock, funcdef); } stmts '}'
          { $<node>$ = currentblock; currentblock = $<node>2; }
    ;

funcdef_stmt
    : FUN IDENT ';' arg_list
      { $<node>$ = funcdef; funcdef = genFuncDef(TYPE_INTEGER, $2, $4, argcount); argcount = 0; }
      comp_stmt
      { $<node>$ = funcdef; funcdef->data.funcdef.body = $<node>6; funcdef = NULL; }
    ;

iter_stmt
    : WHILE expr stmt                     { $$ = genWhile($2, $3); }
    | FOR expr_stmt expr_stmt expr stmt   { $$ = genFor($2, $3, $4, $5, currentblock); }
    | FOR expr_stmt expr_stmt stmt        { $$ = genFor($2, $3, NULL, $4, currentblock); }
    | expr TIMES  stmt                    { $$ = genIter("times", $1, $3, currentblock); }
    ;

select_stmt
    : IF expr stmt %prec LOWERTHANELSE  { $$ = genIf($2, $3, NULL); }
    | IF expr stmt ELSE stmt            { $$ = genIf($2, $3, $5); }
    ;

return_stmt
    : RETURN expr ';'                { $$ = genReturn($2); }
    | RETURN ';'                     { $$ = genReturn(NULL); }
    ;

expr_stmt
    : ';'      { $$ = genNoop();  }
    | expr ';' { $$ = $1; }
    ;

expr
    : assign_expr
    | VAR_IDENT ASSIGN expr { $$ = genAssignment($1, $3, currentblock); }
    ;

assign_op
    : EQ_ADD  { $$ = BINARY_EQ_ADD; }
    | EQ_SUB  { $$ = BINARY_EQ_SUB; }
    | EQ_MUL  { $$ = BINARY_EQ_MUL; }
    | EQ_DIV  { $$ = BINARY_EQ_DIV; }
    | EQ_MOD  { $$ = BINARY_EQ_MOD; }
    ;

assign_expr
    : equ_expr
    | expr assign_op equ_expr { $$ = genBinaryop($1, $3, $2, currentblock); }
    ;

equ_op
    : NE      { $$ = BINARY_NE; }
    | EQ      { $$ = BINARY_EQ; }
    ;

equ_expr
    : cond_expr
    | equ_expr equ_op cond_expr { $$ = genBinaryop($1, $3, $2, currentblock); }
    ;

cond_op
    : GT      { $$ = BINARY_GT; }
    | LT      { $$ = BINARY_LT; }
    | GE      { $$ = BINARY_GE; }
    | LE      { $$ = BINARY_LE; }
    ;

cond_expr
    : add_expr
    | cond_expr cond_op add_expr { $$ = genBinaryop($1, $3, $2, currentblock); }
    ;

add_op
    : '+'     { $$ = BINARY_ADD; }
    | '-'     { $$ = BINARY_SUB; }
    ;

add_expr
    : mult_expr
    | add_expr add_op mult_expr { $$ = genBinaryop($1, $3, $2, currentblock); }
    ;

mult_op
    : '*'     { $$ = BINARY_MUL; }
    | '/'     { $$ = BINARY_DIV; }
    | '%'     { $$ = BINARY_MOD; }
    ;

mult_expr
    : prefix_expr
    | mult_expr mult_op prefix_expr { $$ = genBinaryop($1, $3, $2, currentblock); }
    ;

unary_op
    : '+'     { $$ = UNARY_PLUS; }
    | '-'     { $$ = UNARY_MINUS; }
    ;

prefix_expr
    : postfix_expr
    | PLUSPLUS prefix_expr { $$ = genUnaryop($2, UNARY_PREINC, currentblock); }
    | MINUSMINUS prefix_expr { $$ = genUnaryop($2, UNARY_PREDEC, currentblock); }
    | unary_op prefix_expr { $$ = genUnaryop($2, $1, currentblock); }
    ;

postfix_expr
    : primary_expr
    | IDENT '(' param_list ')' { $$ = genCall($1, $3, paramcount); paramcount = 0; }
    | postfix_expr PLUSPLUS    { $$ = genUnaryop($1, UNARY_POSTINC, currentblock); }
    | postfix_expr MINUSMINUS  { $$ = genUnaryop($1, UNARY_POSTDEC, currentblock); }
    ;

primary_expr
    : VAR_IDENT        { $$ = genExpByName($1, currentblock); }
    | INTEGER          { $$ = genExpByInt($1); }
    | FLOAT            { $$ = genExpByFloat($1); }
    | '(' expr ')'     { $$ = $2; }
    ;

arg_list
    : NONE                         { $$ = NULL; argcount = 0; }
    | VAR_IDENT                    { $$ = genArgList(TYPE_INTEGER, $1, NULL, argcount); argcount = 1; }
    | arg_list ',' VAR_IDENT       { $$ = genArgList(TYPE_INTEGER, $3, $1, argcount); argcount++;   }
    ;

param_list
    : /* empty */                  { $$ = NULL; paramcount = 0; }
    | expr                         { $$ = genParamList($1, NULL, paramcount); paramcount = 1; }
    | param_list ',' expr          { $$ = genParamList($3, $1, paramcount); paramcount++; }
    ;

%%

void yyerror(const char *s)
{
  fflush(stdout);
  printf("%s:%d:%d: %s", source, linenum, column, s);
}

