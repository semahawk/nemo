# Nemo

# About

Nemo is a programming language written entirely in C. It uses Flex and Lemon as its lexer and parser.
Nemo is a interpreted, scripting language, dynamically and weakly typed.

# Installing

To install Nemo, just obtain the sources and run old good `make`. So far, it's not to be "installed", it just compiles and just is.
You could also run tests, by typing `make test`, just to make sure it works.

# Dependencies

+ gcc (`4.5.4`)
+ make (`3.82`)
+ flex (`2.5.35`)

Most distributions should have those out of the box, but still.

Nemo has been tested, and ran multiple number of times, using these versions,
and it is confirmed that they work. Older versions may be working, but on my
computer I havy only those.

# Language

## Comments

Nemo supports same set of comments as C.

Multiline `/* ... */` and oneline `// ...`.

## Types

So far Nemo supports integers, floats and strings.

## Variables

Note: in Nemo there's no declaring.

### Dollar

Those plain-old variables start with a dollar sign, eg. `$very_meaningful_variable_name`.

### Bang

If a variable starts with a `!` sign, it means it's read-only. You can set it only once, and can't change it's value later on.

__Note,__ that `$variable` is *different* from `!variable`, they are two, separate variables.

## Strings

Strings are enquoted by `"` or by `'`. If double quotes were used, interpolation
is possible, and will be made. If single quotes were used, that string will be
you will get. WYSIWYG.

So far, strings get casted to integers only. The rule of casting is pretty
simple. Every number found in the string, gets to be in the integer. Everything
else is discarded.

__Examples:__

`"99 bottles of beer"` will get casted to `99`    
`"99 bottles of beer on the wall, take 1 down and pass it around"` will get
casted to `991`    
`"no more bottles of beer on the wall"` will get casted to `0`

## Booleanism

In Nemo, there is no `bool` type. Like in C/Perl, `0`, `0.0` and an empty string `""` is false, the rest is true.

## Operators

Nemo supports few operators. Here is a list of them:

### Mathematical

`+`, `-`, `*`, `/` and `%`

__Note:__ division operator always returns a float, and modulo operator always returns an integer.

### Assignment

Each mathematical operator has its assignment operation, like in C/Perl.

`+=`, `-=`, `*=`, `/=` and `%=`

### Logical

`<`, `>`, `>=`, `<=`, `==` and `!=`

### String specific

`.` - concatention, casts everything to a string

`ne` and `eq` - check for (non-)equality of strings

`gt` and `lt` - check if a string is 'bigger' (or 'smaller') than the other one

`ge` and `le` - check if a string is 'bigger' (or 'smaller') or equall than the other one

`.=` - concatention assignment

### Unary

`+`, `-`, `++` and `--`

## Statements

In Nemo statements end with a semicolon. Like in C.

## Functions

In Nemo function calls must alway include parenthesis.
Also, if function takes 2 arguments, you must pass 2 of them, otherwise an error
would be shown. One exception is the `print` function. You can specify as many
as you want.

Basic syntax of calling a function:

        IDENT '(' (expr [',' expr]+)* ')'
    VAR_IDENT '(' (expr [',' expr]+)* ')'
       STRING '(' (expr [',' expr]+)* ')'

__Examples:__

    print("yarhar!");
    "print"("yarhar!");
    'print'("yarhar!");

    $function = "print";
    $function("yarhar!");

    $function  = "pr";
    $function .= "int";

    $function("yarhar!");

But, this for example __won't__ work:

    "pr" . "int"("yarhar!");

It can't be an expression, just these three mentioned above.

### Predefined

Nemo, so far, has these predefined functions:

#### `print`

Will print any given parameter.

__Examples:__

    print(2);                       // will print: 2
    print(2.71);                    // will print: 2.71
    print("ahoy, sea!");            // will print: "ahoy, sea!"
    print(99 . " bottles of beer"); // will print: "99 bottles of beer"
    print(2, 2.71, "bekon");        // will print: 2, 2.71, "bekon"

#### `assert`

If you pass 1 parameter, it will check if it's true. If it's false, it will exit the whole script.
If you pass 2 parameters, it will check if they are equal. If they are not, it will exit.

__Examples:__

    assert(1);    // will succeed
    assert(1, 1); // will succeed
    assert(0);    // will fail
    assert(0, 0); // will succeed

#### `strlen`

Name says it all. Takes one parameter. Ideally a string, but, anything else
would get casted anyway.

#### `eval`

Takes 1 parameter, and evaluates it. Since our strings have (more or less
working) string interpolation you can't create variables there or do any stuff
like that, as it will obviously be replaced by the variables value.

__Examples:__

    eval("print(2 + 2);"); // will: print 4

### Userdefined

Basic syntax: `FUN IDENT ARGS block`

If `ARGS` is empty, it means function doesn't take any parameters. If it takes,
specify them, and separate with commas, if there's more than one.

__Examples:__

    fun hello {
      print("hello, world");
    }

    fun print_one $a {
      print($a);
    }

    fun add $a, $b {
      return $a + $b;
    }

__Note:__ if an argument is an exclamation variable (like `!blerh`) you won't be able
to change it's value inside of the function.

## Control structures

### If

Basic syntax: `IF expr stmt`

__Examples:__

    if $a < 10
      $a++;

    if $b == 20 {
      $b = 30;
      $c = 40;
    }

### While

Basic syntax: `WHILE expr stmt`

__Examples:__

    while $d < 10 {
      $d++;
      $e += $d;
    }

### For

Basic syntax: `FOR expr; expr; expr stmt`

__Examples:__

    $a = 0;

    for $a = 0; $a < 10; $a++
      print($a);

## Iterators

Nemo supports so-called 'iters'. Well, it's only one of it, but still. The one that we've got is a 'times' iter.

### Times

Basic syntax: `expr TIMES stmt`

__Examples:__

    2 times {
      $a += 10;
    }

Basically, 'times' iter is gonna execute `stmt` `expr` times. Pretty simple.

One thing, that's worth mentioning: there will be created two special variables inside of `stmt`:
`$+` and `$-`. As the loops go, `$+` will start from `0` and will get bigger, till `expr - 1`, and `$-` the other way round. So, given both:

    10 times {
      print($+);
    }

or

    10 times print($+);

It will print:

    0
    1
    2
    3
    4
    5
    6
    7
    8
    9

If it was `$-` instead of `$+`, the output would be from `9` to `0`.

__Note:__ `expr` is being casted to int, so if `expr` is, say, `2.5`, it's gonna be casted to 2.

## Including other files

In Nemo, we have here a `use` statement, which parses given file, and appends
that block (parsing returns a main block node) to the current AST.

Basic syntax: `USE "filename";`

__Example:__

Given files

    // first.nm

    print("first.nm included");

and

    // second.nm

    use "first.nm"

    print("second.nm executed");

and executed file `second.nm` will yield the following output:

    first.nm included
    second.nm executed

# License

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

