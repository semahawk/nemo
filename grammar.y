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

%}

%union {
  int i;
  char *s;
  char op;
  int type;
  struct Node *node;
}

%token <i> INTEGER
%token <s> VAR_IDENT IDENT
%type <node> source stmts stmt
%type <node> expr_stmt iter_stmt select_stmt comp_stmt
%type <node> expr decl_expr init_expr assign_expr call_expr binary_expr
%type <type> type

%token TYPE_INT
%token WHILST AN

%right '='
%left  '<' '>'
%left  '+' '-'
%left  '*' '/' '%'

%start source

%%

source
    : stmts                { nodest = $1; }
    ;

stmts
    : stmt                 { $$ = block(NULL, $1); }
    | stmts stmt           { $$ = block($1, $2);   }
    ;

stmt
    : expr_stmt            { $$ = $1; }
    | iter_stmt            { $$ = $1; }
    | select_stmt          { $$ = $1; }
    | comp_stmt            { $$ = $1; }
    ;

expr_stmt
    : ';'      { $$ = 0;  }
    | expr ';' { $$ = $1; }
    ;

expr
    : binary_expr     { $$ = $1; }
    | assign_expr     { $$ = $1; }
    | call_expr       { $$ = $1; }
    | decl_expr       { $$ = $1; }
    | init_expr       { $$ = $1; }
    | VAR_IDENT       { $$ = expByName($1); }
    | INTEGER         { $$ = expByNum($1); }
    | '(' expr ')'    { $$ = $2; }
    ;

comp_stmt
    : '{' '}'         { $$ = 0; }
    | '{' stmts '}'   { $$ = $2; }
    ;

decl_expr
    : type VAR_IDENT                 { $$ = declaration($1, $2, NULL); }
    ;

init_expr
    : type VAR_IDENT  '=' expr       { $$ = declaration($1, $2, $4); }
    ;

assign_expr
    : VAR_IDENT  '=' expr            { $$ = assignment($1, $3); }
    ;

call_expr
    : IDENT '(' expr ')'         { $$ = call($1, $3); }
    ;

iter_stmt
    : WHILST '(' expr ')' stmt { $$ = whilst($3, $5); }
    ;

select_stmt
    : AN '(' expr ')' stmt     { $$ = an($3, $5); }
    ;

binary_expr
    : expr '+' expr  { $$ = binaryop($1, $3, '+'); }
    | expr '-' expr  { $$ = binaryop($1, $3, '-'); }
    | expr '*' expr  { $$ = binaryop($1, $3, '*'); }
    | expr '/' expr  { $$ = binaryop($1, $3, '/'); }
    | expr '%' expr  { $$ = binaryop($1, $3, '%'); }
    | expr '>' expr  { $$ = binaryop($1, $3, '>'); }
    | expr '<' expr  { $$ = binaryop($1, $3, '<'); }
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

