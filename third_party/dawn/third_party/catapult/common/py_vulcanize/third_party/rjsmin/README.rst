.. -*- coding: utf-8 -*-

===========================================
 rJSmin - A Javascript Minifier For Python
===========================================

TABLE OF CONTENTS
-----------------

1. Introduction
2. Copyright and License
3. System Requirements
4. Installation
5. Documentation
6. Bugs
7. Author Information


INTRODUCTION
------------

rJSmin is a javascript minifier written in python.

The minifier is based on the semantics of `jsmin.c by Douglas Crockford`_\.

The module is a re-implementation aiming for speed, so it can be used at
runtime (rather than during a preprocessing step). Usually it produces the
same results as the original ``jsmin.c``. It differs in the following ways:

- there is no error detection: unterminated string, regex and comment
  literals are treated as regular javascript code and minified as such.
- Control characters inside string and regex literals are left untouched; they
  are not converted to spaces (nor to \\n)
- Newline characters are not allowed inside string and regex literals, except
  for line continuations in string literals (ECMA-5).
- "return /regex/" is recognized correctly.
- Line terminators after regex literals are handled more sensibly
- "+ +" and "- -" sequences are not collapsed to '++' or '--'
- Newlines before ! operators are removed more sensibly
- Comments starting with an exclamation mark (``!``) can be kept optionally
- rJSmin does not handle streams, but only complete strings. (However, the
  module provides a "streamy" interface).

Since most parts of the logic are handled by the regex engine it's way faster
than the original python port of ``jsmin.c`` by Baruch Even. The speed factor
varies between about 6 and 55 depending on input and python version (it gets
faster the more compressed the input already is).  Compared to the
speed-refactored python port by Dave St.Germain the performance gain is less
dramatic but still between 3 and 50 (for huge inputs). See the docs/BENCHMARKS
file for details.

rjsmin.c is a reimplementation of rjsmin.py in C and speeds it up even more.

.. _jsmin.c by Douglas Crockford: http://www.crockford.com/javascript/jsmin.c


COPYRIGHT AND LICENSE
---------------------

Copyright 2011 - 2015
André Malo or his licensors, as applicable.

The whole package (except for the files in the bench/ directory)
is distributed under the Apache License Version 2.0. You'll find a copy in the
root directory of the distribution or online at:
<http://www.apache.org/licenses/LICENSE-2.0>.


SYSTEM REQUIREMENTS
-------------------

Both python 2 (>=2.4) and python 3 are supported.


INSTALLATION
------------

Using pip
~~~~~~~~~

$ pip install rjsmin


Using distutils
~~~~~~~~~~~~~~~

$ python setup.py install

The following extra options to the install command may be of interest:

  --without-c-extensions  Don't install C extensions
  --without-docs          Do not install documentation files


Drop-in
~~~~~~~

rJSmin effectively consists of two files: rjsmin.py and rjsmin.c, the
latter being entirely optional. So, for simple integration you can just
copy rjsmin.py into your project and use it.


DOCUMENTATION
-------------

A generated API documentation is available in the docs/apidoc/ directory.
But you can just look into the module. It provides a simple function,
called jsmin which takes the script as a string and returns the minified
script as a string.

The module additionally provides a "streamy" interface similar to the one
jsmin.c provides:

$ python -mrjsmin <script >minified

It takes two options:

  -b  Keep bang-comments (Comments starting with an exclamation mark)
  -p  Force using the python implementation (not the C implementation)

The latest documentation is also available online at
<http://opensource.perlig.de/rjsmin/>.


BUGS
----

No bugs, of course. ;-)
But if you've found one or have an idea how to improve rjsmin, feel free
to send a pull request on `github <https://github.com/ndparker/rjsmin>`_
or send a mail to <rjsmin-bugs@perlig.de>.


AUTHOR INFORMATION
------------------

André "nd" Malo <nd@perlig.de>
GPG: 0x8103A37E


  If God intended people to be naked, they would be born that way.
                                                   -- Oscar Wilde
