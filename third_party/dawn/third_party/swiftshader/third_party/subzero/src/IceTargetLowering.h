//===- subzero/src/IceTargetLowering.h - Lowering interface -----*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares the TargetLowering, LoweringContext, and TargetDataLowering
/// classes.
///
/// TargetLowering is an abstract class used to drive the translation/lowering
/// process. LoweringContext maintains a context for lowering each instruction,
/// offering conveniences such as iterating over non-deleted instructions.
/// TargetDataLowering is an abstract class used to drive the lowering/emission
/// of global initializers, external global declarations, and internal constant
/// pools.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICETARGETLOWERING_H
#define SUBZERO_SRC_ICETARGETLOWERING_H

#include "IceBitVector.h"
#include "IceCfgNode.h"
#include "IceDefs.h"
#include "IceInst.h" // for the names of the Inst subtypes
#include "IceOperand.h"
#include "IceRegAlloc.h"
#include "IceTypes.h"

#include <utility>

namespace Ice {

// UnimplementedError is defined as a macro so that we can get actual line
// numbers.
#define UnimplementedError(Flags)                                              \
  do {                                                                         \
    if (!static_cast<const ClFlags &>(Flags).getSkipUnimplemented()) {         \
      /* Use llvm_unreachable instead of report_fatal_error, which gives       \
         better stack traces. */                                               \
      llvm_unreachable("Not yet implemented");                                 \
      abort();                                                                 \
    }                                                                          \
  } while (0)

// UnimplementedLoweringError is similar in style to UnimplementedError.  Given
// a TargetLowering object pointer and an Inst pointer, it adds appropriate
// FakeDef and FakeUse instructions to try maintain liveness consistency.
#define UnimplementedLoweringError(Target, Instr)                              \
  do {                                                                         \
    if (getFlags().getSkipUnimplemented()) {                                   \
      (Target)->addFakeDefUses(Instr);                                         \
    } else {                                                                   \
      /* Use llvm_unreachable instead of report_fatal_error, which gives       \
         better stack traces. */                                               \
      llvm_unreachable(                                                        \
          (std::string("Not yet implemented: ") + Instr->getInstName())        \
              .c_str());                                                       \
      abort();                                                                 \
    }                                                                          \
  } while (0)

/// LoweringContext makes it easy to iterate through non-deleted instructions in
/// a node, and insert new (lowered) instructions at the current point. Along
/// with the instruction list container and associated iterators, it holds the
/// current node, which is needed when inserting new instructions in order to
/// track whether variables are used as single-block or multi-block.
class LoweringContext {
  LoweringContext(const LoweringContext &) = delete;
  LoweringContext &operator=(const LoweringContext &) = delete;

public:
  LoweringContext() = default;
  ~LoweringContext() = default;
  void init(CfgNode *Node);
  Inst *getNextInst() const {
    if (Next == End)
      return nullptr;
    return iteratorToInst(Next);
  }
  Inst *getNextInst(InstList::iterator &Iter) const {
    advanceForward(Iter);
    if (Iter == End)
      return nullptr;
    return iteratorToInst(Iter);
  }
  CfgNode *getNode() const { return Node; }
  bool atEnd() const { return Cur == End; }
  InstList::iterator getCur() const { return Cur; }
  InstList::iterator getNext() const { return Next; }
  InstList::iterator getEnd() const { return End; }
  void insert(Inst *Instr);
  template <typename Inst, typename... Args> Inst *insert(Args &&...A) {
    auto *New = Inst::create(Node->getCfg(), std::forward<Args>(A)...);
    insert(New);
    return New;
  }
  Inst *getLastInserted() const;
  void advanceCur() { Cur = Next; }
  void advanceNext() { advanceForward(Next); }
  void setCur(InstList::iterator C) { Cur = C; }
  void setNext(InstList::iterator N) { Next = N; }
  void rewind();
  void setInsertPoint(const InstList::iterator &Position) { Next = Position; }
  void availabilityReset();
  void availabilityUpdate();
  Variable *availabilityGet(Operand *Src) const;

private:
  /// Node is the argument to Inst::updateVars().
  CfgNode *Node = nullptr;
  Inst *LastInserted = nullptr;
  /// Cur points to the current instruction being considered. It is guaranteed
  /// to point to a non-deleted instruction, or to be End.
  InstList::iterator Cur;
  /// Next doubles as a pointer to the next valid instruction (if any), and the
  /// new-instruction insertion point. It is also updated for the caller in case
  /// the lowering consumes more than one high-level instruction. It is
  /// guaranteed to point to a non-deleted instruction after Cur, or to be End.
  // TODO: Consider separating the notion of "next valid instruction" and "new
  // instruction insertion point", to avoid confusion when previously-deleted
  // instructions come between the two points.
  InstList::iterator Next;
  /// Begin is a copy of Insts.begin(), used if iterators are moved backward.
  InstList::iterator Begin;
  /// End is a copy of Insts.end(), used if Next needs to be advanced.
  InstList::iterator End;
  /// LastDest and LastSrc capture the parameters of the last "Dest=Src" simple
  /// assignment inserted (provided Src is a variable).  This is used for simple
  /// availability analysis.
  Variable *LastDest = nullptr;
  Variable *LastSrc = nullptr;

