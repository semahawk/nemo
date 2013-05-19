On how I see Nemo would look like
=================================

* Statements end with a semicolon (unless it's last statement in the block)
* Comments start with the sign ``#`` or as a block, enclosed with ``/*`` and ``*/``
* Basic functions are actually keywords (still thinking about it)(additionally, no need for parenthesises)
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

   fn hello(){
     print "hello, world!";
   }

.. code-block:: nemo

   fn hello(name){
     print "hello, " + name + "!\n";
   }

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

Documentation in comments
-------------------------

.. code-block:: nemo

    #
    # \name    greet
    # \param   name    name of the person to be greeted
    # \return  null
    #
    fn greet(name){
      print "hello, " + name + "!";
    }

