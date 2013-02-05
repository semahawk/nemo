/*
 *
 * lemon.h
 *
 * Created at:  02/04/2013 05:04:01 PM
 *
 * Author:  Szymon Urbas <szymon.urbas@aol.com>
 *
 * License: the MIT license
 *
 */

#ifndef LEMON_H
#define LEMON_H

#include "yystype.h"

void *ParseAlloc(void * (*)(size_t));
void *Parse(void *, int, YYSTYPE);
void *ParseFree(void *, void(*)(void *));

#endif // LEMON_H

