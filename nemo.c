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
 *   Mon Apr 21, 00:19:50 +0200 2014 - 600 commits!
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
#include "config.h"
#include "debug.h"
#include "parser.h"
#include "version.h"
#include "util.h"
#include "nob.h"

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

  /* initialize the GC pool and everything related */
  gc_init();
  /* initialize the argument stack */
  arg_stack_init();
  /* initialize the types (which includes creating the standard types) and everything related */
  types_init();

  while ((ch = getopt(argc, argv, "d:v")) != -1){
    switch (ch){
      case 'd':
#ifdef DEBUG
        switch (*optarg){
          case 'a':
            NM_DEBUG_SET_FLAG(NM_DEBUG_AST);
            break;
          case 'm':
            NM_DEBUG_SET_FLAG(NM_DEBUG_MEM);
            break;
          case 'l':
            NM_DEBUG_SET_FLAG(NM_DEBUG_LEXER);
            break;
          case 'h':
            fprintf(stderr, "\nAvailable debug flags:\n");
            fprintf(stderr, "  a    AST node creation/execution\n");
            fprintf(stderr, "  m    see how much memory was malloced/freed, etc.\n");
            fprintf(stderr, "  l    lexer stuff; see what tokens were fetched\n");
            fprintf(stderr, "\n");
            return 0;
          default:
            fprintf(stderr, "nemo: unknown option -d%c. run with option -dh "
              "to see the available debug flags.\n", *optarg);
            return 1;
        }
#else
        fprintf(stderr, "Nemo was built without the debug support.\n"
          "To enable it, please recompile the program with "
          "the option `-DDEBUG=1' set.\n");
        return 1;
#endif
        break;
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
  }

  dump_types();

  {
    /*struct infnum a = infnum_from_int(-127);*/
    /*struct infnum b = infnum_from_int(127);*/
    /*struct infnum c = infnum_from_int(73);*/
    /*struct infnum d = infnum_from_int(-73);*/

    struct infnum a = infnum_from_str("-23");
    struct infnum b = infnum_from_str("77");
    struct infnum c = infnum_from_str("23");
    struct infnum d = infnum_from_str("-77");

    printf("infnum from int %d: %s\n", 3, infnum_to_str(infnum_from_int(3)));
    printf("infnum from int %d: %s\n", 73, infnum_to_str(infnum_from_int(73)));
    printf("infnum from int %d: %s\n", -803, infnum_to_str(infnum_from_int(-803)));
    printf("infnum from int %d: %s\n", 3927, infnum_to_str(infnum_from_int(3927)));
    printf("infnum from str %s: %s\n", "3", infnum_to_str(infnum_from_str("3")));
    printf("infnum from str %s: %s\n", "-3", infnum_to_str(infnum_from_str("-3")));

    printf("%s + %s = %s\n", infnum_to_str(a), infnum_to_str(c), infnum_to_str(infnum_add(a, c)));
    printf("%s - %s = %s\n", infnum_to_str(a), infnum_to_str(c), infnum_to_str(infnum_sub(a, c)));
    printf("%s + %s = %s\n", infnum_to_str(b), infnum_to_str(c), infnum_to_str(infnum_add(b, c)));
    printf("%s - %s = %s\n", infnum_to_str(b), infnum_to_str(c), infnum_to_str(infnum_sub(b, c)));
    printf("%s + %s = %s\n", infnum_to_str(b), infnum_to_str(d), infnum_to_str(infnum_add(b, d)));
    printf("%s - %s = %s\n", infnum_to_str(b), infnum_to_str(d), infnum_to_str(infnum_sub(b, d)));
  }

  arg_stack_finish();
  types_finish();
  gc_finish();

  return 0;
}

/*
 * Megadeth, Running Wild, Gamma Ray, Iron Savior
 * Helloween, Testament
 * Within Temptation, Nightwish, Avantasia
 * Stratovarius, Steve Vai, At Vance, Rhapsody of Fire
 * Fear Factory, Scar Symmetry, Dagon, Omnium Gatherum
 * The Algorithm, Dream Theater, Insomnium
 * Dark Age, Equilibrium, Bolt Thrower, Kalmah
 * Coroner, Carach Angren
 * Qntal, Helium Vola
 *
 * Johann Strauss
 *
 * Family Guy, The Office, Monty Python, The I.T. Crowd
 * Black Books
 *
 */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

