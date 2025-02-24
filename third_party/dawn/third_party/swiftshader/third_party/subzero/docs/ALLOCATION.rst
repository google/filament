Object allocation and lifetime in ICE
=====================================

This document discusses object lifetime and scoping issues, starting with
bitcode parsing and ending with ELF file emission.

Multithreaded translation model
-------------------------------

A single thread is responsible for parsing PNaCl bitcode (possibly concurrently
with downloading the bitcode file) and constructing the initial high-level ICE.
The result is a queue of Cfg pointers.  The parser thread incrementally adds a
Cfg pointer to the queue after the Cfg is created, and then moves on to parse
the next function.

Multiple translation worker threads draw from the queue of Cfg pointers as they
are added to the queue, such that several functions can be translated in parallel.
The result is a queue of assembler buffers, each of which consists of machine code
plus fixups.

A single thread is responsible for writing the assembler buffers to an ELF file.
It consumes the assembler buffers from the queue that the translation threads
write to.

This means that Cfgs are created by the parser thread and destroyed by the
translation thread (including Cfg nodes, instructions, and most kinds of
operands), and assembler buffers are created by the translation thread and
destroyed by the writer thread.

Deterministic execution
^^^^^^^^^^^^^^^^^^^^^^^

Although code randomization is a key aspect of security, deterministic and
repeatable translation is sometimes needed, e.g. for regression testing.
Multithreaded translation introduces potential for randomness that may need to
be made deterministic.

* Bitcode parsing is sequential, so it's easy to use a FIFO queue to keep the
  translation queue in deterministic order.  But since translation is
  multithreaded, FIFO order for the assembler buffer queue may not be
  deterministic.  The writer thread would be responsible for reordering the
  buffers, potentially waiting for slower translations to complete even if other
  assembler buffers are available.

* Different translation threads may add new constant pool entries at different
  times.  Some constant pool entries are emitted as read-only data.  This
  includes floating-point constants for x86, as well as integer immediate
  randomization through constant pooling.  These constant pool entries are
  emitted after all assembler buffers have been written.  The writer needs to be
  able to sort them deterministically before emitting them.

Object lifetimes
----------------

Objects of type Constant, or a subclass of Constant, are pooled globally.  The
pooling is managed by the GlobalContext class.  Since Constants are added or
looked up by translation threads and the parser thread, access to the constant
pools, as well as GlobalContext in general, need to be arbitrated by locks.
(It's possible that if there's too much contention, we can maintain a
thread-local cache for Constant pool lookups.)  Constants live across all
function translations, and are destroyed only at the end.

Several object types are scoped within the lifetime of the Cfg.  These include
CfgNode, Inst, Variable, and any target-specific subclasses of Inst and Operand.
When the Cfg is destroyed, these scoped objects are destroyed as well.  To keep
this cheap, the Cfg includes a slab allocator from which these objects are
allocated, and the objects should not contain fields with non-trivial
destructors.  Most of these fields are POD, but in a couple of cases these
fields are STL containers.  We deal with this, and avoid leaking memory, by
providing the container with an allocator that uses the Cfg-local slab
allocator.  Since the container allocator generally needs to be stateless, we
store a pointer to the slab allocator in thread-local storage (TLS).  This is
straightforward since on any of the threads, only one Cfg is active at a time,
and a given Cfg is only active in one thread at a time (either the parser
thread, or at most one translation thread, or the writer thread).

Even though there is a one-to-one correspondence between Cfgs and assembler
buffers, they need to use different allocators.  This is because the translation
thread wants to destroy the Cfg and reclaim all its memory after translation
completes, but possibly before the assembly buffer is written to the ELF file.
Ownership of the assembler buffer and its allocator are transferred to the
writer thread after translation completes, similar to the way ownership of the
Cfg and its allocator are transferred to the translation thread after parsing
completes.

Allocators and TLS
------------------

Part of the Cfg building, and transformations on the Cfg, include STL container
operations which may need to allocate additional memory in a stateless fashion.
This requires maintaining the proper slab allocator pointer in TLS.

When the parser thread creates a new Cfg object, it puts a pointer to the Cfg's
slab allocator into its own TLS.  This is used as the Cfg is built within the
parser thread.  After the Cfg is built, the parser thread clears its allocator
pointer, adds the new Cfg pointer to the translation queue, continues with the
next function.

When the translation thread grabs a new Cfg pointer, it installs the Cfg's slab
allocator into its TLS and translates the function.  When generating the
assembly buffer, it must take care not to use the Cfg's slab allocator.  If
there is a slab allocator for the assembler buffer, a pointer to it can also be
installed in TLS if needed.

The translation thread destroys the Cfg when it is done translating, including
the Cfg's slab allocator, and clears the allocator pointer from its TLS.
Likewise, the writer thread destroys the assembler buffer when it is finished
with it.

Thread safety
-------------

The parse/translate/write stages of the translation pipeline are fairly
independent, with little opportunity for threads to interfere.  The Subzero
design calls for all shared accesses to go through the GlobalContext, which adds
locking as appropriate.  This includes the coarse-grain work queues for Cfgs and
assembler buffers.  It also includes finer-grain access to constant pool
entries, as well as output streams for verbose debugging output.

If locked access to constant pools becomes a bottleneck, we can investigate
thread-local caches of constants (as mentioned earlier).  Also, it should be
safe though slightly less efficient to allow duplicate copies of constants
across threads (which could be de-dupped by the writer at the end).

We will use ThreadSanitizer as a way to detect potential data races in the
implementation.
