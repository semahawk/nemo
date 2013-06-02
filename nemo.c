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

/* list of all modules that were included */
/* by included it's meant that were include-d or use-d */
static Included *included = NULL;

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

/*
 * Add another position in the included list
 */
static inline void included_new(char *name)
{
  Included *new = NmMem_Malloc(sizeof(Included));
  new->name = NmMem_Strdup(name);
  new->next = included;
  included  = new;
}

/*
 * Free the included list
 */
static inline void included_tidyup(void)
{
  Included *curr;
  Included *next;

  for (curr = included; curr != NULL; curr = next){
    next = curr->next;
    NmMem_Free(curr->name);
    NmMem_Free(curr);
  }
}

int main(int argc, char *argv[])
{
  /* the main node from parsing the given file */
  Node *nodest = NULL;
  /* the main file's scope */
  NmScope_New("main");
  /* file input */
  char input[255];
  /* used for getopt */
  int c;
  /* if the --eval flag was supplied, this gets set to TRUE and the interactive
   * mode is not run when an argument was not supplied */
  BOOL met_e_flag = FALSE;

  /* fetch the builtin functions */
  NmBuiltin_Init();

  while (1){
    static struct option long_options[] = {
      { "use",     required_argument, 0, 'u' },
      { "include", required_argument, 0, 'i' },
      { "eval",    required_argument, 0, 'e' },
      { "debug",   required_argument, 0, 'd' },
      { "version", no_argument,       0, 'v' },
      { 0, 0, 0, 0 }
    };

    int option_index = 0;

    c = getopt_long(argc, argv, "u:i:e:d:v", long_options, &option_index);

    if (c == -1)
      break;

    switch (c){
      case 'u':
      {
        char line[64];
        sprintf(line, "use %s", optarg);
        /* parse the string */
        Node *node = NmParser_ParseString(line);
        /* execute the nodes */
        NmAST_Exec(node);
        /* tidy up after executing */
        NmAST_FreeBlock(node);
        break;
      }
      case 'i':
      {
        char line[64];
        sprintf(line, "include %s", optarg);
        /* parse the string */
        Node *node = NmParser_ParseString(line);
        /* execute the nodes */
        NmAST_Exec(node);
        /* tidy up after executing */
        NmAST_FreeBlock(node);
        break;
      }
      case 'e':
      {
        met_e_flag = TRUE;
        /* parse the string */
        Node *node = NmParser_ParseString(optarg);
        /* execute the nodes */
        NmAST_Exec(node);
        /* tidy up after executing */
        NmAST_FreeBlock(node);
        break;
      }
      case 'd':
      {
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
      case '?': return EXIT_FAILURE;
      default: abort();
    }
  }

  /* an argument was passed */
  if (optind < argc){
    strcpy(input, argv[optind++]);
  /* no argument passed atall */
  } else {
    /*
     * XXX exitting code here
     */
    if (met_e_flag)
      return EXIT_SUCCESS;
    else
      return nmInteractive();
  }

  /* first, count how many arguments the are after the input file */
  int ARGV_count = 0;
  for (int i = optind; i < argc; i++)
    ARGV_count++;

  /* create the ARGC variable and the ARGV array if any args are present */
  Variable *ARGC = NmVar_New("ARGC", NmInt_New(ARGV_count));
  NmVar_SETFLAG(ARGC, NMVAR_FLAG_CONST);
  /* set it's members */
  Variable *ARGV = NmVar_New("ARGV", NmArray_New(ARGV_count));
  for (int i = 0; i < ARGV_count; i++){
    NmArray_SETELEM(ARGV->value, i, NmString_New(argv[i + optind]));
  }
  NmVar_SETFLAG(ARGV, NMVAR_FLAG_CONST);

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
  included_tidyup();

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

  printf("Welcome to the \e[1;34mNemo\e[0;0m " VERSION " interactive!\n");
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

BOOL Nm_UseModule(char *name, char *path)
{
  BOOL ret;

  NmScope_New(name);
  ret = Nm_IncludeModule(name, path);
  NmScope_Restore();

  return ret;
}

/*
 * Search paths: (the higher the firster (hehe))
 *
 *   - <path>
 *   - ./
 *   - /usr/lib/nemo
 *
 * Return FALSE if the library couldn't be found
 *        TRUE  if the library loaded fine
 *
 */
BOOL Nm_IncludeModule(char *name, char *path)
{
  /*
   * NOTE: both macros goto included which is at the very end of the function
   */

/* DRY, include a C library */
#define INCLUDE_SO(PATH) do { \
  /* fetching the library */ \
  void  *handle; \
  void (*lib_init)(void); \
  char  *error; \
\
  handle = dlopen(PATH, RTLD_LAZY); \
  if (!handle){ \
    NmError_SetString("%s", dlerror()); \
    return FALSE; \
  } \
\
  dlerror(); /* clear any existing error */ \
\
  lib_init = dlsym(handle, init_func); \
\
  if ((error = dlerror()) != NULL){ \
    NmError_SetString("%s", error); \
    return FALSE; \
  } \
\
  lib_init(); \
  ADD_HANDLE(handle); \
  fclose(fp); \
  goto included; \
} while (0);

/* DRY, include a Nemo file */
#define INCLUDE_NM(PATH) do { \
  Node *nodest = NmParser_ParseFile(PATH); \
  NmAST_Exec(nodest); \
  NmAST_Free(nodest); \
  fclose(fp); \
  goto included; \
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
  /* custom path */
  char custom_path[64];
  /* custom path with name.so appended */
  char custom_path_so[64];
  /* custom path with name.nm appended */
  char custom_path_nm[64];
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
  sprintf(lib_path_so, LIBDIR "/nemo/%s.so",    name);
  sprintf(lib_path_nm, LIBDIR "/nemo/%s.nm",    name);
  sprintf(custom_path,        "%s/",      path);
  sprintf(custom_path_so,     "%s/%s.so", path, name);
  sprintf(custom_path_nm,     "%s/%s.nm", path, name);
  sprintf(relative_path,      "%s/",      cwd);
  sprintf(relative_path_so,   "%s/%s.so", cwd,  name);
  sprintf(relative_path_nm,   "%s/%s.nm", cwd,  name);
  sprintf(init_func,          "%s_init",        name);

  /* there is a .so library in the custom path */
  if ((fp = fopen(custom_path_so, "rb")) != NULL){
    INCLUDE_SO(custom_path_so);
  /* there is a Nemo file in the custom path */
  } else if ((fp = fopen(custom_path_nm, "r")) != NULL){
    INCLUDE_NM(custom_path_nm);
  }

  /* there is a .so library in the current directory */
  if ((fp = fopen(relative_path_so, "rb")) != NULL){
    INCLUDE_SO(relative_path_so);
  /* there is a Nemo file in the current directory */
  } else if ((fp = fopen(relative_path_nm, "r")) != NULL){
    INCLUDE_NM(relative_path_nm);
  }

  /* found a .so library in the LIBDIR directory */
  if ((fp = fopen(lib_path_so, "rb")) != NULL){
    INCLUDE_SO(lib_path_so);
  /* found a Nemo file in the LIBDIR */
  } else if ((fp = fopen(lib_path_nm, "r")) != NULL){
    INCLUDE_NM(lib_path_nm);
  }

  free(cwd); /* getcwd does malloc */
  if (path)
    NmError_SetString("couldn't find module '%s' in any of the following: %s, %s, %s", name, custom_path, relative_path, lib_path);
  else
    NmError_SetString("couldn't find module '%s' neither in %s or %s", name, relative_path, lib_path);

  return FALSE;

included: {
    free(cwd); /* getcwd does malloc */
    /* add the modules name to the included list */
    included_new(name);
    return TRUE;
  }
}

/*
 * Searches the list of included modules and return TRUE if encountered one
 * called <name>, FALSE if not
 */
BOOL NmModule_WasIncluded(char *name)
{
  for (Included *module = included; module != NULL; module = module->next)
    if (!strcmp(name, module->name)){
      printf("checking %s == %s\n", name, module->name);
      return TRUE;
    }

  return FALSE;
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

