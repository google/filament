Target-specific lowering in ICE
===============================

This document discusses several issues around generating target-specific ICE
instructions from high-level ICE instructions.

Meeting register address mode constraints
-----------------------------------------

Target-specific instructions often require specific operands to be in physical
registers.  Sometimes one specific register is required, but usually any
register in a particular register class will suffice, and that register class is
defined by the instruction/operand type.

The challenge is that ``Variable`` represents an operand that is either a stack
location in the current frame, or a physical register.  Register allocation
happens after target-specific lowering, so during lowering we generally don't
know whether a ``Variable`` operand will meet a target instruction's physical
register requirement.

To this end, ICE allows certain directives:

    * ``Variable::setWeightInfinite()`` forces a ``Variable`` to get some
      physical register (without specifying which particular one) from a
      register class.

    * ``Variable::setRegNum()`` forces a ``Variable`` to be assigned a specific
      physical register.

These directives are described below in more detail.  In most cases, though,
they don't need to be explicity used, as the routines that create lowered
instructions have reasonable defaults and simple options that control these
directives.

The recommended ICE lowering strategy is to generate extra assignment
instructions involving extra ``Variable`` temporaries, using the directives to
force suitable register assignments for the temporaries, and then let the
register allocator clean things up.

Note: There is a spectrum of *implementation complexity* versus *translation
speed* versus *code quality*.  This recommended strategy picks a point on the
spectrum representing very low complexity ("splat-isel"), pretty good code
quality in terms of frame size and register shuffling/spilling, but perhaps not
the fastest translation speed since extra instructions and operands are created
up front and cleaned up at the end.

Ensuring a non-specific physical register
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The x86 instruction::

    mov dst, src

needs at least one of its operands in a physical register (ignoring the case
where ``src`` is a constant).  This can be done as follows::

    mov reg, src
    mov dst, reg

so long as ``reg`` is guaranteed to have a physical register assignment.  The
low-level lowering code that accomplishes this looks something like::

    Variable *Reg;
    Reg = Func->makeVariable(Dst->getType());
    Reg->setWeightInfinite();
    NewInst = InstX8632Mov::create(Func, Reg, Src);
    NewInst = InstX8632Mov::create(Func, Dst, Reg);

``Cfg::makeVariable()`` generates a new temporary, and
``Variable::setWeightInfinite()`` gives it infinite weight for the purpose of
register allocation, thus guaranteeing it a physical register (though leaving
the particular physical register to be determined by the register allocator).

The ``_mov(Dest, Src)`` method in the ``TargetX8632`` class is sufficiently
powerful to handle these details in most situations.  Its ``Dest`` argument is
an in/out parameter.  If its input value is ``nullptr``, then a new temporary
variable is created, its type is set to the same type as the ``Src`` operand, it
is given infinite register weight, and the new ``Variable`` is returned through
the in/out parameter.  (This is in addition to the new temporary being the dest
operand of the ``mov`` instruction.)  The simpler version of the above example
is::

    Variable *Reg = nullptr;
    _mov(Reg, Src);
    _mov(Dst, Reg);

Preferring another ``Variable``'s physical register
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

(An older version of ICE allowed the lowering code to provide a register
allocation hint: if a physical register is to be assigned to one ``Variable``,
then prefer a particular ``Variable``'s physical register if available.  This
hint would be used to try to reduce the amount of register shuffling.
Currently, the register allocator does this automatically through the
``FindPreference`` logic.)

Ensuring a specific physical register
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Some instructions require operands in specific physical registers, or produce
results in specific physical registers.  For example, the 32-bit ``ret``
instruction needs its operand in ``eax``.  This can be done with
``Variable::setRegNum()``::

    Variable *Reg;
    Reg = Func->makeVariable(Src->getType());
    Reg->setWeightInfinite();
    Reg->setRegNum(Reg_eax);
    NewInst = InstX8632Mov::create(Func, Reg, Src);
    NewInst = InstX8632Ret::create(Func, Reg);

Precoloring with ``Variable::setRegNum()`` effectively gives it infinite weight
for register allocation, so the call to ``Variable::setWeightInfinite()`` is
technically unnecessary, but perhaps documents the intention a bit more
strongly.

The ``_mov(Dest, Src, RegNum)`` method in the ``TargetX8632`` class has an
optional ``RegNum`` argument to force a specific register assignment when the
input ``Dest`` is ``nullptr``.  As described above, passing in ``Dest=nullptr``
causes a new temporary variable to be created with infinite register weight, and
in addition the specific register is chosen.  The simpler version of the above
example is::

    Variable *Reg = nullptr;
    _mov(Reg, Src, Reg_eax);
    _ret(Reg);

Disabling live-range interference
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

(An older version of ICE allowed an overly strong preference for another
``Variable``'s physical register even if their live ranges interfered.  This was
risky, and currently the register allocator derives this automatically through
the ``AllowOverlap`` logic.)

