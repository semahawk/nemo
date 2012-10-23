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
  struct Node *node;
}

%token <i> INTEGER
%token <s> IDENTIFIER
%token <op> OP
%type <node> source statements statement block assignment whilst an call expression

%token TYPE_INT TYPE_FLOAT
%token WHILST AN

%start source

%%

source
    : statements                { nodest = $1; }
    ;

block
    : '{' statements '}'        { $$ = $2; }
    ;

statements
    :                           { $$ = 0; }
    | statements statement ';'  { $$ = statement($1, $2); }
    ;

statement
    : assignment                { $$ = $1; }
    | call                      { $$ = $1; }
    | whilst                    { $$ = $1; }
    | block                     { $$ = $1; }
    | an                        { $$ = $1; }
    ;

assignment
    : IDENTIFIER '=' expression                    { $$ = assignment($1, $3); }
    ;

call
    : IDENTIFIER '(' expression ')'                { $$ = call($1, $3); }
    ;

whilst
    : WHILST '(' expression ')' statement { $$ = whilst($3, $5); }
    | WHILST     expression     statement { $$ = whilst($2, $3); }
    ;

an
    : AN '(' expression ')' statement     { $$ = an($3, $5); }
    | AN     expression     statement     { $$ = an($2, $3); }
    ;

expression
    : IDENTIFIER                { $$ = expByName($1); }
    | INTEGER                   { $$ = expByNum($1); }
    | expression OP expression  { $$ = expression($1, $3, $2); }
    ;

%%

void yyerror(const char *s)
{
  fflush(stdout);
  printf("%s:%d:%d: %s", source, linenum, column, s);
}

