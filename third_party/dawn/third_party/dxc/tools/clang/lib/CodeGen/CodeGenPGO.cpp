//===--- CodeGenPGO.cpp - PGO Instrumentation for LLVM CodeGen --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Instrumentation-based profile-guided optimization
//
//===----------------------------------------------------------------------===//

#include "CodeGenPGO.h"
#include "CodeGenFunction.h"
#include "CoverageMappingGen.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/StmtVisitor.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/ProfileData/InstrProfReader.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MD5.h"

using namespace clang;
using namespace CodeGen;

void CodeGenPGO::setFuncName(StringRef Name,
                             llvm::GlobalValue::LinkageTypes Linkage) {
  StringRef RawFuncName = Name;

  // Function names may be prefixed with a binary '1' to indicate
  // that the backend should not modify the symbols due to any platform
  // naming convention. Do not include that '1' in the PGO profile name.
  if (RawFuncName[0] == '\1')
    RawFuncName = RawFuncName.substr(1);

  FuncName = RawFuncName;
  if (llvm::GlobalValue::isLocalLinkage(Linkage)) {
    // For local symbols, prepend the main file name to distinguish them.
    // Do not include the full path in the file name since there's no guarantee
    // that it will stay the same, e.g., if the files are checked out from
    // version control in different locations.
    if (CGM.getCodeGenOpts().MainFileName.empty())
      FuncName = FuncName.insert(0, "<unknown>:");
    else
      FuncName = FuncName.insert(0, CGM.getCodeGenOpts().MainFileName + ":");
  }

  // If we're generating a profile, create a variable for the name.
  if (CGM.getCodeGenOpts().ProfileInstrGenerate)
    createFuncNameVar(Linkage);
}

void CodeGenPGO::setFuncName(llvm::Function *Fn) {
  setFuncName(Fn->getName(), Fn->getLinkage());
}

void CodeGenPGO::createFuncNameVar(llvm::GlobalValue::LinkageTypes Linkage) {
  // We generally want to match the function's linkage, but available_externally
  // and extern_weak both have the wrong semantics, and anything that doesn't
  // need to link across compilation units doesn't need to be visible at all.
  if (Linkage == llvm::GlobalValue::ExternalWeakLinkage)
    Linkage = llvm::GlobalValue::LinkOnceAnyLinkage;
  else if (Linkage == llvm::GlobalValue::AvailableExternallyLinkage)
    Linkage = llvm::GlobalValue::LinkOnceODRLinkage;
  else if (Linkage == llvm::GlobalValue::InternalLinkage ||
           Linkage == llvm::GlobalValue::ExternalLinkage)
    Linkage = llvm::GlobalValue::PrivateLinkage;

  auto *Value =
      llvm::ConstantDataArray::getString(CGM.getLLVMContext(), FuncName, false);
  FuncNameVar =
      new llvm::GlobalVariable(CGM.getModule(), Value->getType(), true, Linkage,
                               Value, "__llvm_profile_name_" + FuncName);

  // Hide the symbol so that we correctly get a copy for each executable.
  if (!llvm::GlobalValue::isLocalLinkage(FuncNameVar->getLinkage()))
    FuncNameVar->setVisibility(llvm::GlobalValue::HiddenVisibility);
}

namespace {
/// \brief Stable hasher for PGO region counters.
///
/// PGOHash produces a stable hash of a given function's control flow.
///
/// Changing the output of this hash will invalidate all previously generated
/// profiles -- i.e., don't do it.
///
/// \note  When this hash does eventually change (years?), we still need to
/// support old hashes.  We'll need to pull in the version number from the
/// profile data format and use the matching hash function.
class PGOHash {
  uint64_t Working;
  unsigned Count;
  llvm::MD5 MD5;

  static const int NumBitsPerType = 6;
  static const unsigned NumTypesPerWord = sizeof(uint64_t) * 8 / NumBitsPerType;
  static const unsigned TooBig = 1u << NumBitsPerType;

public:
  /// \brief Hash values for AST nodes.
  ///
  /// Distinct values for AST nodes that have region counters attached.
  ///
  /// These values must be stable.  All new members must be added at the end,
  /// and no members should be removed.  Changing the enumeration value for an
  /// AST node will affect the hash of every function that contains that node.
  enum HashType : unsigned char {
    None = 0,
    LabelStmt = 1,
    WhileStmt,
    DoStmt,
    ForStmt,
    CXXForRangeStmt,
    ObjCForCollectionStmt,
    SwitchStmt,
    CaseStmt,
    DefaultStmt,
    IfStmt,
    CXXTryStmt,
    CXXCatchStmt,
    ConditionalOperator,
    BinaryOperatorLAnd,
    BinaryOperatorLOr,
    BinaryConditionalOperator,

