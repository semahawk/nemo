/*
 * Nemo programming language.
 *
 * Not really to be usefull in feature. Learning by
 * writing and being cool by saying "I've created a programming language".
 *
 * Copyright: (c) 2012-2013 by Szymon Urbaś <szymon.urbas@aol.com>
 *
 * Started at: Sat Sep 15, 16:05:12 2012
 *
 * Checkpoints:
 *   Sat May 25, 14:53:00 2013 - 400 commits!
 *   Sun Mar 31, 19:47:03 2013 - 300 commits!
 *   Mon Feb  5, 22:18:24 2013 - 200 commits!
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
#include <dlfcn.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "nemo.h"
#include "config.h"

static int nmInteractive(void);

/* list of all the library handles that need to be dlclosed */
static LibHandlesList *handles = NULL;
/* a small tiny handy macro to add a handle to the list */
#define ADD_HANDLE(handle) do { \
  LibHandlesList *new_list = NmMem_Malloc(sizeof(LibHandlesList)); \
  new_list->handle = handle; \
  new_list->next = handles; \
  handles = new_list; \
} while (0);
/* a tiny small macro to dlclose all the handles */
#define CLOSE_HANDLES() do { \
  LibHandlesList *list; \
  LibHandlesList *next; \
\
  for (list = handles; list != NULL; list = next){ \
    next = list->next; \
    dlclose(list->handle); \
    NmMem_Free(list); \
  } \
} while (0);

int main(int argc, char *argv[])
{
  /* the main node from parsing the given file */
  Node *nodest = NULL;
  /* the main file's scopereter state */
  NmScope_New("main");
  /* file input */
  char input[255];
  /* used for getopt */
  int c;

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
                    case 'a': NmDebug_SetFlag(DEBUG_FLAG_AST);
                              break;
                    case 'l': NmDebug_SetFlag(DEBUG_FLAG_LEXER);
                              break;
                    case 'm': NmDebug_SetFlag(DEBUG_FLAG_MEMORY);
                              break;
                    case 'p': NmDebug_SetFlag(DEBUG_FLAG_PARSER);
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

  /* fetch the builtin functions */
  NmBuiltin_Init();

  /* an argument was passed */
  if (optind < argc){
    strcpy(input, argv[optind++]);
  /* no argument passed atall */
  } else {
    /*
     * XXX exitting code here
     */
    return nmInteractive();
  }

  /* parse the file */
  nodest = NmParser_ParseFile(input);
  /* execute the nodes */
  NmAST_Exec(nodest);
  /* tidy up after executing */
  NmAST_FreeBlock(nodest);

  /* tidy up */
  NmObject_Tidyup();
  NmScope_Tidyup();
  CLOSE_HANDLES();

  return EXIT_SUCCESS;
}

static int nmInteractive(void)
{
  /* users input, Nemo's prompt */
  char *input, prompt[64];
  /* current line user has entered */
  unsigned line = 0;
  /* result of the executed input */
  NmObject *ob;

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

    ob = NmAST_Exec(NmParser_ParseString(input));

    printf("=> ");
    NmObject_PRINT(stdout, ob);
    printf("\n");
  }

  NmScope_Tidyup();

  return 0;
}

void Nm_InitModule(NmModuleFuncs *funcs)
{
  Scope *scope = NmScope_GetCurr();

  /* append all the C functions to the cfuncs */
  for (NmModuleFuncs *f = funcs; f->name != NULL; f++){
    CFuncsList *list = NmMem_Malloc(sizeof(CFuncsList));
    CFunc *func = NmMem_Malloc(sizeof(CFunc));

    func->name = NmMem_Strdup(f->name);
    func->body = f->ptr;
    list->func = func;
    /* append to the list */
    list->next = scope->cfuncs;
    scope->cfuncs = list;
  }
}

void Nm_UseModule(char *name)
{
  NmScope_New(name);
  Nm_IncludeModule(name);
  NmScope_Restore();
}

/*
 * Search paths: (the higher the firster (hehe))
 *
 *   - ./
 *   - /usr/lib/nemo
 *
 */
void Nm_IncludeModule(char *name)
{
/* DRY, include a C library */
#define INCLUDE_SO(PATH) do { \
  /* fetching the library */ \
  void  *handle; \
  void (*lib_init)(void); \
  char  *error; \
\
  handle = dlopen(PATH, RTLD_LAZY); \
  if (!handle){ \
    NmError_Error("%s", dlerror()); \
    exit(EXIT_FAILURE); \
  } \
\
  dlerror(); /* clear any existing error */ \
\
  lib_init = dlsym(handle, init_func); \
\
  if ((error = dlerror()) != NULL){ \
    NmError_Error("%s", error); \
    exit(EXIT_FAILURE); \
  } \
\
  lib_init(); \
  ADD_HANDLE(handle); \
  fclose(fp); \
} while (0);

/* DRY, include a Nemo file */
#define INCLUDE_NM(PATH) do { \
  Node *nodest = NmParser_ParseFile(PATH); \
  NmAST_Exec(nodest); \
  NmAST_Free(nodest); \
  fclose(fp); \
} while (0);

  FILE *fp;
  /* current working directory */
  char *cwd;
  /* library path */
  char lib_path[64];
  /* function called name_init */
  char init_func[32];
  /* library path with name.so appended */
  char lib_path_so[64];
  /* library path with name.nm appended */
  char lib_path_nm[64];
  /* relative path */
  char relative_path[64];
  /* relative path with name.so appended */
  char relative_path_so[64];
  /* relative path with name.nm appended */
  char relative_path_nm[64];

  /* fetch the cwd */
  cwd = getcwd(0, 0);
  /* init the paths */
  sprintf(lib_path,    LIBDIR "/nemo/");
  sprintf(lib_path_so, LIBDIR "/nemo/%s.so",   name);
  sprintf(lib_path_nm, LIBDIR "/nemo/%s.nm",   name);
  sprintf(relative_path,      "%s/",      cwd);
  sprintf(relative_path_so,   "%s/%s.so", cwd, name);
  sprintf(relative_path_nm,   "%s/%s.nm", cwd, name);
  sprintf(init_func,          "%s_init",       name);

  /* there is a file called name.so in the current directory */
  if ((fp = fopen(relative_path_so, "rb")) != NULL){
    INCLUDE_SO(relative_path_so);
    return;
  /* there is a file called name.nm in the current directory */
  } else if ((fp = fopen(relative_path_nm, "r")) != NULL){
    INCLUDE_NM(relative_path_nm);
    return;
  }

  /* found a .so library in the LIBDIR directory */
  if ((fp = fopen(lib_path_so, "rb")) != NULL){
    INCLUDE_SO(lib_path_so);
    return;
  /* found a Nemo file in the LIBDIR */
  } else if ((fp = fopen(lib_path_nm, "r")) != NULL){
    INCLUDE_NM(lib_path_nm);
    return;
  }

  free(cwd); /* getcwd does malloc */
  NmError_Error("couldn't find module '%s' both in %s and %s", name, relative_path, lib_path);
  exit(EXIT_FAILURE);
}

/*
 * Megadeth, Running Wild, Gamma Ray, Iron Savior
 * Helloween, Testament
 * Within Temptation, Nightwish, Avantasia
 * Stratovarius, Steve Vai, At Vance, Rhapsody of Fire
 *
 * Family Guy, The Office, Monty Python
 *
 */

