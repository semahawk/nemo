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
}

%token <i> INTEGER
%token <f> FLOAT
%token <s> VAR_IDENT IDENT
%type <node> stmts stmt
%type <node> expr_stmt iter_stmt select_stmt comp_stmt funcdef_stmt return_stmt
%type <node> expr assign_expr call_expr binary_expr unary_expr constant
%type <arglist> arg_list
%type <paramlist> param_list

%token WHILE IF ELSE FOR NONE RETURN
%token FUN TIMES
%token GE LE EQ NE
%token PLUSPLUS MINUSMINUS

%right '=' EQ_ADD EQ_SUB EQ_MUL EQ_DIV EQ_MOD
%left  EQ NE
%left  GT LT GE LE
%left  '+' '-'
%left  '*' '/' '%'
%nonassoc PLUSPLUS MINUSMINUS
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

expr_stmt
    : ';'      { $$ = 0;  }
    | expr ';' { $$ = $1; }
    ;

expr
    : binary_expr     { $$ = $1; }
    | unary_expr      { $$ = $1; }
    | assign_expr     { $$ = $1; }
    | call_expr       { $$ = $1; }
    | VAR_IDENT       { $$ = genExpByName($1, currentblock); }
    | constant        { $$ = $1; }
    | '(' expr ')'    { $$ = $2; }
    ;

constant
    : INTEGER         { $$ = genExpByInt($1); }
    | FLOAT           { $$ = genExpByFloat($1); }
    ;

assign_expr
    : VAR_IDENT '=' expr             { $$ = genAssignment($1, $3, currentblock); }
    | constant  '=' expr             { cerror("cannot change value of a constant"); exit(1); }
    ;

call_expr
    : IDENT '(' param_list ')'       { $$ = genCall($1, $3, paramcount); paramcount = 0; }
    | IDENT     param_list           { $$ = genCall($1, $2, paramcount); paramcount = 0; }
    ;

return_stmt
    : RETURN expr                    { $$ = genReturn($2); }
    | RETURN                         { $$ = genReturn(NULL); }
    ;

binary_expr
    : expr '+' expr    { $$ = genBinaryop($1, $3, BINARY_ADD, currentblock); }
    | expr '-' expr    { $$ = genBinaryop($1, $3, BINARY_SUB, currentblock); }
    | expr '*' expr    { $$ = genBinaryop($1, $3, BINARY_MUL, currentblock); }
    | expr '/' expr    { $$ = genBinaryop($1, $3, BINARY_DIV, currentblock); }
    | expr '%' expr    { $$ = genBinaryop($1, $3, BINARY_MOD, currentblock); }
    | expr GT expr     { $$ = genBinaryop($1, $3, BINARY_GT, currentblock); }
    | expr LT expr     { $$ = genBinaryop($1, $3, BINARY_LT, currentblock); }
    | expr GE expr     { $$ = genBinaryop($1, $3, BINARY_GE, currentblock); }
    | expr LE expr     { $$ = genBinaryop($1, $3, BINARY_LE, currentblock); }
    | expr NE expr     { $$ = genBinaryop($1, $3, BINARY_NE, currentblock); }
    | expr EQ expr     { $$ = genBinaryop($1, $3, BINARY_EQ, currentblock); }
    | expr EQ_ADD expr { $$ = genBinaryop($1, $3, BINARY_EQ_ADD, currentblock); }
    | expr EQ_SUB expr { $$ = genBinaryop($1, $3, BINARY_EQ_SUB, currentblock); }
    | expr EQ_MUL expr { $$ = genBinaryop($1, $3, BINARY_EQ_MUL, currentblock); }
    | expr EQ_DIV expr { $$ = genBinaryop($1, $3, BINARY_EQ_DIV, currentblock); }
    | expr EQ_MOD expr { $$ = genBinaryop($1, $3, BINARY_EQ_MOD, currentblock); }
    ;

unary_expr
    : expr PLUSPLUS    { $$ = genUnaryop($1, UNARY_POSTINC, currentblock); }
    | expr MINUSMINUS  { $$ = genUnaryop($1, UNARY_POSTDEC, currentblock); }
    | PLUSPLUS expr    { $$ = genUnaryop($2, UNARY_PREINC, currentblock); }
    | MINUSMINUS expr  { $$ = genUnaryop($2, UNARY_PREDEC, currentblock); }
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