    // Keep this last.  It's for the static assert that follows.
    LastHashType
  };
  static_assert(LastHashType <= TooBig, "Too many types in HashType");

  // TODO: When this format changes, take in a version number here, and use the
  // old hash calculation for file formats that used the old hash.
  PGOHash() : Working(0), Count(0) {}
  void combine(HashType Type);
  uint64_t finalize();
};
const int PGOHash::NumBitsPerType;
const unsigned PGOHash::NumTypesPerWord;
const unsigned PGOHash::TooBig;

/// A RecursiveASTVisitor that fills a map of statements to PGO counters.
struct MapRegionCounters : public RecursiveASTVisitor<MapRegionCounters> {
  /// The next counter value to assign.
  unsigned NextCounter;
  /// The function hash.
  PGOHash Hash;
  /// The map of statements to counters.
  llvm::DenseMap<const Stmt *, unsigned> &CounterMap;

  MapRegionCounters(llvm::DenseMap<const Stmt *, unsigned> &CounterMap)
      : NextCounter(0), CounterMap(CounterMap) {}

  // Blocks and lambdas are handled as separate functions, so we need not
  // traverse them in the parent context.
  bool TraverseBlockExpr(BlockExpr *BE) { return true; }
  bool TraverseLambdaBody(LambdaExpr *LE) { return true; }
  bool TraverseCapturedStmt(CapturedStmt *CS) { return true; }

  bool VisitDecl(const Decl *D) {
    switch (D->getKind()) {
    default:
      break;
    case Decl::Function:
    case Decl::CXXMethod:
    case Decl::CXXConstructor:
    case Decl::CXXDestructor:
    case Decl::CXXConversion:
    case Decl::ObjCMethod:
    case Decl::Block:
    case Decl::Captured:
      CounterMap[D->getBody()] = NextCounter++;
      break;
    }
    return true;
  }

  bool VisitStmt(const Stmt *S) {
    auto Type = getHashType(S);
    if (Type == PGOHash::None)
      return true;

    CounterMap[S] = NextCounter++;
    Hash.combine(Type);
    return true;
  }
  PGOHash::HashType getHashType(const Stmt *S) {
    switch (S->getStmtClass()) {
    default:
      break;
    case Stmt::LabelStmtClass:
      return PGOHash::LabelStmt;
    case Stmt::WhileStmtClass:
      return PGOHash::WhileStmt;
    case Stmt::DoStmtClass:
      return PGOHash::DoStmt;
    case Stmt::ForStmtClass:
      return PGOHash::ForStmt;
    case Stmt::CXXForRangeStmtClass:
      return PGOHash::CXXForRangeStmt;
    case Stmt::ObjCForCollectionStmtClass:
      return PGOHash::ObjCForCollectionStmt;
    case Stmt::SwitchStmtClass:
      return PGOHash::SwitchStmt;
    case Stmt::CaseStmtClass:
      return PGOHash::CaseStmt;
    case Stmt::DefaultStmtClass:
      return PGOHash::DefaultStmt;
    case Stmt::IfStmtClass:
      return PGOHash::IfStmt;
    case Stmt::CXXTryStmtClass:
      return PGOHash::CXXTryStmt;
    case Stmt::CXXCatchStmtClass:
      return PGOHash::CXXCatchStmt;
    case Stmt::ConditionalOperatorClass:
      return PGOHash::ConditionalOperator;
    case Stmt::BinaryConditionalOperatorClass:
      return PGOHash::BinaryConditionalOperator;
    case Stmt::BinaryOperatorClass: {
      const BinaryOperator *BO = cast<BinaryOperator>(S);
      if (BO->getOpcode() == BO_LAnd)
        return PGOHash::BinaryOperatorLAnd;
      if (BO->getOpcode() == BO_LOr)
        return PGOHash::BinaryOperatorLOr;
      break;
    }
    }
    return PGOHash::None;
  }
};

/// A StmtVisitor that propagates the raw counts through the AST and
/// records the count at statements where the value may change.
struct ComputeRegionCounts : public ConstStmtVisitor<ComputeRegionCounts> {
  /// PGO state.
  CodeGenPGO &PGO;

