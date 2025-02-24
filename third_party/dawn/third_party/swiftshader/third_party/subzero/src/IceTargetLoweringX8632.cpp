//===- subzero/src/IceTargetLoweringX8632.cpp - x86-32 lowering -----------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the TargetLoweringX8632 class, which consists almost
/// entirely of the lowering sequence for each high-level instruction.
///
//===----------------------------------------------------------------------===//

#include "IceTargetLoweringX8632.h"

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceClFlags.h"
#include "IceDefs.h"
#include "IceELFObjectWriter.h"
#include "IceGlobalInits.h"
#include "IceInstVarIter.h"
#include "IceInstX8632.h"
#include "IceLiveness.h"
#include "IceOperand.h"
#include "IcePhiLoweringImpl.h"
#include "IceTargetLoweringX8632.def"
#include "IceUtils.h"
#include "IceVariableSplitting.h"

#include "llvm/Support/MathExtras.h"

#include <stack>

#if defined(_WIN32)
extern "C" void _chkstk();
#endif

namespace X8632 {

std::unique_ptr<::Ice::TargetLowering> createTargetLowering(::Ice::Cfg *Func) {
  return ::Ice::X8632::TargetX8632::create(Func);
}

std::unique_ptr<::Ice::TargetDataLowering>
createTargetDataLowering(::Ice::GlobalContext *Ctx) {
  return ::Ice::X8632::TargetDataX8632::create(Ctx);
}

std::unique_ptr<::Ice::TargetHeaderLowering>
createTargetHeaderLowering(::Ice::GlobalContext *Ctx) {
  return ::Ice::X8632::TargetHeaderX86::create(Ctx);
}

void staticInit(::Ice::GlobalContext *Ctx) {
  ::Ice::X8632::TargetX8632::staticInit(Ctx);
}

bool shouldBePooled(const class ::Ice::Constant *C) {
  return ::Ice::X8632::TargetX8632::shouldBePooled(C);
}

::Ice::Type getPointerType() { return ::Ice::Type::IceType_i32; }

} // end of namespace X8632

namespace Ice {
namespace X8632 {

/// The number of bits in a byte
static constexpr uint32_t X86_CHAR_BIT = 8;
/// Size of the return address on the stack
static constexpr uint32_t X86_RET_IP_SIZE_BYTES = 4;

/// \name Limits for unrolling memory intrinsics.
/// @{
static constexpr uint32_t MEMCPY_UNROLL_LIMIT = 8;
static constexpr uint32_t MEMMOVE_UNROLL_LIMIT = 8;
static constexpr uint32_t MEMSET_UNROLL_LIMIT = 8;
/// @}

BoolFoldingEntry::BoolFoldingEntry(Inst *I)
    : Instr(I), IsComplex(BoolFolding::hasComplexLowering(I)) {}

BoolFolding::BoolFoldingProducerKind
BoolFolding::getProducerKind(const Inst *Instr) {
  if (llvm::isa<InstIcmp>(Instr)) {
    if (Instr->getSrc(0)->getType() != IceType_i64)
      return PK_Icmp32;
    return PK_Icmp64;
  }
  if (llvm::isa<InstFcmp>(Instr))
    return PK_Fcmp;
  if (auto *Arith = llvm::dyn_cast<InstArithmetic>(Instr)) {
    if (Arith->getSrc(0)->getType() != IceType_i64) {
      switch (Arith->getOp()) {
      default:
        return PK_None;
      case InstArithmetic::And:
      case InstArithmetic::Or:
        return PK_Arith;
      }
    }
  }
  return PK_None; // TODO(stichnot): remove this

  if (auto *Cast = llvm::dyn_cast<InstCast>(Instr)) {
    switch (Cast->getCastKind()) {
    default:
      return PK_None;
    case InstCast::Trunc:
      return PK_Trunc;
    }
  }
  return PK_None;
}

BoolFolding::BoolFoldingConsumerKind
BoolFolding::getConsumerKind(const Inst *Instr) {
  if (llvm::isa<InstBr>(Instr))
    return CK_Br;
  if (llvm::isa<InstSelect>(Instr))
    return CK_Select;
  return CK_None; // TODO(stichnot): remove this

  if (auto *Cast = llvm::dyn_cast<InstCast>(Instr)) {
    switch (Cast->getCastKind()) {
    default:
      return CK_None;
    case InstCast::Sext:
      return CK_Sext;
    case InstCast::Zext:
      return CK_Zext;
    }
  }
  return CK_None;
}

/// Returns true if the producing instruction has a "complex" lowering sequence.
/// This generally means that its lowering sequence requires more than one
/// conditional branch, namely 64-bit integer compares and some floating-point
/// compares. When this is true, and there is more than one consumer, we prefer
/// to disable the folding optimization because it minimizes branches.

bool BoolFolding::hasComplexLowering(const Inst *Instr) {
  switch (getProducerKind(Instr)) {
  default:
    return false;
  case PK_Icmp64:
    return true;
  case PK_Fcmp:
    return TargetX8632::TableFcmp[llvm::cast<InstFcmp>(Instr)->getCondition()]
               .C2 != CondX86::Br_None;
  }
}

bool BoolFolding::isValidFolding(
    BoolFolding::BoolFoldingProducerKind ProducerKind,
    BoolFolding::BoolFoldingConsumerKind ConsumerKind) {
  switch (ProducerKind) {
  default:
    return false;
  case PK_Icmp32:
  case PK_Icmp64:
  case PK_Fcmp:
    return (ConsumerKind == CK_Br) || (ConsumerKind == CK_Select);
  case PK_Arith:
    return ConsumerKind == CK_Br;
  }
}

void BoolFolding::init(CfgNode *Node) {
  Producers.clear();
  for (Inst &Instr : Node->getInsts()) {
    if (Instr.isDeleted())
      continue;
    invalidateProducersOnStore(&Instr);
    // Check whether Instr is a valid producer.
    Variable *Var = Instr.getDest();
    if (Var) { // only consider instructions with an actual dest var
      if (isBooleanType(Var->getType())) {        // only bool-type dest vars
        if (getProducerKind(&Instr) != PK_None) { // white-listed instructions
          Producers[Var->getIndex()] = BoolFoldingEntry(&Instr);
        }
      }
    }
    // Check each src variable against the map.
    FOREACH_VAR_IN_INST(Var, Instr) {
      SizeT VarNum = Var->getIndex();
      if (!containsValid(VarNum))
        continue;
      // All valid consumers use Var as the first source operand
      if (IndexOfVarOperandInInst(Var) != 0) {
        setInvalid(VarNum);
        continue;
      }
      // Consumer instructions must be white-listed
      BoolFolding::BoolFoldingConsumerKind ConsumerKind =
          getConsumerKind(&Instr);
      if (ConsumerKind == CK_None) {
        setInvalid(VarNum);
        continue;
      }
      BoolFolding::BoolFoldingProducerKind ProducerKind =
          getProducerKind(Producers[VarNum].Instr);
      if (!isValidFolding(ProducerKind, ConsumerKind)) {
        setInvalid(VarNum);
        continue;
      }
      // Avoid creating multiple copies of complex producer instructions.
      if (Producers[VarNum].IsComplex && Producers[VarNum].NumUses > 0) {
        setInvalid(VarNum);
        continue;
      }
      ++Producers[VarNum].NumUses;
      if (Instr.isLastUse(Var)) {
        Producers[VarNum].IsLiveOut = false;
      }
    }
  }
  for (auto &I : Producers) {
    // Ignore entries previously marked invalid.
    if (I.second.Instr == nullptr)
      continue;
    // Disable the producer if its dest may be live beyond this block.
    if (I.second.IsLiveOut) {
      setInvalid(I.first);
      continue;
    }
    // Mark as "dead" rather than outright deleting. This is so that other
    // peephole style optimizations during or before lowering have access to
    // this instruction in undeleted form. See for example
    // tryOptimizedCmpxchgCmpBr().
    I.second.Instr->setDead();
  }
}

const Inst *BoolFolding::getProducerFor(const Operand *Opnd) const {
  auto *Var = llvm::dyn_cast<const Variable>(Opnd);
  if (Var == nullptr)
    return nullptr;
  SizeT VarNum = Var->getIndex();
  auto Element = Producers.find(VarNum);
  if (Element == Producers.end())
    return nullptr;
  return Element->second.Instr;
}

void BoolFolding::dump(const Cfg *Func) const {
  if (!BuildDefs::dump() || !Func->isVerbose(IceV_Folding))
    return;
  OstreamLocker L(Func->getContext());
  Ostream &Str = Func->getContext()->getStrDump();
  for (auto &I : Producers) {
    if (I.second.Instr == nullptr)
      continue;
    Str << "Found foldable producer:\n  ";
    I.second.Instr->dump(Func);
    Str << "\n";
  }
}

/// If the given instruction has potential memory side effects (e.g. store, rmw,
/// or a call instruction with potential memory side effects), then we must not
/// allow a pre-store Producer instruction with memory operands to be folded
/// into a post-store Consumer instruction.  If this is detected, the Producer
/// is invalidated.
///
/// We use the Producer's IsLiveOut field to determine whether any potential
/// Consumers come after this store instruction.  The IsLiveOut field is
/// initialized to true, and BoolFolding::init() sets IsLiveOut to false when it
/// sees the variable's definitive last use (indicating the variable is not in
/// the node's live-out set).  Thus if we see here that IsLiveOut is false, we
/// know that there can be no consumers after the store, and therefore we know
/// the folding is safe despite the store instruction.

void BoolFolding::invalidateProducersOnStore(const Inst *Instr) {
  if (!Instr->isMemoryWrite())
    return;
  for (auto &ProducerPair : Producers) {
    if (!ProducerPair.second.IsLiveOut)
      continue;
    Inst *PInst = ProducerPair.second.Instr;
    if (PInst == nullptr)
      continue;
    bool HasMemOperand = false;
    const SizeT SrcSize = PInst->getSrcSize();
    for (SizeT I = 0; I < SrcSize; ++I) {
      if (llvm::isa<X86OperandMem>(PInst->getSrc(I))) {
        HasMemOperand = true;
        break;
      }
    }
    if (!HasMemOperand)
      continue;
    setInvalid(ProducerPair.first);
  }
}

void TargetX8632::initNodeForLowering(CfgNode *Node) {
  FoldingInfo.init(Node);
  FoldingInfo.dump(Func);
}

TargetX8632::TargetX8632(Cfg *Func) : TargetX86(Func) {}

void TargetX8632::staticInit(GlobalContext *Ctx) {
  RegNumT::setLimit(RegX8632::Reg_NUM);
  RegX8632::initRegisterSet(getFlags(), &TypeToRegisterSet, &RegisterAliases);
  for (size_t i = 0; i < TypeToRegisterSet.size(); ++i)
    TypeToRegisterSetUnfiltered[i] = TypeToRegisterSet[i];
  filterTypeToRegisterSet(Ctx, RegX8632::Reg_NUM, TypeToRegisterSet.data(),
                          TypeToRegisterSet.size(), RegX8632::getRegName,
                          getRegClassName);
}

bool TargetX8632::shouldBePooled(const Constant *C) {
  if (auto *ConstFloat = llvm::dyn_cast<ConstantFloat>(C)) {
    return !Utils::isPositiveZero(ConstFloat->getValue());
  }
  if (auto *ConstDouble = llvm::dyn_cast<ConstantDouble>(C)) {
    return !Utils::isPositiveZero(ConstDouble->getValue());
  }
  return false;
}

Type TargetX8632::getPointerType() { return IceType_i32; }

void TargetX8632::translateO2() {
  TimerMarker T(TimerStack::TT_O2, Func);

  genTargetHelperCalls();
  Func->dump("After target helper call insertion");

  // Merge Alloca instructions, and lay out the stack.
  static constexpr bool SortAndCombineAllocas = true;
  Func->processAllocas(SortAndCombineAllocas);
  Func->dump("After Alloca processing");

  // Run this early so it can be used to focus optimizations on potentially hot
  // code.
  // TODO(stichnot,ascull): currently only used for regalloc not
  // expensive high level optimizations which could be focused on potentially
  // hot code.
  Func->generateLoopInfo();
  Func->dump("After loop analysis");
  if (getFlags().getLoopInvariantCodeMotion()) {
    Func->loopInvariantCodeMotion();
    Func->dump("After LICM");
  }

  if (getFlags().getLocalCSE() != Ice::LCSE_Disabled) {
    Func->localCSE(getFlags().getLocalCSE() == Ice::LCSE_EnabledSSA);
    Func->dump("After Local CSE");
    Func->floatConstantCSE();
  }
  if (getFlags().getEnableShortCircuit()) {
    Func->shortCircuitJumps();
    Func->dump("After Short Circuiting");
  }

  if (!getFlags().getEnablePhiEdgeSplit()) {
    // Lower Phi instructions.
    Func->placePhiLoads();
    if (Func->hasError())
      return;
    Func->placePhiStores();
    if (Func->hasError())
      return;
    Func->deletePhis();
    if (Func->hasError())
      return;
    Func->dump("After Phi lowering");
  }

  // Address mode optimization.
  Func->getVMetadata()->init(VMK_SingleDefs);
  Func->doAddressOpt();
  Func->materializeVectorShuffles();

  // Find read-modify-write opportunities. Do this after address mode
  // optimization so that doAddressOpt() doesn't need to be applied to RMW
  // instructions as well.
  findRMW();
  Func->dump("After RMW transform");

  // Argument lowering
  Func->doArgLowering();

  // Target lowering. This requires liveness analysis for some parts of the
  // lowering decisions, such as compare/branch fusing. If non-lightweight
  // liveness analysis is used, the instructions need to be renumbered first
  // TODO: This renumbering should only be necessary if we're actually
  // calculating live intervals, which we only do for register allocation.
  Func->renumberInstructions();
  if (Func->hasError())
    return;

  // TODO: It should be sufficient to use the fastest liveness calculation,
  // i.e. livenessLightweight(). However, for some reason that slows down the
  // rest of the translation. Investigate.
  Func->liveness(Liveness_Basic);
  if (Func->hasError())
    return;
  Func->dump("After x86 address mode opt");

  doLoadOpt();

  Func->genCode();
  if (Func->hasError())
    return;
  Func->dump("After x86 codegen");
  splitBlockLocalVariables(Func);

  // Register allocation. This requires instruction renumbering and full
  // liveness analysis. Loops must be identified before liveness so variable
  // use weights are correct.
  Func->renumberInstructions();
  if (Func->hasError())
    return;
  Func->liveness(Liveness_Intervals);
  if (Func->hasError())
    return;
  // The post-codegen dump is done here, after liveness analysis and associated
  // cleanup, to make the dump cleaner and more useful.
  Func->dump("After initial x86 codegen");
  // Validate the live range computations. The expensive validation call is
  // deliberately only made when assertions are enabled.
  assert(Func->validateLiveness());
  Func->getVMetadata()->init(VMK_All);
  regAlloc(RAK_Global);
  if (Func->hasError())
    return;
  Func->dump("After linear scan regalloc");

  if (getFlags().getEnablePhiEdgeSplit()) {
    Func->advancedPhiLowering();
    Func->dump("After advanced Phi lowering");
  }

  // Stack frame mapping.
  Func->genFrame();
  if (Func->hasError())
    return;
  Func->dump("After stack frame mapping");

  Func->contractEmptyNodes();
  Func->reorderNodes();

  // Branch optimization.  This needs to be done just before code emission. In
  // particular, no transformations that insert or reorder CfgNodes should be
  // done after branch optimization. We go ahead and do it before nop insertion
  // to reduce the amount of work needed for searching for opportunities.
  Func->doBranchOpt();
  Func->dump("After branch optimization");
}

void TargetX8632::translateOm1() {
  TimerMarker T(TimerStack::TT_Om1, Func);

  genTargetHelperCalls();

  // Do not merge Alloca instructions, and lay out the stack.
  // static constexpr bool SortAndCombineAllocas = false;
  static constexpr bool SortAndCombineAllocas =
      true; // TODO(b/171222930): Fix Win32 bug when this is false
  Func->processAllocas(SortAndCombineAllocas);
  Func->dump("After Alloca processing");

  Func->placePhiLoads();
  if (Func->hasError())
    return;
  Func->placePhiStores();
  if (Func->hasError())
    return;
  Func->deletePhis();
  if (Func->hasError())
    return;
  Func->dump("After Phi lowering");

  Func->doArgLowering();
  Func->genCode();
  if (Func->hasError())
    return;
  Func->dump("After initial x86 codegen");

  regAlloc(RAK_InfOnly);
  if (Func->hasError())
    return;
  Func->dump("After regalloc of infinite-weight variables");

  Func->genFrame();
  if (Func->hasError())
    return;
  Func->dump("After stack frame mapping");
}

inline bool canRMW(const InstArithmetic *Arith) {
  Type Ty = Arith->getDest()->getType();
  // X86 vector instructions write to a register and have no RMW option.
  if (isVectorType(Ty))
    return false;
  bool isI64 = Ty == IceType_i64;

  switch (Arith->getOp()) {
  // Not handled for lack of simple lowering:
  //   shift on i64
  //   mul, udiv, urem, sdiv, srem, frem
  // Not handled for lack of RMW instructions:
  //   fadd, fsub, fmul, fdiv (also vector types)
  default:
    return false;
  case InstArithmetic::Add:
  case InstArithmetic::Sub:
  case InstArithmetic::And:
  case InstArithmetic::Or:
  case InstArithmetic::Xor:
    return true;
  case InstArithmetic::Shl:
  case InstArithmetic::Lshr:
  case InstArithmetic::Ashr:
    return false; // TODO(stichnot): implement
    return !isI64;
  }
}

bool isSameMemAddressOperand(const Operand *A, const Operand *B) {
  if (A == B)
    return true;
  if (auto *MemA = llvm::dyn_cast<X86OperandMem>(A)) {
    if (auto *MemB = llvm::dyn_cast<X86OperandMem>(B)) {
      return MemA->getBase() == MemB->getBase() &&
             MemA->getOffset() == MemB->getOffset() &&
             MemA->getIndex() == MemB->getIndex() &&
             MemA->getShift() == MemB->getShift() &&
             MemA->getSegmentRegister() == MemB->getSegmentRegister();
    }
  }
  return false;
}

void TargetX8632::findRMW() {
  TimerMarker _(TimerStack::TT_findRMW, Func);
  Func->dump("Before RMW");
  if (Func->isVerbose(IceV_RMW))
    Func->getContext()->lockStr();
  for (CfgNode *Node : Func->getNodes()) {
    // Walk through the instructions, considering each sequence of 3
    // instructions, and look for the particular RMW pattern. Note that this
    // search can be "broken" (false negatives) if there are intervening
    // deleted instructions, or intervening instructions that could be safely
    // moved out of the way to reveal an RMW pattern.
    auto E = Node->getInsts().end();
    auto I1 = E, I2 = E, I3 = Node->getInsts().begin();
    for (; I3 != E; I1 = I2, I2 = I3, ++I3) {
      // Make I3 skip over deleted instructions.
      while (I3 != E && I3->isDeleted())
        ++I3;
      if (I1 == E || I2 == E || I3 == E)
        continue;
      assert(!I1->isDeleted());
      assert(!I2->isDeleted());
      assert(!I3->isDeleted());
      auto *Load = llvm::dyn_cast<InstLoad>(I1);
      auto *Arith = llvm::dyn_cast<InstArithmetic>(I2);
      auto *Store = llvm::dyn_cast<InstStore>(I3);
      if (!Load || !Arith || !Store)
        continue;
      // Look for:
      //   a = Load addr
      //   b = <op> a, other
      //   Store b, addr
      // Change to:
      //   a = Load addr
      //   b = <op> a, other
      //   x = FakeDef
      //   RMW <op>, addr, other, x
      //   b = Store b, addr, x
      // Note that inferTwoAddress() makes sure setDestRedefined() gets called
      // on the updated Store instruction, to avoid liveness problems later.
      //
      // With this transformation, the Store instruction acquires a Dest
      // variable and is now subject to dead code elimination if there are no
      // more uses of "b".  Variable "x" is a beacon for determining whether the
      // Store instruction gets dead-code eliminated.  If the Store instruction
      // is eliminated, then it must be the case that the RMW instruction ends
      // x's live range, and therefore the RMW instruction will be retained and
      // later lowered.  On the other hand, if the RMW instruction does not end
      // x's live range, then the Store instruction must still be present, and
      // therefore the RMW instruction is ignored during lowering because it is
      // redundant with the Store instruction.
      //
      // Note that if "a" has further uses, the RMW transformation may still
      // trigger, resulting in two loads and one store, which is worse than the
      // original one load and one store.  However, this is probably rare, and
      // caching probably keeps it just as fast.
      if (!isSameMemAddressOperand(Load->getLoadAddress(),
                                   Store->getStoreAddress()))
        continue;
      Operand *ArithSrcFromLoad = Arith->getSrc(0);
      Operand *ArithSrcOther = Arith->getSrc(1);
      if (ArithSrcFromLoad != Load->getDest()) {
        if (!Arith->isCommutative() || ArithSrcOther != Load->getDest())
          continue;
        std::swap(ArithSrcFromLoad, ArithSrcOther);
      }
      if (Arith->getDest() != Store->getData())
        continue;
      if (!canRMW(Arith))
        continue;
      if (Func->isVerbose(IceV_RMW)) {
        Ostream &Str = Func->getContext()->getStrDump();
        Str << "Found RMW in " << Func->getFunctionName() << ":\n  ";
        Load->dump(Func);
        Str << "\n  ";
        Arith->dump(Func);
        Str << "\n  ";
        Store->dump(Func);
        Str << "\n";
      }
      Variable *Beacon = Func->makeVariable(IceType_i32);
      Beacon->setMustNotHaveReg();
      Store->setRmwBeacon(Beacon);
      auto *BeaconDef = InstFakeDef::create(Func, Beacon);
      Node->getInsts().insert(I3, BeaconDef);
      auto *RMW =
          InstX86FakeRMW::create(Func, ArithSrcOther, Store->getStoreAddress(),
                                 Beacon, Arith->getOp());
      Node->getInsts().insert(I3, RMW);
    }
  }
  if (Func->isVerbose(IceV_RMW))
    Func->getContext()->unlockStr();
}

/// Value is in bytes. Return Value adjusted to the next highest multiple of
/// the stack alignment.
uint32_t TargetX8632::applyStackAlignment(uint32_t Value) {
  return Utils::applyAlignment(Value, X86_STACK_ALIGNMENT_BYTES);
}

// Converts a ConstantInteger32 operand into its constant value, or
// MemoryOrderInvalid if the operand is not a ConstantInteger32.
inline uint64_t getConstantMemoryOrder(Operand *Opnd) {
  if (auto *Integer = llvm::dyn_cast<ConstantInteger32>(Opnd))
    return Integer->getValue();
  return Intrinsics::MemoryOrderInvalid;
}

/// Determines whether the dest of a Load instruction can be folded into one of
/// the src operands of a 2-operand instruction. This is true as long as the
/// load dest matches exactly one of the binary instruction's src operands.
/// Replaces Src0 or Src1 with LoadSrc if the answer is true.
inline bool canFoldLoadIntoBinaryInst(Operand *LoadSrc, Variable *LoadDest,
                                      Operand *&Src0, Operand *&Src1) {
  if (Src0 == LoadDest && Src1 != LoadDest) {
    Src0 = LoadSrc;
    return true;
  }
  if (Src0 != LoadDest && Src1 == LoadDest) {
    Src1 = LoadSrc;
    return true;
  }
  return false;
}

void TargetX8632::doLoadOpt() {
  TimerMarker _(TimerStack::TT_loadOpt, Func);
  for (CfgNode *Node : Func->getNodes()) {
    Context.init(Node);
    while (!Context.atEnd()) {
      Variable *LoadDest = nullptr;
      Operand *LoadSrc = nullptr;
      Inst *CurInst = iteratorToInst(Context.getCur());
      Inst *Next = Context.getNextInst();
      // Determine whether the current instruction is a Load instruction or
      // equivalent.
      if (auto *Load = llvm::dyn_cast<InstLoad>(CurInst)) {
        // An InstLoad qualifies unless it uses a 64-bit absolute address,
        // which requires legalization to insert a copy to register.
        // TODO(b/148272103): Fold these after legalization.
        LoadDest = Load->getDest();
        constexpr bool DoLegalize = false;
        LoadSrc = formMemoryOperand(Load->getLoadAddress(), LoadDest->getType(),
                                    DoLegalize);
      } else if (auto *Intrin = llvm::dyn_cast<InstIntrinsic>(CurInst)) {
        // An AtomicLoad intrinsic qualifies as long as it has a valid memory
        // ordering, and can be implemented in a single instruction (i.e., not
        // i64 on x86-32).
        Intrinsics::IntrinsicID ID = Intrin->getIntrinsicID();
        if (ID == Intrinsics::AtomicLoad &&
            (Intrin->getDest()->getType() != IceType_i64) &&
            Intrinsics::isMemoryOrderValid(
                ID, getConstantMemoryOrder(Intrin->getArg(1)))) {
          LoadDest = Intrin->getDest();
          constexpr bool DoLegalize = false;
          LoadSrc = formMemoryOperand(Intrin->getArg(0), LoadDest->getType(),
                                      DoLegalize);
        }
      }
      // A Load instruction can be folded into the following instruction only
      // if the following instruction ends the Load's Dest variable's live
      // range.
      if (LoadDest && Next && Next->isLastUse(LoadDest)) {
        assert(LoadSrc);
        Inst *NewInst = nullptr;
        if (auto *Arith = llvm::dyn_cast<InstArithmetic>(Next)) {
          Operand *Src0 = Arith->getSrc(0);
          Operand *Src1 = Arith->getSrc(1);
          if (canFoldLoadIntoBinaryInst(LoadSrc, LoadDest, Src0, Src1)) {
            NewInst = InstArithmetic::create(Func, Arith->getOp(),
                                             Arith->getDest(), Src0, Src1);
          }
        } else if (auto *Icmp = llvm::dyn_cast<InstIcmp>(Next)) {
          Operand *Src0 = Icmp->getSrc(0);
          Operand *Src1 = Icmp->getSrc(1);
          if (canFoldLoadIntoBinaryInst(LoadSrc, LoadDest, Src0, Src1)) {
            NewInst = InstIcmp::create(Func, Icmp->getCondition(),
                                       Icmp->getDest(), Src0, Src1);
          }
        } else if (auto *Fcmp = llvm::dyn_cast<InstFcmp>(Next)) {
          Operand *Src0 = Fcmp->getSrc(0);
          Operand *Src1 = Fcmp->getSrc(1);
          if (canFoldLoadIntoBinaryInst(LoadSrc, LoadDest, Src0, Src1)) {
            NewInst = InstFcmp::create(Func, Fcmp->getCondition(),
                                       Fcmp->getDest(), Src0, Src1);
          }
        } else if (auto *Select = llvm::dyn_cast<InstSelect>(Next)) {
          Operand *Src0 = Select->getTrueOperand();
          Operand *Src1 = Select->getFalseOperand();
          if (canFoldLoadIntoBinaryInst(LoadSrc, LoadDest, Src0, Src1)) {
            NewInst = InstSelect::create(Func, Select->getDest(),
                                         Select->getCondition(), Src0, Src1);
          }
        } else if (auto *Cast = llvm::dyn_cast<InstCast>(Next)) {
          // The load dest can always be folded into a Cast instruction.
          auto *Src0 = llvm::dyn_cast<Variable>(Cast->getSrc(0));
          if (Src0 == LoadDest) {
            NewInst = InstCast::create(Func, Cast->getCastKind(),
                                       Cast->getDest(), LoadSrc);
          }
        }
        if (NewInst) {
          CurInst->setDeleted();
          Next->setDeleted();
          Context.insert(NewInst);
          // Update NewInst->LiveRangesEnded so that target lowering may
          // benefit. Also update NewInst->HasSideEffects.
          NewInst->spliceLivenessInfo(Next, CurInst);
        }
      }
      Context.advanceCur();
      Context.advanceNext();
    }
  }
  Func->dump("After load optimization");
}

bool TargetX8632::doBranchOpt(Inst *I, const CfgNode *NextNode) {
  if (auto *Br = llvm::dyn_cast<InstX86Br>(I)) {
    return Br->optimizeBranch(NextNode);
  }
  return false;
}

Variable *TargetX8632::getPhysicalRegister(RegNumT RegNum, Type Ty) {
  if (Ty == IceType_void)
    Ty = IceType_i32;
  if (PhysicalRegisters[Ty].empty())
    PhysicalRegisters[Ty].resize(RegX8632::Reg_NUM);
  assert(unsigned(RegNum) < PhysicalRegisters[Ty].size());
  Variable *Reg = PhysicalRegisters[Ty][RegNum];
  if (Reg == nullptr) {
    Reg = Func->makeVariable(Ty);
    Reg->setRegNum(RegNum);
    PhysicalRegisters[Ty][RegNum] = Reg;
    // Specially mark a named physical register as an "argument" so that it is
    // considered live upon function entry.  Otherwise it's possible to get
    // liveness validation errors for saving callee-save registers.
    Func->addImplicitArg(Reg);
    // Don't bother tracking the live range of a named physical register.
    Reg->setIgnoreLiveness();
  }
  assert(RegX8632::getGprForType(Ty, RegNum) == RegNum);
  return Reg;
}

const char *TargetX8632::getRegName(RegNumT RegNum, Type Ty) const {
  return RegX8632::getRegName(RegX8632::getGprForType(Ty, RegNum));
}

void TargetX8632::emitVariable(const Variable *Var) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Ctx->getStrEmit();
  if (Var->hasReg()) {
    Str << "%" << getRegName(Var->getRegNum(), Var->getType());
    return;
  }
  if (Var->mustHaveReg()) {
    llvm::report_fatal_error("Infinite-weight Variable (" + Var->getName() +
                             ") has no register assigned - function " +
                             Func->getFunctionName());
  }
  const int32_t Offset = Var->getStackOffset();
  auto BaseRegNum = Var->getBaseRegNum();
  if (BaseRegNum.hasNoValue())
    BaseRegNum = getFrameOrStackReg();

  // Print in the form "Offset(%reg)", omitting Offset when it is 0.
  if (getFlags().getDecorateAsm()) {
    Str << Var->getSymbolicStackOffset();
  } else if (Offset != 0) {
    Str << Offset;
  }
  const Type FrameSPTy = WordType;
  Str << "(%" << getRegName(BaseRegNum, FrameSPTy) << ")";
}

void TargetX8632::addProlog(CfgNode *Node) {
  // Stack frame layout:
  //
  // +------------------------+  ^ +
  // | 1. return address      |  |
  // +------------------------+  v -
  // | 2. preserved registers |
  // +------------------------+ <--- BasePointer (if used)
  // | 3. padding             |
  // +------------------------+
  // | 4. global spill area   |
  // +------------------------+
  // | 5. padding             |
  // +------------------------+
  // | 6. local spill area    |
  // +------------------------+
  // | 7. padding             |
  // +------------------------+
  // | 7.5 shadow (WinX64)    |
  // +------------------------+
  // | 8. allocas             |
  // +------------------------+
  // | 9. padding             |
  // +------------------------+
  // | 10. out args           |
  // +------------------------+ <--- StackPointer
  //
  // The following variables record the size in bytes of the given areas:
  //  * X86_RET_IP_SIZE_BYTES:   area 1
  //  * PreservedRegsSizeBytes:  area 2
  //  * SpillAreaPaddingBytes:   area 3
  //  * GlobalsSize:             area 4
  //  * LocalsSlotsPaddingBytes: area 5
  //  * GlobalsAndSubsequentPaddingSize: areas 4 - 5
  //  * LocalsSpillAreaSize:     area 6
  //  * FixedAllocaSizeBytes:    areas 7 - 8
  //  * SpillAreaSizeBytes:      areas 3 - 10
  //  * maxOutArgsSizeBytes():   areas 9 - 10

  // Determine stack frame offsets for each Variable without a register
  // assignment. This can be done as one variable per stack slot. Or, do
  // coalescing by running the register allocator again with an infinite set of
  // registers (as a side effect, this gives variables a second chance at
  // physical register assignment).
  //
  // A middle ground approach is to leverage sparsity and allocate one block of
  // space on the frame for globals (variables with multi-block lifetime), and
  // one block to share for locals (single-block lifetime).

  // StackPointer: points just past return address of calling function

  Context.init(Node);
  Context.setInsertPoint(Context.getCur());

  SmallBitVector CalleeSaves = getRegisterSet(RegSet_CalleeSave, RegSet_None);
  RegsUsed = SmallBitVector(CalleeSaves.size());
  VarList SortedSpilledVariables, VariablesLinkedToSpillSlots;
  size_t GlobalsSize = 0;
  // If there is a separate locals area, this represents that area. Otherwise
  // it counts any variable not counted by GlobalsSize.
  SpillAreaSizeBytes = 0;
  // If there is a separate locals area, this specifies the alignment for it.
  uint32_t LocalsSlotsAlignmentBytes = 0;
  // The entire spill locations area gets aligned to largest natural alignment
  // of the variables that have a spill slot.
  uint32_t SpillAreaAlignmentBytes = 0;
  // A spill slot linked to a variable with a stack slot should reuse that
  // stack slot.
  std::function<bool(Variable *)> TargetVarHook =
      [&VariablesLinkedToSpillSlots](Variable *Var) {
        // TODO(stichnot): Refactor this into the base class.
        Variable *Root = Var->getLinkedToStackRoot();
        if (Root != nullptr) {
          assert(!Root->hasReg());
          if (!Root->hasReg()) {
            VariablesLinkedToSpillSlots.push_back(Var);
            return true;
          }
        }
        return false;
      };

  // Compute the list of spilled variables and bounds for GlobalsSize, etc.
  getVarStackSlotParams(SortedSpilledVariables, RegsUsed, &GlobalsSize,
                        &SpillAreaSizeBytes, &SpillAreaAlignmentBytes,
                        &LocalsSlotsAlignmentBytes, TargetVarHook);
  uint32_t LocalsSpillAreaSize = SpillAreaSizeBytes;
  SpillAreaSizeBytes += GlobalsSize;

  // Add push instructions for preserved registers.
  uint32_t NumCallee = 0;
  size_t PreservedRegsSizeBytes = 0;
  SmallBitVector Pushed(CalleeSaves.size());
  for (RegNumT i : RegNumBVIter(CalleeSaves)) {
    const auto Canonical = RegX8632::getBaseReg(i);
    assert(Canonical == RegX8632::getBaseReg(Canonical));
    if (RegsUsed[i]) {
      Pushed[Canonical] = true;
    }
  }
  for (RegNumT RegNum : RegNumBVIter(Pushed)) {
    assert(RegNum == RegX8632::getBaseReg(RegNum));
    ++NumCallee;
    if (RegX8632::isXmm(RegNum)) {
      PreservedRegsSizeBytes += 16;
    } else {
      PreservedRegsSizeBytes += typeWidthInBytes(WordType);
    }
    _push_reg(RegNum);
  }
  Ctx->statsUpdateRegistersSaved(NumCallee);

  // StackPointer: points past preserved registers at start of spill area

  // Generate "push frameptr; mov frameptr, stackptr"
  if (IsEbpBasedFrame) {
    assert(
        (RegsUsed & getRegisterSet(RegSet_FramePointer, RegSet_None)).count() ==
        0);
    PreservedRegsSizeBytes += typeWidthInBytes(WordType);
    _link_bp();
  }

  // Align the variables area. SpillAreaPaddingBytes is the size of the region
  // after the preserved registers and before the spill areas.
  // LocalsSlotsPaddingBytes is the amount of padding between the globals and
  // locals area if they are separate.
  assert(LocalsSlotsAlignmentBytes <= SpillAreaAlignmentBytes);
  uint32_t SpillAreaPaddingBytes = 0;
  uint32_t LocalsSlotsPaddingBytes = 0;
  alignStackSpillAreas(X86_RET_IP_SIZE_BYTES + PreservedRegsSizeBytes,
                       SpillAreaAlignmentBytes, GlobalsSize,
                       LocalsSlotsAlignmentBytes, &SpillAreaPaddingBytes,
                       &LocalsSlotsPaddingBytes);
  SpillAreaSizeBytes += SpillAreaPaddingBytes + LocalsSlotsPaddingBytes;
  uint32_t GlobalsAndSubsequentPaddingSize =
      GlobalsSize + LocalsSlotsPaddingBytes;

  // Functions returning scalar floating point types may need to convert values
  // from an in-register xmm value to the top of the x87 floating point stack.
  // This is done by a movp[sd] and an fld[sd].  Ensure there is enough scratch
  // space on the stack for this.
  const Type ReturnType = Func->getReturnType();
  if (isScalarFloatingType(ReturnType)) {
    // Avoid misaligned double-precision load/store.
    RequiredStackAlignment =
        std::max<size_t>(RequiredStackAlignment, X86_STACK_ALIGNMENT_BYTES);
    SpillAreaSizeBytes =
        std::max(typeWidthInBytesOnStack(ReturnType), SpillAreaSizeBytes);
  }

  RequiredStackAlignment =
      std::max<size_t>(RequiredStackAlignment, SpillAreaAlignmentBytes);

  if (PrologEmitsFixedAllocas) {
    RequiredStackAlignment =
        std::max(RequiredStackAlignment, FixedAllocaAlignBytes);
  }

  // Combine fixed allocations into SpillAreaSizeBytes if we are emitting the
  // fixed allocations in the prolog.
  if (PrologEmitsFixedAllocas)
    SpillAreaSizeBytes += FixedAllocaSizeBytes;

  // Entering the function has made the stack pointer unaligned. Re-align it by
  // adjusting the stack size.
  // Note that StackOffset does not include spill area. It's the offset from the
  // base stack pointer (epb), whether we set it or not, to the the first stack
  // arg (if any). StackSize, on the other hand, does include the spill area.
  const uint32_t StackOffset = X86_RET_IP_SIZE_BYTES + PreservedRegsSizeBytes;
  uint32_t StackSize = Utils::applyAlignment(StackOffset + SpillAreaSizeBytes,
                                             RequiredStackAlignment);
  StackSize = Utils::applyAlignment(StackSize + maxOutArgsSizeBytes(),
                                    RequiredStackAlignment);
  SpillAreaSizeBytes = StackSize - StackOffset; // Adjust for alignment, if any

  if (SpillAreaSizeBytes) {
    auto *Func = Node->getCfg();
    if (SpillAreaSizeBytes > Func->getStackSizeLimit()) {
      Func->setError("Stack size limit exceeded");
    }

    emitStackProbe(SpillAreaSizeBytes);

    // Generate "sub stackptr, SpillAreaSizeBytes"
    _sub_sp(Ctx->getConstantInt32(SpillAreaSizeBytes));
  }

  // StackPointer: points just past the spill area (end of stack frame)

  // If the required alignment is greater than the stack pointer's guaranteed
  // alignment, align the stack pointer accordingly.
  if (RequiredStackAlignment > X86_STACK_ALIGNMENT_BYTES) {
    assert(IsEbpBasedFrame);
    _and(getPhysicalRegister(getStackReg(), WordType),
         Ctx->getConstantInt32(-RequiredStackAlignment));
  }

  // StackPointer: may have just been offset for alignment

  // Account for known-frame-offset alloca instructions that were not already
  // combined into the prolog.
  if (!PrologEmitsFixedAllocas)
    SpillAreaSizeBytes += FixedAllocaSizeBytes;

  Ctx->statsUpdateFrameBytes(SpillAreaSizeBytes);

  // Fill in stack offsets for stack args, and copy args into registers for
  // those that were register-allocated. Args are pushed right to left, so
  // Arg[0] is closest to the stack/frame pointer.
  RegNumT FrameOrStackReg = IsEbpBasedFrame ? getFrameReg() : getStackReg();
  Variable *FramePtr = getPhysicalRegister(FrameOrStackReg, WordType);
  size_t BasicFrameOffset = StackOffset;
  if (!IsEbpBasedFrame)
    BasicFrameOffset += SpillAreaSizeBytes;

  const VarList &Args = Func->getArgs();
  size_t InArgsSizeBytes = 0;
  unsigned NumXmmArgs = 0;
  unsigned NumGPRArgs = 0;
  for (SizeT i = 0, NumArgs = Args.size(); i < NumArgs; ++i) {
    Variable *Arg = Args[i];
    // Skip arguments passed in registers.
    if (isVectorType(Arg->getType())) {
      if (RegX8632::getRegisterForXmmArgNum(
              RegX8632::getArgIndex(i, NumXmmArgs))
              .hasValue()) {
        ++NumXmmArgs;
        continue;
      }
    } else if (!isScalarFloatingType(Arg->getType())) {
      assert(isScalarIntegerType(Arg->getType()));
      if (RegX8632::getRegisterForGprArgNum(
              WordType, RegX8632::getArgIndex(i, NumGPRArgs))
              .hasValue()) {
        ++NumGPRArgs;
        continue;
      }
    }
    // For esp-based frames where the allocas are done outside the prolog, the
    // esp value may not stabilize to its home value until after all the
    // fixed-size alloca instructions have executed.  In this case, a stack
    // adjustment is needed when accessing in-args in order to copy them into
    // registers.
    size_t StackAdjBytes = 0;
    if (!IsEbpBasedFrame && !PrologEmitsFixedAllocas)
      StackAdjBytes -= FixedAllocaSizeBytes;
    finishArgumentLowering(Arg, FramePtr, BasicFrameOffset, StackAdjBytes,
                           InArgsSizeBytes);
  }

  // Fill in stack offsets for locals.
  assignVarStackSlots(SortedSpilledVariables, SpillAreaPaddingBytes,
                      SpillAreaSizeBytes, GlobalsAndSubsequentPaddingSize,
                      IsEbpBasedFrame && !needsStackPointerAlignment());
  // Assign stack offsets to variables that have been linked to spilled
  // variables.
  for (Variable *Var : VariablesLinkedToSpillSlots) {
    const Variable *Root = Var->getLinkedToStackRoot();
    assert(Root != nullptr);
    Var->setStackOffset(Root->getStackOffset());

    // If the stack root variable is an arg, make this variable an arg too so
    // that stackVarToAsmAddress uses the correct base pointer (e.g. ebp on
    // x86).
    Var->setIsArg(Root->getIsArg());
  }
  this->HasComputedFrame = true;

  if (BuildDefs::dump() && Func->isVerbose(IceV_Frame)) {
    OstreamLocker L(Func->getContext());
    Ostream &Str = Func->getContext()->getStrDump();

    Str << "Stack layout:\n";
    uint32_t EspAdjustmentPaddingSize =
        SpillAreaSizeBytes - LocalsSpillAreaSize -
        GlobalsAndSubsequentPaddingSize - SpillAreaPaddingBytes -
        maxOutArgsSizeBytes();
    Str << " in-args = " << InArgsSizeBytes << " bytes\n"
        << " return address = " << X86_RET_IP_SIZE_BYTES << " bytes\n"
        << " preserved registers = " << PreservedRegsSizeBytes << " bytes\n"
        << " spill area padding = " << SpillAreaPaddingBytes << " bytes\n"
        << " globals spill area = " << GlobalsSize << " bytes\n"
        << " globals-locals spill areas intermediate padding = "
        << GlobalsAndSubsequentPaddingSize - GlobalsSize << " bytes\n"
        << " locals spill area = " << LocalsSpillAreaSize << " bytes\n"
        << " esp alignment padding = " << EspAdjustmentPaddingSize
        << " bytes\n";

    Str << "Stack details:\n"
        << " esp adjustment = " << SpillAreaSizeBytes << " bytes\n"
        << " spill area alignment = " << SpillAreaAlignmentBytes << " bytes\n"
        << " outgoing args size = " << maxOutArgsSizeBytes() << " bytes\n"
        << " locals spill area alignment = " << LocalsSlotsAlignmentBytes
        << " bytes\n"
        << " is ebp based = " << IsEbpBasedFrame << "\n";
  }
}

/// Helper function for addProlog().
///
/// This assumes Arg is an argument passed on the stack. This sets the frame
/// offset for Arg and updates InArgsSizeBytes according to Arg's width. For an
/// I64 arg that has been split into Lo and Hi components, it calls itself
/// recursively on the components, taking care to handle Lo first because of the
/// little-endian architecture. Lastly, this function generates an instruction
/// to copy Arg into its assigned register if applicable.

void TargetX8632::finishArgumentLowering(Variable *Arg, Variable *FramePtr,
                                         size_t BasicFrameOffset,
                                         size_t StackAdjBytes,
                                         size_t &InArgsSizeBytes) {
  if (auto *Arg64On32 = llvm::dyn_cast<Variable64On32>(Arg)) {
    Variable *Lo = Arg64On32->getLo();
    Variable *Hi = Arg64On32->getHi();
    finishArgumentLowering(Lo, FramePtr, BasicFrameOffset, StackAdjBytes,
                           InArgsSizeBytes);
    finishArgumentLowering(Hi, FramePtr, BasicFrameOffset, StackAdjBytes,
                           InArgsSizeBytes);
    return;
  }
  Type Ty = Arg->getType();
  if (isVectorType(Ty)) {
    InArgsSizeBytes = applyStackAlignment(InArgsSizeBytes);
  }
  Arg->setStackOffset(BasicFrameOffset + InArgsSizeBytes);
  InArgsSizeBytes += typeWidthInBytesOnStack(Ty);
  if (Arg->hasReg()) {
    assert(Ty != IceType_i64);
    auto *Mem = X86OperandMem::create(
        Func, Ty, FramePtr,
        Ctx->getConstantInt32(Arg->getStackOffset() + StackAdjBytes));
    if (isVectorType(Arg->getType())) {
      _movp(Arg, Mem);
    } else {
      _mov(Arg, Mem);
    }
    // This argument-copying instruction uses an explicit X86OperandMem
    // operand instead of a Variable, so its fill-from-stack operation has to
    // be tracked separately for statistics.
    Ctx->statsUpdateFills();
  }
}

void TargetX8632::addEpilog(CfgNode *Node) {
  InstList &Insts = Node->getInsts();
  InstList::reverse_iterator RI, E;
  for (RI = Insts.rbegin(), E = Insts.rend(); RI != E; ++RI) {
    if (llvm::isa<Insts::Ret>(*RI))
      break;
  }
  if (RI == E)
    return;

  // Convert the reverse_iterator position into its corresponding (forward)
  // iterator position.
  InstList::iterator InsertPoint = reverseToForwardIterator(RI);
  --InsertPoint;
  Context.init(Node);
  Context.setInsertPoint(InsertPoint);

  if (IsEbpBasedFrame) {
    _unlink_bp();
  } else {
    // add stackptr, SpillAreaSizeBytes
    if (SpillAreaSizeBytes != 0) {
      _add_sp(Ctx->getConstantInt32(SpillAreaSizeBytes));
    }
  }

  // Add pop instructions for preserved registers.
  SmallBitVector CalleeSaves = getRegisterSet(RegSet_CalleeSave, RegSet_None);
  SmallBitVector Popped(CalleeSaves.size());
  for (int32_t i = CalleeSaves.size() - 1; i >= 0; --i) {
    const auto RegNum = RegNumT::fromInt(i);
    if (RegNum == getFrameReg() && IsEbpBasedFrame)
      continue;
    const RegNumT Canonical = RegX8632::getBaseReg(RegNum);
    if (CalleeSaves[i] && RegsUsed[i]) {
      Popped[Canonical] = true;
    }
  }
  for (int32_t i = Popped.size() - 1; i >= 0; --i) {
    if (!Popped[i])
      continue;
    const auto RegNum = RegNumT::fromInt(i);
    assert(RegNum == RegX8632::getBaseReg(RegNum));
    _pop_reg(RegNum);
  }
}

Type TargetX8632::stackSlotType() { return WordType; }

Operand *TargetX8632::loOperand(Operand *Operand) {
  assert(Operand->getType() == IceType_i64 ||
         Operand->getType() == IceType_f64);
  if (Operand->getType() != IceType_i64 && Operand->getType() != IceType_f64)
    return Operand;
  if (auto *Var64On32 = llvm::dyn_cast<Variable64On32>(Operand))
    return Var64On32->getLo();
  if (auto *Const = llvm::dyn_cast<ConstantInteger64>(Operand)) {
    auto *ConstInt = llvm::dyn_cast<ConstantInteger32>(
        Ctx->getConstantInt32(static_cast<int32_t>(Const->getValue())));
    // Check if we need to blind/pool the constant.
    return legalize(ConstInt);
  }
  if (auto *Mem = llvm::dyn_cast<X86OperandMem>(Operand)) {
    auto *MemOperand = X86OperandMem::create(
        Func, IceType_i32, Mem->getBase(), Mem->getOffset(), Mem->getIndex(),
        Mem->getShift(), Mem->getSegmentRegister(), Mem->getIsRebased());
    // Test if we should randomize or pool the offset, if so randomize it or
    // pool it then create mem operand with the blinded/pooled constant.
    // Otherwise, return the mem operand as ordinary mem operand.
    return legalize(MemOperand);
  }
  llvm_unreachable("Unsupported operand type");
  return nullptr;
}

Operand *TargetX8632::hiOperand(Operand *Operand) {
  assert(Operand->getType() == IceType_i64 ||
         Operand->getType() == IceType_f64);
  if (Operand->getType() != IceType_i64 && Operand->getType() != IceType_f64)
    return Operand;
  if (auto *Var64On32 = llvm::dyn_cast<Variable64On32>(Operand))
    return Var64On32->getHi();
  if (auto *Const = llvm::dyn_cast<ConstantInteger64>(Operand)) {
    auto *ConstInt = llvm::dyn_cast<ConstantInteger32>(
        Ctx->getConstantInt32(static_cast<int32_t>(Const->getValue() >> 32)));
    // Check if we need to blind/pool the constant.
    return legalize(ConstInt);
  }
  if (auto *Mem = llvm::dyn_cast<X86OperandMem>(Operand)) {
    Constant *Offset = Mem->getOffset();
    if (Offset == nullptr) {
      Offset = Ctx->getConstantInt32(4);
    } else if (auto *IntOffset = llvm::dyn_cast<ConstantInteger32>(Offset)) {
      Offset = Ctx->getConstantInt32(4 + IntOffset->getValue());
    } else if (auto *SymOffset = llvm::dyn_cast<ConstantRelocatable>(Offset)) {
      assert(!Utils::WouldOverflowAdd(SymOffset->getOffset(), 4));
      Offset =
          Ctx->getConstantSym(4 + SymOffset->getOffset(), SymOffset->getName());
    }
    auto *MemOperand = X86OperandMem::create(
        Func, IceType_i32, Mem->getBase(), Offset, Mem->getIndex(),
        Mem->getShift(), Mem->getSegmentRegister(), Mem->getIsRebased());
    // Test if the Offset is an eligible i32 constants for randomization and
    // pooling. Blind/pool it if it is. Otherwise return as oridinary mem
    // operand.
    return legalize(MemOperand);
  }
  llvm_unreachable("Unsupported operand type");
  return nullptr;
}

SmallBitVector TargetX8632::getRegisterSet(RegSetMask Include,
                                           RegSetMask Exclude) const {
  return RegX8632::getRegisterSet(getFlags(), Include, Exclude);
}

void TargetX8632::lowerAlloca(const InstAlloca *Instr) {
  // Conservatively require the stack to be aligned. Some stack adjustment
  // operations implemented below assume that the stack is aligned before the
  // alloca. All the alloca code ensures that the stack alignment is preserved
  // after the alloca. The stack alignment restriction can be relaxed in some
  // cases.
  RequiredStackAlignment =
      std::max<size_t>(RequiredStackAlignment, X86_STACK_ALIGNMENT_BYTES);

  // For default align=0, set it to the real value 1, to avoid any
  // bit-manipulation problems below.
  const uint32_t AlignmentParam = std::max(1u, Instr->getAlignInBytes());

  // LLVM enforces power of 2 alignment.
  assert(llvm::isPowerOf2_32(AlignmentParam));
  assert(llvm::isPowerOf2_32(X86_STACK_ALIGNMENT_BYTES));

  const uint32_t Alignment =
      std::max(AlignmentParam, X86_STACK_ALIGNMENT_BYTES);
  const bool OverAligned = Alignment > X86_STACK_ALIGNMENT_BYTES;
  const bool OptM1 = Func->getOptLevel() == Opt_m1;
  const bool AllocaWithKnownOffset = Instr->getKnownFrameOffset();
  const bool UseFramePointer =
      hasFramePointer() || OverAligned || !AllocaWithKnownOffset || OptM1;

  if (UseFramePointer)
    setHasFramePointer();

  Variable *esp = getPhysicalRegister(getStackReg(), WordType);
  if (OverAligned) {
    _and(esp, Ctx->getConstantInt32(-Alignment));
  }

  Variable *Dest = Instr->getDest();
  Operand *TotalSize = legalize(Instr->getSizeInBytes());

  if (const auto *ConstantTotalSize =
          llvm::dyn_cast<ConstantInteger32>(TotalSize)) {
    const uint32_t Value =
        Utils::applyAlignment(ConstantTotalSize->getValue(), Alignment);
    if (UseFramePointer) {
      _sub_sp(Ctx->getConstantInt32(Value));
    } else {
      // If we don't need a Frame Pointer, this alloca has a known offset to the
      // stack pointer. We don't need adjust the stack pointer, nor assign any
      // value to Dest, as Dest is rematerializable.
      assert(Dest->isRematerializable());
      FixedAllocaSizeBytes += Value;
      Context.insert<InstFakeDef>(Dest);
    }
  } else {
    // Non-constant sizes need to be adjusted to the next highest multiple of
    // the required alignment at runtime.
    Variable *T = makeReg(IceType_i32);
    _mov(T, TotalSize);
    _add(T, Ctx->getConstantInt32(Alignment - 1));
    _and(T, Ctx->getConstantInt32(-Alignment));
    _sub_sp(T);
  }
  // Add enough to the returned address to account for the out args area.
  uint32_t OutArgsSize = maxOutArgsSizeBytes();
  if (OutArgsSize > 0) {
    Variable *T = makeReg(Dest->getType());
    auto *CalculateOperand = X86OperandMem::create(
        Func, IceType_void, esp, Ctx->getConstantInt(IceType_i32, OutArgsSize));
    _lea(T, CalculateOperand);
    _mov(Dest, T);
  } else {
    _mov(Dest, esp);
  }
}

void TargetX8632::lowerArguments() {
  const bool OptM1 = Func->getOptLevel() == Opt_m1;
  VarList &Args = Func->getArgs();
  unsigned NumXmmArgs = 0;
  bool XmmSlotsRemain = true;
  unsigned NumGprArgs = 0;
  bool GprSlotsRemain = true;

  Context.init(Func->getEntryNode());
  Context.setInsertPoint(Context.getCur());

  for (SizeT i = 0, End = Args.size();
       i < End && (XmmSlotsRemain || GprSlotsRemain); ++i) {
    Variable *Arg = Args[i];
    Type Ty = Arg->getType();
    Variable *RegisterArg = nullptr;
    RegNumT RegNum;
    if (isVectorType(Ty)) {
      RegNum = RegX8632::getRegisterForXmmArgNum(
          RegX8632::getArgIndex(i, NumXmmArgs));
      if (RegNum.hasNoValue()) {
        XmmSlotsRemain = false;
        continue;
      }
      ++NumXmmArgs;
      RegisterArg = Func->makeVariable(Ty);
    } else if (isScalarFloatingType(Ty)) {
      continue;
    } else if (isScalarIntegerType(Ty)) {
      RegNum = RegX8632::getRegisterForGprArgNum(
          Ty, RegX8632::getArgIndex(i, NumGprArgs));
      if (RegNum.hasNoValue()) {
        GprSlotsRemain = false;
        continue;
      }
      ++NumGprArgs;
      RegisterArg = Func->makeVariable(Ty);
    }
    assert(RegNum.hasValue());
    assert(RegisterArg != nullptr);
    // Replace Arg in the argument list with the home register. Then generate
    // an instruction in the prolog to copy the home register to the assigned
    // location of Arg.
    if (BuildDefs::dump())
      RegisterArg->setName(Func, "home_reg:" + Arg->getName());
    RegisterArg->setRegNum(RegNum);
    RegisterArg->setIsArg();
    Arg->setIsArg(false);

    Args[i] = RegisterArg;
    // When not Om1, do the assignment through a temporary, instead of directly
    // from the pre-colored variable, so that a subsequent availabilityGet()
    // call has a chance to work.  (In Om1, don't bother creating extra
    // instructions with extra variables to register-allocate.)
    if (OptM1) {
      Context.insert<InstAssign>(Arg, RegisterArg);
    } else {
      Variable *Tmp = makeReg(RegisterArg->getType());
      Context.insert<InstAssign>(Tmp, RegisterArg);
      Context.insert<InstAssign>(Arg, Tmp);
    }
  }
  if (!OptM1)
    Context.availabilityUpdate();
}

/// Strength-reduce scalar integer multiplication by a constant (for i32 or
/// narrower) for certain constants. The lea instruction can be used to multiply
/// by 3, 5, or 9, and the lsh instruction can be used to multiply by powers of
/// 2. These can be combined such that e.g. multiplying by 100 can be done as 2
/// lea-based multiplies by 5, combined with left-shifting by 2.

bool TargetX8632::optimizeScalarMul(Variable *Dest, Operand *Src0,
                                    int32_t Src1) {
  // Disable this optimization for Om1 and O0, just to keep things simple
  // there.
  if (Func->getOptLevel() < Opt_1)
    return false;
  Type Ty = Dest->getType();
  if (Src1 == -1) {
    Variable *T = nullptr;
    _mov(T, Src0);
    _neg(T);
    _mov(Dest, T);
    return true;
  }
  if (Src1 == 0) {
    _mov(Dest, Ctx->getConstantZero(Ty));
    return true;
  }
  if (Src1 == 1) {
    Variable *T = nullptr;
    _mov(T, Src0);
    _mov(Dest, T);
    return true;
  }
  // Don't bother with the edge case where Src1 == MININT.
  if (Src1 == -Src1)
    return false;
  const bool Src1IsNegative = Src1 < 0;
  if (Src1IsNegative)
    Src1 = -Src1;
  uint32_t Count9 = 0;
  uint32_t Count5 = 0;
  uint32_t Count3 = 0;
  uint32_t Count2 = 0;
  uint32_t CountOps = 0;
  while (Src1 > 1) {
    if (Src1 % 9 == 0) {
      ++CountOps;
      ++Count9;
      Src1 /= 9;
    } else if (Src1 % 5 == 0) {
      ++CountOps;
      ++Count5;
      Src1 /= 5;
    } else if (Src1 % 3 == 0) {
      ++CountOps;
      ++Count3;
      Src1 /= 3;
    } else if (Src1 % 2 == 0) {
      if (Count2 == 0)
        ++CountOps;
      ++Count2;
      Src1 /= 2;
    } else {
      return false;
    }
  }
  // Lea optimization only works for i16 and i32 types, not i8.
  if (Ty != IceType_i32 && (Count3 || Count5 || Count9))
    return false;
  // Limit the number of lea/shl operations for a single multiply, to a
  // somewhat arbitrary choice of 3.
  constexpr uint32_t MaxOpsForOptimizedMul = 3;
  if (CountOps > MaxOpsForOptimizedMul)
    return false;
  Variable *T = makeReg(WordType);
  if (typeWidthInBytes(Src0->getType()) < typeWidthInBytes(T->getType())) {
    Operand *Src0RM = legalize(Src0, Legal_Reg | Legal_Mem);
    _movzx(T, Src0RM);
  } else {
    _mov(T, Src0);
  }
  Constant *Zero = Ctx->getConstantZero(IceType_i32);
  for (uint32_t i = 0; i < Count9; ++i) {
    constexpr uint16_t Shift = 3; // log2(9-1)
    _lea(T, X86OperandMem::create(Func, IceType_void, T, Zero, T, Shift));
  }
  for (uint32_t i = 0; i < Count5; ++i) {
    constexpr uint16_t Shift = 2; // log2(5-1)
    _lea(T, X86OperandMem::create(Func, IceType_void, T, Zero, T, Shift));
  }
  for (uint32_t i = 0; i < Count3; ++i) {
    constexpr uint16_t Shift = 1; // log2(3-1)
    _lea(T, X86OperandMem::create(Func, IceType_void, T, Zero, T, Shift));
  }
  if (Count2) {
    _shl(T, Ctx->getConstantInt(Ty, Count2));
  }
  if (Src1IsNegative)
    _neg(T);
  _mov(Dest, T);
  return true;
}

void TargetX8632::lowerShift64(InstArithmetic::OpKind Op, Operand *Src0Lo,
                               Operand *Src0Hi, Operand *Src1Lo,
                               Variable *DestLo, Variable *DestHi) {
  // TODO: Refactor the similarities between Shl, Lshr, and Ashr.
  Variable *T_1 = nullptr, *T_2 = nullptr, *T_3 = nullptr;
  Constant *Zero = Ctx->getConstantZero(IceType_i32);
  Constant *SignExtend = Ctx->getConstantInt32(0x1f);
  if (auto *ConstantShiftAmount = llvm::dyn_cast<ConstantInteger32>(Src1Lo)) {
    uint32_t ShiftAmount = ConstantShiftAmount->getValue();
    if (ShiftAmount > 32) {
      Constant *ReducedShift = Ctx->getConstantInt32(ShiftAmount - 32);
      switch (Op) {
      default:
        assert(0 && "non-shift op");
        break;
      case InstArithmetic::Shl: {
        // a=b<<c ==>
        //   t2 = b.lo
        //   t2 = shl t2, ShiftAmount-32
        //   t3 = t2
        //   t2 = 0
        _mov(T_2, Src0Lo);
        _shl(T_2, ReducedShift);
        _mov(DestHi, T_2);
        _mov(DestLo, Zero);
      } break;
      case InstArithmetic::Lshr: {
        // a=b>>c (unsigned) ==>
        //   t2 = b.hi
        //   t2 = shr t2, ShiftAmount-32
        //   a.lo = t2
        //   a.hi = 0
        _mov(T_2, Src0Hi);
        _shr(T_2, ReducedShift);
        _mov(DestLo, T_2);
        _mov(DestHi, Zero);
      } break;
      case InstArithmetic::Ashr: {
        // a=b>>c (signed) ==>
        //   t3 = b.hi
        //   t3 = sar t3, 0x1f
        //   t2 = b.hi
        //   t2 = shrd t2, t3, ShiftAmount-32
        //   a.lo = t2
        //   a.hi = t3
        _mov(T_3, Src0Hi);
        _sar(T_3, SignExtend);
        _mov(T_2, Src0Hi);
        _shrd(T_2, T_3, ReducedShift);
        _mov(DestLo, T_2);
        _mov(DestHi, T_3);
      } break;
      }
    } else if (ShiftAmount == 32) {
      switch (Op) {
      default:
        assert(0 && "non-shift op");
        break;
      case InstArithmetic::Shl: {
        // a=b<<c ==>
        //   t2 = b.lo
        //   a.hi = t2
        //   a.lo = 0
        _mov(T_2, Src0Lo);
        _mov(DestHi, T_2);
        _mov(DestLo, Zero);
      } break;
      case InstArithmetic::Lshr: {
        // a=b>>c (unsigned) ==>
        //   t2 = b.hi
        //   a.lo = t2
        //   a.hi = 0
        _mov(T_2, Src0Hi);
        _mov(DestLo, T_2);
        _mov(DestHi, Zero);
      } break;
      case InstArithmetic::Ashr: {
        // a=b>>c (signed) ==>
        //   t2 = b.hi
        //   a.lo = t2
        //   t3 = b.hi
        //   t3 = sar t3, 0x1f
        //   a.hi = t3
        _mov(T_2, Src0Hi);
        _mov(DestLo, T_2);
        _mov(T_3, Src0Hi);
        _sar(T_3, SignExtend);
        _mov(DestHi, T_3);
      } break;
      }
    } else {
      // COMMON PREFIX OF: a=b SHIFT_OP c ==>
      //   t2 = b.lo
      //   t3 = b.hi
      _mov(T_2, Src0Lo);
      _mov(T_3, Src0Hi);
      switch (Op) {
      default:
        assert(0 && "non-shift op");
        break;
      case InstArithmetic::Shl: {
        // a=b<<c ==>
        //   t3 = shld t3, t2, ShiftAmount
        //   t2 = shl t2, ShiftAmount
        _shld(T_3, T_2, ConstantShiftAmount);
        _shl(T_2, ConstantShiftAmount);
      } break;
      case InstArithmetic::Lshr: {
        // a=b>>c (unsigned) ==>
        //   t2 = shrd t2, t3, ShiftAmount
        //   t3 = shr t3, ShiftAmount
        _shrd(T_2, T_3, ConstantShiftAmount);
        _shr(T_3, ConstantShiftAmount);
      } break;
      case InstArithmetic::Ashr: {
        // a=b>>c (signed) ==>
        //   t2 = shrd t2, t3, ShiftAmount
        //   t3 = sar t3, ShiftAmount
        _shrd(T_2, T_3, ConstantShiftAmount);
        _sar(T_3, ConstantShiftAmount);
      } break;
      }
      // COMMON SUFFIX OF: a=b SHIFT_OP c ==>
      //   a.lo = t2
      //   a.hi = t3
      _mov(DestLo, T_2);
      _mov(DestHi, T_3);
    }
  } else {
    // NON-CONSTANT CASES.
    Constant *BitTest = Ctx->getConstantInt32(0x20);
    InstX86Label *Label = InstX86Label::create(Func, this);
    // COMMON PREFIX OF: a=b SHIFT_OP c ==>
    //   t1:ecx = c.lo & 0xff
    //   t2 = b.lo
    //   t3 = b.hi
    T_1 = copyToReg8(Src1Lo, RegX8632::Reg_cl);
    _mov(T_2, Src0Lo);
    _mov(T_3, Src0Hi);
    switch (Op) {
    default:
      assert(0 && "non-shift op");
      break;
    case InstArithmetic::Shl: {
      // a=b<<c ==>
      //   t3 = shld t3, t2, t1
      //   t2 = shl t2, t1
      //   test t1, 0x20
      //   je L1
      //   use(t3)
      //   t3 = t2
      //   t2 = 0
      _shld(T_3, T_2, T_1);
      _shl(T_2, T_1);
      _test(T_1, BitTest);
      _br(CondX86::Br_e, Label);
      // T_2 and T_3 are being assigned again because of the intra-block control
      // flow, so we need to use _redefined to avoid liveness problems.
      _redefined(_mov(T_3, T_2));
      _redefined(_mov(T_2, Zero));
    } break;
    case InstArithmetic::Lshr: {
      // a=b>>c (unsigned) ==>
      //   t2 = shrd t2, t3, t1
      //   t3 = shr t3, t1
      //   test t1, 0x20
      //   je L1
      //   use(t2)
      //   t2 = t3
      //   t3 = 0
      _shrd(T_2, T_3, T_1);
      _shr(T_3, T_1);
      _test(T_1, BitTest);
      _br(CondX86::Br_e, Label);
      // T_2 and T_3 are being assigned again because of the intra-block control
      // flow, so we need to use _redefined to avoid liveness problems.
      _redefined(_mov(T_2, T_3));
      _redefined(_mov(T_3, Zero));
    } break;
    case InstArithmetic::Ashr: {
      // a=b>>c (signed) ==>
      //   t2 = shrd t2, t3, t1
      //   t3 = sar t3, t1
      //   test t1, 0x20
      //   je L1
      //   use(t2)
      //   t2 = t3
      //   t3 = sar t3, 0x1f
      Constant *SignExtend = Ctx->getConstantInt32(0x1f);
      _shrd(T_2, T_3, T_1);
      _sar(T_3, T_1);
      _test(T_1, BitTest);
      _br(CondX86::Br_e, Label);
      // T_2 and T_3 are being assigned again because of the intra-block control
      // flow, so T_2 needs to use _redefined to avoid liveness problems. T_3
      // doesn't need special treatment because it is reassigned via _sar
      // instead of _mov.
      _redefined(_mov(T_2, T_3));
      _sar(T_3, SignExtend);
    } break;
    }
    // COMMON SUFFIX OF: a=b SHIFT_OP c ==>
    // L1:
    //   a.lo = t2
    //   a.hi = t3
    Context.insert(Label);
    _mov(DestLo, T_2);
    _mov(DestHi, T_3);
  }
}

void TargetX8632::lowerArithmetic(const InstArithmetic *Instr) {
  Variable *Dest = Instr->getDest();
  if (Dest->isRematerializable()) {
    Context.insert<InstFakeDef>(Dest);
    return;
  }
  Type Ty = Dest->getType();
  Operand *Src0 = legalize(Instr->getSrc(0));
  Operand *Src1 = legalize(Instr->getSrc(1));
  if (Instr->isCommutative()) {
    uint32_t SwapCount = 0;
    if (!llvm::isa<Variable>(Src0) && llvm::isa<Variable>(Src1)) {
      std::swap(Src0, Src1);
      ++SwapCount;
    }
    if (llvm::isa<Constant>(Src0) && !llvm::isa<Constant>(Src1)) {
      std::swap(Src0, Src1);
      ++SwapCount;
    }
    // Improve two-address code patterns by avoiding a copy to the dest
    // register when one of the source operands ends its lifetime here.
    if (!Instr->isLastUse(Src0) && Instr->isLastUse(Src1)) {
      std::swap(Src0, Src1);
      ++SwapCount;
    }
    assert(SwapCount <= 1);
    (void)SwapCount;
  }
  if (Ty == IceType_i64) {
    // These x86-32 helper-call-involved instructions are lowered in this
    // separate switch. This is because loOperand() and hiOperand() may insert
    // redundant instructions for constant blinding and pooling. Such redundant
    // instructions will fail liveness analysis under -Om1 setting. And,
    // actually these arguments do not need to be processed with loOperand()
    // and hiOperand() to be used.
    switch (Instr->getOp()) {
    case InstArithmetic::Udiv:
    case InstArithmetic::Sdiv:
    case InstArithmetic::Urem:
    case InstArithmetic::Srem:
      llvm::report_fatal_error("Helper call was expected");
      return;
    default:
      break;
    }

    auto *DestLo = llvm::cast<Variable>(loOperand(Dest));
    auto *DestHi = llvm::cast<Variable>(hiOperand(Dest));
    Operand *Src0Lo = loOperand(Src0);
    Operand *Src0Hi = hiOperand(Src0);
    Operand *Src1Lo = loOperand(Src1);
    Operand *Src1Hi = hiOperand(Src1);
    Variable *T_Lo = nullptr, *T_Hi = nullptr;
    switch (Instr->getOp()) {
    case InstArithmetic::_num:
      llvm_unreachable("Unknown arithmetic operator");
      break;
    case InstArithmetic::Add:
      _mov(T_Lo, Src0Lo);
      _add(T_Lo, Src1Lo);
      _mov(DestLo, T_Lo);
      _mov(T_Hi, Src0Hi);
      _adc(T_Hi, Src1Hi);
      _mov(DestHi, T_Hi);
      break;
    case InstArithmetic::And:
      _mov(T_Lo, Src0Lo);
      _and(T_Lo, Src1Lo);
      _mov(DestLo, T_Lo);
      _mov(T_Hi, Src0Hi);
      _and(T_Hi, Src1Hi);
      _mov(DestHi, T_Hi);
      break;
    case InstArithmetic::Or:
      _mov(T_Lo, Src0Lo);
      _or(T_Lo, Src1Lo);
      _mov(DestLo, T_Lo);
      _mov(T_Hi, Src0Hi);
      _or(T_Hi, Src1Hi);
      _mov(DestHi, T_Hi);
      break;
    case InstArithmetic::Xor:
      _mov(T_Lo, Src0Lo);
      _xor(T_Lo, Src1Lo);
      _mov(DestLo, T_Lo);
      _mov(T_Hi, Src0Hi);
      _xor(T_Hi, Src1Hi);
      _mov(DestHi, T_Hi);
      break;
    case InstArithmetic::Sub:
      _mov(T_Lo, Src0Lo);
      _sub(T_Lo, Src1Lo);
      _mov(DestLo, T_Lo);
      _mov(T_Hi, Src0Hi);
      _sbb(T_Hi, Src1Hi);
      _mov(DestHi, T_Hi);
      break;
    case InstArithmetic::Mul: {
      Variable *T_1 = nullptr, *T_2 = nullptr, *T_3 = nullptr;
      Variable *T_4Lo = makeReg(IceType_i32, RegX8632::Reg_eax);
      Variable *T_4Hi = makeReg(IceType_i32, RegX8632::Reg_edx);
      // gcc does the following:
      // a=b*c ==>
      //   t1 = b.hi; t1 *=(imul) c.lo
      //   t2 = c.hi; t2 *=(imul) b.lo
      //   t3:eax = b.lo
      //   t4.hi:edx,t4.lo:eax = t3:eax *(mul) c.lo
      //   a.lo = t4.lo
      //   t4.hi += t1
      //   t4.hi += t2
      //   a.hi = t4.hi
      // The mul instruction cannot take an immediate operand.
      Src1Lo = legalize(Src1Lo, Legal_Reg | Legal_Mem);
      _mov(T_1, Src0Hi);
      _imul(T_1, Src1Lo);
      _mov(T_3, Src0Lo, RegX8632::Reg_eax);
      _mul(T_4Lo, T_3, Src1Lo);
      // The mul instruction produces two dest variables, edx:eax. We create a
      // fake definition of edx to account for this.
      Context.insert<InstFakeDef>(T_4Hi, T_4Lo);
      Context.insert<InstFakeUse>(T_4Hi);
      _mov(DestLo, T_4Lo);
      _add(T_4Hi, T_1);
      _mov(T_2, Src1Hi);
      Src0Lo = legalize(Src0Lo, Legal_Reg | Legal_Mem);
      _imul(T_2, Src0Lo);
      _add(T_4Hi, T_2);
      _mov(DestHi, T_4Hi);
    } break;
    case InstArithmetic::Shl:
    case InstArithmetic::Lshr:
    case InstArithmetic::Ashr:
      lowerShift64(Instr->getOp(), Src0Lo, Src0Hi, Src1Lo, DestLo, DestHi);
      break;
    case InstArithmetic::Fadd:
    case InstArithmetic::Fsub:
    case InstArithmetic::Fmul:
    case InstArithmetic::Fdiv:
    case InstArithmetic::Frem:
      llvm_unreachable("FP instruction with i64 type");
      break;
    case InstArithmetic::Udiv:
    case InstArithmetic::Sdiv:
    case InstArithmetic::Urem:
    case InstArithmetic::Srem:
      llvm_unreachable("Call-helper-involved instruction for i64 type \
                       should have already been handled before");
      break;
    }
    return;
  }
  if (isVectorType(Ty)) {
    // TODO: Trap on integer divide and integer modulo by zero. See:
    // https://code.google.com/p/nativeclient/issues/detail?id=3899
    if (llvm::isa<X86OperandMem>(Src1))
      Src1 = legalizeToReg(Src1);
    switch (Instr->getOp()) {
    case InstArithmetic::_num:
      llvm_unreachable("Unknown arithmetic operator");
      break;
    case InstArithmetic::Add: {
      Variable *T = makeReg(Ty);
      _movp(T, Src0);
      _padd(T, Src1);
      _movp(Dest, T);
    } break;
    case InstArithmetic::And: {
      Variable *T = makeReg(Ty);
      _movp(T, Src0);
      _pand(T, Src1);
      _movp(Dest, T);
    } break;
    case InstArithmetic::Or: {
      Variable *T = makeReg(Ty);
      _movp(T, Src0);
      _por(T, Src1);
      _movp(Dest, T);
    } break;
    case InstArithmetic::Xor: {
      Variable *T = makeReg(Ty);
      _movp(T, Src0);
      _pxor(T, Src1);
      _movp(Dest, T);
    } break;
    case InstArithmetic::Sub: {
      Variable *T = makeReg(Ty);
      _movp(T, Src0);
      _psub(T, Src1);
      _movp(Dest, T);
    } break;
    case InstArithmetic::Mul: {
      bool TypesAreValidForPmull = Ty == IceType_v4i32 || Ty == IceType_v8i16;
      bool InstructionSetIsValidForPmull =
          Ty == IceType_v8i16 || InstructionSet >= SSE4_1;
      if (TypesAreValidForPmull && InstructionSetIsValidForPmull) {
        Variable *T = makeReg(Ty);
        _movp(T, Src0);
        _pmull(T, Src0 == Src1 ? T : Src1);
        _movp(Dest, T);
      } else if (Ty == IceType_v4i32) {
        // Lowering sequence:
        // Note: The mask arguments have index 0 on the left.
        //
        // movups  T1, Src0
        // pshufd  T2, Src0, {1,0,3,0}
        // pshufd  T3, Src1, {1,0,3,0}
        // # T1 = {Src0[0] * Src1[0], Src0[2] * Src1[2]}
        // pmuludq T1, Src1
        // # T2 = {Src0[1] * Src1[1], Src0[3] * Src1[3]}
        // pmuludq T2, T3
        // # T1 = {lo(T1[0]), lo(T1[2]), lo(T2[0]), lo(T2[2])}
        // shufps  T1, T2, {0,2,0,2}
        // pshufd  T4, T1, {0,2,1,3}
        // movups  Dest, T4

        // Mask that directs pshufd to create a vector with entries
        // Src[1, 0, 3, 0]
        constexpr unsigned Constant1030 = 0x31;
        Constant *Mask1030 = Ctx->getConstantInt32(Constant1030);
        // Mask that directs shufps to create a vector with entries
        // Dest[0, 2], Src[0, 2]
        constexpr unsigned Mask0202 = 0x88;
        // Mask that directs pshufd to create a vector with entries
        // Src[0, 2, 1, 3]
        constexpr unsigned Mask0213 = 0xd8;
        Variable *T1 = makeReg(IceType_v4i32);
        Variable *T2 = makeReg(IceType_v4i32);
        Variable *T3 = makeReg(IceType_v4i32);
        Variable *T4 = makeReg(IceType_v4i32);
        _movp(T1, Src0);
        _pshufd(T2, Src0, Mask1030);
        _pshufd(T3, Src1, Mask1030);
        _pmuludq(T1, Src1);
        _pmuludq(T2, T3);
        _shufps(T1, T2, Ctx->getConstantInt32(Mask0202));
        _pshufd(T4, T1, Ctx->getConstantInt32(Mask0213));
        _movp(Dest, T4);
      } else if (Ty == IceType_v16i8) {
        llvm::report_fatal_error("Scalarized operation was expected");
      } else {
        llvm::report_fatal_error("Invalid vector multiply type");
      }
    } break;
    case InstArithmetic::Shl: {
      assert(llvm::isa<Constant>(Src1) && "Non-constant shift not scalarized");
      Variable *T = makeReg(Ty);
      _movp(T, Src0);
      _psll(T, Src1);
      _movp(Dest, T);
    } break;
    case InstArithmetic::Lshr: {
      assert(llvm::isa<Constant>(Src1) && "Non-constant shift not scalarized");
      Variable *T = makeReg(Ty);
      _movp(T, Src0);
      _psrl(T, Src1);
      _movp(Dest, T);
    } break;
    case InstArithmetic::Ashr: {
      assert(llvm::isa<Constant>(Src1) && "Non-constant shift not scalarized");
      Variable *T = makeReg(Ty);
      _movp(T, Src0);
      _psra(T, Src1);
      _movp(Dest, T);
    } break;
    case InstArithmetic::Udiv:
    case InstArithmetic::Urem:
    case InstArithmetic::Sdiv:
    case InstArithmetic::Srem:
      llvm::report_fatal_error("Scalarized operation was expected");
      break;
    case InstArithmetic::Fadd: {
      Variable *T = makeReg(Ty);
      _movp(T, Src0);
      _addps(T, Src1);
      _movp(Dest, T);
    } break;
    case InstArithmetic::Fsub: {
      Variable *T = makeReg(Ty);
      _movp(T, Src0);
      _subps(T, Src1);
      _movp(Dest, T);
    } break;
    case InstArithmetic::Fmul: {
      Variable *T = makeReg(Ty);
      _movp(T, Src0);
      _mulps(T, Src0 == Src1 ? T : Src1);
      _movp(Dest, T);
    } break;
    case InstArithmetic::Fdiv: {
      Variable *T = makeReg(Ty);
      _movp(T, Src0);
      _divps(T, Src1);
      _movp(Dest, T);
    } break;
    case InstArithmetic::Frem:
      llvm::report_fatal_error("Scalarized operation was expected");
      break;
    }
    return;
  }
  Variable *T_edx = nullptr;
  Variable *T = nullptr;
  switch (Instr->getOp()) {
  case InstArithmetic::_num:
    llvm_unreachable("Unknown arithmetic operator");
    break;
  case InstArithmetic::Add: {
    const bool ValidType = Ty == IceType_i32;
    auto *Const = llvm::dyn_cast<Constant>(Instr->getSrc(1));
    const bool ValidKind =
        Const != nullptr && (llvm::isa<ConstantInteger32>(Const) ||
                             llvm::isa<ConstantRelocatable>(Const));
    if (getFlags().getAggressiveLea() && ValidType && ValidKind) {
      auto *Var = legalizeToReg(Src0);
      auto *Mem = X86OperandMem::create(Func, IceType_void, Var, Const);
      T = makeReg(Ty);
      _lea(T, Mem);
      _mov(Dest, T);
      break;
    }
    _mov(T, Src0);
    _add(T, Src1);
    _mov(Dest, T);
  } break;
  case InstArithmetic::And:
    _mov(T, Src0);
    _and(T, Src1);
    _mov(Dest, T);
    break;
  case InstArithmetic::Or:
    _mov(T, Src0);
    _or(T, Src1);
    _mov(Dest, T);
    break;
  case InstArithmetic::Xor:
    _mov(T, Src0);
    _xor(T, Src1);
    _mov(Dest, T);
    break;
  case InstArithmetic::Sub:
    _mov(T, Src0);
    _sub(T, Src1);
    _mov(Dest, T);
    break;
  case InstArithmetic::Mul:
    if (auto *C = llvm::dyn_cast<ConstantInteger32>(Src1)) {
      if (optimizeScalarMul(Dest, Src0, C->getValue()))
        return;
    }
    // The 8-bit version of imul only allows the form "imul r/m8" where T must
    // be in al.
    if (isByteSizedArithType(Ty)) {
      _mov(T, Src0, RegX8632::Reg_al);
      Src1 = legalize(Src1, Legal_Reg | Legal_Mem);
      _imul(T, Src0 == Src1 ? T : Src1);
      _mov(Dest, T);
    } else if (auto *ImmConst = llvm::dyn_cast<ConstantInteger32>(Src1)) {
      T = makeReg(Ty);
      Src0 = legalize(Src0, Legal_Reg | Legal_Mem);
      _imul_imm(T, Src0, ImmConst);
      _mov(Dest, T);
    } else {
      _mov(T, Src0);
      // No need to legalize Src1 to Reg | Mem because the Imm case is handled
      // already by the ConstantInteger32 case above.
      _imul(T, Src0 == Src1 ? T : Src1);
      _mov(Dest, T);
    }
    break;
  case InstArithmetic::Shl:
    _mov(T, Src0);
    if (!llvm::isa<ConstantInteger32>(Src1) &&
        !llvm::isa<ConstantInteger64>(Src1))
      Src1 = copyToReg8(Src1, RegX8632::Reg_cl);
    _shl(T, Src1);
    _mov(Dest, T);
    break;
  case InstArithmetic::Lshr:
    _mov(T, Src0);
    if (!llvm::isa<ConstantInteger32>(Src1) &&
        !llvm::isa<ConstantInteger64>(Src1))
      Src1 = copyToReg8(Src1, RegX8632::Reg_cl);
    _shr(T, Src1);
    _mov(Dest, T);
    break;
  case InstArithmetic::Ashr:
    _mov(T, Src0);
    if (!llvm::isa<ConstantInteger32>(Src1) &&
        !llvm::isa<ConstantInteger64>(Src1))
      Src1 = copyToReg8(Src1, RegX8632::Reg_cl);
    _sar(T, Src1);
    _mov(Dest, T);
    break;
  case InstArithmetic::Udiv: {
    // div and idiv are the few arithmetic operators that do not allow
    // immediates as the operand.
    Src1 = legalize(Src1, Legal_Reg | Legal_Mem);
    RegNumT Eax;
    RegNumT Edx;
    switch (Ty) {
    default:
      llvm::report_fatal_error("Bad type for udiv");
    case IceType_i32:
      Eax = RegX8632::Reg_eax;
      Edx = RegX8632::Reg_edx;
      break;
    case IceType_i16:
      Eax = RegX8632::Reg_ax;
      Edx = RegX8632::Reg_dx;
      break;
    case IceType_i8:
      Eax = RegX8632::Reg_al;
      Edx = RegX8632::Reg_ah;
      break;
    }
    T_edx = makeReg(Ty, Edx);
    _mov(T, Src0, Eax);
    _mov(T_edx, Ctx->getConstantZero(Ty));
    _div(T_edx, Src1, T);
    _redefined(Context.insert<InstFakeDef>(T, T_edx));
    _mov(Dest, T);
  } break;
  case InstArithmetic::Sdiv:
    // TODO(stichnot): Enable this after doing better performance and cross
    // testing.
    if (false && Func->getOptLevel() >= Opt_1) {
      // Optimize division by constant power of 2, but not for Om1 or O0, just
      // to keep things simple there.
      if (auto *C = llvm::dyn_cast<ConstantInteger32>(Src1)) {
        const int32_t Divisor = C->getValue();
        const uint32_t UDivisor = Divisor;
        if (Divisor > 0 && llvm::isPowerOf2_32(UDivisor)) {
          uint32_t LogDiv = llvm::Log2_32(UDivisor);
          // LLVM does the following for dest=src/(1<<log):
          //   t=src
          //   sar t,typewidth-1 // -1 if src is negative, 0 if not
          //   shr t,typewidth-log
          //   add t,src
          //   sar t,log
          //   dest=t
          uint32_t TypeWidth = X86_CHAR_BIT * typeWidthInBytes(Ty);
          _mov(T, Src0);
          // If for some reason we are dividing by 1, just treat it like an
          // assignment.
          if (LogDiv > 0) {
            // The initial sar is unnecessary when dividing by 2.
            if (LogDiv > 1)
              _sar(T, Ctx->getConstantInt(Ty, TypeWidth - 1));
            _shr(T, Ctx->getConstantInt(Ty, TypeWidth - LogDiv));
            _add(T, Src0);
            _sar(T, Ctx->getConstantInt(Ty, LogDiv));
          }
          _mov(Dest, T);
          return;
        }
      }
    }
    Src1 = legalize(Src1, Legal_Reg | Legal_Mem);
    switch (Ty) {
    default:
      llvm::report_fatal_error("Bad type for sdiv");
    case IceType_i32:
      T_edx = makeReg(Ty, RegX8632::Reg_edx);
      _mov(T, Src0, RegX8632::Reg_eax);
      break;
    case IceType_i16:
      T_edx = makeReg(Ty, RegX8632::Reg_dx);
      _mov(T, Src0, RegX8632::Reg_ax);
      break;
    case IceType_i8:
      T_edx = makeReg(IceType_i16, RegX8632::Reg_ax);
      _mov(T, Src0, RegX8632::Reg_al);
      break;
    }
    _cbwdq(T_edx, T);
    _idiv(T_edx, Src1, T);
    _redefined(Context.insert<InstFakeDef>(T, T_edx));
    _mov(Dest, T);
    break;
  case InstArithmetic::Urem: {
    Src1 = legalize(Src1, Legal_Reg | Legal_Mem);
    RegNumT Eax;
    RegNumT Edx;
    switch (Ty) {
    default:
      llvm::report_fatal_error("Bad type for urem");
    case IceType_i32:
      Eax = RegX8632::Reg_eax;
      Edx = RegX8632::Reg_edx;
      break;
    case IceType_i16:
      Eax = RegX8632::Reg_ax;
      Edx = RegX8632::Reg_dx;
      break;
    case IceType_i8:
      Eax = RegX8632::Reg_al;
      Edx = RegX8632::Reg_ah;
      break;
    }
    T_edx = makeReg(Ty, Edx);
    _mov(T_edx, Ctx->getConstantZero(Ty));
    _mov(T, Src0, Eax);
    _div(T, Src1, T_edx);
    _redefined(Context.insert<InstFakeDef>(T_edx, T));
    if (Ty == IceType_i8) {
      // Register ah must be moved into one of {al,bl,cl,dl} before it can be
      // moved into a general 8-bit register.
      auto *T_AhRcvr = makeReg(Ty);
      T_AhRcvr->setRegClass(RCX86_IsAhRcvr);
      _mov(T_AhRcvr, T_edx);
      T_edx = T_AhRcvr;
    }
    _mov(Dest, T_edx);
  } break;
  case InstArithmetic::Srem: {
    // TODO(stichnot): Enable this after doing better performance and cross
    // testing.
    if (false && Func->getOptLevel() >= Opt_1) {
      // Optimize mod by constant power of 2, but not for Om1 or O0, just to
      // keep things simple there.
      if (auto *C = llvm::dyn_cast<ConstantInteger32>(Src1)) {
        const int32_t Divisor = C->getValue();
        const uint32_t UDivisor = Divisor;
        if (Divisor > 0 && llvm::isPowerOf2_32(UDivisor)) {
          uint32_t LogDiv = llvm::Log2_32(UDivisor);
          // LLVM does the following for dest=src%(1<<log):
          //   t=src
          //   sar t,typewidth-1 // -1 if src is negative, 0 if not
          //   shr t,typewidth-log
          //   add t,src
          //   and t, -(1<<log)
          //   sub t,src
          //   neg t
          //   dest=t
          uint32_t TypeWidth = X86_CHAR_BIT * typeWidthInBytes(Ty);
          // If for some reason we are dividing by 1, just assign 0.
          if (LogDiv == 0) {
            _mov(Dest, Ctx->getConstantZero(Ty));
            return;
          }
          _mov(T, Src0);
          // The initial sar is unnecessary when dividing by 2.
          if (LogDiv > 1)
            _sar(T, Ctx->getConstantInt(Ty, TypeWidth - 1));
          _shr(T, Ctx->getConstantInt(Ty, TypeWidth - LogDiv));
          _add(T, Src0);
          _and(T, Ctx->getConstantInt(Ty, -(1 << LogDiv)));
          _sub(T, Src0);
          _neg(T);
          _mov(Dest, T);
          return;
        }
      }
    }
    Src1 = legalize(Src1, Legal_Reg | Legal_Mem);
    RegNumT Eax;
    RegNumT Edx;
    switch (Ty) {
    default:
      llvm::report_fatal_error("Bad type for srem");
    case IceType_i32:
      Eax = RegX8632::Reg_eax;
      Edx = RegX8632::Reg_edx;
      break;
    case IceType_i16:
      Eax = RegX8632::Reg_ax;
      Edx = RegX8632::Reg_dx;
      break;
    case IceType_i8:
      Eax = RegX8632::Reg_al;
      Edx = RegX8632::Reg_ah;
      break;
    }
    T_edx = makeReg(Ty, Edx);
    _mov(T, Src0, Eax);
    _cbwdq(T_edx, T);
    _idiv(T, Src1, T_edx);
    _redefined(Context.insert<InstFakeDef>(T_edx, T));
    if (Ty == IceType_i8) {
      // Register ah must be moved into one of {al,bl,cl,dl} before it can be
      // moved into a general 8-bit register.
      auto *T_AhRcvr = makeReg(Ty);
      T_AhRcvr->setRegClass(RCX86_IsAhRcvr);
      _mov(T_AhRcvr, T_edx);
      T_edx = T_AhRcvr;
    }
    _mov(Dest, T_edx);
  } break;
  case InstArithmetic::Fadd:
    _mov(T, Src0);
    _addss(T, Src1);
    _mov(Dest, T);
    break;
  case InstArithmetic::Fsub:
    _mov(T, Src0);
    _subss(T, Src1);
    _mov(Dest, T);
    break;
  case InstArithmetic::Fmul:
    _mov(T, Src0);
    _mulss(T, Src0 == Src1 ? T : Src1);
    _mov(Dest, T);
    break;
  case InstArithmetic::Fdiv:
    _mov(T, Src0);
    _divss(T, Src1);
    _mov(Dest, T);
    break;
  case InstArithmetic::Frem:
    llvm::report_fatal_error("Helper call was expected");
    break;
  }
}

void TargetX8632::lowerAssign(const InstAssign *Instr) {
  Variable *Dest = Instr->getDest();
  if (Dest->isRematerializable()) {
    Context.insert<InstFakeDef>(Dest);
    return;
  }
  Operand *Src = Instr->getSrc(0);
  assert(Dest->getType() == Src->getType());
  lowerMove(Dest, Src, false);
}

void TargetX8632::lowerBr(const InstBr *Br) {
  if (Br->isUnconditional()) {
    _br(Br->getTargetUnconditional());
    return;
  }
  Operand *Cond = Br->getCondition();

  // Handle folding opportunities.
  if (const Inst *Producer = FoldingInfo.getProducerFor(Cond)) {
    assert(Producer->isDeleted());
    switch (BoolFolding::getProducerKind(Producer)) {
    default:
      break;
    case BoolFolding::PK_Icmp32:
    case BoolFolding::PK_Icmp64: {
      lowerIcmpAndConsumer(llvm::cast<InstIcmp>(Producer), Br);
      return;
    }
    case BoolFolding::PK_Fcmp: {
      lowerFcmpAndConsumer(llvm::cast<InstFcmp>(Producer), Br);
      return;
    }
    case BoolFolding::PK_Arith: {
      lowerArithAndConsumer(llvm::cast<InstArithmetic>(Producer), Br);
      return;
    }
    }
  }
  Operand *Src0 = legalize(Cond, Legal_Reg | Legal_Mem);
  Constant *Zero = Ctx->getConstantZero(IceType_i32);
  _cmp(Src0, Zero);
  _br(CondX86::Br_ne, Br->getTargetTrue(), Br->getTargetFalse());
}

// constexprMax returns a (constexpr) max(S0, S1), and it is used for defining
// OperandList in lowerCall. std::max() is supposed to work, but it doesn't.
inline constexpr SizeT constexprMax(SizeT S0, SizeT S1) {
  return S0 < S1 ? S1 : S0;
}

void TargetX8632::lowerCall(const InstCall *Instr) {
  // System V x86-32 calling convention lowering:
  //
  // * At the point before the call, the stack must be aligned to 16 bytes.
  //
  // * Non-register arguments are pushed onto the stack in right-to-left order,
  // such that the left-most argument ends up on the top of the stack at the
  // lowest memory address.
  //
  // * Stack arguments of vector type are aligned to start at the next highest
  // multiple of 16 bytes. Other stack arguments are aligned to the next word
  // size boundary (4 or 8 bytes, respectively).
  //
  // This is compatible with the Microsoft x86-32 'cdecl' calling convention,
  // which doesn't have a 16-byte stack alignment requirement.

  RequiredStackAlignment =
      std::max<size_t>(RequiredStackAlignment, X86_STACK_ALIGNMENT_BYTES);

  constexpr SizeT MaxOperands =
      constexprMax(RegX8632::X86_MAX_XMM_ARGS, RegX8632::X86_MAX_GPR_ARGS);
  using OperandList = llvm::SmallVector<Operand *, MaxOperands>;

  OperandList XmmArgs;
  llvm::SmallVector<SizeT, MaxOperands> XmmArgIndices;
  CfgVector<std::pair<const Type, Operand *>> GprArgs;
  CfgVector<SizeT> GprArgIndices;
  OperandList StackArgs, StackArgLocations;
  uint32_t ParameterAreaSizeBytes = 0;

  // Classify each argument operand according to the location where the argument
  // is passed.
  for (SizeT i = 0, NumArgs = Instr->getNumArgs(); i < NumArgs; ++i) {
    Operand *Arg = Instr->getArg(i);
    const Type Ty = Arg->getType();
    // The PNaCl ABI requires the width of arguments to be at least 32 bits.
    assert(typeWidthInBytes(Ty) >= 4);
    if (isVectorType(Ty) && RegX8632::getRegisterForXmmArgNum(
                                RegX8632::getArgIndex(i, XmmArgs.size()))
                                .hasValue()) {
      XmmArgs.push_back(Arg);
      XmmArgIndices.push_back(i);
    } else if (isScalarIntegerType(Ty) &&
               RegX8632::getRegisterForGprArgNum(
                   Ty, RegX8632::getArgIndex(i, GprArgs.size()))
                   .hasValue()) {
      GprArgs.emplace_back(Ty, Arg);
      GprArgIndices.push_back(i);
    } else {
      // Place on stack.
      StackArgs.push_back(Arg);
      if (isVectorType(Arg->getType())) {
        ParameterAreaSizeBytes = applyStackAlignment(ParameterAreaSizeBytes);
      }
      Variable *esp = getPhysicalRegister(getStackReg(), WordType);
      Constant *Loc = Ctx->getConstantInt32(ParameterAreaSizeBytes);
      StackArgLocations.push_back(X86OperandMem::create(Func, Ty, esp, Loc));
      ParameterAreaSizeBytes += typeWidthInBytesOnStack(Arg->getType());
    }
  }
  // Ensure there is enough space for the fstp/movs for floating returns.
  Variable *Dest = Instr->getDest();
  const Type DestTy = Dest ? Dest->getType() : IceType_void;
  if (isScalarFloatingType(DestTy)) {
    ParameterAreaSizeBytes =
        std::max(static_cast<size_t>(ParameterAreaSizeBytes),
                 typeWidthInBytesOnStack(DestTy));
  }
  // Adjust the parameter area so that the stack is aligned. It is assumed that
  // the stack is already aligned at the start of the calling sequence.
  ParameterAreaSizeBytes = applyStackAlignment(ParameterAreaSizeBytes);
  assert(ParameterAreaSizeBytes <= maxOutArgsSizeBytes());
  // Copy arguments that are passed on the stack to the appropriate stack
  // locations.  We make sure legalize() is called on each argument at this
  // point, to allow availabilityGet() to work.
  for (SizeT i = 0, NumStackArgs = StackArgs.size(); i < NumStackArgs; ++i) {
    lowerStore(
        InstStore::create(Func, legalize(StackArgs[i]), StackArgLocations[i]));
  }
  // Copy arguments to be passed in registers to the appropriate registers.
  for (SizeT i = 0, NumXmmArgs = XmmArgs.size(); i < NumXmmArgs; ++i) {
    XmmArgs[i] = legalizeToReg(legalize(XmmArgs[i]),
                               RegX8632::getRegisterForXmmArgNum(
                                   RegX8632::getArgIndex(XmmArgIndices[i], i)));
  }
  // Materialize moves for arguments passed in GPRs.
  for (SizeT i = 0, NumGprArgs = GprArgs.size(); i < NumGprArgs; ++i) {
    const Type SignatureTy = GprArgs[i].first;
    Operand *Arg =
        legalize(GprArgs[i].second, Legal_Default | Legal_Rematerializable);
    GprArgs[i].second = legalizeToReg(
        Arg, RegX8632::getRegisterForGprArgNum(
                 Arg->getType(), RegX8632::getArgIndex(GprArgIndices[i], i)));
    assert(SignatureTy == IceType_i64 || SignatureTy == IceType_i32);
    assert(SignatureTy == Arg->getType());
    (void)SignatureTy;
  }
  // Generate a FakeUse of register arguments so that they do not get dead code
  // eliminated as a result of the FakeKill of scratch registers after the call.
  // These need to be right before the call instruction.
  for (auto *Arg : XmmArgs) {
    Context.insert<InstFakeUse>(llvm::cast<Variable>(Arg));
  }
  for (auto &ArgPair : GprArgs) {
    Context.insert<InstFakeUse>(llvm::cast<Variable>(ArgPair.second));
  }
  // Generate the call instruction. Assign its result to a temporary with high
  // register allocation weight.
  // ReturnReg doubles as ReturnRegLo as necessary.
  Variable *ReturnReg = nullptr;
  Variable *ReturnRegHi = nullptr;
  if (Dest) {
    switch (DestTy) {
    case IceType_NUM:
    case IceType_void:
    case IceType_i1:
    case IceType_i8:
    case IceType_i16:
      llvm::report_fatal_error("Invalid Call dest type");
      break;
    case IceType_i32:
      ReturnReg = makeReg(DestTy, RegX8632::Reg_eax);
      break;
    case IceType_i64:
      ReturnReg = makeReg(IceType_i32, RegX8632::Reg_eax);
      ReturnRegHi = makeReg(IceType_i32, RegX8632::Reg_edx);
      break;
    case IceType_f32:
    case IceType_f64:
      // Leave ReturnReg==ReturnRegHi==nullptr, and capture the result with
      // the fstp instruction.
      break;
    // Fallthrough intended.
    case IceType_v4i1:
    case IceType_v8i1:
    case IceType_v16i1:
    case IceType_v16i8:
    case IceType_v8i16:
    case IceType_v4i32:
    case IceType_v4f32:
      ReturnReg = makeReg(DestTy, RegX8632::Reg_xmm0);
      break;
    }
  }
  // Emit the call to the function.
  Operand *CallTarget =
      legalize(Instr->getCallTarget(), Legal_Reg | Legal_Imm | Legal_AddrAbs);
  size_t NumVariadicFpArgs = Instr->isVariadic() ? XmmArgs.size() : 0;
  Inst *NewCall = emitCallToTarget(CallTarget, ReturnReg, NumVariadicFpArgs);
  // Keep the upper return register live on 32-bit platform.
  if (ReturnRegHi)
    Context.insert<InstFakeDef>(ReturnRegHi);
  // Mark the call as killing all the caller-save registers.
  Context.insert<InstFakeKill>(NewCall);
  // Handle x86-32 floating point returns.
  if (Dest != nullptr && isScalarFloatingType(DestTy)) {
    // Special treatment for an FP function which returns its result in st(0).
    // If Dest ends up being a physical xmm register, the fstp emit code will
    // route st(0) through the space reserved in the function argument area
    // we allocated.
    _fstp(Dest);
    // Create a fake use of Dest in case it actually isn't used, because st(0)
    // still needs to be popped.
    Context.insert<InstFakeUse>(Dest);
  }
  // Generate a FakeUse to keep the call live if necessary.
  if (Instr->hasSideEffects() && ReturnReg) {
    Context.insert<InstFakeUse>(ReturnReg);
  }
  // Process the return value, if any.
  if (Dest == nullptr)
    return;
  // Assign the result of the call to Dest.  Route it through a temporary so
  // that the local register availability peephole can be subsequently used.
  Variable *Tmp = nullptr;
  if (isVectorType(DestTy)) {
    assert(ReturnReg && "Vector type requires a return register");
    Tmp = makeReg(DestTy);
    _movp(Tmp, ReturnReg);
    _movp(Dest, Tmp);
  } else if (!isScalarFloatingType(DestTy)) {
    assert(isScalarIntegerType(DestTy));
    assert(ReturnReg && "Integer type requires a return register");
    if (DestTy == IceType_i64) {
      assert(ReturnRegHi && "64-bit type requires two return registers");
      auto *Dest64On32 = llvm::cast<Variable64On32>(Dest);
      Variable *DestLo = Dest64On32->getLo();
      Variable *DestHi = Dest64On32->getHi();
      _mov(Tmp, ReturnReg);
      _mov(DestLo, Tmp);
      Variable *TmpHi = nullptr;
      _mov(TmpHi, ReturnRegHi);
      _mov(DestHi, TmpHi);
    } else {
      _mov(Tmp, ReturnReg);
      _mov(Dest, Tmp);
    }
  }
}

void TargetX8632::lowerCast(const InstCast *Instr) {
  // a = cast(b) ==> t=cast(b); a=t; (link t->b, link a->t, no overlap)
  InstCast::OpKind CastKind = Instr->getCastKind();
  Variable *Dest = Instr->getDest();
  Type DestTy = Dest->getType();
  switch (CastKind) {
  default:
    Func->setError("Cast type not supported");
    return;
  case InstCast::Sext: {
    // Src0RM is the source operand legalized to physical register or memory,
    // but not immediate, since the relevant x86 native instructions don't
    // allow an immediate operand. If the operand is an immediate, we could
    // consider computing the strength-reduced result at translation time, but
    // we're unlikely to see something like that in the bitcode that the
    // optimizer wouldn't have already taken care of.
    Operand *Src0RM = legalize(Instr->getSrc(0), Legal_Reg | Legal_Mem);
    if (isVectorType(DestTy)) {
      if (DestTy == IceType_v16i8) {
        // onemask = materialize(1,1,...); dst = (src & onemask) > 0
        Variable *OneMask = makeVectorOfOnes(DestTy);
        Variable *T = makeReg(DestTy);
        _movp(T, Src0RM);
        _pand(T, OneMask);
        Variable *Zeros = makeVectorOfZeros(DestTy);
        _pcmpgt(T, Zeros);
        _movp(Dest, T);
      } else {
        /// width = width(elty) - 1; dest = (src << width) >> width
        SizeT ShiftAmount =
            X86_CHAR_BIT * typeWidthInBytes(typeElementType(DestTy)) - 1;
        Constant *ShiftConstant = Ctx->getConstantInt8(ShiftAmount);
        Variable *T = makeReg(DestTy);
        _movp(T, Src0RM);
        _psll(T, ShiftConstant);
        _psra(T, ShiftConstant);
        _movp(Dest, T);
      }
    } else if (DestTy == IceType_i64) {
      // t1=movsx src; t2=t1; t2=sar t2, 31; dst.lo=t1; dst.hi=t2
      Constant *Shift = Ctx->getConstantInt32(31);
      auto *DestLo = llvm::cast<Variable>(loOperand(Dest));
      auto *DestHi = llvm::cast<Variable>(hiOperand(Dest));
      Variable *T_Lo = makeReg(DestLo->getType());
      if (Src0RM->getType() == IceType_i32) {
        _mov(T_Lo, Src0RM);
      } else if (Src0RM->getType() == IceType_i1) {
        _movzx(T_Lo, Src0RM);
        _shl(T_Lo, Shift);
        _sar(T_Lo, Shift);
      } else {
        _movsx(T_Lo, Src0RM);
      }
      _mov(DestLo, T_Lo);
      Variable *T_Hi = nullptr;
      _mov(T_Hi, T_Lo);
      if (Src0RM->getType() != IceType_i1)
        // For i1, the sar instruction is already done above.
        _sar(T_Hi, Shift);
      _mov(DestHi, T_Hi);
    } else if (Src0RM->getType() == IceType_i1) {
      // t1 = src
      // shl t1, dst_bitwidth - 1
      // sar t1, dst_bitwidth - 1
      // dst = t1
      size_t DestBits = X86_CHAR_BIT * typeWidthInBytes(DestTy);
      Constant *ShiftAmount = Ctx->getConstantInt32(DestBits - 1);
      Variable *T = makeReg(DestTy);
      if (typeWidthInBytes(DestTy) <= typeWidthInBytes(Src0RM->getType())) {
        _mov(T, Src0RM);
      } else {
        // Widen the source using movsx or movzx. (It doesn't matter which one,
        // since the following shl/sar overwrite the bits.)
        _movzx(T, Src0RM);
      }
      _shl(T, ShiftAmount);
      _sar(T, ShiftAmount);
      _mov(Dest, T);
    } else {
      // t1 = movsx src; dst = t1
      Variable *T = makeReg(DestTy);
      _movsx(T, Src0RM);
      _mov(Dest, T);
    }
    break;
  }
  case InstCast::Zext: {
    Operand *Src0RM = legalize(Instr->getSrc(0), Legal_Reg | Legal_Mem);
    if (isVectorType(DestTy)) {
      // onemask = materialize(1,1,...); dest = onemask & src
      Variable *OneMask = makeVectorOfOnes(DestTy);
      Variable *T = makeReg(DestTy);
      _movp(T, Src0RM);
      _pand(T, OneMask);
      _movp(Dest, T);
    } else if (DestTy == IceType_i64) {
      // t1=movzx src; dst.lo=t1; dst.hi=0
      Constant *Zero = Ctx->getConstantZero(IceType_i32);
      auto *DestLo = llvm::cast<Variable>(loOperand(Dest));
      auto *DestHi = llvm::cast<Variable>(hiOperand(Dest));
      Variable *Tmp = makeReg(DestLo->getType());
      if (Src0RM->getType() == IceType_i32) {
        _mov(Tmp, Src0RM);
      } else {
        _movzx(Tmp, Src0RM);
      }
      _mov(DestLo, Tmp);
      _mov(DestHi, Zero);
    } else if (Src0RM->getType() == IceType_i1) {
      // t = Src0RM; Dest = t
      Variable *T = nullptr;
      if (DestTy == IceType_i8) {
        _mov(T, Src0RM);
      } else {
        assert(DestTy != IceType_i1);
        assert(DestTy != IceType_i64);
        // Use 32-bit for both 16-bit and 32-bit, since 32-bit ops are shorter.
        // In x86-64 we need to widen T to 64-bits to ensure that T -- if
        // written to the stack (i.e., in -Om1) will be fully zero-extended.
        T = makeReg(DestTy == IceType_i64 ? IceType_i64 : IceType_i32);
        _movzx(T, Src0RM);
      }
      _mov(Dest, T);
    } else {
      // t1 = movzx src; dst = t1
      Variable *T = makeReg(DestTy);
      _movzx(T, Src0RM);
      _mov(Dest, T);
    }
    break;
  }
  case InstCast::Trunc: {
    if (isVectorType(DestTy)) {
      // onemask = materialize(1,1,...); dst = src & onemask
      Operand *Src0RM = legalize(Instr->getSrc(0), Legal_Reg | Legal_Mem);
      Type Src0Ty = Src0RM->getType();
      Variable *OneMask = makeVectorOfOnes(Src0Ty);
      Variable *T = makeReg(DestTy);
      _movp(T, Src0RM);
      _pand(T, OneMask);
      _movp(Dest, T);
    } else if (DestTy == IceType_i1 || DestTy == IceType_i8) {
      // Make sure we truncate from and into valid registers.
      Operand *Src0 = legalizeUndef(Instr->getSrc(0));
      if (Src0->getType() == IceType_i64)
        Src0 = loOperand(Src0);
      Operand *Src0RM = legalize(Src0, Legal_Reg | Legal_Mem);
      Variable *T = copyToReg8(Src0RM);
      if (DestTy == IceType_i1)
        _and(T, Ctx->getConstantInt1(1));
      _mov(Dest, T);
    } else {
      Operand *Src0 = legalizeUndef(Instr->getSrc(0));
      if (Src0->getType() == IceType_i64)
        Src0 = loOperand(Src0);
      Operand *Src0RM = legalize(Src0, Legal_Reg | Legal_Mem);
      // t1 = trunc Src0RM; Dest = t1
      Variable *T = makeReg(DestTy);
      _mov(T, Src0RM);
      _mov(Dest, T);
    }
    break;
  }
  case InstCast::Fptrunc:
  case InstCast::Fpext: {
    Operand *Src0RM = legalize(Instr->getSrc(0), Legal_Reg | Legal_Mem);
    // t1 = cvt Src0RM; Dest = t1
    Variable *T = makeReg(DestTy);
    _cvt(T, Src0RM, Insts::Cvt::Float2float);
    _mov(Dest, T);
    break;
  }
  case InstCast::Fptosi:
    if (isVectorType(DestTy)) {
      assert(DestTy == IceType_v4i32);
      assert(Instr->getSrc(0)->getType() == IceType_v4f32);
      Operand *Src0R = legalizeToReg(Instr->getSrc(0));
      Variable *T = makeReg(DestTy);
      _cvt(T, Src0R, Insts::Cvt::Tps2dq);
      _movp(Dest, T);
    } else if (DestTy == IceType_i64) {
      llvm::report_fatal_error("Helper call was expected");
    } else {
      Operand *Src0RM = legalize(Instr->getSrc(0), Legal_Reg | Legal_Mem);
      // t1.i32 = cvt Src0RM; t2.dest_type = t1; Dest = t2.dest_type
      Variable *T_1 = nullptr;
      assert(DestTy != IceType_i64);
      T_1 = makeReg(IceType_i32);
      // cvt() requires its integer argument to be a GPR.
      Variable *T_2 = makeReg(DestTy);
      if (isByteSizedType(DestTy)) {
        assert(T_1->getType() == IceType_i32);
        T_1->setRegClass(RCX86_Is32To8);
        T_2->setRegClass(RCX86_IsTrunc8Rcvr);
      }
      _cvt(T_1, Src0RM, Insts::Cvt::Tss2si);
      _mov(T_2, T_1); // T_1 and T_2 may have different integer types
      if (DestTy == IceType_i1)
        _and(T_2, Ctx->getConstantInt1(1));
      _mov(Dest, T_2);
    }
    break;
  case InstCast::Fptoui:
    if (isVectorType(DestTy)) {
      llvm::report_fatal_error("Helper call was expected");
    } else if (DestTy == IceType_i64 || DestTy == IceType_i32) {
      llvm::report_fatal_error("Helper call was expected");
    } else {
      Operand *Src0RM = legalize(Instr->getSrc(0), Legal_Reg | Legal_Mem);
      // t1.i32 = cvt Src0RM; t2.dest_type = t1; Dest = t2.dest_type
      assert(DestTy != IceType_i64);
      Variable *T_1 = nullptr;
      assert(DestTy != IceType_i32);
      T_1 = makeReg(IceType_i32);
      Variable *T_2 = makeReg(DestTy);
      if (isByteSizedType(DestTy)) {
        assert(T_1->getType() == IceType_i32);
        T_1->setRegClass(RCX86_Is32To8);
        T_2->setRegClass(RCX86_IsTrunc8Rcvr);
      }
      _cvt(T_1, Src0RM, Insts::Cvt::Tss2si);
      _mov(T_2, T_1); // T_1 and T_2 may have different integer types
      if (DestTy == IceType_i1)
        _and(T_2, Ctx->getConstantInt1(1));
      _mov(Dest, T_2);
    }
    break;
  case InstCast::Sitofp:
    if (isVectorType(DestTy)) {
      assert(DestTy == IceType_v4f32);
      assert(Instr->getSrc(0)->getType() == IceType_v4i32);
      Operand *Src0R = legalizeToReg(Instr->getSrc(0));
      Variable *T = makeReg(DestTy);
      _cvt(T, Src0R, Insts::Cvt::Dq2ps);
      _movp(Dest, T);
    } else if (Instr->getSrc(0)->getType() == IceType_i64) {
      llvm::report_fatal_error("Helper call was expected");
    } else {
      Operand *Src0RM = legalize(Instr->getSrc(0), Legal_Reg | Legal_Mem);
      // Sign-extend the operand.
      // t1.i32 = movsx Src0RM; t2 = Cvt t1.i32; Dest = t2
      Variable *T_1 = nullptr;
      assert(Src0RM->getType() != IceType_i64);
      T_1 = makeReg(IceType_i32);
      Variable *T_2 = makeReg(DestTy);
      if (Src0RM->getType() == T_1->getType())
        _mov(T_1, Src0RM);
      else
        _movsx(T_1, Src0RM);
      _cvt(T_2, T_1, Insts::Cvt::Si2ss);
      _mov(Dest, T_2);
    }
    break;
  case InstCast::Uitofp: {
    Operand *Src0 = Instr->getSrc(0);
    if (isVectorType(Src0->getType())) {
      llvm::report_fatal_error("Helper call was expected");
    } else if (Src0->getType() == IceType_i64 ||
               Src0->getType() == IceType_i32) {
      llvm::report_fatal_error("Helper call was expected");
    } else {
      Operand *Src0RM = legalize(Src0, Legal_Reg | Legal_Mem);
      // Zero-extend the operand.
      // t1.i32 = movzx Src0RM; t2 = Cvt t1.i32; Dest = t2
      Variable *T_1 = nullptr;
      assert(Src0RM->getType() != IceType_i64);
      assert(Src0RM->getType() != IceType_i32);
      T_1 = makeReg(IceType_i32);
      Variable *T_2 = makeReg(DestTy);
      if (Src0RM->getType() == T_1->getType())
        _mov(T_1, Src0RM);
      else
        _movzx(T_1, Src0RM)->setMustKeep();
      _cvt(T_2, T_1, Insts::Cvt::Si2ss);
      _mov(Dest, T_2);
    }
    break;
  }
  case InstCast::Bitcast: {
    Operand *Src0 = Instr->getSrc(0);
    if (DestTy == Src0->getType()) {
      auto *Assign = InstAssign::create(Func, Dest, Src0);
      lowerAssign(Assign);
      return;
    }
    switch (DestTy) {
    default:
      llvm_unreachable("Unexpected Bitcast dest type");
    case IceType_i8: {
      llvm::report_fatal_error("Helper call was expected");
    } break;
    case IceType_i16: {
      llvm::report_fatal_error("Helper call was expected");
    } break;
    case IceType_i32:
    case IceType_f32: {
      Variable *Src0R = legalizeToReg(Src0);
      Variable *T = makeReg(DestTy);
      _movd(T, Src0R);
      _mov(Dest, T);
    } break;
    case IceType_i64: {
      assert(Src0->getType() == IceType_f64);
      Operand *Src0RM = legalize(Src0, Legal_Reg | Legal_Mem);
      // a.i64 = bitcast b.f64 ==>
      //   s.f64 = spill b.f64
      //   t_lo.i32 = lo(s.f64)
      //   a_lo.i32 = t_lo.i32
      //   t_hi.i32 = hi(s.f64)
      //   a_hi.i32 = t_hi.i32
      Operand *SpillLo, *SpillHi;
      if (auto *Src0Var = llvm::dyn_cast<Variable>(Src0RM)) {
        Variable *Spill = Func->makeVariable(IceType_f64);
        Spill->setLinkedTo(Src0Var);
        Spill->setMustNotHaveReg();
        _movq(Spill, Src0RM);
        SpillLo = VariableSplit::create(Func, Spill, VariableSplit::Low);
        SpillHi = VariableSplit::create(Func, Spill, VariableSplit::High);
      } else {
        SpillLo = loOperand(Src0RM);
        SpillHi = hiOperand(Src0RM);
      }

      auto *DestLo = llvm::cast<Variable>(loOperand(Dest));
      auto *DestHi = llvm::cast<Variable>(hiOperand(Dest));
      Variable *T_Lo = makeReg(IceType_i32);
      Variable *T_Hi = makeReg(IceType_i32);

      _mov(T_Lo, SpillLo);
      _mov(DestLo, T_Lo);
      _mov(T_Hi, SpillHi);
      _mov(DestHi, T_Hi);
    } break;
    case IceType_f64: {
      assert(Src0->getType() == IceType_i64);
      Src0 = legalize(Src0);
      if (llvm::isa<X86OperandMem>(Src0)) {
        Variable *T = makeReg(DestTy);
        _movq(T, Src0);
        _movq(Dest, T);
        break;
      }
      // a.f64 = bitcast b.i64 ==>
      //   t_lo.i32 = b_lo.i32
      //   FakeDef(s.f64)
      //   lo(s.f64) = t_lo.i32
      //   t_hi.i32 = b_hi.i32
      //   hi(s.f64) = t_hi.i32
      //   a.f64 = s.f64
      Variable *Spill = Func->makeVariable(IceType_f64);
      Spill->setLinkedTo(Dest);
      Spill->setMustNotHaveReg();

      Variable *T_Lo = nullptr, *T_Hi = nullptr;
      auto *SpillLo = VariableSplit::create(Func, Spill, VariableSplit::Low);
      auto *SpillHi = VariableSplit::create(Func, Spill, VariableSplit::High);
      _mov(T_Lo, loOperand(Src0));
      // Technically, the Spill is defined after the _store happens, but
      // SpillLo is considered a "use" of Spill so define Spill before it is
      // used.
      Context.insert<InstFakeDef>(Spill);
      _store(T_Lo, SpillLo);
      _mov(T_Hi, hiOperand(Src0));
      _store(T_Hi, SpillHi);
      _movq(Dest, Spill);
    } break;
    case IceType_v8i1: {
      llvm::report_fatal_error("Helper call was expected");
    } break;
    case IceType_v16i1: {
      llvm::report_fatal_error("Helper call was expected");
    } break;
    case IceType_v8i16:
    case IceType_v16i8:
    case IceType_v4i32:
    case IceType_v4f32: {
      if (Src0->getType() == IceType_i32) {
        // Bitcast requires equal type sizes, which isn't strictly the case
        // between scalars and vectors, but to emulate v4i8 vectors one has to
        // use v16i8 vectors.
        Operand *Src0RM = legalize(Src0, Legal_Reg | Legal_Mem);
        Variable *T = makeReg(DestTy);
        _movd(T, Src0RM);
        _mov(Dest, T);
      } else {
        _movp(Dest, legalizeToReg(Src0));
      }
    } break;
    }
    break;
  }
  }
}

void TargetX8632::lowerExtractElement(const InstExtractElement *Instr) {
  Operand *SourceVectNotLegalized = Instr->getSrc(0);
  auto *ElementIndex = llvm::dyn_cast<ConstantInteger32>(Instr->getSrc(1));
  // Only constant indices are allowed in PNaCl IR.
  assert(ElementIndex);

  unsigned Index = ElementIndex->getValue();
  Type Ty = SourceVectNotLegalized->getType();
  Type ElementTy = typeElementType(Ty);
  Type InVectorElementTy = InstX86Base::getInVectorElementType(Ty);

  // TODO(wala): Determine the best lowering sequences for each type.
  bool CanUsePextr = Ty == IceType_v8i16 || Ty == IceType_v8i1 ||
                     (InstructionSet >= SSE4_1 && Ty != IceType_v4f32);
  Variable *ExtractedElementR =
      makeReg(CanUsePextr ? IceType_i32 : InVectorElementTy);
  if (CanUsePextr) {
    // Use pextrb, pextrw, or pextrd.  The "b" and "w" versions clear the upper
    // bits of the destination register, so we represent this by always
    // extracting into an i32 register.  The _mov into Dest below will do
    // truncation as necessary.
    Constant *Mask = Ctx->getConstantInt32(Index);
    Variable *SourceVectR = legalizeToReg(SourceVectNotLegalized);
    _pextr(ExtractedElementR, SourceVectR, Mask);
  } else if (Ty == IceType_v4i32 || Ty == IceType_v4f32 || Ty == IceType_v4i1) {
    // Use pshufd and movd/movss.
    Variable *T = nullptr;
    if (Index) {
      // The shuffle only needs to occur if the element to be extracted is not
      // at the lowest index.
      Constant *Mask = Ctx->getConstantInt32(Index);
      T = makeReg(Ty);
      _pshufd(T, legalize(SourceVectNotLegalized, Legal_Reg | Legal_Mem), Mask);
    } else {
      T = legalizeToReg(SourceVectNotLegalized);
    }

    if (InVectorElementTy == IceType_i32) {
      _movd(ExtractedElementR, T);
    } else { // Ty == IceType_f32
      // TODO(wala): _movss is only used here because _mov does not allow a
      // vector source and a scalar destination.  _mov should be able to be
      // used here.
      // _movss is a binary instruction, so the FakeDef is needed to keep the
      // live range analysis consistent.
      Context.insert<InstFakeDef>(ExtractedElementR);
      _movss(ExtractedElementR, T);
    }
  } else {
    assert(Ty == IceType_v16i8 || Ty == IceType_v16i1);
    // Spill the value to a stack slot and do the extraction in memory.
    //
    // TODO(wala): use legalize(SourceVectNotLegalized, Legal_Mem) when support
    // for legalizing to mem is implemented.
    Variable *Slot = Func->makeVariable(Ty);
    Slot->setMustNotHaveReg();
    _movp(Slot, legalizeToReg(SourceVectNotLegalized));

    // Compute the location of the element in memory.
    unsigned Offset = Index * typeWidthInBytes(InVectorElementTy);
    X86OperandMem *Loc =
        getMemoryOperandForStackSlot(InVectorElementTy, Slot, Offset);
    _mov(ExtractedElementR, Loc);
  }

  if (ElementTy == IceType_i1) {
    // Truncate extracted integers to i1s if necessary.
    Variable *T = makeReg(IceType_i1);
    InstCast *Cast =
        InstCast::create(Func, InstCast::Trunc, T, ExtractedElementR);
    lowerCast(Cast);
    ExtractedElementR = T;
  }

  // Copy the element to the destination.
  Variable *Dest = Instr->getDest();
  _mov(Dest, ExtractedElementR);
}

void TargetX8632::lowerFcmp(const InstFcmp *Fcmp) {
  Variable *Dest = Fcmp->getDest();

  if (isVectorType(Dest->getType())) {
    lowerFcmpVector(Fcmp);
  } else {
    constexpr Inst *Consumer = nullptr;
    lowerFcmpAndConsumer(Fcmp, Consumer);
  }
}

void TargetX8632::lowerFcmpAndConsumer(const InstFcmp *Fcmp,
                                       const Inst *Consumer) {
  Operand *Src0 = Fcmp->getSrc(0);
  Operand *Src1 = Fcmp->getSrc(1);
  Variable *Dest = Fcmp->getDest();

  if (Consumer != nullptr) {
    if (auto *Select = llvm::dyn_cast<InstSelect>(Consumer)) {
      if (lowerOptimizeFcmpSelect(Fcmp, Select))
        return;
    }
  }

  if (isVectorType(Dest->getType())) {
    lowerFcmp(Fcmp);
    if (Consumer != nullptr)
      lowerSelectVector(llvm::cast<InstSelect>(Consumer));
    return;
  }

  // Lowering a = fcmp cond, b, c
  //   ucomiss b, c       /* only if C1 != Br_None */
  //                      /* but swap b,c order if SwapOperands==true */
  //   mov a, <default>
  //   j<C1> label        /* only if C1 != Br_None */
  //   j<C2> label        /* only if C2 != Br_None */
  //   FakeUse(a)         /* only if C1 != Br_None */
  //   mov a, !<default>  /* only if C1 != Br_None */
  //   label:             /* only if C1 != Br_None */
  //
  // setcc lowering when C1 != Br_None && C2 == Br_None:
  //   ucomiss b, c       /* but swap b,c order if SwapOperands==true */
  //   setcc a, C1
  InstFcmp::FCond Condition = Fcmp->getCondition();
  assert(static_cast<size_t>(Condition) < TableFcmpSize);
  if (TableFcmp[Condition].SwapScalarOperands)
    std::swap(Src0, Src1);
  const bool HasC1 = (TableFcmp[Condition].C1 != CondX86::Br_None);
  const bool HasC2 = (TableFcmp[Condition].C2 != CondX86::Br_None);
  if (HasC1) {
    Src0 = legalize(Src0);
    Operand *Src1RM = legalize(Src1, Legal_Reg | Legal_Mem);
    Variable *T = nullptr;
    _mov(T, Src0);
    _ucomiss(T, Src1RM);
    if (!HasC2) {
      assert(TableFcmp[Condition].Default);
      setccOrConsumer(TableFcmp[Condition].C1, Dest, Consumer);
      return;
    }
  }
  int32_t IntDefault = TableFcmp[Condition].Default;
  if (Consumer == nullptr) {
    Constant *Default = Ctx->getConstantInt(Dest->getType(), IntDefault);
    _mov(Dest, Default);
    if (HasC1) {
      InstX86Label *Label = InstX86Label::create(Func, this);
      _br(TableFcmp[Condition].C1, Label);
      if (HasC2) {
        _br(TableFcmp[Condition].C2, Label);
      }
      Constant *NonDefault = Ctx->getConstantInt(Dest->getType(), !IntDefault);
      _redefined(_mov(Dest, NonDefault));
      Context.insert(Label);
    }
    return;
  }
  if (const auto *Br = llvm::dyn_cast<InstBr>(Consumer)) {
    CfgNode *TrueSucc = Br->getTargetTrue();
    CfgNode *FalseSucc = Br->getTargetFalse();
    if (IntDefault != 0)
      std::swap(TrueSucc, FalseSucc);
    if (HasC1) {
      _br(TableFcmp[Condition].C1, FalseSucc);
      if (HasC2) {
        _br(TableFcmp[Condition].C2, FalseSucc);
      }
      _br(TrueSucc);
      return;
    }
    _br(FalseSucc);
    return;
  }
  if (auto *Select = llvm::dyn_cast<InstSelect>(Consumer)) {
    Operand *SrcT = Select->getTrueOperand();
    Operand *SrcF = Select->getFalseOperand();
    Variable *SelectDest = Select->getDest();
    if (IntDefault != 0)
      std::swap(SrcT, SrcF);
    lowerMove(SelectDest, SrcF, false);
    if (HasC1) {
      InstX86Label *Label = InstX86Label::create(Func, this);
      _br(TableFcmp[Condition].C1, Label);
      if (HasC2) {
        _br(TableFcmp[Condition].C2, Label);
      }
      static constexpr bool IsRedefinition = true;
      lowerMove(SelectDest, SrcT, IsRedefinition);
      Context.insert(Label);
    }
    return;
  }
  llvm::report_fatal_error("Unexpected consumer type");
}

void TargetX8632::lowerFcmpVector(const InstFcmp *Fcmp) {
  Operand *Src0 = Fcmp->getSrc(0);
  Operand *Src1 = Fcmp->getSrc(1);
  Variable *Dest = Fcmp->getDest();

  if (!isVectorType(Dest->getType()))
    llvm::report_fatal_error("Expected vector compare");

  InstFcmp::FCond Condition = Fcmp->getCondition();
  assert(static_cast<size_t>(Condition) < TableFcmpSize);

  if (TableFcmp[Condition].SwapVectorOperands)
    std::swap(Src0, Src1);

  Variable *T = nullptr;

  if (Condition == InstFcmp::True) {
    // makeVectorOfOnes() requires an integer vector type.
    T = makeVectorOfMinusOnes(IceType_v4i32);
  } else if (Condition == InstFcmp::False) {
    T = makeVectorOfZeros(Dest->getType());
  } else {
    Operand *Src0RM = legalize(Src0, Legal_Reg | Legal_Mem);
    Operand *Src1RM = legalize(Src1, Legal_Reg | Legal_Mem);
    if (llvm::isa<X86OperandMem>(Src1RM))
      Src1RM = legalizeToReg(Src1RM);

    switch (Condition) {
    default: {
      const CmppsCond Predicate = TableFcmp[Condition].Predicate;
      assert(Predicate != CondX86::Cmpps_Invalid);
      T = makeReg(Src0RM->getType());
      _movp(T, Src0RM);
      _cmpps(T, Src1RM, Predicate);
    } break;
    case InstFcmp::One: {
      // Check both unequal and ordered.
      T = makeReg(Src0RM->getType());
      Variable *T2 = makeReg(Src0RM->getType());
      _movp(T, Src0RM);
      _cmpps(T, Src1RM, CondX86::Cmpps_neq);
      _movp(T2, Src0RM);
      _cmpps(T2, Src1RM, CondX86::Cmpps_ord);
      _pand(T, T2);
    } break;
    case InstFcmp::Ueq: {
      // Check both equal or unordered.
      T = makeReg(Src0RM->getType());
      Variable *T2 = makeReg(Src0RM->getType());
      _movp(T, Src0RM);
      _cmpps(T, Src1RM, CondX86::Cmpps_eq);
      _movp(T2, Src0RM);
      _cmpps(T2, Src1RM, CondX86::Cmpps_unord);
      _por(T, T2);
    } break;
    }
  }

  assert(T != nullptr);
  _movp(Dest, T);
  eliminateNextVectorSextInstruction(Dest);
}

inline bool isZero(const Operand *Opnd) {
  if (auto *C64 = llvm::dyn_cast<ConstantInteger64>(Opnd))
    return C64->getValue() == 0;
  if (auto *C32 = llvm::dyn_cast<ConstantInteger32>(Opnd))
    return C32->getValue() == 0;
  return false;
}

void TargetX8632::lowerIcmpAndConsumer(const InstIcmp *Icmp,
                                       const Inst *Consumer) {
  Operand *Src0 = legalize(Icmp->getSrc(0));
  Operand *Src1 = legalize(Icmp->getSrc(1));
  Variable *Dest = Icmp->getDest();

  if (isVectorType(Dest->getType())) {
    lowerIcmp(Icmp);
    if (Consumer != nullptr)
      lowerSelectVector(llvm::cast<InstSelect>(Consumer));
    return;
  }

  if (Src0->getType() == IceType_i64) {
    lowerIcmp64(Icmp, Consumer);
    return;
  }

  // cmp b, c
  if (isZero(Src1)) {
    switch (Icmp->getCondition()) {
    default:
      break;
    case InstIcmp::Uge:
      movOrConsumer(true, Dest, Consumer);
      return;
    case InstIcmp::Ult:
      movOrConsumer(false, Dest, Consumer);
      return;
    }
  }
  Operand *Src0RM = legalizeSrc0ForCmp(Src0, Src1);
  _cmp(Src0RM, Src1);
  setccOrConsumer(getIcmp32Mapping(Icmp->getCondition()), Dest, Consumer);
}

void TargetX8632::lowerIcmpVector(const InstIcmp *Icmp) {
  Operand *Src0 = legalize(Icmp->getSrc(0));
  Operand *Src1 = legalize(Icmp->getSrc(1));
  Variable *Dest = Icmp->getDest();

  if (!isVectorType(Dest->getType()))
    llvm::report_fatal_error("Expected a vector compare");

  Type Ty = Src0->getType();
  // Promote i1 vectors to 128 bit integer vector types.
  if (typeElementType(Ty) == IceType_i1) {
    Type NewTy = IceType_NUM;
    switch (Ty) {
    default:
      llvm::report_fatal_error("unexpected type");
      break;
    case IceType_v4i1:
      NewTy = IceType_v4i32;
      break;
    case IceType_v8i1:
      NewTy = IceType_v8i16;
      break;
    case IceType_v16i1:
      NewTy = IceType_v16i8;
      break;
    }
    Variable *NewSrc0 = Func->makeVariable(NewTy);
    Variable *NewSrc1 = Func->makeVariable(NewTy);
    lowerCast(InstCast::create(Func, InstCast::Sext, NewSrc0, Src0));
    lowerCast(InstCast::create(Func, InstCast::Sext, NewSrc1, Src1));
    Src0 = NewSrc0;
    Src1 = NewSrc1;
    Ty = NewTy;
  }

  InstIcmp::ICond Condition = Icmp->getCondition();

  Operand *Src0RM = legalize(Src0, Legal_Reg | Legal_Mem);
  Operand *Src1RM = legalize(Src1, Legal_Reg | Legal_Mem);

  // SSE2 only has signed comparison operations. Transform unsigned inputs in
  // a manner that allows for the use of signed comparison operations by
  // flipping the high order bits.
  if (Condition == InstIcmp::Ugt || Condition == InstIcmp::Uge ||
      Condition == InstIcmp::Ult || Condition == InstIcmp::Ule) {
    Variable *T0 = makeReg(Ty);
    Variable *T1 = makeReg(Ty);
    Variable *HighOrderBits = makeVectorOfHighOrderBits(Ty);
    _movp(T0, Src0RM);
    _pxor(T0, HighOrderBits);
    _movp(T1, Src1RM);
    _pxor(T1, HighOrderBits);
    Src0RM = T0;
    Src1RM = T1;
  }

  Variable *T = makeReg(Ty);
  switch (Condition) {
  default:
    llvm_unreachable("unexpected condition");
    break;
  case InstIcmp::Eq: {
    if (llvm::isa<X86OperandMem>(Src1RM))
      Src1RM = legalizeToReg(Src1RM);
    _movp(T, Src0RM);
    _pcmpeq(T, Src1RM);
  } break;
  case InstIcmp::Ne: {
    if (llvm::isa<X86OperandMem>(Src1RM))
      Src1RM = legalizeToReg(Src1RM);
    _movp(T, Src0RM);
    _pcmpeq(T, Src1RM);
    Variable *MinusOne = makeVectorOfMinusOnes(Ty);
    _pxor(T, MinusOne);
  } break;
  case InstIcmp::Ugt:
  case InstIcmp::Sgt: {
    if (llvm::isa<X86OperandMem>(Src1RM))
      Src1RM = legalizeToReg(Src1RM);
    _movp(T, Src0RM);
    _pcmpgt(T, Src1RM);
  } break;
  case InstIcmp::Uge:
  case InstIcmp::Sge: {
    // !(Src1RM > Src0RM)
    if (llvm::isa<X86OperandMem>(Src0RM))
      Src0RM = legalizeToReg(Src0RM);
    _movp(T, Src1RM);
    _pcmpgt(T, Src0RM);
    Variable *MinusOne = makeVectorOfMinusOnes(Ty);
    _pxor(T, MinusOne);
  } break;
  case InstIcmp::Ult:
  case InstIcmp::Slt: {
    if (llvm::isa<X86OperandMem>(Src0RM))
      Src0RM = legalizeToReg(Src0RM);
    _movp(T, Src1RM);
    _pcmpgt(T, Src0RM);
  } break;
  case InstIcmp::Ule:
  case InstIcmp::Sle: {
    // !(Src0RM > Src1RM)
    if (llvm::isa<X86OperandMem>(Src1RM))
      Src1RM = legalizeToReg(Src1RM);
    _movp(T, Src0RM);
    _pcmpgt(T, Src1RM);
    Variable *MinusOne = makeVectorOfMinusOnes(Ty);
    _pxor(T, MinusOne);
  } break;
  }

  _movp(Dest, T);
  eliminateNextVectorSextInstruction(Dest);
}

void TargetX8632::lowerIcmp64(const InstIcmp *Icmp, const Inst *Consumer) {
  // a=icmp cond, b, c ==> cmp b,c; a=1; br cond,L1; FakeUse(a); a=0; L1:
  Operand *Src0 = legalize(Icmp->getSrc(0));
  Operand *Src1 = legalize(Icmp->getSrc(1));
  Variable *Dest = Icmp->getDest();
  InstIcmp::ICond Condition = Icmp->getCondition();
  assert(static_cast<size_t>(Condition) < TableIcmp64Size);
  Operand *Src0LoRM = nullptr;
  Operand *Src0HiRM = nullptr;
  // Legalize the portions of Src0 that are going to be needed.
  if (isZero(Src1)) {
    switch (Condition) {
    default:
      llvm_unreachable("unexpected condition");
      break;
    // These two are not optimized, so we fall through to the general case,
    // which needs the upper and lower halves legalized.
    case InstIcmp::Sgt:
    case InstIcmp::Sle:
    // These four compare after performing an "or" of the high and low half, so
    // they need the upper and lower halves legalized.
    case InstIcmp::Eq:
    case InstIcmp::Ule:
    case InstIcmp::Ne:
    case InstIcmp::Ugt:
      Src0LoRM = legalize(loOperand(Src0), Legal_Reg | Legal_Mem);
    // These two test only the high half's sign bit, so they need only
    // the upper half legalized.
    case InstIcmp::Sge:
    case InstIcmp::Slt:
      Src0HiRM = legalize(hiOperand(Src0), Legal_Reg | Legal_Mem);
      break;

    // These two move constants and hence need no legalization.
    case InstIcmp::Uge:
    case InstIcmp::Ult:
      break;
    }
  } else {
    Src0LoRM = legalize(loOperand(Src0), Legal_Reg | Legal_Mem);
    Src0HiRM = legalize(hiOperand(Src0), Legal_Reg | Legal_Mem);
  }
  // Optimize comparisons with zero.
  if (isZero(Src1)) {
    Constant *SignMask = Ctx->getConstantInt32(0x80000000);
    Variable *Temp = nullptr;
    switch (Condition) {
    default:
      llvm_unreachable("unexpected condition");
      break;
    case InstIcmp::Eq:
    case InstIcmp::Ule:
      // Mov Src0HiRM first, because it was legalized most recently, and will
      // sometimes avoid a move before the OR.
      _mov(Temp, Src0HiRM);
      _or(Temp, Src0LoRM);
      Context.insert<InstFakeUse>(Temp);
      setccOrConsumer(CondX86::Br_e, Dest, Consumer);
      return;
    case InstIcmp::Ne:
    case InstIcmp::Ugt:
      // Mov Src0HiRM first, because it was legalized most recently, and will
      // sometimes avoid a move before the OR.
      _mov(Temp, Src0HiRM);
      _or(Temp, Src0LoRM);
      Context.insert<InstFakeUse>(Temp);
      setccOrConsumer(CondX86::Br_ne, Dest, Consumer);
      return;
    case InstIcmp::Uge:
      movOrConsumer(true, Dest, Consumer);
      return;
    case InstIcmp::Ult:
      movOrConsumer(false, Dest, Consumer);
      return;
    case InstIcmp::Sgt:
      break;
    case InstIcmp::Sge:
      _test(Src0HiRM, SignMask);
      setccOrConsumer(CondX86::Br_e, Dest, Consumer);
      return;
    case InstIcmp::Slt:
      _test(Src0HiRM, SignMask);
      setccOrConsumer(CondX86::Br_ne, Dest, Consumer);
      return;
    case InstIcmp::Sle:
      break;
    }
  }
  // Handle general compares.
  Operand *Src1LoRI = legalize(loOperand(Src1), Legal_Reg | Legal_Imm);
  Operand *Src1HiRI = legalize(hiOperand(Src1), Legal_Reg | Legal_Imm);
  if (Consumer == nullptr) {
    Constant *Zero = Ctx->getConstantInt(Dest->getType(), 0);
    Constant *One = Ctx->getConstantInt(Dest->getType(), 1);
    InstX86Label *LabelFalse = InstX86Label::create(Func, this);
    InstX86Label *LabelTrue = InstX86Label::create(Func, this);
    _mov(Dest, One);
    _cmp(Src0HiRM, Src1HiRI);
    if (TableIcmp64[Condition].C1 != CondX86::Br_None)
      _br(TableIcmp64[Condition].C1, LabelTrue);
    if (TableIcmp64[Condition].C2 != CondX86::Br_None)
      _br(TableIcmp64[Condition].C2, LabelFalse);
    _cmp(Src0LoRM, Src1LoRI);
    _br(TableIcmp64[Condition].C3, LabelTrue);
    Context.insert(LabelFalse);
    _redefined(_mov(Dest, Zero));
    Context.insert(LabelTrue);
    return;
  }
  if (const auto *Br = llvm::dyn_cast<InstBr>(Consumer)) {
    _cmp(Src0HiRM, Src1HiRI);
    if (TableIcmp64[Condition].C1 != CondX86::Br_None)
      _br(TableIcmp64[Condition].C1, Br->getTargetTrue());
    if (TableIcmp64[Condition].C2 != CondX86::Br_None)
      _br(TableIcmp64[Condition].C2, Br->getTargetFalse());
    _cmp(Src0LoRM, Src1LoRI);
    _br(TableIcmp64[Condition].C3, Br->getTargetTrue(), Br->getTargetFalse());
    return;
  }
  if (auto *Select = llvm::dyn_cast<InstSelect>(Consumer)) {
    Operand *SrcT = Select->getTrueOperand();
    Operand *SrcF = Select->getFalseOperand();
    Variable *SelectDest = Select->getDest();
    InstX86Label *LabelFalse = InstX86Label::create(Func, this);
    InstX86Label *LabelTrue = InstX86Label::create(Func, this);
    lowerMove(SelectDest, SrcT, false);
    _cmp(Src0HiRM, Src1HiRI);
    if (TableIcmp64[Condition].C1 != CondX86::Br_None)
      _br(TableIcmp64[Condition].C1, LabelTrue);
    if (TableIcmp64[Condition].C2 != CondX86::Br_None)
      _br(TableIcmp64[Condition].C2, LabelFalse);
    _cmp(Src0LoRM, Src1LoRI);
    _br(TableIcmp64[Condition].C3, LabelTrue);
    Context.insert(LabelFalse);
    static constexpr bool IsRedefinition = true;
    lowerMove(SelectDest, SrcF, IsRedefinition);
    Context.insert(LabelTrue);
    return;
  }
  llvm::report_fatal_error("Unexpected consumer type");
}

void TargetX8632::setccOrConsumer(BrCond Condition, Variable *Dest,
                                  const Inst *Consumer) {
  if (Consumer == nullptr) {
    _setcc(Dest, Condition);
    return;
  }
  if (const auto *Br = llvm::dyn_cast<InstBr>(Consumer)) {
    _br(Condition, Br->getTargetTrue(), Br->getTargetFalse());
    return;
  }
  if (const auto *Select = llvm::dyn_cast<InstSelect>(Consumer)) {
    Operand *SrcT = Select->getTrueOperand();
    Operand *SrcF = Select->getFalseOperand();
    Variable *SelectDest = Select->getDest();
    lowerSelectMove(SelectDest, Condition, SrcT, SrcF);
    return;
  }
  llvm::report_fatal_error("Unexpected consumer type");
}

void TargetX8632::movOrConsumer(bool IcmpResult, Variable *Dest,
                                const Inst *Consumer) {
  if (Consumer == nullptr) {
    _mov(Dest, Ctx->getConstantInt(Dest->getType(), (IcmpResult ? 1 : 0)));
    return;
  }
  if (const auto *Br = llvm::dyn_cast<InstBr>(Consumer)) {
    // TODO(sehr,stichnot): This could be done with a single unconditional
    // branch instruction, but subzero doesn't know how to handle the resulting
    // control flow graph changes now.  Make it do so to eliminate mov and cmp.
    _mov(Dest, Ctx->getConstantInt(Dest->getType(), (IcmpResult ? 1 : 0)));
    _cmp(Dest, Ctx->getConstantInt(Dest->getType(), 0));
    _br(CondX86::Br_ne, Br->getTargetTrue(), Br->getTargetFalse());
    return;
  }
  if (const auto *Select = llvm::dyn_cast<InstSelect>(Consumer)) {
    Operand *Src = nullptr;
    if (IcmpResult) {
      Src = legalize(Select->getTrueOperand(), Legal_Reg | Legal_Imm);
    } else {
      Src = legalize(Select->getFalseOperand(), Legal_Reg | Legal_Imm);
    }
    Variable *SelectDest = Select->getDest();
    lowerMove(SelectDest, Src, false);
    return;
  }
  llvm::report_fatal_error("Unexpected consumer type");
}

void TargetX8632::lowerArithAndConsumer(const InstArithmetic *Arith,
                                        const Inst *Consumer) {
  Variable *T = nullptr;
  Operand *Src0 = legalize(Arith->getSrc(0));
  Operand *Src1 = legalize(Arith->getSrc(1));
  Variable *Dest = Arith->getDest();
  switch (Arith->getOp()) {
  default:
    llvm_unreachable("arithmetic operator not AND or OR");
    break;
  case InstArithmetic::And:
    _mov(T, Src0);
    // Test cannot have an address in the second position.  Since T is
    // guaranteed to be a register and Src1 could be a memory load, ensure
    // that the second argument is a register.
    if (llvm::isa<Constant>(Src1))
      _test(T, Src1);
    else
      _test(Src1, T);
    break;
  case InstArithmetic::Or:
    _mov(T, Src0);
    _or(T, Src1);
    break;
  }

  if (Consumer == nullptr) {
    llvm::report_fatal_error("Expected a consumer instruction");
  }
  if (const auto *Br = llvm::dyn_cast<InstBr>(Consumer)) {
    Context.insert<InstFakeUse>(T);
    Context.insert<InstFakeDef>(Dest);
    _br(CondX86::Br_ne, Br->getTargetTrue(), Br->getTargetFalse());
    return;
  }
  llvm::report_fatal_error("Unexpected consumer type");
}

void TargetX8632::lowerInsertElement(const InstInsertElement *Instr) {
  Operand *SourceVectNotLegalized = Instr->getSrc(0);
  Operand *ElementToInsertNotLegalized = Instr->getSrc(1);
  auto *ElementIndex = llvm::dyn_cast<ConstantInteger32>(Instr->getSrc(2));
  // Only constant indices are allowed in PNaCl IR.
  assert(ElementIndex);
  unsigned Index = ElementIndex->getValue();
  assert(Index < typeNumElements(SourceVectNotLegalized->getType()));

  Type Ty = SourceVectNotLegalized->getType();
  Type ElementTy = typeElementType(Ty);
  Type InVectorElementTy = InstX86Base::getInVectorElementType(Ty);

  if (ElementTy == IceType_i1) {
    // Expand the element to the appropriate size for it to be inserted in the
    // vector.
    Variable *Expanded = Func->makeVariable(InVectorElementTy);
    auto *Cast = InstCast::create(Func, InstCast::Zext, Expanded,
                                  ElementToInsertNotLegalized);
    lowerCast(Cast);
    ElementToInsertNotLegalized = Expanded;
  }

  if (Ty == IceType_v8i16 || Ty == IceType_v8i1 || InstructionSet >= SSE4_1) {
    // Use insertps, pinsrb, pinsrw, or pinsrd.
    Operand *ElementRM =
        legalize(ElementToInsertNotLegalized, Legal_Reg | Legal_Mem);
    Operand *SourceVectRM =
        legalize(SourceVectNotLegalized, Legal_Reg | Legal_Mem);
    Variable *T = makeReg(Ty);
    _movp(T, SourceVectRM);
    if (Ty == IceType_v4f32) {
      _insertps(T, ElementRM, Ctx->getConstantInt32(Index << 4));
    } else {
      // For the pinsrb and pinsrw instructions, when the source operand is a
      // register, it must be a full r32 register like eax, and not ax/al/ah.
      // For filetype=asm, InstX86Pinsr::emit() compensates for
      // the use
      // of r16 and r8 by converting them through getBaseReg(), while emitIAS()
      // validates that the original and base register encodings are the same.
      if (ElementRM->getType() == IceType_i8 &&
          llvm::isa<Variable>(ElementRM)) {
        // Don't use ah/bh/ch/dh for pinsrb.
        ElementRM = copyToReg8(ElementRM);
      }
      _pinsr(T, ElementRM, Ctx->getConstantInt32(Index));
    }
    _movp(Instr->getDest(), T);
  } else if (Ty == IceType_v4i32 || Ty == IceType_v4f32 || Ty == IceType_v4i1) {
    // Use shufps or movss.
    Variable *ElementR = nullptr;
    Operand *SourceVectRM =
        legalize(SourceVectNotLegalized, Legal_Reg | Legal_Mem);

    if (InVectorElementTy == IceType_f32) {
      // ElementR will be in an XMM register since it is floating point.
      ElementR = legalizeToReg(ElementToInsertNotLegalized);
    } else {
      // Copy an integer to an XMM register.
      Operand *T = legalize(ElementToInsertNotLegalized, Legal_Reg | Legal_Mem);
      ElementR = makeReg(Ty);
      _movd(ElementR, T);
    }

    if (Index == 0) {
      Variable *T = makeReg(Ty);
      _movp(T, SourceVectRM);
      _movss(T, ElementR);
      _movp(Instr->getDest(), T);
      return;
    }

    // shufps treats the source and destination operands as vectors of four
    // doublewords. The destination's two high doublewords are selected from
    // the source operand and the two low doublewords are selected from the
    // (original value of) the destination operand. An insertelement operation
    // can be effected with a sequence of two shufps operations with
    // appropriate masks. In all cases below, Element[0] is being inserted into
    // SourceVectOperand. Indices are ordered from left to right.
    //
    // insertelement into index 1 (result is stored in ElementR):
    //   ElementR := ElementR[0, 0] SourceVectRM[0, 0]
    //   ElementR := ElementR[3, 0] SourceVectRM[2, 3]
    //
    // insertelement into index 2 (result is stored in T):
    //   T := SourceVectRM
    //   ElementR := ElementR[0, 0] T[0, 3]
    //   T := T[0, 1] ElementR[0, 3]
    //
    // insertelement into index 3 (result is stored in T):
    //   T := SourceVectRM
    //   ElementR := ElementR[0, 0] T[0, 2]
    //   T := T[0, 1] ElementR[3, 0]
    const unsigned char Mask1[3] = {0, 192, 128};
    const unsigned char Mask2[3] = {227, 196, 52};

    Constant *Mask1Constant = Ctx->getConstantInt32(Mask1[Index - 1]);
    Constant *Mask2Constant = Ctx->getConstantInt32(Mask2[Index - 1]);

    if (Index == 1) {
      _shufps(ElementR, SourceVectRM, Mask1Constant);
      _shufps(ElementR, SourceVectRM, Mask2Constant);
      _movp(Instr->getDest(), ElementR);
    } else {
      Variable *T = makeReg(Ty);
      _movp(T, SourceVectRM);
      _shufps(ElementR, T, Mask1Constant);
      _shufps(T, ElementR, Mask2Constant);
      _movp(Instr->getDest(), T);
    }
  } else {
    assert(Ty == IceType_v16i8 || Ty == IceType_v16i1);
    // Spill the value to a stack slot and perform the insertion in memory.
    //
    // TODO(wala): use legalize(SourceVectNotLegalized, Legal_Mem) when support
    // for legalizing to mem is implemented.
    Variable *Slot = Func->makeVariable(Ty);
    Slot->setMustNotHaveReg();
    _movp(Slot, legalizeToReg(SourceVectNotLegalized));

    // Compute the location of the position to insert in memory.
    unsigned Offset = Index * typeWidthInBytes(InVectorElementTy);
    X86OperandMem *Loc =
        getMemoryOperandForStackSlot(InVectorElementTy, Slot, Offset);
    _store(legalizeToReg(ElementToInsertNotLegalized), Loc);

    Variable *T = makeReg(Ty);
    _movp(T, Slot);
    _movp(Instr->getDest(), T);
  }
}

void TargetX8632::lowerIntrinsic(const InstIntrinsic *Instr) {
  switch (Intrinsics::IntrinsicID ID = Instr->getIntrinsicID()) {
  case Intrinsics::AtomicCmpxchg: {
    if (!Intrinsics::isMemoryOrderValid(
            ID, getConstantMemoryOrder(Instr->getArg(3)),
            getConstantMemoryOrder(Instr->getArg(4)))) {
      Func->setError("Unexpected memory ordering for AtomicCmpxchg");
      return;
    }
    Variable *DestPrev = Instr->getDest();
    Operand *PtrToMem = legalize(Instr->getArg(0));
    Operand *Expected = legalize(Instr->getArg(1));
    Operand *Desired = legalize(Instr->getArg(2));
    if (tryOptimizedCmpxchgCmpBr(DestPrev, PtrToMem, Expected, Desired))
      return;
    lowerAtomicCmpxchg(DestPrev, PtrToMem, Expected, Desired);
    return;
  }
  case Intrinsics::AtomicFence:
    if (!Intrinsics::isMemoryOrderValid(
            ID, getConstantMemoryOrder(Instr->getArg(0)))) {
      Func->setError("Unexpected memory ordering for AtomicFence");
      return;
    }
    _mfence();
    return;
  case Intrinsics::AtomicFenceAll:
    // NOTE: FenceAll should prevent and load/store from being moved across the
    // fence (both atomic and non-atomic). The InstX8632Mfence instruction is
    // currently marked coarsely as "HasSideEffects".
    _mfence();
    return;
  case Intrinsics::AtomicIsLockFree: {
    // X86 is always lock free for 8/16/32/64 bit accesses.
    // TODO(jvoung): Since the result is constant when given a constant byte
    // size, this opens up DCE opportunities.
    Operand *ByteSize = Instr->getArg(0);
    Variable *Dest = Instr->getDest();
    if (auto *CI = llvm::dyn_cast<ConstantInteger32>(ByteSize)) {
      Constant *Result;
      switch (CI->getValue()) {
      default:
        // Some x86-64 processors support the cmpxchg16b instruction, which can
        // make 16-byte operations lock free (when used with the LOCK prefix).
        // However, that's not supported in 32-bit mode, so just return 0 even
        // for large sizes.
        Result = Ctx->getConstantZero(IceType_i32);
        break;
      case 1:
      case 2:
      case 4:
      case 8:
        Result = Ctx->getConstantInt32(1);
        break;
      }
      _mov(Dest, Result);
      return;
    }
    // The PNaCl ABI requires the byte size to be a compile-time constant.
    Func->setError("AtomicIsLockFree byte size should be compile-time const");
    return;
  }
  case Intrinsics::AtomicLoad: {
    // We require the memory address to be naturally aligned. Given that is the
    // case, then normal loads are atomic.
    if (!Intrinsics::isMemoryOrderValid(
            ID, getConstantMemoryOrder(Instr->getArg(1)))) {
      Func->setError("Unexpected memory ordering for AtomicLoad");
      return;
    }
    Variable *Dest = Instr->getDest();
    if (auto *Dest64On32 = llvm::dyn_cast<Variable64On32>(Dest)) {
      // Follow what GCC does and use a movq instead of what lowerLoad()
      // normally does (split the load into two). Thus, this skips
      // load/arithmetic op folding. Load/arithmetic folding can't happen
      // anyway, since this is x86-32 and integer arithmetic only happens on
      // 32-bit quantities.
      Variable *T = makeReg(IceType_f64);
      X86OperandMem *Addr = formMemoryOperand(Instr->getArg(0), IceType_f64);
      _movq(T, Addr);
      // Then cast the bits back out of the XMM register to the i64 Dest.
      auto *Cast = InstCast::create(Func, InstCast::Bitcast, Dest, T);
      lowerCast(Cast);
      // Make sure that the atomic load isn't elided when unused.
      Context.insert<InstFakeUse>(Dest64On32->getLo());
      Context.insert<InstFakeUse>(Dest64On32->getHi());
      return;
    }
    auto *Load = InstLoad::create(Func, Dest, Instr->getArg(0));
    lowerLoad(Load);
    // Make sure the atomic load isn't elided when unused, by adding a FakeUse.
    // Since lowerLoad may fuse the load w/ an arithmetic instruction, insert
    // the FakeUse on the last-inserted instruction's dest.
    Context.insert<InstFakeUse>(Context.getLastInserted()->getDest());
    return;
  }
  case Intrinsics::AtomicRMW:
    if (!Intrinsics::isMemoryOrderValid(
            ID, getConstantMemoryOrder(Instr->getArg(3)))) {
      Func->setError("Unexpected memory ordering for AtomicRMW");
      return;
    }
    lowerAtomicRMW(
        Instr->getDest(),
        static_cast<uint32_t>(
            llvm::cast<ConstantInteger32>(Instr->getArg(0))->getValue()),
        Instr->getArg(1), Instr->getArg(2));
    return;
  case Intrinsics::AtomicStore: {
    if (!Intrinsics::isMemoryOrderValid(
            ID, getConstantMemoryOrder(Instr->getArg(2)))) {
      Func->setError("Unexpected memory ordering for AtomicStore");
      return;
    }
    // We require the memory address to be naturally aligned. Given that is the
    // case, then normal stores are atomic. Add a fence after the store to make
    // it visible.
    Operand *Value = Instr->getArg(0);
    Operand *Ptr = Instr->getArg(1);
    if (Value->getType() == IceType_i64) {
      // Use a movq instead of what lowerStore() normally does (split the store
      // into two), following what GCC does. Cast the bits from int -> to an
      // xmm register first.
      Variable *T = makeReg(IceType_f64);
      auto *Cast = InstCast::create(Func, InstCast::Bitcast, T, Value);
      lowerCast(Cast);
      // Then store XMM w/ a movq.
      X86OperandMem *Addr = formMemoryOperand(Ptr, IceType_f64);
      _storeq(T, Addr);
      _mfence();
      return;
    }
    auto *Store = InstStore::create(Func, Value, Ptr);
    lowerStore(Store);
    _mfence();
    return;
  }
  case Intrinsics::Bswap: {
    Variable *Dest = Instr->getDest();
    Operand *Val = Instr->getArg(0);
    // In 32-bit mode, bswap only works on 32-bit arguments, and the argument
    // must be a register. Use rotate left for 16-bit bswap.
    if (Val->getType() == IceType_i64) {
      Val = legalizeUndef(Val);
      Variable *T_Lo = legalizeToReg(loOperand(Val));
      Variable *T_Hi = legalizeToReg(hiOperand(Val));
      auto *DestLo = llvm::cast<Variable>(loOperand(Dest));
      auto *DestHi = llvm::cast<Variable>(hiOperand(Dest));
      _bswap(T_Lo);
      _bswap(T_Hi);
      _mov(DestLo, T_Hi);
      _mov(DestHi, T_Lo);
    } else if (Val->getType() == IceType_i32) {
      Variable *T = legalizeToReg(Val);
      _bswap(T);
      _mov(Dest, T);
    } else {
      assert(Val->getType() == IceType_i16);
      Constant *Eight = Ctx->getConstantInt16(8);
      Variable *T = nullptr;
      Val = legalize(Val);
      _mov(T, Val);
      _rol(T, Eight);
      _mov(Dest, T);
    }
    return;
  }
  case Intrinsics::Ctpop: {
    Variable *Dest = Instr->getDest();
    Operand *Val = Instr->getArg(0);
    Type ValTy = Val->getType();
    assert(ValTy == IceType_i32 || ValTy == IceType_i64);

    InstCall *Call =
        makeHelperCall(ValTy == IceType_i32 ? RuntimeHelper::H_call_ctpop_i32
                                            : RuntimeHelper::H_call_ctpop_i64,
                       Dest, 1);
    Call->addArg(Val);
    lowerCall(Call);
    // The popcount helpers always return 32-bit values, while the intrinsic's
    // signature matches the native POPCNT instruction and fills a 64-bit reg
    // (in 64-bit mode). Thus, clear the upper bits of the dest just in case
    // the user doesn't do that in the IR. If the user does that in the IR,
    // then this zero'ing instruction is dead and gets optimized out.
    if (Val->getType() == IceType_i64) {
      auto *DestHi = llvm::cast<Variable>(hiOperand(Dest));
      Constant *Zero = Ctx->getConstantZero(IceType_i32);
      _mov(DestHi, Zero);
    }
    return;
  }
  case Intrinsics::Ctlz: {
    // The "is zero undef" parameter is ignored and we always return a
    // well-defined value.
    Operand *Val = legalize(Instr->getArg(0));
    Operand *FirstVal;
    Operand *SecondVal = nullptr;
    if (Val->getType() == IceType_i64) {
      FirstVal = loOperand(Val);
      SecondVal = hiOperand(Val);
    } else {
      FirstVal = Val;
    }
    constexpr bool IsCttz = false;
    lowerCountZeros(IsCttz, Val->getType(), Instr->getDest(), FirstVal,
                    SecondVal);
    return;
  }
  case Intrinsics::Cttz: {
    // The "is zero undef" parameter is ignored and we always return a
    // well-defined value.
    Operand *Val = legalize(Instr->getArg(0));
    Operand *FirstVal;
    Operand *SecondVal = nullptr;
    if (Val->getType() == IceType_i64) {
      FirstVal = hiOperand(Val);
      SecondVal = loOperand(Val);
    } else {
      FirstVal = Val;
    }
    constexpr bool IsCttz = true;
    lowerCountZeros(IsCttz, Val->getType(), Instr->getDest(), FirstVal,
                    SecondVal);
    return;
  }
  case Intrinsics::Fabs: {
    Operand *Src = legalize(Instr->getArg(0));
    Type Ty = Src->getType();
    Variable *Dest = Instr->getDest();
    Variable *T = makeVectorOfFabsMask(Ty);
    // The pand instruction operates on an m128 memory operand, so if Src is an
    // f32 or f64, we need to make sure it's in a register.
    if (isVectorType(Ty)) {
      if (llvm::isa<X86OperandMem>(Src))
        Src = legalizeToReg(Src);
    } else {
      Src = legalizeToReg(Src);
    }
    _pand(T, Src);
    if (isVectorType(Ty))
      _movp(Dest, T);
    else
      _mov(Dest, T);
    return;
  }
  case Intrinsics::Longjmp: {
    InstCall *Call = makeHelperCall(RuntimeHelper::H_call_longjmp, nullptr, 2);
    Call->addArg(Instr->getArg(0));
    Call->addArg(Instr->getArg(1));
    lowerCall(Call);
    return;
  }
  case Intrinsics::Memcpy: {
    lowerMemcpy(Instr->getArg(0), Instr->getArg(1), Instr->getArg(2));
    return;
  }
  case Intrinsics::Memmove: {
    lowerMemmove(Instr->getArg(0), Instr->getArg(1), Instr->getArg(2));
    return;
  }
  case Intrinsics::Memset: {
    lowerMemset(Instr->getArg(0), Instr->getArg(1), Instr->getArg(2));
    return;
  }
  case Intrinsics::Setjmp: {
    InstCall *Call =
        makeHelperCall(RuntimeHelper::H_call_setjmp, Instr->getDest(), 1);
    Call->addArg(Instr->getArg(0));
    lowerCall(Call);
    return;
  }
  case Intrinsics::Sqrt: {
    Operand *Src = legalize(Instr->getArg(0));
    Variable *Dest = Instr->getDest();
    Variable *T = makeReg(Dest->getType());
    _sqrt(T, Src);
    if (isVectorType(Dest->getType())) {
      _movp(Dest, T);
    } else {
      _mov(Dest, T);
    }
    return;
  }
  case Intrinsics::Stacksave: {
    Variable *esp =
        Func->getTarget()->getPhysicalRegister(getStackReg(), WordType);
    Variable *Dest = Instr->getDest();
    _mov(Dest, esp);
    return;
  }
  case Intrinsics::Stackrestore: {
    Operand *Src = Instr->getArg(0);
    _mov_sp(Src);
    return;
  }

  case Intrinsics::Trap:
    _ud2();
    return;
  case Intrinsics::LoadSubVector: {
    assert(llvm::isa<ConstantInteger32>(Instr->getArg(1)) &&
           "LoadSubVector second argument must be a constant");
    Variable *Dest = Instr->getDest();
    Type Ty = Dest->getType();
    auto *SubVectorSize = llvm::cast<ConstantInteger32>(Instr->getArg(1));
    Operand *Addr = Instr->getArg(0);
    X86OperandMem *Src = formMemoryOperand(Addr, Ty);
    doMockBoundsCheck(Src);

    if (Dest->isRematerializable()) {
      Context.insert<InstFakeDef>(Dest);
      return;
    }

    auto *T = makeReg(Ty);
    switch (SubVectorSize->getValue()) {
    case 4:
      _movd(T, Src);
      break;
    case 8:
      _movq(T, Src);
      break;
    default:
      Func->setError("Unexpected size for LoadSubVector");
      return;
    }
    _movp(Dest, T);
    return;
  }
  case Intrinsics::StoreSubVector: {
    assert(llvm::isa<ConstantInteger32>(Instr->getArg(2)) &&
           "StoreSubVector third argument must be a constant");
    auto *SubVectorSize = llvm::cast<ConstantInteger32>(Instr->getArg(2));
    Operand *Value = Instr->getArg(0);
    Operand *Addr = Instr->getArg(1);
    X86OperandMem *NewAddr = formMemoryOperand(Addr, Value->getType());
    doMockBoundsCheck(NewAddr);

    Value = legalizeToReg(Value);

    switch (SubVectorSize->getValue()) {
    case 4:
      _stored(Value, NewAddr);
      break;
    case 8:
      _storeq(Value, NewAddr);
      break;
    default:
      Func->setError("Unexpected size for StoreSubVector");
      return;
    }
    return;
  }
  case Intrinsics::VectorPackSigned: {
    Operand *Src0 = Instr->getArg(0);
    Operand *Src1 = Instr->getArg(1);
    Variable *Dest = Instr->getDest();
    auto *T = makeReg(Src0->getType());
    auto *Src0RM = legalize(Src0, Legal_Reg | Legal_Mem);
    auto *Src1RM = legalize(Src1, Legal_Reg | Legal_Mem);
    _movp(T, Src0RM);
    _packss(T, Src1RM);
    _movp(Dest, T);
    return;
  }
  case Intrinsics::VectorPackUnsigned: {
    Operand *Src0 = Instr->getArg(0);
    Operand *Src1 = Instr->getArg(1);
    Variable *Dest = Instr->getDest();
    auto *T = makeReg(Src0->getType());
    auto *Src0RM = legalize(Src0, Legal_Reg | Legal_Mem);
    auto *Src1RM = legalize(Src1, Legal_Reg | Legal_Mem);
    _movp(T, Src0RM);
    _packus(T, Src1RM);
    _movp(Dest, T);
    return;
  }
  case Intrinsics::SignMask: {
    Operand *SrcReg = legalizeToReg(Instr->getArg(0));
    Variable *Dest = Instr->getDest();
    Variable *T = makeReg(IceType_i32);
    if (SrcReg->getType() == IceType_v4f32 ||
        SrcReg->getType() == IceType_v4i32 ||
        SrcReg->getType() == IceType_v16i8) {
      _movmsk(T, SrcReg);
    } else {
      // TODO(capn): We could implement v8i16 sign mask using packsswb/pmovmskb
      llvm::report_fatal_error("Invalid type for SignMask intrinsic");
    }
    _mov(Dest, T);
    return;
  }
  case Intrinsics::MultiplyHighSigned: {
    Operand *Src0 = Instr->getArg(0);
    Operand *Src1 = Instr->getArg(1);
    Variable *Dest = Instr->getDest();
    auto *T = makeReg(Dest->getType());
    auto *Src0RM = legalize(Src0, Legal_Reg | Legal_Mem);
    auto *Src1RM = legalize(Src1, Legal_Reg | Legal_Mem);
    _movp(T, Src0RM);
    _pmulhw(T, Src1RM);
    _movp(Dest, T);
    return;
  }
  case Intrinsics::MultiplyHighUnsigned: {
    Operand *Src0 = Instr->getArg(0);
    Operand *Src1 = Instr->getArg(1);
    Variable *Dest = Instr->getDest();
    auto *T = makeReg(Dest->getType());
    auto *Src0RM = legalize(Src0, Legal_Reg | Legal_Mem);
    auto *Src1RM = legalize(Src1, Legal_Reg | Legal_Mem);
    _movp(T, Src0RM);
    _pmulhuw(T, Src1RM);
    _movp(Dest, T);
    return;
  }
  case Intrinsics::MultiplyAddPairs: {
    Operand *Src0 = Instr->getArg(0);
    Operand *Src1 = Instr->getArg(1);
    Variable *Dest = Instr->getDest();
    auto *T = makeReg(Dest->getType());
    auto *Src0RM = legalize(Src0, Legal_Reg | Legal_Mem);
    auto *Src1RM = legalize(Src1, Legal_Reg | Legal_Mem);
    _movp(T, Src0RM);
    _pmaddwd(T, Src1RM);
    _movp(Dest, T);
    return;
  }
  case Intrinsics::AddSaturateSigned: {
    Operand *Src0 = Instr->getArg(0);
    Operand *Src1 = Instr->getArg(1);
    Variable *Dest = Instr->getDest();
    auto *T = makeReg(Dest->getType());
    auto *Src0RM = legalize(Src0, Legal_Reg | Legal_Mem);
    auto *Src1RM = legalize(Src1, Legal_Reg | Legal_Mem);
    _movp(T, Src0RM);
    _padds(T, Src1RM);
    _movp(Dest, T);
    return;
  }
  case Intrinsics::SubtractSaturateSigned: {
    Operand *Src0 = Instr->getArg(0);
    Operand *Src1 = Instr->getArg(1);
    Variable *Dest = Instr->getDest();
    auto *T = makeReg(Dest->getType());
    auto *Src0RM = legalize(Src0, Legal_Reg | Legal_Mem);
    auto *Src1RM = legalize(Src1, Legal_Reg | Legal_Mem);
    _movp(T, Src0RM);
    _psubs(T, Src1RM);
    _movp(Dest, T);
    return;
  }
  case Intrinsics::AddSaturateUnsigned: {
    Operand *Src0 = Instr->getArg(0);
    Operand *Src1 = Instr->getArg(1);
    Variable *Dest = Instr->getDest();
    auto *T = makeReg(Dest->getType());
    auto *Src0RM = legalize(Src0, Legal_Reg | Legal_Mem);
    auto *Src1RM = legalize(Src1, Legal_Reg | Legal_Mem);
    _movp(T, Src0RM);
    _paddus(T, Src1RM);
    _movp(Dest, T);
    return;
  }
  case Intrinsics::SubtractSaturateUnsigned: {
    Operand *Src0 = Instr->getArg(0);
    Operand *Src1 = Instr->getArg(1);
    Variable *Dest = Instr->getDest();
    auto *T = makeReg(Dest->getType());
    auto *Src0RM = legalize(Src0, Legal_Reg | Legal_Mem);
    auto *Src1RM = legalize(Src1, Legal_Reg | Legal_Mem);
    _movp(T, Src0RM);
    _psubus(T, Src1RM);
    _movp(Dest, T);
    return;
  }
  case Intrinsics::Nearbyint: {
    Operand *Src = Instr->getArg(0);
    Variable *Dest = Instr->getDest();
    Type DestTy = Dest->getType();
    if (isVectorType(DestTy)) {
      assert(DestTy == IceType_v4i32);
      assert(Src->getType() == IceType_v4f32);
      Operand *Src0R = legalizeToReg(Src);
      Variable *T = makeReg(DestTy);
      _cvt(T, Src0R, Insts::Cvt::Ps2dq);
      _movp(Dest, T);
    } else if (DestTy == IceType_i64) {
      llvm::report_fatal_error("Helper call was expected");
    } else {
      Operand *Src0RM = legalize(Src, Legal_Reg | Legal_Mem);
      // t1.i32 = cvt Src0RM; t2.dest_type = t1; Dest = t2.dest_type
      assert(DestTy != IceType_i64);
      Variable *T_1 = makeReg(IceType_i32);
      // cvt() requires its integer argument to be a GPR.
      Variable *T_2 = makeReg(DestTy);
      if (isByteSizedType(DestTy)) {
        assert(T_1->getType() == IceType_i32);
        T_1->setRegClass(RCX86_Is32To8);
        T_2->setRegClass(RCX86_IsTrunc8Rcvr);
      }
      _cvt(T_1, Src0RM, Insts::Cvt::Ss2si);
      _mov(T_2, T_1); // T_1 and T_2 may have different integer types
      if (DestTy == IceType_i1)
        _and(T_2, Ctx->getConstantInt1(1));
      _mov(Dest, T_2);
    }
    return;
  }
  case Intrinsics::Round: {
    assert(InstructionSet >= SSE4_1);
    Variable *Dest = Instr->getDest();
    Operand *Src = Instr->getArg(0);
    Operand *Mode = Instr->getArg(1);
    assert(llvm::isa<ConstantInteger32>(Mode) &&
           "Round last argument must be a constant");
    auto *SrcRM = legalize(Src, Legal_Reg | Legal_Mem);
    int32_t Imm = llvm::cast<ConstantInteger32>(Mode)->getValue();
    (void)Imm;
    assert(Imm >= 0 && Imm < 4 && "Invalid rounding mode");
    auto *T = makeReg(Dest->getType());
    _round(T, SrcRM, Mode);
    _movp(Dest, T);
    return;
  }
  default: // UnknownIntrinsic
    Func->setError("Unexpected intrinsic");
    return;
  }
  return;
}

void TargetX8632::lowerAtomicCmpxchg(Variable *DestPrev, Operand *Ptr,
                                     Operand *Expected, Operand *Desired) {
  Type Ty = Expected->getType();
  if (Ty == IceType_i64) {
    // Reserve the pre-colored registers first, before adding any more
    // infinite-weight variables from formMemoryOperand's legalization.
    Variable *T_edx = makeReg(IceType_i32, RegX8632::Reg_edx);
    Variable *T_eax = makeReg(IceType_i32, RegX8632::Reg_eax);
    Variable *T_ecx = makeReg(IceType_i32, RegX8632::Reg_ecx);
    Variable *T_ebx = makeReg(IceType_i32, RegX8632::Reg_ebx);
    _mov(T_eax, loOperand(Expected));
    _mov(T_edx, hiOperand(Expected));
    _mov(T_ebx, loOperand(Desired));
    _mov(T_ecx, hiOperand(Desired));
    X86OperandMem *Addr = formMemoryOperand(Ptr, Ty);
    constexpr bool Locked = true;
    _cmpxchg8b(Addr, T_edx, T_eax, T_ecx, T_ebx, Locked);
    auto *DestLo = llvm::cast<Variable>(loOperand(DestPrev));
    auto *DestHi = llvm::cast<Variable>(hiOperand(DestPrev));
    _mov(DestLo, T_eax);
    _mov(DestHi, T_edx);
    return;
  }
  RegNumT Eax;
  switch (Ty) {
  default:
    llvm::report_fatal_error("Bad type for cmpxchg");
  case IceType_i32:
    Eax = RegX8632::Reg_eax;
    break;
  case IceType_i16:
    Eax = RegX8632::Reg_ax;
    break;
  case IceType_i8:
    Eax = RegX8632::Reg_al;
    break;
  }
  Variable *T_eax = makeReg(Ty, Eax);
  _mov(T_eax, Expected);
  X86OperandMem *Addr = formMemoryOperand(Ptr, Ty);
  Variable *DesiredReg = legalizeToReg(Desired);
  constexpr bool Locked = true;
  _cmpxchg(Addr, T_eax, DesiredReg, Locked);
  _mov(DestPrev, T_eax);
}

bool TargetX8632::tryOptimizedCmpxchgCmpBr(Variable *Dest, Operand *PtrToMem,
                                           Operand *Expected,
                                           Operand *Desired) {
  if (Func->getOptLevel() == Opt_m1)
    return false;
  // Peek ahead a few instructions and see how Dest is used.
  // It's very common to have:
  //
  // %x = call i32 @llvm.nacl.atomic.cmpxchg.i32(i32* ptr, i32 %expected, ...)
  // [%y_phi = ...] // list of phi stores
  // %p = icmp eq i32 %x, %expected
  // br i1 %p, label %l1, label %l2
  //
  // which we can optimize into:
  //
  // %x = <cmpxchg code>
  // [%y_phi = ...] // list of phi stores
  // br eq, %l1, %l2
  InstList::iterator I = Context.getCur();
  // I is currently the InstIntrinsic. Peek past that.
  // This assumes that the atomic cmpxchg has not been lowered yet,
  // so that the instructions seen in the scan from "Cur" is simple.
  assert(llvm::isa<InstIntrinsic>(*I));
  Inst *NextInst = Context.getNextInst(I);
  if (!NextInst)
    return false;
  // There might be phi assignments right before the compare+branch, since
  // this could be a backward branch for a loop. This placement of assignments
  // is determined by placePhiStores().
  CfgVector<InstAssign *> PhiAssigns;
  while (auto *PhiAssign = llvm::dyn_cast<InstAssign>(NextInst)) {
    if (PhiAssign->getDest() == Dest)
      return false;
    PhiAssigns.push_back(PhiAssign);
    NextInst = Context.getNextInst(I);
    if (!NextInst)
      return false;
  }
  if (auto *NextCmp = llvm::dyn_cast<InstIcmp>(NextInst)) {
    if (!(NextCmp->getCondition() == InstIcmp::Eq &&
          ((NextCmp->getSrc(0) == Dest && NextCmp->getSrc(1) == Expected) ||
           (NextCmp->getSrc(1) == Dest && NextCmp->getSrc(0) == Expected)))) {
      return false;
    }
    NextInst = Context.getNextInst(I);
    if (!NextInst)
      return false;
    if (auto *NextBr = llvm::dyn_cast<InstBr>(NextInst)) {
      if (!NextBr->isUnconditional() &&
          NextCmp->getDest() == NextBr->getCondition() &&
          NextBr->isLastUse(NextCmp->getDest())) {
        lowerAtomicCmpxchg(Dest, PtrToMem, Expected, Desired);
        for (size_t i = 0; i < PhiAssigns.size(); ++i) {
          // Lower the phi assignments now, before the branch (same placement
          // as before).
          InstAssign *PhiAssign = PhiAssigns[i];
          PhiAssign->setDeleted();
          lowerAssign(PhiAssign);
          Context.advanceNext();
        }
        _br(CondX86::Br_e, NextBr->getTargetTrue(), NextBr->getTargetFalse());
        // Skip over the old compare and branch, by deleting them.
        NextCmp->setDeleted();
        NextBr->setDeleted();
        Context.advanceNext();
        Context.advanceNext();
        return true;
      }
    }
  }
  return false;
}

void TargetX8632::lowerAtomicRMW(Variable *Dest, uint32_t Operation,
                                 Operand *Ptr, Operand *Val) {
  bool NeedsCmpxchg = false;
  LowerBinOp Op_Lo = nullptr;
  LowerBinOp Op_Hi = nullptr;
  switch (Operation) {
  default:
    Func->setError("Unknown AtomicRMW operation");
    return;
  case Intrinsics::AtomicAdd: {
    if (Dest->getType() == IceType_i64) {
      // All the fall-through paths must set this to true, but use this
      // for asserting.
      NeedsCmpxchg = true;
      Op_Lo = &TargetX8632::_add;
      Op_Hi = &TargetX8632::_adc;
      break;
    }
    X86OperandMem *Addr = formMemoryOperand(Ptr, Dest->getType());
    constexpr bool Locked = true;
    Variable *T = nullptr;
    _mov(T, Val);
    _xadd(Addr, T, Locked);
    _mov(Dest, T);
    return;
  }
  case Intrinsics::AtomicSub: {
    if (Dest->getType() == IceType_i64) {
      NeedsCmpxchg = true;
      Op_Lo = &TargetX8632::_sub;
      Op_Hi = &TargetX8632::_sbb;
      break;
    }
    X86OperandMem *Addr = formMemoryOperand(Ptr, Dest->getType());
    constexpr bool Locked = true;
    Variable *T = nullptr;
    _mov(T, Val);
    _neg(T);
    _xadd(Addr, T, Locked);
    _mov(Dest, T);
    return;
  }
  case Intrinsics::AtomicOr:
    // TODO(jvoung): If Dest is null or dead, then some of these
    // operations do not need an "exchange", but just a locked op.
    // That appears to be "worth" it for sub, or, and, and xor.
    // xadd is probably fine vs lock add for add, and xchg is fine
    // vs an atomic store.
    NeedsCmpxchg = true;
    Op_Lo = &TargetX8632::_or;
    Op_Hi = &TargetX8632::_or;
    break;
  case Intrinsics::AtomicAnd:
    NeedsCmpxchg = true;
    Op_Lo = &TargetX8632::_and;
    Op_Hi = &TargetX8632::_and;
    break;
  case Intrinsics::AtomicXor:
    NeedsCmpxchg = true;
    Op_Lo = &TargetX8632::_xor;
    Op_Hi = &TargetX8632::_xor;
    break;
  case Intrinsics::AtomicExchange:
    if (Dest->getType() == IceType_i64) {
      NeedsCmpxchg = true;
      // NeedsCmpxchg, but no real Op_Lo/Op_Hi need to be done. The values
      // just need to be moved to the ecx and ebx registers.
      Op_Lo = nullptr;
      Op_Hi = nullptr;
      break;
    }
    X86OperandMem *Addr = formMemoryOperand(Ptr, Dest->getType());
    Variable *T = nullptr;
    _mov(T, Val);
    _xchg(Addr, T);
    _mov(Dest, T);
    return;
  }
  // Otherwise, we need a cmpxchg loop.
  (void)NeedsCmpxchg;
  assert(NeedsCmpxchg);
  expandAtomicRMWAsCmpxchg(Op_Lo, Op_Hi, Dest, Ptr, Val);
}

void TargetX8632::expandAtomicRMWAsCmpxchg(LowerBinOp Op_Lo, LowerBinOp Op_Hi,
                                           Variable *Dest, Operand *Ptr,
                                           Operand *Val) {
  // Expand a more complex RMW operation as a cmpxchg loop:
  // For 64-bit:
  //   mov     eax, [ptr]
  //   mov     edx, [ptr + 4]
  // .LABEL:
  //   mov     ebx, eax
  //   <Op_Lo> ebx, <desired_adj_lo>
  //   mov     ecx, edx
  //   <Op_Hi> ecx, <desired_adj_hi>
  //   lock cmpxchg8b [ptr]
  //   jne     .LABEL
  //   mov     <dest_lo>, eax
  //   mov     <dest_lo>, edx
  //
  // For 32-bit:
  //   mov     eax, [ptr]
  // .LABEL:
  //   mov     <reg>, eax
  //   op      <reg>, [desired_adj]
  //   lock cmpxchg [ptr], <reg>
  //   jne     .LABEL
  //   mov     <dest>, eax
  //
  // If Op_{Lo,Hi} are nullptr, then just copy the value.
  Val = legalize(Val);
  Type Ty = Val->getType();
  if (Ty == IceType_i64) {
    Variable *T_edx = makeReg(IceType_i32, RegX8632::Reg_edx);
    Variable *T_eax = makeReg(IceType_i32, RegX8632::Reg_eax);
    X86OperandMem *Addr = formMemoryOperand(Ptr, Ty);
    _mov(T_eax, loOperand(Addr));
    _mov(T_edx, hiOperand(Addr));
    Variable *T_ecx = makeReg(IceType_i32, RegX8632::Reg_ecx);
    Variable *T_ebx = makeReg(IceType_i32, RegX8632::Reg_ebx);
    InstX86Label *Label = InstX86Label::create(Func, this);
    const bool IsXchg8b = Op_Lo == nullptr && Op_Hi == nullptr;
    if (!IsXchg8b) {
      Context.insert(Label);
      _mov(T_ebx, T_eax);
      (this->*Op_Lo)(T_ebx, loOperand(Val));
      _mov(T_ecx, T_edx);
      (this->*Op_Hi)(T_ecx, hiOperand(Val));
    } else {
      // This is for xchg, which doesn't need an actual Op_Lo/Op_Hi.
      // It just needs the Val loaded into ebx and ecx.
      // That can also be done before the loop.
      _mov(T_ebx, loOperand(Val));
      _mov(T_ecx, hiOperand(Val));
      Context.insert(Label);
    }
    constexpr bool Locked = true;
    _cmpxchg8b(Addr, T_edx, T_eax, T_ecx, T_ebx, Locked);
    _br(CondX86::Br_ne, Label);
    if (!IsXchg8b) {
      // If Val is a variable, model the extended live range of Val through
      // the end of the loop, since it will be re-used by the loop.
      if (auto *ValVar = llvm::dyn_cast<Variable>(Val)) {
        auto *ValLo = llvm::cast<Variable>(loOperand(ValVar));
        auto *ValHi = llvm::cast<Variable>(hiOperand(ValVar));
        Context.insert<InstFakeUse>(ValLo);
        Context.insert<InstFakeUse>(ValHi);
      }
    } else {
      // For xchg, the loop is slightly smaller and ebx/ecx are used.
      Context.insert<InstFakeUse>(T_ebx);
      Context.insert<InstFakeUse>(T_ecx);
    }
    // The address base (if any) is also reused in the loop.
    if (Variable *Base = Addr->getBase())
      Context.insert<InstFakeUse>(Base);
    auto *DestLo = llvm::cast<Variable>(loOperand(Dest));
    auto *DestHi = llvm::cast<Variable>(hiOperand(Dest));
    _mov(DestLo, T_eax);
    _mov(DestHi, T_edx);
    return;
  }
  X86OperandMem *Addr = formMemoryOperand(Ptr, Ty);
  RegNumT Eax;
  switch (Ty) {
  default:
    llvm::report_fatal_error("Bad type for atomicRMW");
  case IceType_i32:
    Eax = RegX8632::Reg_eax;
    break;
  case IceType_i16:
    Eax = RegX8632::Reg_ax;
    break;
  case IceType_i8:
    Eax = RegX8632::Reg_al;
    break;
  }
  Variable *T_eax = makeReg(Ty, Eax);
  _mov(T_eax, Addr);
  auto *Label = Context.insert<InstX86Label>(this);
  // We want to pick a different register for T than Eax, so don't use
  // _mov(T == nullptr, T_eax).
  Variable *T = makeReg(Ty);
  _mov(T, T_eax);
  (this->*Op_Lo)(T, Val);
  constexpr bool Locked = true;
  _cmpxchg(Addr, T_eax, T, Locked);
  _br(CondX86::Br_ne, Label);
  // If Val is a variable, model the extended live range of Val through
  // the end of the loop, since it will be re-used by the loop.
  if (auto *ValVar = llvm::dyn_cast<Variable>(Val)) {
    Context.insert<InstFakeUse>(ValVar);
  }
  // The address base (if any) is also reused in the loop.
  if (Variable *Base = Addr->getBase())
    Context.insert<InstFakeUse>(Base);
  _mov(Dest, T_eax);
}

/// Lowers count {trailing, leading} zeros intrinsic.
///
/// We could do constant folding here, but that should have
/// been done by the front-end/middle-end optimizations.

void TargetX8632::lowerCountZeros(bool Cttz, Type Ty, Variable *Dest,
                                  Operand *FirstVal, Operand *SecondVal) {
  // TODO(jvoung): Determine if the user CPU supports LZCNT (BMI).
  // Then the instructions will handle the Val == 0 case much more simply
  // and won't require conversion from bit position to number of zeros.
  //
  // Otherwise:
  //   bsr IF_NOT_ZERO, Val
  //   mov T_DEST, ((Ty == i32) ? 63 : 127)
  //   cmovne T_DEST, IF_NOT_ZERO
  //   xor T_DEST, ((Ty == i32) ? 31 : 63)
  //   mov DEST, T_DEST
  //
  // NOTE: T_DEST must be a register because cmov requires its dest to be a
  // register. Also, bsf and bsr require their dest to be a register.
  //
  // The xor DEST, C(31|63) converts a bit position to # of leading zeroes.
  // E.g., for 000... 00001100, bsr will say that the most significant bit
  // set is at position 3, while the number of leading zeros is 28. Xor is
  // like (M - N) for N <= M, and converts 63 to 32, and 127 to 64 (for the
  // all-zeros case).
  //
  // X8632 only: Similar for 64-bit, but start w/ speculating that the upper
  // 32 bits are all zero, and compute the result for that case (checking the
  // lower 32 bits). Then actually compute the result for the upper bits and
  // cmov in the result from the lower computation if the earlier speculation
  // was correct.
  //
  // Cttz, is similar, but uses bsf instead, and doesn't require the xor
  // bit position conversion, and the speculation is reversed.

  // TODO(jpp): refactor this method.
  assert(Ty == IceType_i32 || Ty == IceType_i64);
  const Type DestTy = IceType_i32;
  Variable *T = makeReg(DestTy);
  Operand *FirstValRM = legalize(FirstVal, Legal_Mem | Legal_Reg);
  if (Cttz) {
    _bsf(T, FirstValRM);
  } else {
    _bsr(T, FirstValRM);
  }
  Variable *T_Dest = makeReg(DestTy);
  Constant *_31 = Ctx->getConstantInt32(31);
  Constant *_32 = Ctx->getConstantInt(DestTy, 32);
  Constant *_63 = Ctx->getConstantInt(DestTy, 63);
  Constant *_64 = Ctx->getConstantInt(DestTy, 64);
  if (Cttz) {
    if (DestTy == IceType_i64) {
      _mov(T_Dest, _64);
    } else {
      _mov(T_Dest, _32);
    }
  } else {
    Constant *_127 = Ctx->getConstantInt(DestTy, 127);
    if (DestTy == IceType_i64) {
      _mov(T_Dest, _127);
    } else {
      _mov(T_Dest, _63);
    }
  }
  _cmov(T_Dest, T, CondX86::Br_ne);
  if (!Cttz) {
    if (DestTy == IceType_i64) {
      // Even though there's a _63 available at this point, that constant
      // might not be an i32, which will cause the xor emission to fail.
      Constant *_63 = Ctx->getConstantInt32(63);
      _xor(T_Dest, _63);
    } else {
      _xor(T_Dest, _31);
    }
  }
  if (Ty == IceType_i32) {
    _mov(Dest, T_Dest);
    return;
  }
  _add(T_Dest, _32);
  auto *DestLo = llvm::cast<Variable>(loOperand(Dest));
  auto *DestHi = llvm::cast<Variable>(hiOperand(Dest));
  // Will be using "test" on this, so we need a registerized variable.
  Variable *SecondVar = legalizeToReg(SecondVal);
  Variable *T_Dest2 = makeReg(IceType_i32);
  if (Cttz) {
    _bsf(T_Dest2, SecondVar);
  } else {
    _bsr(T_Dest2, SecondVar);
    _xor(T_Dest2, _31);
  }
  _test(SecondVar, SecondVar);
  _cmov(T_Dest2, T_Dest, CondX86::Br_e);
  _mov(DestLo, T_Dest2);
  _mov(DestHi, Ctx->getConstantZero(IceType_i32));
}

void TargetX8632::typedLoad(Type Ty, Variable *Dest, Variable *Base,
                            Constant *Offset) {
  // If Offset is a ConstantRelocatable in Non-SFI mode, we will need to
  // legalize Mem properly.
  if (Offset)
    assert(!llvm::isa<ConstantRelocatable>(Offset));

  auto *Mem = X86OperandMem::create(Func, Ty, Base, Offset);

  if (isVectorType(Ty))
    _movp(Dest, Mem);
  else if (Ty == IceType_f64)
    _movq(Dest, Mem);
  else
    _mov(Dest, Mem);
}

void TargetX8632::typedStore(Type Ty, Variable *Value, Variable *Base,
                             Constant *Offset) {
  // If Offset is a ConstantRelocatable in Non-SFI mode, we will need to
  // legalize Mem properly.
  if (Offset)
    assert(!llvm::isa<ConstantRelocatable>(Offset));

  auto *Mem = X86OperandMem::create(Func, Ty, Base, Offset);

  if (isVectorType(Ty))
    _storep(Value, Mem);
  else if (Ty == IceType_f64)
    _storeq(Value, Mem);
  else
    _store(Value, Mem);
}

void TargetX8632::copyMemory(Type Ty, Variable *Dest, Variable *Src,
                             int32_t OffsetAmt) {
  Constant *Offset = OffsetAmt ? Ctx->getConstantInt32(OffsetAmt) : nullptr;
  // TODO(ascull): this or add nullptr test to _movp, _movq
  Variable *Data = makeReg(Ty);

  typedLoad(Ty, Data, Src, Offset);
  typedStore(Ty, Data, Dest, Offset);
}

void TargetX8632::lowerMemcpy(Operand *Dest, Operand *Src, Operand *Count) {
  // There is a load and store for each chunk in the unroll
  constexpr uint32_t BytesPerStorep = 16;

  // Check if the operands are constants
  const auto *CountConst = llvm::dyn_cast<const ConstantInteger32>(Count);
  const bool IsCountConst = CountConst != nullptr;
  const uint32_t CountValue = IsCountConst ? CountConst->getValue() : 0;

  if (shouldOptimizeMemIntrins() && IsCountConst &&
      CountValue <= BytesPerStorep * MEMCPY_UNROLL_LIMIT) {
    // Unlikely, but nothing to do if it does happen
    if (CountValue == 0)
      return;

    Variable *SrcBase = legalizeToReg(Src);
    Variable *DestBase = legalizeToReg(Dest);

    // Find the largest type that can be used and use it as much as possible
    // in reverse order. Then handle any remainder with overlapping copies.
    // Since the remainder will be at the end, there will be reduced pressure
    // on the memory unit as the accesses to the same memory are far apart.
    Type Ty = largestTypeInSize(CountValue);
    uint32_t TyWidth = typeWidthInBytes(Ty);

    uint32_t RemainingBytes = CountValue;
    int32_t Offset = (CountValue & ~(TyWidth - 1)) - TyWidth;
    while (RemainingBytes >= TyWidth) {
      copyMemory(Ty, DestBase, SrcBase, Offset);
      RemainingBytes -= TyWidth;
      Offset -= TyWidth;
    }

    if (RemainingBytes == 0)
      return;

    // Lower the remaining bytes. Adjust to larger types in order to make use
    // of overlaps in the copies.
    Type LeftOverTy = firstTypeThatFitsSize(RemainingBytes);
    Offset = CountValue - typeWidthInBytes(LeftOverTy);
    copyMemory(LeftOverTy, DestBase, SrcBase, Offset);
    return;
  }

  // Fall back on a function call
  InstCall *Call = makeHelperCall(RuntimeHelper::H_call_memcpy, nullptr, 3);
  Call->addArg(Dest);
  Call->addArg(Src);
  Call->addArg(Count);
  lowerCall(Call);
}

void TargetX8632::lowerMemmove(Operand *Dest, Operand *Src, Operand *Count) {
  // There is a load and store for each chunk in the unroll
  constexpr uint32_t BytesPerStorep = 16;

  // Check if the operands are constants
  const auto *CountConst = llvm::dyn_cast<const ConstantInteger32>(Count);
  const bool IsCountConst = CountConst != nullptr;
  const uint32_t CountValue = IsCountConst ? CountConst->getValue() : 0;

  if (shouldOptimizeMemIntrins() && IsCountConst &&
      CountValue <= BytesPerStorep * MEMMOVE_UNROLL_LIMIT) {
    // Unlikely, but nothing to do if it does happen
    if (CountValue == 0)
      return;

    Variable *SrcBase = legalizeToReg(Src);
    Variable *DestBase = legalizeToReg(Dest);

    std::tuple<Type, Constant *, Variable *> Moves[MEMMOVE_UNROLL_LIMIT];
    Constant *Offset;
    Variable *Reg;

    // Copy the data into registers as the source and destination could
    // overlap so make sure not to clobber the memory. This also means
    // overlapping moves can be used as we are taking a safe snapshot of the
    // memory.
    Type Ty = largestTypeInSize(CountValue);
    uint32_t TyWidth = typeWidthInBytes(Ty);

    uint32_t RemainingBytes = CountValue;
    int32_t OffsetAmt = (CountValue & ~(TyWidth - 1)) - TyWidth;
    size_t N = 0;
    while (RemainingBytes >= TyWidth) {
      assert(N <= MEMMOVE_UNROLL_LIMIT);
      Offset = Ctx->getConstantInt32(OffsetAmt);
      Reg = makeReg(Ty);
      typedLoad(Ty, Reg, SrcBase, Offset);
      RemainingBytes -= TyWidth;
      OffsetAmt -= TyWidth;
      Moves[N++] = std::make_tuple(Ty, Offset, Reg);
    }

    if (RemainingBytes != 0) {
      // Lower the remaining bytes. Adjust to larger types in order to make
      // use of overlaps in the copies.
      assert(N <= MEMMOVE_UNROLL_LIMIT);
      Ty = firstTypeThatFitsSize(RemainingBytes);
      Offset = Ctx->getConstantInt32(CountValue - typeWidthInBytes(Ty));
      Reg = makeReg(Ty);
      typedLoad(Ty, Reg, SrcBase, Offset);
      Moves[N++] = std::make_tuple(Ty, Offset, Reg);
    }

    // Copy the data out into the destination memory
    for (size_t i = 0; i < N; ++i) {
      std::tie(Ty, Offset, Reg) = Moves[i];
      typedStore(Ty, Reg, DestBase, Offset);
    }

    return;
  }

  // Fall back on a function call
  InstCall *Call = makeHelperCall(RuntimeHelper::H_call_memmove, nullptr, 3);
  Call->addArg(Dest);
  Call->addArg(Src);
  Call->addArg(Count);
  lowerCall(Call);
}

void TargetX8632::lowerMemset(Operand *Dest, Operand *Val, Operand *Count) {
  constexpr uint32_t BytesPerStorep = 16;
  constexpr uint32_t BytesPerStoreq = 8;
  constexpr uint32_t BytesPerStorei32 = 4;
  assert(Val->getType() == IceType_i8);

  // Check if the operands are constants
  const auto *CountConst = llvm::dyn_cast<const ConstantInteger32>(Count);
  const auto *ValConst = llvm::dyn_cast<const ConstantInteger32>(Val);
  const bool IsCountConst = CountConst != nullptr;
  const bool IsValConst = ValConst != nullptr;
  const uint32_t CountValue = IsCountConst ? CountConst->getValue() : 0;
  const uint32_t ValValue = IsValConst ? ValConst->getValue() : 0;

  // Unlikely, but nothing to do if it does happen
  if (IsCountConst && CountValue == 0)
    return;

  // TODO(ascull): if the count is constant but val is not it would be
  // possible to inline by spreading the value across 4 bytes and accessing
  // subregs e.g. eax, ax and al.
  if (shouldOptimizeMemIntrins() && IsCountConst && IsValConst) {
    Variable *Base = nullptr;
    Variable *VecReg = nullptr;
    const uint32_t MaskValue = (ValValue & 0xff);
    const uint32_t SpreadValue =
        (MaskValue << 24) | (MaskValue << 16) | (MaskValue << 8) | MaskValue;

    auto lowerSet = [this, &Base, SpreadValue, &VecReg](Type Ty,
                                                        uint32_t OffsetAmt) {
      assert(Base != nullptr);
      Constant *Offset = OffsetAmt ? Ctx->getConstantInt32(OffsetAmt) : nullptr;

      // TODO(ascull): is 64-bit better with vector or scalar movq?
      auto *Mem = X86OperandMem::create(Func, Ty, Base, Offset);
      if (isVectorType(Ty)) {
        assert(VecReg != nullptr);
        _storep(VecReg, Mem);
      } else if (Ty == IceType_f64) {
        assert(VecReg != nullptr);
        _storeq(VecReg, Mem);
      } else {
        assert(Ty != IceType_i64);
        _store(Ctx->getConstantInt(Ty, SpreadValue), Mem);
      }
    };

    // Find the largest type that can be used and use it as much as possible
    // in reverse order. Then handle any remainder with overlapping copies.
    // Since the remainder will be at the end, there will be reduces pressure
    // on the memory unit as the access to the same memory are far apart.
    Type Ty = IceType_void;
    if (ValValue == 0 && CountValue >= BytesPerStoreq &&
        CountValue <= BytesPerStorep * MEMSET_UNROLL_LIMIT) {
      // When the value is zero it can be loaded into a vector register
      // cheaply using the xor trick.
      Base = legalizeToReg(Dest);
      VecReg = makeVectorOfZeros(IceType_v16i8);
      Ty = largestTypeInSize(CountValue);
    } else if (CountValue <= BytesPerStorei32 * MEMSET_UNROLL_LIMIT) {
      // When the value is non-zero or the count is small we can't use vector
      // instructions so are limited to 32-bit stores.
      Base = legalizeToReg(Dest);
      constexpr uint32_t MaxSize = 4;
      Ty = largestTypeInSize(CountValue, MaxSize);
    }

    if (Base) {
      uint32_t TyWidth = typeWidthInBytes(Ty);

      uint32_t RemainingBytes = CountValue;
      uint32_t Offset = (CountValue & ~(TyWidth - 1)) - TyWidth;
      while (RemainingBytes >= TyWidth) {
        lowerSet(Ty, Offset);
        RemainingBytes -= TyWidth;
        Offset -= TyWidth;
      }

      if (RemainingBytes == 0)
        return;

      // Lower the remaining bytes. Adjust to larger types in order to make
      // use of overlaps in the copies.
      Type LeftOverTy = firstTypeThatFitsSize(RemainingBytes);
      Offset = CountValue - typeWidthInBytes(LeftOverTy);
      lowerSet(LeftOverTy, Offset);
      return;
    }
  }

  // Fall back on calling the memset function. The value operand needs to be
  // extended to a stack slot size because the PNaCl ABI requires arguments to
  // be at least 32 bits wide.
  Operand *ValExt;
  if (IsValConst) {
    ValExt = Ctx->getConstantInt(stackSlotType(), ValValue);
  } else {
    Variable *ValExtVar = Func->makeVariable(stackSlotType());
    lowerCast(InstCast::create(Func, InstCast::Zext, ValExtVar, Val));
    ValExt = ValExtVar;
  }
  InstCall *Call = makeHelperCall(RuntimeHelper::H_call_memset, nullptr, 3);
  Call->addArg(Dest);
  Call->addArg(ValExt);
  Call->addArg(Count);
  lowerCall(Call);
}

class AddressOptimizer {
  AddressOptimizer() = delete;
  AddressOptimizer(const AddressOptimizer &) = delete;
  AddressOptimizer &operator=(const AddressOptimizer &) = delete;

public:
  explicit AddressOptimizer(const Cfg *Func)
      : Func(Func), VMetadata(Func->getVMetadata()) {}

  inline void dumpAddressOpt(const ConstantRelocatable *const Relocatable,
                             int32_t Offset, const Variable *Base,
                             const Variable *Index, uint16_t Shift,
                             const Inst *Reason) const;

  inline const Inst *matchAssign(Variable **Var,
                                 ConstantRelocatable **Relocatable,
                                 int32_t *Offset);

  inline const Inst *matchCombinedBaseIndex(Variable **Base, Variable **Index,
                                            uint16_t *Shift);

  inline const Inst *matchShiftedIndex(Variable **Index, uint16_t *Shift);

  inline const Inst *matchOffsetIndexOrBase(Variable **IndexOrBase,
                                            const uint16_t Shift,
                                            ConstantRelocatable **Relocatable,
                                            int32_t *Offset);

private:
  const Cfg *const Func;
  const VariablesMetadata *const VMetadata;

  static bool isAdd(const Inst *Instr) {
    if (auto *Arith = llvm::dyn_cast_or_null<const InstArithmetic>(Instr)) {
      return (Arith->getOp() == InstArithmetic::Add);
    }
    return false;
  }
};

void AddressOptimizer::dumpAddressOpt(
    const ConstantRelocatable *const Relocatable, int32_t Offset,
    const Variable *Base, const Variable *Index, uint16_t Shift,
    const Inst *Reason) const {
  if (!BuildDefs::dump())
    return;
  if (!Func->isVerbose(IceV_AddrOpt))
    return;
  OstreamLocker L(Func->getContext());
  Ostream &Str = Func->getContext()->getStrDump();
  Str << "Instruction: ";
  Reason->dumpDecorated(Func);
  Str << "  results in Base=";
  if (Base)
    Base->dump(Func);
  else
    Str << "<null>";
  Str << ", Index=";
  if (Index)
    Index->dump(Func);
  else
    Str << "<null>";
  Str << ", Shift=" << Shift << ", Offset=" << Offset
      << ", Relocatable=" << Relocatable << "\n";
}

const Inst *AddressOptimizer::matchAssign(Variable **Var,
                                          ConstantRelocatable **Relocatable,
                                          int32_t *Offset) {
  // Var originates from Var=SrcVar ==> set Var:=SrcVar
  if (*Var == nullptr)
    return nullptr;
  if (const Inst *VarAssign = VMetadata->getSingleDefinition(*Var)) {
    assert(!VMetadata->isMultiDef(*Var));
    if (llvm::isa<InstAssign>(VarAssign)) {
      Operand *SrcOp = VarAssign->getSrc(0);
      assert(SrcOp);
      if (auto *SrcVar = llvm::dyn_cast<Variable>(SrcOp)) {
        if (!VMetadata->isMultiDef(SrcVar) &&
            // TODO: ensure SrcVar stays single-BB
            true) {
          *Var = SrcVar;
          return VarAssign;
        }
      } else if (auto *Const = llvm::dyn_cast<ConstantInteger32>(SrcOp)) {
        int32_t MoreOffset = Const->getValue();
        if (Utils::WouldOverflowAdd(*Offset, MoreOffset))
          return nullptr;
        *Var = nullptr;
        *Offset += MoreOffset;
        return VarAssign;
      } else if (auto *AddReloc = llvm::dyn_cast<ConstantRelocatable>(SrcOp)) {
        if (*Relocatable == nullptr) {
          // It is always safe to fold a relocatable through assignment -- the
          // assignment frees a slot in the address operand that can be used
          // to hold the Sandbox Pointer -- if any.
          *Var = nullptr;
          *Relocatable = AddReloc;
          return VarAssign;
        }
      }
    }
  }
  return nullptr;
}

const Inst *AddressOptimizer::matchCombinedBaseIndex(Variable **Base,
                                                     Variable **Index,
                                                     uint16_t *Shift) {
  // Index==nullptr && Base is Base=Var1+Var2 ==>
  //   set Base=Var1, Index=Var2, Shift=0
  if (*Base == nullptr)
    return nullptr;
  if (*Index != nullptr)
    return nullptr;
  auto *BaseInst = VMetadata->getSingleDefinition(*Base);
  if (BaseInst == nullptr)
    return nullptr;
  assert(!VMetadata->isMultiDef(*Base));
  if (BaseInst->getSrcSize() < 2)
    return nullptr;
  if (auto *Var1 = llvm::dyn_cast<Variable>(BaseInst->getSrc(0))) {
    if (VMetadata->isMultiDef(Var1))
      return nullptr;
    if (auto *Var2 = llvm::dyn_cast<Variable>(BaseInst->getSrc(1))) {
      if (VMetadata->isMultiDef(Var2))
        return nullptr;
      if (isAdd(BaseInst) &&
          // TODO: ensure Var1 and Var2 stay single-BB
          true) {
        *Base = Var1;
        *Index = Var2;
        *Shift = 0; // should already have been 0
        return BaseInst;
      }
    }
  }
  return nullptr;
}

const Inst *AddressOptimizer::matchShiftedIndex(Variable **Index,
                                                uint16_t *Shift) {
  // Index is Index=Var*Const && log2(Const)+Shift<=3 ==>
  //   Index=Var, Shift+=log2(Const)
  if (*Index == nullptr)
    return nullptr;
  auto *IndexInst = VMetadata->getSingleDefinition(*Index);
  if (IndexInst == nullptr)
    return nullptr;
  assert(!VMetadata->isMultiDef(*Index));

  // When using an unsigned 32-bit array index on x64, it gets zero-extended
  // before the shift & add. The explicit zero extension can be eliminated
  // because x86 32-bit operations automatically get zero-extended into the
  // corresponding 64-bit register.
  if (auto *CastInst = llvm::dyn_cast<InstCast>(IndexInst)) {
    if (CastInst->getCastKind() == InstCast::Zext) {
      if (auto *Var = llvm::dyn_cast<Variable>(CastInst->getSrc(0))) {
        if (Var->getType() == IceType_i32 &&
            CastInst->getDest()->getType() == IceType_i64) {
          IndexInst = VMetadata->getSingleDefinition(Var);
        }
      }
    }
  }

  if (IndexInst->getSrcSize() < 2)
    return nullptr;
  if (auto *ArithInst = llvm::dyn_cast<InstArithmetic>(IndexInst)) {
    if (auto *Var = llvm::dyn_cast<Variable>(ArithInst->getSrc(0))) {
      if (auto *Const =
              llvm::dyn_cast<ConstantInteger32>(ArithInst->getSrc(1))) {
        if (VMetadata->isMultiDef(Var) || Const->getType() != IceType_i32)
          return nullptr;
        switch (ArithInst->getOp()) {
        default:
          return nullptr;
        case InstArithmetic::Mul: {
          uint32_t Mult = Const->getValue();
          uint32_t LogMult;
          switch (Mult) {
          case 1:
            LogMult = 0;
            break;
          case 2:
            LogMult = 1;
            break;
          case 4:
            LogMult = 2;
            break;
          case 8:
            LogMult = 3;
            break;
          default:
            return nullptr;
          }
          if (*Shift + LogMult <= 3) {
            *Index = Var;
            *Shift += LogMult;
            return IndexInst;
          }
        }
        case InstArithmetic::Shl: {
          uint32_t ShiftAmount = Const->getValue();
          switch (ShiftAmount) {
          case 0:
          case 1:
          case 2:
          case 3:
            break;
          default:
            return nullptr;
          }
          if (*Shift + ShiftAmount <= 3) {
            *Index = Var;
            *Shift += ShiftAmount;
            return IndexInst;
          }
        }
        }
      }
    }
  }
  return nullptr;
}

const Inst *AddressOptimizer::matchOffsetIndexOrBase(
    Variable **IndexOrBase, const uint16_t Shift,
    ConstantRelocatable **Relocatable, int32_t *Offset) {
  // Base is Base=Var+Const || Base is Base=Const+Var ==>
  //   set Base=Var, Offset+=Const
  // Base is Base=Var-Const ==>
  //   set Base=Var, Offset-=Const
  // Index is Index=Var+Const ==>
  //   set Index=Var, Offset+=(Const<<Shift)
  // Index is Index=Const+Var ==>
  //   set Index=Var, Offset+=(Const<<Shift)
  // Index is Index=Var-Const ==>
  //   set Index=Var, Offset-=(Const<<Shift)
  // Treat Index=Var Or Const as Index=Var + Const
  //    when Var = Var' << N and log2(Const) <= N
  // or when Var = (2^M) * (2^N) and log2(Const) <= (M+N)

  if (*IndexOrBase == nullptr) {
    return nullptr;
  }
  const Inst *Definition = VMetadata->getSingleDefinition(*IndexOrBase);
  if (Definition == nullptr) {
    return nullptr;
  }
  assert(!VMetadata->isMultiDef(*IndexOrBase));
  if (auto *ArithInst = llvm::dyn_cast<const InstArithmetic>(Definition)) {
    switch (ArithInst->getOp()) {
    case InstArithmetic::Add:
    case InstArithmetic::Sub:
    case InstArithmetic::Or:
      break;
    default:
      return nullptr;
    }

    Operand *Src0 = ArithInst->getSrc(0);
    Operand *Src1 = ArithInst->getSrc(1);
    auto *Var0 = llvm::dyn_cast<Variable>(Src0);
    auto *Var1 = llvm::dyn_cast<Variable>(Src1);
    auto *Const0 = llvm::dyn_cast<ConstantInteger32>(Src0);
    auto *Const1 = llvm::dyn_cast<ConstantInteger32>(Src1);
    auto *Reloc0 = llvm::dyn_cast<ConstantRelocatable>(Src0);
    auto *Reloc1 = llvm::dyn_cast<ConstantRelocatable>(Src1);

    bool IsAdd = false;
    if (ArithInst->getOp() == InstArithmetic::Or) {
      Variable *Var = nullptr;
      ConstantInteger32 *Const = nullptr;
      if (Var0 && Const1) {
        Var = Var0;
        Const = Const1;
      } else if (Const0 && Var1) {
        Var = Var1;
        Const = Const0;
      } else {
        return nullptr;
      }
      auto *VarDef =
          llvm::dyn_cast<InstArithmetic>(VMetadata->getSingleDefinition(Var));
      if (VarDef == nullptr)
        return nullptr;

      SizeT ZeroesAvailable = 0;
      if (VarDef->getOp() == InstArithmetic::Shl) {
        if (auto *ConstInt =
                llvm::dyn_cast<ConstantInteger32>(VarDef->getSrc(1))) {
          ZeroesAvailable = ConstInt->getValue();
        }
      } else if (VarDef->getOp() == InstArithmetic::Mul) {
        SizeT PowerOfTwo = 0;
        if (auto *MultConst =
                llvm::dyn_cast<ConstantInteger32>(VarDef->getSrc(0))) {
          if (llvm::isPowerOf2_32(MultConst->getValue())) {
            PowerOfTwo += MultConst->getValue();
          }
        }
        if (auto *MultConst =
                llvm::dyn_cast<ConstantInteger32>(VarDef->getSrc(1))) {
          if (llvm::isPowerOf2_32(MultConst->getValue())) {
            PowerOfTwo += MultConst->getValue();
          }
        }
        ZeroesAvailable = llvm::Log2_32(PowerOfTwo) + 1;
      }
      SizeT ZeroesNeeded = llvm::Log2_32(Const->getValue()) + 1;
      if (ZeroesNeeded == 0 || ZeroesNeeded > ZeroesAvailable)
        return nullptr;
      IsAdd = true; // treat it as an add if the above conditions hold
    } else {
      IsAdd = ArithInst->getOp() == InstArithmetic::Add;
    }

    Variable *NewIndexOrBase = nullptr;
    int32_t NewOffset = 0;
    ConstantRelocatable *NewRelocatable = *Relocatable;
    if (Var0 && Var1)
      // TODO(sehr): merge base/index splitting into here.
      return nullptr;
    if (!IsAdd && Var1)
      return nullptr;
    if (Var0)
      NewIndexOrBase = Var0;
    else if (Var1)
      NewIndexOrBase = Var1;
    // Don't know how to add/subtract two relocatables.
    if ((*Relocatable && (Reloc0 || Reloc1)) || (Reloc0 && Reloc1))
      return nullptr;
    // Don't know how to subtract a relocatable.
    if (!IsAdd && Reloc1)
      return nullptr;
    // Incorporate ConstantRelocatables.
    if (Reloc0)
      NewRelocatable = Reloc0;
    else if (Reloc1)
      NewRelocatable = Reloc1;
    // Compute the updated constant offset.
    if (Const0) {
      const int32_t MoreOffset =
          IsAdd ? Const0->getValue() : -Const0->getValue();
      if (Utils::WouldOverflowAdd(*Offset + NewOffset, MoreOffset))
        return nullptr;
      NewOffset += MoreOffset;
    }
    if (Const1) {
      const int32_t MoreOffset =
          IsAdd ? Const1->getValue() : -Const1->getValue();
      if (Utils::WouldOverflowAdd(*Offset + NewOffset, MoreOffset))
        return nullptr;
      NewOffset += MoreOffset;
    }
    if (Utils::WouldOverflowAdd(*Offset, NewOffset << Shift))
      return nullptr;
    *IndexOrBase = NewIndexOrBase;
    *Offset += (NewOffset << Shift);
    // Shift is always zero if this is called with the base
    *Relocatable = NewRelocatable;
    return Definition;
  }
  return nullptr;
}

X86OperandMem *TargetX8632::computeAddressOpt(const Inst *Instr, Type MemType,
                                              Operand *Addr) {
  Func->resetCurrentNode();
  if (Func->isVerbose(IceV_AddrOpt)) {
    OstreamLocker L(Func->getContext());
    Ostream &Str = Func->getContext()->getStrDump();
    Str << "\nStarting computeAddressOpt for instruction:\n  ";
    Instr->dumpDecorated(Func);
  }

  OptAddr NewAddr;
  NewAddr.Base = llvm::dyn_cast<Variable>(Addr);
  if (NewAddr.Base == nullptr)
    return nullptr;

  // If the Base has more than one use or is live across multiple blocks, then
  // don't go further. Alternatively (?), never consider a transformation that
  // would change a variable that is currently *not* live across basic block
  // boundaries into one that *is*.
  if (!getFlags().getLoopInvariantCodeMotion()) {
    // Need multi block address opt when licm is enabled.
    // Might make sense to restrict to current node and loop header.
    if (Func->getVMetadata()->isMultiBlock(
            NewAddr.Base) /* || Base->getUseCount() > 1*/)
      return nullptr;
  }
  AddressOptimizer AddrOpt(Func);
  const bool MockBounds = getFlags().getMockBoundsCheck();
  const Inst *Reason = nullptr;
  bool AddressWasOptimized = false;
  // The following unnamed struct identifies the address mode formation steps
  // that could potentially create an invalid memory operand (i.e., no free
  // slots for RebasePtr.) We add all those variables to this struct so that
  // we can use memset() to reset all members to false.
  struct {
    bool AssignBase = false;
    bool AssignIndex = false;
    bool OffsetFromBase = false;
    bool OffsetFromIndex = false;
    bool CombinedBaseIndex = false;
  } Skip;
  // NewAddrCheckpoint is used to rollback the address being formed in case an
  // invalid address is formed.
  OptAddr NewAddrCheckpoint;
  Reason = Instr;
  do {
    if (Reason) {
      AddrOpt.dumpAddressOpt(NewAddr.Relocatable, NewAddr.Offset, NewAddr.Base,
                             NewAddr.Index, NewAddr.Shift, Reason);
      AddressWasOptimized = true;
      Reason = nullptr;
      memset(reinterpret_cast<void *>(&Skip), 0, sizeof(Skip));
    }

    NewAddrCheckpoint = NewAddr;

    // Update Base and Index to follow through assignments to definitions.
    if (!Skip.AssignBase &&
        (Reason = AddrOpt.matchAssign(&NewAddr.Base, &NewAddr.Relocatable,
                                      &NewAddr.Offset))) {
      // Assignments of Base from a Relocatable or ConstantInt32 can result
      // in Base becoming nullptr.  To avoid code duplication in this loop we
      // prefer that Base be non-nullptr if possible.
      if ((NewAddr.Base == nullptr) && (NewAddr.Index != nullptr) &&
          NewAddr.Shift == 0) {
        std::swap(NewAddr.Base, NewAddr.Index);
      }
      continue;
    }
    if (!Skip.AssignBase &&
        (Reason = AddrOpt.matchAssign(&NewAddr.Index, &NewAddr.Relocatable,
                                      &NewAddr.Offset))) {
      continue;
    }

    if (!MockBounds) {
      // Transition from:
      //   <Relocatable + Offset>(Base) to
      //   <Relocatable + Offset>(Base, Index)
      if (!Skip.CombinedBaseIndex &&
          (Reason = AddrOpt.matchCombinedBaseIndex(
               &NewAddr.Base, &NewAddr.Index, &NewAddr.Shift))) {
        continue;
      }

      // Recognize multiply/shift and update Shift amount.
      // Index becomes Index=Var<<Const && Const+Shift<=3 ==>
      //   Index=Var, Shift+=Const
      // Index becomes Index=Const*Var && log2(Const)+Shift<=3 ==>
      //   Index=Var, Shift+=log2(Const)
      if ((Reason =
               AddrOpt.matchShiftedIndex(&NewAddr.Index, &NewAddr.Shift))) {
        continue;
      }

      // If Shift is zero, the choice of Base and Index was purely arbitrary.
      // Recognize multiply/shift and set Shift amount.
      // Shift==0 && Base is Base=Var*Const && log2(Const)+Shift<=3 ==>
      //   swap(Index,Base)
      // Similar for Base=Const*Var and Base=Var<<Const
      if (NewAddr.Shift == 0 &&
          (Reason = AddrOpt.matchShiftedIndex(&NewAddr.Base, &NewAddr.Shift))) {
        std::swap(NewAddr.Base, NewAddr.Index);
        continue;
      }
    }

    // Update Offset to reflect additions/subtractions with constants and
    // relocatables.
    // TODO: consider overflow issues with respect to Offset.
    if (!Skip.OffsetFromBase && (Reason = AddrOpt.matchOffsetIndexOrBase(
                                     &NewAddr.Base, /*Shift =*/0,
                                     &NewAddr.Relocatable, &NewAddr.Offset))) {
      continue;
    }
    if (!Skip.OffsetFromIndex && (Reason = AddrOpt.matchOffsetIndexOrBase(
                                      &NewAddr.Index, NewAddr.Shift,
                                      &NewAddr.Relocatable, &NewAddr.Offset))) {
      continue;
    }

    break;
  } while (Reason);

  if (!AddressWasOptimized) {
    return nullptr;
  }

  // Undo any addition of RebasePtr.  It will be added back when the mem
  // operand is sandboxed.
  if (NewAddr.Base == RebasePtr) {
    NewAddr.Base = nullptr;
  }

  if (NewAddr.Index == RebasePtr) {
    NewAddr.Index = nullptr;
    NewAddr.Shift = 0;
  }

  Constant *OffsetOp = nullptr;
  if (NewAddr.Relocatable == nullptr) {
    OffsetOp = Ctx->getConstantInt32(NewAddr.Offset);
  } else {
    OffsetOp =
        Ctx->getConstantSym(NewAddr.Relocatable->getOffset() + NewAddr.Offset,
                            NewAddr.Relocatable->getName());
  }
  // Vanilla ICE load instructions should not use the segment registers, and
  // computeAddressOpt only works at the level of Variables and Constants, not
  // other X86OperandMem, so there should be no mention of segment
  // registers there either.
  static constexpr auto SegmentReg =
      X86OperandMem::SegmentRegisters::DefaultSegment;

  return X86OperandMem::create(Func, MemType, NewAddr.Base, OffsetOp,
                               NewAddr.Index, NewAddr.Shift, SegmentReg);
}

/// Add a mock bounds check on the memory address before using it as a load or
/// store operand.  The basic idea is that given a memory operand [reg], we
/// would first add bounds-check code something like:
///
///   cmp reg, <lb>
///   jl out_of_line_error
///   cmp reg, <ub>
///   jg out_of_line_error
///
/// In reality, the specific code will depend on how <lb> and <ub> are
/// represented, e.g. an immediate, a global, or a function argument.
///
/// As such, we need to enforce that the memory operand does not have the form
/// [reg1+reg2], because then there is no simple cmp instruction that would
/// suffice.  However, we consider [reg+offset] to be OK because the offset is
/// usually small, and so <ub> could have a safety buffer built in and then we
/// could instead branch to a custom out_of_line_error that does the precise
/// check and jumps back if it turns out OK.
///
/// For the purpose of mocking the bounds check, we'll do something like this:
///
///   cmp reg, 0
///   je label
///   cmp reg, 1
///   je label
///   label:
///
/// Also note that we don't need to add a bounds check to a dereference of a
/// simple global variable address.

void TargetX8632::doMockBoundsCheck(Operand *Opnd) {
  if (!getFlags().getMockBoundsCheck())
    return;
  if (auto *Mem = llvm::dyn_cast<X86OperandMem>(Opnd)) {
    if (Mem->getIndex()) {
      llvm::report_fatal_error("doMockBoundsCheck: Opnd contains index reg");
    }
    Opnd = Mem->getBase();
  }
  // At this point Opnd could be nullptr, or Variable, or Constant, or perhaps
  // something else.  We only care if it is Variable.
  auto *Var = llvm::dyn_cast_or_null<Variable>(Opnd);
  if (Var == nullptr)
    return;
  // We use lowerStore() to copy out-args onto the stack.  This creates a
  // memory operand with the stack pointer as the base register.  Don't do
  // bounds checks on that.
  if (Var->getRegNum() == getStackReg())
    return;

  auto *Label = InstX86Label::create(Func, this);
  _cmp(Opnd, Ctx->getConstantZero(IceType_i32));
  _br(CondX86::Br_e, Label);
  _cmp(Opnd, Ctx->getConstantInt32(1));
  _br(CondX86::Br_e, Label);
  Context.insert(Label);
}

void TargetX8632::lowerLoad(const InstLoad *Load) {
  // A Load instruction can be treated the same as an Assign instruction,
  // after the source operand is transformed into an X86OperandMem operand.
  // Note that the address mode optimization already creates an X86OperandMem
  // operand, so it doesn't need another level of transformation.
  Variable *DestLoad = Load->getDest();
  Type Ty = DestLoad->getType();
  Operand *Src0 = formMemoryOperand(Load->getLoadAddress(), Ty);
  doMockBoundsCheck(Src0);
  auto *Assign = InstAssign::create(Func, DestLoad, Src0);
  lowerAssign(Assign);
}

void TargetX8632::doAddressOptOther() {
  // Inverts some Icmp instructions which helps doAddressOptLoad later.
  // TODO(manasijm): Refactor to unify the conditions for Var0 and Var1
  Inst *Instr = iteratorToInst(Context.getCur());
  auto *VMetadata = Func->getVMetadata();
  if (auto *Icmp = llvm::dyn_cast<InstIcmp>(Instr)) {
    if (llvm::isa<Constant>(Icmp->getSrc(0)) ||
        llvm::isa<Constant>(Icmp->getSrc(1)))
      return;
    auto *Var0 = llvm::dyn_cast<Variable>(Icmp->getSrc(0));
    if (Var0 == nullptr)
      return;
    if (!VMetadata->isTracked(Var0))
      return;
    auto *Op0Def = VMetadata->getFirstDefinitionSingleBlock(Var0);
    if (Op0Def == nullptr || !llvm::isa<InstLoad>(Op0Def))
      return;
    if (VMetadata->getLocalUseNode(Var0) != Context.getNode())
      return;

    auto *Var1 = llvm::dyn_cast<Variable>(Icmp->getSrc(1));
    if (Var1 != nullptr && VMetadata->isTracked(Var1)) {
      auto *Op1Def = VMetadata->getFirstDefinitionSingleBlock(Var1);
      if (Op1Def != nullptr && !VMetadata->isMultiBlock(Var1) &&
          llvm::isa<InstLoad>(Op1Def)) {
        return; // Both are loads
      }
    }
    Icmp->reverseConditionAndOperands();
  }
}

void TargetX8632::doAddressOptLoad() {
  Inst *Instr = iteratorToInst(Context.getCur());
  Operand *Addr = Instr->getSrc(0);
  Variable *Dest = Instr->getDest();
  if (auto *OptAddr = computeAddressOpt(Instr, Dest->getType(), Addr)) {
    Instr->setDeleted();
    Context.insert<InstLoad>(Dest, OptAddr);
  }
}

void TargetX8632::doAddressOptLoadSubVector() {
  auto *Intrinsic = llvm::cast<InstIntrinsic>(Context.getCur());
  Operand *Addr = Intrinsic->getArg(0);
  Variable *Dest = Intrinsic->getDest();
  if (auto *OptAddr = computeAddressOpt(Intrinsic, Dest->getType(), Addr)) {
    Intrinsic->setDeleted();
    const Ice::Intrinsics::IntrinsicInfo Info = {
        Ice::Intrinsics::LoadSubVector, Ice::Intrinsics::SideEffects_F,
        Ice::Intrinsics::ReturnsTwice_F, Ice::Intrinsics::MemoryWrite_F};
    auto *NewLoad = Context.insert<InstIntrinsic>(2, Dest, Info);
    NewLoad->addArg(OptAddr);
    NewLoad->addArg(Intrinsic->getArg(1));
  }
}

void TargetX8632::lowerPhi(const InstPhi * /*Instr*/) {
  Func->setError("Phi found in regular instruction list");
}

void TargetX8632::lowerRet(const InstRet *Instr) {
  Variable *Reg = nullptr;
  if (Instr->hasRetValue()) {
    Operand *RetValue = legalize(Instr->getRetValue());
    const Type ReturnType = RetValue->getType();
    assert(isVectorType(ReturnType) || isScalarFloatingType(ReturnType) ||
           (ReturnType == IceType_i32) || (ReturnType == IceType_i64));
    Reg = moveReturnValueToRegister(RetValue, ReturnType);
  }
  // Add a ret instruction even if sandboxing is enabled, because addEpilog
  // explicitly looks for a ret instruction as a marker for where to insert
  // the frame removal instructions.
  _ret(Reg);
  // Add a fake use of esp to make sure esp stays alive for the entire
  // function. Otherwise post-call esp adjustments get dead-code eliminated.
  keepEspLiveAtExit();
}

inline uint32_t makePshufdMask(SizeT Index0, SizeT Index1, SizeT Index2,
                               SizeT Index3) {
  const SizeT Mask = (Index0 & 0x3) | ((Index1 & 0x3) << 2) |
                     ((Index2 & 0x3) << 4) | ((Index3 & 0x3) << 6);
  assert(Mask < 256);
  return Mask;
}

Variable *TargetX8632::lowerShuffleVector_AllFromSameSrc(
    Operand *Src, SizeT Index0, SizeT Index1, SizeT Index2, SizeT Index3) {
  constexpr SizeT SrcBit = 1 << 2;
  assert((Index0 & SrcBit) == (Index1 & SrcBit));
  assert((Index0 & SrcBit) == (Index2 & SrcBit));
  assert((Index0 & SrcBit) == (Index3 & SrcBit));
  (void)SrcBit;

  const Type SrcTy = Src->getType();
  auto *T = makeReg(SrcTy);
  auto *SrcRM = legalize(Src, Legal_Reg | Legal_Mem);
  auto *Mask =
      Ctx->getConstantInt32(makePshufdMask(Index0, Index1, Index2, Index3));
  _pshufd(T, SrcRM, Mask);
  return T;
}

Variable *
TargetX8632::lowerShuffleVector_TwoFromSameSrc(Operand *Src0, SizeT Index0,
                                               SizeT Index1, Operand *Src1,
                                               SizeT Index2, SizeT Index3) {
  constexpr SizeT SrcBit = 1 << 2;
  assert((Index0 & SrcBit) == (Index1 & SrcBit) || (Index1 == IGNORE_INDEX));
  assert((Index2 & SrcBit) == (Index3 & SrcBit) || (Index3 == IGNORE_INDEX));
  (void)SrcBit;

  const Type SrcTy = Src0->getType();
  assert(Src1->getType() == SrcTy);
  auto *T = makeReg(SrcTy);
  auto *Src0R = legalizeToReg(Src0);
  auto *Src1RM = legalize(Src1, Legal_Reg | Legal_Mem);
  auto *Mask =
      Ctx->getConstantInt32(makePshufdMask(Index0, Index1, Index2, Index3));
  _movp(T, Src0R);
  _shufps(T, Src1RM, Mask);
  return T;
}

Variable *TargetX8632::lowerShuffleVector_UnifyFromDifferentSrcs(Operand *Src0,
                                                                 SizeT Index0,
                                                                 Operand *Src1,
                                                                 SizeT Index1) {
  return lowerShuffleVector_TwoFromSameSrc(Src0, Index0, IGNORE_INDEX, Src1,
                                           Index1, IGNORE_INDEX);
}

inline SizeT makeSrcSwitchMask(SizeT Index0, SizeT Index1, SizeT Index2,
                               SizeT Index3) {
  constexpr SizeT SrcBit = 1 << 2;
  const SizeT Index0Bits = ((Index0 & SrcBit) == 0) ? 0 : (1 << 0);
  const SizeT Index1Bits = ((Index1 & SrcBit) == 0) ? 0 : (1 << 1);
  const SizeT Index2Bits = ((Index2 & SrcBit) == 0) ? 0 : (1 << 2);
  const SizeT Index3Bits = ((Index3 & SrcBit) == 0) ? 0 : (1 << 3);
  return Index0Bits | Index1Bits | Index2Bits | Index3Bits;
}

GlobalString TargetX8632::lowerShuffleVector_NewMaskName() {
  GlobalString FuncName = Func->getFunctionName();
  const SizeT Id = PshufbMaskCount++;
  if (!BuildDefs::dump() || !FuncName.hasStdString()) {
    return GlobalString::createWithString(
        Ctx,
        "$PS" + std::to_string(FuncName.getID()) + "_" + std::to_string(Id));
  }
  return GlobalString::createWithString(
      Ctx, "Pshufb$" + Func->getFunctionName() + "$" + std::to_string(Id));
}

ConstantRelocatable *TargetX8632::lowerShuffleVector_CreatePshufbMask(
    int8_t Idx0, int8_t Idx1, int8_t Idx2, int8_t Idx3, int8_t Idx4,
    int8_t Idx5, int8_t Idx6, int8_t Idx7, int8_t Idx8, int8_t Idx9,
    int8_t Idx10, int8_t Idx11, int8_t Idx12, int8_t Idx13, int8_t Idx14,
    int8_t Idx15) {
  static constexpr uint8_t NumElements = 16;
  const char Initializer[NumElements] = {
      Idx0, Idx1, Idx2,  Idx3,  Idx4,  Idx5,  Idx6,  Idx7,
      Idx8, Idx9, Idx10, Idx11, Idx12, Idx13, Idx14, Idx15,
  };

  static constexpr Type V4VectorType = IceType_v4i32;
  const uint32_t MaskAlignment = typeWidthInBytesOnStack(V4VectorType);
  auto *Mask = VariableDeclaration::create(Func->getGlobalPool());
  GlobalString MaskName = lowerShuffleVector_NewMaskName();
  Mask->setIsConstant(true);
  Mask->addInitializer(VariableDeclaration::DataInitializer::create(
      Func->getGlobalPool(), Initializer, NumElements));
  Mask->setName(MaskName);
  // Mask needs to be 16-byte aligned, or pshufb will seg fault.
  Mask->setAlignment(MaskAlignment);
  Func->addGlobal(Mask);

  constexpr RelocOffsetT Offset = 0;
  return llvm::cast<ConstantRelocatable>(Ctx->getConstantSym(Offset, MaskName));
}

void TargetX8632::lowerShuffleVector_UsingPshufb(
    Variable *Dest, Operand *Src0, Operand *Src1, int8_t Idx0, int8_t Idx1,
    int8_t Idx2, int8_t Idx3, int8_t Idx4, int8_t Idx5, int8_t Idx6,
    int8_t Idx7, int8_t Idx8, int8_t Idx9, int8_t Idx10, int8_t Idx11,
    int8_t Idx12, int8_t Idx13, int8_t Idx14, int8_t Idx15) {
  const Type DestTy = Dest->getType();
  static constexpr bool NotRebased = false;
  static constexpr Variable *NoBase = nullptr;
  // We use void for the memory operand instead of DestTy because using the
  // latter causes a validation failure: the X86 Inst layer complains that
  // vector mem operands could be under aligned. Thus, using void we avoid the
  // validation error. Note that the mask global declaration is aligned, so it
  // can be used as an XMM mem operand.
  static constexpr Type MaskType = IceType_void;
#define IDX_IN_SRC(N, S)                                                       \
  ((((N) & (1 << 4)) == (S << 4)) ? ((N)&0xf) : CLEAR_ALL_BITS)
  auto *Mask0M = X86OperandMem::create(
      Func, MaskType, NoBase,
      lowerShuffleVector_CreatePshufbMask(
          IDX_IN_SRC(Idx0, 0), IDX_IN_SRC(Idx1, 0), IDX_IN_SRC(Idx2, 0),
          IDX_IN_SRC(Idx3, 0), IDX_IN_SRC(Idx4, 0), IDX_IN_SRC(Idx5, 0),
          IDX_IN_SRC(Idx6, 0), IDX_IN_SRC(Idx7, 0), IDX_IN_SRC(Idx8, 0),
          IDX_IN_SRC(Idx9, 0), IDX_IN_SRC(Idx10, 0), IDX_IN_SRC(Idx11, 0),
          IDX_IN_SRC(Idx12, 0), IDX_IN_SRC(Idx13, 0), IDX_IN_SRC(Idx14, 0),
          IDX_IN_SRC(Idx15, 0)),
      NotRebased);

  auto *T0 = makeReg(DestTy);
  auto *Src0RM = legalize(Src0, Legal_Reg | Legal_Mem);
  _movp(T0, Src0RM);

  _pshufb(T0, Mask0M);

  if (Idx0 >= 16 || Idx1 >= 16 || Idx2 >= 16 || Idx3 >= 16 || Idx4 >= 16 ||
      Idx5 >= 16 || Idx6 >= 16 || Idx7 >= 16 || Idx8 >= 16 || Idx9 >= 16 ||
      Idx10 >= 16 || Idx11 >= 16 || Idx12 >= 16 || Idx13 >= 16 || Idx14 >= 16 ||
      Idx15 >= 16) {
    auto *Mask1M = X86OperandMem::create(
        Func, MaskType, NoBase,
        lowerShuffleVector_CreatePshufbMask(
            IDX_IN_SRC(Idx0, 1), IDX_IN_SRC(Idx1, 1), IDX_IN_SRC(Idx2, 1),
            IDX_IN_SRC(Idx3, 1), IDX_IN_SRC(Idx4, 1), IDX_IN_SRC(Idx5, 1),
            IDX_IN_SRC(Idx6, 1), IDX_IN_SRC(Idx7, 1), IDX_IN_SRC(Idx8, 1),
            IDX_IN_SRC(Idx9, 1), IDX_IN_SRC(Idx10, 1), IDX_IN_SRC(Idx11, 1),
            IDX_IN_SRC(Idx12, 1), IDX_IN_SRC(Idx13, 1), IDX_IN_SRC(Idx14, 1),
            IDX_IN_SRC(Idx15, 1)),
        NotRebased);
#undef IDX_IN_SRC
    auto *T1 = makeReg(DestTy);
    auto *Src1RM = legalize(Src1, Legal_Reg | Legal_Mem);
    _movp(T1, Src1RM);
    _pshufb(T1, Mask1M);
    _por(T0, T1);
  }

  _movp(Dest, T0);
}

void TargetX8632::lowerShuffleVector(const InstShuffleVector *Instr) {
  auto *Dest = Instr->getDest();
  const Type DestTy = Dest->getType();
  auto *Src0 = Instr->getSrc(0);
  auto *Src1 = Instr->getSrc(1);
  const SizeT NumElements = typeNumElements(DestTy);

  auto *T = makeReg(DestTy);

  switch (DestTy) {
  default:
    llvm::report_fatal_error("Unexpected vector type.");
  case IceType_v16i1:
  case IceType_v16i8: {
    static constexpr SizeT ExpectedNumElements = 16;
    assert(ExpectedNumElements == Instr->getNumIndexes());
    (void)ExpectedNumElements;

    if (Instr->indexesAre(0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7)) {
      auto *T = makeReg(DestTy);
      auto *Src0RM = legalize(Src0, Legal_Reg | Legal_Mem);
      _movp(T, Src0RM);
      _punpckl(T, Src0RM);
      _movp(Dest, T);
      return;
    }

    if (Instr->indexesAre(0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7,
                          23)) {
      auto *T = makeReg(DestTy);
      auto *Src0RM = legalize(Src0, Legal_Reg | Legal_Mem);
      auto *Src1RM = legalize(Src1, Legal_Reg | Legal_Mem);
      _movp(T, Src0RM);
      _punpckl(T, Src1RM);
      _movp(Dest, T);
      return;
    }

    if (Instr->indexesAre(8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14,
                          15, 15)) {
      auto *T = makeReg(DestTy);
      auto *Src0RM = legalize(Src0, Legal_Reg | Legal_Mem);
      _movp(T, Src0RM);
      _punpckh(T, Src0RM);
      _movp(Dest, T);
      return;
    }

    if (Instr->indexesAre(8, 24, 9, 25, 10, 26, 11, 27, 12, 28, 13, 29, 14, 30,
                          15, 31)) {
      auto *T = makeReg(DestTy);
      auto *Src0RM = legalize(Src0, Legal_Reg | Legal_Mem);
      auto *Src1RM = legalize(Src1, Legal_Reg | Legal_Mem);
      _movp(T, Src0RM);
      _punpckh(T, Src1RM);
      _movp(Dest, T);
      return;
    }

    if (InstructionSet < SSE4_1) {
      // TODO(jpp): figure out how to lower with sse2.
      break;
    }

    const SizeT Index0 = Instr->getIndexValue(0);
    const SizeT Index1 = Instr->getIndexValue(1);
    const SizeT Index2 = Instr->getIndexValue(2);
    const SizeT Index3 = Instr->getIndexValue(3);
    const SizeT Index4 = Instr->getIndexValue(4);
    const SizeT Index5 = Instr->getIndexValue(5);
    const SizeT Index6 = Instr->getIndexValue(6);
    const SizeT Index7 = Instr->getIndexValue(7);
    const SizeT Index8 = Instr->getIndexValue(8);
    const SizeT Index9 = Instr->getIndexValue(9);
    const SizeT Index10 = Instr->getIndexValue(10);
    const SizeT Index11 = Instr->getIndexValue(11);
    const SizeT Index12 = Instr->getIndexValue(12);
    const SizeT Index13 = Instr->getIndexValue(13);
    const SizeT Index14 = Instr->getIndexValue(14);
    const SizeT Index15 = Instr->getIndexValue(15);

    lowerShuffleVector_UsingPshufb(Dest, Src0, Src1, Index0, Index1, Index2,
                                   Index3, Index4, Index5, Index6, Index7,
                                   Index8, Index9, Index10, Index11, Index12,
                                   Index13, Index14, Index15);
    return;
  }
  case IceType_v8i1:
  case IceType_v8i16: {
    static constexpr SizeT ExpectedNumElements = 8;
    assert(ExpectedNumElements == Instr->getNumIndexes());
    (void)ExpectedNumElements;

    if (Instr->indexesAre(0, 0, 1, 1, 2, 2, 3, 3)) {
      auto *T = makeReg(DestTy);
      auto *Src0RM = legalize(Src0, Legal_Reg | Legal_Mem);
      _movp(T, Src0RM);
      _punpckl(T, Src0RM);
      _movp(Dest, T);
      return;
    }

    if (Instr->indexesAre(0, 8, 1, 9, 2, 10, 3, 11)) {
      auto *T = makeReg(DestTy);
      auto *Src0RM = legalize(Src0, Legal_Reg | Legal_Mem);
      auto *Src1RM = legalize(Src1, Legal_Reg | Legal_Mem);
      _movp(T, Src0RM);
      _punpckl(T, Src1RM);
      _movp(Dest, T);
      return;
    }

    if (Instr->indexesAre(4, 4, 5, 5, 6, 6, 7, 7)) {
      auto *T = makeReg(DestTy);
      auto *Src0RM = legalize(Src0, Legal_Reg | Legal_Mem);
      _movp(T, Src0RM);
      _punpckh(T, Src0RM);
      _movp(Dest, T);
      return;
    }

    if (Instr->indexesAre(4, 12, 5, 13, 6, 14, 7, 15)) {
      auto *T = makeReg(DestTy);
      auto *Src0RM = legalize(Src0, Legal_Reg | Legal_Mem);
      auto *Src1RM = legalize(Src1, Legal_Reg | Legal_Mem);
      _movp(T, Src0RM);
      _punpckh(T, Src1RM);
      _movp(Dest, T);
      return;
    }

    if (InstructionSet < SSE4_1) {
      // TODO(jpp): figure out how to lower with sse2.
      break;
    }

    const SizeT Index0 = Instr->getIndexValue(0);
    const SizeT Index1 = Instr->getIndexValue(1);
    const SizeT Index2 = Instr->getIndexValue(2);
    const SizeT Index3 = Instr->getIndexValue(3);
    const SizeT Index4 = Instr->getIndexValue(4);
    const SizeT Index5 = Instr->getIndexValue(5);
    const SizeT Index6 = Instr->getIndexValue(6);
    const SizeT Index7 = Instr->getIndexValue(7);

#define TO_BYTE_INDEX(I) ((I) << 1)
    lowerShuffleVector_UsingPshufb(
        Dest, Src0, Src1, TO_BYTE_INDEX(Index0), TO_BYTE_INDEX(Index0) + 1,
        TO_BYTE_INDEX(Index1), TO_BYTE_INDEX(Index1) + 1, TO_BYTE_INDEX(Index2),
        TO_BYTE_INDEX(Index2) + 1, TO_BYTE_INDEX(Index3),
        TO_BYTE_INDEX(Index3) + 1, TO_BYTE_INDEX(Index4),
        TO_BYTE_INDEX(Index4) + 1, TO_BYTE_INDEX(Index5),
        TO_BYTE_INDEX(Index5) + 1, TO_BYTE_INDEX(Index6),
        TO_BYTE_INDEX(Index6) + 1, TO_BYTE_INDEX(Index7),
        TO_BYTE_INDEX(Index7) + 1);
#undef TO_BYTE_INDEX
    return;
  }
  case IceType_v4i1:
  case IceType_v4i32:
  case IceType_v4f32: {
    static constexpr SizeT ExpectedNumElements = 4;
    assert(ExpectedNumElements == Instr->getNumIndexes());
    const SizeT Index0 = Instr->getIndexValue(0);
    const SizeT Index1 = Instr->getIndexValue(1);
    const SizeT Index2 = Instr->getIndexValue(2);
    const SizeT Index3 = Instr->getIndexValue(3);
    Variable *T = nullptr;
    switch (makeSrcSwitchMask(Index0, Index1, Index2, Index3)) {
#define CASE_SRCS_IN(S0, S1, S2, S3)                                           \
  case (((S0) << 0) | ((S1) << 1) | ((S2) << 2) | ((S3) << 3))
      CASE_SRCS_IN(0, 0, 0, 0) : {
        T = lowerShuffleVector_AllFromSameSrc(Src0, Index0, Index1, Index2,
                                              Index3);
      }
      break;
      CASE_SRCS_IN(0, 0, 0, 1) : {
        auto *Unified = lowerShuffleVector_UnifyFromDifferentSrcs(Src0, Index2,
                                                                  Src1, Index3);
        T = lowerShuffleVector_TwoFromSameSrc(Src0, Index0, Index1, Unified,
                                              UNIFIED_INDEX_0, UNIFIED_INDEX_1);
      }
      break;
      CASE_SRCS_IN(0, 0, 1, 0) : {
        auto *Unified = lowerShuffleVector_UnifyFromDifferentSrcs(Src1, Index2,
                                                                  Src0, Index3);
        T = lowerShuffleVector_TwoFromSameSrc(Src0, Index0, Index1, Unified,
                                              UNIFIED_INDEX_0, UNIFIED_INDEX_1);
      }
      break;
      CASE_SRCS_IN(0, 0, 1, 1) : {
        T = lowerShuffleVector_TwoFromSameSrc(Src0, Index0, Index1, Src1,
                                              Index2, Index3);
      }
      break;
      CASE_SRCS_IN(0, 1, 0, 0) : {
        auto *Unified = lowerShuffleVector_UnifyFromDifferentSrcs(Src0, Index0,
                                                                  Src1, Index1);
        T = lowerShuffleVector_TwoFromSameSrc(
            Unified, UNIFIED_INDEX_0, UNIFIED_INDEX_1, Src0, Index2, Index3);
      }
      break;
      CASE_SRCS_IN(0, 1, 0, 1) : {
        if (Index0 == 0 && (Index1 - ExpectedNumElements) == 0 && Index2 == 1 &&
            (Index3 - ExpectedNumElements) == 1) {
          auto *Src1RM = legalize(Src1, Legal_Reg | Legal_Mem);
          auto *Src0R = legalizeToReg(Src0);
          T = makeReg(DestTy);
          _movp(T, Src0R);
          _punpckl(T, Src1RM);
        } else if (Index0 == Index2 && Index1 == Index3) {
          auto *Unified = lowerShuffleVector_UnifyFromDifferentSrcs(
              Src0, Index0, Src1, Index1);
          T = lowerShuffleVector_AllFromSameSrc(
              Unified, UNIFIED_INDEX_0, UNIFIED_INDEX_1, UNIFIED_INDEX_0,
              UNIFIED_INDEX_1);
        } else {
          auto *Unified0 = lowerShuffleVector_UnifyFromDifferentSrcs(
              Src0, Index0, Src1, Index1);
          auto *Unified1 = lowerShuffleVector_UnifyFromDifferentSrcs(
              Src0, Index2, Src1, Index3);
          T = lowerShuffleVector_TwoFromSameSrc(
              Unified0, UNIFIED_INDEX_0, UNIFIED_INDEX_1, Unified1,
              UNIFIED_INDEX_0, UNIFIED_INDEX_1);
        }
      }
      break;
      CASE_SRCS_IN(0, 1, 1, 0) : {
        if (Index0 == Index3 && Index1 == Index2) {
          auto *Unified = lowerShuffleVector_UnifyFromDifferentSrcs(
              Src0, Index0, Src1, Index1);
          T = lowerShuffleVector_AllFromSameSrc(
              Unified, UNIFIED_INDEX_0, UNIFIED_INDEX_1, UNIFIED_INDEX_1,
              UNIFIED_INDEX_0);
        } else {
          auto *Unified0 = lowerShuffleVector_UnifyFromDifferentSrcs(
              Src0, Index0, Src1, Index1);
          auto *Unified1 = lowerShuffleVector_UnifyFromDifferentSrcs(
              Src1, Index2, Src0, Index3);
          T = lowerShuffleVector_TwoFromSameSrc(
              Unified0, UNIFIED_INDEX_0, UNIFIED_INDEX_1, Unified1,
              UNIFIED_INDEX_0, UNIFIED_INDEX_1);
        }
      }
      break;
      CASE_SRCS_IN(0, 1, 1, 1) : {
        auto *Unified = lowerShuffleVector_UnifyFromDifferentSrcs(Src0, Index0,
                                                                  Src1, Index1);
        T = lowerShuffleVector_TwoFromSameSrc(
            Unified, UNIFIED_INDEX_0, UNIFIED_INDEX_1, Src1, Index2, Index3);
      }
      break;
      CASE_SRCS_IN(1, 0, 0, 0) : {
        auto *Unified = lowerShuffleVector_UnifyFromDifferentSrcs(Src1, Index0,
                                                                  Src0, Index1);
        T = lowerShuffleVector_TwoFromSameSrc(
            Unified, UNIFIED_INDEX_0, UNIFIED_INDEX_1, Src0, Index2, Index3);
      }
      break;
      CASE_SRCS_IN(1, 0, 0, 1) : {
        if (Index0 == Index3 && Index1 == Index2) {
          auto *Unified = lowerShuffleVector_UnifyFromDifferentSrcs(
              Src1, Index0, Src0, Index1);
          T = lowerShuffleVector_AllFromSameSrc(
              Unified, UNIFIED_INDEX_0, UNIFIED_INDEX_1, UNIFIED_INDEX_1,
              UNIFIED_INDEX_0);
        } else {
          auto *Unified0 = lowerShuffleVector_UnifyFromDifferentSrcs(
              Src1, Index0, Src0, Index1);
          auto *Unified1 = lowerShuffleVector_UnifyFromDifferentSrcs(
              Src0, Index2, Src1, Index3);
          T = lowerShuffleVector_TwoFromSameSrc(
              Unified0, UNIFIED_INDEX_0, UNIFIED_INDEX_1, Unified1,
              UNIFIED_INDEX_0, UNIFIED_INDEX_1);
        }
      }
      break;
      CASE_SRCS_IN(1, 0, 1, 0) : {
        if ((Index0 - ExpectedNumElements) == 0 && Index1 == 0 &&
            (Index2 - ExpectedNumElements) == 1 && Index3 == 1) {
          auto *Src1RM = legalize(Src0, Legal_Reg | Legal_Mem);
          auto *Src0R = legalizeToReg(Src1);
          T = makeReg(DestTy);
          _movp(T, Src0R);
          _punpckl(T, Src1RM);
        } else if (Index0 == Index2 && Index1 == Index3) {
          auto *Unified = lowerShuffleVector_UnifyFromDifferentSrcs(
              Src1, Index0, Src0, Index1);
          T = lowerShuffleVector_AllFromSameSrc(
              Unified, UNIFIED_INDEX_0, UNIFIED_INDEX_1, UNIFIED_INDEX_0,
              UNIFIED_INDEX_1);
        } else {
          auto *Unified0 = lowerShuffleVector_UnifyFromDifferentSrcs(
              Src1, Index0, Src0, Index1);
          auto *Unified1 = lowerShuffleVector_UnifyFromDifferentSrcs(
              Src1, Index2, Src0, Index3);
          T = lowerShuffleVector_TwoFromSameSrc(
              Unified0, UNIFIED_INDEX_0, UNIFIED_INDEX_1, Unified1,
              UNIFIED_INDEX_0, UNIFIED_INDEX_1);
        }
      }
      break;
      CASE_SRCS_IN(1, 0, 1, 1) : {
        auto *Unified = lowerShuffleVector_UnifyFromDifferentSrcs(Src1, Index0,
                                                                  Src0, Index1);
        T = lowerShuffleVector_TwoFromSameSrc(
            Unified, UNIFIED_INDEX_0, UNIFIED_INDEX_1, Src1, Index2, Index3);
      }
      break;
      CASE_SRCS_IN(1, 1, 0, 0) : {
        T = lowerShuffleVector_TwoFromSameSrc(Src1, Index0, Index1, Src0,
                                              Index2, Index3);
      }
      break;
      CASE_SRCS_IN(1, 1, 0, 1) : {
        auto *Unified = lowerShuffleVector_UnifyFromDifferentSrcs(Src0, Index2,
                                                                  Src1, Index3);
        T = lowerShuffleVector_TwoFromSameSrc(Src1, Index0, Index1, Unified,
                                              UNIFIED_INDEX_0, UNIFIED_INDEX_1);
      }
      break;
      CASE_SRCS_IN(1, 1, 1, 0) : {
        auto *Unified = lowerShuffleVector_UnifyFromDifferentSrcs(Src1, Index2,
                                                                  Src0, Index3);
        T = lowerShuffleVector_TwoFromSameSrc(Src1, Index0, Index1, Unified,
                                              UNIFIED_INDEX_0, UNIFIED_INDEX_1);
      }
      break;
      CASE_SRCS_IN(1, 1, 1, 1) : {
        T = lowerShuffleVector_AllFromSameSrc(Src1, Index0, Index1, Index2,
                                              Index3);
      }
      break;
#undef CASE_SRCS_IN
    }

    assert(T != nullptr);
    assert(T->getType() == DestTy);
    _movp(Dest, T);
    return;
  } break;
  }

  // Unoptimized shuffle. Perform a series of inserts and extracts.
  Context.insert<InstFakeDef>(T);
  const Type ElementType = typeElementType(DestTy);
  for (SizeT I = 0; I < Instr->getNumIndexes(); ++I) {
    auto *Index = Instr->getIndex(I);
    const SizeT Elem = Index->getValue();
    auto *ExtElmt = makeReg(ElementType);
    if (Elem < NumElements) {
      lowerExtractElement(
          InstExtractElement::create(Func, ExtElmt, Src0, Index));
    } else {
      lowerExtractElement(InstExtractElement::create(
          Func, ExtElmt, Src1, Ctx->getConstantInt32(Elem - NumElements)));
    }
    auto *NewT = makeReg(DestTy);
    lowerInsertElement(InstInsertElement::create(Func, NewT, T, ExtElmt,
                                                 Ctx->getConstantInt32(I)));
    T = NewT;
  }
  _movp(Dest, T);
}

void TargetX8632::lowerSelect(const InstSelect *Select) {
  Variable *Dest = Select->getDest();

  Operand *Condition = Select->getCondition();
  // Handle folding opportunities.
  if (const Inst *Producer = FoldingInfo.getProducerFor(Condition)) {
    assert(Producer->isDeleted());
    switch (BoolFolding::getProducerKind(Producer)) {
    default:
      break;
    case BoolFolding::PK_Icmp32:
    case BoolFolding::PK_Icmp64: {
      lowerIcmpAndConsumer(llvm::cast<InstIcmp>(Producer), Select);
      return;
    }
    case BoolFolding::PK_Fcmp: {
      lowerFcmpAndConsumer(llvm::cast<InstFcmp>(Producer), Select);
      return;
    }
    }
  }

  if (isVectorType(Dest->getType())) {
    lowerSelectVector(Select);
    return;
  }

  Operand *CmpResult = legalize(Condition, Legal_Reg | Legal_Mem);
  Operand *Zero = Ctx->getConstantZero(IceType_i32);
  _cmp(CmpResult, Zero);
  Operand *SrcT = Select->getTrueOperand();
  Operand *SrcF = Select->getFalseOperand();
  const BrCond Cond = CondX86::Br_ne;
  lowerSelectMove(Dest, Cond, SrcT, SrcF);
}

void TargetX8632::lowerSelectMove(Variable *Dest, BrCond Cond, Operand *SrcT,
                                  Operand *SrcF) {
  Type DestTy = Dest->getType();
  if (typeWidthInBytes(DestTy) == 1 || isFloatingType(DestTy)) {
    // The cmov instruction doesn't allow 8-bit or FP operands, so we need
    // explicit control flow.
    // d=cmp e,f; a=d?b:c ==> cmp e,f; a=b; jne L1; a=c; L1:
    auto *Label = InstX86Label::create(Func, this);
    SrcT = legalize(SrcT, Legal_Reg | Legal_Imm);
    _mov(Dest, SrcT);
    _br(Cond, Label);
    SrcF = legalize(SrcF, Legal_Reg | Legal_Imm);
    _redefined(_mov(Dest, SrcF));
    Context.insert(Label);
    return;
  }
  // mov t, SrcF; cmov_cond t, SrcT; mov dest, t
  // But if SrcT is immediate, we might be able to do better, as the cmov
  // instruction doesn't allow an immediate operand:
  // mov t, SrcT; cmov_!cond t, SrcF; mov dest, t
  if (llvm::isa<Constant>(SrcT) && !llvm::isa<Constant>(SrcF)) {
    std::swap(SrcT, SrcF);
    Cond = InstX86Base::getOppositeCondition(Cond);
  }
  if (DestTy == IceType_i64) {
    SrcT = legalizeUndef(SrcT);
    SrcF = legalizeUndef(SrcF);
    // Set the low portion.
    auto *DestLo = llvm::cast<Variable>(loOperand(Dest));
    lowerSelectIntMove(DestLo, Cond, loOperand(SrcT), loOperand(SrcF));
    // Set the high portion.
    auto *DestHi = llvm::cast<Variable>(hiOperand(Dest));
    lowerSelectIntMove(DestHi, Cond, hiOperand(SrcT), hiOperand(SrcF));
    return;
  }

  assert(DestTy == IceType_i16 || DestTy == IceType_i32);
  lowerSelectIntMove(Dest, Cond, SrcT, SrcF);
}

void TargetX8632::lowerSelectIntMove(Variable *Dest, BrCond Cond, Operand *SrcT,
                                     Operand *SrcF) {
  Variable *T = nullptr;
  SrcF = legalize(SrcF);
  _mov(T, SrcF);
  SrcT = legalize(SrcT, Legal_Reg | Legal_Mem);
  _cmov(T, SrcT, Cond);
  _mov(Dest, T);
}

void TargetX8632::lowerMove(Variable *Dest, Operand *Src, bool IsRedefinition) {
  assert(Dest->getType() == Src->getType());
  assert(!Dest->isRematerializable());
  if (Dest->getType() == IceType_i64) {
    Src = legalize(Src);
    Operand *SrcLo = loOperand(Src);
    Operand *SrcHi = hiOperand(Src);
    auto *DestLo = llvm::cast<Variable>(loOperand(Dest));
    auto *DestHi = llvm::cast<Variable>(hiOperand(Dest));
    Variable *T_Lo = nullptr, *T_Hi = nullptr;
    _mov(T_Lo, SrcLo);
    _redefined(_mov(DestLo, T_Lo), IsRedefinition);
    _mov(T_Hi, SrcHi);
    _redefined(_mov(DestHi, T_Hi), IsRedefinition);
  } else {
    Operand *SrcLegal;
    if (Dest->hasReg()) {
      // If Dest already has a physical register, then only basic legalization
      // is needed, as the source operand can be a register, immediate, or
      // memory.
      SrcLegal = legalize(Src, Legal_Reg, Dest->getRegNum());
    } else {
      // If Dest could be a stack operand, then RI must be a physical register
      // or a scalar integer immediate.
      SrcLegal = legalize(Src, Legal_Reg | Legal_Imm);
    }
    if (isVectorType(Dest->getType())) {
      _redefined(_movp(Dest, SrcLegal), IsRedefinition);
    } else {
      _redefined(_mov(Dest, SrcLegal), IsRedefinition);
    }
  }
}

bool TargetX8632::lowerOptimizeFcmpSelect(const InstFcmp *Fcmp,
                                          const InstSelect *Select) {
  Operand *CmpSrc0 = Fcmp->getSrc(0);
  Operand *CmpSrc1 = Fcmp->getSrc(1);
  Operand *SelectSrcT = Select->getTrueOperand();
  Operand *SelectSrcF = Select->getFalseOperand();
  Variable *SelectDest = Select->getDest();

  // TODO(capn): also handle swapped compare/select operand order.
  if (CmpSrc0 != SelectSrcT || CmpSrc1 != SelectSrcF)
    return false;

  // TODO(sehr, stichnot): fcmp/select patterns (e.g., minsd/maxss) go here.
  InstFcmp::FCond Condition = Fcmp->getCondition();
  switch (Condition) {
  default:
    return false;
  case InstFcmp::True:
    break;
  case InstFcmp::False:
    break;
  case InstFcmp::Ogt: {
    Variable *T = makeReg(SelectDest->getType());
    if (isScalarFloatingType(SelectSrcT->getType())) {
      _mov(T, legalize(SelectSrcT, Legal_Reg | Legal_Mem));
      _maxss(T, legalize(SelectSrcF, Legal_Reg | Legal_Mem));
      _mov(SelectDest, T);
    } else {
      _movp(T, legalize(SelectSrcT, Legal_Reg | Legal_Mem));
      _maxps(T, legalize(SelectSrcF, Legal_Reg | Legal_Mem));
      _movp(SelectDest, T);
    }
    return true;
  } break;
  case InstFcmp::Olt: {
    Variable *T = makeReg(SelectSrcT->getType());
    if (isScalarFloatingType(SelectSrcT->getType())) {
      _mov(T, legalize(SelectSrcT, Legal_Reg | Legal_Mem));
      _minss(T, legalize(SelectSrcF, Legal_Reg | Legal_Mem));
      _mov(SelectDest, T);
    } else {
      _movp(T, legalize(SelectSrcT, Legal_Reg | Legal_Mem));
      _minps(T, legalize(SelectSrcF, Legal_Reg | Legal_Mem));
      _movp(SelectDest, T);
    }
    return true;
  } break;
  }
  return false;
}

void TargetX8632::lowerIcmp(const InstIcmp *Icmp) {
  Variable *Dest = Icmp->getDest();
  if (isVectorType(Dest->getType())) {
    lowerIcmpVector(Icmp);
  } else {
    constexpr Inst *Consumer = nullptr;
    lowerIcmpAndConsumer(Icmp, Consumer);
  }
}

void TargetX8632::lowerSelectVector(const InstSelect *Instr) {
  Variable *Dest = Instr->getDest();
  Type DestTy = Dest->getType();
  Operand *SrcT = Instr->getTrueOperand();
  Operand *SrcF = Instr->getFalseOperand();
  Operand *Condition = Instr->getCondition();

  if (!isVectorType(DestTy))
    llvm::report_fatal_error("Expected a vector select");

  Type SrcTy = SrcT->getType();
  Variable *T = makeReg(SrcTy);
  Operand *SrcTRM = legalize(SrcT, Legal_Reg | Legal_Mem);
  Operand *SrcFRM = legalize(SrcF, Legal_Reg | Legal_Mem);

  if (InstructionSet >= SSE4_1) {
    // TODO(wala): If the condition operand is a constant, use blendps or
    // pblendw.
    //
    // Use blendvps or pblendvb to implement select.
    if (SrcTy == IceType_v4i1 || SrcTy == IceType_v4i32 ||
        SrcTy == IceType_v4f32) {
      Operand *ConditionRM = legalize(Condition, Legal_Reg | Legal_Mem);
      Variable *xmm0 = makeReg(IceType_v4i32, RegX8632::Reg_xmm0);
      _movp(xmm0, ConditionRM);
      _psll(xmm0, Ctx->getConstantInt8(31));
      _movp(T, SrcFRM);
      _blendvps(T, SrcTRM, xmm0);
      _movp(Dest, T);
    } else {
      assert(typeNumElements(SrcTy) == 8 || typeNumElements(SrcTy) == 16);
      Type SignExtTy =
          Condition->getType() == IceType_v8i1 ? IceType_v8i16 : IceType_v16i8;
      Variable *xmm0 = makeReg(SignExtTy, RegX8632::Reg_xmm0);
      lowerCast(InstCast::create(Func, InstCast::Sext, xmm0, Condition));
      _movp(T, SrcFRM);
      _pblendvb(T, SrcTRM, xmm0);
      _movp(Dest, T);
    }
    return;
  }
  // Lower select without SSE4.1:
  // a=d?b:c ==>
  //   if elementtype(d) != i1:
  //      d=sext(d);
  //   a=(b&d)|(c&~d);
  Variable *T2 = makeReg(SrcTy);
  // Sign extend the condition operand if applicable.
  if (SrcTy == IceType_v4f32) {
    // The sext operation takes only integer arguments.
    Variable *T3 = Func->makeVariable(IceType_v4i32);
    lowerCast(InstCast::create(Func, InstCast::Sext, T3, Condition));
    _movp(T, T3);
  } else if (typeElementType(SrcTy) != IceType_i1) {
    lowerCast(InstCast::create(Func, InstCast::Sext, T, Condition));
  } else {
    Operand *ConditionRM = legalize(Condition, Legal_Reg | Legal_Mem);
    _movp(T, ConditionRM);
  }
  _movp(T2, T);
  _pand(T, SrcTRM);
  _pandn(T2, SrcFRM);
  _por(T, T2);
  _movp(Dest, T);

  return;
}

void TargetX8632::lowerStore(const InstStore *Instr) {
  Operand *Value = Instr->getData();
  Operand *Addr = Instr->getStoreAddress();
  X86OperandMem *NewAddr = formMemoryOperand(Addr, Value->getType());
  doMockBoundsCheck(NewAddr);
  Type Ty = NewAddr->getType();

  if (Ty == IceType_i64) {
    Value = legalizeUndef(Value);
    Operand *ValueHi = legalize(hiOperand(Value), Legal_Reg | Legal_Imm);
    _store(ValueHi, llvm::cast<X86OperandMem>(hiOperand(NewAddr)));
    Operand *ValueLo = legalize(loOperand(Value), Legal_Reg | Legal_Imm);
    _store(ValueLo, llvm::cast<X86OperandMem>(loOperand(NewAddr)));
  } else if (isVectorType(Ty)) {
    _storep(legalizeToReg(Value), NewAddr);
  } else {
    Value = legalize(Value, Legal_Reg | Legal_Imm);
    _store(Value, NewAddr);
  }
}

void TargetX8632::doAddressOptStore() {
  auto *Instr = llvm::cast<InstStore>(Context.getCur());
  Operand *Addr = Instr->getStoreAddress();
  Operand *Data = Instr->getData();
  if (auto *OptAddr = computeAddressOpt(Instr, Data->getType(), Addr)) {
    Instr->setDeleted();
    auto *NewStore = Context.insert<InstStore>(Data, OptAddr);
    if (Instr->getDest())
      NewStore->setRmwBeacon(Instr->getRmwBeacon());
  }
}

void TargetX8632::doAddressOptStoreSubVector() {
  auto *Intrinsic = llvm::cast<InstIntrinsic>(Context.getCur());
  Operand *Addr = Intrinsic->getArg(1);
  Operand *Data = Intrinsic->getArg(0);
  if (auto *OptAddr = computeAddressOpt(Intrinsic, Data->getType(), Addr)) {
    Intrinsic->setDeleted();
    const Ice::Intrinsics::IntrinsicInfo Info = {
        Ice::Intrinsics::StoreSubVector, Ice::Intrinsics::SideEffects_T,
        Ice::Intrinsics::ReturnsTwice_F, Ice::Intrinsics::MemoryWrite_T};
    auto *NewStore = Context.insert<InstIntrinsic>(3, nullptr, Info);
    NewStore->addArg(Data);
    NewStore->addArg(OptAddr);
    NewStore->addArg(Intrinsic->getArg(2));
  }
}

Operand *TargetX8632::lowerCmpRange(Operand *Comparison, uint64_t Min,
                                    uint64_t Max) {
  // TODO(ascull): 64-bit should not reach here but only because it is not
  // implemented yet. This should be able to handle the 64-bit case.
  assert(Comparison->getType() != IceType_i64);
  // Subtracting 0 is a nop so don't do it
  if (Min != 0) {
    // Avoid clobbering the comparison by copying it
    Variable *T = nullptr;
    _mov(T, Comparison);
    _sub(T, Ctx->getConstantInt32(Min));
    Comparison = T;
  }

  _cmp(Comparison, Ctx->getConstantInt32(Max - Min));

  return Comparison;
}

void TargetX8632::lowerCaseCluster(const CaseCluster &Case, Operand *Comparison,
                                   bool DoneCmp, CfgNode *DefaultTarget) {
  switch (Case.getKind()) {
  case CaseCluster::JumpTable: {
    InstX86Label *SkipJumpTable;

    Operand *RangeIndex =
        lowerCmpRange(Comparison, Case.getLow(), Case.getHigh());
    if (DefaultTarget == nullptr) {
      // Skip over jump table logic if comparison not in range and no default
      SkipJumpTable = InstX86Label::create(Func, this);
      _br(CondX86::Br_a, SkipJumpTable);
    } else {
      _br(CondX86::Br_a, DefaultTarget);
    }

    InstJumpTable *JumpTable = Case.getJumpTable();
    Context.insert(JumpTable);

    // Make sure the index is a register of the same width as the base
    Variable *Index;
    const Type PointerType = IceType_i32;
    if (RangeIndex->getType() != PointerType) {
      Index = makeReg(PointerType);
      assert(RangeIndex->getType() != IceType_i64);
      Operand *RangeIndexRM = legalize(RangeIndex, Legal_Reg | Legal_Mem);
      _movzx(Index, RangeIndexRM);
    } else {
      Index = legalizeToReg(RangeIndex);
    }

    constexpr RelocOffsetT RelocOffset = 0;
    constexpr Variable *NoBase = nullptr;
    constexpr Constant *NoOffset = nullptr;
    auto JTName = GlobalString::createWithString(Ctx, JumpTable->getName());
    Constant *Offset = Ctx->getConstantSym(RelocOffset, JTName);
    uint16_t Shift = typeWidthInBytesLog2(PointerType);
    constexpr auto Segment = X86OperandMem::SegmentRegisters::DefaultSegment;

    Variable *Target = nullptr;
    if (PointerType == IceType_i32) {
      _mov(Target, X86OperandMem::create(Func, PointerType, NoBase, Offset,
                                         Index, Shift, Segment));
    } else {
      auto *Base = makeReg(IceType_i64);
      _lea(Base, X86OperandMem::create(Func, IceType_void, NoBase, Offset));
      _mov(Target, X86OperandMem::create(Func, PointerType, Base, NoOffset,
                                         Index, Shift, Segment));
    }

    lowerIndirectJump(Target);

    if (DefaultTarget == nullptr)
      Context.insert(SkipJumpTable);
    return;
  }
  case CaseCluster::Range: {
    if (Case.isUnitRange()) {
      // Single item
      if (!DoneCmp) {
        Constant *Value = Ctx->getConstantInt32(Case.getLow());
        _cmp(Comparison, Value);
      }
      _br(CondX86::Br_e, Case.getTarget());
    } else if (DoneCmp && Case.isPairRange()) {
      // Range of two items with first item aleady compared against
      _br(CondX86::Br_e, Case.getTarget());
      Constant *Value = Ctx->getConstantInt32(Case.getHigh());
      _cmp(Comparison, Value);
      _br(CondX86::Br_e, Case.getTarget());
    } else {
      // Range
      lowerCmpRange(Comparison, Case.getLow(), Case.getHigh());
      _br(CondX86::Br_be, Case.getTarget());
    }
    if (DefaultTarget != nullptr)
      _br(DefaultTarget);
    return;
  }
  }
}

void TargetX8632::lowerSwitch(const InstSwitch *Instr) {
  // Group cases together and navigate through them with a binary search
  CaseClusterArray CaseClusters = CaseCluster::clusterizeSwitch(Func, Instr);
  Operand *Src0 = Instr->getComparison();
  CfgNode *DefaultTarget = Instr->getLabelDefault();

  assert(CaseClusters.size() != 0); // Should always be at least one

  if (Src0->getType() == IceType_i64) {
    Src0 = legalize(Src0); // get Base/Index into physical registers
    Operand *Src0Lo = loOperand(Src0);
    Operand *Src0Hi = hiOperand(Src0);
    if (CaseClusters.back().getHigh() > UINT32_MAX) {
      // TODO(ascull): handle 64-bit case properly (currently naive version)
      // This might be handled by a higher level lowering of switches.
      SizeT NumCases = Instr->getNumCases();
      if (NumCases >= 2) {
        Src0Lo = legalizeToReg(Src0Lo);
        Src0Hi = legalizeToReg(Src0Hi);
      } else {
        Src0Lo = legalize(Src0Lo, Legal_Reg | Legal_Mem);
        Src0Hi = legalize(Src0Hi, Legal_Reg | Legal_Mem);
      }
      for (SizeT I = 0; I < NumCases; ++I) {
        Constant *ValueLo = Ctx->getConstantInt32(Instr->getValue(I));
        Constant *ValueHi = Ctx->getConstantInt32(Instr->getValue(I) >> 32);
        InstX86Label *Label = InstX86Label::create(Func, this);
        _cmp(Src0Lo, ValueLo);
        _br(CondX86::Br_ne, Label);
        _cmp(Src0Hi, ValueHi);
        _br(CondX86::Br_e, Instr->getLabel(I));
        Context.insert(Label);
      }
      _br(Instr->getLabelDefault());
      return;
    } else {
      // All the values are 32-bit so just check the operand is too and then
      // fall through to the 32-bit implementation. This is a common case.
      Src0Hi = legalize(Src0Hi, Legal_Reg | Legal_Mem);
      Constant *Zero = Ctx->getConstantInt32(0);
      _cmp(Src0Hi, Zero);
      _br(CondX86::Br_ne, DefaultTarget);
      Src0 = Src0Lo;
    }
  }

  // 32-bit lowering

  if (CaseClusters.size() == 1) {
    // Jump straight to default if needed. Currently a common case as jump
    // tables occur on their own.
    constexpr bool DoneCmp = false;
    lowerCaseCluster(CaseClusters.front(), Src0, DoneCmp, DefaultTarget);
    return;
  }

  // Going to be using multiple times so get it in a register early
  Variable *Comparison = legalizeToReg(Src0);

  // A span is over the clusters
  struct SearchSpan {
    SearchSpan(SizeT Begin, SizeT Size, InstX86Label *Label)
        : Begin(Begin), Size(Size), Label(Label) {}

    SizeT Begin;
    SizeT Size;
    InstX86Label *Label;
  };
  // The stack will only grow to the height of the tree so 12 should be plenty
  std::stack<SearchSpan, llvm::SmallVector<SearchSpan, 12>> SearchSpanStack;
  SearchSpanStack.emplace(0, CaseClusters.size(), nullptr);
  bool DoneCmp = false;

  while (!SearchSpanStack.empty()) {
    SearchSpan Span = SearchSpanStack.top();
    SearchSpanStack.pop();

    if (Span.Label != nullptr)
      Context.insert(Span.Label);

    switch (Span.Size) {
    case 0:
      llvm::report_fatal_error("Invalid SearchSpan size");
      break;

    case 1:
      lowerCaseCluster(CaseClusters[Span.Begin], Comparison, DoneCmp,
                       SearchSpanStack.empty() ? nullptr : DefaultTarget);
      DoneCmp = false;
      break;

    case 2: {
      const CaseCluster *CaseA = &CaseClusters[Span.Begin];
      const CaseCluster *CaseB = &CaseClusters[Span.Begin + 1];

      // Placing a range last may allow register clobbering during the range
      // test. That means there is no need to clone the register. If it is a
      // unit range the comparison may have already been done in the binary
      // search (DoneCmp) and so it should be placed first. If this is a range
      // of two items and the comparison with the low value has already been
      // done, comparing with the other element is cheaper than a range test.
      // If the low end of the range is zero then there is no subtraction and
      // nothing to be gained.
      if (!CaseA->isUnitRange() &&
          !(CaseA->getLow() == 0 || (DoneCmp && CaseA->isPairRange()))) {
        std::swap(CaseA, CaseB);
        DoneCmp = false;
      }

      lowerCaseCluster(*CaseA, Comparison, DoneCmp);
      DoneCmp = false;
      lowerCaseCluster(*CaseB, Comparison, DoneCmp,
                       SearchSpanStack.empty() ? nullptr : DefaultTarget);
    } break;

    default:
      // Pick the middle item and branch b or ae
      SizeT PivotIndex = Span.Begin + (Span.Size / 2);
      const CaseCluster &Pivot = CaseClusters[PivotIndex];
      Constant *Value = Ctx->getConstantInt32(Pivot.getLow());
      InstX86Label *Label = InstX86Label::create(Func, this);
      _cmp(Comparison, Value);
      // TODO(ascull): does it alway have to be far?
      _br(CondX86::Br_b, Label, InstX86Br::Far);
      // Lower the left and (pivot+right) sides, falling through to the right
      SearchSpanStack.emplace(Span.Begin, Span.Size / 2, Label);
      SearchSpanStack.emplace(PivotIndex, Span.Size - (Span.Size / 2), nullptr);
      DoneCmp = true;
      break;
    }
  }

  _br(DefaultTarget);
}

/// The following pattern occurs often in lowered C and C++ code:
///
///   %cmp     = fcmp/icmp pred <n x ty> %src0, %src1
///   %cmp.ext = sext <n x i1> %cmp to <n x ty>
///
/// We can eliminate the sext operation by copying the result of pcmpeqd,
/// pcmpgtd, or cmpps (which produce sign extended results) to the result of
/// the sext operation.

void TargetX8632::eliminateNextVectorSextInstruction(
    Variable *SignExtendedResult) {
  if (auto *NextCast =
          llvm::dyn_cast_or_null<InstCast>(Context.getNextInst())) {
    if (NextCast->getCastKind() == InstCast::Sext &&
        NextCast->getSrc(0) == SignExtendedResult) {
      NextCast->setDeleted();
      _movp(NextCast->getDest(), legalizeToReg(SignExtendedResult));
      // Skip over the instruction.
      Context.advanceNext();
    }
  }
}

void TargetX8632::lowerUnreachable(const InstUnreachable * /*Instr*/) {
  _ud2();
  // Add a fake use of esp to make sure esp adjustments after the unreachable
  // do not get dead-code eliminated.
  keepEspLiveAtExit();
}

void TargetX8632::lowerBreakpoint(const InstBreakpoint * /*Instr*/) { _int3(); }

void TargetX8632::lowerRMW(const InstX86FakeRMW *RMW) {
  // If the beacon variable's live range does not end in this instruction,
  // then it must end in the modified Store instruction that follows. This
  // means that the original Store instruction is still there, either because
  // the value being stored is used beyond the Store instruction, or because
  // dead code elimination did not happen. In either case, we cancel RMW
  // lowering (and the caller deletes the RMW instruction).
  if (!RMW->isLastUse(RMW->getBeacon()))
    return;
  Operand *Src = RMW->getData();
  Type Ty = Src->getType();
  X86OperandMem *Addr = formMemoryOperand(RMW->getAddr(), Ty);
  doMockBoundsCheck(Addr);
  if (Ty == IceType_i64) {
    Src = legalizeUndef(Src);
    Operand *SrcLo = legalize(loOperand(Src), Legal_Reg | Legal_Imm);
    Operand *SrcHi = legalize(hiOperand(Src), Legal_Reg | Legal_Imm);
    auto *AddrLo = llvm::cast<X86OperandMem>(loOperand(Addr));
    auto *AddrHi = llvm::cast<X86OperandMem>(hiOperand(Addr));
    switch (RMW->getOp()) {
    default:
      // TODO(stichnot): Implement other arithmetic operators.
      break;
    case InstArithmetic::Add:
      _add_rmw(AddrLo, SrcLo);
      _adc_rmw(AddrHi, SrcHi);
      return;
    case InstArithmetic::Sub:
      _sub_rmw(AddrLo, SrcLo);
      _sbb_rmw(AddrHi, SrcHi);
      return;
    case InstArithmetic::And:
      _and_rmw(AddrLo, SrcLo);
      _and_rmw(AddrHi, SrcHi);
      return;
    case InstArithmetic::Or:
      _or_rmw(AddrLo, SrcLo);
      _or_rmw(AddrHi, SrcHi);
      return;
    case InstArithmetic::Xor:
      _xor_rmw(AddrLo, SrcLo);
      _xor_rmw(AddrHi, SrcHi);
      return;
    }
  } else {
    // x86-32: i8, i16, i32
    // x86-64: i8, i16, i32, i64
    switch (RMW->getOp()) {
    default:
      // TODO(stichnot): Implement other arithmetic operators.
      break;
    case InstArithmetic::Add:
      Src = legalize(Src, Legal_Reg | Legal_Imm);
      _add_rmw(Addr, Src);
      return;
    case InstArithmetic::Sub:
      Src = legalize(Src, Legal_Reg | Legal_Imm);
      _sub_rmw(Addr, Src);
      return;
    case InstArithmetic::And:
      Src = legalize(Src, Legal_Reg | Legal_Imm);
      _and_rmw(Addr, Src);
      return;
    case InstArithmetic::Or:
      Src = legalize(Src, Legal_Reg | Legal_Imm);
      _or_rmw(Addr, Src);
      return;
    case InstArithmetic::Xor:
      Src = legalize(Src, Legal_Reg | Legal_Imm);
      _xor_rmw(Addr, Src);
      return;
    }
  }
  llvm::report_fatal_error("Couldn't lower RMW instruction");
}

void TargetX8632::lowerOther(const Inst *Instr) {
  if (const auto *RMW = llvm::dyn_cast<InstX86FakeRMW>(Instr)) {
    lowerRMW(RMW);
  } else {
    TargetLowering::lowerOther(Instr);
  }
}

/// Turn an i64 Phi instruction into a pair of i32 Phi instructions, to
/// preserve integrity of liveness analysis. Undef values are also turned into
/// zeroes, since loOperand() and hiOperand() don't expect Undef input.
void TargetX8632::prelowerPhis() {
  PhiLowering::prelowerPhis32Bit<TargetX8632>(this, Context.getNode(), Func);
}

void TargetX8632::genTargetHelperCallFor(Inst *Instr) {
  uint32_t StackArgumentsSize = 0;
  if (auto *Arith = llvm::dyn_cast<InstArithmetic>(Instr)) {
    RuntimeHelper HelperID = RuntimeHelper::H_Num;
    Variable *Dest = Arith->getDest();
    Type DestTy = Dest->getType();
    if (DestTy == IceType_i64) {
      switch (Arith->getOp()) {
      default:
        return;
      case InstArithmetic::Udiv:
        HelperID = RuntimeHelper::H_udiv_i64;
        break;
      case InstArithmetic::Sdiv:
        HelperID = RuntimeHelper::H_sdiv_i64;
        break;
      case InstArithmetic::Urem:
        HelperID = RuntimeHelper::H_urem_i64;
        break;
      case InstArithmetic::Srem:
        HelperID = RuntimeHelper::H_srem_i64;
        break;
      }
    } else if (isVectorType(DestTy)) {
      Variable *Dest = Arith->getDest();
      Operand *Src0 = Arith->getSrc(0);
      Operand *Src1 = Arith->getSrc(1);
      switch (Arith->getOp()) {
      default:
        return;
      case InstArithmetic::Mul:
        if (DestTy == IceType_v16i8) {
          scalarizeArithmetic(Arith->getOp(), Dest, Src0, Src1);
          Arith->setDeleted();
        }
        return;
      case InstArithmetic::Shl:
      case InstArithmetic::Lshr:
      case InstArithmetic::Ashr:
        if (llvm::isa<Constant>(Src1)) {
          return;
        }
      case InstArithmetic::Udiv:
      case InstArithmetic::Urem:
      case InstArithmetic::Sdiv:
      case InstArithmetic::Srem:
      case InstArithmetic::Frem:
        scalarizeArithmetic(Arith->getOp(), Dest, Src0, Src1);
        Arith->setDeleted();
        return;
      }
    } else {
      switch (Arith->getOp()) {
      default:
        return;
      case InstArithmetic::Frem:
        if (isFloat32Asserting32Or64(DestTy))
          HelperID = RuntimeHelper::H_frem_f32;
        else
          HelperID = RuntimeHelper::H_frem_f64;
      }
    }
    constexpr SizeT MaxSrcs = 2;
    InstCall *Call = makeHelperCall(HelperID, Dest, MaxSrcs);
    Call->addArg(Arith->getSrc(0));
    Call->addArg(Arith->getSrc(1));
    StackArgumentsSize = getCallStackArgumentsSizeBytes(Call);
    Context.insert(Call);
    Arith->setDeleted();
  } else if (auto *Cast = llvm::dyn_cast<InstCast>(Instr)) {
    InstCast::OpKind CastKind = Cast->getCastKind();
    Operand *Src0 = Cast->getSrc(0);
    const Type SrcType = Src0->getType();
    Variable *Dest = Cast->getDest();
    const Type DestTy = Dest->getType();
    RuntimeHelper HelperID = RuntimeHelper::H_Num;
    Variable *CallDest = Dest;
    switch (CastKind) {
    default:
      return;
    case InstCast::Fptosi:
      if (DestTy == IceType_i64) {
        HelperID = isFloat32Asserting32Or64(SrcType)
                       ? RuntimeHelper::H_fptosi_f32_i64
                       : RuntimeHelper::H_fptosi_f64_i64;
      } else {
        return;
      }
      break;
    case InstCast::Fptoui:
      if (isVectorType(DestTy)) {
        assert(DestTy == IceType_v4i32);
        assert(SrcType == IceType_v4f32);
        HelperID = RuntimeHelper::H_fptoui_4xi32_f32;
      } else if (DestTy == IceType_i64 || DestTy == IceType_i32) {
        if (isInt32Asserting32Or64(DestTy)) {
          HelperID = isFloat32Asserting32Or64(SrcType)
                         ? RuntimeHelper::H_fptoui_f32_i32
                         : RuntimeHelper::H_fptoui_f64_i32;
        } else {
          HelperID = isFloat32Asserting32Or64(SrcType)
                         ? RuntimeHelper::H_fptoui_f32_i64
                         : RuntimeHelper::H_fptoui_f64_i64;
        }
      } else {
        return;
      }
      break;
    case InstCast::Sitofp:
      if (SrcType == IceType_i64) {
        HelperID = isFloat32Asserting32Or64(DestTy)
                       ? RuntimeHelper::H_sitofp_i64_f32
                       : RuntimeHelper::H_sitofp_i64_f64;
      } else {
        return;
      }
      break;
    case InstCast::Uitofp:
      if (isVectorType(SrcType)) {
        assert(DestTy == IceType_v4f32);
        assert(SrcType == IceType_v4i32);
        HelperID = RuntimeHelper::H_uitofp_4xi32_4xf32;
      } else if (SrcType == IceType_i64 || SrcType == IceType_i32) {
        if (isInt32Asserting32Or64(SrcType)) {
          HelperID = isFloat32Asserting32Or64(DestTy)
                         ? RuntimeHelper::H_uitofp_i32_f32
                         : RuntimeHelper::H_uitofp_i32_f64;
        } else {
          HelperID = isFloat32Asserting32Or64(DestTy)
                         ? RuntimeHelper::H_uitofp_i64_f32
                         : RuntimeHelper::H_uitofp_i64_f64;
        }
      } else {
        return;
      }
      break;
    case InstCast::Bitcast: {
      if (DestTy == Src0->getType())
        return;
      switch (DestTy) {
      default:
        return;
      case IceType_i8:
        assert(Src0->getType() == IceType_v8i1);
        HelperID = RuntimeHelper::H_bitcast_8xi1_i8;
        CallDest = Func->makeVariable(IceType_i32);
        break;
      case IceType_i16:
        assert(Src0->getType() == IceType_v16i1);
        HelperID = RuntimeHelper::H_bitcast_16xi1_i16;
        CallDest = Func->makeVariable(IceType_i32);
        break;
      case IceType_v8i1: {
        assert(Src0->getType() == IceType_i8);
        HelperID = RuntimeHelper::H_bitcast_i8_8xi1;
        Variable *Src0AsI32 = Func->makeVariable(stackSlotType());
        // Arguments to functions are required to be at least 32 bits wide.
        Context.insert<InstCast>(InstCast::Zext, Src0AsI32, Src0);
        Src0 = Src0AsI32;
      } break;
      case IceType_v16i1: {
        assert(Src0->getType() == IceType_i16);
        HelperID = RuntimeHelper::H_bitcast_i16_16xi1;
        Variable *Src0AsI32 = Func->makeVariable(stackSlotType());
        // Arguments to functions are required to be at least 32 bits wide.
        Context.insert<InstCast>(InstCast::Zext, Src0AsI32, Src0);
        Src0 = Src0AsI32;
      } break;
      }
    } break;
    }
    constexpr SizeT MaxSrcs = 1;
    InstCall *Call = makeHelperCall(HelperID, CallDest, MaxSrcs);
    Call->addArg(Src0);
    StackArgumentsSize = getCallStackArgumentsSizeBytes(Call);
    Context.insert(Call);
    // The PNaCl ABI disallows i8/i16 return types, so truncate the helper
    // call result to the appropriate type as necessary.
    if (CallDest->getType() != Dest->getType())
      Context.insert<InstCast>(InstCast::Trunc, Dest, CallDest);
    Cast->setDeleted();
  } else if (auto *Intrinsic = llvm::dyn_cast<InstIntrinsic>(Instr)) {
    CfgVector<Type> ArgTypes;
    Type ReturnType = IceType_void;
    switch (Intrinsic->getIntrinsicID()) {
    default:
      return;
    case Intrinsics::Ctpop: {
      Operand *Val = Intrinsic->getArg(0);
      Type ValTy = Val->getType();
      if (ValTy == IceType_i64)
        ArgTypes = {IceType_i64};
      else
        ArgTypes = {IceType_i32};
      ReturnType = IceType_i32;
    } break;
    case Intrinsics::Longjmp:
      ArgTypes = {IceType_i32, IceType_i32};
      ReturnType = IceType_void;
      break;
    case Intrinsics::Memcpy:
      ArgTypes = {IceType_i32, IceType_i32, IceType_i32};
      ReturnType = IceType_void;
      break;
    case Intrinsics::Memmove:
      ArgTypes = {IceType_i32, IceType_i32, IceType_i32};
      ReturnType = IceType_void;
      break;
    case Intrinsics::Memset:
      ArgTypes = {IceType_i32, IceType_i32, IceType_i32};
      ReturnType = IceType_void;
      break;
    case Intrinsics::Setjmp:
      ArgTypes = {IceType_i32};
      ReturnType = IceType_i32;
      break;
    }
    StackArgumentsSize = getCallStackArgumentsSizeBytes(ArgTypes, ReturnType);
  } else if (auto *Call = llvm::dyn_cast<InstCall>(Instr)) {
    StackArgumentsSize = getCallStackArgumentsSizeBytes(Call);
  } else if (auto *Ret = llvm::dyn_cast<InstRet>(Instr)) {
    if (!Ret->hasRetValue())
      return;
    Operand *RetValue = Ret->getRetValue();
    Type ReturnType = RetValue->getType();
    if (!isScalarFloatingType(ReturnType))
      return;
    StackArgumentsSize = typeWidthInBytes(ReturnType);
  } else {
    return;
  }
  StackArgumentsSize = applyStackAlignment(StackArgumentsSize);
  updateMaxOutArgsSizeBytes(StackArgumentsSize);
}

uint32_t
TargetX8632::getCallStackArgumentsSizeBytes(const CfgVector<Type> &ArgTypes,
                                            Type ReturnType) {
  uint32_t OutArgumentsSizeBytes = 0;
  uint32_t XmmArgCount = 0;
  uint32_t GprArgCount = 0;
  for (SizeT i = 0, NumArgTypes = ArgTypes.size(); i < NumArgTypes; ++i) {
    Type Ty = ArgTypes[i];
    // The PNaCl ABI requires the width of arguments to be at least 32 bits.
    assert(typeWidthInBytes(Ty) >= 4);
    if (isVectorType(Ty) &&
        RegX8632::getRegisterForXmmArgNum(RegX8632::getArgIndex(i, XmmArgCount))
            .hasValue()) {
      ++XmmArgCount;
    } else if (isScalarIntegerType(Ty) &&
               RegX8632::getRegisterForGprArgNum(
                   Ty, RegX8632::getArgIndex(i, GprArgCount))
                   .hasValue()) {
      // The 64 bit ABI allows some integers to be passed in GPRs.
      ++GprArgCount;
    } else {
      if (isVectorType(Ty)) {
        OutArgumentsSizeBytes = applyStackAlignment(OutArgumentsSizeBytes);
      }
      OutArgumentsSizeBytes += typeWidthInBytesOnStack(Ty);
    }
  }
  // The 32 bit ABI requires floating point values to be returned on the x87
  // FP stack. Ensure there is enough space for the fstp/movs for floating
  // returns.
  if (isScalarFloatingType(ReturnType)) {
    OutArgumentsSizeBytes =
        std::max(OutArgumentsSizeBytes,
                 static_cast<uint32_t>(typeWidthInBytesOnStack(ReturnType)));
  }
  return OutArgumentsSizeBytes;
}

uint32_t TargetX8632::getCallStackArgumentsSizeBytes(const InstCall *Instr) {
  // Build a vector of the arguments' types.
  const SizeT NumArgs = Instr->getNumArgs();
  CfgVector<Type> ArgTypes;
  ArgTypes.reserve(NumArgs);
  for (SizeT i = 0; i < NumArgs; ++i) {
    Operand *Arg = Instr->getArg(i);
    ArgTypes.emplace_back(Arg->getType());
  }
  // Compute the return type (if any);
  Type ReturnType = IceType_void;
  Variable *Dest = Instr->getDest();
  if (Dest != nullptr)
    ReturnType = Dest->getType();
  return getCallStackArgumentsSizeBytes(ArgTypes, ReturnType);
}

Variable *TargetX8632::makeZeroedRegister(Type Ty, RegNumT RegNum) {
  Variable *Reg = makeReg(Ty, RegNum);
  switch (Ty) {
  case IceType_i1:
  case IceType_i8:
  case IceType_i16:
  case IceType_i32:
  case IceType_i64:
    // Conservatively do "mov reg, 0" to avoid modifying FLAGS.
    _mov(Reg, Ctx->getConstantZero(Ty));
    break;
  case IceType_f32:
  case IceType_f64:
    Context.insert<InstFakeDef>(Reg);
    _xorps(Reg, Reg);
    break;
  default:
    // All vector types use the same pxor instruction.
    assert(isVectorType(Ty));
    Context.insert<InstFakeDef>(Reg);
    _pxor(Reg, Reg);
    break;
  }
  return Reg;
}

// There is no support for loading or emitting vector constants, so the vector
// values returned from makeVectorOfZeros, makeVectorOfOnes, etc. are
// initialized with register operations.
//
// TODO(wala): Add limited support for vector constants so that complex
// initialization in registers is unnecessary.

Variable *TargetX8632::makeVectorOfZeros(Type Ty, RegNumT RegNum) {
  return makeZeroedRegister(Ty, RegNum);
}

Variable *TargetX8632::makeVectorOfMinusOnes(Type Ty, RegNumT RegNum) {
  Variable *MinusOnes = makeReg(Ty, RegNum);
  // Insert a FakeDef so the live range of MinusOnes is not overestimated.
  Context.insert<InstFakeDef>(MinusOnes);
  if (Ty == IceType_f64)
    // Making a vector of minus ones of type f64 is currently only used for
    // the fabs intrinsic.  To use the f64 type to create this mask with
    // pcmpeqq requires SSE 4.1.  Since we're just creating a mask, pcmpeqd
    // does the same job and only requires SSE2.
    _pcmpeq(MinusOnes, MinusOnes, IceType_f32);
  else
    _pcmpeq(MinusOnes, MinusOnes);
  return MinusOnes;
}

Variable *TargetX8632::makeVectorOfOnes(Type Ty, RegNumT RegNum) {
  Variable *Dest = makeVectorOfZeros(Ty, RegNum);
  Variable *MinusOne = makeVectorOfMinusOnes(Ty);
  _psub(Dest, MinusOne);
  return Dest;
}

Variable *TargetX8632::makeVectorOfHighOrderBits(Type Ty, RegNumT RegNum) {
  assert(Ty == IceType_v4i32 || Ty == IceType_v4f32 || Ty == IceType_v8i16 ||
         Ty == IceType_v16i8);
  if (Ty == IceType_v4f32 || Ty == IceType_v4i32 || Ty == IceType_v8i16) {
    Variable *Reg = makeVectorOfOnes(Ty, RegNum);
    SizeT Shift = typeWidthInBytes(typeElementType(Ty)) * X86_CHAR_BIT - 1;
    _psll(Reg, Ctx->getConstantInt8(Shift));
    return Reg;
  } else {
    // SSE has no left shift operation for vectors of 8 bit integers.
    constexpr uint32_t HIGH_ORDER_BITS_MASK = 0x80808080;
    Constant *ConstantMask = Ctx->getConstantInt32(HIGH_ORDER_BITS_MASK);
    Variable *Reg = makeReg(Ty, RegNum);
    _movd(Reg, legalize(ConstantMask, Legal_Reg | Legal_Mem));
    _pshufd(Reg, Reg, Ctx->getConstantZero(IceType_i8));
    return Reg;
  }
}

/// Construct a mask in a register that can be and'ed with a floating-point
/// value to mask off its sign bit. The value will be <4 x 0x7fffffff> for f32
/// and v4f32, and <2 x 0x7fffffffffffffff> for f64. Construct it as vector of
/// ones logically right shifted one bit.
// TODO(stichnot): Fix the wala
// TODO: above, to represent vector constants in memory.

Variable *TargetX8632::makeVectorOfFabsMask(Type Ty, RegNumT RegNum) {
  Variable *Reg = makeVectorOfMinusOnes(Ty, RegNum);
  _psrl(Reg, Ctx->getConstantInt8(1));
  return Reg;
}

X86OperandMem *TargetX8632::getMemoryOperandForStackSlot(Type Ty,
                                                         Variable *Slot,
                                                         uint32_t Offset) {
  // Ensure that Loc is a stack slot.
  assert(Slot->mustNotHaveReg());
  assert(Slot->getRegNum().hasNoValue());
  // Compute the location of Loc in memory.
  // TODO(wala,stichnot): lea should not
  // be required. The address of the stack slot is known at compile time
  // (although not until after addProlog()).
  const Type PointerType = IceType_i32;
  Variable *Loc = makeReg(PointerType);
  _lea(Loc, Slot);
  Constant *ConstantOffset = Ctx->getConstantInt32(Offset);
  return X86OperandMem::create(Func, Ty, Loc, ConstantOffset);
}

/// Lowering helper to copy a scalar integer source operand into some 8-bit
/// GPR. Src is assumed to already be legalized.  If the source operand is
/// known to be a memory or immediate operand, a simple mov will suffice.  But
/// if the source operand can be a physical register, then it must first be
/// copied into a physical register that is truncable to 8-bit, then truncated
/// into a physical register that can receive a truncation, and finally copied
/// into the result 8-bit register (which in general can be any 8-bit
/// register).  For example, moving %ebp into %ah may be accomplished as:
///   movl %ebp, %edx
///   mov_trunc %edx, %dl  // this redundant assignment is ultimately elided
///   movb %dl, %ah
/// On the other hand, moving a memory or immediate operand into ah:
///   movb 4(%ebp), %ah
///   movb $my_imm, %ah
///
/// Note #1.  On a 64-bit target, the "movb 4(%ebp), %ah" is likely not
/// encodable, so RegNum=Reg_ah should NOT be given as an argument.  Instead,
/// use RegNum=RegNumT() and then let the caller do a separate copy into
/// Reg_ah.
///
/// Note #2.  ConstantRelocatable operands are also put through this process
/// (not truncated directly) because our ELF emitter does R_386_32 relocations
/// but not R_386_8 relocations.
///
/// Note #3.  If Src is a Variable, the result will be an infinite-weight i8
/// Variable with the RCX86_IsTrunc8Rcvr register class.  As such, this helper
/// is a convenient way to prevent ah/bh/ch/dh from being an (invalid)
/// argument to the pinsrb instruction.

Variable *TargetX8632::copyToReg8(Operand *Src, RegNumT RegNum) {
  Type Ty = Src->getType();
  assert(isScalarIntegerType(Ty));
  assert(Ty != IceType_i1);
  Variable *Reg = makeReg(IceType_i8, RegNum);
  Reg->setRegClass(RCX86_IsTrunc8Rcvr);
  if (llvm::isa<Variable>(Src) || llvm::isa<ConstantRelocatable>(Src)) {
    Variable *SrcTruncable = makeReg(Ty);
    switch (Ty) {
    case IceType_i64:
      SrcTruncable->setRegClass(RCX86_Is64To8);
      break;
    case IceType_i32:
      SrcTruncable->setRegClass(RCX86_Is32To8);
      break;
    case IceType_i16:
      SrcTruncable->setRegClass(RCX86_Is16To8);
      break;
    default:
      // i8 - just use default register class
      break;
    }
    Variable *SrcRcvr = makeReg(IceType_i8);
    SrcRcvr->setRegClass(RCX86_IsTrunc8Rcvr);
    _mov(SrcTruncable, Src);
    _mov(SrcRcvr, SrcTruncable);
    Src = SrcRcvr;
  }
  _mov(Reg, Src);
  return Reg;
}

/// Helper for legalize() to emit the right code to lower an operand to a
/// register of the appropriate type.

Variable *TargetX8632::copyToReg(Operand *Src, RegNumT RegNum) {
  Type Ty = Src->getType();
  Variable *Reg = makeReg(Ty, RegNum);
  if (isVectorType(Ty)) {
    _movp(Reg, Src);
  } else {
    _mov(Reg, Src);
  }
  return Reg;
}

Operand *TargetX8632::legalize(Operand *From, LegalMask Allowed,
                               RegNumT RegNum) {
  const Type Ty = From->getType();
  // Assert that a physical register is allowed. To date, all calls to
  // legalize() allow a physical register. If a physical register needs to be
  // explicitly disallowed, then new code will need to be written to force a
  // spill.
  assert(Allowed & Legal_Reg);
  // If we're asking for a specific physical register, make sure we're not
  // allowing any other operand kinds. (This could be future work, e.g. allow
  // the shl shift amount to be either an immediate or in ecx.)
  assert(RegNum.hasNoValue() || Allowed == Legal_Reg);

  // Substitute with an available infinite-weight variable if possible.  Only
  // do this when we are not asking for a specific register, and when the
  // substitution is not locked to a specific register, and when the types
  // match, in order to capture the vast majority of opportunities and avoid
  // corner cases in the lowering.
  if (RegNum.hasNoValue()) {
    if (Variable *Subst = getContext().availabilityGet(From)) {
      // At this point we know there is a potential substitution available.
      if (Subst->mustHaveReg() && !Subst->hasReg()) {
        // At this point we know the substitution will have a register.
        if (From->getType() == Subst->getType()) {
          // At this point we know the substitution's register is compatible.
          return Subst;
        }
      }
    }
  }

  if (auto *Mem = llvm::dyn_cast<X86OperandMem>(From)) {
    // Before doing anything with a Mem operand, we need to ensure that the
    // Base and Index components are in physical registers.
    Variable *Base = Mem->getBase();
    Variable *Index = Mem->getIndex();
    Constant *Offset = Mem->getOffset();
    Variable *RegBase = nullptr;
    Variable *RegIndex = nullptr;
    uint16_t Shift = Mem->getShift();
    if (Base) {
      RegBase = llvm::cast<Variable>(
          legalize(Base, Legal_Reg | Legal_Rematerializable));
    }
    if (Index) {
      // TODO(jpp): perhaps we should only allow Legal_Reg if
      // Base->isRematerializable.
      RegIndex = llvm::cast<Variable>(
          legalize(Index, Legal_Reg | Legal_Rematerializable));
    }

    if (Base != RegBase || Index != RegIndex) {
      Mem = X86OperandMem::create(Func, Ty, RegBase, Offset, RegIndex, Shift,
                                  Mem->getSegmentRegister());
    }

    From = Mem;

    if (!(Allowed & Legal_Mem)) {
      From = copyToReg(From, RegNum);
    }
    return From;
  }

  if (auto *Const = llvm::dyn_cast<Constant>(From)) {
    if (llvm::isa<ConstantUndef>(Const)) {
      From = legalizeUndef(Const, RegNum);
      if (isVectorType(Ty))
        return From;
      Const = llvm::cast<Constant>(From);
    }
    // There should be no constants of vector type (other than undef).
    assert(!isVectorType(Ty));

    if (!llvm::dyn_cast<ConstantRelocatable>(Const)) {
      if (isScalarFloatingType(Ty)) {
        // Convert a scalar floating point constant into an explicit memory
        // operand.
        if (auto *ConstFloat = llvm::dyn_cast<ConstantFloat>(Const)) {
          if (Utils::isPositiveZero(ConstFloat->getValue()))
            return makeZeroedRegister(Ty, RegNum);
        } else if (auto *ConstDouble = llvm::dyn_cast<ConstantDouble>(Const)) {
          if (Utils::isPositiveZero(ConstDouble->getValue()))
            return makeZeroedRegister(Ty, RegNum);
        }

        auto *CFrom = llvm::cast<Constant>(From);
        assert(CFrom->getShouldBePooled());
        Constant *Offset = Ctx->getConstantSym(0, CFrom->getLabelName());
        auto *Mem = X86OperandMem::create(Func, Ty, nullptr, Offset);
        From = Mem;
      }
    }

    bool NeedsReg = false;
    if (!(Allowed & Legal_Imm) && !isScalarFloatingType(Ty))
      // Immediate specifically not allowed.
      NeedsReg = true;
    if (!(Allowed & Legal_Mem) && isScalarFloatingType(Ty))
      // On x86, FP constants are lowered to mem operands.
      NeedsReg = true;
    if (NeedsReg) {
      From = copyToReg(From, RegNum);
    }
    return From;
  }

  if (auto *Var = llvm::dyn_cast<Variable>(From)) {
    // Check if the variable is guaranteed a physical register. This can
    // happen either when the variable is pre-colored or when it is assigned
    // infinite weight.
    bool MustHaveRegister = (Var->hasReg() || Var->mustHaveReg());
    bool MustRematerialize =
        (Var->isRematerializable() && !(Allowed & Legal_Rematerializable));
    // We need a new physical register for the operand if:
    // - Mem is not allowed and Var isn't guaranteed a physical register, or
    // - RegNum is required and Var->getRegNum() doesn't match, or
    // - Var is a rematerializable variable and rematerializable pass-through
    // is
    //   not allowed (in which case we need a lea instruction).
    if (MustRematerialize) {
      Variable *NewVar = makeReg(Ty, RegNum);
      // Since Var is rematerializable, the offset will be added when the lea
      // is emitted.
      constexpr Constant *NoOffset = nullptr;
      auto *Mem = X86OperandMem::create(Func, Ty, Var, NoOffset);
      _lea(NewVar, Mem);
      From = NewVar;
    } else if ((!(Allowed & Legal_Mem) && !MustHaveRegister) ||
               (RegNum.hasValue() && RegNum != Var->getRegNum())) {
      From = copyToReg(From, RegNum);
    }
    return From;
  }

  llvm::report_fatal_error("Unhandled operand kind in legalize()");
  return From;
}

/// Provide a trivial wrapper to legalize() for this common usage.

Variable *TargetX8632::legalizeToReg(Operand *From, RegNumT RegNum) {
  return llvm::cast<Variable>(legalize(From, Legal_Reg, RegNum));
}

/// Legalize undef values to concrete values.

Operand *TargetX8632::legalizeUndef(Operand *From, RegNumT RegNum) {
  Type Ty = From->getType();
  if (llvm::isa<ConstantUndef>(From)) {
    // Lower undefs to zero.  Another option is to lower undefs to an
    // uninitialized register; however, using an uninitialized register
    // results in less predictable code.
    //
    // If in the future the implementation is changed to lower undef values to
    // uninitialized registers, a FakeDef will be needed:
    //     Context.insert<InstFakeDef>(Reg);
    // This is in order to ensure that the live range of Reg is not
    // overestimated.  If the constant being lowered is a 64 bit value, then
    // the result should be split and the lo and hi components will need to go
    // in uninitialized registers.
    if (isVectorType(Ty))
      return makeVectorOfZeros(Ty, RegNum);
    return Ctx->getConstantZero(Ty);
  }
  return From;
}

/// For the cmp instruction, if Src1 is an immediate, or known to be a
/// physical register, we can allow Src0 to be a memory operand. Otherwise,
/// Src0 must be copied into a physical register. (Actually, either Src0 or
/// Src1 can be chosen for the physical register, but unfortunately we have to
/// commit to one or the other before register allocation.)

Operand *TargetX8632::legalizeSrc0ForCmp(Operand *Src0, Operand *Src1) {
  bool IsSrc1ImmOrReg = false;
  if (llvm::isa<Constant>(Src1)) {
    IsSrc1ImmOrReg = true;
  } else if (auto *Var = llvm::dyn_cast<Variable>(Src1)) {
    if (Var->hasReg())
      IsSrc1ImmOrReg = true;
  }
  return legalize(Src0, IsSrc1ImmOrReg ? (Legal_Reg | Legal_Mem) : Legal_Reg);
}

X86OperandMem *TargetX8632::formMemoryOperand(Operand *Opnd, Type Ty,
                                              bool DoLegalize) {
  auto *Mem = llvm::dyn_cast<X86OperandMem>(Opnd);
  // It may be the case that address mode optimization already creates an
  // X86OperandMem, so in that case it wouldn't need another level of
  // transformation.
  if (!Mem) {
    auto *Base = llvm::dyn_cast<Variable>(Opnd);
    auto *Offset = llvm::dyn_cast<Constant>(Opnd);
    assert(Base || Offset);
    if (Offset) {
      if (!llvm::isa<ConstantRelocatable>(Offset)) {
        if (llvm::isa<ConstantInteger64>(Offset)) {
          // Memory operands cannot have 64-bit immediates, so they must be
          // legalized into a register only.
          Base = llvm::cast<Variable>(legalize(Offset, Legal_Reg));
          Offset = nullptr;
        } else {
          Offset = llvm::cast<Constant>(legalize(Offset));

          assert(llvm::isa<ConstantInteger32>(Offset) ||
                 llvm::isa<ConstantRelocatable>(Offset));
        }
      }
    }
    Mem = X86OperandMem::create(Func, Ty, Base, Offset);
  }
  return llvm::cast<X86OperandMem>(DoLegalize ? legalize(Mem) : Mem);
}

Variable *TargetX8632::makeReg(Type Type, RegNumT RegNum) {
  // There aren't any 64-bit integer registers for x86-32.
  assert(Type != IceType_i64);
  Variable *Reg = Func->makeVariable(Type);
  if (RegNum.hasValue())
    Reg->setRegNum(RegNum);
  else
    Reg->setMustHaveReg();
  return Reg;
}

const Type TypeForSize[] = {IceType_i8, IceType_i16, IceType_i32, IceType_f64,
                            IceType_v16i8};

Type TargetX8632::largestTypeInSize(uint32_t Size, uint32_t MaxSize) {
  assert(Size != 0);
  uint32_t TyIndex = llvm::findLastSet(Size, llvm::ZB_Undefined);
  uint32_t MaxIndex = MaxSize == NoSizeLimit
                          ? llvm::array_lengthof(TypeForSize) - 1
                          : llvm::findLastSet(MaxSize, llvm::ZB_Undefined);
  return TypeForSize[std::min(TyIndex, MaxIndex)];
}

Type TargetX8632::firstTypeThatFitsSize(uint32_t Size, uint32_t MaxSize) {
  assert(Size != 0);
  uint32_t TyIndex = llvm::findLastSet(Size, llvm::ZB_Undefined);
  if (!llvm::isPowerOf2_32(Size))
    ++TyIndex;
  uint32_t MaxIndex = MaxSize == NoSizeLimit
                          ? llvm::array_lengthof(TypeForSize) - 1
                          : llvm::findLastSet(MaxSize, llvm::ZB_Undefined);
  return TypeForSize[std::min(TyIndex, MaxIndex)];
}

void TargetX8632::postLower() {
  if (Func->getOptLevel() == Opt_m1)
    return;
  markRedefinitions();
  Context.availabilityUpdate();
}

void TargetX8632::emit(const ConstantInteger32 *C) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Ctx->getStrEmit();
  Str << "$" << C->getValue();
}

void TargetX8632::emit(const ConstantInteger64 *C) const {
  llvm::report_fatal_error("Not expecting to emit 64-bit integers");
}

void TargetX8632::emit(const ConstantFloat *C) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Ctx->getStrEmit();
  Str << C->getLabelName();
}

void TargetX8632::emit(const ConstantDouble *C) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Ctx->getStrEmit();
  Str << C->getLabelName();
}

void TargetX8632::emit(const ConstantUndef *) const {
  llvm::report_fatal_error("undef value encountered by emitter.");
}

void TargetX8632::emit(const ConstantRelocatable *C) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Ctx->getStrEmit();
  Str << "$";
  emitWithoutPrefix(C);
}

void TargetX8632::emitJumpTable(const Cfg *,
                                const InstJumpTable *JumpTable) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Ctx->getStrEmit();
  Str << "\t.section\t.rodata." << JumpTable->getSectionName()
      << ",\"a\",@progbits\n"
         "\t.align\t"
      << typeWidthInBytes(IceType_i32) << "\n"
      << JumpTable->getName() << ":";

  for (SizeT I = 0; I < JumpTable->getNumTargets(); ++I)
    Str << "\n\t.long\t" << JumpTable->getTarget(I)->getAsmName();
  Str << "\n";
}

const TargetX8632::TableFcmpType TargetX8632::TableFcmp[] = {
#define X(val, dflt, swapS, C1, C2, swapV, pred)                               \
  {dflt, swapS, CondX86::C1, CondX86::C2, swapV, CondX86::pred},
    FCMPX8632_TABLE
#undef X
};

const size_t TargetX8632::TableFcmpSize = llvm::array_lengthof(TableFcmp);

const TargetX8632::TableIcmp32Type TargetX8632::TableIcmp32[] = {
#define X(val, C_32, C1_64, C2_64, C3_64) {CondX86::C_32},
    ICMPX8632_TABLE
#undef X
};

const size_t TargetX8632::TableIcmp32Size = llvm::array_lengthof(TableIcmp32);

const TargetX8632::TableIcmp64Type TargetX8632::TableIcmp64[] = {
#define X(val, C_32, C1_64, C2_64, C3_64)                                      \
  {CondX86::C1_64, CondX86::C2_64, CondX86::C3_64},
    ICMPX8632_TABLE
#undef X
};

const size_t TargetX8632::TableIcmp64Size = llvm::array_lengthof(TableIcmp64);

std::array<SmallBitVector, RCX86_NUM> TargetX8632::TypeToRegisterSet = {{}};

std::array<SmallBitVector, RCX86_NUM> TargetX8632::TypeToRegisterSetUnfiltered =
    {{}};

std::array<SmallBitVector, RegisterSet::Reg_NUM> TargetX8632::RegisterAliases =
    {{}};

template <typename T>
void TargetDataX8632::emitConstantPool(GlobalContext *Ctx) {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Ctx->getStrEmit();
  Type Ty = T::Ty;
  SizeT Align = typeAlignInBytes(Ty);
  ConstantList Pool = Ctx->getConstantPool(Ty);

  Str << "\t.section\t.rodata.cst" << Align << ",\"aM\",@progbits," << Align
      << "\n";
  Str << "\t.align\t" << Align << "\n";

  for (Constant *C : Pool) {
    if (!C->getShouldBePooled())
      continue;
    auto *Const = llvm::cast<typename T::IceType>(C);
    typename T::IceType::PrimType Value = Const->getValue();
    // Use memcpy() to copy bits from Value into RawValue in a way that avoids
    // breaking strict-aliasing rules.
    typename T::PrimitiveIntType RawValue;
    memcpy(&RawValue, &Value, sizeof(Value));
    char buf[30];
    int CharsPrinted =
        snprintf(buf, llvm::array_lengthof(buf), T::PrintfString, RawValue);
    assert(CharsPrinted >= 0);
    assert((size_t)CharsPrinted < llvm::array_lengthof(buf));
    (void)CharsPrinted; // avoid warnings if asserts are disabled
    Str << Const->getLabelName();
    Str << ":\n\t" << T::AsmTag << "\t" << buf << "\t/* " << T::TypeName << " "
        << Value << " */\n";
  }
}

void TargetDataX8632::lowerConstants() {
  if (getFlags().getDisableTranslation())
    return;
  switch (getFlags().getOutFileType()) {
  case FT_Elf: {
    ELFObjectWriter *Writer = Ctx->getObjectWriter();

    Writer->writeConstantPool<ConstantInteger32>(IceType_i8);
    Writer->writeConstantPool<ConstantInteger32>(IceType_i16);
    Writer->writeConstantPool<ConstantInteger32>(IceType_i32);

    Writer->writeConstantPool<ConstantFloat>(IceType_f32);
    Writer->writeConstantPool<ConstantDouble>(IceType_f64);
  } break;
  case FT_Asm:
  case FT_Iasm: {
    OstreamLocker L(Ctx);

    emitConstantPool<PoolTypeConverter<uint8_t>>(Ctx);
    emitConstantPool<PoolTypeConverter<uint16_t>>(Ctx);
    emitConstantPool<PoolTypeConverter<uint32_t>>(Ctx);

    emitConstantPool<PoolTypeConverter<float>>(Ctx);
    emitConstantPool<PoolTypeConverter<double>>(Ctx);
  } break;
  }
}

void TargetDataX8632::lowerJumpTables() {
  const bool IsPIC = false;
  switch (getFlags().getOutFileType()) {
  case FT_Elf: {
    ELFObjectWriter *Writer = Ctx->getObjectWriter();
    const FixupKind RelocationKind = FK_Abs;
    for (const JumpTableData &JT : Ctx->getJumpTables())
      Writer->writeJumpTable(JT, RelocationKind, IsPIC);
  } break;
  case FT_Asm:
    // Already emitted from Cfg
    break;
  case FT_Iasm: {
    if (!BuildDefs::dump())
      return;
    Ostream &Str = Ctx->getStrEmit();
    const char *Prefix = IsPIC ? ".data.rel.ro." : ".rodata.";
    for (const JumpTableData &JT : Ctx->getJumpTables()) {
      Str << "\t.section\t" << Prefix << JT.getSectionName()
          << ",\"a\",@progbits\n"
             "\t.align\t"
          << typeWidthInBytes(IceType_i32) << "\n"
          << JT.getName().toString() << ":";

      // On X8664 ILP32 pointers are 32-bit hence the use of .long
      for (intptr_t TargetOffset : JT.getTargetOffsets())
        Str << "\n\t.long\t" << JT.getFunctionName() << "+" << TargetOffset;
      Str << "\n";
    }
  } break;
  }
}

void TargetDataX8632::lowerGlobals(const VariableDeclarationList &Vars,
                                   const std::string &SectionSuffix) {
  const bool IsPIC = false;
  switch (getFlags().getOutFileType()) {
  case FT_Elf: {
    ELFObjectWriter *Writer = Ctx->getObjectWriter();
    Writer->writeDataSection(Vars, FK_Abs, SectionSuffix, IsPIC);
  } break;
  case FT_Asm:
  case FT_Iasm: {
    OstreamLocker L(Ctx);
    for (const VariableDeclaration *Var : Vars) {
      if (getFlags().matchTranslateOnly(Var->getName(), 0)) {
        emitGlobal(*Var, SectionSuffix);
      }
    }
  } break;
  }
}

//------------------------------------------------------------------------------
//     __      ______  __     __  ______  ______  __  __   __  ______
//    /\ \    /\  __ \/\ \  _ \ \/\  ___\/\  == \/\ \/\ "-.\ \/\  ___\
//    \ \ \___\ \ \/\ \ \ \/ ".\ \ \  __\\ \  __<\ \ \ \ \-.  \ \ \__ \
//     \ \_____\ \_____\ \__/".~\_\ \_____\ \_\ \_\ \_\ \_\\"\_\ \_____\
//      \/_____/\/_____/\/_/   \/_/\/_____/\/_/ /_/\/_/\/_/ \/_/\/_____/
//
//------------------------------------------------------------------------------
void TargetX8632::_add_sp(Operand *Adjustment) {
  Variable *esp = getPhysicalRegister(RegX8632::Reg_esp);
  _add(esp, Adjustment);
}

void TargetX8632::_mov_sp(Operand *NewValue) {
  Variable *esp = getPhysicalRegister(RegX8632::Reg_esp);
  _redefined(_mov(esp, NewValue));
}

void TargetX8632::_sub_sp(Operand *Adjustment) {
  Variable *esp = getPhysicalRegister(RegX8632::Reg_esp);
  _sub(esp, Adjustment);
  // Add a fake use of the stack pointer, to prevent the stack pointer
  // adustment from being dead-code eliminated in a function that doesn't
  // return.
  Context.insert<InstFakeUse>(esp);
}

void TargetX8632::_link_bp() {
  Variable *ebp = getPhysicalRegister(RegX8632::Reg_ebp);
  Variable *esp = getPhysicalRegister(RegX8632::Reg_esp);
  _push(ebp);
  _mov(ebp, esp);
  // Keep ebp live for late-stage liveness analysis (e.g. asm-verbose mode).
  Context.insert<InstFakeUse>(ebp);
}

void TargetX8632::_unlink_bp() {
  Variable *esp = getPhysicalRegister(RegX8632::Reg_esp);
  Variable *ebp = getPhysicalRegister(RegX8632::Reg_ebp);
  // For late-stage liveness analysis (e.g. asm-verbose mode), adding a fake
  // use of esp before the assignment of esp=ebp keeps previous esp
  // adjustments from being dead-code eliminated.
  Context.insert<InstFakeUse>(esp);
  _mov(esp, ebp);
  _pop(ebp);
}

void TargetX8632::_push_reg(RegNumT RegNum) {
  _push(getPhysicalRegister(RegNum, WordType));
}

void TargetX8632::_pop_reg(RegNumT RegNum) {
  _pop(getPhysicalRegister(RegNum, WordType));
}

/// Lower an indirect jump adding sandboxing when needed.
void TargetX8632::lowerIndirectJump(Variable *JumpTarget) { _jmp(JumpTarget); }

Inst *TargetX8632::emitCallToTarget(Operand *CallTarget, Variable *ReturnReg,
                                    size_t NumVariadicFpArgs) {
  (void)NumVariadicFpArgs;
  // Note that NumVariadicFpArgs is only used for System V x86-64 variadic
  // calls, because floating point arguments are passed via vector registers,
  // whereas for x86-32, all args are passed via the stack.

  return Context.insert<Insts::Call>(ReturnReg, CallTarget);
}

Variable *TargetX8632::moveReturnValueToRegister(Operand *Value,
                                                 Type ReturnType) {
  if (isVectorType(ReturnType)) {
    return legalizeToReg(Value, RegX8632::Reg_xmm0);
  } else if (isScalarFloatingType(ReturnType)) {
    _fld(Value);
    return nullptr;
  } else {
    assert(ReturnType == IceType_i32 || ReturnType == IceType_i64);
    if (ReturnType == IceType_i64) {
      Variable *eax = legalizeToReg(loOperand(Value), RegX8632::Reg_eax);
      Variable *edx = legalizeToReg(hiOperand(Value), RegX8632::Reg_edx);
      Context.insert<InstFakeUse>(edx);
      return eax;
    } else {
      Variable *Reg = nullptr;
      _mov(Reg, Value, RegX8632::Reg_eax);
      return Reg;
    }
  }
}

void TargetX8632::emitStackProbe(size_t StackSizeBytes) {
#if defined(_WIN32)
  if (StackSizeBytes >= 4096) {
    // _chkstk on Win32 is actually __alloca_probe, which adjusts ESP by the
    // stack amount specified in EAX, so we save ESP in ECX, and restore them
    // both after the call.

    Variable *EAX = makeReg(IceType_i32, RegX8632::Reg_eax);
    Variable *ESP = makeReg(IceType_i32, RegX8632::Reg_esp);
    Variable *ECX = makeReg(IceType_i32, RegX8632::Reg_ecx);

    _push_reg(ECX->getRegNum());
    _mov(ECX, ESP);

    _mov(EAX, Ctx->getConstantInt32(StackSizeBytes));

    auto *CallTarget =
        Ctx->getConstantInt32(reinterpret_cast<int32_t>(&_chkstk));
    emitCallToTarget(CallTarget, nullptr);

    _mov(ESP, ECX);
    _pop_reg(ECX->getRegNum());
  }
#endif
}

// In some cases, there are x-macros tables for both high-level and low-level
// instructions/operands that use the same enum key value. The tables are kept
// separate to maintain a proper separation between abstraction layers. There
// is a risk that the tables could get out of sync if enum values are
// reordered or if entries are added or deleted. The following dummy
// namespaces use static_asserts to ensure everything is kept in sync.

namespace {
// Validate the enum values in FCMPX8632_TABLE.
namespace dummy1 {
// Define a temporary set of enum values based on low-level table entries.
enum _tmp_enum {
#define X(val, dflt, swapS, C1, C2, swapV, pred) _tmp_##val,
  FCMPX8632_TABLE
#undef X
      _num
};
// Define a set of constants based on high-level table entries.
#define X(tag, str) static const int _table1_##tag = InstFcmp::tag;
ICEINSTFCMP_TABLE
#undef X
// Define a set of constants based on low-level table entries, and ensure the
// table entry keys are consistent.
#define X(val, dflt, swapS, C1, C2, swapV, pred)                               \
  static const int _table2_##val = _tmp_##val;                                 \
  static_assert(                                                               \
      _table1_##val == _table2_##val,                                          \
      "Inconsistency between FCMPX8632_TABLE and ICEINSTFCMP_TABLE");
FCMPX8632_TABLE
#undef X
// Repeat the static asserts with respect to the high-level table entries in
// case the high-level table has extra entries.
#define X(tag, str)                                                            \
  static_assert(                                                               \
      _table1_##tag == _table2_##tag,                                          \
      "Inconsistency between FCMPX8632_TABLE and ICEINSTFCMP_TABLE");
ICEINSTFCMP_TABLE
#undef X
} // end of namespace dummy1

// Validate the enum values in ICMPX8632_TABLE.
namespace dummy2 {
// Define a temporary set of enum values based on low-level table entries.
enum _tmp_enum {
#define X(val, C_32, C1_64, C2_64, C3_64) _tmp_##val,
  ICMPX8632_TABLE
#undef X
      _num
};
// Define a set of constants based on high-level table entries.
#define X(tag, reverse, str) static const int _table1_##tag = InstIcmp::tag;
ICEINSTICMP_TABLE
#undef X
// Define a set of constants based on low-level table entries, and ensure the
// table entry keys are consistent.
#define X(val, C_32, C1_64, C2_64, C3_64)                                      \
  static const int _table2_##val = _tmp_##val;                                 \
  static_assert(                                                               \
      _table1_##val == _table2_##val,                                          \
      "Inconsistency between ICMPX8632_TABLE and ICEINSTICMP_TABLE");
ICMPX8632_TABLE
#undef X
// Repeat the static asserts with respect to the high-level table entries in
// case the high-level table has extra entries.
#define X(tag, reverse, str)                                                   \
  static_assert(                                                               \
      _table1_##tag == _table2_##tag,                                          \
      "Inconsistency between ICMPX8632_TABLE and ICEINSTICMP_TABLE");
ICEINSTICMP_TABLE
#undef X
} // end of namespace dummy2

// Validate the enum values in ICETYPEX86_TABLE.
namespace dummy3 {
// Define a temporary set of enum values based on low-level table entries.
enum _tmp_enum {
#define X(tag, elty, cvt, sdss, pdps, spsd, int_, unpack, pack, width, fld)    \
  _tmp_##tag,
  ICETYPEX86_TABLE
#undef X
      _num
};
// Define a set of constants based on high-level table entries.
#define X(tag, sizeLog2, align, elts, elty, str, rcstr)                        \
  static const int _table1_##tag = IceType_##tag;
ICETYPE_TABLE
#undef X
// Define a set of constants based on low-level table entries, and ensure the
// table entry keys are consistent.
#define X(tag, elty, cvt, sdss, pdps, spsd, int_, unpack, pack, width, fld)    \
  static const int _table2_##tag = _tmp_##tag;                                 \
  static_assert(_table1_##tag == _table2_##tag,                                \
                "Inconsistency between ICETYPEX86_TABLE and ICETYPE_TABLE");
ICETYPEX86_TABLE
#undef X
// Repeat the static asserts with respect to the high-level table entries in
// case the high-level table has extra entries.
#define X(tag, sizeLog2, align, elts, elty, str, rcstr)                        \
  static_assert(_table1_##tag == _table2_##tag,                                \
                "Inconsistency between ICETYPEX86_TABLE and ICETYPE_TABLE");
ICETYPE_TABLE
#undef X

} // end of namespace dummy3
} // end of anonymous namespace

} // end of namespace X8632
} // end of namespace Ice
