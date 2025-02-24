Design of the Subzero fast code generator
=========================================

Introduction
------------

The `Portable Native Client (PNaCl) <http://gonacl.com>`_ project includes
compiler technology based on `LLVM <http://llvm.org/>`_.  The developer uses the
PNaCl toolchain to compile their application to architecture-neutral PNaCl
bitcode (a ``.pexe`` file), using as much architecture-neutral optimization as
possible.  The ``.pexe`` file is downloaded to the user's browser where the
PNaCl translator (a component of Chrome) compiles the ``.pexe`` file to
`sandboxed
<https://developer.chrome.com/native-client/reference/sandbox_internals/index>`_
native code.  The translator uses architecture-specific optimizations as much as
practical to generate good native code.

The native code can be cached by the browser to avoid repeating translation on
future page loads.  However, first-time user experience is hampered by long
translation times.  The LLVM-based PNaCl translator is pretty slow, even when
using ``-O0`` to minimize optimizations, so delays are especially noticeable on
slow browser platforms such as ARM-based Chromebooks.

Translator slowness can be mitigated or hidden in a number of ways.

- Parallel translation.  However, slow machines where this matters most, e.g.
  ARM-based Chromebooks, are likely to have fewer cores to parallelize across,
  and are likely to less memory available for multiple translation threads to
  use.

- Streaming translation, i.e. start translating as soon as the download starts.
  This doesn't help much when translation speed is 10× slower than download
  speed, or the ``.pexe`` file is already cached while the translated binary was
  flushed from the cache.

- Arrange the web page such that translation is done in parallel with
  downloading large assets.

- Arrange the web page to distract the user with `cat videos
  <https://www.youtube.com/watch?v=tLt5rBfNucc>`_ while translation is in
  progress.

Or, improve translator performance to something more reasonable.

This document describes Subzero's attempt to improve translation speed by an
order of magnitude while rivaling LLVM's code quality.  Subzero does this
through minimal IR layering, lean data structures and passes, and a careful
selection of fast optimization passes.  It has two optimization recipes: full
optimizations (``O2``) and minimal optimizations (``Om1``).  The recipes are the
following (described in more detail below):

+----------------------------------------+-----------------------------+
| O2 recipe                              | Om1 recipe                  |
+========================================+=============================+
| Parse .pexe file                       | Parse .pexe file            |
+----------------------------------------+-----------------------------+
| Loop nest analysis                     |                             |
+----------------------------------------+-----------------------------+
| Local common subexpression elimination |                             |
+----------------------------------------+-----------------------------+
| Address mode inference                 |                             |
+----------------------------------------+-----------------------------+
| Read-modify-write (RMW) transform      |                             |
+----------------------------------------+-----------------------------+
| Basic liveness analysis                |                             |
+----------------------------------------+-----------------------------+
| Load optimization                      |                             |
+----------------------------------------+-----------------------------+
|                                        | Phi lowering (simple)       |
+----------------------------------------+-----------------------------+
| Target lowering                        | Target lowering             |
+----------------------------------------+-----------------------------+
| Full liveness analysis                 |                             |
+----------------------------------------+-----------------------------+
| Register allocation                    | Minimal register allocation |
+----------------------------------------+-----------------------------+
| Phi lowering (advanced)                |                             |
+----------------------------------------+-----------------------------+
| Post-phi register allocation           |                             |
+----------------------------------------+-----------------------------+
| Branch optimization                    |                             |
+----------------------------------------+-----------------------------+
| Code emission                          | Code emission               |
+----------------------------------------+-----------------------------+

Goals
=====

Translation speed
-----------------

We'd like to be able to translate a ``.pexe`` file as fast as download speed.
Any faster is in a sense wasted effort.  Download speed varies greatly, but
we'll arbitrarily say 1 MB/sec.  We'll pick the ARM A15 CPU as the example of a
slow machine.  We observe a 3× single-thread performance difference between A15
and a high-end x86 Xeon E5-2690 based workstation, and aggressively assume a
``.pexe`` file could be compressed to 50% on the web server using gzip transport
compression, so we set the translation speed goal to 6 MB/sec on the high-end
Xeon workstation.

Currently, at the ``-O0`` level, the LLVM-based PNaCl translation translates at
⅒ the target rate.  The ``-O2`` mode takes 3× as long as the ``-O0`` mode.

In other words, Subzero's goal is to improve over LLVM's translation speed by
10×.

Code quality
------------

Subzero's initial goal is to produce code that meets or exceeds LLVM's ``-O0``
code quality.  The stretch goal is to approach LLVM ``-O2`` code quality.  On
average, LLVM ``-O2`` performs twice as well as LLVM ``-O0``.

It's important to note that the quality of Subzero-generated code depends on
target-neutral optimizations and simplifications being run beforehand in the
developer environment.  The ``.pexe`` file reflects these optimizations.  For
example, Subzero assumes that the basic blocks are ordered topologically where
possible (which makes liveness analysis converge fastest), and Subzero does not
do any function inlining because it should already have been done.

Translator size
---------------

The current LLVM-based translator binary (``pnacl-llc``) is about 10 MB in size.
We think 1 MB is a more reasonable size -- especially for such a component that
is distributed to a billion Chrome users.  Thus we target a 10× reduction in
binary size.

For development, Subzero can be built for all target architectures, and all
debugging and diagnostic options enabled.  For a smaller translator, we restrict
to a single target architecture, and define a ``MINIMAL`` build where
unnecessary features are compiled out.

Subzero leverages some data structures from LLVM's ``ADT`` and ``Support``
include directories, which have little impact on translator size.  It also uses
some of LLVM's bitcode decoding code (for binary-format ``.pexe`` files), again
with little size impact.  In non-``MINIMAL`` builds, the translator size is much
larger due to including code for parsing text-format bitcode files and forming
LLVM IR.

Memory footprint
----------------

The current LLVM-based translator suffers from an issue in which some
function-specific data has to be retained in memory until all translation
completes, and therefore the memory footprint grows without bound.  Large
``.pexe`` files can lead to the translator process holding hundreds of MB of
memory by the end.  The translator runs in a separate process, so this memory
growth doesn't *directly* affect other processes, but it does dirty the physical
memory and contributes to a perception of bloat and sometimes a reality of
out-of-memory tab killing, especially noticeable on weaker systems.

Subzero should maintain a stable memory footprint throughout translation.  It's
not really practical to set a specific limit, because there is not really a
practical limit on a single function's size, but the footprint should be
"reasonable" and be proportional to the largest input function size, not the
total ``.pexe`` file size.  Simply put, Subzero should not have memory leaks or
inexorable memory growth.  (We use ASAN builds to test for leaks.)

Multithreaded translation
-------------------------