  /// A flag that is set when the current count should be recorded on the
  /// next statement, such as at the exit of a loop.
  bool RecordNextStmtCount;

  /// The count at the current location in the traversal.
  uint64_t CurrentCount;

  /// The map of statements to count values.
  llvm::DenseMap<const Stmt *, uint64_t> &CountMap;

  /// BreakContinueStack - Keep counts of breaks and continues inside loops.
  struct BreakContinue {
    uint64_t BreakCount;
    uint64_t ContinueCount;
    BreakContinue() : BreakCount(0), ContinueCount(0) {}
  };
  SmallVector<BreakContinue, 8> BreakContinueStack;

  ComputeRegionCounts(llvm::DenseMap<const Stmt *, uint64_t> &CountMap,
                      CodeGenPGO &PGO)
      : PGO(PGO), RecordNextStmtCount(false), CountMap(CountMap) {}

  void RecordStmtCount(const Stmt *S) {
    if (RecordNextStmtCount) {
      CountMap[S] = CurrentCount;
      RecordNextStmtCount = false;
    }
  }

  /// Set and return the current count.
  uint64_t setCount(uint64_t Count) {
    CurrentCount = Count;
    return Count;
  }

  void VisitStmt(const Stmt *S) {
    RecordStmtCount(S);
    for (const Stmt *Child : S->children())
      if (Child)
        this->Visit(Child);
  }

  void VisitFunctionDecl(const FunctionDecl *D) {
    // Counter tracks entry to the function body.
    uint64_t BodyCount = setCount(PGO.getRegionCount(D->getBody()));
    CountMap[D->getBody()] = BodyCount;
    Visit(D->getBody());
  }

  // Skip lambda expressions. We visit these as FunctionDecls when we're
  // generating them and aren't interested in the body when generating a
  // parent context.
  void VisitLambdaExpr(const LambdaExpr *LE) {}

  void VisitCapturedDecl(const CapturedDecl *D) {
    // Counter tracks entry to the capture body.
    uint64_t BodyCount = setCount(PGO.getRegionCount(D->getBody()));
    CountMap[D->getBody()] = BodyCount;
    Visit(D->getBody());
  }

  void VisitObjCMethodDecl(const ObjCMethodDecl *D) {
    // Counter tracks entry to the method body.
    uint64_t BodyCount = setCount(PGO.getRegionCount(D->getBody()));
    CountMap[D->getBody()] = BodyCount;
    Visit(D->getBody());
  }

  void VisitBlockDecl(const BlockDecl *D) {
    // Counter tracks entry to the block body.
    uint64_t BodyCount = setCount(PGO.getRegionCount(D->getBody()));
    CountMap[D->getBody()] = BodyCount;
    Visit(D->getBody());
  }

  void VisitReturnStmt(const ReturnStmt *S) {
    RecordStmtCount(S);
    if (S->getRetValue())
      Visit(S->getRetValue());
    CurrentCount = 0;
    RecordNextStmtCount = true;
  }

  void VisitCXXThrowExpr(const CXXThrowExpr *E) {
    RecordStmtCount(E);
    if (E->getSubExpr())
      Visit(E->getSubExpr());
    CurrentCount = 0;
    RecordNextStmtCount = true;
  }

  void VisitGotoStmt(const GotoStmt *S) {
    RecordStmtCount(S);
    CurrentCount = 0;
    RecordNextStmtCount = true;
  }

  void VisitLabelStmt(const LabelStmt *S) {
    RecordNextStmtCount = false;
    // Counter tracks the block following the label.
    uint64_t BlockCount = setCount(PGO.getRegionCount(S));
    CountMap[S] = BlockCount;
    Visit(S->getSubStmt());
  }

  void VisitBreakStmt(const BreakStmt *S) {
    RecordStmtCount(S);
    assert(!BreakContinueStack.empty() && "break not in a loop or switch!");
    BreakContinueStack.back().BreakCount += CurrentCount;
    CurrentCount = 0;
    RecordNextStmtCount = true;
  }

  void VisitContinueStmt(const ContinueStmt *S) {
    RecordStmtCount(S);
    assert(!BreakContinueStack.empty() && "continue stmt not in a loop!");
    BreakContinueStack.back().ContinueCount += CurrentCount;
    CurrentCount = 0;
    RecordNextStmtCount = true;
  }

