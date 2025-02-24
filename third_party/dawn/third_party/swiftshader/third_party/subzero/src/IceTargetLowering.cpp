//===- subzero/src/IceTargetLowering.cpp - Basic lowering implementation --===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the skeleton of the TargetLowering class.
///
/// Specifically this invokes the appropriate lowering method for a given
/// instruction kind and driving global register allocation. It also implements
/// the non-deleted instruction iteration in LoweringContext.
///
//===----------------------------------------------------------------------===//

#include "IceTargetLowering.h"

#include "IceBitVector.h"
#include "IceCfg.h" // setError()
#include "IceCfgNode.h"
#include "IceGlobalContext.h"
#include "IceGlobalInits.h"
#include "IceInstVarIter.h"
#include "IceLiveness.h"
#include "IceOperand.h"
#include "IceRegAlloc.h"

#include <string>
#include <vector>

#define TARGET_LOWERING_CLASS_FOR(t) Target_##t

// We prevent target-specific implementation details from leaking outside their
// implementations by forbidding #include of target-specific header files
// anywhere outside their own files. To create target-specific objects
// (TargetLowering, TargetDataLowering, and TargetHeaderLowering) we use the
// following named constructors. For reference, each target Foo needs to
// implement the following named constructors and initializer:
//
// namespace Foo {
//   unique_ptr<Ice::TargetLowering> createTargetLowering(Ice::Cfg *);
//   unique_ptr<Ice::TargetDataLowering>
//       createTargetDataLowering(Ice::GlobalContext*);
//   unique_ptr<Ice::TargetHeaderLowering>
//       createTargetHeaderLowering(Ice::GlobalContext *);
//   void staticInit(::Ice::GlobalContext *);
// }
#define SUBZERO_TARGET(X)                                                      \
  namespace X {                                                                \
  std::unique_ptr<::Ice::TargetLowering>                                       \
  createTargetLowering(::Ice::Cfg *Func);                                      \
  std::unique_ptr<::Ice::TargetDataLowering>                                   \
  createTargetDataLowering(::Ice::GlobalContext *Ctx);                         \
  std::unique_ptr<::Ice::TargetHeaderLowering>                                 \
  createTargetHeaderLowering(::Ice::GlobalContext *Ctx);                       \
  void staticInit(::Ice::GlobalContext *Ctx);                                  \
  bool shouldBePooled(const ::Ice::Constant *C);                               \
  ::Ice::Type getPointerType();                                                \
  } // end of namespace X
#include "SZTargets.def"
#undef SUBZERO_TARGET

