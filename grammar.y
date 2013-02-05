/*
 * grammar.y
 *
 * Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
 *
 */

%token_type { int }

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

%include {

  #include "nemo.h"
  #include "nodes.h"
  #include "gen.h"
  #include "handy.h"
  #include "vars.h"
  #include "grammar.h"

  extern char source[255];

  struct Node *currentblock = NULL;
  struct Node *funcdef = NULL;
  struct Node *iterblock = NULL;
  int argcount = 0, paramcount = 0;

}

%parse_accept {
  printf("parsing completed!\n");
}

%syntax_error {
  fprintf(stderr, "syntax error!\n");
  exit(1);
}

source ::= stmts .      {  }

stmts ::= stmt .        {  }
stmts ::= stmts stmt .  {  }

stmt ::= comp_stmt .    {  }
stmt ::= iter_stmt .    {  }
stmt ::= select_stmt .  {  }
stmt ::= funcdef_stmt . {  }
stmt ::= expr_stmt .    {  }
stmt ::= use_stmt .     {  }
stmt ::= return_stmt .  {  }

comp_stmt ::= LMUSTASHE stmts RMUSTASHE .         {  }

iter_stmt ::= FOR expr_stmt expr_stmt expr stmt . {  }
iter_stmt ::= WHILE expr stmt .                   {  }
iter_stmt ::= expr TIMES stmt .                   {  }

select_stmt ::= IF expr stmt .                    {  }
select_stmt ::= IF expr stmt ELSE stmt .          {  }

funcdef_stmt ::= FUN IDENT arg_list comp_stmt .   {  }

use_stmt ::= USE STRING SEMICOLON .               {  }

return_stmt ::= RETURN expr SEMICOLON .           {  }
return_stmt ::= RETURN SEMICOLON .                {  }

expr_stmt ::= SEMICOLON .                         {  }
expr_stmt ::= expr SEMICOLON .                    {  }

expr ::= IDENT LPAREN param_list RPAREN .         {  }
expr ::= LPAREN expr RPAREN . {  }
expr ::= INTEGER .            {  }
expr ::= FLOATING .           {  }
expr ::= STRING .             {  }
expr ::= lvalue .             {  }
expr ::= expr ASSIGN expr .   {  }
expr ::= expr ADD expr .      {  }
expr ::= expr SUB expr .      {  }
expr ::= expr MUL expr .      {  }
expr ::= expr DIV expr .      {  }
expr ::= expr MOD expr .      {  }
expr ::= expr EQ_ADD expr .   {  }
expr ::= expr EQ_SUB expr .   {  }
expr ::= expr EQ_MUL expr .   {  }
expr ::= expr EQ_DIV expr .   {  }
expr ::= expr EQ_MOD expr .   {  }
expr ::= expr NE expr .       {  }
expr ::= expr EQ expr .       {  }
expr ::= expr GT expr .       {  }
expr ::= expr LT expr .       {  }
expr ::= expr GE expr .       {  }
expr ::= expr LE expr .       {  }
expr ::= expr STR_NE expr .   {  }
expr ::= expr STR_EQ expr .   {  }
expr ::= expr STR_GT expr .   {  }
expr ::= expr STR_LT expr .   {  }
expr ::= expr STR_GE expr .   {  }
expr ::= expr STR_LE expr .   {  }
expr ::= ADD expr . [UPLUS]   {  }
expr ::= SUB expr . [UMINUS]  {  }
expr ::= lvalue PLUSPLUS .    {  }
expr ::= lvalue MINUSMINUS .  {  }

// If I turn these two on, there will be 2 conflicts, that prevent from
// compiling. It would work if I commented those post- guys.
//
//expr ::= PLUSPLUS lvalue .    {  }
//expr ::= MINUSMINUS lvalue .  {  }

arg_list ::= lvalue .                  { }
arg_list ::= arg_list COMMA lvalue .   { }
arg_list ::= .                         { }

param_list ::= expr .                  { }
param_list ::= param_list COMMA expr . { }
param_list ::= .                       { }

lvalue ::= VAR_IDENT . {  }

