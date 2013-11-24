Nemo
****

Nemo is a scripting, interpreted,
dynamically and strongly typed programming language written in ANSI C.

**Note** I'm rewriting Nemo from the ground up. There's not much to see right now.

Build status
============

.. image:: https://travis-ci.org/semahawk/nemo.png?branch=master
   :target: https://travis-ci.org/semahawk/nemo

Install
=======

**1** Obtain the sources

.. code-block:: sh

    $ git clone https://github.com/semahawk/nemo.git

**2** Compile

.. code-block:: sh

    $ cmake . \
        [-DDEBUG=ON|YES|1] \
        [-DBINDIR=/where/to/install/binaries]  \
        [-DINCDIR=/where/to/install/headers]   \
        [-DLIBDIR=/where/to/install/libraries] \
    $ make
    $ sudo make install

The default values for the specific options are:

.. code-block:: sh

    DEBUG:  0
    BINDIR: "/usr/local/bin"
    INCDIR: "/usr/local/include"
    LIBDIR: "/usr/local/lib"

Dependencies
============

* C90 compatible C compiler
* CMake
* make

What's what
===========

+------------------+---------------------------------------------------------+
| cmake/           | CMake build-only related files                          |
+------------------+---------------------------------------------------------+
| ext/             | External files/libraries, sometimes replacements for    |
|                  | existing libc functions                                 |
+------------------+---------------------------------------------------------+
| ast.c            | AST related stuff - node creation, execution etc.       |
+------------------+---------------------------------------------------------+
| lexer.c          | The lexer, tokenizing, keywords list etc.               |
+------------------+---------------------------------------------------------+
| mem.c            | Basically just malloc/realloc/calloc wrappers           |
+------------------+---------------------------------------------------------+
| nemo.c           | The main file of the executable, contains main()        |
+------------------+---------------------------------------------------------+
| parser.c         | The parser and all the grammar                          |
+------------------+---------------------------------------------------------+
| utf8.c           | Handful of handy functions to help with UTF-8           |
+------------------+---------------------------------------------------------+
| util.c           | Handful of handy functions to help with anything else   |
+------------------+---------------------------------------------------------+

License
=======

This code is licensed under the New / Modified (3 clause) BSD License.
For more details, please visit the LICENSE file.

