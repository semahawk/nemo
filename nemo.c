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

#include "nemo.h"
#include "parser.h"
#include "error.h"
#include "debug.h"
#include "mem.h"

int main(int argc, char *argv[])
{
  /* file input */
  char input[255];
  /* used for getopt */
  int c;
  /* creating Nemo's main object */
  /* (not using nmCalloc, because the -dm flag is not set yet) */
  Nemo *NM = calloc(1, sizeof(Nemo));
  if (!NM){
    nmFatal("calloc failed to create the main object");
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
                              printf("  m   memory allocation, freeing and so-on\n");
                              printf("  p   parser messages\n");
                              printf("  l   lexer messages\n\n");
                              return EXIT_SUCCESS;
                    case 'm': NM->flags.debug.memory = TRUE;
                              break;
                    case 'p': NM->flags.debug.parser = TRUE;
                              break;
                    case 'l': NM->flags.debug.lexer = TRUE;
                              break;
                    default:  nmError("unknown option for debug '%c', run with -dh to see the possible options", *optarg);
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
  debugCalloc(NM, 1, sizeof(Nemo));

  /* an argument was passed */
  if (optind < argc){
    strcpy(input, argv[optind++]);
  /* no argument passed atall */
  } else {
    nmFatal("no input files");
    return EXIT_FAILURE;
  }

  /* set the sources name */
  NM->source = nmMalloc(NM, strlen(input) + 1);
  strcpy(NM->source, input);

  /* parse the file */
  parseFile(NM, NM->source);

  /* tidy up */
  nmFree(NM, NM->source);
  nmFree(NM, NM);

  return EXIT_SUCCESS;
}

/*
 * Megadeth, Running Wild, Gamma Ray, Iron Savior
 * Helloween, Testament
 * Within Temptation, Nightwish, Avantasia
 * Stratovarius
 *
 * Family Guy, The Office, Monty Python
 *
 */