  void VisitWhileStmt(const WhileStmt *S) {
    RecordStmtCount(S);
    uint64_t ParentCount = CurrentCount;

    BreakContinueStack.push_back(BreakContinue());
    // Visit the body region first so the break/continue adjustments can be
    // included when visiting the condition.
    uint64_t BodyCount = setCount(PGO.getRegionCount(S));
    CountMap[S->getBody()] = CurrentCount;
    Visit(S->getBody());
    uint64_t BackedgeCount = CurrentCount;

    // ...then go back and propagate counts through the condition. The count
    // at the start of the condition is the sum of the incoming edges,
    // the backedge from the end of the loop body, and the edges from
    // continue statements.
    BreakContinue BC = BreakContinueStack.pop_back_val();
    uint64_t CondCount =
        setCount(ParentCount + BackedgeCount + BC.ContinueCount);
    CountMap[S->getCond()] = CondCount;
    Visit(S->getCond());
    setCount(BC.BreakCount + CondCount - BodyCount);
    RecordNextStmtCount = true;
  }

  void VisitDoStmt(const DoStmt *S) {
    RecordStmtCount(S);
    uint64_t LoopCount = PGO.getRegionCount(S);

    BreakContinueStack.push_back(BreakContinue());
    // The count doesn't include the fallthrough from the parent scope. Add it.
    uint64_t BodyCount = setCount(LoopCount + CurrentCount);
    CountMap[S->getBody()] = BodyCount;
    Visit(S->getBody());
    uint64_t BackedgeCount = CurrentCount;

    BreakContinue BC = BreakContinueStack.pop_back_val();
    // The count at the start of the condition is equal to the count at the
    // end of the body, plus any continues.
    uint64_t CondCount = setCount(BackedgeCount + BC.ContinueCount);
    CountMap[S->getCond()] = CondCount;
    Visit(S->getCond());
    setCount(BC.BreakCount + CondCount - LoopCount);
    RecordNextStmtCount = true;
  }

  void VisitForStmt(const ForStmt *S) {
    RecordStmtCount(S);
    if (S->getInit())
      Visit(S->getInit());

    uint64_t ParentCount = CurrentCount;

    BreakContinueStack.push_back(BreakContinue());
    // Visit the body region first. (This is basically the same as a while
    // loop; see further comments in VisitWhileStmt.)
    uint64_t BodyCount = setCount(PGO.getRegionCount(S));
    CountMap[S->getBody()] = BodyCount;
    Visit(S->getBody());
    uint64_t BackedgeCount = CurrentCount;
    BreakContinue BC = BreakContinueStack.pop_back_val();

    // The increment is essentially part of the body but it needs to include
    // the count for all the continue statements.
    if (S->getInc()) {
      uint64_t IncCount = setCount(BackedgeCount + BC.ContinueCount);
      CountMap[S->getInc()] = IncCount;
      Visit(S->getInc());
    }

    // ...then go back and propagate counts through the condition.
    uint64_t CondCount =
        setCount(ParentCount + BackedgeCount + BC.ContinueCount);
    if (S->getCond()) {
      CountMap[S->getCond()] = CondCount;
      Visit(S->getCond());
    }
    setCount(BC.BreakCount + CondCount - BodyCount);
    RecordNextStmtCount = true;
  }

  void VisitCXXForRangeStmt(const CXXForRangeStmt *S) {
    RecordStmtCount(S);
    Visit(S->getLoopVarStmt());
    Visit(S->getRangeStmt());
    Visit(S->getBeginEndStmt());

    uint64_t ParentCount = CurrentCount;
    BreakContinueStack.push_back(BreakContinue());
    // Visit the body region first. (This is basically the same as a while
    // loop; see further comments in VisitWhileStmt.)
    uint64_t BodyCount = setCount(PGO.getRegionCount(S));
    CountMap[S->getBody()] = BodyCount;
    Visit(S->getBody());
    uint64_t BackedgeCount = CurrentCount;
    BreakContinue BC = BreakContinueStack.pop_back_val();

    // The increment is essentially part of the body but it needs to include
    // the count for all the continue statements.
    uint64_t IncCount = setCount(BackedgeCount + BC.ContinueCount);
    CountMap[S->getInc()] = IncCount;
    Visit(S->getInc());

    // ...then go back and propagate counts through the condition.
    uint64_t CondCount =
        setCount(ParentCount + BackedgeCount + BC.ContinueCount);
    CountMap[S->getCond()] = CondCount;
    Visit(S->getCond());
    setCount(BC.BreakCount + CondCount - BodyCount);
    RecordNextStmtCount = true;
  }

