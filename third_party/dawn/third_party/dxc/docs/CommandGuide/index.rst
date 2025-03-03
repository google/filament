LLVM Command Guide
------------------

NOTE: the DirectX Compiler project drops support for many of these tools, as
they are no longer suitable or have a more specialized replacement.

The following documents are command descriptions for all of the LLVM tools.
These pages describe how to use the LLVM commands and what their options are.
Note that these pages do not describe all of the options available for all
tools. To get a complete listing, pass the ``--help`` (general options) or
``--help-hidden`` (general and debugging options) arguments to the tool you are
interested in.

Basic Commands
~~~~~~~~~~~~~~

.. toctree::
   :maxdepth: 1

   llvm-as
   llvm-dis
   opt
   llc
   lli
   llvm-link
   llvm-ar
   llvm-nm
   llvm-config
   llvm-diff
   llvm-cov
   llvm-profdata
   llvm-stress
   llvm-symbolizer
   llvm-dwarfdump

Debugging Tools
~~~~~~~~~~~~~~~

.. toctree::
   :maxdepth: 1

   llvm-extract
   llvm-bcanalyzer

Developer Tools
~~~~~~~~~~~~~~~

.. toctree::
   :maxdepth: 1

   FileCheck
   tblgen
   lit
   llvm-build
   llvm-readobj
