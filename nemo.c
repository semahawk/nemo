/*
 * Nemo programming language.
 *
 * Not really to be usefull in feature.
 *
 * Just learning by writing and being cool by
 * saying "I've created a programming language".
 *
 * Copyright: (c) 2012-2013 by Szymon Urbaś <szymon.urbas@aol.com>
 *
 * Started at:
 *   Sat Sep 15, 16:05:12 +0200 2012
 *
 * Checkpoints:
 *   Sun Jul  7, 19:12:26 +0200 2013 - 500 commits!
 *   Sat May 25, 14:53:00 +0200 2013 - 400 commits!
 *   Sun Mar 31, 19:47:03 +0200 2013 - 300 commits!
 *   Mon Feb  5, 22:18:24 +0200 2013 - 200 commits!
 *
 * License: New / Modified (3 clause) BSD
 *          For more details please visit the LICENSE file.
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

/*
 * This file is the source for the executable "nemo".
 * It's not a part of the "libnemo".
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <locale.h>

#include "ast.h"
#include "parser.h"
#include "version.h"

int main(int argc, char *argv[])
{
  char *locale;
  /* used for getopt */
  int ch;

  if (((locale = getenv("LC_ALL")) && *locale) ||
      ((locale = getenv("LC_CTYPE")) && *locale) ||
      ((locale = getenv("LANG")) && *locale)){
    if (!strstr(locale, "UTF-8")){
      fprintf(stderr, "nemo: the locale '%s' is not supported; the only supported locale is (a variant of) UTF-8\n", locale);
      exit(1);
    }
  }

  setlocale(LC_ALL, "");

  /* initialize the argument stack */
  arg_stack_init();

  while ((ch = getopt(argc, argv, "v")) != -1){
    switch (ch){
      case 'v': printf("Nemo v%d.%d.%d, " __DATE__ " " __TIME__"\n",
                  NM_VERSION_MAJOR, NM_VERSION_MINOR, NM_VERSION_PATCH);
                return 0;
      case '?': return 1;
      default:  abort();
    }
  }

  argc -= optind;
  argv += optind;

  if (argc >= 1){
    parse_file(argv[0]);
  } else {
    parse_string("my £ my żółć ¢¥€");
  }

  return 0;
}

/*
 * Megadeth, Running Wild, Gamma Ray, Iron Savior
 * Helloween, Testament
 * Within Temptation, Nightwish, Avantasia
 * Stratovarius, Steve Vai, At Vance, Rhapsody of Fire
 * Fear Factory, Scar Symmetry, Dagon, Omnium Gatherum
 * The Algorithm, Dream Theater, Insomnium
 * Dark Age
 *
 * Family Guy, The Office, Monty Python, The I.T. Crowd
 *
 */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

