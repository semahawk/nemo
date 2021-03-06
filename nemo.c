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
#include <unistd.h>

#include "ast.h"
#include "config.h"
#include "debug.h"
#include "infer.h"
#include "mem.h"
#include "parser.h"
#include "version.h"
#include "util.h"
#include "nob.h"

/* the output file in case we are compiling */
FILE *outfile;
/* and it's name */
char outfilename[32];

int main(int argc, char *argv[])
{
  char *locale;
  /* used for getopt */
  int ch;
  /* return value */
  int ret = 0;
  /* the top-most node created from parsing */
  struct node *root;
  /* THE scope */
  struct scope *_main = new_scope("main", NULL);

  /* are we compiling? */
  bool compile = false;

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
  /* initialize the types (which includes creating the standard types) and everything related */
  types_init();

  while ((ch = getopt(argc, argv, "cd:v")) != -1){
    switch (ch){
      case 'c':
        compile = true;
        break;
      case 'd':
#ifdef DEBUG
        switch (*optarg){
          case 'a':
            NM_DEBUG_SET_FLAG(NM_DEBUG_AST);
            break;
          case 'l':
            NM_DEBUG_SET_FLAG(NM_DEBUG_LEXER);
            break;
          case 'm':
            NM_DEBUG_SET_FLAG(NM_DEBUG_MEM);
            break;
          case 'p':
            NM_DEBUG_SET_FLAG(NM_DEBUG_PARSER);
            break;
          case 't':
            NM_DEBUG_SET_FLAG(NM_DEBUG_TYPES);
            break;
          case 'h':
            fprintf(stderr, "\nAvailable debug flags:\n");
            fprintf(stderr, "  a    AST node creation/execution\n");
            fprintf(stderr, "  l    lexer stuff; see what tokens were fetched\n");
            fprintf(stderr, "  m    see how much memory was malloced/freed, etc.\n");
            fprintf(stderr, "  p    parser stuff; see a primitive representation of the parsing process\n");
            fprintf(stderr, "  t    dump the object types at the end of execution\n");
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
    if ((root = parse_file(argv[0], _main)) == NULL){
      fprintf(stderr, "nemo: execution failed :c\n");
      ret = 1;
      goto end;
    }

    if (compile){
      char systemcall[128];
      char *noextname = strdup(argv[0]);
      char *p = strrchr(noextname, '.');
      /* remove the extension from the file name */
      if (p) *p = '\0';

      snprintf(outfilename, sizeof(outfilename), "%s.asm", noextname);

      if ((outfile = fopen(outfilename, "w")) == NULL){
        perror("nemo: fopen");
        exit(1);
      }

      /* compile the program (ie. write all the assembly out into <outfile>) */
      comp_nodes(root);
      /* close the assembly file so we can proceed and compile it */
      fclose(outfile);

      snprintf(systemcall, sizeof(systemcall), "nasm -g -f elf32 %s.asm -o %s.o", noextname, noextname);
      system(systemcall);

      snprintf(systemcall, sizeof(systemcall), "ld -o %s %s.o", noextname, noextname);
      system(systemcall);

      free(noextname);
    } else
      exec_nodes(root);

    /* TODO clean up after the parser, lexer, etc. */
  } else {
    /* interactive */
    char input_buffer[512];
    char *input;
    bool show_type;
    struct nob_type *inferred_type;

    for (;;){
      input = input_buffer;
      show_type = false;

      printf("N: ");
      fgets(input_buffer, 512, stdin);

      /* chomp input */
      char *p = strrchr(input, '\n');
      if (p) *p = '\0';

      if (!strcmp(input, ".q"))
        break;

      if (!strncmp(input, ".t ", 3)){
        show_type = true;
        input += 3;
      }

      if ((root = parse_string("stdin", input, _main)) != NULL){
        if (show_type){
          printf("%s : ", input);

          if ((inferred_type = infer_node_type(_main, root)) != NULL)
            nob_print_type(inferred_type);

          printf("\n");
        } else {
          exec_nodes(root);
        /* TODO clean up after the parser, lexer, etc. */
        }
      }
    }
  }

  if (NM_DEBUG_GET_FLAG(NM_DEBUG_TYPES))
    dump_types();

end:
  /* the order quite matters */
  arg_stack_finish();
  gc_finish();
  types_finish();
  scopes_finish();

  return ret;
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
 * Mourning Beloveth, Doom:VS, Draconian
 * Lascaille's Shroud
 * Thy Light, Furia, Vinterland
 *
 * Johann Strauss
 *
 * Family Guy, The Office, Monty Python, The I.T. Crowd
 * Black Books, The Big Bang Theory
 *
 */

/*
 * vi: ft=c:ts=2:sw=2:expandtab
 */

