//
// Nemo programming language.
//
// Not really to be usefull in feature. Learning by
// writing and being cool by saying "I've created a programming language".
//
// License: MIT
//
// Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
// 
// Date: Sat Sep 15, 16:05:12
// 

#include "nemo.h"
#include "handy.h"
#include "nodes_exec.h"

extern int yyparse(void);
extern FILE *yyin;
extern FILE *yyout;

// struct pointer to our nodes
struct Node *nodest;
// name of the source file to be interpreted
char source[255];

int main(int argc, char *argv[])
{
  FILE *fp;

  if (argc > 1){
    strcpy(source, argv[1]);
  }
  
#if DEBUG
  else {
    strcpy(source, "example.nm");
  }
#else
  else {
    error("no input files");
    return 1;
  }
#endif

  if ((fp = fopen(source, "r")) != NULL){
    yyin = fp;
  } else {
    perror(source);
    return 1;
  }

  do {
    yyparse();
    assert(nodest);
    struct ExecEnv *e = createEnv();
    execNodes(e, nodest);
    freeEnv(e);
    // TODO: destroy the AST
  } while (!feof(yyin));

  return 0;
}

