============
HLSL Changes
============

Introduction
============

This document is meant to describe the changes that have been made to the LLVM
and Clang codebases to support HLSL, the High-Level Shading Language.  The
focus is on design changes and general approach rather than specific changes
made to support the language and runtime.

Driving Forces
==============

This section outlines the considerations that have prompted multiple changes.

* Provide a successor to the fxc.exe and d3dcompiler_47.dll components. This
  means providing a DLL that can be used in a variety of situations, as well
  as a command-line program to access its functionality.

* The requirement to act as a proper DLL means that it shouldn't rely on
  process-wide state, as this prevents callers from using it from more than
  one thread concurrently. This includes changing the current directory, using
  stdin/stdout/stderr, or deciding to terminate the process (except for fatal
  conditions).

* There are scenarios in which applications may choose to redistribute the
  built DLL, for example an IDE for writing shaders, tools to instrument and
  debug them, or a game that chooses to emit shaders at runtime. This means
  that compilation time and DLL/program size are important considerations.

* Because the new command-line program should be a replacement of specific
  components, it's desirable to keep the interface similar to the prior
  versions (API in case of the DLL, command-line format in the case of the
  program).

Forking LLVM and Clang
======================

This section describes why and how the HLSL on LLVM project has forked LLVM
and Clang.

LLVM and Clang provide an excellent starting point for building a compiler,
especially one that will exist in an ecosystem of organizations contributing
to the pipeline, whether it be in the form of tooling and abstractions by
middleware and tool authors, or backend compilers by hardware vendors.

The decision was made to fork LLVM and Clang rather than work directly and
upstream all changes for the following reasons:

* While HLSL started out as a C derivative, over time is has drifted away, not
  to the extent that C++ and Objective-C have but certainly a fair
  bit. Furthermore, some of the behavior was never compatible to begin with,
  and so there are significant differences, especially in the type system,
  that make upstreaming difficult.

* HLSL is expected to evolve over the next few years to shed some of the
  incompatibilities that exist for historical reasons (as opposed to those
  that provide actual developer benefit). It's entirely possible that a future
  version will be much more closely aligned to C/C++ semantics and could be
  more easily adapted. This version of the codebase isn't it.

* The changes to LLVM are meaningless without an execution model, which is
  currently being worked on. Changes to LLVM are more likely to get
  upstreamed, however we have chosen to not consider this until we have
  multiple implementations of DXIL backends.

We have already done a 3.4 to 3.7 upgrade in the sources, and it's entirely
possible that this will happen again. Therefore, all changes introduces are
done in entirely new files, or marked with an 'HLSL Change', or marked with a
pair of 'HLSL Change Starts' / 'HLSL Change Ends'. This makes integrations
easier (or rather less difficult).

Dependency Injection
====================

One of the goals of dxcompiler is to be a reusable component for a variety of
applications. While Clang and LLVM have support for being hosted as a
dynamically loaded library, there is still a number of assumptions made that
make usage problematic, specifically:

- Usage of process-wide handles (stdin/stdout/stderr).

- Reliance on file system access (clang has some level of virtual file system
  support in the later versions), including temporary files.

- Direct usage of memory allocation mechanisms.

- Reliance on other process-wide constructs like environment variables.

Redesigning a number of these mechanisms would require changes throughout the
codebase, and it's hard to whether any regressions are introduced when a large
number of changes are integrated.

The solution we have implemented relies on having a thread-local component
that can service I/O requests as well as other OS-implemented or process-wide
constructs. The library is then meant to be used through specific API points
that will set and tear down this component as appropriate. The MSFileSystem
class in include/llvm/Support/MSFileSystem.h provides the access point
(although the API should likely be renamed, as it's broader than a pur file
system abstraction, and 'MS' monikers are being changed to 'HLSL').

To guard against regressions, we can simply verify that no libraries include
APIs that are virtualized, such as calls to CreateFile. If a case is found, a
drop-in replacement function call can be made in its place.

Some of the tasks that are simplified include:

- Host in-process in an IDE to provide tooling services.

- Guarantee that no process execution takes place, or that the host
  environment influences the tools without an explicit usage.

- Provide virtualized I/O for all kinds of file access, including redirecting
  to in-memory buffers.

- Override memory allocation to constrain memory consumption.

At the moment, memory allocation is still not redirected, but I/O has been
cleaned up.


Error Handling
==============

Clang and LLVM already provide a number of error handling mechanisms:
returning a bool flag to indicate success or failure, returning a structured
object that flags the result along with other information, using STL system
errors for certain errors, or using errno for some C standard library calls.

There are two other kinds of error handling mechanisms introduced by HLSL
on LLVM.