It should be practical to translate different functions concurrently and see
good scalability.  Some locking may be needed, such as accessing output buffers
or constant pools, but that should be fairly minimal.  In contrast, LLVM was
only designed for module-level parallelism, and as such, the PNaCl translator
internally splits a ``.pexe`` file into several modules for concurrent
translation.  All output needs to be deterministic regardless of the level of
multithreading, i.e. functions and data should always be output in the same
order.

Target architectures
--------------------

Initial target architectures are x86-32, x86-64, ARM32, and MIPS32.  Future
targets include ARM64 and MIPS64, though these targets lack NaCl support
including a sandbox model or a validator.

The first implementation is for x86-32, because it was expected to be
particularly challenging, and thus more likely to draw out any design problems
early:

- There are a number of special cases, asymmetries, and warts in the x86
  instruction set.

- Complex addressing modes may be leveraged for better code quality.

- 64-bit integer operations have to be lowered into longer sequences of 32-bit
  operations.

- Paucity of physical registers may reveal code quality issues early in the
  design.

Detailed design
===============

Intermediate representation - ICE
---------------------------------

Subzero's IR is called ICE.  It is designed to be reasonably similar to LLVM's
IR, which is reflected in the ``.pexe`` file's bitcode structure.  It has a
representation of global variables and initializers, and a set of functions.
Each function contains a list of basic blocks, and each basic block constains a
list of instructions.  Instructions that operate on stack and register variables
do so using static single assignment (SSA) form.

The ``.pexe`` file is translated one function at a time (or in parallel by
multiple translation threads).  The recipe for optimization passes depends on
the specific target and optimization level, and is described in detail below.
Global variables (types and initializers) are simply and directly translated to
object code, without any meaningful attempts at optimization.

A function's control flow graph (CFG) is represented by the ``Ice::Cfg`` class.
Its key contents include:

- A list of ``CfgNode`` pointers, generally held in topological order.

- A list of ``Variable`` pointers corresponding to local variables used in the
  function plus compiler-generated temporaries.

A basic block is represented by the ``Ice::CfgNode`` class.  Its key contents
include:

- A linear list of instructions, in the same style as LLVM.  The last
  instruction of the list is always a terminator instruction: branch, switch,
  return, unreachable.

- A list of Phi instructions, also in the same style as LLVM.  They are held as
  a linear list for convenience, though per Phi semantics, they are executed "in
  parallel" without dependencies on each other.

- An unordered list of ``CfgNode`` pointers corresponding to incoming edges, and
  another list for outgoing edges.

- The node's unique, 0-based index into the CFG's node list.

An instruction is represented by the ``Ice::Inst`` class.  Its key contents
include:

- A list of source operands.

- Its destination variable, if the instruction produces a result in an
  ``Ice::Variable``.

- A bitvector indicating which variables' live ranges this instruction ends.
  This is computed during liveness analysis.

Instructions kinds are divided into high-level ICE instructions and low-level
ICE instructions.  High-level instructions consist of the PNaCl/LLVM bitcode
instruction kinds.  Each target architecture implementation extends the
instruction space with its own set of low-level instructions.  Generally,
low-level instructions correspond to individual machine instructions.  The
high-level ICE instruction space includes a few additional instruction kinds
that are not part of LLVM but are generally useful (e.g., an Assignment
instruction), or are useful across targets.

Specifically, high-level ICE instructions that derive from LLVM (but with PNaCl
ABI restrictions as documented in the `PNaCl Bitcode Reference Manual
<https://developer.chrome.com/native-client/reference/pnacl-bitcode-abi>`_) are
the following:

- Alloca: allocate data on the stack

- Arithmetic: binary operations of the form ``A = B op C``

- Br: conditional or unconditional branch

- Call: function call

- Cast: unary type-conversion operations

- ExtractElement: extract a scalar element from a vector-type value

- Fcmp: floating-point comparison

- Icmp: integer comparison

- Intrinsic: invoke a known intrinsic

- InsertElement: insert a scalar element into a vector-type value

- Load: load a value from memory

- Phi: implement the SSA phi node

- Ret: return from the function

- Select: essentially the C language operation of the form ``X = C ? Y : Z``

- Store: store a value into memory

- Switch: generalized branch to multiple possible locations

- Unreachable: indicate that this portion of the code is unreachable

The additional high-level ICE instructions are the following:

- Assign: a simple ``A=B`` assignment.  This is useful for e.g. lowering Phi
  instructions to non-SSA assignments, before lowering to machine code.

- FakeDef, FakeUse, FakeKill.  These are tools used to preserve consistency in
  liveness analysis, elevated to the high-level because they are used by all
  targets.  They are described in more detail at the end of this section.

- JumpTable: this represents the result of switch optimization analysis, where
  some switch instructions may use jump tables instead of cascading
  compare/branches.

An operand is represented by the ``Ice::Operand`` class.  In high-level ICE, an
operand is either an ``Ice::Constant`` or an ``Ice::Variable``.  Constants
include scalar integer constants, scalar floating point constants, Undef (an
unspecified constant of a particular scalar or vector type), and symbol
constants (essentially addresses of globals).  Note that the PNaCl ABI does not
include vector-type constants besides Undef, and as such, Subzero (so far) has
no reason to represent vector-type constants internally.  A variable represents
a value allocated on the stack (though not including alloca-derived storage).
Among other things, a variable holds its unique, 0-based index into the CFG's
variable list.

Each target can extend the ``Constant`` and ``Variable`` classes for its own
needs.  In addition, the ``Operand`` class may be extended, e.g. to define an
x86 ``MemOperand`` that encodes a base register, an index register, an index
register shift amount, and a constant offset.

Register allocation and liveness analysis are restricted to Variable operands.
Because of the importance of register allocation to code quality, and the
translation-time cost of liveness analysis, Variable operands get some special
treatment in ICE.  Most notably, a frequent pattern in Subzero is to iterate
across all the Variables of an instruction.  An instruction holds a list of
operands, but an operand may contain 0, 1, or more Variables.  As such, the
``Operand`` class specially holds a list of Variables contained within, for
quick access.

A Subzero transformation pass may work by deleting an existing instruction and
replacing it with zero or more new instructions.  Instead of actually deleting
the existing instruction, we generally mark it as deleted and insert the new
instructions right after the deleted instruction.  When printing the IR for
debugging, this is a big help because it makes it much more clear how the
non-deleted instructions came about.

Subzero has a few special instructions to help with liveness analysis
consistency.

- The FakeDef instruction gives a fake definition of some variable.  For
  example, on x86-32, a divide instruction defines both ``%eax`` and ``%edx``
  but an ICE instruction can represent only one destination variable.  This is
  similar for multiply instructions, and for function calls that return a 64-bit
  integer result in the ``%edx:%eax`` pair.  Also, using the ``xor %eax, %eax``
  trick to set ``%eax`` to 0 requires an initial FakeDef of ``%eax``.

