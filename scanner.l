/*
 * scanner.l
 *
 * Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
 *
 */

%option outfile="scanner.c"
%option reentrant
%option noyywrap
%option noinput
%option nounput

%{

  #include <stdio.h>

  #include "handy.h"
  #include "grammar.h"
  #include "yystype.h"

  extern char *stdup(const char *);

  // keep track of what line is it in the source
  int line = 0;
  // keep track of what column is it in the source
  int col = 0;

%}

D [0-9]

%x IN_COMMENT

%%

"/"{2,}.*                { /* one line comments */ ++line; }

"use"                    { return USE; }
"fun"                    { return FUN; }
"times"                  { return TIMES; }

"if"                     { return IF; }
"else"                   { return ELSE; }
"for"                    { return FOR; }
"while"                  { return WHILE; }
"return"                 { return RETURN; }

"\n"                     { ++line; }

"++"                     { return PLUSPLUS; }
"--"                     { return MINUSMINUS; }
">="                     { return GE; }
"<="                     { return LE; }
"!="                     { return NE; }
"=="                     { return EQ; }
"+="                     { return EQ_ADD; }
"-="                     { return EQ_SUB; }
"*="                     { return EQ_MUL; }
"/="                     { return EQ_DIV; }
"%="                     { return EQ_MOD; }
".="                     { return EQ_DOT; }
"="                      { return ASSIGN; }
"<"                      { return LT; }
">"                      { return GT; }
"ge"                     { return STR_GE; }
"le"                     { return STR_LE; }
"ne"                     { return STR_NE; }
"eq"                     { return STR_EQ; }
"lt"                     { return STR_LT; }
"gt"                     { return STR_GT; }
"+"                      { return ADD; }
"-"                      { return SUB; }
"*"                      { return MUL; }
"/"                      { return DIV; }
"%"                      { return MOD; }

"("                      { return LPAREN; }
")"                      { return RPAREN; }
"{"                      { return LMUSTASHE; }
"}"                      { return RMUSTASHE; }
";"                      { return SEMICOLON; }
"."                      { return DOT; }
","                      { return COMMA; }

\$[a-zA-Z_][a-zA-Z0-9_]* { yylval.s = strdup(yytext); return VAR_IDENT; }
\$[+-]                   { yylval.s = strdup(yytext); return VAR_IDENT; }
![a-zA-Z_][a-zA-Z0-9_]*  { yylval.s = strdup(yytext); return VAR_IDENT; }
![+-]                    { yylval.s = strdup(yytext); return VAR_IDENT; }
[a-zA-Z_][a-zA-Z0-9_]*   { yylval.s = strdup(yytext); return IDENT;     }
{D}{D}*                  { yylval.i = atoi(yytext);   return INTEGER;   }
{D}*\.{D}+               { yylval.f = atof(yytext);   return FLOATING;  }
\"(\\.|[^\\"])*\"        { yylval.s = strip_quotes(strdup(yytext)); return STRING; }

[ \t]                    { }
.                        { /* ignooar */ }

<INITIAL>{
  "/*"                   BEGIN(IN_COMMENT);
}
<IN_COMMENT>{
  "*/"                   BEGIN(INITIAL);
  [^*\n]+                // eat comment in chunks
  "*"                    // eat the lone star
  \n                     ++line;
}

%%

