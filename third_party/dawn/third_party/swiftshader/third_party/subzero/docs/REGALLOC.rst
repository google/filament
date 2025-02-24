Register allocation in Subzero
==============================

Introduction
------------

`Subzero
<https://chromium.googlesource.com/native_client/pnacl-subzero/+/master/docs/DESIGN.rst>`_
is a fast code generator that translates architecture-independent `PNaCl bitcode
<https://developer.chrome.com/native-client/reference/pnacl-bitcode-abi>`_ into
architecture-specific machine code.  PNaCl bitcode is LLVM bitcode that has been
simplified (e.g. weird-width primitive types like 57-bit integers are not
allowed) and has had architecture-independent optimizations already applied.
Subzero aims to generate high-quality code as fast as practical, and as such
Subzero needs to make tradeoffs between compilation speed and output code
quality.

In Subzero, we have found register allocation to be one of the most important
optimizations toward achieving the best code quality, which is in tension with
register allocation's reputation for being complex and expensive.  Linear-scan
register allocation is a modern favorite for getting fairly good register
allocation at relatively low cost.  Subzero uses linear-scan for its core
register allocator, with a few internal modifications to improve its
effectiveness (specifically: register preference, register overlap, and register
aliases).  Subzero also does a few high-level things on top of its core register
allocator to improve overall effectiveness (specifically: repeat until
convergence, delayed phi lowering, and local live range splitting).

What we describe here are techniques that have worked well for Subzero, in the
context of its particular intermediate representation (IR) and compilation
strategy.  Some of these techniques may not be applicable to another compiler,
depending on its particular IR and compilation strategy.  Some key concepts in
Subzero are the following:

- Subzero's ``Variable`` operand is an operand that resides either on the stack
  or in a physical register.  A Variable can be tagged as *must-have-register*
  or *must-not-have-register*, but its default is *may-have-register*.  All uses
  of the Variable map to the same physical register or stack location.

- Basic lowering is done before register allocation.  Lowering is the process of
  translating PNaCl bitcode instructions into native instructions.  Sometimes a
  native instruction, like the x86 ``add`` instruction, allows one of its
  Variable operands to be either in a physical register or on the stack, in
  which case the lowering is relatively simple.  But if the lowered instruction
  requires the operand to be in a physical register, we generate code that
  copies the Variable into a *must-have-register* temporary, and then use that
  temporary in the lowered instruction.

- Instructions within a basic block appear in a linearized order (as opposed to
  something like a directed acyclic graph of dependency edges).

- An instruction has 0 or 1 *dest* Variables and an arbitrary (but usually
  small) number of *source* Variables.  Assuming SSA form, the instruction
  begins the live range of the dest Variable, and may end the live range of one
  or more of the source Variables.

Executive summary
-----------------

- Liveness analysis and live range construction are prerequisites for register
  allocation.  Without careful attention, they can be potentially very
  expensive, especially when the number of variables and basic blocks gets very
  large.  Subzero uses some clever data structures to take advantage of the
  sparsity of the data, resulting in stable performance as function size scales
  up.  This means that the proportion of compilation time spent on liveness
  analysis stays roughly the same.

- The core of Subzero's register allocator is taken from `Mössenböck and
  Pfeiffer's paper <ftp://ftp.ssw.uni-linz.ac.at/pub/Papers/Moe02.PDF>`_ on
  linear-scan register allocation.

- We enhance the core algorithm with a good automatic preference mechanism when
  more than one register is available, to try to minimize register shuffling.

- We also enhance it to allow overlapping live ranges to share the same
  register, when one variable is recognized as a read-only copy of the other.

- We deal with register aliasing in a clean and general fashion.  Register
  aliasing is when e.g. 16-bit architectural registers share some bits with
  their 32-bit counterparts, or 64-bit registers are actually pairs of 32-bit
  registers.

- We improve register allocation by rerunning the algorithm on likely candidates
  that didn't get registers during the previous iteration, without imposing much
  additional cost.

- The main register allocation is done before phi lowering, because phi lowering
  imposes early and unnecessary ordering constraints on the resulting
  assigments, which create spurious interferences in live ranges.

- Within each basic block, we aggressively split each variable's live range at
  every use, so that portions of the live range can get registers even if the
  whole live range can't.  Doing this separately for each basic block avoids
  merge complications, and keeps liveness analysis and register allocation fast
  by fitting well into the sparsity optimizations of their data structures.

Liveness analysis
-----------------

