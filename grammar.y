/*
 * grammar.y
 *
 * Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
 *
 */

%output "y.tab.c"

%{

  #include "nemo.h"
  #include "nodes_gen.h"
  #include "handy.h"

  #define YYERROR_VERBOSE

  void yyerror(const char *);
  int yylex();

  extern int linenum;
  extern int column;

  extern char source[255];

  extern struct Node *nodest;
  struct Node *currentblock;
  int argcount = 0, paramcount = 0;

%}

%union {
  int i;
  char *s;
  char op;
  int type;
  struct Node *node;
  struct ArgList *arglist;
  struct ParamList *paramlist;
}

%token <i> INTEGER
%token <s> VAR_IDENT IDENT
%type <node> stmts stmt
%type <node> expr_stmt iter_stmt select_stmt comp_stmt funcdef_stmt
%type <node> expr decl_expr init_expr assign_expr call_expr binary_expr unary_expr
%type <type> type
%type <arglist> arg_list
%type <paramlist> param_list

%token TYPE_INT
%token WHILE IF ELSE FOR NONE
%token PLUSPLUS MINUSMINUS

%right '='
%left  '<' '>'
%left  '+' '-'
%left  '*' '/' '%'
%nonassoc PLUSPLUS MINUSMINUS
%left  '('

%nonassoc LOWERTHANELSE
%nonassoc ELSE

%start source

%%

source
    : { currentblock = nodest = genEmptyBlock(NULL); } stmts
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
    | decl_expr       { $$ = $1; }
    | init_expr       { $$ = $1; }
    | VAR_IDENT       { $$ = genExpByName($1, currentblock); }
    | INTEGER         { $$ = genExpByNum($1); }
    | '(' expr ')'    { $$ = $2; }
    ;

comp_stmt
    : '{' '}'         { $$ = 0; }
    | '{' { $<node>$ = currentblock; currentblock = genEmptyBlock(currentblock); } stmts '}'
          { $<node>$ = currentblock; currentblock = $<node>2; }
    ;

funcdef_stmt
    : type IDENT ';' arg_list comp_stmt   { $$ = genFuncDef($1, $2, $4, argcount, $5); argcount = 0; }
    ;

arg_list
    : NONE                         { $$ = NULL; argcount = 0; }
    | type VAR_IDENT               { $$ = genArgList($1, $2, NULL, argcount); argcount = 1; }
    | arg_list ',' type VAR_IDENT  { $$ = genArgList($3, $4, $1, argcount); argcount++;   }
    ;

param_list
    : /* empty */                  { $$ = NULL; paramcount = 0; }
    | expr                         { $$ = genParamList($1, NULL, paramcount); paramcount = 1; }
    | param_list ',' expr          { $$ = genParamList($3, $1, paramcount); paramcount++; }
    ;

decl_expr
    : type VAR_IDENT                 { $$ = genDeclaration($1, $2, NULL, currentblock); }
    ;

init_expr
    : type VAR_IDENT  '=' expr       { $$ = genDeclaration($1, $2, $4, currentblock); }
    ;

assign_expr
    : VAR_IDENT  '=' expr            { $$ = genAssignment($1, $3, currentblock); }
    ;

call_expr
    : IDENT '(' param_list ')'       { $$ = genCall($1, $3, paramcount); paramcount = 0; }
    ;

iter_stmt
    : WHILE '(' expr ')' stmt                     { $$ = genWhile($3, $5); }
    | FOR '(' expr_stmt expr_stmt expr ')' stmt   { $$ = genFor($3, $4, $5, $7, currentblock); }
    | FOR '(' expr_stmt expr_stmt ')' stmt        { $$ = genFor($3, $4, NULL, $6, currentblock); }
    ;

select_stmt
    : IF '(' expr ')' stmt %prec LOWERTHANELSE  { $$ = genIf($3, $5, NULL); }
    | IF '(' expr ')' stmt ELSE stmt            { $$ = genIf($3, $5, $7); }
    ;

binary_expr
    : expr '+' expr    { $$ = genBinaryop($1, $3, '+'); }
    | expr '-' expr    { $$ = genBinaryop($1, $3, '-'); }
    | expr '*' expr    { $$ = genBinaryop($1, $3, '*'); }
    | expr '/' expr    { $$ = genBinaryop($1, $3, '/'); }
    | expr '%' expr    { $$ = genBinaryop($1, $3, '%'); }
    | expr '>' expr    { $$ = genBinaryop($1, $3, '>'); }
    | expr '<' expr    { $$ = genBinaryop($1, $3, '<'); }
    ;

unary_expr
    : expr PLUSPLUS    { $$ = genUnaryop($1, UNARY_POSTINC, currentblock); }
    | expr MINUSMINUS  { $$ = genUnaryop($1, UNARY_POSTDEC, currentblock); }
    | PLUSPLUS expr    { $$ = genUnaryop($2, UNARY_PREINC, currentblock); }
    | MINUSMINUS expr  { $$ = genUnaryop($2, UNARY_PREDEC, currentblock); }
    ;

type
    : TYPE_INT { $$ = TYPE_INTEGER; }
    ;

%%

void yyerror(const char *s)
{
  fflush(stdout);
  printf("%s:%d:%d: %s", source, linenum, column, s);
}