- The FakeUse instruction registers a use of a variable, typically to prevent an
  earlier assignment to that variable from being dead-code eliminated.  For
  example, lowering an operation like ``x=cc?y:z`` may be done using x86's
  conditional move (cmov) instruction: ``mov z, x; cmov_cc y, x``.  Without a
  FakeUse of ``x`` between the two instructions, the liveness analysis pass may
  dead-code eliminate the first instruction.

- The FakeKill instruction is added after a call instruction, and is a quick way
  of indicating that caller-save registers are invalidated.

Pexe parsing
------------

Subzero includes an integrated PNaCl bitcode parser for ``.pexe`` files.  It
parses the ``.pexe`` file function by function, ultimately constructing an ICE
CFG for each function.  After a function is parsed, its CFG is handed off to the
translation phase.  The bitcode parser also parses global initializer data and
hands it off to be translated to data sections in the object file.

Subzero has another parsing strategy for testing/debugging.  LLVM libraries can
be used to parse a module into LLVM IR (though very slowly relative to Subzero
native parsing).  Then we iterate across the LLVM IR and construct high-level
ICE, handing off each CFG to the translation phase.

Overview of lowering
--------------------

In general, translation goes like this:

- Parse the next function from the ``.pexe`` file and construct a CFG consisting
  of high-level ICE.

- Do analysis passes and transformation passes on the high-level ICE, as
  desired.

- Lower each high-level ICE instruction into a sequence of zero or more
  low-level ICE instructions.  Each high-level instruction is generally lowered
  independently, though the target lowering is allowed to look ahead in the
  CfgNode's instruction list if desired.

- Do more analysis and transformation passes on the low-level ICE, as desired.

- Assemble the low-level CFG into an ELF object file (alternatively, a textual
  assembly file that is later assembled by some external tool).

- Repeat for all functions, and also produce object code for data such as global
  initializers and internal constant pools.

Currently there are two optimization levels: ``O2`` and ``Om1``.  For ``O2``,
the intention is to apply all available optimizations to get the best code
quality (though the initial code quality goal is measured against LLVM's ``O0``
code quality).  For ``Om1``, the intention is to apply as few optimizations as
possible and produce code as quickly as possible, accepting poor code quality.
``Om1`` is short for "O-minus-one", i.e. "worse than O0", or in other words,
"sub-zero".