  void skipDeleted(InstList::iterator &I) const;
  void advanceForward(InstList::iterator &I) const;
};

/// A helper class to advance the LoweringContext at each loop iteration.
class PostIncrLoweringContext {
  PostIncrLoweringContext() = delete;
  PostIncrLoweringContext(const PostIncrLoweringContext &) = delete;
  PostIncrLoweringContext &operator=(const PostIncrLoweringContext &) = delete;

public:
  explicit PostIncrLoweringContext(LoweringContext &Context)
      : Context(Context) {}
  ~PostIncrLoweringContext() {
    Context.advanceCur();
    Context.advanceNext();
  }

private:
  LoweringContext &Context;
};

/// TargetLowering is the base class for all backends in Subzero. In addition to
/// implementing the abstract methods in this class, each concrete target must
/// also implement a named constructor in its own namespace. For instance, for
/// X8632 we have:
///
///  namespace X8632 {
///    void createTargetLowering(Cfg *Func);
///  }
class TargetLowering {
  TargetLowering() = delete;
  TargetLowering(const TargetLowering &) = delete;
  TargetLowering &operator=(const TargetLowering &) = delete;

public:
  static void staticInit(GlobalContext *Ctx);
  // Each target must define a public static method:
  //   static void staticInit(GlobalContext *Ctx);
  static bool shouldBePooled(const class Constant *C);
  static Type getPointerType();

  static std::unique_ptr<TargetLowering> createLowering(TargetArch Target,
                                                        Cfg *Func);

  virtual std::unique_ptr<Assembler> createAssembler() const = 0;

  void translate() {
    switch (Func->getOptLevel()) {
    case Opt_m1:
      translateOm1();
      break;
    case Opt_0:
      translateO0();
      break;
    case Opt_1:
      translateO1();
      break;
    case Opt_2:
      translateO2();
      break;
    }
  }
  virtual void translateOm1() {
    Func->setError("Target doesn't specify Om1 lowering steps.");
  }
  virtual void translateO0() {
    Func->setError("Target doesn't specify O0 lowering steps.");
  }
  virtual void translateO1() {
    Func->setError("Target doesn't specify O1 lowering steps.");
  }
  virtual void translateO2() {
    Func->setError("Target doesn't specify O2 lowering steps.");
  }

  /// Generates calls to intrinsics for operations the Target can't handle.
  void genTargetHelperCalls();
  /// Tries to do address mode optimization on a single instruction.
  void doAddressOpt();
  /// Lowers a single non-Phi instruction.
  void lower();
  /// Inserts and lowers a single high-level instruction at a specific insertion
  /// point.
  void lowerInst(CfgNode *Node, InstList::iterator Next, InstHighLevel *Instr);
  /// Does preliminary lowering of the set of Phi instructions in the current
  /// node. The main intention is to do what's needed to keep the unlowered Phi
  /// instructions consistent with the lowered non-Phi instructions, e.g. to
  /// lower 64-bit operands on a 32-bit target.
  virtual void prelowerPhis() {}
  /// Tries to do branch optimization on a single instruction. Returns true if
  /// some optimization was done.
  virtual bool doBranchOpt(Inst * /*I*/, const CfgNode * /*NextNode*/) {
    return false;
  }

