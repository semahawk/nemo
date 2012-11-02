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
#include "nodes_gen.h"

extern int yyparse(void);
extern FILE *yyin;
extern FILE *yyout;

// struct pointer to our nodes
struct Node *nodest = NULL;
// name of the source file to be interpreted
char source[255];
// keep track of what line is it in the source
int linenum = 0;
// keep track of what column is it in the source
int column = 0;

int main(int argc, char *argv[])
{
  FILE *fp;

  int c;

  while (true){
    static struct option long_options[] = {
      { "version", no_argument, 0, 'v' },
      { 0, 0, 0, 0 }
    };

    int option_index = 0;

    c = getopt_long(argc, argv, "v", long_options, &option_index);

    if (c == -1)
      break;

    switch (c){
      case 'v': version();
                exit(0);

      case '?': break;
      default: abort();
    }
  }

  if (optind < argc){
    strcpy(source, argv[1]);
  }
  
#if DEBUG
  else {
    strcpy(source, "testing.nm");
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
    if (!nodest){
      error("compilation failed due to some errors:");
      exit(1);
    }
    struct ExecEnv *e = createEnv();
    execNodes(e, nodest);
    freeEnv(e);
    // destroy the AST
    for (int i = 0; i < nodest->data.statements.count; i++){
      debug("deleting node #%d");
      free(nodest->data.statements.statements[i]);
    }
    debug("deleting nodest");
    free(nodest);
  } while (!feof(yyin));

  return 0;
}

void version(void)
{
  printf("Nemo v%s\n", VERSION);
}