High-level debuggability of generated code is so far not a design requirement.
Subzero doesn't really do transformations that would obfuscate debugging; the
main thing might be that register allocation (including stack slot coalescing
for stack-allocated variables whose live ranges don't overlap) may render a
variable's value unobtainable after its live range ends.  This would not be an
issue for ``Om1`` since it doesn't register-allocate program-level variables,
nor does it coalesce stack slots.  That said, fully supporting debuggability
would require a few additions:

- DWARF support would need to be added to Subzero's ELF file emitter.  Subzero
  propagates global symbol names, local variable names, and function-internal
  label names that are present in the ``.pexe`` file.  This would allow a
  debugger to map addresses back to symbols in the ``.pexe`` file.

- To map ``.pexe`` file symbols back to meaningful source-level symbol names,
  file names, line numbers, etc., Subzero would need to handle `LLVM bitcode
  metadata <http://llvm.org/docs/LangRef.html#metadata>`_ and ``llvm.dbg``
  `instrinsics<http://llvm.org/docs/LangRef.html#dbg-intrinsics>`_.

- The PNaCl toolchain explicitly strips all this from the ``.pexe`` file, and so
  the toolchain would need to be modified to preserve it.

Our experience so far is that ``Om1`` translates twice as fast as ``O2``, but
produces code with one third the code quality.  ``Om1`` is good for testing and
debugging -- during translation, it tends to expose errors in the basic lowering
that might otherwise have been hidden by the register allocator or other
optimization passes.  It also helps determine whether a code correctness problem
is a fundamental problem in the basic lowering, or an error in another
optimization pass.

The implementation of target lowering also controls the recipe of passes used
for ``Om1`` and ``O2`` translation.  For example, address mode inference may
only be relevant for x86.

Lowering strategy
-----------------

The core of Subzero's lowering from high-level ICE to low-level ICE is to lower
each high-level instruction down to a sequence of low-level target-specific
instructions, in a largely context-free setting.  That is, each high-level
instruction conceptually has a simple template expansion into low-level
instructions, and lowering can in theory be done in any order.  This may sound
like a small effort, but quite a large number of templates may be needed because
of the number of PNaCl types and instruction variants.  Furthermore, there may
be optimized templates, e.g. to take advantage of operator commutativity (for
example, ``x=x+1`` might allow a bettern lowering than ``x=1+x``).  This is
similar to other template-based approaches in fast code generation or
interpretation, though some decisions are deferred until after some global
analysis passes, mostly related to register allocation, stack slot assignment,
and specific choice of instruction variant and addressing mode.

The key idea for a lowering template is to produce valid low-level instructions
that are guaranteed to meet address mode and other structural requirements of
the instruction set.  For example, on x86, the source operand of an integer
store instruction must be an immediate or a physical register; a shift
instruction's shift amount must be an immediate or in register ``%cl``; a
function's integer return value is in ``%eax``; most x86 instructions are
two-operand, in contrast to corresponding three-operand high-level instructions;
etc.

Because target lowering runs before register allocation, there is no way to know
whether a given ``Ice::Variable`` operand lives on the stack or in a physical
register.  When the low-level instruction calls for a physical register operand,
the target lowering can create an infinite-weight Variable.  This tells the
register allocator to assign infinite weight when making decisions, effectively
guaranteeing some physical register.  Variables can also be pre-colored to a
specific physical register (``cl`` in the shift example above), which also gives
infinite weight.

To illustrate, consider a high-level arithmetic instruction on 32-bit integer
operands::

    A = B + C

X86 target lowering might produce the following::

    T.inf = B  // mov instruction
    T.inf += C // add instruction
    A = T.inf  // mov instruction

Here, ``T.inf`` is an infinite-weight temporary.  As long as ``T.inf`` has a
physical register, the three lowered instructions are all encodable regardless
of whether ``B`` and ``C`` are physical registers, memory, or immediates, and
whether ``A`` is a physical register or in memory.

In this example, ``A`` must be a Variable and one may be tempted to simplify the
lowering sequence by setting ``A`` as infinite-weight and using::

        A = B  // mov instruction
        A += C // add instruction

This has two problems.  First, if the original instruction was actually ``A =
B + A``, the result would be incorrect.  Second, assigning ``A`` a physical
register applies throughout ``A``'s entire live range.  This is probably not
what is intended, and may ultimately lead to a failure to allocate a register
for an infinite-weight variable.

This style of lowering leads to many temporaries being generated, so in ``O2``
mode, we rely on the register allocator to clean things up.  For example, in the
example above, if ``B`` ends up getting a physical register and its live range
ends at this instruction, the register allocator is likely to reuse that
register for ``T.inf``.  This leads to ``T.inf=B`` being a redundant register
copy, which is removed as an emission-time peephole optimization.

O2 lowering
-----------

Currently, the ``O2`` lowering recipe is the following:

- Loop nest analysis

- Local common subexpression elimination

- Address mode inference

- Read-modify-write (RMW) transformation

- Basic liveness analysis

- Load optimization

- Target lowering

- Full liveness analysis

- Register allocation

- Phi instruction lowering (advanced)

- Post-phi lowering register allocation

- Branch optimization

These passes are described in more detail below.

Om1 lowering
------------

Currently, the ``Om1`` lowering recipe is the following:

- Phi instruction lowering (simple)

- Target lowering

- Register allocation (infinite-weight and pre-colored only)

Optimization passes
-------------------

Liveness analysis
^^^^^^^^^^^^^^^^^

Liveness analysis is a standard dataflow optimization, implemented as follows.
For each node (basic block), its live-out set is computed as the union of the
live-in sets of its successor nodes.  Then the node's instructions are processed
in reverse order, updating the live set, until the beginning of the node is
reached, and the node's live-in set is recorded.  If this iteration has changed
the node's live-in set, the node's predecessors are marked for reprocessing.
This continues until no more nodes need reprocessing.  If nodes are processed in
reverse topological order, the number of iterations over the CFG is generally
equal to the maximum loop nest depth.

To implement this, each node records its live-in and live-out sets, initialized
to the empty set.  Each instruction records which of its Variables' live ranges
end in that instruction, initialized to the empty set.  A side effect of
liveness analysis is dead instruction elimination.  Each instruction can be
marked as tentatively dead, and after the algorithm converges, the tentatively
dead instructions are permanently deleted.

Optionally, after this liveness analysis completes, we can do live range
construction, in which we calculate the live range of each variable in terms of
instruction numbers.  A live range is represented as a union of segments, where
the segment endpoints are instruction numbers.  Instruction numbers are required
to be unique across the CFG, and monotonically increasing within a basic block.
As a union of segments, live ranges can contain "gaps" and are therefore
precise.  Because of SSA properties, a variable's live range can start at most
once in a basic block, and can end at most once in a basic block.  Liveness
analysis keeps track of which variable/instruction tuples begin live ranges and
end live ranges, and combined with live-in and live-out sets, we can efficiently
build up live ranges of all variables across all basic blocks.

A lot of care is taken to try to make liveness analysis fast and efficient.
Because of the lowering strategy, the number of variables is generally
proportional to the number of instructions, leading to an O(N^2) complexity
algorithm if implemented naively.  To improve things based on sparsity, we note
that most variables are "local" and referenced in at most one basic block (in
contrast to the "global" variables with multi-block usage), and therefore cannot
be live across basic blocks.  Therefore, the live-in and live-out sets,
typically represented as bit vectors, can be limited to the set of global
variables, and the intra-block liveness bit vector can be compacted to hold the
global variables plus the local variables for that block.

Register allocation
^^^^^^^^^^^^^^^^^^^

Subzero implements a simple linear-scan register allocator, based on the
allocator described by Hanspeter Mössenböck and Michael Pfeiffer in `Linear Scan
Register Allocation in the Context of SSA Form and Register Constraints
<ftp://ftp.ssw.uni-linz.ac.at/pub/Papers/Moe02.PDF>`_.  This allocator has
several nice features:

- Live ranges are represented as unions of segments, as described above, rather
  than a single start/end tuple.

- It allows pre-coloring of variables with specific physical registers.

- It applies equally well to pre-lowered Phi instructions.

The paper suggests an approach of aggressively coalescing variables across Phi
instructions (i.e., trying to force Phi source and destination variables to have
the same register assignment), but we reject that in favor of the more natural
preference mechanism described below.

We enhance the algorithm in the paper with the capability of automatic inference
of register preference, and with the capability of allowing overlapping live
ranges to safely share the same register in certain circumstances.  If we are
considering register allocation for variable ``A``, and ``A`` has a single
defining instruction ``A=B+C``, then the preferred register for ``A``, if
available, would be the register assigned to ``B`` or ``C``, if any, provided
that ``B`` or ``C``'s live range does not overlap ``A``'s live range.  In this
way we infer a good register preference for ``A``.

We allow overlapping live ranges to get the same register in certain cases.
Suppose a high-level instruction like::

    A = unary_op(B)

has been target-lowered like::

    T.inf = B
    A = unary_op(T.inf)

Further, assume that ``B``'s live range continues beyond this instruction
sequence, and that ``B`` has already been assigned some register.  Normally, we
might want to infer ``B``'s register as a good candidate for ``T.inf``, but it
turns out that ``T.inf`` and ``B``'s live ranges overlap, requiring them to have
different registers.  But ``T.inf`` is just a read-only copy of ``B`` that is
guaranteed to be in a register, so in theory these overlapping live ranges could
safely have the same register.  Our implementation allows this overlap as long
as ``T.inf`` is never modified within ``B``'s live range, and ``B`` is never
modified within ``T.inf``'s live range.

Subzero's register allocator can be run in 3 configurations.

- Normal mode.  All Variables are considered for register allocation.  It
  requires full liveness analysis and live range construction as a prerequisite.
  This is used by ``O2`` lowering.

- Minimal mode.  Only infinite-weight or pre-colored Variables are considered.
  All other Variables are stack-allocated.  It does not require liveness
  analysis; instead, it quickly scans the instructions and records first
  definitions and last uses of all relevant Variables, using that to construct a
  single-segment live range.  Although this includes most of the Variables, the
  live ranges are mostly simple, short, and rarely overlapping, which the
  register allocator handles efficiently.  This is used by ``Om1`` lowering.

- Post-phi lowering mode.  Advanced phi lowering is done after normal-mode
  register allocation, and may result in new infinite-weight Variables that need
  registers.  One would like to just run something like minimal mode to assign
  registers to the new Variables while respecting existing register allocation
  decisions.  However, it sometimes happens that there are no free registers.
  In this case, some register needs to be forcibly spilled to the stack and
  temporarily reassigned to the new Variable, and reloaded at the end of the new
  Variable's live range.  The register must be one that has no explicit
  references during the Variable's live range.  Since Subzero currently doesn't
  track def/use chains (though it does record the CfgNode where a Variable is
  defined), we just do a brute-force search across the CfgNode's instruction
  list for the instruction numbers of interest.  This situation happens very
  rarely, so there's little point for now in improving its performance.

The basic linear-scan algorithm may, as it proceeds, rescind an early register
allocation decision, leaving that Variable to be stack-allocated.  Some of these
times, it turns out that the Variable could have been given a different register
without conflict, but by this time it's too late.  The literature recognizes
this situation and describes "second-chance bin-packing", which Subzero can do.
We can rerun the register allocator in a mode that respects existing register
allocation decisions, and sometimes it finds new non-conflicting opportunities.
In fact, we can repeatedly run the register allocator until convergence.
Unfortunately, in the current implementation, these subsequent register
allocation passes end up being extremely expensive.  This is because of the
treatment of the "unhandled pre-colored" Variable set, which is normally very
small but ends up being quite large on subsequent passes.  Its performance can
probably be made acceptable with a better choice of data structures, but for now
this second-chance mechanism is disabled.

Future work is to implement LLVM's `Greedy
<http://blog.llvm.org/2011/09/greedy-register-allocation-in-llvm-30.html>`_
register allocator as a replacement for the basic linear-scan algorithm, given
LLVM's experience with its improvement in code quality.  (The blog post claims
that the Greedy allocator also improved maintainability because a lot of hacks
could be removed, but Subzero is probably not yet to that level of hacks, and is
less likely to see that particular benefit.)

