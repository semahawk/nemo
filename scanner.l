/*
 * scanner.l
 *
 * Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
 *
 */

%option outfile="lex.yy.c"
%option noinput
%option nounput

%{

  #include <stdio.h>
  #include "handy.h"
  #include "y.tab.h"

  #define MAX_INCLUDE_DEPTH 10
  YY_BUFFER_STATE include_stack[MAX_INCLUDE_DEPTH];
  int include_stack_ptr = 0;

  void count();
  extern char *stdup(const char *);

  extern int linenum;
  extern int column;

%}

D [0-9]

%x USE
%x IN_COMMENT

%%

"/"{2,}.*                { /* one line comments */ ++linenum; }

"use"                    { BEGIN(USE); }
"fun"                    { count(); return FUN; }

"int"                    { count(); return TYPE_INT; }
"float"                  { count(); return TYPE_FLOAT; }

"none"                   { count(); return NONE; }

"if"                     { count(); return IF; }
"else"                   { count(); return ELSE; }
"for"                    { count(); return FOR; }
"while"                  { count(); return WHILE; }
"return"                 { count(); return RETURN; }

"\n"                     { count(); ++linenum; }

"++"                     { count(); return PLUSPLUS; }
"--"                     { count(); return MINUSMINUS; }
"+"                      { count(); return '+'; }
"-"                      { count(); return '-'; }
"*"                      { count(); return '*'; }
"/"                      { count(); return '/'; }
"%"                      { count(); return '%'; }
"<"                      { count(); return '<'; }
">"                      { count(); return '>'; }

"("                      { count(); return '('; }
")"                      { count(); return ')'; }
"{"                      { count(); return '{'; }
"}"                      { count(); return '}'; }
"="                      { count(); return '='; }
";"                      { count(); return ';'; }
","                      { count(); return ','; }

\$[a-zA-Z][a-zA-Z0-9_']* { count(); yylval.s = strdup(yytext); return VAR_IDENT; }
[a-zA-Z_][a-zA-Z0-9_]*   { count(); yylval.s = strdup(yytext); return IDENT; }
{D}{D}*                  { count(); yylval.i = atoi(yytext); return INTEGER; }
{D}*\.{D}+               { count(); yylval.f = atof(yytext); return FLOAT; }

[ \t]                    { count(); }
.                        { /* ignooar */ }

<INITIAL>{
  "/*"                   BEGIN(IN_COMMENT);
}
<IN_COMMENT>{
  "*/"                   BEGIN(INITIAL);
  [^*\n]+                // eat comment in chunks
  "*"                    // eat the lone star
  \n                     ++linenum;
}

<USE>{
  [ \t]*                 { /* eat the whitespace */ }
  [^ \t\n\;]+ {
    if (include_stack_ptr >= MAX_INCLUDE_DEPTH){
      error("includes nested too deeply");
      exit(1);
    }

    include_stack[include_stack_ptr++] = YY_CURRENT_BUFFER;
    yyin = fopen(yytext, "r");

    if (!yyin){
      fprintf(stderr, "nemo: error in 'use': ");
      perror(yytext);
      exit(1);
    }

    yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));

    BEGIN(INITIAL);
  }
}

<<EOF>> {
  if (--include_stack_ptr < 0){
    yyterminate();
  } else {
    yy_switch_to_buffer(include_stack[include_stack_ptr]);
  }
}

%%

int yywrap()
{
  return 1;
}

void count()
{
  int i;

  for (i = 0; yytext[i] != '\0'; i++){
    if (yytext[i] == '\n'){
      column = 0;
    } else if (yytext[i] == '\t'){
      column += 8 - (column % 8);
    } else {
      column++;
    }
  }
}

