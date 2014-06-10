Nemo <a href='https://travis-ci.org/semahawk/nemo'><img src='https://travis-ci.org/semahawk/nemo.png?branch=master'></a>
====

Nemo is a scripting, interpreted, statically and strongly typed programming (well, that's the plan) language written in ANSI C.

**Note** I'm rewriting Nemo from the ground up. There's not much to see right now.

Install
=======

**1**. Obtain the sources

```sh
$ git clone https://github.com/semahawk/nemo.git
```

**2**. Compile

```sh
$ cmake . \
    [-DDEBUG=ON|YES|1] \
    [-DBINDIR=/where/to/install/binaries]  \
    [-DINCDIR=/where/to/install/headers]   \
    [-DLIBDIR=/where/to/install/libraries] \
$ make
$ sudo make install
```

The default values for the specific options are:

```sh
DEBUG:  0
BINDIR: "/usr/local/bin"
INCDIR: "/usr/local/include"
LIBDIR: "/usr/local/lib"
```

Dependencies
============

* C99 compatible C compiler
* CMake
* make

What's what
===========

<table>
 <tr>
  <td><code>cmake/</code></td>
  <td>CMake build-only related files</td>
 </tr>
 <tr>
  <td><code>ext/</code></td>
  <td>External files/libraries, sometimes replacements for existing libc functions</td>
 </tr>
 <tr>
  <td><code>ast.c</code></td>
  <td>AST related stuff - node creation, execution etc.</td>
 </tr>
 <tr>
  <td><code>lexer.c</code></td>
  <td>The lexer, tokenizing, keywords list etc.</td>
 </tr>
 <tr>
  <td><code>mem.c</code></td>
  <td>Basically just malloc/realloc/calloc wrappers</td>
 </tr>
 <tr>
  <td><code>nemo.c</code></td>
  <td>The main file of the executable, contains main(), the REPL</td>
 </tr>
 <tr>
  <td><code>parser.c</code></td>
  <td>The (recursive-descent) parser and all the grammar</td>
 </tr>
 <tr>
  <td><code>utf8.c</code></td>
  <td>Handful of handy functions to help with UTF-8</td>
 </tr>
 <tr>
  <td><code>util.c</code></td>
  <td>Handful of handy functions to help with anything else</td>
 </tr>
</table>

License
=======

This code is licensed under the New / Modified (3 clause) BSD License.
For more details, please visit the LICENSE file.