  virtual SizeT getNumRegisters() const = 0;
  /// Returns a variable pre-colored to the specified physical register. This is
  /// generally used to get very direct access to the register such as in the
  /// prolog or epilog or for marking scratch registers as killed by a call. If
  /// a Type is not provided, a target-specific default type is used.
  virtual Variable *getPhysicalRegister(RegNumT RegNum,
                                        Type Ty = IceType_void) = 0;
  /// Returns a printable name for the register.
  virtual const char *getRegName(RegNumT RegNum, Type Ty) const = 0;

  virtual bool hasFramePointer() const { return false; }
  virtual void setHasFramePointer() = 0;
  virtual RegNumT getStackReg() const = 0;
  virtual RegNumT getFrameReg() const = 0;
  virtual RegNumT getFrameOrStackReg() const = 0;
  virtual size_t typeWidthInBytesOnStack(Type Ty) const = 0;
  virtual uint32_t getStackAlignment() const = 0;
  virtual bool needsStackPointerAlignment() const { return false; }
  virtual void reserveFixedAllocaArea(size_t Size, size_t Align) = 0;
  virtual int32_t getFrameFixedAllocaOffset() const = 0;
  virtual uint32_t maxOutArgsSizeBytes() const { return 0; }
  // Addressing relative to frame pointer differs in MIPS compared to X86/ARM
  // since MIPS decrements its stack pointer prior to saving it in the frame
  // pointer register.
  virtual uint32_t getFramePointerOffset(uint32_t CurrentOffset,
                                         uint32_t Size) const {
    return -(CurrentOffset + Size);
  }
  /// Return whether a 64-bit Variable should be split into a Variable64On32.
  virtual bool shouldSplitToVariable64On32(Type Ty) const = 0;

  /// Return whether a Vector Variable should be split into a VariableVecOn32.
  virtual bool shouldSplitToVariableVecOn32(Type Ty) const {
    (void)Ty;
    return false;
  }

  bool hasComputedFrame() const { return HasComputedFrame; }
  /// Returns true if this function calls a function that has the "returns
  /// twice" attribute.
  bool callsReturnsTwice() const { return CallsReturnsTwice; }
  void setCallsReturnsTwice(bool RetTwice) { CallsReturnsTwice = RetTwice; }
  SizeT makeNextLabelNumber() { return NextLabelNumber++; }
  SizeT makeNextJumpTableNumber() { return NextJumpTableNumber++; }
  LoweringContext &getContext() { return Context; }
  Cfg *getFunc() const { return Func; }
  GlobalContext *getGlobalContext() const { return Ctx; }

  enum RegSet {
    RegSet_None = 0,
    RegSet_CallerSave = 1 << 0,
    RegSet_CalleeSave = 1 << 1,
    RegSet_StackPointer = 1 << 2,
    RegSet_FramePointer = 1 << 3,
    RegSet_All = ~RegSet_None
  };
  using RegSetMask = uint32_t;

  virtual SmallBitVector getRegisterSet(RegSetMask Include,
                                        RegSetMask Exclude) const = 0;
  /// Get the set of physical registers available for the specified Variable's
  /// register class, applying register restrictions from the command line.
  virtual const SmallBitVector &
  getRegistersForVariable(const Variable *Var) const = 0;
  /// Get the set of *all* physical registers available for the specified
  /// Variable's register class, *not* applying register restrictions from the
  /// command line.
  virtual const SmallBitVector &
  getAllRegistersForVariable(const Variable *Var) const = 0;
  virtual const SmallBitVector &getAliasesForRegister(RegNumT) const = 0;

  void regAlloc(RegAllocKind Kind);
  void postRegallocSplitting(const SmallBitVector &RegMask);

  /// Get the minimum number of clusters required for a jump table to be
  /// considered.
  virtual SizeT getMinJumpTableSize() const = 0;
  virtual void emitJumpTable(const Cfg *Func,
                             const InstJumpTable *JumpTable) const = 0;

  virtual void emitVariable(const Variable *Var) const = 0;

  void emitWithoutPrefix(const ConstantRelocatable *CR,
                         const char *Suffix = "") const;

  virtual void emit(const ConstantInteger32 *C) const = 0;
  virtual void emit(const ConstantInteger64 *C) const = 0;
  virtual void emit(const ConstantFloat *C) const = 0;
  virtual void emit(const ConstantDouble *C) const = 0;
  virtual void emit(const ConstantUndef *C) const = 0;
  virtual void emit(const ConstantRelocatable *CR) const = 0;

  /// Performs target-specific argument lowering.
  virtual void lowerArguments() = 0;

