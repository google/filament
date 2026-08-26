Overview
========

The DirectX HLSL Compiler is a compiler and related set of tools used to
compile High-Level Shader Language (HLSL) programs into DirectX Intermediate
Language (DXIL) representation. Applications that make use of DirectX for
graphics, games, and computation can use it to generate shader programs. The
DirectX HLSL Compiler is built on the 3.7 releases of LLVM and Clang.

The LLVM compiler infrastructure supports a wide range of projects, from
industrial strength compilers to specialized JIT applications to small
research projects.

Similarly, documentation is broken down into several high-level groupings
targeted at different audiences:

LLVM Design & Overview
======================

Several introductory papers and presentations.

.. toctree::
   :hidden:

   LangRef
   DXIL
   HLSLChanges

:doc:`LangRef`
  Defines the LLVM intermediate representation.

:doc:`DXIL`
  Defines the DirectX Intermediate Language (DXIL) for GPU shaders.

`Introduction to the LLVM Compiler`__
  Presentation providing a users introduction to LLVM.

  .. __: http://llvm.org/pubs/2008-10-04-ACAT-LLVM-Intro.html

`Intro to LLVM`__
  Book chapter providing a compiler hacker's introduction to LLVM.

  .. __: http://www.aosabook.org/en/llvm.html


`LLVM: A Compilation Framework for Lifelong Program Analysis & Transformation`__
  Design overview.

  .. __: http://llvm.org/pubs/2004-01-30-CGO-LLVM.html

:doc:`HLSLChanges`
  Describes high-level changes made to LLVM and Clang to accomodate HLSL and DXIL.

`LLVM: An Infrastructure for Multi-Stage Optimization`__
  More details (quite old now).

  .. __: http://llvm.org/pubs/2002-12-LattnerMSThesis.html


User Guides
===========

For those new to the LLVM system.

The documentation here is intended for users who have a need to work with the
intermediate LLVM representation.

.. toctree::
   :hidden:

   CMake
   CommandGuide/index
   Lexicon
   Passes
   YamlIO
   GetElementPtr
   Frontend/PerformanceTips

:doc:`CMake`
   An addendum to the main Getting Started guide for those using the `CMake
   build system <http://www.cmake.org>`_.

:doc:`LLVM Command Guide <CommandGuide/index>`
   A reference manual for the LLVM command line utilities ("man" pages for LLVM
   tools).

:doc:`Passes`
   A list of optimizations and analyses implemented in LLVM.

`How to build the C, C++, ObjC, and ObjC++ front end`__
   Instructions for building the clang front-end from source.

   .. __: http://clang.llvm.org/get_started.html

:doc:`Lexicon`
   Definition of acronyms, terms and concepts used in LLVM.

:doc:`YamlIO`
   A reference guide for using LLVM's YAML I/O library.

:doc:`GetElementPtr`
  Answers to some very frequent questions about LLVM's most frequently
  misunderstood instruction.

:doc:`Frontend/PerformanceTips`
   A collection of tips for frontend authors on how to generate IR 
   which LLVM is able to effectively optimize.


Programming Documentation
=========================

For developers of applications which use LLVM as a library.

.. toctree::
   :hidden:

   Atomics
   CodingStandards
   CommandLine
   ExtendingLLVM
   HowToSetUpLLVMStyleRTTI
   ProgrammersManual
   LibFuzzer

:doc:`LLVM Language Reference Manual <LangRef>`
  Defines the LLVM intermediate representation and the assembly form of the
  different nodes.

:doc:`Atomics`
  Information about LLVM's concurrency model.

:doc:`ProgrammersManual`
  Introduction to the general layout of the LLVM sourcebase, important classes
  and APIs, and some tips & tricks.

:doc:`CommandLine`
  Provides information on using the command line parsing library.

:doc:`CodingStandards`
  Details the LLVM coding standards and provides useful information on writing
  efficient C++ code.

:doc:`HowToSetUpLLVMStyleRTTI`
  How to make ``isa<>``, ``dyn_cast<>``, etc. available for clients of your
  class hierarchy.

:doc:`ExtendingLLVM`
  Look here to see how to add instructions and intrinsics to LLVM.

:doc:`LibFuzzer`
  A library for writing in-process guided fuzzers.

Subsystem Documentation
=======================

For API clients and LLVM developers.

.. toctree::
   :hidden:

   AliasAnalysis
   BitCodeFormat
   BlockFrequencyTerminology
   BranchWeightMetadata
   CodeGenerator
   ExceptionHandling
   LinkTimeOptimization
   TableGen/index
   MarkedUpDisassembly
   SystemLibrary
   SourceLevelDebugging
   SourceLevelDebuggingHLSL
   Vectorizers
   WritingAnLLVMBackend
   WritingAnLLVMPass
   HowToUseAttributes
   InAlloca
   CoverageMappingFormat
   MergeFunctions
   BitSets
   FaultMaps
   LLVMBuild

:doc:`WritingAnLLVMPass`
   Information on how to write LLVM transformations and analyses.

:doc:`WritingAnLLVMBackend`
   Information on how to write LLVM backends for machine targets.

:doc:`CodeGenerator`
   The design and implementation of the LLVM code generator.  Useful if you are
   working on retargeting LLVM to a new architecture, designing a new codegen
   pass, or enhancing existing components.

:doc:`TableGen <TableGen/index>`
   Describes the TableGen tool, which is used heavily by the LLVM code
   generator.

:doc:`AliasAnalysis`
   Information on how to write a new alias analysis implementation or how to
   use existing analyses.

:doc:`Source Level Debugging with LLVM <SourceLevelDebugging>`
   This document describes the design and philosophy behind the LLVM
   source-level debugger.

:doc:`Source Level Debugging with HLSL <SourceLevelDebuggingHLSL>`
    This document describes specifics of using source-level debuggers for DXIL
    and HLSL.

:doc:`Vectorizers`
   This document describes the current status of vectorization in LLVM.

:doc:`ExceptionHandling`
   This document describes the design and implementation of exception handling
   in LLVM.

:doc:`BitCodeFormat`
   This describes the file format and encoding used for LLVM "bc" files.

:doc:`System Library <SystemLibrary>`
   This document describes the LLVM System Library (``lib/System``) and
   how to keep LLVM source code portable

:doc:`LinkTimeOptimization`
   This document describes the interface between LLVM intermodular optimizer
   and the linker and its design

:doc:`BranchWeightMetadata`
   Provides information about Branch Prediction Information.

:doc:`BlockFrequencyTerminology`
   Provides information about terminology used in the ``BlockFrequencyInfo``
   analysis pass.

:doc:`MarkedUpDisassembly`
   This document describes the optional rich disassembly output syntax.

:doc:`HowToUseAttributes`
  Answers some questions about the new Attributes infrastructure.

:doc:`CoverageMappingFormat`
  This describes the format and encoding used for LLVM’s code coverage mapping.

:doc:`MergeFunctions`
  Describes functions merging optimization.

:doc:`InAlloca`
  Description of the ``inalloca`` argument attribute.

:doc:`FaultMaps`
  LLVM support for folding control flow into faulting machine instructions.

:doc:`LLVMBuild`
  Describes the LLVMBuild organization and files used by LLVM to specify
  component descriptions.

Indices and tables
==================

* :ref:`genindex`
* :ref:`search`
