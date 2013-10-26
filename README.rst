Nemo
****

Nemo is a scripting, interpreted,
dynamically and strongly typed programming language written entirely in C.

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
        [-DDEBUG=ON] \
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

* C compiler with C99 support (tested on GCC and Clang)
* cmake
* make

License
=======

Copyright (c) 2012-2013 Szymon Urbaś <szymon.urbas@aol.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

