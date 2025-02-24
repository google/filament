Missing support
===============

* The PNaCl LLVM backend expands shufflevector operations into sequences of
  insertelement and extractelement operations. For instance:

    define <4 x i32> @shuffle(<4 x i32> %arg1, <4 x i32> %arg2) {
    entry:
      %res = shufflevector <4 x i32> %arg1,
                           <4 x i32> %arg2,
                           <4 x i32> <i32 4, i32 5, i32 0, i32 1>
      ret <4 x i32> %res
    }

  gets expanded into:

    define <4 x i32> @shuffle(<4 x i32> %arg1, <4 x i32> %arg2) {
    entry:
      %0 = extractelement <4 x i32> %arg2, i32 0
      %1 = insertelement <4 x i32> undef, i32 %0, i32 0
      %2 = extractelement <4 x i32> %arg2, i32 1
      %3 = insertelement <4 x i32> %1, i32 %2, i32 1
      %4 = extractelement <4 x i32> %arg1, i32 0
      %5 = insertelement <4 x i32> %3, i32 %4, i32 2
      %6 = extractelement <4 x i32> %arg1, i32 1
      %7 = insertelement <4 x i32> %5, i32 %6, i32 3
      ret <4 x i32> %7
    }

  Subzero should recognize these sequences and recombine them into
  shuffle operations where appropriate.

* Add support for vector constants in the backend. The current code
  materializes the vector constants it needs (eg. for performing icmp on
  unsigned operands) using register operations, but this should be changed to
  loading them from a constant pool if the register initialization is too
  complicated (such as in TargetX8632::makeVectorOfHighOrderBits()).

* [x86 specific] llvm-mc does not allow lea to take a mem128 memory operand
  when assembling x86-32 code. The current InstX8632Lea::emit() code uses
  Variable::asType() to convert any mem128 Variables into a compatible memory
  operand type. However, the emit code does not do any conversions of
  OperandX8632Mem, so if an OperandX8632Mem is passed to lea as mem128 the
  resulting code will not assemble.  One way to fix this is by implementing
  OperandX8632Mem::asType().

* [x86 specific] Lower shl with <4 x i32> using some clever float conversion:
http://lists.cs.uiuc.edu/pipermail/llvm-commits/Week-of-Mon-20100726/105087.html

* [x86 specific] Add support for using aligned mov operations (movaps). This
  will require passing alignment information to loads and stores.

x86 SIMD Diversification
========================

* Vector "bitwise" operations have several variant instructions: the AND
  operation can be implemented with pand, andpd, or andps. This pattern also
  holds for ANDN, OR, and XOR.

* Vector "mov" instructions can be diversified (eg. movdqu instead of movups)
  at the cost of a possible performance penalty.

* Scalar FP arithmetic can be diversified by performing the operations with the
  vector version of the instructions.