Local common subexpression elimination
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Local CSE implementation goes through each instruction and records a portion
of each ``Seen`` instruction in a hashset-like container.  That portion consists
of the entire instruction except for any dest variable. That means ``A = X + Y``
and ``B = X + Y`` will be considered to be 'equal' for this purpose. This allows
us to detect common subexpressions.

Whenever a repetition is detected, the redundant variables are stored in a
container mapping the replacee to the replacement. In the case above, it would
be ``MAP[B] = A`` assuming ``B = X + Y`` comes after ``A = X + Y``.

At any point if a variable that has an entry in the replacement table is
encountered, it is replaced with the variable it is mapped to. This ensures that
the redundant variables will not have any uses in the basic block, allowing
dead code elimination to clean up the redundant instruction.

With SSA, the information stored is never invalidated. However, non-SSA input is
supported with the ``-lcse=no-ssa`` option. This has to maintain some extra
dependency information to ensure proper invalidation on variable assignment.
This is not rigorously tested because this pass is run at an early stage where
it is safe to assume variables have a single definition. This is not enabled by
default because it bumps the compile time overhead from 2% to 6%.

Loop-invariant code motion
^^^^^^^^^^^^^^^^^^^^^^^^^^

This pass utilizes the loop analysis information to hoist invariant instructions
to loop pre-headers. A loop must have a single entry node (header) and that node
must have a single external predecessor for this optimization to work, as it is
currently implemented.

The pass works by iterating over all instructions in the loop until the set of
invariant instructions converges. In each iteration, a non-invariant instruction
involving only constants or a variable known to be invariant is added to the
result set. The destination variable of that instruction is added to the set of
variables known to be invariant (which is initialized with the constant args).

Improving the loop-analysis infrastructure is likely to have significant impact
on this optimization. Inserting an extra node to act as the pre-header when the
header has multiple incoming edges from outside could also be a good idea.
Expanding the initial invariant variable set to contain all variables that do
not have definitions inside the loop does not seem to improve anything.

Short circuit evaluation
^^^^^^^^^^^^^^^^^^^^^^^^

Short circuit evaluation splits nodes and introduces early jumps when the result
of a logical operation can be determined early and there are no observable side
effects of skipping the rest of the computation. The instructions considered
backwards from the end of the basic blocks. When a definition of a variable
involved in a conditional jump is found, an extra jump can be inserted in that
location, moving the rest of the instructions in the node to a newly inserted
node. Consider this example::

  __N :
    a = <something>
    Instruction 1 without side effect
    ... b = <something> ...
    Instruction N without side effect
    t1 = or a b
    br t1 __X __Y

is transformed to::

  __N :
    a = <something>
    br a __X __N_ext

  __N_ext :
    Instruction 1 without side effect
    ... b = <something> ...
    Instruction N without side effect
    br b __X __Y

The logic for AND is analogous, the only difference is that the early jump is
facilitated by a ``false`` value instead of ``true``.

Global Variable Splitting
^^^^^^^^^^^^^^^^^^^^^^^^^

Global variable splitting (``-split-global-vars``) is run after register
allocation. It works on the variables that did not manage to get registers (but
are allowed to) and decomposes their live ranges into the individual segments
(which span a single node at most). New variables are created (but not yet used)
with these smaller live ranges and the register allocator is run again. This is
not inefficient as the old variables that already had registers are now
considered pre-colored.