  virtual void initNodeForLowering(CfgNode *) {}
  virtual void addProlog(CfgNode *Node) = 0;
  virtual void addEpilog(CfgNode *Node) = 0;

  /// Create a properly-typed "mov" instruction.  This is primarily for local
  /// variable splitting.
  virtual Inst *createLoweredMove(Variable *Dest, Variable *SrcVar) {
    // TODO(stichnot): make pure virtual by implementing for all targets
    (void)Dest;
    (void)SrcVar;
    llvm::report_fatal_error("createLoweredMove() unimplemented");
    return nullptr;
  }

  virtual ~TargetLowering() = default;

private:
  /// This indicates whether we are in the genTargetHelperCalls phase, and
  /// therefore can do things like scalarization.
  bool GeneratingTargetHelpers = false;

protected:
  explicit TargetLowering(Cfg *Func);
  // Applies command line filters to TypeToRegisterSet array.
  static void filterTypeToRegisterSet(
      GlobalContext *Ctx, int32_t NumRegs, SmallBitVector TypeToRegisterSet[],
      size_t TypeToRegisterSetSize,
      std::function<std::string(RegNumT)> getRegName,
      std::function<const char *(RegClass)> getRegClassName);
  virtual void lowerAlloca(const InstAlloca *Instr) = 0;
  virtual void lowerArithmetic(const InstArithmetic *Instr) = 0;
  virtual void lowerAssign(const InstAssign *Instr) = 0;
  virtual void lowerBr(const InstBr *Instr) = 0;
  virtual void lowerBreakpoint(const InstBreakpoint *Instr) = 0;
  virtual void lowerCall(const InstCall *Instr) = 0;
  virtual void lowerCast(const InstCast *Instr) = 0;
  virtual void lowerFcmp(const InstFcmp *Instr) = 0;
  virtual void lowerExtractElement(const InstExtractElement *Instr) = 0;
  virtual void lowerIcmp(const InstIcmp *Instr) = 0;
  virtual void lowerInsertElement(const InstInsertElement *Instr) = 0;
  virtual void lowerIntrinsic(const InstIntrinsic *Instr) = 0;
  virtual void lowerLoad(const InstLoad *Instr) = 0;
  virtual void lowerPhi(const InstPhi *Instr) = 0;
  virtual void lowerRet(const InstRet *Instr) = 0;
  virtual void lowerSelect(const InstSelect *Instr) = 0;
  virtual void lowerShuffleVector(const InstShuffleVector *Instr) = 0;
  virtual void lowerStore(const InstStore *Instr) = 0;
  virtual void lowerSwitch(const InstSwitch *Instr) = 0;
  virtual void lowerUnreachable(const InstUnreachable *Instr) = 0;
  virtual void lowerOther(const Inst *Instr);

  virtual void genTargetHelperCallFor(Inst *Instr) = 0;
  virtual uint32_t getCallStackArgumentsSizeBytes(const InstCall *Instr) = 0;

  /// Opportunity to modify other instructions to help Address Optimization
  virtual void doAddressOptOther() {}
  virtual void doAddressOptLoad() {}
  virtual void doAddressOptStore() {}
  virtual void doAddressOptLoadSubVector() {}
  virtual void doAddressOptStoreSubVector() {}
  virtual void doMockBoundsCheck(Operand *) {}
  /// This gives the target an opportunity to post-process the lowered expansion
  /// before returning.
  virtual void postLower() {}

  /// When the SkipUnimplemented flag is set, addFakeDefUses() gets invoked by
  /// the UnimplementedLoweringError macro to insert fake uses of all the
  /// instruction variables and a fake def of the instruction dest, in order to
  /// preserve integrity of liveness analysis.
  void addFakeDefUses(const Inst *Instr);

  /// Find (non-SSA) instructions where the Dest variable appears in some source
  /// operand, and set the IsDestRedefined flag.  This keeps liveness analysis
  /// consistent.
  void markRedefinitions();

