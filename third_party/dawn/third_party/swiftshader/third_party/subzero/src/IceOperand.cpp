//===- subzero/src/IceOperand.cpp - High-level operand implementation -----===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the Operand class and its target-independent subclasses,
/// primarily for the methods of the Variable class.
///
//===----------------------------------------------------------------------===//

#include "IceOperand.h"

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceInst.h"
#include "IceInstVarIter.h"
#include "IceMemory.h"
#include "IceTargetLowering.h" // dumping stack/frame pointer register

namespace Ice {

void Constant::initShouldBePooled() {
  ShouldBePooled = TargetLowering::shouldBePooled(this);
}

bool operator==(const RelocatableTuple &A, const RelocatableTuple &B) {
  // A and B are the same if:
  //   (1) they have the same name; and
  //   (2) they have the same offset.
  //
  // (1) is trivial to check, but (2) requires some care.
  //
  // For (2):
  //  if A and B have known offsets (i.e., no symbolic references), then
  //     A == B -> A.Offset == B.Offset.
  //  else each element i in A.OffsetExpr[i] must be the same (or have the same
  //    value) as B.OffsetExpr[i].
  if (A.Name != B.Name) {
    return false;
  }

  bool BothHaveKnownOffsets = true;
  RelocOffsetT OffsetA = A.Offset;
  RelocOffsetT OffsetB = B.Offset;
  for (SizeT i = 0; i < A.OffsetExpr.size() && BothHaveKnownOffsets; ++i) {
    BothHaveKnownOffsets = A.OffsetExpr[i]->hasOffset();
    if (BothHaveKnownOffsets) {
      OffsetA += A.OffsetExpr[i]->getOffset();
    }
  }
  for (SizeT i = 0; i < B.OffsetExpr.size() && BothHaveKnownOffsets; ++i) {
    BothHaveKnownOffsets = B.OffsetExpr[i]->hasOffset();
    if (BothHaveKnownOffsets) {
      OffsetB += B.OffsetExpr[i]->getOffset();
    }
  }
  if (BothHaveKnownOffsets) {
    // Both have known offsets (i.e., no unresolved symbolic references), so
    // A == B -> A.Offset == B.Offset.
    return OffsetA == OffsetB;
  }

  // Otherwise, A and B are not the same if their OffsetExpr's have different
  // sizes.
  if (A.OffsetExpr.size() != B.OffsetExpr.size()) {
    return false;
  }

  // If the OffsetExprs' sizes are the same, then
  // for each i in OffsetExprSize:
  for (SizeT i = 0; i < A.OffsetExpr.size(); ++i) {
    const auto *const RelocOffsetA = A.OffsetExpr[i];
    const auto *const RelocOffsetB = B.OffsetExpr[i];
    if (RelocOffsetA->hasOffset() && RelocOffsetB->hasOffset()) {
      // A.OffsetExpr[i].Offset == B.OffsetExpr[i].Offset iff they are both
      // defined;
      if (RelocOffsetA->getOffset() != RelocOffsetB->getOffset()) {
        return false;
      }
    } else if (RelocOffsetA != RelocOffsetB) {
      // or, if they are undefined, then the RelocOffsets must be the same.
      return false;
    }
  }

  return true;
}

RegNumT::BaseType RegNumT::Limit = 0;

bool operator<(const RegWeight &A, const RegWeight &B) {
  return A.getWeight() < B.getWeight();
}
bool operator<=(const RegWeight &A, const RegWeight &B) { return !(B < A); }
bool operator==(const RegWeight &A, const RegWeight &B) {
  return !(B < A) && !(A < B);
}

void LiveRange::addSegment(InstNumberT Start, InstNumberT End, CfgNode *Node) {
  if (getFlags().getSplitGlobalVars()) {
    // Disable merging to make sure a live range 'segment' has a single node.
    // Might be possible to enable when the target segment has the same node.
    assert(NodeMap.find(Start) == NodeMap.end());
    NodeMap[Start] = Node;
  } else {
    if (!Range.empty()) {
      // Check for merge opportunity.
      InstNumberT CurrentEnd = Range.back().second;
      assert(Start >= CurrentEnd);
      if (Start == CurrentEnd) {
        Range.back().second = End;
        return;
      }
    }
  }
  Range.push_back(RangeElementType(Start, End));
}

// Returns true if this live range ends before Other's live range starts. This
// means that the highest instruction number in this live range is less than or
// equal to the lowest instruction number of the Other live range.
bool LiveRange::endsBefore(const LiveRange &Other) const {
  // Neither range should be empty, but let's be graceful.
  if (Range.empty() || Other.Range.empty())
    return true;
  InstNumberT MyEnd = (*Range.rbegin()).second;
  InstNumberT OtherStart = (*Other.Range.begin()).first;
  return MyEnd <= OtherStart;
}

// Returns true if there is any overlap between the two live ranges.
bool LiveRange::overlaps(const LiveRange &Other, bool UseTrimmed) const {
  // Do a two-finger walk through the two sorted lists of segments.
  auto I1 = (UseTrimmed ? TrimmedBegin : Range.begin()),
       I2 = (UseTrimmed ? Other.TrimmedBegin : Other.Range.begin());
  auto E1 = Range.end(), E2 = Other.Range.end();
  while (I1 != E1 && I2 != E2) {
    if (I1->second <= I2->first) {
      ++I1;
      continue;
    }
    if (I2->second <= I1->first) {
      ++I2;
      continue;
    }
    return true;
  }
  return false;
}

bool LiveRange::overlapsInst(InstNumberT OtherBegin, bool UseTrimmed) const {
  bool Result = false;
  for (auto I = (UseTrimmed ? TrimmedBegin : Range.begin()), E = Range.end();
       I != E; ++I) {
    if (OtherBegin < I->first) {
      Result = false;
      break;
    }
    if (OtherBegin < I->second) {
      Result = true;
      break;
    }
  }
  // This is an equivalent but less inefficient implementation. It's expensive
  // enough that we wouldn't want to run it under any build, but it could be
  // enabled if e.g. the LiveRange implementation changes and extra testing is
  // needed.
  if (BuildDefs::extraValidation()) {
    LiveRange Temp;
    Temp.addSegment(OtherBegin, OtherBegin + 1);
    bool Validation = overlaps(Temp);
    (void)Validation;
    assert(Result == Validation);
  }
  return Result;
}

// Returns true if the live range contains the given instruction number. This
// is only used for validating the live range calculation. The IsDest argument
// indicates whether the Variable being tested is used in the Dest position (as
// opposed to a Src position).
bool LiveRange::containsValue(InstNumberT Value, bool IsDest) const {
  for (const RangeElementType &I : Range) {
    if (I.first <= Value &&
        (Value < I.second || (!IsDest && Value == I.second)))
      return true;
  }
  return false;
}

void LiveRange::trim(InstNumberT Lower) {
  while (TrimmedBegin != Range.end() && TrimmedBegin->second <= Lower)
    ++TrimmedBegin;
}

const Variable *Variable::asType(const Cfg *Func, Type Ty,
                                 RegNumT NewRegNum) const {
  // Note: This returns a Variable, even if the "this" object is a subclass of
  // Variable.
  if (!BuildDefs::dump() || getType() == Ty)
    return this;
  static constexpr SizeT One = 1;
  auto *V = new (CfgLocalAllocator<Variable>().allocate(One))
      Variable(Func, kVariable, Ty, Number);
  V->Name = Name;
  V->RegNum = NewRegNum.hasValue() ? NewRegNum : RegNum;
  V->StackOffset = StackOffset;
  V->LinkedTo = LinkedTo;
  return V;
}

RegWeight Variable::getWeight(const Cfg *Func) const {
  if (mustHaveReg())
    return RegWeight(RegWeight::Inf);
  if (mustNotHaveReg())
    return RegWeight(RegWeight::Zero);
  return Func->getVMetadata()->getUseWeight(this);
}

int32_t
Variable::getRematerializableOffset(const ::Ice::TargetLowering *Target) {
  int32_t Disp = getStackOffset();
  const auto RegNum = getRegNum();
  if (RegNum == Target->getFrameReg()) {
    Disp += Target->getFrameFixedAllocaOffset();
  } else if (RegNum != Target->getStackReg()) {
    llvm::report_fatal_error("Unexpected rematerializable register type");
  }
  return Disp;
}
void VariableTracking::markUse(MetadataKind TrackingKind, const Inst *Instr,
                               CfgNode *Node, bool IsImplicit) {
  (void)TrackingKind;

  // Increment the use weight depending on the loop nest depth. The weight is
  // exponential in the nest depth as inner loops are expected to be executed
  // an exponentially greater number of times.
  constexpr uint32_t LogLoopTripCountEstimate = 2; // 2^2 = 4
  constexpr SizeT MaxShift = sizeof(uint32_t) * CHAR_BIT - 1;
  constexpr SizeT MaxLoopNestDepth = MaxShift / LogLoopTripCountEstimate;
  const uint32_t LoopNestDepth =
      std::min(Node->getLoopNestDepth(), MaxLoopNestDepth);
  const uint32_t ThisUseWeight = uint32_t(1)
                                 << LoopNestDepth * LogLoopTripCountEstimate;
  UseWeight.addWeight(ThisUseWeight);

  if (MultiBlock == MBS_MultiBlock)
    return;
  // TODO(stichnot): If the use occurs as a source operand in the first
  // instruction of the block, and its definition is in this block's only
  // predecessor, we might consider not marking this as a separate use. This
  // may also apply if it's the first instruction of the block that actually
  // uses a Variable.
  assert(Node);
  bool MakeMulti = false;
  if (IsImplicit)
    MakeMulti = true;
  // A phi source variable conservatively needs to be marked as multi-block,
  // even if its definition is in the same block. This is because there can be
  // additional control flow before branching back to this node, and the
  // variable is live throughout those nodes.
  if (Instr && llvm::isa<InstPhi>(Instr))
    MakeMulti = true;

  if (!MakeMulti) {
    switch (MultiBlock) {
    case MBS_Unknown:
    case MBS_NoUses:
      MultiBlock = MBS_SingleBlock;
      SingleUseNode = Node;
      break;
    case MBS_SingleBlock:
      if (SingleUseNode != Node)
        MakeMulti = true;
      break;
    case MBS_MultiBlock:
      break;
    }
  }

  if (MakeMulti) {
    MultiBlock = MBS_MultiBlock;
    SingleUseNode = nullptr;
  }
}

void VariableTracking::markDef(MetadataKind TrackingKind, const Inst *Instr,
                               CfgNode *Node) {
  // TODO(stichnot): If the definition occurs in the last instruction of the
  // block, consider not marking this as a separate use. But be careful not to
  // omit all uses of the variable if markDef() and markUse() both use this
  // optimization.
  assert(Node);
  // Verify that instructions are added in increasing order.
  if (BuildDefs::asserts()) {
    if (TrackingKind == VMK_All) {
      const Inst *LastInstruction =
          Definitions.empty() ? FirstOrSingleDefinition : Definitions.back();
      (void)LastInstruction;
      assert(LastInstruction == nullptr ||
             Instr->getNumber() >= LastInstruction->getNumber());
    }
  }
  constexpr bool IsImplicit = false;
  markUse(TrackingKind, Instr, Node, IsImplicit);
  if (TrackingKind == VMK_Uses)
    return;
  if (FirstOrSingleDefinition == nullptr)
    FirstOrSingleDefinition = Instr;
  else if (TrackingKind == VMK_All)
    Definitions.push_back(Instr);
  switch (MultiDef) {
  case MDS_Unknown:
    assert(SingleDefNode == nullptr);
    MultiDef = MDS_SingleDef;
    SingleDefNode = Node;
    break;
  case MDS_SingleDef:
    assert(SingleDefNode);
    if (Node == SingleDefNode) {
      MultiDef = MDS_MultiDefSingleBlock;
    } else {
      MultiDef = MDS_MultiDefMultiBlock;
      SingleDefNode = nullptr;
    }
    break;
  case MDS_MultiDefSingleBlock:
    assert(SingleDefNode);
    if (Node != SingleDefNode) {
      MultiDef = MDS_MultiDefMultiBlock;
      SingleDefNode = nullptr;
    }
    break;
  case MDS_MultiDefMultiBlock:
    assert(SingleDefNode == nullptr);
    break;
  }
}

const Inst *VariableTracking::getFirstDefinitionSingleBlock() const {
  switch (MultiDef) {
  case MDS_Unknown:
  case MDS_MultiDefMultiBlock:
    return nullptr;
  case MDS_SingleDef:
  case MDS_MultiDefSingleBlock:
    assert(FirstOrSingleDefinition);
    return FirstOrSingleDefinition;
  }
  return nullptr;
}

const Inst *VariableTracking::getSingleDefinition() const {
  switch (MultiDef) {
  case MDS_Unknown:
  case MDS_MultiDefMultiBlock:
  case MDS_MultiDefSingleBlock:
    return nullptr;
  case MDS_SingleDef:
    assert(FirstOrSingleDefinition);
    return FirstOrSingleDefinition;
  }
  return nullptr;
}

const Inst *VariableTracking::getFirstDefinition() const {
  switch (MultiDef) {
  case MDS_Unknown:
    return nullptr;
  case MDS_MultiDefMultiBlock:
  case MDS_SingleDef:
  case MDS_MultiDefSingleBlock:
    assert(FirstOrSingleDefinition);
    return FirstOrSingleDefinition;
  }
  return nullptr;
}

void VariablesMetadata::init(MetadataKind TrackingKind) {
  TimerMarker T(TimerStack::TT_vmetadata, Func);
  Kind = TrackingKind;
  Metadata.clear();
  Metadata.resize(Func->getNumVariables(), VariableTracking::MBS_NoUses);

  // Mark implicit args as being used in the entry node.
  for (Variable *Var : Func->getImplicitArgs()) {
    constexpr Inst *NoInst = nullptr;
    CfgNode *EntryNode = Func->getEntryNode();
    constexpr bool IsImplicit = true;
    Metadata[Var->getIndex()].markUse(Kind, NoInst, EntryNode, IsImplicit);
  }

  for (CfgNode *Node : Func->getNodes())
    addNode(Node);
}

void VariablesMetadata::addNode(CfgNode *Node) {
  if (Func->getNumVariables() > Metadata.size())
    Metadata.resize(Func->getNumVariables());

  for (Inst &I : Node->getPhis()) {
    if (I.isDeleted())
      continue;
    if (Variable *Dest = I.getDest()) {
      SizeT DestNum = Dest->getIndex();
      assert(DestNum < Metadata.size());
      Metadata[DestNum].markDef(Kind, &I, Node);
    }
    for (SizeT SrcNum = 0; SrcNum < I.getSrcSize(); ++SrcNum) {
      if (auto *Var = llvm::dyn_cast<Variable>(I.getSrc(SrcNum))) {
        SizeT VarNum = Var->getIndex();
        assert(VarNum < Metadata.size());
        constexpr bool IsImplicit = false;
        Metadata[VarNum].markUse(Kind, &I, Node, IsImplicit);
      }
    }
  }

  for (Inst &I : Node->getInsts()) {
    if (I.isDeleted())
      continue;
    // Note: The implicit definitions (and uses) from InstFakeKill are
    // deliberately ignored.
    if (Variable *Dest = I.getDest()) {
      SizeT DestNum = Dest->getIndex();
      assert(DestNum < Metadata.size());
      Metadata[DestNum].markDef(Kind, &I, Node);
    }
    FOREACH_VAR_IN_INST(Var, I) {
      SizeT VarNum = Var->getIndex();
      assert(VarNum < Metadata.size());
      constexpr bool IsImplicit = false;
      Metadata[VarNum].markUse(Kind, &I, Node, IsImplicit);
    }
  }
}

bool VariablesMetadata::isMultiDef(const Variable *Var) const {
  assert(Kind != VMK_Uses);
  if (Var->getIsArg())
    return false;
  if (!isTracked(Var))
    return true; // conservative answer
  SizeT VarNum = Var->getIndex();
  // Conservatively return true if the state is unknown.
  return Metadata[VarNum].getMultiDef() != VariableTracking::MDS_SingleDef;
}

bool VariablesMetadata::isMultiBlock(const Variable *Var) const {
  if (Var->getIsArg())
    return true;
  if (Var->isRematerializable())
    return false;
  if (!isTracked(Var))
    return true; // conservative answer
  SizeT VarNum = Var->getIndex();
  switch (Metadata[VarNum].getMultiBlock()) {
  case VariableTracking::MBS_NoUses:
  case VariableTracking::MBS_SingleBlock:
    return false;
  // Conservatively return true if the state is unknown.
  case VariableTracking::MBS_Unknown:
  case VariableTracking::MBS_MultiBlock:
    return true;
  }
  assert(0);
  return true;
}

bool VariablesMetadata::isSingleBlock(const Variable *Var) const {
  if (Var->getIsArg())
    return false;
  if (Var->isRematerializable())
    return false;
  if (!isTracked(Var))
    return false; // conservative answer
  SizeT VarNum = Var->getIndex();
  switch (Metadata[VarNum].getMultiBlock()) {
  case VariableTracking::MBS_SingleBlock:
    return true;
  case VariableTracking::MBS_Unknown:
  case VariableTracking::MBS_NoUses:
  case VariableTracking::MBS_MultiBlock:
    return false;
  }
  assert(0);
  return false;
}

const Inst *
VariablesMetadata::getFirstDefinitionSingleBlock(const Variable *Var) const {
  assert(Kind != VMK_Uses);
  if (!isTracked(Var))
    return nullptr; // conservative answer
  SizeT VarNum = Var->getIndex();
  return Metadata[VarNum].getFirstDefinitionSingleBlock();
}

const Inst *VariablesMetadata::getSingleDefinition(const Variable *Var) const {
  assert(Kind != VMK_Uses);
  if (!isTracked(Var))
    return nullptr; // conservative answer
  SizeT VarNum = Var->getIndex();
  return Metadata[VarNum].getSingleDefinition();
}

const Inst *VariablesMetadata::getFirstDefinition(const Variable *Var) const {
  assert(Kind != VMK_Uses);
  if (!isTracked(Var))
    return nullptr; // conservative answer
  SizeT VarNum = Var->getIndex();
  return Metadata[VarNum].getFirstDefinition();
}

const InstDefList &
VariablesMetadata::getLatterDefinitions(const Variable *Var) const {
  assert(Kind == VMK_All);
  if (!isTracked(Var)) {
    // NoDefinitions has to be initialized after we've had a chance to set the
    // CfgAllocator, so it can't be a static global object. Also, while C++11
    // guarantees the initialization of static local objects to be thread-safe,
    // we use a pointer to it so we can avoid frequent  mutex locking overhead.
    if (NoDefinitions == nullptr) {
      static const InstDefList NoDefinitionsInstance;
      NoDefinitions = &NoDefinitionsInstance;
    }
    return *NoDefinitions;
  }
  SizeT VarNum = Var->getIndex();
  return Metadata[VarNum].getLatterDefinitions();
}

CfgNode *VariablesMetadata::getLocalUseNode(const Variable *Var) const {
  if (!isTracked(Var))
    return nullptr; // conservative answer
  SizeT VarNum = Var->getIndex();
  return Metadata[VarNum].getNode();
}

RegWeight VariablesMetadata::getUseWeight(const Variable *Var) const {
  if (!isTracked(Var))
    return RegWeight(1); // conservative answer
  SizeT VarNum = Var->getIndex();
  return Metadata[VarNum].getUseWeight();
}

const InstDefList *VariablesMetadata::NoDefinitions = nullptr;

// ======================== dump routines ======================== //

void Variable::emit(const Cfg *Func) const {
  if (BuildDefs::dump())
    Func->getTarget()->emitVariable(this);
}

void Variable::dump(const Cfg *Func, Ostream &Str) const {
  if (!BuildDefs::dump())
    return;
  if (Func == nullptr) {
    Str << "%" << getName();
    return;
  }
  if (Func->isVerbose(IceV_RegOrigins) ||
      (!hasReg() && !Func->getTarget()->hasComputedFrame())) {
    Str << "%" << getName();
    for (Variable *Link = getLinkedTo(); Link != nullptr;
         Link = Link->getLinkedTo()) {
      Str << ":%" << Link->getName();
    }
  }
  if (hasReg()) {
    if (Func->isVerbose(IceV_RegOrigins))
      Str << ":";
    Str << Func->getTarget()->getRegName(RegNum, getType());
  } else if (Func->getTarget()->hasComputedFrame()) {
    if (Func->isVerbose(IceV_RegOrigins))
      Str << ":";
    const auto BaseRegisterNumber =
        hasReg() ? getBaseRegNum() : Func->getTarget()->getFrameOrStackReg();
    Str << "["
        << Func->getTarget()->getRegName(BaseRegisterNumber, IceType_i32);
    if (hasKnownStackOffset()) {
      int32_t Offset = getStackOffset();
      if (Offset) {
        if (Offset > 0)
          Str << "+";
        Str << Offset;
      }
    }
    Str << "]";
  }
}

template <> void ConstantInteger32::emit(TargetLowering *Target) const {
  Target->emit(this);
}

template <> void ConstantInteger64::emit(TargetLowering *Target) const {
  Target->emit(this);
}

template <> void ConstantFloat::emit(TargetLowering *Target) const {
  Target->emit(this);
}

template <> void ConstantDouble::emit(TargetLowering *Target) const {
  Target->emit(this);
}

void ConstantRelocatable::emit(TargetLowering *Target) const {
  Target->emit(this);
}

void ConstantRelocatable::emitWithoutPrefix(const TargetLowering *Target,
                                            const char *Suffix) const {
  Target->emitWithoutPrefix(this, Suffix);
}

void ConstantRelocatable::dump(const Cfg *, Ostream &Str) const {
  if (!BuildDefs::dump())
    return;
  if (!EmitString.empty()) {
    Str << EmitString;
    return;
  }
  Str << "@" << (Name.hasStdString() ? Name.toString() : "<Unnamed>");
  const RelocOffsetT Offset = getOffset();
  if (Offset) {
    if (Offset >= 0) {
      Str << "+";
    }
    Str << Offset;
  }
}

void ConstantUndef::emit(TargetLowering *Target) const { Target->emit(this); }

void LiveRange::dump(Ostream &Str) const {
  if (!BuildDefs::dump())
    return;
  bool First = true;
  for (const RangeElementType &I : Range) {
    if (!First)
      Str << ", ";
    First = false;
    Str << "[" << I.first << ":" << I.second << ")";
  }
}

Ostream &operator<<(Ostream &Str, const LiveRange &L) {
  if (!BuildDefs::dump())
    return Str;
  L.dump(Str);
  return Str;
}

Ostream &operator<<(Ostream &Str, const RegWeight &W) {
  if (!BuildDefs::dump())
    return Str;
  if (W.getWeight() == RegWeight::Inf)
    Str << "Inf";
  else
    Str << W.getWeight();
  return Str;
}

} // end of namespace Ice