* C++ Exceptions. The primary use case for these is to handle out-of-memory
  exceptions. A second case is to handle cases where LLVM and Clang today
  attempt to terminate the process even though they are in a recoverable state
  (for example, when a '--help' switch is found in command-line options).

* HRESULT. The APIs are designed to be familiar to Windows developers and to
  provide a simple transition for d3dcompiler_47.dll users. As such, error
  codes are typically returned as HRESULT values that can be tested with the
  SUCCEEDED/FAILED macros.

Some of the code that is written to interface with the rest of the system can
also make use of these error handling strategies. A number of macros to handle
them, or to convert one into the other, are available in
include/dxc/Support/Global.h.

Removing Unused Functionality
=============================

Removing unused functionality can help reduce the binary size, improve
performance, and speed up compilation time for the project. However, this has
to be traded off against changing the behavior of LLVM and Clang (and the
cost of understanding this change for developers who are familiar with those
projects), as well as the future maintenance for integrations.

The recommendations is to avoid removing small bits of functionality, and only
do so for significant subsystems that can be "sliced off" cleanly (for
example, the interpreter component or target support).

Component Design
================

The dxcompiler DLL is designed to export a number of components that can be
reused in different contexts. The API is exposed as a lightweight form of the
Microsoft Component Object Model (COM); a similar approach can be seen in the
design of the xmllite library.

The functionality of the library is encapsulated in discrete compmonents, each
of which is embodied in an object that implements one or more
interfaces. Interfaces are derived from IUnknown as in COM and are responsible
for interface discovery and lifetime management. Object construction is done
via the single exported API, DxcCreateObject, which acts much like
DllCreateObject would in a COM library.

Interfaces are mostly COM-compatible and have been designed to be easy to use
from other languages that can consume COM libraries, such as the .NET runtime
or C++ applications. Importantly, memory allocated in the library that should
be freed by the consumer is allocated using the COM allocation, through
CoTaskMemAlloc or the use of IMalloc via CoGetMalloc().

Note that this lightweight COM support implies that some features are missing:

- There is no support for marshalling across COM apartments.

- There is (at the moment at least) no management of library references based
  on outstanding objects (a typical bug that would arise from this would be,
  for example, unloading dxcompiler while outstanding objects exist, at which
  point even releasing them would lead to an access violation).

Text and Buffer Management
==========================

Tradionally, the D3D compilers have used an ID3DBlob interface to encapsulate
a buffer. The HLSL compiler avoids pulling in DirectX headers and defines an
IDxcBlob interface that has the same layout and interface identifier (IID).

Buffers are often used to hold text, for example shader sources or compilation
logs. IDxcBlobEncoding inherits from IDxcBlob and has functionality to declare
the encoding for the buffer.

The design principle for using a character pointer or an IDxcBlobEncoding is
as follows: for internal dxcompiler text, UTF-8 and char* are used; for API
parameters that are "short" such as file names or command line parameters,
UTF-16 and wchar_t* are used; for longer text such as source files or error
logs, IDxcBlobEncoding is used.

The DLL provides a "library" component that provides utility functions to
create and transform blobs and strings.

Specification Database
======================

In the utils\hct directory, an hctdb.py file can be found that initialized a
number of Python object instances describing different aspects of the DXIL
specification. These act as a queryable, programmable repository of
information which feed into other tasks, such as generating documentation,
generating code or performing compatibility checks across versions.

We require that the database be kept up-to-date for the concepts embedded
there to drive a number of code-generation tasks, which can be found in other
.py files in that same directory.

HLSL Modules
============

llvm::Module is the type that represents a shader program. It includes
metadata nodes to provide details around the ABI, flags, etc. However,
manipulation of all this information in terms of metadata is not very
efficient or convenient.

As part of the work with HLSL, we introduce two modules that are attached
in-memory to an llvm::Module: a high-level HLModule, and a low-level
DxilModule. The high-level module is used in the early passes to deal with
HLSL-as-a-language concepts, such as intrinsics, matrices and vectors; the
low-level module is used to deal with concepts as they exist in the DXIL
specification. Only one of these additional modules ever exists at one point;
the DxilGenerationPass that does the translation destroys the high-level
representation and creates a low-level one as part of its work.

To preserve many of the benefits of LLVM's modular pipeline, it is useful to
serialize and deserialize shaders at different stages of processing, and so
both HLModule and DxilModule provide support for these. The expectation for a
wholesale compilation from source, however, is that this information lives
only in memory until it's ready to be serialized out in final DIXL form. As
such, various passes along the way may need to do update to these modules to
maintain consistency (for example, if global DCE removes a variable, the
corresponding resource mapping that reflects the shader ABI should be cleaned
up as well).


