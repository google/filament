==========================================
Building and Testing DirectXShaderCompiler
==========================================

.. contents::
   :local:
   :depth: 3

.. warning::
   This document describes a new and slightly experimental build process
   building DXC and using LLVM's LIT tool to run DXC's tests. This workflow
   is complete on Linux and Unix platforms, but is incomplete but usable on
   Windows. Instructions for building on Windows are available in the repository
   `readme <https://github.com/microsoft/DirectXShaderCompiler/blob/main/README.md>`_.

Introduction
============

DXC's build is configured using CMake and should be buildable with any preferred
generator. In subsequent sections we will describe workflows using Ninja and
Visual Studio. The Ninja workflow should also apply to makefile generators with
minimal adaption.

Prerequisites
-------------

* [Git](http://git-scm.com/downloads).
* [Python](https://www.python.org/downloads/) - version 3.x is required
* [CMake](https://cmake.org/download/) - version >= 3.17.2
    * The bundled version with Visual Studio works for Windows.
* The C++ 14 compiler and runtime of your choosing.
    * DXC is known to compile with recent versions of GCC, Clang and MSVC.

Building on windows additionally requires:

* [Visual Studio 2019 or later](https://www.visualstudio.com/downloads) - select the following workloads: 
    * Universal Windows Platform Development
    * Desktop Development with C++
* [Windows SDK](https://developer.microsoft.com/en-US/windows/downloads/windows-10-sdk) - version 10.0.18362.0 or newer
* [Windows Driver Kit](https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk) - same version as the SDK

Note: DXC uses submodules for some external dependencies. You must initialize
the submodules in order to build DXC:

.. code-block:: sh
  git submodule update --init --recursive

Basic CMake Usage
-----------------

When building DXC there are a bunch of CMake options which must be set. To
simplify configuration those options are collected into a CMake cache script
which can be passed into the CMake tool using the ``-C`` flag. Cache scripts run
before any other CMake script evaluation and can be used to initialize variables
for the configuration process.

All of the most basic CMake configurations for DXC follow a similar format to:

.. code-block:: sh

  cmake <Repository Root> \
    -C <Repository Root>/cmake/caches/PredefinedParams.cmake \
    -DCMAKE_BUILD_TYPE=<Build Type> \
    -G <Generator>

Useful CMake Flags
------------------

By default CMake will place the generated build output files in the directory
you run CMake in. Our build system disallows running CMake in the root of the
repository so you either need to create an alternate directory to run CMake in,
or you can use the CMake ``-B <path>`` flag to direct CMake to place the
generated files in a different location.

Generating a Visual Studio Solution
-----------------------------------

Open a Visual Stuido command prompt and run:

.. code-block:: sh

  cmake <Repository Root> \
    -B <Path to Output> \
    -C <Repository Root>/cmake/caches/PredefinedParams.cmake \
    -DCMAKE_BUILD_TYPE=<Build Type> \
    -G "Visual Studio 17 2022"

Open the resulting LLVM.sln placed under the ``<Path to Output>``. DXC should
build successfully with either the ``Visual Studio 17 2022`` or ``Visual Studio
16 2019`` generators.

Using Visual Studio's CMake Integration
---------------------------------------

Open Visual Studio and select "Open a local folder". Open the folder DXC is
cloned into. If Visual Studio does not start automatically configuring the build
you may need to go to the "Project" menu and select "CMake Workspace Settings"
and add ``"enableCMake"; true`` to the JSON configuration file.

After CMake configuration completes you should be able to toggle the "Solution
Explorer" into "CMake Targets View" to see the available build targets.

Generating Ninja or Makefiles
-----------------------------

In your preferred terminal run:

.. code-block:: sh

  cmake <Repository Root> \
    -B <Path to Output> \
    -C <Repository Root>/cmake/caches/PredefinedParams.cmake \
    -DCMAKE_BUILD_TYPE=<Build Type> \
    -G Ninja

You may substitute ``Ninja`` for ``Unix Makefiles`` to generate a makefile
build. After generation completes you can run ``ninja`` or ``make`` as
appropriate.

Building and Running Tests
--------------------------

With the LIT-based testing solution, builds and tests are all run through the
generated build system. Regardless of which tool you use to build DXC you should
have the following targets available:

**llvm-test-depends** Builds all the binaries used by the tests.
**clang-test-depends** Builds all the binaries used by the clang tests.
**test-depends** Builds all the binaries used by all the tests.
**check-llvm** Runs the LLVM tests after rebuilding any required out-of-date targets.
**check-clang** Runs the Clang tests after rebuilding any required out-of-date targets.
**check-all** Runs all available tests after rebuilding any out-of-date targets.

Useful CMake Options
--------------------

By convention CMake options are all capital, underscore separated words, and the
first word signifies what the option applies to. In the DXC codebase there are
four commonly used option prefixes:

#. CMAKE - For options defined by CMake itself which apply across the entire
   configuration.
#. LLVM - For options defined by LLVM which DXC has inherited. These apply
   across the entire DXC codebase.
#. CLANG - For options defined in the clang sub-project which DXC has inherited.
   These options apply across just the tools/clang subdirectory.
#. DXC - For DXC-specific options, which may apply across the entire codebase.

**CMAKE_BUILD_TYPE**:STRING
  Sets the build type for single-configuration generators (i.e. Ninja and
  makefiles) Possible values are Release, Debug, RelWithDebInfo and MinSizeRel.
  On systems like Visual Studio or Xcode the user sets the build type with the
  IDE settings.

**LLVM_USE_LINKER**:STRING
  When building with Clang or GCC this option allows overriding the default
  linker used by setting the ``-fuse-ld=`` flag. This may be important for Linux
  users on systems where the system linker is ``ld.bfd`` as linking DXC with
  debug information can be very memory intensive.

**LLVM_PARALLEL_COMPILE_JOBS**:STRING
  When building with Ninja, this option can be used to limit the number of
  concurrent compilation jobs.

**LLVM_PARALLEL_LINK_JOBS**:STRING
  When building with Ninja, this option can be used to limit number of
  concurrent link jobs.

**DXC_COVERAGE**:BOOL
  This option must be passed before the ``-C`` flag to set the PredefinedParams
  cache script because it is handled by the cache script. This option enables
  building DXC with code coverage instrumentation and build targets to generate
  code coverage reports. With this setting enabled the
  ``generate-coverage-report`` target is added to the build which produces a
  static HTML page with code coverage analysis results.

Legacy Windows Build Tooling
----------------------------

After cloning the project, you can set up a build environment shortcut by
double-clicking the `utils\hct\hctshortcut.js` file. This will create a shortcut
on your desktop with a default configuration. If your system doesn't have the
requisite association for .js files, this may not work. If so, open a cmd window
and invoke: `wscript.exe utils\hct\hctshortcut.js`.

Tests are built using the TAEF framework which is included in the Windows Driver
Kit.

To build, run this command on the HLSL Console.

.. code-block:: sh

  hctbuild

You can also run tests with this command.

.. code-block:: sh

  hcttest

Some tests will run shaders and verify their behavior. These tests also involve
a driver that can execute these shaders. See the next section on how this should
be currently set up.

To see a list of additional commands available, run `hcthelp`
