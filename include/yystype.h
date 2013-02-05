/*
 *
 * yystype.h
 *
 * Created at:  02/04/2013 06:57:08 PM
 *
 * Author:  Szymon Urbas <szymon.urbas@aol.com>
 *
 * License: the MIT license
 *
 */

#ifndef YYSTYPE_H
#define YYSTYPE_H

#ifndef YYSTYPE
  typedef union {
    int i;
    float f;
    char *s;
  } yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif

YYSTYPE yylval;

#endif // YYSTYPE_H

