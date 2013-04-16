On how I see Nemo would look like
---------------------------------

+ Statements end with a semicolon (unless it's last statement in the block)
+ Everything is an expression
+ Comments start with the sign `#` or as a block, enclosed with `/*` and `*/`
+ Integers, floats and strings are scalars (Perl-like)
+ Basic functions are actually keywords (additionally, no need for parenthesises)
+ More technically, it would be stack based, Perl-like
+ Support for metaprogramming

# Declaring a variable

    my var;

# Initializing a variable

    my var = 6;

# Block are enclosed with `{` and `}`

## Statements in blocks

When a statement is a last one in a block (or in the whole script), it doesn't have to be followed by a
semicolon, eg.

    my var1 = 2;
    my var2 = 4

    ##

    {
      my var1 = 2;
      my var2 = 4
    }

    ##

    my var1 = 2;
    my var2 = 4

    {
      # some stuff
    }

You also don't need a semicolon when that statement is followed by any control
statement like "if" or "while". (we'll see later what else wouldn't need that
semicolon)

# Defining a function

    fn hello {
      print "hello, world!";
    }

When you pass an argument to the function, it gets pushed onto the stack, and
then, to get it inside of the function, you have to pop/shift it (Perl-like)

    fn hello {
      my name = pop; # or shift, we'll see later
      print "hello, " . name . "\n";
    }

# If

    if <stmt>
      <stmt>


    if five == 5;
      print "five is equal 5";

# While

    while <stmt>
      <stmt>


    while a < 10; a++;
    
    while { my a = 2; 1 } print "hello, world\n"

# "Everything is an expression"

    my var = if a == 1; { 4 } else { 8 };

# Metaprogramming

    %define true  1
    %define false 0

    %if 0
      Goodbye!
    %endif

# Documentation in comments

    #
    # @name    greet
    # @param   name    name of the person to be greeted
    # @return  none
    #
    fn greet {
      my name = pop;
      print "hello, " . name . "!";
    }

