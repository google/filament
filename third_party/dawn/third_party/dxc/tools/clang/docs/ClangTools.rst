========
Overview
========

NOTE: this document applies to the original Clang project, not the DirectX
Compiler. It's made available for informational purposes only.

Clang Tools are standalone command line (and potentially GUI) tools
designed for use by C++ developers who are already using and enjoying
Clang as their compiler. These tools provide developer-oriented
functionality such as fast syntax checking, automatic formatting,
refactoring, etc.

Only a couple of the most basic and fundamental tools are kept in the
primary Clang Subversion project. The rest of the tools are kept in a
side-project so that developers who don't want or need to build them
don't. If you want to get access to the extra Clang Tools repository,
simply check it out into the tools tree of your Clang checkout and
follow the usual process for building and working with a combined
LLVM/Clang checkout:

-  With Subversion:

   -  ``cd llvm/tools/clang/tools``
   -  ``svn co http://llvm.org/svn/llvm-project/clang-tools-extra/trunk extra``

-  Or with Git:

   -  ``cd llvm/tools/clang/tools``
   -  ``git clone http://llvm.org/git/clang-tools-extra.git extra``

This document describes a high-level overview of the organization of
Clang Tools within the project as well as giving an introduction to some
of the more important tools. However, it should be noted that this
document is currently focused on Clang and Clang Tool developers, not on
end users of these tools.

Clang Tools Organization
========================

Clang Tools are CLI or GUI programs that are intended to be directly
used by C++ developers. That is they are *not* primarily for use by
Clang developers, although they are hopefully useful to C++ developers
who happen to work on Clang, and we try to actively dogfood their
functionality. They are developed in three components: the underlying
infrastructure for building a standalone tool based on Clang, core
shared logic used by many different tools in the form of refactoring and
rewriting libraries, and the tools themselves.

The underlying infrastructure for Clang Tools is the
:doc:`LibTooling <LibTooling>` platform. See its documentation for much
more detailed information about how this infrastructure works. The
common refactoring and rewriting toolkit-style library is also part of
LibTooling organizationally.

A few Clang Tools are developed along side the core Clang libraries as
examples and test cases of fundamental functionality. However, most of
the tools are developed in a side repository to provide easy separation
from the core libraries. We intentionally do not support public
libraries in the side repository, as we want to carefully review and
find good APIs for libraries as they are lifted out of a few tools and
into the core Clang library set.

Regardless of which repository Clang Tools' code resides in, the
development process and practices for all Clang Tools are exactly those
of Clang itself. They are entirely within the Clang *project*,
regardless of the version control scheme.

Core Clang Tools
================

The core set of Clang tools that are within the main repository are
tools that very specifically complement, and allow use and testing of
*Clang* specific functionality.

``clang-check``
---------------

:doc:`ClangCheck` combines the LibTooling framework for running a
Clang tool with the basic Clang diagnostics by syntax checking specific files
in a fast, command line interface. It can also accept flags to re-display the
diagnostics in different formats with different flags, suitable for use driving
an IDE or editor. Furthermore, it can be used in fixit-mode to directly apply
fixit-hints offered by clang. See :doc:`HowToSetupToolingForLLVM` for
instructions on how to setup and used `clang-check`.

``clang-format``
~~~~~~~~~~~~~~~~

Clang-format is both a :doc:`library <LibFormat>` and a :doc:`stand-alone tool
<ClangFormat>` with the goal of automatically reformatting C++ sources files
according to configurable style guides.  To do so, clang-format uses Clang's
``Lexer`` to transform an input file into a token stream and then changes all
the whitespace around those tokens.  The goal is for clang-format to serve both
as a user tool (ideally with powerful IDE integrations) and as part of other
refactoring tools, e.g. to do a reformatting of all the lines changed during a
renaming.

``clang-modernize``
~~~~~~~~~~~~~~~~~~~
``clang-modernize`` migrates C++ code to use C++11 features where appropriate.
Currently it can:

* convert loops to range-based for loops;

* convert null pointer constants (like ``NULL`` or ``0``) to C++11 ``nullptr``;

* replace the type specifier in variable declarations with the ``auto`` type specifier;

* add the ``override`` specifier to applicable member functions.

Extra Clang Tools
=================

As various categories of Clang Tools are added to the extra repository,
they'll be tracked here. The focus of this documentation is on the scope
and features of the tools for other tool developers; each tool should
provide its own user-focused documentation.

Ideas for new Tools
===================

* C++ cast conversion tool.  Will convert C-style casts (``(type) value``) to
  appropriate C++ cast (``static_cast``, ``const_cast`` or
  ``reinterpret_cast``).
* Non-member ``begin()`` and ``end()`` conversion tool.  Will convert
  ``foo.begin()`` into ``begin(foo)`` and similarly for ``end()``, where
  ``foo`` is a standard container.  We could also detect similar patterns for
  arrays.
* ``make_shared`` / ``make_unique`` conversion.  Part of this transformation
  can be incorporated into the ``auto`` transformation.  Will convert

  .. code-block:: c++

    std::shared_ptr<Foo> sp(new Foo);
    std::unique_ptr<Foo> up(new Foo);

    func(std::shared_ptr<Foo>(new Foo), bar());

  into:

  .. code-block:: c++

    auto sp = std::make_shared<Foo>();
    auto up = std::make_unique<Foo>(); // In C++14 mode.

    // This also affects correctness.  For the cases where bar() throws,
    // make_shared() is safe and the original code may leak.
    func(std::make_shared<Foo>(), bar());

* ``tr1`` removal tool.  Will migrate source code from using TR1 library
  features to C++11 library.  For example:

  .. code-block:: c++

    #include <tr1/unordered_map>
    int main()
    {
        std::tr1::unordered_map <int, int> ma;
        std::cout << ma.size () << std::endl;
        return 0;
    }

  should be rewritten to:

  .. code-block:: c++

    #include <unordered_map>
    int main()
    {
        std::unordered_map <int, int> ma;
        std::cout << ma.size () << std::endl;
        return 0;
    }

* A tool to remove ``auto``.  Will convert ``auto`` to an explicit type or add
  comments with deduced types.  The motivation is that there are developers
  that don't want to use ``auto`` because they are afraid that they might lose
  control over their code.

* C++14: less verbose operator function objects (`N3421
  <http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3421.htm>`_).
  For example:

  .. code-block:: c++

    sort(v.begin(), v.end(), greater<ValueType>());

  should be rewritten to:

  .. code-block:: c++

    sort(v.begin(), v.end(), greater<>());