  void VisitObjCForCollectionStmt(const ObjCForCollectionStmt *S) {
    RecordStmtCount(S);
    Visit(S->getElement());
    uint64_t ParentCount = CurrentCount;
    BreakContinueStack.push_back(BreakContinue());
    // Counter tracks the body of the loop.
    uint64_t BodyCount = setCount(PGO.getRegionCount(S));
    CountMap[S->getBody()] = BodyCount;
    Visit(S->getBody());
    uint64_t BackedgeCount = CurrentCount;
    BreakContinue BC = BreakContinueStack.pop_back_val();

    setCount(BC.BreakCount + ParentCount + BackedgeCount + BC.ContinueCount -
             BodyCount);
    RecordNextStmtCount = true;
  }

  void VisitSwitchStmt(const SwitchStmt *S) {
    RecordStmtCount(S);
    Visit(S->getCond());
    CurrentCount = 0;
    BreakContinueStack.push_back(BreakContinue());
    Visit(S->getBody());
    // If the switch is inside a loop, add the continue counts.
    BreakContinue BC = BreakContinueStack.pop_back_val();
    if (!BreakContinueStack.empty())
      BreakContinueStack.back().ContinueCount += BC.ContinueCount;
    // Counter tracks the exit block of the switch.
    setCount(PGO.getRegionCount(S));
    RecordNextStmtCount = true;
  }

  void VisitSwitchCase(const SwitchCase *S) {
    RecordNextStmtCount = false;
    // Counter for this particular case. This counts only jumps from the
    // switch header and does not include fallthrough from the case before
    // this one.
    uint64_t CaseCount = PGO.getRegionCount(S);
    setCount(CurrentCount + CaseCount);
    // We need the count without fallthrough in the mapping, so it's more useful
    // for branch probabilities.
    CountMap[S] = CaseCount;
    RecordNextStmtCount = true;
    Visit(S->getSubStmt());
  }

  void VisitIfStmt(const IfStmt *S) {
    RecordStmtCount(S);
    uint64_t ParentCount = CurrentCount;
    Visit(S->getCond());

    // Counter tracks the "then" part of an if statement. The count for
    // the "else" part, if it exists, will be calculated from this counter.
    uint64_t ThenCount = setCount(PGO.getRegionCount(S));
    CountMap[S->getThen()] = ThenCount;
    Visit(S->getThen());
    uint64_t OutCount = CurrentCount;

    uint64_t ElseCount = ParentCount - ThenCount;
    if (S->getElse()) {
      setCount(ElseCount);
      CountMap[S->getElse()] = ElseCount;
      Visit(S->getElse());
      OutCount += CurrentCount;
    } else
      OutCount += ElseCount;
    setCount(OutCount);
    RecordNextStmtCount = true;
  }

  void VisitCXXTryStmt(const CXXTryStmt *S) {
    RecordStmtCount(S);
    Visit(S->getTryBlock());
    for (unsigned I = 0, E = S->getNumHandlers(); I < E; ++I)
      Visit(S->getHandler(I));
    // Counter tracks the continuation block of the try statement.
    setCount(PGO.getRegionCount(S));
    RecordNextStmtCount = true;
  }

  void VisitCXXCatchStmt(const CXXCatchStmt *S) {
    RecordNextStmtCount = false;
    // Counter tracks the catch statement's handler block.
    uint64_t CatchCount = setCount(PGO.getRegionCount(S));
    CountMap[S] = CatchCount;
    Visit(S->getHandlerBlock());
  }

  void VisitAbstractConditionalOperator(const AbstractConditionalOperator *E) {
    RecordStmtCount(E);
    uint64_t ParentCount = CurrentCount;
    Visit(E->getCond());

    // Counter tracks the "true" part of a conditional operator. The
    // count in the "false" part will be calculated from this counter.
    uint64_t TrueCount = setCount(PGO.getRegionCount(E));
    CountMap[E->getTrueExpr()] = TrueCount;
    Visit(E->getTrueExpr());
    uint64_t OutCount = CurrentCount;

    uint64_t FalseCount = setCount(ParentCount - TrueCount);
    CountMap[E->getFalseExpr()] = FalseCount;
    Visit(E->getFalseExpr());
    OutCount += CurrentCount;

    setCount(OutCount);
    RecordNextStmtCount = true;
  }