The new variables that get registers replace their parent variables for their
portion of its (parent's) live range. A copy from the old variable to the new
is introduced before the first use and the reverse after the last def in the
live range.

Basic phi lowering
^^^^^^^^^^^^^^^^^^

The simplest phi lowering strategy works as follows (this is how LLVM ``-O0``
implements it).  Consider this example::

    L1:
      ...
      br L3
    L2:
      ...
      br L3
    L3:
      A = phi [B, L1], [C, L2]
      X = phi [Y, L1], [Z, L2]

For each destination of a phi instruction, we can create a temporary and insert
the temporary's assignment at the end of the predecessor block::

    L1:
      ...
      A' = B
      X' = Y
      br L3
    L2:
      ...
      A' = C
      X' = Z
      br L3
    L2:
      A = A'
      X = X'

This transformation is very simple and reliable.  It can be done before target
lowering and register allocation, and it easily avoids the classic lost-copy and
related problems.  ``Om1`` lowering uses this strategy.

However, it has the disadvantage of initializing temporaries even for branches
not taken, though that could be mitigated by splitting non-critical edges and
putting assignments in the edge-split nodes.  Another problem is that without
extra machinery, the assignments to ``A``, ``A'``, ``X``, and ``X'`` are given a
specific ordering even though phi semantics are that the assignments are
parallel or unordered.  This sometimes imposes false live range overlaps and
leads to poorer register allocation.

Advanced phi lowering
^^^^^^^^^^^^^^^^^^^^^

``O2`` lowering defers phi lowering until after register allocation to avoid the
problem of false live range overlaps.  It works as follows.  We split each
incoming edge and move the (parallel) phi assignments into the split nodes.  We
linearize each set of assignments by finding a safe, topological ordering of the
assignments, respecting register assignments as well.  For example::

    A = B
    X = Y

Normally these assignments could be executed in either order, but if ``B`` and
``X`` are assigned the same physical register, we would want to use the above
ordering.  Dependency cycles are broken by introducing a temporary.  For
example::

    A = B
    B = A

Here, a temporary breaks the cycle::

    t = A
    A = B
    B = t

Finally, we use the existing target lowering to lower the assignments in this
basic block, and once that is done for all basic blocks, we run the post-phi
variant of register allocation on the edge-split basic blocks.

When computing a topological order, we try to first schedule assignments whose
source has a physical register, and last schedule assignments whose destination
has a physical register.  This helps reduce register pressure.

X86 address mode inference
^^^^^^^^^^^^^^^^^^^^^^^^^^

We try to take advantage of the x86 addressing mode that includes a base
register, an index register, an index register scale amount, and an immediate
offset.  We do this through simple pattern matching.  Starting with a load or
store instruction where the address is a variable, we initialize the base
register to that variable, and look up the instruction where that variable is
defined.  If that is an add instruction of two variables and the index register
hasn't been set, we replace the base and index register with those two
variables.  If instead it is an add instruction of a variable and a constant, we
replace the base register with the variable and add the constant to the
immediate offset.

There are several more patterns that can be matched.  This pattern matching
continues on the load or store instruction until no more matches are found.
Because a program typically has few load and store instructions (not to be
confused with instructions that manipulate stack variables), this address mode
inference pass is fast.

X86 read-modify-write inference
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

A reasonably common bitcode pattern is a non-atomic update of a memory
location::

    x = load addr
    y = add x, 1
    store y, addr

On x86, with good register allocation, the Subzero passes described above
generate code with only this quality::

    mov [%ebx], %eax
    add $1, %eax
    mov %eax, [%ebx]

However, x86 allows for this kind of code::

    add $1, [%ebx]

which requires fewer instructions, but perhaps more importantly, requires fewer
physical registers.

It's also important to note that this transformation only makes sense if the
store instruction ends ``x``'s live range.

Subzero's ``O2`` recipe includes an early pass to find read-modify-write (RMW)
opportunities via simple pattern matching.  The only problem is that it is run
before liveness analysis, which is needed to determine whether ``x``'s live
range ends after the RMW.  Since liveness analysis is one of the most expensive
passes, it's not attractive to run it an extra time just for RMW analysis.
Instead, we essentially generate both the RMW and the non-RMW versions, and then
during lowering, the RMW version deletes itself if it finds x still live.

X86 compare-branch inference
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In the LLVM instruction set, the compare/branch pattern works like this::

    cond = icmp eq a, b
    br cond, target

The result of the icmp instruction is a single bit, and a conditional branch
tests that bit.  By contrast, most target architectures use this pattern::

    cmp a, b  // implicitly sets various bits of FLAGS register
    br eq, target  // branch on a particular FLAGS bit

A naive lowering sequence conditionally sets ``cond`` to 0 or 1, then tests
``cond`` and conditionally branches.  Subzero has a pass that identifies
boolean-based operations like this and folds them into a single
compare/branch-like operation.  It is set up for more than just cmp/br though.
Boolean producers include icmp (integer compare), fcmp (floating-point compare),
and trunc (integer truncation when the destination has bool type).  Boolean
consumers include branch, select (the ternary operator from the C language), and
sign-extend and zero-extend when the source has bool type.

Code emission
-------------

Subzero's integrated assembler is derived from Dart's `assembler code
<https://github.com/dart-lang/sdk/tree/master/runtime/vm>'_.  There is a pass
that iterates through the low-level ICE instructions and invokes the relevant
assembler functions.  Placeholders are added for later fixup of branch target
offsets.  (Backward branches use short offsets if possible; forward branches
generally use long offsets unless it is an intra-block branch of "known" short
length.)  The assembler emits into a staging buffer.  Once emission into the
staging buffer for a function is complete, the data is emitted to the output
file as an ELF object file, and metadata such as relocations, symbol table, and
string table, are accumulated for emission at the end.  Global data initializers
are emitted similarly.  A key point is that at this point, the staging buffer
can be deallocated, and only a minimum of data needs to held until the end.

As a debugging alternative, Subzero can emit textual assembly code which can
then be run through an external assembler.  This is of course super slow, but
quite valuable when bringing up a new target.

As another debugging option, the staging buffer can be emitted as textual
assembly, primarily in the form of ".byte" lines.  This allows the assembler to
be tested separately from the ELF related code.

Memory management
-----------------

Where possible, we allocate from a ``CfgLocalAllocator`` which derives from
LLVM's ``BumpPtrAllocator``.  This is an arena-style allocator where objects
allocated from the arena are never actually freed; instead, when the CFG
translation completes and the CFG is deleted, the entire arena memory is
reclaimed at once.  This style of allocation works well in an environment like a
compiler where there are distinct phases with only easily-identifiable objects
living across phases.  It frees the developer from having to manage object
deletion, and it amortizes deletion costs across just a single arena deletion at
the end of the phase.  Furthermore, it helps scalability by allocating entirely
from thread-local memory pools, and minimizing global locking of the heap.

Instructions are probably the most heavily allocated complex class in Subzero.
We represent an instruction list as an intrusive doubly linked list, allocate
all instructions from the ``CfgLocalAllocator``, and we make sure each
instruction subclass is basically `POD
<http://en.cppreference.com/w/cpp/concept/PODType>`_ (Plain Old Data) with a
trivial destructor.  This way, when the CFG is finished, we don't need to
individually deallocate every instruction.  We do similar for Variables, which
is probably the second most popular complex class.

There are some situations where passes need to use some `STL container class
<http://en.cppreference.com/w/cpp/container>`_.  Subzero has a way of using the
``CfgLocalAllocator`` as the container allocator if this is needed.

Multithreaded translation
-------------------------

Subzero is designed to be able to translate functions in parallel.  With the
``-threads=N`` command-line option, there is a 3-stage producer-consumer
pipeline:

- A single thread parses the ``.pexe`` file and produces a sequence of work
  units.  A work unit can be either a fully constructed CFG, or a set of global
  initializers.  The work unit includes its sequence number denoting its parse
  order.  Each work unit is added to the translation queue.

- There are N translation threads that draw work units from the translation
  queue and lower them into assembler buffers.  Each assembler buffer is added
  to the emitter queue, tagged with its sequence number.  The CFG and its
  ``CfgLocalAllocator`` are disposed of at this point.

- A single thread draws assembler buffers from the emitter queue and appends to
  the output file.  It uses the sequence numbers to reintegrate the assembler
  buffers according to the original parse order, such that output order is
  always deterministic.

This means that with ``-threads=N``, there are actually ``N+1`` spawned threads
for a total of ``N+2`` execution threads, taking the parser and emitter threads
into account.  For the special case of ``N=0``, execution is entirely sequential
-- the same thread parses, translates, and emits, one function at a time.  This
is useful for performance measurements.

Ideally, we would like to get near-linear scalability as the number of
translation threads increases.  We expect that ``-threads=1`` should be slightly
faster than ``-threads=0`` as the small amount of time spent parsing and
emitting is done largely in parallel with translation.  With perfect
scalability, we see ``-threads=N`` translating ``N`` times as fast as
``-threads=1``, up until the point where parsing or emitting becomes the
bottleneck, or ``N+2`` exceeds the number of CPU cores.  In reality, memory
performance would become a bottleneck and efficiency might peak at, say, 75%.

Currently, parsing takes about 11% of total sequential time.  If translation
scalability ever gets so fast and awesomely scalable that parsing becomes a
bottleneck, it should be possible to make parsing multithreaded as well.

Internally, all shared, mutable data is held in the GlobalContext object, and
access to each field is guarded by a mutex.

Security
--------

Subzero includes a number of security features in the generated code, as well as
in the Subzero translator itself, which run on top of the existing Native Client
sandbox as well as Chrome's OS-level sandbox.

Sandboxed translator
^^^^^^^^^^^^^^^^^^^^

When running inside the browser, the Subzero translator executes as sandboxed,
untrusted code that is initially checked by the validator, just like the
LLVM-based ``pnacl-llc`` translator.  As such, the Subzero binary should be no
more or less secure than the translator it replaces, from the point of view of
the Chrome sandbox.  That said, Subzero is much smaller than ``pnacl-llc`` and
was designed from the start with security in mind, so one expects fewer attacker
opportunities here.

Fuzzing
^^^^^^^

We have started fuzz-testing the ``.pexe`` files input to Subzero, using a
combination of `afl-fuzz <http://lcamtuf.coredump.cx/afl/>`_, LLVM's `libFuzzer
<http://llvm.org/docs/LibFuzzer.html>`_, and custom tooling.  The purpose is to
find and fix cases where Subzero crashes or otherwise ungracefully fails on
unexpected inputs, and to do so automatically over a large range of unexpected
inputs.  By fixing bugs that arise from fuzz testing, we reduce the possibility
of an attacker exploiting these bugs.

Most of the problems found so far are ones most appropriately handled in the
parser.  However, there have been a couple that have identified problems in the
lowering, or otherwise inappropriately triggered assertion failures and fatal
errors.  We continue to dig into this area.

Future security work
^^^^^^^^^^^^^^^^^^^^

Subzero is well-positioned to explore other future security enhancements, e.g.:

- Tightening the Native Client sandbox.  ABI changes, such as the previous work
  on `hiding the sandbox base address
  <https://docs.google.com/document/d/1eskaI4353XdsJQFJLRnZzb_YIESQx4gNRzf31dqXVG8>`_
  in x86-64, are easy to experiment with in Subzero.

- Making the executable code section read-only.  This would prevent a PNaCl
  application from inspecting its own binary and trying to find ROP gadgets even
  after code diversification has been performed.  It may still be susceptible to
  `blind ROP <http://www.scs.stanford.edu/brop/bittau-brop.pdf>`_ attacks,
  security is still overall improved.

- Instruction selection diversification.  It may be possible to lower a given
  instruction in several largely equivalent ways, which gives more opportunities
  for code randomization.

Chrome integration
------------------

Currently Subzero is available in Chrome for the x86-32 architecture, but under
a flag.  When the flag is enabled, Subzero is used when the `manifest file
<https://developer.chrome.com/native-client/reference/nacl-manifest-format>`_
linking to the ``.pexe`` file specifies the ``O0`` optimization level.

The next step is to remove the flag, i.e. invoke Subzero as the only translator
for ``O0``-specified manifest files.

Ultimately, Subzero might produce code rivaling LLVM ``O2`` quality, in which
case Subzero could be used for all PNaCl translation.

Command line options
--------------------

Subzero has a number of command-line options for debugging and diagnostics.
Among the more interesting are the following.

- Using the ``-verbose`` flag, Subzero will dump the CFG, or produce other
  diagnostic output, with various levels of detail after each pass.  Instruction
  numbers can be printed or suppressed.  Deleted instructions can be printed or
  suppressed (they are retained in the instruction list, as discussed earlier,
  because they can help explain how lower-level instructions originated).
  Liveness information can be printed when available.  Details of register
  allocation can be printed as register allocator decisions are made.  And more.

- Running Subzero with any level of verbosity produces an enormous amount of
  output.  When debugging a single function, verbose output can be suppressed
  except for a particular function.  The ``-verbose-focus`` flag suppresses
  verbose output except for the specified function.

- Subzero has a ``-timing`` option that prints a breakdown of pass-level timing
  at exit.  Timing markers can be placed in the Subzero source code to demarcate
  logical operations or passes of interest.  Basic timing information plus
  call-stack type timing information is printed at the end.

- Along with ``-timing``, the user can instead get a report on the overall
  translation time for each function, to help focus on timing outliers.  Also,
  ``-timing-focus`` limits the ``-timing`` reporting to a single function,
  instead of aggregating pass timing across all functions.

- The ``-szstats`` option reports various statistics on each function, such as
  stack frame size, static instruction count, etc.  It may be helpful to track
  these stats over time as Subzero is improved, as an approximate measure of
  code quality.

- The flag ``-asm-verbose``, in conjunction with emitting textual assembly
  output, annotate the assembly output with register-focused liveness
  information.  In particular, each basic block is annotated with which
  registers are live-in and live-out, and each instruction is annotated with
  which registers' and stack locations' live ranges end at that instruction.
  This is really useful when studying the generated code to find opportunities
  for code quality improvements.

Testing and debugging
---------------------

LLVM lit tests
^^^^^^^^^^^^^^

For basic testing, Subzero uses LLVM's `lit
<http://llvm.org/docs/CommandGuide/lit.html>`_ framework for running tests.  We
have a suite of hundreds of small functions where we test for particular
assembly code patterns across different target architectures.

Cross tests
^^^^^^^^^^^

Unfortunately, the lit tests don't do a great job of precisely testing the
correctness of the output.  Much better are the cross tests, which are execution
tests that compare Subzero and ``pnacl-llc`` translated bitcode across a wide
variety of interesting inputs.  Each cross test consists of a set of C, C++,
and/or low-level bitcode files.  The C and C++ source files are compiled down to
bitcode.  The bitcode files are translated by ``pnacl-llc`` and also by Subzero.
Subzero mangles global symbol names with a special prefix to avoid duplicate
symbol errors.  A driver program invokes both versions on a large set of
interesting inputs, and reports when the Subzero and ``pnacl-llc`` results
differ.  Cross tests turn out to be an excellent way of testing the basic
lowering patterns, but they are less useful for testing more global things like
liveness analysis and register allocation.

Bisection debugging
^^^^^^^^^^^^^^^^^^^

Sometimes with a new application, Subzero will end up producing incorrect code
that either crashes at runtime or otherwise produces the wrong results.  When
this happens, we need to narrow it down to a single function (or small set of
functions) that yield incorrect behavior.  For this, we have a bisection
debugging framework.  Here, we initially translate the entire application once
with Subzero and once with ``pnacl-llc``.  We then use ``objdump`` to
selectively weaken symbols based on a list provided on the command line. The
two object files can then be linked together without link errors, with the
desired version of each method "winning".  Then the binary is tested, and
bisection proceeds based on whether the binary produces correct output.

When the bisection completes, we are left with a minimal set of
Subzero-translated functions that cause the failure.  Usually it is a single
function, though sometimes it might require a combination of several functions
to cause a failure; this may be due to an incorrect call ABI, for example.
However, Murphy's Law implies that the single failing function is enormous and
impractical to debug.  In that case, we can restart the bisection, explicitly
ignoring the enormous function, and try to find another candidate to debug.
(Future work is to automate this to find all minimal sets of functions, so that
debugging can focus on the simplest example.)

Fuzz testing
^^^^^^^^^^^^

As described above, we try to find internal Subzero bugs using fuzz testing
techniques.

Sanitizers
^^^^^^^^^^

Subzero can be built with `AddressSanitizer
<http://clang.llvm.org/docs/AddressSanitizer.html>`_ (ASan) or `ThreadSanitizer
<http://clang.llvm.org/docs/ThreadSanitizer.html>`_ (TSan) support.  This is
done using something as simple as ``make ASAN=1`` or ``make TSAN=1``.  So far,
multithreading has been simple enough that TSan hasn't found any bugs, but ASan
has found at least one memory leak which was subsequently fixed.
`UndefinedBehaviorSanitizer
<http://clang.llvm.org/docs/UsersManual.html#controlling-code-generation>`_
(UBSan) support is in progress.  `Control flow integrity sanitization
<http://clang.llvm.org/docs/ControlFlowIntegrity.html>`_ is also under
consideration.

Current status
==============

Target architectures
--------------------

Subzero is currently more or less complete for the x86-32 target.  It has been
refactored and extended to handle x86-64 as well, and that is mostly complete at
this point.

ARM32 work is in progress.  It currently lacks the testing level of x86, at
least in part because Subzero's register allocator needs modifications to handle
ARM's aliasing of floating point and vector registers.  Specifically, a 64-bit
register is actually a gang of two consecutive and aligned 32-bit registers, and
a 128-bit register is a gang of 4 consecutive and aligned 32-bit registers.
ARM64 work has not started; when it does, it will be native-only since the
Native Client sandbox model, validator, and other tools have never been defined.

An external contributor is adding MIPS support, in most part by following the
ARM work.

Translator performance
----------------------

Single-threaded translation speed is currently about 5× the ``pnacl-llc``
translation speed.  For a large ``.pexe`` file, the time breaks down as:

- 11% for parsing and initial IR building

- 4% for emitting to /dev/null

- 27% for liveness analysis (two liveness passes plus live range construction)

- 15% for linear-scan register allocation

- 9% for basic lowering

- 10% for advanced phi lowering

- ~11% for other minor analysis

- ~10% measurement overhead to acquire these numbers

Some improvements could undoubtedly be made, but it will be hard to increase the
speed to 10× of ``pnacl-llc`` while keeping acceptable code quality.  With
``-Om1`` (lack of) optimization, we do actually achieve roughly 10×
``pnacl-llc`` translation speed, but code quality drops by a factor of 3.

Code quality
------------

Measured across 16 components of spec2k, Subzero's code quality is uniformly
better than ``pnacl-llc`` ``-O0`` code quality, and in many cases solidly
between ``pnacl-llc`` ``-O0`` and ``-O2``.

Translator size
---------------

When built in MINIMAL mode, the x86-64 native translator size for the x86-32
target is about 700 KB, not including the size of functions referenced in
dynamically-linked libraries.  The sandboxed version of Subzero is a bit over 1
MB, and it is statically linked and also includes nop padding for bundling as
well as indirect branch masking.

Translator memory footprint
---------------------------

It's hard to draw firm conclusions about memory footprint, since the footprint
is at least proportional to the input function size, and there is no real limit
on the size of functions in the ``.pexe`` file.

That said, we looked at the memory footprint over time as Subzero translated
``pnacl-llc.pexe``, which is the largest ``.pexe`` file (7.2 MB) at our
disposal.  One of LLVM's libraries that Subzero uses can report the current
malloc heap usage.  With single-threaded translation, Subzero tends to hover
around 15 MB of memory usage.  There are a couple of monstrous functions where
Subzero grows to around 100 MB, but then it drops back down after those
functions finish translating.  In contrast, ``pnacl-llc`` grows larger and
larger throughout translation, reaching several hundred MB by the time it
completes.

It's a bit more interesting when we enable multithreaded translation.  When
there are N translation threads, Subzero implements a policy that limits the
size of the translation queue to N entries -- if it is "full" when the parser
tries to add a new CFG, the parser blocks until one of the translation threads
removes a CFG.  This means the number of in-memory CFGs can (and generally does)
reach 2*N+1, and so the memory footprint rises in proportion to the number of
threads.  Adding to the pressure is the observation that the monstrous functions
also take proportionally longer time to translate, so there's a good chance many
of the monstrous functions will be active at the same time with multithreaded
translation.  As a result, for N=32, Subzero's memory footprint peaks at about
260 MB, but drops back down as the large functions finish translating.

If this peak memory size becomes a problem, it might be possible for the parser
to resequence the functions to try to spread out the larger functions, or to
throttle the translation queue to prevent too many in-flight large functions.
It may also be possible to throttle based on memory pressure signaling from
Chrome.

Translator scalability
----------------------

Currently scalability is "not very good".  Multiple translation threads lead to
faster translation, but not to the degree desired.  We haven't dug in to
investigate yet.

There are a few areas to investigate.  First, there may be contention on the
constant pool, which all threads access, and which requires locked access even
for reading.  This could be mitigated by keeping a CFG-local cache of the most
common constants.

Second, there may be contention on memory allocation.  While almost all CFG
objects are allocated from the CFG-local allocator, some passes use temporary
STL containers that use the default allocator, which may require global locking.
This could be mitigated by switching these to the CFG-local allocator.

Third, multithreading may make the default allocator strategy more expensive.
In a single-threaded environment, a pass will allocate its containers, run the
pass, and deallocate the containers.  This results in stack-like allocation
behavior and makes the heap free list easier to manage, with less heap
fragmentation.  But when multithreading is added, the allocations and
deallocations become much less stack-like, making allocation and deallocation
operations individually more expensive.  Again, this could be mitigated by
switching these to the CFG-local allocator.