With respect to register allocation, the main purpose of liveness analysis is to
calculate the live range of each variable.  The live range is represented as a
set of instruction number ranges.  Instruction numbers within a basic block must
be monotonically increasing, and the instruction ranges of two different basic
blocks must not overlap.

Basic liveness
^^^^^^^^^^^^^^

Liveness analysis is a straightforward dataflow algorithm.  For each basic
block, we keep track of the live-in and live-out set, i.e. the set of variables
that are live coming into or going out of the basic block.  Processing of a
basic block starts by initializing a temporary set as the union of the live-in
sets of the basic block's successor blocks.  (This basic block's live-out set is
captured as the initial value of the temporary set.)  Then each instruction of
the basic block is processed in reverse order.  All the source variables of the
instruction are marked as live, by adding them to the temporary set, and the
dest variable of the instruction (if any) is marked as not-live, by removing it
from the temporary set.  When we finish processing all of the block's
instructions, we add/union the temporary set into the basic block's live-in set.
If this changes the basic block's live-in set, then we mark all of this basic
block's predecessor blocks to be reprocessed.  Then we repeat for other basic
blocks until convergence, i.e. no more basic blocks are marked to be
reprocessed.  If basic blocks are processed in reverse topological order, then
the number of times each basic block need to be reprocessed is generally its
loop nest depth.

The result of this pass is the live-in and live-out set for each basic block.

With so many set operations, choice of data structure is crucial for
performance.  We tried a few options, and found that a simple dense bit vector
works best.  This keeps the per-instruction cost very low.  However, we found
that when the function gets very large, merging and copying bit vectors at basic
block boundaries dominates the cost.  This is due to the number of variables
(hence the bit vector size) and the number of basic blocks getting large.

A simple enhancement brought this under control in Subzero.  It turns out that
the vast majority of variables are referenced, and therefore live, only in a
single basic block.  This is largely due to the SSA form of PNaCl bitcode.  To
take advantage of this, we partition variables by single-block versus
multi-block liveness.  Multi-block variables get lower-numbered bit vector
indexes, and single-block variables get higher-number indexes.  Single-block bit
vector indexes are reused across different basic blocks.  As such, the size of
live-in and live-out bit vectors is limited to the number of multi-block
variables, and the temporary set's size can be limited to that plus the largest
number of single-block variables across all basic blocks.

For the purpose of live range construction, we also need to track definitions
(LiveBegin) and last-uses (LiveEnd) of variables used within instructions of the
basic block.  These are easy to detect while processing the instructions; data
structure choices are described below.

Live range construction
^^^^^^^^^^^^^^^^^^^^^^^

After the live-in and live-out sets are calculated, we construct each variable's
live range (as an ordered list of instruction ranges, described above).  We do
this by considering the live-in and live-out sets, combined with LiveBegin and
LiveEnd information.  This is done separately for each basic block.

As before, we need to take advantage of sparsity of variable uses across basic
blocks, to avoid overly copying/merging data structures.  The following is what
worked well for Subzero (after trying several other options).

The basic liveness pass, described above, keeps track of when a variable's live
range begins or ends within the block.  LiveBegin and LiveEnd are unordered
vectors where each element is a pair of the variable and the instruction number,
representing that the particular variable's live range begins or ends at the
particular instruction.  When the liveness pass finds a variable whose live
range begins or ends, it appends and entry to LiveBegin or LiveEnd.

During live range construction, the LiveBegin and LiveEnd vectors are sorted by
variable number.  Then we iterate across both vectors in parallel.  If a
variable appears in both LiveBegin and LiveEnd, then its live range is entirely
within this block.  If it appears in only LiveBegin, then its live range starts
here and extends through the end of the block.  If it appears in only LiveEnd,
then its live range starts at the beginning of the block and ends here.  (Note
that this only covers the live range within this block, and this process is
repeated across all blocks.)

It is also possible that a variable is live within this block but its live range
does not begin or end in this block.  These variables are identified simply by
taking the intersection of the live-in and live-out sets.

As a result of these data structures, performance of liveness analysis and live
range construction tend to be stable across small, medium, and large functions,
as measured by a fairly consistent proportion of total compilation time spent on
the liveness passes.

Linear-scan register allocation
-------------------------------

The basis of Subzero's register allocator is the allocator described by
Hanspeter Mössenböck and Michael Pfeiffer in `Linear Scan Register Allocation in
the Context of SSA Form and Register Constraints
<ftp://ftp.ssw.uni-linz.ac.at/pub/Papers/Moe02.PDF>`_.  It allows live ranges to
be a union of intervals rather than a single conservative interval, and it
allows pre-coloring of variables with specific physical registers.