  void VisitBinLAnd(const BinaryOperator *E) {
    RecordStmtCount(E);
    uint64_t ParentCount = CurrentCount;
    Visit(E->getLHS());
    // Counter tracks the right hand side of a logical and operator.
    uint64_t RHSCount = setCount(PGO.getRegionCount(E));
    CountMap[E->getRHS()] = RHSCount;
    Visit(E->getRHS());
    setCount(ParentCount + RHSCount - CurrentCount);
    RecordNextStmtCount = true;
  }

  void VisitBinLOr(const BinaryOperator *E) {
    RecordStmtCount(E);
    uint64_t ParentCount = CurrentCount;
    Visit(E->getLHS());
    // Counter tracks the right hand side of a logical or operator.
    uint64_t RHSCount = setCount(PGO.getRegionCount(E));
    CountMap[E->getRHS()] = RHSCount;
    Visit(E->getRHS());
    setCount(ParentCount + RHSCount - CurrentCount);
    RecordNextStmtCount = true;
  }
};
}

void PGOHash::combine(HashType Type) {
  // Check that we never combine 0 and only have six bits.
  assert(Type && "Hash is invalid: unexpected type 0");
  assert(unsigned(Type) < TooBig && "Hash is invalid: too many types");

  // Pass through MD5 if enough work has built up.
  if (Count && Count % NumTypesPerWord == 0) {
    using namespace llvm::support;
    uint64_t Swapped = endian::byte_swap<uint64_t, little>(Working);
    MD5.update(llvm::makeArrayRef((uint8_t *)&Swapped, sizeof(Swapped)));
    Working = 0;
  }

  // Accumulate the current type.
  ++Count;
  Working = Working << NumBitsPerType | Type;
}

uint64_t PGOHash::finalize() {
  // Use Working as the hash directly if we never used MD5.
  if (Count <= NumTypesPerWord)
    // No need to byte swap here, since none of the math was endian-dependent.
    // This number will be byte-swapped as required on endianness transitions,
    // so we will see the same value on the other side.
    return Working;

  // Check for remaining work in Working.
  if (Working)
    MD5.update(Working);

  // Finalize the MD5 and return the hash.
  llvm::MD5::MD5Result Result;
  MD5.final(Result);
  using namespace llvm::support;
  return endian::read<uint64_t, little, unaligned>(Result);
}

void CodeGenPGO::checkGlobalDecl(GlobalDecl GD) {
  // Make sure we only emit coverage mapping for one constructor/destructor.
  // Clang emits several functions for the constructor and the destructor of
  // a class. Every function is instrumented, but we only want to provide
  // coverage for one of them. Because of that we only emit the coverage mapping
  // for the base constructor/destructor.
  if ((isa<CXXConstructorDecl>(GD.getDecl()) &&
       GD.getCtorType() != Ctor_Base) ||
      (isa<CXXDestructorDecl>(GD.getDecl()) &&
       GD.getDtorType() != Dtor_Base)) {
    SkipCoverageMapping = true;
  }
}

void CodeGenPGO::assignRegionCounters(const Decl *D, llvm::Function *Fn) {
  bool InstrumentRegions = CGM.getCodeGenOpts().ProfileInstrGenerate;
  llvm::IndexedInstrProfReader *PGOReader = CGM.getPGOReader();
  if (!InstrumentRegions && !PGOReader)
    return;
  if (D->isImplicit())
    return;
  CGM.ClearUnusedCoverageMapping(D);
  setFuncName(Fn);

  mapRegionCounters(D);
  if (CGM.getCodeGenOpts().CoverageMapping)
    emitCounterRegionMapping(D);
  if (PGOReader) {
    SourceManager &SM = CGM.getContext().getSourceManager();
    loadRegionCounts(PGOReader, SM.isInMainFile(D->getLocation()));
    computeRegionCounts(D);
    applyFunctionAttributes(PGOReader, Fn);
  }
}

void CodeGenPGO::mapRegionCounters(const Decl *D) {
  RegionCounterMap.reset(new llvm::DenseMap<const Stmt *, unsigned>);
  MapRegionCounters Walker(*RegionCounterMap);
  if (const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(D))
    Walker.TraverseDecl(const_cast<FunctionDecl *>(FD));
  else if (const ObjCMethodDecl *MD = dyn_cast_or_null<ObjCMethodDecl>(D))
    Walker.TraverseDecl(const_cast<ObjCMethodDecl *>(MD));
  else if (const BlockDecl *BD = dyn_cast_or_null<BlockDecl>(D))
    Walker.TraverseDecl(const_cast<BlockDecl *>(BD));
  else if (const CapturedDecl *CD = dyn_cast_or_null<CapturedDecl>(D))
    Walker.TraverseDecl(const_cast<CapturedDecl *>(CD));
  assert(Walker.NextCounter > 0 && "no entry counter mapped for decl");
  NumRegionCounters = Walker.NextCounter;
  FunctionHash = Walker.Hash.finalize();
}