  /// Make a pass over the Cfg to determine which variables need stack slots and
  /// place them in a sorted list (SortedSpilledVariables). Among those, vars,
  /// classify the spill variables as local to the basic block vs global
  /// (multi-block) in order to compute the parameters GlobalsSize and
  /// SpillAreaSizeBytes (represents locals or general vars if the coalescing of
  /// locals is disallowed) along with alignments required for variables in each
  /// area. We rely on accurate VMetadata in order to classify a variable as
  /// global vs local (otherwise the variable is conservatively global). The
  /// in-args should be initialized to 0.
  ///
  /// This is only a pre-pass and the actual stack slot assignment is handled
  /// separately.
  ///
  /// There may be target-specific Variable types, which will be handled by
  /// TargetVarHook. If the TargetVarHook returns true, then the variable is
  /// skipped and not considered with the rest of the spilled variables.
  void getVarStackSlotParams(VarList &SortedSpilledVariables,
                             SmallBitVector &RegsUsed, size_t *GlobalsSize,
                             size_t *SpillAreaSizeBytes,
                             uint32_t *SpillAreaAlignmentBytes,
                             uint32_t *LocalsSlotsAlignmentBytes,
                             std::function<bool(Variable *)> TargetVarHook);

  /// Calculate the amount of padding needed to align the local and global areas
  /// to the required alignment. This assumes the globals/locals layout used by
  /// getVarStackSlotParams and assignVarStackSlots.
  void alignStackSpillAreas(uint32_t SpillAreaStartOffset,
                            uint32_t SpillAreaAlignmentBytes,
                            size_t GlobalsSize,
                            uint32_t LocalsSlotsAlignmentBytes,
                            uint32_t *SpillAreaPaddingBytes,
                            uint32_t *LocalsSlotsPaddingBytes);

  /// Make a pass through the SortedSpilledVariables and actually assign stack
  /// slots. SpillAreaPaddingBytes takes into account stack alignment padding.
  /// The SpillArea starts after that amount of padding. This matches the scheme
  /// in getVarStackSlotParams, where there may be a separate multi-block global
  /// var spill area and a local var spill area.
  void assignVarStackSlots(VarList &SortedSpilledVariables,
                           size_t SpillAreaPaddingBytes,
                           size_t SpillAreaSizeBytes,
                           size_t GlobalsAndSubsequentPaddingSize,
                           bool UsesFramePointer);

  /// Sort the variables in Source based on required alignment. The variables
  /// with the largest alignment need are placed in the front of the Dest list.
  void sortVarsByAlignment(VarList &Dest, const VarList &Source) const;

  InstCall *makeHelperCall(RuntimeHelper FuncID, Variable *Dest, SizeT MaxSrcs);

  void _set_dest_redefined() { Context.getLastInserted()->setDestRedefined(); }

  bool shouldOptimizeMemIntrins();

  void scalarizeArithmetic(InstArithmetic::OpKind K, Variable *Dest,
                           Operand *Src0, Operand *Src1);

  /// Generalizes scalarizeArithmetic to support other instruction types.
  ///
  /// insertScalarInstruction is a function-like object with signature
  /// (Variable *Dest, Variable *Src0, Variable *Src1) -> Instr *.
  template <typename... Operands,
            typename F = std::function<Inst *(Variable *, Operands *...)>>
  void scalarizeInstruction(Variable *Dest, F insertScalarInstruction,
                            Operands *...Srcs) {
    assert(GeneratingTargetHelpers &&
           "scalarizeInstruction called during incorrect phase");
    const Type DestTy = Dest->getType();
    assert(isVectorType(DestTy));
    const Type DestElementTy = typeElementType(DestTy);
    const SizeT NumElements = typeNumElements(DestTy);

    Variable *T = Func->makeVariable(DestTy);
    if (auto *VarVecOn32 = llvm::dyn_cast<VariableVecOn32>(T)) {
      VarVecOn32->initVecElement(Func);
      auto *Undef = ConstantUndef::create(Ctx, DestTy);
      Context.insert<InstAssign>(T, Undef);
    } else {
      Context.insert<InstFakeDef>(T);
    }

    for (SizeT I = 0; I < NumElements; ++I) {
      auto *Index = Ctx->getConstantInt32(I);

      auto makeExtractThunk = [this, Index, NumElements](Operand *Src) {
        return [this, Index, NumElements, Src]() {
          (void)NumElements;
          assert(typeNumElements(Src->getType()) == NumElements);

          const auto ElementTy = typeElementType(Src->getType());
          auto *Op = Func->makeVariable(ElementTy);
          Context.insert<InstExtractElement>(Op, Src, Index);
          return Op;
        };
      };

      // Perform the operation as a scalar operation.
      auto *Res = Func->makeVariable(DestElementTy);
      auto *Arith = applyToThunkedArgs(insertScalarInstruction, Res,
                                       makeExtractThunk(Srcs)...);
      genTargetHelperCallFor(Arith);

      Variable *DestT = Func->makeVariable(DestTy);
      Context.insert<InstInsertElement>(DestT, T, Res, Index);
      T = DestT;
    }
    Context.insert<InstAssign>(Dest, T);
  }