Call instructions kill scratch registers
----------------------------------------

A ``call`` instruction kills the values in all scratch registers, so it's
important that the register allocator doesn't allocate a scratch register to a
``Variable`` whose live range spans the ``call`` instruction.  ICE provides the
``InstFakeKill`` pseudo-instruction to compactly mark such register kills.  For
each scratch register, a fake trivial live range is created that begins and ends
in that instruction.  The ``InstFakeKill`` instruction is inserted after the
``call`` instruction.  For example::

    CallInst = InstX8632Call::create(Func, ... );
    NewInst = InstFakeKill::create(Func, CallInst);

The last argument to the ``InstFakeKill`` constructor links it to the previous
call instruction, such that if its linked instruction is dead-code eliminated,
the ``InstFakeKill`` instruction is eliminated as well.  The linked ``call``
instruction could be to a target known to be free of side effects, and therefore
safe to remove if its result is unused.

Instructions producing multiple values
--------------------------------------

ICE instructions allow at most one destination ``Variable``.  Some machine
instructions produce more than one usable result.  For example, the x86-32
``call`` ABI returns a 64-bit integer result in the ``edx:eax`` register pair.
Also, x86-32 has a version of the ``imul`` instruction that produces a 64-bit
result in the ``edx:eax`` register pair.  The x86-32 ``idiv`` instruction
produces the quotient in ``eax`` and the remainder in ``edx``, though generally
only one or the other is needed in the lowering.

To support multi-dest instructions, ICE provides the ``InstFakeDef``
pseudo-instruction, whose destination can be precolored to the appropriate
physical register.  For example, a ``call`` returning a 64-bit result in
``edx:eax``::

    CallInst = InstX8632Call::create(Func, RegLow, ... );
    NewInst = InstFakeKill::create(Func, CallInst);
    Variable *RegHigh = Func->makeVariable(IceType_i32);
    RegHigh->setRegNum(Reg_edx);
    NewInst = InstFakeDef::create(Func, RegHigh);

``RegHigh`` is then assigned into the desired ``Variable``.  If that assignment
ends up being dead-code eliminated, the ``InstFakeDef`` instruction may be
eliminated as well.

Managing dead-code elimination
------------------------------

ICE instructions with a non-nullptr ``Dest`` are subject to dead-code
elimination.  However, some instructions must not be eliminated in order to
preserve side effects.  This applies to most function calls, volatile loads, and
loads and integer divisions where the underlying language and runtime are
relying on hardware exception handling.

ICE facilitates this with the ``InstFakeUse`` pseudo-instruction.  This forces a
use of its source ``Variable`` to keep that variable's definition alive.  Since
the ``InstFakeUse`` instruction has no ``Dest``, it will not be eliminated.

Here is the full example of the x86-32 ``call`` returning a 32-bit integer
result::

    Variable *Reg = Func->makeVariable(IceType_i32);
    Reg->setRegNum(Reg_eax);
    CallInst = InstX8632Call::create(Func, Reg, ... );
    NewInst = InstFakeKill::create(Func, CallInst);
    NewInst = InstFakeUse::create(Func, Reg);
    NewInst = InstX8632Mov::create(Func, Result, Reg);

Without the ``InstFakeUse``, the entire call sequence could be dead-code
eliminated if its result were unused.

One more note on this topic.  These tools can be used to allow a multi-dest
instruction to be dead-code eliminated only when none of its results is live.
The key is to use the optional source parameter of the ``InstFakeDef``
instruction.  Using pseudocode::

    t1:eax = call foo(arg1, ...)
    InstFakeKill  // eax, ecx, edx
    t2:edx = InstFakeDef(t1)
    v_result_low = t1
    v_result_high = t2

If ``v_result_high`` is live but ``v_result_low`` is dead, adding ``t1`` as an
argument to ``InstFakeDef`` suffices to keep the ``call`` instruction live.

Instructions modifying source operands
--------------------------------------

Some native instructions may modify one or more source operands.  For example,
the x86 ``xadd`` and ``xchg`` instructions modify both source operands.  Some
analysis needs to identify every place a ``Variable`` is modified, and it uses
the presence of a ``Dest`` variable for this analysis.  Since ICE instructions
have at most one ``Dest``, the ``xadd`` and ``xchg`` instructions need special
treatment.

A ``Variable`` that is not the ``Dest`` can be marked as modified by adding an
``InstFakeDef``.  However, this is not sufficient, as the ``Variable`` may have
no more live uses, which could result in the ``InstFakeDef`` being dead-code
eliminated.  The solution is to add an ``InstFakeUse`` as well.

To summarize, for every source ``Variable`` that is not equal to the
instruction's ``Dest``, append an ``InstFakeDef`` and ``InstFakeUse``
instruction to provide the necessary analysis information.
