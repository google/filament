================================
Source Level Debugging with HLSL
================================

.. contents::
   :local:

Introduction
============

This document describes the specifics of source level debuging with HLSL. The
basic infrastructure is based on :doc:`Source Level Debugging with LLVM
<SourceLevelDebugging>`, so the focus here is on the specifics of DXIL
programs compiled from HLSL.

DXIL Debug Information Format
=============================

The debug information for an HLSL program in DXIL form is stored as an LLVM
module with debug information represented according to the :doc:`Source Level
Debugging with LLVM <SourceLevelDebugging>` document.

The :ref:`dxil_container_format` describes how a single data structure
holds both a DXIL program, debug information, and other optional parts.

There are three parts that are associated with debug information.

* DFCC_DXIL ('DXIL'). A valid DXIL program has no debug information. This is
  the program described by debug information.

* DFCC_ShaderDebugInfoDXIL ('ILDB'). This is an LLVM module with debug
  information. It's an augmented version of the original DXIL module. For
  historical reasons, this is sometimes referred to as 'the PDB of the
  program'.

* DFCC_ShaderDebugName ('ILDN'). This is a name for an external entity holding
  the debug information.

Using Debug Information
=======================

The debug information can be used directly by looking up the debug information
part and loading into an LLVM module. There is full fidelity with debug
information via this mechanism, although it requires linking in the LLVM
supporting libraries.

For compatibility, the dxcompiler.dll binary also exposes a limited
implementation of the DIA APIs. To do this, a CLSID_DxcDiaDataSource class
should be created via a call to DxcCreateInstance, and a loadDataFromIStream
call with the debug part will initialize it.

The DxcContext::Recompile implementation provides an example of how to
initialize the diagnostic objects from debug information, extract high-level
information and recreate the compilation options and inputs.

Using Debug Names
=================

The only current use case for the debug name is as a relative path to a file
that provides shader debug information. A debugging tool would typically have
a list of paths to act as search roots.

Command-Line Options
====================

The following command-line options are used with the DirectX Shader Compiler
tools to work with debug information.

* /Zi. Enables debug information during compilation.

* /Zss. Builds debug names that consider source information.

* /Zsb. Builds debug names that consider only the output binary.

* /Fd. Extracts debug information to a different file.

* /Qstrip_debug. Removes debug information from a container.

The most common use cases are as follows.

* Build debug information and leave it in the container. In this case, simply
  compiling with /Zi will do the trick.

* Build debug information and extract it to an auto-generated external
  file. In this case, /Zi and /Fd should both be used, and the /Fd value
  should end in a trailing backslash when using dxc, naming the target
  directory in which to place the file. /Zss is the default, but /Zsb can be
  used to deduplicate files. When using /Fd with a directory name,
  /Qstrip_debug is implied.

A less common use case is to specify an explicit name for the external
file. In this case, the command-line should include /Zi, /Fd with a specific
name, and /Qstrip_debug.

Implementation Notes
====================

The current implementation provides a few interesting behaviors worth noting.

* The shader debug name is derived from either the DXIL or the ILDB parts by
  hashing the byte contents, but it can be replaced programmatically.

* Source content is included in the debug information blob by default. This
  helps with scenarios where the code never exists on-disk, but is instead
  generated on-the-fly.
  
* Typically the derivation is done from the ILDB part, which includes
  source-specific information, and so two shaders with different sources will
  have different debug information. However the option is provided via (-Zsb)
  to include debug information that only takes into consideration the DXIL
  binary. In this case, two shaders that compile to the same binary will have
  the same debug information, which can be used to deduplicate content when
  any equivalent source program is acceptable for debugging.

Future Directions
=================

This section is purely speculative, but captures some of the thoughts about
future debugging capabilities.

* If driver-level constructs should be debugged, they need to be mapped to
  DXIL first, and from there on to HLSL.

* Including content in debug is convenient, especially when sources are
  transient, but they are inefficient (again, especially for a large number of
  transient sources). Deduplicating sources would be beneficial.

* Integration with symbol servers and source servers can simplify some of the
  developer workflows.