  // applyToThunkedArgs is used by scalarizeInstruction. Ideally, we would just
  // call insertScalarInstruction(Res, Srcs...), but C++ does not specify
  // evaluation order which means this leads to an unpredictable final
  // output. Instead, we wrap each of the Srcs in a thunk and these
  // applyToThunkedArgs functions apply the thunks in a well defined order so we
  // still get well-defined output.
  Inst *applyToThunkedArgs(
      std::function<Inst *(Variable *, Variable *)> insertScalarInstruction,
      Variable *Res, std::function<Variable *()> thunk0) {
    auto *Src0 = thunk0();
    return insertScalarInstruction(Res, Src0);
  }

  Inst *
  applyToThunkedArgs(std::function<Inst *(Variable *, Variable *, Variable *)>
                         insertScalarInstruction,
                     Variable *Res, std::function<Variable *()> thunk0,
                     std::function<Variable *()> thunk1) {
    auto *Src0 = thunk0();
    auto *Src1 = thunk1();
    return insertScalarInstruction(Res, Src0, Src1);
  }

  Inst *applyToThunkedArgs(
      std::function<Inst *(Variable *, Variable *, Variable *, Variable *)>
          insertScalarInstruction,
      Variable *Res, std::function<Variable *()> thunk0,
      std::function<Variable *()> thunk1, std::function<Variable *()> thunk2) {
    auto *Src0 = thunk0();
    auto *Src1 = thunk1();
    auto *Src2 = thunk2();
    return insertScalarInstruction(Res, Src0, Src1, Src2);
  }

  Cfg *Func;
  GlobalContext *Ctx;
  bool HasComputedFrame = false;
  bool CallsReturnsTwice = false;
  SizeT NextLabelNumber = 0;
  SizeT NextJumpTableNumber = 0;
  LoweringContext Context;
};

/// TargetDataLowering is used for "lowering" data including initializers for
/// global variables, and the internal constant pools. It is separated out from
/// TargetLowering because it does not require a Cfg.
class TargetDataLowering {
  TargetDataLowering() = delete;
  TargetDataLowering(const TargetDataLowering &) = delete;
  TargetDataLowering &operator=(const TargetDataLowering &) = delete;

public:
  static std::unique_ptr<TargetDataLowering> createLowering(GlobalContext *Ctx);
  virtual ~TargetDataLowering();

  virtual void lowerGlobals(const VariableDeclarationList &Vars,
                            const std::string &SectionSuffix) = 0;
  virtual void lowerConstants() = 0;
  virtual void lowerJumpTables() = 0;
  virtual void emitTargetRODataSections() {}

protected:
  void emitGlobal(const VariableDeclaration &Var,
                  const std::string &SectionSuffix);

  /// For now, we assume .long is the right directive for emitting 4 byte emit
  /// global relocations. However, LLVM MIPS usually uses .4byte instead.
  /// Perhaps there is some difference when the location is unaligned.
  static const char *getEmit32Directive() { return ".long"; }

  explicit TargetDataLowering(GlobalContext *Ctx) : Ctx(Ctx) {}
  GlobalContext *Ctx;
};

/// TargetHeaderLowering is used to "lower" the header of an output file. It
/// writes out the target-specific header attributes. E.g., for ARM this writes
/// out the build attributes (float ABI, etc.).
class TargetHeaderLowering {
  TargetHeaderLowering() = delete;
  TargetHeaderLowering(const TargetHeaderLowering &) = delete;
  TargetHeaderLowering &operator=(const TargetHeaderLowering &) = delete;

public:
  static std::unique_ptr<TargetHeaderLowering>
  createLowering(GlobalContext *Ctx);
  virtual ~TargetHeaderLowering();

  virtual void lower() {}

protected:
  explicit TargetHeaderLowering(GlobalContext *Ctx) : Ctx(Ctx) {}
  GlobalContext *Ctx;
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICETARGETLOWERING_H
