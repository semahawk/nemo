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
%type <node> stmts stmt
%type <node> expr_stmt iter_stmt select_stmt comp_stmt
%type <node> expr decl_expr init_expr assign_expr call_expr binary_expr unary_expr
%type <type> type

%token TYPE_INT
%token WHILE IF
%token PLUSPLUS MINUSMINUS

%right '='
%left  '<' '>'
%left  '+' '-'
%left  '*' '/' '%'
%nonassoc PLUSPLUS MINUSMINUS
%left  '('

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
    : IDENT '(' expr ')'         { $$ = genCall($1, $3); }
    ;

iter_stmt
    : WHILE '(' expr ')' stmt { $$ = genWhile($3, $5); }
    ;

select_stmt
    : IF '(' expr ')' stmt     { $$ = genIf($3, $5); }
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