The paper suggests an approach of aggressively coalescing variables across Phi
instructions (i.e., trying to force Phi source and dest variables to have the
same register assignment), but we omit that in favor of the more natural
preference mechanism described below.

We found the paper quite remarkable in that a straightforward implementation of
its pseudo-code led to an entirely correct register allocator.  The only thing
we found in the specification that was even close to a mistake is that it was
too aggressive in evicting inactive ranges in the "else" clause of the
AssignMemLoc routine.  An inactive range only needs to be evicted if it actually
overlaps the current range being considered, whereas the paper evicts it
unconditionally.  (Search for "original paper" in Subzero's register allocator
source code.)

Register preference
-------------------

The linear-scan algorithm from the paper talks about choosing an available
register, but isn't specific on how to choose among several available registers.
The simplest approach is to just choose the first available register, e.g. the
lowest-numbered register.  Often a better choice is possible.

Specifically, if variable ``V`` is assigned in an instruction ``V=f(S1,S2,...)``
with source variables ``S1,S2,...``, and that instruction ends the live range of
one of those source variables ``Sn``, and ``Sn`` was assigned a register, then
``Sn``'s register is usually a good choice for ``V``.  This is especially true
when the instruction is a simple assignment, because an assignment where the
dest and source variables end up with the same register can be trivially elided,
reducing the amount of register-shuffling code.

This requires being able to find and inspect a variable's defining instruction,
which is not an assumption in the original paper but is easily tracked in
practice.

Register overlap
----------------

Because Subzero does register allocation after basic lowering, the lowering has
to be prepared for the possibility of any given program variable not getting a
physical register.  It does this by introducing *must-have-register* temporary
variables, and copies the program variable into the temporary to ensure that
register requirements in the target instruction are met.

In many cases, the program variable and temporary variable have overlapping live
ranges, and would be forced to have different registers even if the temporary
variable is effectively a read-only copy of the program variable.  We recognize
this when the program variable has no definitions within the temporary
variable's live range, and the temporary variable has no definitions within the
program variable's live range with the exception of the copy assignment.

This analysis is done as part of register preference detection.

The main impact on the linear-scan implementation is that instead of
setting/resetting a boolean flag to indicate whether a register is free or in
use, we increment/decrement a number-of-uses counter.

Register aliases
----------------

Sometimes registers of different register classes partially overlap.  For
example, in x86, registers ``al`` and ``ah`` alias ``ax`` (though they don't
alias each other), and all three alias ``eax`` and ``rax``.  And in ARM,
registers ``s0`` and ``s1`` (which are single-precision floating-point
registers) alias ``d0`` (double-precision floating-point), and registers ``d0``
and ``d1`` alias ``q0`` (128-bit vector register).  The linear-scan paper
doesn't address this issue; it assumes that all registers are independent.  A
simple solution is to essentially avoid aliasing by disallowing a subset of the
registers, but there is obviously a reduction in code quality when e.g. half of
the registers are taken away.

Subzero handles this more elegantly.  For each register, we keep a bitmask
indicating which registers alias/conflict with it.  For example, in x86,
``ah``'s alias set is ``ah``, ``ax``, ``eax``, and ``rax`` (but not ``al``), and
in ARM, ``s1``'s alias set is ``s1``, ``d0``, and ``q0``.  Whenever we want to
mark a register as being used or released, we do the same for all of its
aliases.

Before implementing this, we were a bit apprehensive about the compile-time
performance impact.  Instead of setting one bit in a bit vector or decrementing
one counter, this generally needs to be done in a loop that iterates over all
aliases.  Fortunately, this seemed to make very little difference in
performance, as the bulk of the cost ends up being in live range overlap
computations, which are not affected by register aliasing.

Repeat until convergence
------------------------

Sometimes the linear-scan algorithm makes a register assignment only to later
revoke it in favor of a higher-priority variable, but it turns out that a
different initial register choice would not have been revoked.  For relatively
low compile-time cost, we can give those variables another chance.

During register allocation, we keep track of the revoked variables and then do
another round of register allocation targeted only to that set.  We repeat until
no new register assignments are made, which is usually just a handful of
successively cheaper iterations.

Another approach would be to repeat register allocation for *all* variables that
haven't had a register assigned (rather than variables that got a register that
was subsequently revoked), but our experience is that this greatly increases
compile-time cost, with little or no code quality gain.

Delayed Phi lowering
--------------------