namespace Ice {
void LoweringContext::init(CfgNode *N) {
  Node = N;
  End = getNode()->getInsts().end();
  rewind();
  advanceForward(Next);
}

void LoweringContext::rewind() {
  Begin = getNode()->getInsts().begin();
  Cur = Begin;
  skipDeleted(Cur);
  Next = Cur;
  availabilityReset();
}

void LoweringContext::insert(Inst *Instr) {
  getNode()->getInsts().insert(Next, Instr);
  LastInserted = Instr;
}

void LoweringContext::skipDeleted(InstList::iterator &I) const {
  while (I != End && I->isDeleted())
    ++I;
}

void LoweringContext::advanceForward(InstList::iterator &I) const {
  if (I != End) {
    ++I;
    skipDeleted(I);
  }
}

Inst *LoweringContext::getLastInserted() const {
  assert(LastInserted);
  return LastInserted;
}

void LoweringContext::availabilityReset() {
  LastDest = nullptr;
  LastSrc = nullptr;
}

void LoweringContext::availabilityUpdate() {
  availabilityReset();
  Inst *Instr = LastInserted;
  if (Instr == nullptr)
    return;
  if (!Instr->isVarAssign())
    return;
  // Since isVarAssign() is true, the source operand must be a Variable.
  LastDest = Instr->getDest();
  LastSrc = llvm::cast<Variable>(Instr->getSrc(0));
}

Variable *LoweringContext::availabilityGet(Operand *Src) const {
  assert(Src);
  if (Src == LastDest)
    return LastSrc;
  return nullptr;
}

namespace {

void printRegisterSet(Ostream &Str, const SmallBitVector &Bitset,
                      std::function<std::string(RegNumT)> getRegName,
                      const std::string &LineIndentString) {
  constexpr size_t RegistersPerLine = 16;
  size_t Count = 0;
  for (RegNumT RegNum : RegNumBVIter(Bitset)) {
    if (Count == 0) {
      Str << LineIndentString;
    } else {
      Str << ",";
    }
    if (Count > 0 && Count % RegistersPerLine == 0)
      Str << "\n" << LineIndentString;
    ++Count;
    Str << getRegName(RegNum);
  }
  if (Count)
    Str << "\n";
}

// Splits "<class>:<reg>" into "<class>" plus "<reg>".  If there is no <class>
// component, the result is "" plus "<reg>".
void splitToClassAndName(const std::string &RegName, std::string *SplitRegClass,
                         std::string *SplitRegName) {
  constexpr const char Separator[] = ":";
  constexpr size_t SeparatorWidth = llvm::array_lengthof(Separator) - 1;
  size_t Pos = RegName.find(Separator);
  if (Pos == std::string::npos) {
    *SplitRegClass = "";
    *SplitRegName = RegName;
  } else {
    *SplitRegClass = RegName.substr(0, Pos);
    *SplitRegName = RegName.substr(Pos + SeparatorWidth);
  }
}

LLVM_ATTRIBUTE_NORETURN void badTargetFatalError(TargetArch Target) {
  llvm::report_fatal_error("Unsupported target: " +
                           std::string(targetArchString(Target)));
}

} // end of anonymous namespace

void TargetLowering::filterTypeToRegisterSet(
    GlobalContext *Ctx, int32_t NumRegs, SmallBitVector TypeToRegisterSet[],
    size_t TypeToRegisterSetSize,
    std::function<std::string(RegNumT)> getRegName,
    std::function<const char *(RegClass)> getRegClassName) {
  std::vector<SmallBitVector> UseSet(TypeToRegisterSetSize,
                                     SmallBitVector(NumRegs));
  std::vector<SmallBitVector> ExcludeSet(TypeToRegisterSetSize,
                                         SmallBitVector(NumRegs));

  std::unordered_map<std::string, RegNumT> RegNameToIndex;
  for (int32_t RegIndex = 0; RegIndex < NumRegs; ++RegIndex) {
    const auto RegNum = RegNumT::fromInt(RegIndex);
    RegNameToIndex[getRegName(RegNum)] = RegNum;
  }

  std::vector<std::string> BadRegNames;

  // The processRegList function iterates across the RegNames vector.  Each
  // entry in the vector is a string of the form "<reg>" or "<class>:<reg>".
  // The register class and register number are computed, and the corresponding
  // bit is set in RegSet[][].  If "<class>:" is missing, then the bit is set
  // for all classes.
  auto processRegList = [&](const std::vector<std::string> &RegNames,
                            std::vector<SmallBitVector> &RegSet) {
    for (const std::string &RegClassAndName : RegNames) {
      std::string RClass;
      std::string RName;
      splitToClassAndName(RegClassAndName, &RClass, &RName);
      if (!RegNameToIndex.count(RName)) {
        BadRegNames.push_back(RName);
        continue;
      }
      const int32_t RegIndex = RegNameToIndex.at(RName);
      for (SizeT TypeIndex = 0; TypeIndex < TypeToRegisterSetSize;
           ++TypeIndex) {
        if (RClass.empty() ||
            RClass == getRegClassName(static_cast<RegClass>(TypeIndex))) {
          RegSet[TypeIndex][RegIndex] = TypeToRegisterSet[TypeIndex][RegIndex];
        }
      }
    }
  };

  processRegList(getFlags().getUseRestrictedRegisters(), UseSet);
  processRegList(getFlags().getExcludedRegisters(), ExcludeSet);

  if (!BadRegNames.empty()) {
    std::string Buffer;
    llvm::raw_string_ostream StrBuf(Buffer);
    StrBuf << "Unrecognized use/exclude registers:";
    for (const auto &RegName : BadRegNames)
      StrBuf << " " << RegName;
    llvm::report_fatal_error(StrBuf.str());
  }

  // Apply filters.
  for (size_t TypeIndex = 0; TypeIndex < TypeToRegisterSetSize; ++TypeIndex) {
    SmallBitVector *TypeBitSet = &TypeToRegisterSet[TypeIndex];
    SmallBitVector *UseBitSet = &UseSet[TypeIndex];
    SmallBitVector *ExcludeBitSet = &ExcludeSet[TypeIndex];
    if (UseBitSet->any())
      *TypeBitSet = *UseBitSet;
    (*TypeBitSet).reset(*ExcludeBitSet);
  }

  // Display filtered register sets, if requested.
  if (BuildDefs::dump() && NumRegs &&
      (getFlags().getVerbose() & IceV_AvailableRegs)) {
    Ostream &Str = Ctx->getStrDump();
    const std::string Indent = "  ";
    const std::string IndentTwice = Indent + Indent;
    Str << "Registers available for register allocation:\n";
    for (size_t TypeIndex = 0; TypeIndex < TypeToRegisterSetSize; ++TypeIndex) {
      Str << Indent << getRegClassName(static_cast<RegClass>(TypeIndex))
          << ":\n";
      printRegisterSet(Str, TypeToRegisterSet[TypeIndex], getRegName,
                       IndentTwice);
    }
    Str << "\n";
  }
}

std::unique_ptr<TargetLowering>
TargetLowering::createLowering(TargetArch Target, Cfg *Func) {
  switch (Target) {
  default:
    badTargetFatalError(Target);
#define SUBZERO_TARGET(X)                                                      \
  case TARGET_LOWERING_CLASS_FOR(X):                                           \
    return ::X::createTargetLowering(Func);
#include "SZTargets.def"
#undef SUBZERO_TARGET
  }
}

void TargetLowering::staticInit(GlobalContext *Ctx) {
  const TargetArch Target = getFlags().getTargetArch();
  // Call the specified target's static initializer.
  switch (Target) {
  default:
    badTargetFatalError(Target);
#define SUBZERO_TARGET(X)                                                      \
  case TARGET_LOWERING_CLASS_FOR(X): {                                         \
    static bool InitGuard##X = false;                                          \
    if (InitGuard##X) {                                                        \
      return;                                                                  \
    }                                                                          \
    InitGuard##X = true;                                                       \
    ::X::staticInit(Ctx);                                                      \
  } break;
#include "SZTargets.def"
#undef SUBZERO_TARGET
  }
}

bool TargetLowering::shouldBePooled(const Constant *C) {
  const TargetArch Target = getFlags().getTargetArch();
  switch (Target) {
  default:
    return false;
#define SUBZERO_TARGET(X)                                                      \
  case TARGET_LOWERING_CLASS_FOR(X):                                           \
    return ::X::shouldBePooled(C);
#include "SZTargets.def"
#undef SUBZERO_TARGET
  }
}

::Ice::Type TargetLowering::getPointerType() {
  const TargetArch Target = getFlags().getTargetArch();
  switch (Target) {
  default:
    return ::Ice::IceType_void;
#define SUBZERO_TARGET(X)                                                      \
  case TARGET_LOWERING_CLASS_FOR(X):                                           \
    return ::X::getPointerType();
#include "SZTargets.def"
#undef SUBZERO_TARGET
  }
}

TargetLowering::TargetLowering(Cfg *Func)
    : Func(Func), Ctx(Func->getContext()) {}

void TargetLowering::genTargetHelperCalls() {
  TimerMarker T(TimerStack::TT_genHelpers, Func);
  Utils::BoolFlagSaver _(GeneratingTargetHelpers, true);
  for (CfgNode *Node : Func->getNodes()) {
    Context.init(Node);
    while (!Context.atEnd()) {
      PostIncrLoweringContext _(Context);
      genTargetHelperCallFor(iteratorToInst(Context.getCur()));
    }
  }
}

void TargetLowering::doAddressOpt() {
  doAddressOptOther();
  if (llvm::isa<InstLoad>(*Context.getCur()))
    doAddressOptLoad();
  else if (llvm::isa<InstStore>(*Context.getCur()))
    doAddressOptStore();
  else if (auto *Intrinsic =
               llvm::dyn_cast<InstIntrinsic>(&*Context.getCur())) {
    if (Intrinsic->getIntrinsicID() == Intrinsics::LoadSubVector)
      doAddressOptLoadSubVector();
    else if (Intrinsic->getIntrinsicID() == Intrinsics::StoreSubVector)
      doAddressOptStoreSubVector();
  }
  Context.advanceCur();
  Context.advanceNext();
}

// Lowers a single instruction according to the information in Context, by
// checking the Context.Cur instruction kind and calling the appropriate
// lowering method. The lowering method should insert target instructions at
// the Cur.Next insertion point, and should not delete the Context.Cur
// instruction or advance Context.Cur.
//
// The lowering method may look ahead in the instruction stream as desired, and
// lower additional instructions in conjunction with the current one, for
// example fusing a compare and branch. If it does, it should advance
// Context.Cur to point to the next non-deleted instruction to process, and it
// should delete any additional instructions it consumes.
void TargetLowering::lower() {
  assert(!Context.atEnd());
  Inst *Instr = iteratorToInst(Context.getCur());
  Instr->deleteIfDead();
  if (!Instr->isDeleted() && !llvm::isa<InstFakeDef>(Instr) &&
      !llvm::isa<InstFakeUse>(Instr)) {
    // Mark the current instruction as deleted before lowering, otherwise the
    // Dest variable will likely get marked as non-SSA. See
    // Variable::setDefinition(). However, just pass-through FakeDef and
    // FakeUse instructions that might have been inserted prior to lowering.
    Instr->setDeleted();
    switch (Instr->getKind()) {
    case Inst::Alloca:
      lowerAlloca(llvm::cast<InstAlloca>(Instr));
      break;
    case Inst::Arithmetic:
      lowerArithmetic(llvm::cast<InstArithmetic>(Instr));
      break;
    case Inst::Assign:
      lowerAssign(llvm::cast<InstAssign>(Instr));
      break;
    case Inst::Br:
      lowerBr(llvm::cast<InstBr>(Instr));
      break;
    case Inst::Breakpoint:
      lowerBreakpoint(llvm::cast<InstBreakpoint>(Instr));
      break;
    case Inst::Call:
      lowerCall(llvm::cast<InstCall>(Instr));
      break;
    case Inst::Cast:
      lowerCast(llvm::cast<InstCast>(Instr));
      break;
    case Inst::ExtractElement:
      lowerExtractElement(llvm::cast<InstExtractElement>(Instr));
      break;
    case Inst::Fcmp:
      lowerFcmp(llvm::cast<InstFcmp>(Instr));
      break;
    case Inst::Icmp:
      lowerIcmp(llvm::cast<InstIcmp>(Instr));
      break;
    case Inst::InsertElement:
      lowerInsertElement(llvm::cast<InstInsertElement>(Instr));
      break;
    case Inst::Intrinsic: {
      auto *Intrinsic = llvm::cast<InstIntrinsic>(Instr);
      if (Intrinsic->getIntrinsicInfo().ReturnsTwice)
        setCallsReturnsTwice(true);
      lowerIntrinsic(Intrinsic);
      break;
    }
    case Inst::Load:
      lowerLoad(llvm::cast<InstLoad>(Instr));
      break;
    case Inst::Phi:
      lowerPhi(llvm::cast<InstPhi>(Instr));
      break;
    case Inst::Ret:
      lowerRet(llvm::cast<InstRet>(Instr));
      break;
    case Inst::Select:
      lowerSelect(llvm::cast<InstSelect>(Instr));
      break;
    case Inst::ShuffleVector:
      lowerShuffleVector(llvm::cast<InstShuffleVector>(Instr));
      break;
    case Inst::Store:
      lowerStore(llvm::cast<InstStore>(Instr));
      break;
    case Inst::Switch:
      lowerSwitch(llvm::cast<InstSwitch>(Instr));
      break;
    case Inst::Unreachable:
      lowerUnreachable(llvm::cast<InstUnreachable>(Instr));
      break;
    default:
      lowerOther(Instr);
      break;
    }

    postLower();
  }

  Context.advanceCur();
  Context.advanceNext();
}

void TargetLowering::lowerInst(CfgNode *Node, InstList::iterator Next,
                               InstHighLevel *Instr) {
  // TODO(stichnot): Consider modifying the design/implementation to avoid
  // multiple init() calls when using lowerInst() to lower several instructions
  // in the same node.
  Context.init(Node);
  Context.setNext(Next);
  Context.insert(Instr);
  --Next;
  assert(iteratorToInst(Next) == Instr);
  Context.setCur(Next);
  lower();
}

void TargetLowering::lowerOther(const Inst *Instr) {
  (void)Instr;
  Func->setError("Can't lower unsupported instruction type");
}

// Drives register allocation, allowing all physical registers (except perhaps
// for the frame pointer) to be allocated. This set of registers could
// potentially be parameterized if we want to restrict registers e.g. for
// performance testing.
void TargetLowering::regAlloc(RegAllocKind Kind) {
  TimerMarker T(TimerStack::TT_regAlloc, Func);
  LinearScan LinearScan(Func);
  RegSetMask RegInclude = RegSet_None;
  RegSetMask RegExclude = RegSet_None;
  RegInclude |= RegSet_CallerSave;
  RegInclude |= RegSet_CalleeSave;
  if (hasFramePointer())
    RegExclude |= RegSet_FramePointer;
  SmallBitVector RegMask = getRegisterSet(RegInclude, RegExclude);
  bool Repeat = (Kind == RAK_Global && getFlags().getRepeatRegAlloc());
  CfgSet<Variable *> EmptySet;
  do {
    LinearScan.init(Kind, EmptySet);
    LinearScan.scan(RegMask);
    if (!LinearScan.hasEvictions())
      Repeat = false;
    Kind = RAK_SecondChance;
  } while (Repeat);
  // TODO(stichnot): Run the register allocator one more time to do stack slot
  // coalescing.  The idea would be to initialize the Unhandled list with the
  // set of Variables that have no register and a non-empty live range, and
  // model an infinite number of registers.  Maybe use the register aliasing
  // mechanism to get better packing of narrower slots.
  if (getFlags().getSplitGlobalVars())
    postRegallocSplitting(RegMask);
}

namespace {
CfgVector<Inst *> getInstructionsInRange(CfgNode *Node, InstNumberT Start,
                                         InstNumberT End) {
  CfgVector<Inst *> Result;
  bool Started = false;
  auto Process = [&](InstList &Insts) {
    for (auto &Instr : Insts) {
      if (Instr.isDeleted()) {
        continue;
      }
      if (Instr.getNumber() == Start) {
        Started = true;
      }
      if (Started) {
        Result.emplace_back(&Instr);
      }
      if (Instr.getNumber() == End) {
        break;
      }
    }
  };
  Process(Node->getPhis());
  Process(Node->getInsts());
  // TODO(manasijm): Investigate why checking >= End significantly changes
  // output. Should not happen when renumbering produces monotonically
  // increasing instruction numbers and live ranges begin and end on non-deleted
  // instructions.
  return Result;
}
} // namespace

void TargetLowering::postRegallocSplitting(const SmallBitVector &RegMask) {
  // Splits the live ranges of global(/multi block) variables and runs the
  // register allocator to find registers for as many of the new variables as
  // possible.
  // TODO(manasijm): Merge the small liveranges back into multi-block ones when
  // the variables get the same register. This will reduce the amount of new
  // instructions inserted. This might involve a full dataflow analysis.
  // Also, modify the preference mechanism in the register allocator to match.

  TimerMarker _(TimerStack::TT_splitGlobalVars, Func);
  CfgSet<Variable *> SplitCandidates;

  // Find variables that do not have registers but are allowed to. Also skip
  // variables with single segment live ranges as they are not split further in
  // this function.
  for (Variable *Var : Func->getVariables()) {
    if (!Var->mustNotHaveReg() && !Var->hasReg()) {
      if (Var->getLiveRange().getNumSegments() > 1)
        SplitCandidates.insert(Var);
    }
  }
  if (SplitCandidates.empty())
    return;

  CfgSet<Variable *> ExtraVars;

  struct UseInfo {
    Variable *Replacing = nullptr;
    Inst *FirstUse = nullptr;
    Inst *LastDef = nullptr;
    SizeT UseCount = 0;
  };
  CfgUnorderedMap<Variable *, UseInfo> VarInfo;
  // Split the live ranges of the viable variables by node.
  // Compute metadata (UseInfo) for each of the resulting variables.
  for (auto *Var : SplitCandidates) {
    for (auto &Segment : Var->getLiveRange().getSegments()) {
      UseInfo Info;
      Info.Replacing = Var;
      auto *Node = Var->getLiveRange().getNodeForSegment(Segment.first);

      for (auto *Instr :
           getInstructionsInRange(Node, Segment.first, Segment.second)) {
        for (SizeT i = 0; i < Instr->getSrcSize(); ++i) {
          // It's safe to iterate over the top-level src operands rather than
          // using FOREACH_VAR_IN_INST(), because any variables inside e.g.
          // mem operands should already have registers.
          if (auto *Var = llvm::dyn_cast<Variable>(Instr->getSrc(i))) {
            if (Var == Info.Replacing) {
              if (Info.FirstUse == nullptr && !llvm::isa<InstPhi>(Instr)) {
                Info.FirstUse = Instr;
              }
              Info.UseCount++;
            }
          }
        }
        if (Instr->getDest() == Info.Replacing && !llvm::isa<InstPhi>(Instr)) {
          Info.LastDef = Instr;
        }
      }

      static constexpr SizeT MinUseThreshold = 3;
      // Skip if variable has less than `MinUseThreshold` uses in the segment.
      if (Info.UseCount < MinUseThreshold)
        continue;

      if (!Info.FirstUse && !Info.LastDef) {
        continue;
      }

      LiveRange LR;
      LR.addSegment(Segment);
      Variable *NewVar = Func->makeVariable(Var->getType());

      NewVar->setLiveRange(LR);

      VarInfo[NewVar] = Info;

      ExtraVars.insert(NewVar);
    }
  }
  // Run the register allocator with all these new variables included
  LinearScan RegAlloc(Func);
  RegAlloc.init(RAK_Global, SplitCandidates);
  RegAlloc.scan(RegMask);

  // Modify the Cfg to use the new variables that now have registers.
  for (auto *ExtraVar : ExtraVars) {
    if (!ExtraVar->hasReg()) {
      continue;
    }

    auto &Info = VarInfo[ExtraVar];

    assert(ExtraVar->getLiveRange().getSegments().size() == 1);
    auto Segment = ExtraVar->getLiveRange().getSegments()[0];

    auto *Node =
        Info.Replacing->getLiveRange().getNodeForSegment(Segment.first);

    auto RelevantInsts =
        getInstructionsInRange(Node, Segment.first, Segment.second);

    if (RelevantInsts.empty())
      continue;

    // Replace old variables
    for (auto *Instr : RelevantInsts) {
      if (llvm::isa<InstPhi>(Instr))
        continue;
      // TODO(manasijm): Figure out how to safely enable replacing phi dest
      // variables. The issue is that we can not insert low level mov
      // instructions into the PhiList.
      for (SizeT i = 0; i < Instr->getSrcSize(); ++i) {
        // FOREACH_VAR_IN_INST() not needed. Same logic as above.
        if (auto *Var = llvm::dyn_cast<Variable>(Instr->getSrc(i))) {
          if (Var == Info.Replacing) {
            Instr->replaceSource(i, ExtraVar);
          }
        }
      }
      if (Instr->getDest() == Info.Replacing) {
        Instr->replaceDest(ExtraVar);
      }
    }

    assert(Info.FirstUse != Info.LastDef);
    assert(Info.FirstUse || Info.LastDef);

    // Insert spill code
    if (Info.FirstUse != nullptr) {
      auto *NewInst =
          Func->getTarget()->createLoweredMove(ExtraVar, Info.Replacing);
      Node->getInsts().insert(instToIterator(Info.FirstUse), NewInst);
    }
    if (Info.LastDef != nullptr) {
      auto *NewInst =
          Func->getTarget()->createLoweredMove(Info.Replacing, ExtraVar);
      Node->getInsts().insertAfter(instToIterator(Info.LastDef), NewInst);
    }
  }
}

void TargetLowering::markRedefinitions() {
  // Find (non-SSA) instructions where the Dest variable appears in some source
  // operand, and set the IsDestRedefined flag to keep liveness analysis
  // consistent.
  for (auto Instr = Context.getCur(), E = Context.getNext(); Instr != E;
       ++Instr) {
    if (Instr->isDeleted())
      continue;
    Variable *Dest = Instr->getDest();
    if (Dest == nullptr)
      continue;
    FOREACH_VAR_IN_INST(Var, *Instr) {
      if (Var == Dest) {
        Instr->setDestRedefined();
        break;
      }
    }
  }
}

void TargetLowering::addFakeDefUses(const Inst *Instr) {
  FOREACH_VAR_IN_INST(Var, *Instr) {
    if (auto *Var64 = llvm::dyn_cast<Variable64On32>(Var)) {
      Context.insert<InstFakeUse>(Var64->getLo());
      Context.insert<InstFakeUse>(Var64->getHi());
    } else if (auto *VarVec = llvm::dyn_cast<VariableVecOn32>(Var)) {
      for (Variable *Var : VarVec->getContainers()) {
        Context.insert<InstFakeUse>(Var);
      }
    } else {
      Context.insert<InstFakeUse>(Var);
    }
  }
  Variable *Dest = Instr->getDest();
  if (Dest == nullptr)
    return;
  if (auto *Var64 = llvm::dyn_cast<Variable64On32>(Dest)) {
    Context.insert<InstFakeDef>(Var64->getLo());
    Context.insert<InstFakeDef>(Var64->getHi());
  } else if (auto *VarVec = llvm::dyn_cast<VariableVecOn32>(Dest)) {
    for (Variable *Var : VarVec->getContainers()) {
      Context.insert<InstFakeDef>(Var);
    }
  } else {
    Context.insert<InstFakeDef>(Dest);
  }
}

void TargetLowering::sortVarsByAlignment(VarList &Dest,
                                         const VarList &Source) const {
  Dest = Source;
  // Instead of std::sort, we could do a bucket sort with log2(alignment) as
  // the buckets, if performance is an issue.
  std::sort(Dest.begin(), Dest.end(),
            [this](const Variable *V1, const Variable *V2) {
              const size_t WidthV1 = typeWidthInBytesOnStack(V1->getType());
              const size_t WidthV2 = typeWidthInBytesOnStack(V2->getType());
              if (WidthV1 == WidthV2)
                return V1->getIndex() < V2->getIndex();
              return WidthV1 > WidthV2;
            });
}

void TargetLowering::getVarStackSlotParams(
    VarList &SortedSpilledVariables, SmallBitVector &RegsUsed,
    size_t *GlobalsSize, size_t *SpillAreaSizeBytes,
    uint32_t *SpillAreaAlignmentBytes, uint32_t *LocalsSlotsAlignmentBytes,
    std::function<bool(Variable *)> TargetVarHook) {
  const VariablesMetadata *VMetadata = Func->getVMetadata();
  BitVector IsVarReferenced(Func->getNumVariables());
  for (CfgNode *Node : Func->getNodes()) {
    for (Inst &Instr : Node->getInsts()) {
      if (Instr.isDeleted())
        continue;
      if (const Variable *Var = Instr.getDest())
        IsVarReferenced[Var->getIndex()] = true;
      FOREACH_VAR_IN_INST(Var, Instr) {
        IsVarReferenced[Var->getIndex()] = true;
      }
    }
  }

  // If SimpleCoalescing is false, each variable without a register gets its
  // own unique stack slot, which leads to large stack frames. If
  // SimpleCoalescing is true, then each "global" variable without a register
  // gets its own slot, but "local" variable slots are reused across basic
  // blocks. E.g., if A and B are local to block 1 and C is local to block 2,
  // then C may share a slot with A or B.
  //
  // We cannot coalesce stack slots if this function calls a "returns twice"
  // function. In that case, basic blocks may be revisited, and variables local
  // to those basic blocks are actually live until after the called function
  // returns a second time.
  const bool SimpleCoalescing = !callsReturnsTwice();

  CfgVector<size_t> LocalsSize(Func->getNumNodes());
  const VarList &Variables = Func->getVariables();
  VarList SpilledVariables;
  for (Variable *Var : Variables) {
    if (Var->hasReg()) {
      // Don't consider a rematerializable variable to be an actual register use
      // (specifically of the frame pointer).  Otherwise, the prolog may decide
      // to save the frame pointer twice - once because of the explicit need for
      // a frame pointer, and once because of an active use of a callee-save
      // register.
      if (!Var->isRematerializable())
        RegsUsed[Var->getRegNum()] = true;
      continue;
    }
    // An argument either does not need a stack slot (if passed in a register)
    // or already has one (if passed on the stack).
    if (Var->getIsArg()) {
      if (!Var->hasReg()) {
        assert(!Var->hasStackOffset());
        Var->setHasStackOffset();
      }
      continue;
    }
    // An unreferenced variable doesn't need a stack slot.
    if (!IsVarReferenced[Var->getIndex()])
      continue;
    // Check a target-specific variable (it may end up sharing stack slots) and
    // not need accounting here.
    if (TargetVarHook(Var))
      continue;
    assert(!Var->hasStackOffset());
    Var->setHasStackOffset();
    SpilledVariables.push_back(Var);
  }

  SortedSpilledVariables.reserve(SpilledVariables.size());
  sortVarsByAlignment(SortedSpilledVariables, SpilledVariables);

  for (Variable *Var : SortedSpilledVariables) {
    size_t Increment = typeWidthInBytesOnStack(Var->getType());
    // We have sorted by alignment, so the first variable we encounter that is
    // located in each area determines the max alignment for the area.
    if (!*SpillAreaAlignmentBytes)
      *SpillAreaAlignmentBytes = Increment;
    if (SimpleCoalescing && VMetadata->isTracked(Var)) {
      if (VMetadata->isMultiBlock(Var)) {
        *GlobalsSize += Increment;
      } else {
        SizeT NodeIndex = VMetadata->getLocalUseNode(Var)->getIndex();
        LocalsSize[NodeIndex] += Increment;
        if (LocalsSize[NodeIndex] > *SpillAreaSizeBytes)
          *SpillAreaSizeBytes = LocalsSize[NodeIndex];
        if (!*LocalsSlotsAlignmentBytes)
          *LocalsSlotsAlignmentBytes = Increment;
      }
    } else {
      *SpillAreaSizeBytes += Increment;
    }
  }
  // For testing legalization of large stack offsets on targets with limited
  // offset bits in instruction encodings, add some padding.
  *SpillAreaSizeBytes += getFlags().getTestStackExtra();
}

void TargetLowering::alignStackSpillAreas(uint32_t SpillAreaStartOffset,
                                          uint32_t SpillAreaAlignmentBytes,
                                          size_t GlobalsSize,
                                          uint32_t LocalsSlotsAlignmentBytes,
                                          uint32_t *SpillAreaPaddingBytes,
                                          uint32_t *LocalsSlotsPaddingBytes) {
  if (SpillAreaAlignmentBytes) {
    uint32_t PaddingStart = SpillAreaStartOffset;
    uint32_t SpillAreaStart =
        Utils::applyAlignment(PaddingStart, SpillAreaAlignmentBytes);
    *SpillAreaPaddingBytes = SpillAreaStart - PaddingStart;
  }

  // If there are separate globals and locals areas, make sure the locals area
  // is aligned by padding the end of the globals area.
  if (LocalsSlotsAlignmentBytes) {
    uint32_t GlobalsAndSubsequentPaddingSize = GlobalsSize;
    GlobalsAndSubsequentPaddingSize =
        Utils::applyAlignment(GlobalsSize, LocalsSlotsAlignmentBytes);
    *LocalsSlotsPaddingBytes = GlobalsAndSubsequentPaddingSize - GlobalsSize;
  }
}

void TargetLowering::assignVarStackSlots(VarList &SortedSpilledVariables,
                                         size_t SpillAreaPaddingBytes,
                                         size_t SpillAreaSizeBytes,
                                         size_t GlobalsAndSubsequentPaddingSize,
                                         bool UsesFramePointer) {
  const VariablesMetadata *VMetadata = Func->getVMetadata();
  // For testing legalization of large stack offsets on targets with limited
  // offset bits in instruction encodings, add some padding. This assumes that
  // SpillAreaSizeBytes has accounted for the extra test padding. When
  // UseFramePointer is true, the offset depends on the padding, not just the
  // SpillAreaSizeBytes. On the other hand, when UseFramePointer is false, the
  // offsets depend on the gap between SpillAreaSizeBytes and
  // SpillAreaPaddingBytes, so we don't increment that.
  size_t TestPadding = getFlags().getTestStackExtra();
  if (UsesFramePointer)
    SpillAreaPaddingBytes += TestPadding;
  size_t GlobalsSpaceUsed = SpillAreaPaddingBytes;
  size_t NextStackOffset = SpillAreaPaddingBytes;
  CfgVector<size_t> LocalsSize(Func->getNumNodes());
  const bool SimpleCoalescing = !callsReturnsTwice();

  for (Variable *Var : SortedSpilledVariables) {
    size_t Increment = typeWidthInBytesOnStack(Var->getType());
    if (SimpleCoalescing && VMetadata->isTracked(Var)) {
      if (VMetadata->isMultiBlock(Var)) {
        GlobalsSpaceUsed += Increment;
        NextStackOffset = GlobalsSpaceUsed;
      } else {
        SizeT NodeIndex = VMetadata->getLocalUseNode(Var)->getIndex();
        LocalsSize[NodeIndex] += Increment;
        NextStackOffset = SpillAreaPaddingBytes +
                          GlobalsAndSubsequentPaddingSize +
                          LocalsSize[NodeIndex];
      }
    } else {
      NextStackOffset += Increment;
    }
    if (UsesFramePointer)
      Var->setStackOffset(-NextStackOffset);
    else
      Var->setStackOffset(SpillAreaSizeBytes - NextStackOffset);
  }
}

InstCall *TargetLowering::makeHelperCall(RuntimeHelper FuncID, Variable *Dest,
                                         SizeT MaxSrcs) {
  constexpr bool HasTailCall = false;
  Constant *CallTarget = Ctx->getRuntimeHelperFunc(FuncID);
  InstCall *Call =
      InstCall::create(Func, MaxSrcs, Dest, CallTarget, HasTailCall);
  return Call;
}

bool TargetLowering::shouldOptimizeMemIntrins() {
  return Func->getOptLevel() >= Opt_1 || getFlags().getForceMemIntrinOpt();
}

void TargetLowering::scalarizeArithmetic(InstArithmetic::OpKind Kind,
                                         Variable *Dest, Operand *Src0,
                                         Operand *Src1) {
  scalarizeInstruction(
      Dest,
      [this, Kind](Variable *Dest, Operand *Src0, Operand *Src1) {
        return Context.insert<InstArithmetic>(Kind, Dest, Src0, Src1);
      },
      Src0, Src1);
}

void TargetLowering::emitWithoutPrefix(const ConstantRelocatable *C,
                                       const char *Suffix) const {
  if (!BuildDefs::dump())
    return;
  Ostream &Str = Ctx->getStrEmit();
  const std::string &EmitStr = C->getEmitString();
  if (!EmitStr.empty()) {
    // C has a custom emit string, so we use it instead of the canonical
    // Name + Offset form.
    Str << EmitStr;
    return;
  }
  Str << C->getName() << Suffix;
  RelocOffsetT Offset = C->getOffset();
  if (Offset) {
    if (Offset > 0)
      Str << "+";
    Str << Offset;
  }
}

std::unique_ptr<TargetDataLowering>
TargetDataLowering::createLowering(GlobalContext *Ctx) {
  TargetArch Target = getFlags().getTargetArch();
  switch (Target) {
  default:
    badTargetFatalError(Target);
#define SUBZERO_TARGET(X)                                                      \
  case TARGET_LOWERING_CLASS_FOR(X):                                           \
    return ::X::createTargetDataLowering(Ctx);
#include "SZTargets.def"
#undef SUBZERO_TARGET
  }
}

TargetDataLowering::~TargetDataLowering() = default;

namespace {

// dataSectionSuffix decides whether to use SectionSuffix or VarName as data
// section suffix. Essentially, when using separate data sections for globals
// SectionSuffix is not necessary.
std::string dataSectionSuffix(const std::string &SectionSuffix,
                              const std::string &VarName,
                              const bool DataSections) {
  if (SectionSuffix.empty() && !DataSections) {
    return "";
  }

  if (DataSections) {
    // With data sections we don't need to use the SectionSuffix.
    return "." + VarName;
  }

  assert(!SectionSuffix.empty());
  return "." + SectionSuffix;
}

} // end of anonymous namespace

void TargetDataLowering::emitGlobal(const VariableDeclaration &Var,
                                    const std::string &SectionSuffix) {
  if (!BuildDefs::dump())
    return;

  // If external and not initialized, this must be a cross test. Don't generate
  // a declaration for such cases.
  const bool IsExternal = Var.isExternal() || getFlags().getDisableInternal();
  if (IsExternal && !Var.hasInitializer())
    return;

  Ostream &Str = Ctx->getStrEmit();
  const bool HasNonzeroInitializer = Var.hasNonzeroInitializer();
  const bool IsConstant = Var.getIsConstant();
  const SizeT Size = Var.getNumBytes();
  const std::string Name = Var.getName().toString();

  Str << "\t.type\t" << Name << ",%object\n";

  const bool UseDataSections = getFlags().getDataSections();
  const std::string Suffix =
      dataSectionSuffix(SectionSuffix, Name, UseDataSections);
  if (IsConstant)
    Str << "\t.section\t.rodata" << Suffix << ",\"a\",%progbits\n";
  else if (HasNonzeroInitializer)
    Str << "\t.section\t.data" << Suffix << ",\"aw\",%progbits\n";
  else
    Str << "\t.section\t.bss" << Suffix << ",\"aw\",%nobits\n";

  if (IsExternal)
    Str << "\t.globl\t" << Name << "\n";

  const uint32_t Align = Var.getAlignment();
  if (Align > 1) {
    assert(llvm::isPowerOf2_32(Align));
    // Use the .p2align directive, since the .align N directive can either
    // interpret N as bytes, or power of 2 bytes, depending on the target.
    Str << "\t.p2align\t" << llvm::Log2_32(Align) << "\n";
  }

  Str << Name << ":\n";

  if (HasNonzeroInitializer) {
    for (const auto *Init : Var.getInitializers()) {
      switch (Init->getKind()) {
      case VariableDeclaration::Initializer::DataInitializerKind: {
        const auto &Data =
            llvm::cast<VariableDeclaration::DataInitializer>(Init)
                ->getContents();
        for (SizeT i = 0; i < Init->getNumBytes(); ++i) {
          Str << "\t.byte\t" << (((unsigned)Data[i]) & 0xff) << "\n";
        }
        break;
      }
      case VariableDeclaration::Initializer::ZeroInitializerKind:
        Str << "\t.zero\t" << Init->getNumBytes() << "\n";
        break;
      case VariableDeclaration::Initializer::RelocInitializerKind: {
        const auto *Reloc =
            llvm::cast<VariableDeclaration::RelocInitializer>(Init);
        Str << "\t" << getEmit32Directive() << "\t";
        Str << Reloc->getDeclaration()->getName();
        if (Reloc->hasFixup()) {
          // TODO(jpp): this is ARM32 specific.
          Str << "(GOTOFF)";
        }
        if (RelocOffsetT Offset = Reloc->getOffset()) {
          if (Offset >= 0 || (Offset == INT32_MIN))
            Str << " + " << Offset;
          else
            Str << " - " << -Offset;
        }
        Str << "\n";
        break;
      }
      }
    }
  } else {
    // NOTE: for non-constant zero initializers, this is BSS (no bits), so an
    // ELF writer would not write to the file, and only track virtual offsets,
    // but the .s writer still needs this .zero and cannot simply use the .size
    // to advance offsets.
    Str << "\t.zero\t" << Size << "\n";
  }

  Str << "\t.size\t" << Name << ", " << Size << "\n";
}

std::unique_ptr<TargetHeaderLowering>
TargetHeaderLowering::createLowering(GlobalContext *Ctx) {
  TargetArch Target = getFlags().getTargetArch();
  switch (Target) {
  default:
    badTargetFatalError(Target);
#define SUBZERO_TARGET(X)                                                      \
  case TARGET_LOWERING_CLASS_FOR(X):                                           \
    return ::X::createTargetHeaderLowering(Ctx);
#include "SZTargets.def"
#undef SUBZERO_TARGET
  }
}

TargetHeaderLowering::~TargetHeaderLowering() = default;

} // end of namespace Ice
