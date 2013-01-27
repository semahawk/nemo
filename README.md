# Nemo

## About

Nemo is a programming language written entirely in C. It uses Flex and Bison as its lexer and parser.
Nemo is a interpreted, scripting language, dynamically and weakly typed.

## Installing

To install Nemo, just obtain the sources and run old good `make`. So far, it's not to be "installed", it just compiles and just is.
You could also run tests, by typing `make test`, just to make sure it works.

## Comments

Nemo supports same set of comments as C.

Multiline `/* ... */` and oneline `//`.

## Types

So far Nemo supports integers and floats. Hey, it still in development!

## Variables

Note: in Nemo there's no declaring.

### Dollar

Those plain-old variables start with a dollar sign, eg. `$very_meaningful_variable_name`.

### Exclamation

If a variable starts with a `!` sign, it means it's read-only. You can set it only once, and can't change it's value later on.

## Booleanism

In Nemo, there is no `bool` type. Like in C/Perl, `0` or `0.0` is false, the rest is true.

## Operators

Nemo supports few operators. Here is a list of them:

### Mathematical

`+`, `-`, `*`, `/` and `%`

Note: division operator always returns a float, and modulo operator always returns an integer.

### Assignment

Each mathematical operator has its assignment operation, like in C/Perl.

`+=`, `-=`, `*=`, `/=` and `%=`

### Logical

`<`, `>`, `>=`, `<=`, `==` and `!=`

`lt`, `gt`, `ge`, `le`, `eq` and `ne`

Note: in both cases (like with `>` and `gt`) they are exactly the same.

### Unary

`+`, `-`, `++` and `--`

## Statements

In Nemo statements end with a semicolon. Like in C.

## Functions

In Nemo function calls must alway include parenthesis.

### Predefined

Nemo, so far, has two predefined functions.

#### `print`

Will print any given parameter.

#### `assert`

If you pass 1 parameter, it will check if it's true. If it's false, it will exit the whole script.
If you pass 2 parameters, it will check if they are equal. If they are not, it will exit.

### Userdefined

Basic syntax: `FUN name; ARGS block`

`ARGS` could one of following: `none` keyword (function doesn't take params [well, now I think about it, I will probably change it somehow]) or one or more (separated by commas) variables.

## Control structures

### If

Basic syntax: `IF expr stmt`

Examples:

    if $a < 10
      $a++;

    if $b == 20 {
      $b = 30;
      $c = 40;
    }

### While

Basic syntax: `WHILE expr stmt`

Examples:

    while $d < 10 {
      $d++;
      $e += $d;
    }

## Iters

### Times

Nemo supports so-called iters. Well, it's only one of it, but still. The one that we've got is a 'times' iter.

Basic syntax: `expr TIMES stmt`

Examples:

    2 times {
      $a += 10;
    }

Basically, 'times' iter is gonna execute `stmt` `expr` times. Pretty simple.

One thing, that's worth mentioning: if `stmt` is a block, not a single statement, there will be created two special variables inside of it:
`$+` and `$-`. As the loops go, `$+` will start from `0` and will get bigger, till `expr - 1`, and `$-` the other way round. So, given:

    10 times {
      print($+);
    }

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

Note: `expr` is being casted to int, so if `expr` is, say, `2.5`, it's gonna be casted to 2.