The linear-scan algorithm works for phi instructions as well as regular
instructions, but it is tempting to lower phi instructions into non-SSA
assignments before register allocation, so that register allocation need only
happen once.

Unfortunately, simple phi lowering imposes an arbitrary ordering on the
resulting assignments that can cause artificial overlap/interference between
lowered assignments, and can lead to worse register allocation decisions.  As a
simple example, consider these two phi instructions which are semantically
unordered::

  A = phi(B) // B's live range ends here
  C = phi(D) // D's live range ends here

A straightforward lowering might yield::

  A = B // end of B's live range
  C = D // end of D's live range

The potential problem here is that A and D's live ranges overlap, and so they
are prevented from having the same register.  Swapping the order of lowered
assignments fixes that (but then B and C would overlap), but we can't really
know which is better until after register allocation.

Subzero deals with this by doing the main register allocation before phi
lowering, followed by phi lowering, and finally a special register allocation
pass limited to the new lowered assignments.

Phi lowering considers the phi operands separately for each predecessor edge,
and starts by finding a topological sort of the Phi instructions, such that
assignments can be executed in that order without violating dependencies on
registers or stack locations.  If a topological sort is not possible due to a
cycle, the cycle is broken by introducing a temporary, e.g. ``A=B;B=C;C=A`` can
become ``T=A;A=B;B=C;C=T``.  The topological order is tuned to favor freeing up
registers early to reduce register pressure.

It then lowers the linearized assignments into machine instructions (which may
require extra physical registers e.g. to copy from one stack location to
another), and finally runs the register allocator limited to those instructions.

In rare cases, the register allocator may fail on some *must-have-register*
variable, because register pressure is too high to satisfy requirements arising
from cycle-breaking temporaries and registers required for stack-to-stack
copies.  In this case, we have to find a register with no active uses within the
variable's live range, and actively spill/restore that register around the live
range.  This makes the code quality suffer and may be slow to implement
depending on compiler data structures, but in practice we find the situation to
be vanishingly rare and so not really worth optimizing.

Local live range splitting
--------------------------

The basic linear-scan algorithm has an "all-or-nothing" policy: a variable gets
a register for its entire live range, or not at all.  This is unfortunate when a
variable has many uses close together, but ultimately a large enough live range
to prevent register assignment.  Another bad example is on x86 where all vector
and floating-point registers (the ``xmm`` registers) are killed by call
instructions, per the x86 call ABI, so such variables are completely prevented
from having a register when their live ranges contain a call instruction.

The general solution here is some policy for splitting live ranges.  A variable
can be split into multiple copies and each can be register-allocated separately.
The complication comes in finding a sane policy for where and when to split
variables such that complexity doesn't explode, and how to join the different
values at merge points.

Subzero implements aggressive block-local splitting of variables.  Each basic
block is handled separately and independently.  Within the block, we maintain a
table ``T`` that maps each variable ``V`` to its split version ``T[V]``, with
every variable ``V``'s initial state set (implicitly) as ``T[V]=V``.  For each
instruction in the block, and for each *may-have-register* variable ``V`` in the
instruction, we do the following:

- Replace all uses of ``V`` in the instruction with ``T[V]``.

- Create a new split variable ``V'``.

- Add a new assignment ``V'=T[V]``, placing it adjacent to (either immediately
  before or immediately after) the current instruction.

- Update ``T[V]`` to be ``V'``.

This leads to a chain of copies of ``V`` through the block, linked by assignment
instructions.  The live ranges of these copies are usually much smaller, and
more likely to get registers.  In fact, because of the preference mechanism
described above, they are likely to get the same register whenever possible.

One obvious question comes up: won't this proliferation of new variables cause
an explosion in the running time of liveness analysis and register allocation?
As it turns out, not really.

First, for register allocation, the cost tends to be dominated by live range
overlap computations, whose cost is roughly proportional to the size of the live
range.  All the new variable copies' live ranges sum up to the original
variable's live range, so the cost isn't vastly greater.

Second, for liveness analysis, the cost is dominated by merging bit vectors
corresponding to the set of variables that have multi-block liveness.  All the
new copies are guaranteed to be single-block, so the main additional cost is
that of processing the new assignments.

There's one other key issue here.  The original variable and all of its copies
need to be "linked", in the sense that all of these variables that get a stack
slot (because they didn't get a register) are guaranteed to have the same stack
slot.  This way, we can avoid generating any code related to ``V'=V`` when
neither gets a register.  In addition, we can elide instructions that write a
value to a stack slot that is linked to another stack slot, because that is
guaranteed to be just rewriting the same value to the stack.
