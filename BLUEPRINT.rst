On how I see Nemo would look like
=================================

* Ifs/unlesses and whiles/untils actually expect a statement, instead of an
  expression, like pretty much every other language.
* Statements end with a semicolon (unless it's last statement in the block)
* Comments start with the sign ``#`` or as a block, enclosed with ``/*`` and ``*/``
* Basic functions are actually keywords (still thinking about it)(additionally, no need for parenthesises)
* Functions can take shell-like options, eg. -n
* Support for metaprogramming

Declaring a variable
--------------------

.. code-block:: nemo

   my var;

Initializing a variable
-----------------------

.. code-block:: nemo

   my var = 6;

Block are enclosed with ``{`` and ``}``
---------------------------------------

Statements in blocks
--------------------

When a statement is the last one in a block (or in the whole script), it doesn't have to be followed by a
semicolon, eg.

.. code-block:: nemo

   my var1 = 2;
   my var2 = 4

   # or

   {
     my var1 = 2;
     my var2 = 4
   }

   # or

   my var1 = 2;
   my var2 = 4

   {
     # some stuff
   }

Defining a function
-------------------

.. code-block:: nemo

.. code-block:: nemo

   fun puts # put string
       -n   # prints the newline
       msg  # the message to be printed
   {
     print msg;
     unless n {
       print "\n";
     }
   }

   fun sort # sorts a given array
       -r   # reverse order
       arr  # the array
   {
     if r {
       # sort in reverse order
     } else {
       # sort normally
     }
   }

   # Here, puts is the function name, and the option -n is supplied
   puts-n("sorted array: ");
   # Here, sort is the function name, and the options -r and -q is supplied
   my array = [4, 2, 3, 6, 10];
   puts(sort-rq(array));

   # And, if the parenthesisless thing works out, the above could be
   # written like this:
   puts sort-rq array;

If
--

.. code-block:: nemo

   if <stmt>
     <stmt>

   if five == 5;
     print "five is equal 5";

While
-----

.. code-block:: nemo

   while <stmt>
     <stmt>

   while a < 10; a++;

   while { my a = 2; 1 } print "hello, world\n"

Metaprogramming
---------------

.. code-block:: nemo

   #define true  1
   #define false 0

   #if 0
     Goodbye!
   #endif

Yup, they look a lot like comments, but it's gonna look nice when there are
comments around preprocessor declarations.

