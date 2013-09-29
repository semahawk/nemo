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

static bool running_interactive = false;
static int nmInteractive(void);

/* list of all modules that were included */
/* by included it's meant that were include-d or use-d */
static Included *included = NULL;

/* list of all the library handles that need to be dlclosed */
static LibHandlesList *handles = NULL;
/* a small tiny handy macro to add a handle to the list */
#define ADD_HANDLE(handle) do { \
  LibHandlesList *new_list = nmalloc(sizeof(LibHandlesList)); \
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
    nfree(list); \
  } \
} while (0);

/* here are alllll the things that should be done at the exit */
#define CLEANUP() \
  nm_namespace_cleanup(); \
  CLOSE_HANDLES(); \
  gc_sweepall(); \
  arg_stack_cleanup(); \
  included_tidyup();

/*
 * Add another position in the included list
 */
static inline void included_new(char *name)
{
  Included *new = nmalloc(sizeof(Included));
  new->name = nm_strdup(name);
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
    nfree(curr->name);
    nfree(curr);
  }
}

int main(int argc, char *argv[])
{
  /* the main node from parsing the given file */
  Node *nodest = NULL;
  /* file input */
  char input[255];
  /* used for getopt */
  int c;
  /* if the --eval flag was supplied, this gets set to true and the interactive
   * mode is not run when an argument was not supplied */
  bool met_e_flag = false;

  /* initialize the garbage collector */
  gc_init();
  /* the kind of global namespace where all the predefined functions etc. reside */
  nm_new_namespace("core");
  /* create the "null" variable */
  nm_new_var("null", null);
  /* fetch the predefined functions */
  predef_init();
  /* the main file's namespace */
  nm_new_namespace("main");

  while (1){
    static struct option long_options[] = {
      { "use",     required_argument, 0, 'u' },
      { "eval",    required_argument, 0, 'e' },
#if DEBUG
      { "debug",   required_argument, 0, 'd' },
#endif
      { "version", no_argument,       0, 'v' },
      { 0, 0, 0, 0 }
    };

    int option_index = 0;

#if DEBUG
    c = getopt_long(argc, argv, "u:i:e:d:v", long_options, &option_index);
#else
    c = getopt_long(argc, argv, "u:i:e:v", long_options, &option_index);
#endif

    if (c == -1)
      break;

    switch (c){
      case 'u':
      {
        nm_use_module(optarg);
        break;
      }
      case 'e':
      {
        met_e_flag = true;
        /* parse the string */
        Node *node = nm_parse_string(optarg);
        /* execute the nodes */
        nm_ast_exec_block(node);
        /* tidy up after executing */
        nm_ast_free_block(node);
        break;
      }
#if DEBUG
      case 'd':
      {
        switch (*optarg){
          case 'h': printf("\nDebug options include:\n");
                    printf("  a   abstract syntax tree nodes creation and deallocation\n");
                    printf("  m   memory allocation, deallocation and so-on\n");
                    printf("  p   parser messages\n");
                    printf("  l   lexer messages\n\n");
                    return EXIT_SUCCESS;
          case 'a': nm_debug_set_flag(DEBUG_FLAG_AST);
                    break;
          case 'l': nm_debug_set_flag(DEBUG_FLAG_LEXER);
                    break;
          case 'm': nm_debug_set_flag(DEBUG_FLAG_MEMORY);
                    break;
          case 'p': nm_debug_set_flag(DEBUG_FLAG_PARSER);
                    break;
          default:  nm_error("unknown option for debug '%c', run with '-dh' to see the possible options", *optarg);
                    return EXIT_FAILURE;
        }
        break;
      }
#endif
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
  Variable *ARGC = nm_new_var("ARGC", nm_new_int(ARGV_count));
  nm_var_set_flag(ARGC, NMVAR_FLAG_CONST);
  /* set it's members */
  Variable *ARGV = nm_new_var("ARGV", nm_new_arr(ARGV_count));
  for (int i = 0; i < ARGV_count; i++){
    nm_arr_set_elem(ARGV->value, i, nm_new_str(argv[i + optind]));
  }
  nm_var_set_flag(ARGV, NMVAR_FLAG_CONST);

  /* parse the file */
  nodest = nm_parse_file(input);
  /* execute the nodes */
  nm_ast_exec_block(nodest);
  /* tidy up after executing */
  nm_ast_free_block(nodest);

  /* tidy up */
  CLEANUP();

  return EXIT_SUCCESS;
}

static int nmInteractive(void)
{
  /* users input, Nemo's prompt */
  char *input, prompt[64];
  /* current line user has entered */
  unsigned line = 0;
  /* result of the executed input */
  Nob *ob;
  /* set the running_interactive flag */
  running_interactive = true;

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
      printf("=> Have a good day!\n");
      return 0;
    }

    ob = nm_ast_exec_block(nm_parse_string(input));

    printf("=> ");
    nm_obj_print(stdout, ob);
    printf("\n");
  }


  return 0;
}

