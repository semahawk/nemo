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

  void count();
  extern char *stdup(const char *);

  int linenum = 1;

%}

D [0-9]

%x IN_COMMENT

%%

"int"                    { count(); return TYPE_INT; }
"float"                  { count(); return TYPE_FLOAT; }

"whilst"                 { count(); return WHILST; }

"\n"                     { count(); linenum++; }

"+"                      { count(); yylval.op = *yytext; return OP; }
"-"                      { count(); yylval.op = *yytext; return OP; }
"*"                      { count(); yylval.op = *yytext; return OP; }
"/"                      { count(); yylval.op = *yytext; return OP; }
"%"                      { count(); yylval.op = *yytext; return OP; }
"<"                      { count(); yylval.op = *yytext; return OP; }
">"                      { count(); yylval.op = *yytext; return OP; }

"("                      { count(); return '('; }
")"                      { count(); return ')'; }
"{"                      { count(); return '{'; }
"}"                      { count(); return '}'; }
"="                      { count(); return '='; }
";"                      { count(); return ';'; }

[a-zA-Z_][a-zA-Z0-9_]*   { count(); yylval.s = strdup(yytext); return IDENTIFIER; }
{D}{D}*                  { count(); yylval.i = atoi(yytext); return INTEGER; }

[ \t]                    { count(); }
.                        { /* ignooar */ }

<INITIAL>{
  "/*"                   BEGIN(IN_COMMENT);
}
<IN_COMMENT>{
  "*/"\n*                BEGIN(INITIAL);
  [^*\n]+                // eat comment in chunks
  "*"                    // eat the lone star
  \n                     linenum++;
}

%%

int yywrap()
{
  return 1;
}

int column = 0;

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