void CodeGenPGO::emitCounterRegionMapping(const Decl *D) {
  if (SkipCoverageMapping)
    return;
  // Don't map the functions inside the system headers
  auto Loc = D->getBody()->getLocStart();
  if (CGM.getContext().getSourceManager().isInSystemHeader(Loc))
    return;

  std::string CoverageMapping;
  llvm::raw_string_ostream OS(CoverageMapping);
  CoverageMappingGen MappingGen(*CGM.getCoverageMapping(),
                                CGM.getContext().getSourceManager(),
                                CGM.getLangOpts(), RegionCounterMap.get());
  MappingGen.emitCounterMapping(D, OS);
  OS.flush();

  if (CoverageMapping.empty())
    return;

  CGM.getCoverageMapping()->addFunctionMappingRecord(
      FuncNameVar, FuncName, FunctionHash, CoverageMapping);
}

void
CodeGenPGO::emitEmptyCounterMapping(const Decl *D, StringRef Name,
                                    llvm::GlobalValue::LinkageTypes Linkage) {
  if (SkipCoverageMapping)
    return;
  // Don't map the functions inside the system headers
  auto Loc = D->getBody()->getLocStart();
  if (CGM.getContext().getSourceManager().isInSystemHeader(Loc))
    return;

  std::string CoverageMapping;
  llvm::raw_string_ostream OS(CoverageMapping);
  CoverageMappingGen MappingGen(*CGM.getCoverageMapping(),
                                CGM.getContext().getSourceManager(),
                                CGM.getLangOpts());
  MappingGen.emitEmptyMapping(D, OS);
  OS.flush();

  if (CoverageMapping.empty())
    return;

  setFuncName(Name, Linkage);
  CGM.getCoverageMapping()->addFunctionMappingRecord(
      FuncNameVar, FuncName, FunctionHash, CoverageMapping);
}

void CodeGenPGO::computeRegionCounts(const Decl *D) {
  StmtCountMap.reset(new llvm::DenseMap<const Stmt *, uint64_t>);
  ComputeRegionCounts Walker(*StmtCountMap, *this);
  if (const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(D))
    Walker.VisitFunctionDecl(FD);
  else if (const ObjCMethodDecl *MD = dyn_cast_or_null<ObjCMethodDecl>(D))
    Walker.VisitObjCMethodDecl(MD);
  else if (const BlockDecl *BD = dyn_cast_or_null<BlockDecl>(D))
    Walker.VisitBlockDecl(BD);
  else if (const CapturedDecl *CD = dyn_cast_or_null<CapturedDecl>(D))
    Walker.VisitCapturedDecl(const_cast<CapturedDecl *>(CD));
}

void
CodeGenPGO::applyFunctionAttributes(llvm::IndexedInstrProfReader *PGOReader,
                                    llvm::Function *Fn) {
  if (!haveRegionCounts())
    return;

  uint64_t MaxFunctionCount = PGOReader->getMaximumFunctionCount();
  uint64_t FunctionCount = getRegionCount(0);
  if (FunctionCount >= (uint64_t)(0.3 * (double)MaxFunctionCount))
    // Turn on InlineHint attribute for hot functions.
    // FIXME: 30% is from preliminary tuning on SPEC, it may not be optimal.
    Fn->addFnAttr(llvm::Attribute::InlineHint);
  else if (FunctionCount <= (uint64_t)(0.01 * (double)MaxFunctionCount))
    // Turn on Cold attribute for cold functions.
    // FIXME: 1% is from preliminary tuning on SPEC, it may not be optimal.
    Fn->addFnAttr(llvm::Attribute::Cold);

  Fn->setEntryCount(FunctionCount);
}

