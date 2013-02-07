//
// nemo.h
//
// Copyright: (c) 2012 by Szymon Urbaś <szymon.urbas@aol.com>
//

#ifndef NEMO_H
#define NEMO_H

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <assert.h>
#include <time.h>
#include <sys/stat.h>

// it's actually 10, as we start from 0
//
// booyah!
#define MAX_EVAL_FLAGS 9
// version of Nemo, obviously
#define VERSION "0.13.1"

struct Node *parseFile(char *);
struct Node *parseString(char *);

void version(void);

#endif // NEMO_H
