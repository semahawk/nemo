/*
 * Nemo programming language.
 *
 * Not really to be usefull in feature. Learning by
 * writing and being cool by saying "I've created a programming language".
 *
 * Copyright: (c) 2012-2013 by Szymon Urbaś <szymon.urbas@aol.com>
 *
 * Started at: Sat Sep 15, 16:05:12
 *
 * Checkpoints:
 *   Sun Mar 31, 19:47:03 - 300 commits!
 *   Mon Feb  5, 22:18:24 - 200 commits!
 *
 * License: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

/*
 * "You take a mortal man
 *  And put him in control
 *  Watch him become a god
 *  Watch people's heads a-roll"
 *
 *  Megadeth - Symphony of Destruction
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "nemo.h"
#include "parser.h"
#include "error.h"
#include "debug.h"
#include "mem.h"
#include "ast.h"

static int nmInteractive(Nemo *);

int main(int argc, char *argv[])
{
  /* used to iterate through the variables list
   * (when tidying up after them) */
  VariablesList *g;
  VariablesList *gnext;
  /* the main node from parsing the given file */
  Node *nodest = NULL;
  /* file input */
  char input[255];
  /* used for getopt */
  int c;
  /* creating Nemo's main object */
  /* (not using NmMem_Calloc, because the -dm flag is not set yet) */
  Nemo *NM = calloc(1, sizeof(Nemo));
  if (!NM){
    NmError_Fatal("calloc failed to create the main object");
    return EXIT_FAILURE;
  }

  while (1){
    static struct option long_options[] = {
      { "debug", required_argument, 0, 'd' },
      { "version", no_argument, 0, 'v' },
      { 0, 0, 0, 0 }
    };

    int option_index = 0;

    c = getopt_long(argc, argv, "d:v", long_options, &option_index);

    if (c == -1)
      break;

    switch (c){
      case 'd': {
                  switch (*optarg){
                    case 'h': printf("\nDebug options include:\n");
                              printf("  a   abstract syntax tree nodes creation and deallocation\n");
                              printf("  m   memory allocation, deallocation and so-on\n");
                              printf("  p   parser messages\n");
                              printf("  l   lexer messages\n\n");
                              return EXIT_SUCCESS;
                    case 'a': NM->flags.debug.ast = TRUE;
                              break;
                    case 'm': NM->flags.debug.memory = TRUE;
                              break;
                    case 'p': NM->flags.debug.parser = TRUE;
                              break;
                    case 'l': NM->flags.debug.lexer = TRUE;
                              break;
                    default:  NmError_Error("unknown option for debug '%c', run with '-dh' to see the possible options", *optarg);
                              return EXIT_FAILURE;
                  }
                  break;
                }
      case 'v': printf("Nemo v" VERSION ", " __DATE__ " " __TIME__"\n");
                return EXIT_SUCCESS;
      case '?': break;
      default: abort();
    }
  }
  NmDebug_CALLOC(NM, 1, sizeof(Nemo));

  /* an argument was passed */
  if (optind < argc){
    strcpy(input, argv[optind++]);
  /* no argument passed atall */
  } else {
    /*
     * XXX exitting code here
     */
    return nmInteractive(NM);
  }

  /* set the sources name */
  NM->source = NmMem_Malloc(NM, strlen(input) + 1);
  strcpy(NM->source, input);

  /* parse the file */
  nodest = NmParser_ParseFile(NM, NM->source);
  /* execute the nodes */
  NmAST_Exec(NM, nodest);
  /* tidy up after executing */
  NmAST_FreeBlock(NM, nodest);

  /* iterate through the variables */
  for (g = NM->globals; g != NULL; g = gnext){
    gnext = g->next;
    NmMem_Free(NM, g->var->name);
    NmMem_Free(NM, g->var);
    NmMem_Free(NM, g);
  }

  /* tidy up */
  NmObject_Tidyup(NM);
  NmMem_Free(NM, NM->source);
  NmMem_Free(NM, NM);

  return EXIT_SUCCESS;
}

static int nmInteractive(Nemo *NM)
{
  char *input, prompt[64];
  unsigned line = 0;

  printf("Welcome to the Nemo " VERSION " interactive!\n");
  printf("If you want to quit, just type 'quit' and hit Enter, or just ^D.\n\n");

  for (;;){
    /* set the prompt */
    sprintf(prompt, "nm:%x %% ", line);
    /* get the input */
    input = readline(prompt);
    /* eof */
    if (!input) break;
    /* add the input to the history */
    add_history(input);
    /* yup */
    line++;
    /* "quit" hit */
    if (!strcmp(input, "quit")){
      printf("\n  Have a good day!\n\n");
      return 0;
    }

    printf("=> %s\n", valueToS(NmAST_Exec(NM, NmParser_ParseString(NM, input))));
    /* do stuff */
  }

  return 0;
}

/*
 * Megadeth, Running Wild, Gamma Ray, Iron Savior
 * Helloween, Testament
 * Within Temptation, Nightwish, Avantasia
 * Stratovarius, Steve Vai
 *
 * Family Guy, The Office, Monty Python
 *
 */

