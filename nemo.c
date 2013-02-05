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
// Started at: Sat Sep 15, 16:05:12
//
//
// Mon Feb  5, 22:18:24 - 200 commits!
//

#include <stdio.h>

#include "nemo.h"
#include "handy.h"
#include "exec.h"
#include "nodes.h"
#include "free.h"
#include "lemon.h"
#include "grammar.h"
#include "scanner.h"
#include "yystype.h"

// --debug, -d
bool debug_flag = false;
// --eval, -e
bool eval_flag = false;
// number of --eval flags
unsigned int eval_size = 0;
// array of strings to be evaled
char *eval_strings[MAX_EVAL_FLAGS];

int main(int argc, char *argv[])
{
  // temporary file
  FILE *tmp_fp = NULL;
  char *tmp_fn = NULL;
  // value to be returned (from executing the main block)
  int ret = 0;
  // name of the source file to be interpreted
  char source[255];

  // struct pointer to our nodes
  struct Node *nodest = NULL;

  int c;

  while (true){
    static struct option long_options[] = {
      { "eval",    required_argument, 0, 'e' },
      { "debug",   no_argument,       0, 'd' },
      { "version", no_argument,       0, 'v' },
      { 0, 0, 0, 0 }
    };

    int option_index = 0;

    c = getopt_long(argc, argv, "e:dv", long_options, &option_index);

    if (c == -1)
      break;

    switch (c){
      case 'e': eval_flag = true;
                if (eval_size >= MAX_EVAL_FLAGS){
                  error("there is a limit of `--eval' flags of %d", MAX_EVAL_FLAGS);
                  exit(1);
                }

                eval_strings[eval_size] = strdup(optarg);
                eval_size++;
                break;

      case 'd': debug_flag = true;
                break;

      case 'v': version();
                exit(0);

      case '?': break;
      default: abort();
    }
  }

  // at least one --eval flag passed
  if (eval_flag){
    time_t seconds = time(NULL);
    tmp_fn = tmpnam(NULL);
    tmp_fp = fopen(tmp_fn, "w");

    fprintf(tmp_fp, "// A temporary file automagically created by Nemo, at %d seconds since epoch.\n", (unsigned int)seconds);
    for (unsigned int i = 0; i < eval_size; i++){
      fprintf(tmp_fp, "%s\n", eval_strings[i]);
    }
    fclose(tmp_fp);

    strcpy(source, tmp_fn);
  // no --eval flags passed
  } else {
    if (optind < argc){
      strcpy(source, argv[optind++]);
    }
    else {
      error("no input files");
      return 1;
    }

  }

  nodest = parseFile(source);

  ret = execNodes(nodest).v.i;
  freeNodes(nodest);
  /*freeStack();*/

  if (eval_flag)
    fclose(tmp_fp);

  return ret;
}

struct Node *parseFile(char *fname)
{
  struct Context context;
  void *parser;
  FILE *fp;
  yyscan_t scanner;
  struct yyguts_t *yyg;

  if ((fp = fopen(fname, "r")) == NULL){
    fprintf(stderr, "nemo: error: ");
    perror(fname);
    exit(1);
  }

  yylex_init(&scanner);
  parser = ParseAlloc(myalloc);
  yyg = (struct yyguts *)scanner;
  context.filename = fname;
  yyset_in(fp, scanner);

  YYSTYPE token;

  int lex_code;
  do {
    lex_code = yylex(scanner);
    token = yylval;
    Parse(parser, lex_code, token, &context);
  } while (lex_code > 0);

  yylex_destroy(scanner);
  ParseFree(parser, free);

  if (!context.nodest){
    error("execution failed due to some errors");
    exit(1);
  }

  fclose(fp);

  return context.nodest;
}

struct Node *parseString(char *string)
{
  struct Context context;
  void *parser;
  yyscan_t scanner;
  YY_BUFFER_STATE buffer;

  yylex_init(&scanner);
  buffer = yy_scan_string(string, scanner);
  parser = ParseAlloc(malloc);
  context.filename = string;

  YYSTYPE token;

  int lex_code;
  do {
    lex_code = yylex(scanner);
    token = yylval;
    Parse(parser, lex_code, token, &context);
  } while (lex_code > 0);

  yy_delete_buffer(buffer, scanner);
  yylex_destroy(scanner);
  ParseFree(parser, free);

  if (!context.nodest){
    error("execution failed due to some errors");
    exit(1);
  }

  return context.nodest;
}

void version(void)
{
  printf("Nemo v%s, %s\n", VERSION, __DATE__);
}

/*
 * Megadeth, Running Wild, Gamma Ray, Iron Savior
 * Helloween, Testament
 * Within Temptation, Nightwish
 *
 * Family Guy, The Office, Monty Python
 *
 */

