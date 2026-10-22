============================
Using DirectX Compiler Tools
============================

.. contents::
   :local:

Introduction
============

The HLSL on LLVM project provides a number of tools to create and manipulate
DirectX shader programs.

After building the project, the tools described are available under the
Debug\bin or Release\bin subdirectories under the build target directory.


Tools
=====

dxc.exe
    This command-line tool is a replacement for fxc, and accepts the same
    command-line switches.

dndxc.exe
    This tool provides a GUI to compile HLSL programs and examine details of
    compilation, including the output assembly, the container structure, and
    the optimizer stages run.

dxexp.exe
    This command-line tool checks whether the current setup is able to run
    experimental shaders, that is, shaders that use a driver's experimental
    shader support or that are not properly validated.

dxa.exe
    This command-line tool provides a number of options to
    assemble/disassemble a shader.

dxr.exe
    This command-line tool allows a shader file to be rewritten in a
    consistent style, and optionally trim unused declarations.

Running Experimental Shaders
============================

To run experimental shaders in a process, the following conditions must be
met:

- A recent flight of Windows must be used.

- The 'Use developer features' must be set to 'Developer mode' in the 'For
  developers' page of Settings.

- For a 64-bit OS, the process should be a 64-bit process. This will be fixed
  to support 32-bit processes on a 64-bit OS.

- The process must call D3D12EnableExperimentalFeatures to enable the
  D3D12ExperimentalShaderModelsID setting. The ExecutionTest.cpp file under
  tools\clang\unittests\HLSL\ExecutionTest.cpp has an example of how this can
  be done.

- The API used must be D3D12.