void CodeGenPGO::emitCounterIncrement(CGBuilderTy &Builder, const Stmt *S) {
  if (!CGM.getCodeGenOpts().ProfileInstrGenerate || !RegionCounterMap)
    return;
  if (!Builder.GetInsertPoint())
    return;

  unsigned Counter = (*RegionCounterMap)[S];
  auto *I8PtrTy = llvm::Type::getInt8PtrTy(CGM.getLLVMContext());
  Builder.CreateCall(CGM.getIntrinsic(llvm::Intrinsic::instrprof_increment),
                     {llvm::ConstantExpr::getBitCast(FuncNameVar, I8PtrTy),
                      Builder.getInt64(FunctionHash),
                      Builder.getInt32(NumRegionCounters),
                      Builder.getInt32(Counter)});
}

void CodeGenPGO::loadRegionCounts(llvm::IndexedInstrProfReader *PGOReader,
                                  bool IsInMainFile) {
  CGM.getPGOStats().addVisited(IsInMainFile);
  RegionCounts.clear();
  if (std::error_code EC =
          PGOReader->getFunctionCounts(FuncName, FunctionHash, RegionCounts)) {
    if (EC == llvm::instrprof_error::unknown_function)
      CGM.getPGOStats().addMissing(IsInMainFile);
    else if (EC == llvm::instrprof_error::hash_mismatch)
      CGM.getPGOStats().addMismatched(IsInMainFile);
    else if (EC == llvm::instrprof_error::malformed)
      // TODO: Consider a more specific warning for this case.
      CGM.getPGOStats().addMismatched(IsInMainFile);
    RegionCounts.clear();
  }
}

/// \brief Calculate what to divide by to scale weights.
///
/// Given the maximum weight, calculate a divisor that will scale all the
/// weights to strictly less than UINT32_MAX.
static uint64_t calculateWeightScale(uint64_t MaxWeight) {
  return MaxWeight < UINT32_MAX ? 1 : MaxWeight / UINT32_MAX + 1;
}

/// \brief Scale an individual branch weight (and add 1).
///
/// Scale a 64-bit weight down to 32-bits using \c Scale.
///
/// According to Laplace's Rule of Succession, it is better to compute the
/// weight based on the count plus 1, so universally add 1 to the value.
///
/// \pre \c Scale was calculated by \a calculateWeightScale() with a weight no
/// greater than \c Weight.
static uint32_t scaleBranchWeight(uint64_t Weight, uint64_t Scale) {
  assert(Scale && "scale by 0?");
  uint64_t Scaled = Weight / Scale + 1;
  assert(Scaled <= UINT32_MAX && "overflow 32-bits");
  return Scaled;
}

llvm::MDNode *CodeGenFunction::createProfileWeights(uint64_t TrueCount,
                                                    uint64_t FalseCount) {
  // Check for empty weights.
  if (!TrueCount && !FalseCount)
    return nullptr;

  // Calculate how to scale down to 32-bits.
  uint64_t Scale = calculateWeightScale(std::max(TrueCount, FalseCount));

  llvm::MDBuilder MDHelper(CGM.getLLVMContext());
  return MDHelper.createBranchWeights(scaleBranchWeight(TrueCount, Scale),
                                      scaleBranchWeight(FalseCount, Scale));
}

llvm::MDNode *
CodeGenFunction::createProfileWeights(ArrayRef<uint64_t> Weights) {
  // We need at least two elements to create meaningful weights.
  if (Weights.size() < 2)
    return nullptr;

  // Check for empty weights.
  uint64_t MaxWeight = *std::max_element(Weights.begin(), Weights.end());
  if (MaxWeight == 0)
    return nullptr;

  // Calculate how to scale down to 32-bits.
  uint64_t Scale = calculateWeightScale(MaxWeight);

  SmallVector<uint32_t, 16> ScaledWeights;
  ScaledWeights.reserve(Weights.size());
  for (uint64_t W : Weights)
    ScaledWeights.push_back(scaleBranchWeight(W, Scale));

  llvm::MDBuilder MDHelper(CGM.getLLVMContext());
  return MDHelper.createBranchWeights(ScaledWeights);
}

llvm::MDNode *CodeGenFunction::createProfileWeightsForLoop(const Stmt *Cond,
                                                           uint64_t LoopCount) {
  if (!PGO.haveRegionCounts())
    return nullptr;
  Optional<uint64_t> CondCount = PGO.getStmtCount(Cond);
  assert(CondCount.hasValue() && "missing expected loop condition count");
  if (*CondCount == 0)
    return nullptr;
  return createProfileWeights(LoopCount,
                              std::max(*CondCount, LoopCount) - LoopCount);
}