void nm_init_module(NmModuleFuncs *funcs)
{
  Namespace *namespace = nm_curr_namespace();

  /* append all the C functions to the cfuncs */
  for (NmModuleFuncs *f = funcs; f->name != NULL; f++){
    CFuncsList *list = nmalloc(sizeof(CFuncsList));
    CFunc *func = nmalloc(sizeof(CFunc));

    func->name = nm_strdup(f->name);
    func->body = f->ptr;
    func->argc = f->argc;
    func->opts = f->opts;
    func->types = f->types;
    list->func = func;
    /* append to the list */
    list->next = namespace->cfuncs;
    namespace->cfuncs = list;
  }
}

/*
 * Search paths: (the higher the firster (hehe))
 *
 *   - ./
 *   - ${LIBDIR}/nemo
 *
 * Return false if the library couldn't be found
 *        true  if the library loaded fine
 *
 */
bool nm_use_module(char *name)
{
  nm_new_namespace(name);
  /* make a copy of the name, and convert all the '.'s into '/' */
  char *name_converted = nm_strdup(name);
  for (char *p = name_converted; *p != '\0'; p++)
    if (*p == '.')
      *p = '/';

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
    nm_set_error("%s", dlerror()); \
    return false; \
  } \
\
  dlerror(); /* clear any existing error */ \
\
  lib_init = dlsym(handle, init_func); \
\
  if ((error = dlerror()) != NULL){ \
    nm_set_error("%s", error); \
    return false; \
  } \
\
  lib_init(); \
  ADD_HANDLE(handle); \
  fclose(fp); \
  goto included; \
} while (0);

/* DRY, include a Nemo file */
#define INCLUDE_NM(PATH) do { \
  Node *nodest = nm_parse_file(PATH); \
  nm_ast_exec_block(nodest); \
  nm_ast_free(nodest); \
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
  sprintf(lib_path_so, LIBDIR "/nemo/%s.so",    name_converted);
  sprintf(lib_path_nm, LIBDIR "/nemo/%s.nm",    name_converted);
  sprintf(relative_path,      "%s/",      cwd);
  sprintf(relative_path_so,   "%s/%s.so", cwd,  name_converted);
  sprintf(relative_path_nm,   "%s/%s.nm", cwd,  name_converted);
  sprintf(init_func,          "%s_init",        name_converted);

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
  free(name_converted);
  nm_set_error("couldn't find module '%s' neither in %s or %s", name, relative_path, lib_path);

  return false;

included: {
    free(cwd); /* getcwd does malloc */
    free(name_converted);
    /* add the modules name to the included list */
    included_new(name);
    nm_restore_namespace();
    return true;
  }
}

/*
 * Searches the list of included modules and return true if encountered one
 * called <name>, false if not
 */
bool NmModule_WasIncluded(char *name)
{
  for (Included *module = included; module != NULL; module = module->next)
    if (!strcmp(name, module->name)){
      return true;
    }

  return false;
}

/*
 * Searches for the given <name> if it exists as a variable or a function.
 *
 * Return: The following bits get ORed and that value gets returned
 *
 *              0 if not found
 *         1 << 0 if found a variable
 *         1 << 1 if found a C function
 *         1 << 2 if found a defined Nemo function
 *         1 << 3 if found a declared Nemo function
 *
 * Param: <namespace> - in which namespace to look in
 *                      if NULL the current namespace would be used
 */
int name_lookup(char *name, Namespace *namespace)
{
  int ret = 0;
  Namespace *namespaces[3];
  namespaces[0] = nm_get_namespace_by_name("core");
  namespaces[1] = namespace != NULL ? namespace : nm_curr_namespace();
  namespaces[2] = NULL;

  for (int i = 0; namespaces[i] != NULL; i++){
    namespace = namespaces[i];
    /* search for a global variable in the given namespace */
    for (VariablesList *vars = namespace->globals; vars != NULL; vars = vars->next){
      if (!strcmp(vars->var->name, name)){
        ret |= 1 << 0;
      }
    }
    /* search for the C functions */
    for (CFuncsList *cfuncs = namespace->cfuncs; cfuncs != NULL; cfuncs = cfuncs->next){
      if (!strcmp(cfuncs->func->name, name)){
        ret |= 1 << 1;
      }
    }
    /* search the user defined functions */
    for (FuncsList *funcs = namespace->funcs; funcs != NULL; funcs = funcs->next){
      if (!strcmp(funcs->func->name, name)){
        if (funcs->func->body)
          ret |= 1 << 2;
        else
          ret |= 1 << 3;
      }
    }
  }

  return ret;
}

/*
 * Nemo's wrapper around the exit() function.
 */
void nexit()
{
  if (!running_interactive){
    CLEANUP();
    exit(EXIT_FAILURE);
  } else {
    ;
  }
}

/*
 * Megadeth, Running Wild, Gamma Ray, Iron Savior
 * Helloween, Testament
 * Within Temptation, Nightwish, Avantasia
 * Stratovarius, Steve Vai, At Vance, Rhapsody of Fire
 * Fear Factory
 *
 * Family Guy, The Office, Monty Python
 *
 */

