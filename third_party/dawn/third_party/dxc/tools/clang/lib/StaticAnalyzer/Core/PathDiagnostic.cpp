//===--- PathDiagnostic.cpp - Path-Specific Diagnostic Handling -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the PathDiagnostic-related interfaces.
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Core/BugReporter/PathDiagnostic.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/ParentMap.h"
#include "clang/AST/StmtCXX.h"
#include "clang/Basic/SourceManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/ExplodedGraph.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace ento;

bool PathDiagnosticMacroPiece::containsEvent() const {
  for (PathPieces::const_iterator I = subPieces.begin(), E = subPieces.end();
       I!=E; ++I) {
    if (isa<PathDiagnosticEventPiece>(*I))
      return true;
    if (PathDiagnosticMacroPiece *MP = dyn_cast<PathDiagnosticMacroPiece>(*I))
      if (MP->containsEvent())
        return true;
  }
  return false;
}

static StringRef StripTrailingDots(StringRef s) {
  for (StringRef::size_type i = s.size(); i != 0; --i)
    if (s[i - 1] != '.')
      return s.substr(0, i);
  return "";
}

PathDiagnosticPiece::PathDiagnosticPiece(StringRef s,
                                         Kind k, DisplayHint hint)
  : str(StripTrailingDots(s)), kind(k), Hint(hint),
    LastInMainSourceFile(false) {}

PathDiagnosticPiece::PathDiagnosticPiece(Kind k, DisplayHint hint)
  : kind(k), Hint(hint), LastInMainSourceFile(false) {}

PathDiagnosticPiece::~PathDiagnosticPiece() {}
PathDiagnosticEventPiece::~PathDiagnosticEventPiece() {}
PathDiagnosticCallPiece::~PathDiagnosticCallPiece() {}
PathDiagnosticControlFlowPiece::~PathDiagnosticControlFlowPiece() {}
PathDiagnosticMacroPiece::~PathDiagnosticMacroPiece() {}


PathPieces::~PathPieces() {}

void PathPieces::flattenTo(PathPieces &Primary, PathPieces &Current,
                           bool ShouldFlattenMacros) const {
  for (PathPieces::const_iterator I = begin(), E = end(); I != E; ++I) {
    PathDiagnosticPiece *Piece = I->get();

    switch (Piece->getKind()) {
    case PathDiagnosticPiece::Call: {
      PathDiagnosticCallPiece *Call = cast<PathDiagnosticCallPiece>(Piece);
      IntrusiveRefCntPtr<PathDiagnosticEventPiece> CallEnter =
        Call->getCallEnterEvent();
      if (CallEnter)
        Current.push_back(CallEnter);
      Call->path.flattenTo(Primary, Primary, ShouldFlattenMacros);
      IntrusiveRefCntPtr<PathDiagnosticEventPiece> callExit =
        Call->getCallExitEvent();
      if (callExit)
        Current.push_back(callExit);
      break;
    }
    case PathDiagnosticPiece::Macro: {
      PathDiagnosticMacroPiece *Macro = cast<PathDiagnosticMacroPiece>(Piece);
      if (ShouldFlattenMacros) {
        Macro->subPieces.flattenTo(Primary, Primary, ShouldFlattenMacros);
      } else {
        Current.push_back(Piece);
        PathPieces NewPath;
        Macro->subPieces.flattenTo(Primary, NewPath, ShouldFlattenMacros);
        // FIXME: This probably shouldn't mutate the original path piece.
        Macro->subPieces = NewPath;
      }
      break;
    }
    case PathDiagnosticPiece::Event:
    case PathDiagnosticPiece::ControlFlow:
      Current.push_back(Piece);
      break;
    }
  }
}


PathDiagnostic::~PathDiagnostic() {}

PathDiagnostic::PathDiagnostic(StringRef CheckName, const Decl *declWithIssue,
                               StringRef bugtype, StringRef verboseDesc,
                               StringRef shortDesc, StringRef category,
                               PathDiagnosticLocation LocationToUnique,
                               const Decl *DeclToUnique)
  : CheckName(CheckName),
    DeclWithIssue(declWithIssue),
    BugType(StripTrailingDots(bugtype)),
    VerboseDesc(StripTrailingDots(verboseDesc)),
    ShortDesc(StripTrailingDots(shortDesc)),
    Category(StripTrailingDots(category)),
    UniqueingLoc(LocationToUnique),
    UniqueingDecl(DeclToUnique),
    path(pathImpl) {}

static PathDiagnosticCallPiece *
getFirstStackedCallToHeaderFile(PathDiagnosticCallPiece *CP,
                                const SourceManager &SMgr) {
  SourceLocation CallLoc = CP->callEnter.asLocation();

  // If the call is within a macro, don't do anything (for now).
  if (CallLoc.isMacroID())
    return nullptr;

  assert(SMgr.isInMainFile(CallLoc) &&
         "The call piece should be in the main file.");

  // Check if CP represents a path through a function outside of the main file.
  if (!SMgr.isInMainFile(CP->callEnterWithin.asLocation()))
    return CP;

  const PathPieces &Path = CP->path;
  if (Path.empty())
    return nullptr;

  // Check if the last piece in the callee path is a call to a function outside
  // of the main file.
  if (PathDiagnosticCallPiece *CPInner =
      dyn_cast<PathDiagnosticCallPiece>(Path.back())) {
    return getFirstStackedCallToHeaderFile(CPInner, SMgr);
  }

  // Otherwise, the last piece is in the main file.
  return nullptr;
}

void PathDiagnostic::resetDiagnosticLocationToMainFile() {
  if (path.empty())
    return;

  PathDiagnosticPiece *LastP = path.back().get();
  assert(LastP);
  const SourceManager &SMgr = LastP->getLocation().getManager();

  // We only need to check if the report ends inside headers, if the last piece
  // is a call piece.
  if (PathDiagnosticCallPiece *CP = dyn_cast<PathDiagnosticCallPiece>(LastP)) {
    CP = getFirstStackedCallToHeaderFile(CP, SMgr);
    if (CP) {
      // Mark the piece.
       CP->setAsLastInMainSourceFile();

      // Update the path diagnostic message.
      const NamedDecl *ND = dyn_cast<NamedDecl>(CP->getCallee());
      if (ND) {
        SmallString<200> buf;
        llvm::raw_svector_ostream os(buf);
        os << " (within a call to '" << ND->getDeclName() << "')";
        appendToDesc(os.str());
      }

      // Reset the report containing declaration and location.
      DeclWithIssue = CP->getCaller();
      Loc = CP->getLocation();
      
      return;
    }
  }
}

void PathDiagnosticConsumer::anchor() { }

PathDiagnosticConsumer::~PathDiagnosticConsumer() {
  // Delete the contents of the FoldingSet if it isn't empty already.
  for (llvm::FoldingSet<PathDiagnostic>::iterator it =
       Diags.begin(), et = Diags.end() ; it != et ; ++it) {
    delete &*it;
  }
}

void PathDiagnosticConsumer::HandlePathDiagnostic(
    std::unique_ptr<PathDiagnostic> D) {
  if (!D || D->path.empty())
    return;
  
  // We need to flatten the locations (convert Stmt* to locations) because
  // the referenced statements may be freed by the time the diagnostics
  // are emitted.
  D->flattenLocations();

  // If the PathDiagnosticConsumer does not support diagnostics that
  // cross file boundaries, prune out such diagnostics now.
  if (!supportsCrossFileDiagnostics()) {
    // Verify that the entire path is from the same FileID.
    FileID FID;
    const SourceManager &SMgr = D->path.front()->getLocation().getManager();
    SmallVector<const PathPieces *, 5> WorkList;
    WorkList.push_back(&D->path);

    while (!WorkList.empty()) {
      const PathPieces &path = *WorkList.pop_back_val();

      for (PathPieces::const_iterator I = path.begin(), E = path.end(); I != E;
           ++I) {
        const PathDiagnosticPiece *piece = I->get();
        FullSourceLoc L = piece->getLocation().asLocation().getExpansionLoc();
      
        if (FID.isInvalid()) {
          FID = SMgr.getFileID(L);
        } else if (SMgr.getFileID(L) != FID)
          return; // FIXME: Emit a warning?
      
        // Check the source ranges.
        ArrayRef<SourceRange> Ranges = piece->getRanges();
        for (ArrayRef<SourceRange>::iterator I = Ranges.begin(),
                                             E = Ranges.end(); I != E; ++I) {
          SourceLocation L = SMgr.getExpansionLoc(I->getBegin());
          if (!L.isFileID() || SMgr.getFileID(L) != FID)
            return; // FIXME: Emit a warning?
          L = SMgr.getExpansionLoc(I->getEnd());
          if (!L.isFileID() || SMgr.getFileID(L) != FID)
            return; // FIXME: Emit a warning?
        }
        
        if (const PathDiagnosticCallPiece *call =
            dyn_cast<PathDiagnosticCallPiece>(piece)) {
          WorkList.push_back(&call->path);
        }
        else if (const PathDiagnosticMacroPiece *macro =
                 dyn_cast<PathDiagnosticMacroPiece>(piece)) {
          WorkList.push_back(&macro->subPieces);
        }
      }
    }
    
    if (FID.isInvalid())
      return; // FIXME: Emit a warning?
  }  

  // Profile the node to see if we already have something matching it
  llvm::FoldingSetNodeID profile;
  D->Profile(profile);
  void *InsertPos = nullptr;

  if (PathDiagnostic *orig = Diags.FindNodeOrInsertPos(profile, InsertPos)) {
    // Keep the PathDiagnostic with the shorter path.
    // Note, the enclosing routine is called in deterministic order, so the
    // results will be consistent between runs (no reason to break ties if the
    // size is the same).
    const unsigned orig_size = orig->full_size();
    const unsigned new_size = D->full_size();
    if (orig_size <= new_size)
      return;

    assert(orig != D.get());
    Diags.RemoveNode(orig);
    delete orig;
  }

  Diags.InsertNode(D.release());
}

static Optional<bool> comparePath(const PathPieces &X, const PathPieces &Y);
static Optional<bool>
compareControlFlow(const PathDiagnosticControlFlowPiece &X,
                   const PathDiagnosticControlFlowPiece &Y) {
  FullSourceLoc XSL = X.getStartLocation().asLocation();
  FullSourceLoc YSL = Y.getStartLocation().asLocation();
  if (XSL != YSL)
    return XSL.isBeforeInTranslationUnitThan(YSL);
  FullSourceLoc XEL = X.getEndLocation().asLocation();
  FullSourceLoc YEL = Y.getEndLocation().asLocation();
  if (XEL != YEL)
    return XEL.isBeforeInTranslationUnitThan(YEL);
  return None;
}

static Optional<bool> compareMacro(const PathDiagnosticMacroPiece &X,
                                   const PathDiagnosticMacroPiece &Y) {
  return comparePath(X.subPieces, Y.subPieces);
}

static Optional<bool> compareCall(const PathDiagnosticCallPiece &X,
                                  const PathDiagnosticCallPiece &Y) {
  FullSourceLoc X_CEL = X.callEnter.asLocation();
  FullSourceLoc Y_CEL = Y.callEnter.asLocation();
  if (X_CEL != Y_CEL)
    return X_CEL.isBeforeInTranslationUnitThan(Y_CEL);
  FullSourceLoc X_CEWL = X.callEnterWithin.asLocation();
  FullSourceLoc Y_CEWL = Y.callEnterWithin.asLocation();
  if (X_CEWL != Y_CEWL)
    return X_CEWL.isBeforeInTranslationUnitThan(Y_CEWL);
  FullSourceLoc X_CRL = X.callReturn.asLocation();
  FullSourceLoc Y_CRL = Y.callReturn.asLocation();
  if (X_CRL != Y_CRL)
    return X_CRL.isBeforeInTranslationUnitThan(Y_CRL);
  return comparePath(X.path, Y.path);
}

static Optional<bool> comparePiece(const PathDiagnosticPiece &X,
                                   const PathDiagnosticPiece &Y) {
  if (X.getKind() != Y.getKind())
    return X.getKind() < Y.getKind();
  
  FullSourceLoc XL = X.getLocation().asLocation();
  FullSourceLoc YL = Y.getLocation().asLocation();
  if (XL != YL)
    return XL.isBeforeInTranslationUnitThan(YL);

  if (X.getString() != Y.getString())
    return X.getString() < Y.getString();

  if (X.getRanges().size() != Y.getRanges().size())
    return X.getRanges().size() < Y.getRanges().size();

  const SourceManager &SM = XL.getManager();
  
  for (unsigned i = 0, n = X.getRanges().size(); i < n; ++i) {
    SourceRange XR = X.getRanges()[i];
    SourceRange YR = Y.getRanges()[i];
    if (XR != YR) {
      if (XR.getBegin() != YR.getBegin())
        return SM.isBeforeInTranslationUnit(XR.getBegin(), YR.getBegin());
      return SM.isBeforeInTranslationUnit(XR.getEnd(), YR.getEnd());
    }
  }
  
  switch (X.getKind()) {
    case clang::ento::PathDiagnosticPiece::ControlFlow:
      return compareControlFlow(cast<PathDiagnosticControlFlowPiece>(X),
                                cast<PathDiagnosticControlFlowPiece>(Y));
    case clang::ento::PathDiagnosticPiece::Event:
      return None;
    case clang::ento::PathDiagnosticPiece::Macro:
      return compareMacro(cast<PathDiagnosticMacroPiece>(X),
                          cast<PathDiagnosticMacroPiece>(Y));
    case clang::ento::PathDiagnosticPiece::Call:
      return compareCall(cast<PathDiagnosticCallPiece>(X),
                         cast<PathDiagnosticCallPiece>(Y));
  }
  llvm_unreachable("all cases handled");
}

static Optional<bool> comparePath(const PathPieces &X, const PathPieces &Y) {
  if (X.size() != Y.size())
    return X.size() < Y.size();

  PathPieces::const_iterator X_I = X.begin(), X_end = X.end();
  PathPieces::const_iterator Y_I = Y.begin(), Y_end = Y.end();

  for ( ; X_I != X_end && Y_I != Y_end; ++X_I, ++Y_I) {
    Optional<bool> b = comparePiece(**X_I, **Y_I);
    if (b.hasValue())
      return b.getValue();
  }

  return None;
}

static bool compare(const PathDiagnostic &X, const PathDiagnostic &Y) {
  FullSourceLoc XL = X.getLocation().asLocation();
  FullSourceLoc YL = Y.getLocation().asLocation();
  if (XL != YL)
    return XL.isBeforeInTranslationUnitThan(YL);
  if (X.getBugType() != Y.getBugType())
    return X.getBugType() < Y.getBugType();
  if (X.getCategory() != Y.getCategory())
    return X.getCategory() < Y.getCategory();
  if (X.getVerboseDescription() != Y.getVerboseDescription())
    return X.getVerboseDescription() < Y.getVerboseDescription();
  if (X.getShortDescription() != Y.getShortDescription())
    return X.getShortDescription() < Y.getShortDescription();
  if (X.getDeclWithIssue() != Y.getDeclWithIssue()) {
    const Decl *XD = X.getDeclWithIssue();
    if (!XD)
      return true;
    const Decl *YD = Y.getDeclWithIssue();
    if (!YD)
      return false;
    SourceLocation XDL = XD->getLocation();
    SourceLocation YDL = YD->getLocation();
    if (XDL != YDL) {
      const SourceManager &SM = XL.getManager();
      return SM.isBeforeInTranslationUnit(XDL, YDL);
    }
  }
  PathDiagnostic::meta_iterator XI = X.meta_begin(), XE = X.meta_end();
  PathDiagnostic::meta_iterator YI = Y.meta_begin(), YE = Y.meta_end();
  if (XE - XI != YE - YI)
    return (XE - XI) < (YE - YI);
  for ( ; XI != XE ; ++XI, ++YI) {
    if (*XI != *YI)
      return (*XI) < (*YI);
  }
  Optional<bool> b = comparePath(X.path, Y.path);
  assert(b.hasValue());
  return b.getValue();
}

void PathDiagnosticConsumer::FlushDiagnostics(
                                     PathDiagnosticConsumer::FilesMade *Files) {
  if (flushed)
    return;
  
  flushed = true;
  
  std::vector<const PathDiagnostic *> BatchDiags;
  for (llvm::FoldingSet<PathDiagnostic>::iterator it = Diags.begin(),
       et = Diags.end(); it != et; ++it) {
    const PathDiagnostic *D = &*it;
    BatchDiags.push_back(D);
  }

  // Sort the diagnostics so that they are always emitted in a deterministic
  // order.
  int (*Comp)(const PathDiagnostic *const *, const PathDiagnostic *const *) =
      [](const PathDiagnostic *const *X, const PathDiagnostic *const *Y) {
        assert(*X != *Y && "PathDiagnostics not uniqued!");
        if (compare(**X, **Y))
          return -1;
        assert(compare(**Y, **X) && "Not a total order!");
        return 1;
      };
  array_pod_sort(BatchDiags.begin(), BatchDiags.end(), Comp);

  FlushDiagnosticsImpl(BatchDiags, Files);

  // Delete the flushed diagnostics.
  for (std::vector<const PathDiagnostic *>::iterator it = BatchDiags.begin(),
       et = BatchDiags.end(); it != et; ++it) {
    const PathDiagnostic *D = *it;
    delete D;
  }
  
  // Clear out the FoldingSet.
  Diags.clear();
}

PathDiagnosticConsumer::FilesMade::~FilesMade() {
  for (PDFileEntry &Entry : Set)
    Entry.~PDFileEntry();
}

void PathDiagnosticConsumer::FilesMade::addDiagnostic(const PathDiagnostic &PD,
                                                      StringRef ConsumerName,
                                                      StringRef FileName) {
  llvm::FoldingSetNodeID NodeID;
  NodeID.Add(PD);
  void *InsertPos;
  PDFileEntry *Entry = Set.FindNodeOrInsertPos(NodeID, InsertPos);
  if (!Entry) {
    Entry = Alloc.Allocate<PDFileEntry>();
    Entry = new (Entry) PDFileEntry(NodeID);
    Set.InsertNode(Entry, InsertPos);
  }
  
  // Allocate persistent storage for the file name.
  char *FileName_cstr = (char*) Alloc.Allocate(FileName.size(), 1);
  memcpy(FileName_cstr, FileName.data(), FileName.size());

  Entry->files.push_back(std::make_pair(ConsumerName,
                                        StringRef(FileName_cstr,
                                                  FileName.size())));
}

PathDiagnosticConsumer::PDFileEntry::ConsumerFiles *
PathDiagnosticConsumer::FilesMade::getFiles(const PathDiagnostic &PD) {
  llvm::FoldingSetNodeID NodeID;
  NodeID.Add(PD);
  void *InsertPos;
  PDFileEntry *Entry = Set.FindNodeOrInsertPos(NodeID, InsertPos);
  if (!Entry)
    return nullptr;
  return &Entry->files;
}

//===----------------------------------------------------------------------===//
// PathDiagnosticLocation methods.
//===----------------------------------------------------------------------===//

static SourceLocation getValidSourceLocation(const Stmt* S,
                                             LocationOrAnalysisDeclContext LAC,
                                             bool UseEnd = false) {
  SourceLocation L = UseEnd ? S->getLocEnd() : S->getLocStart();
  assert(!LAC.isNull() && "A valid LocationContext or AnalysisDeclContext should "
                          "be passed to PathDiagnosticLocation upon creation.");

  // S might be a temporary statement that does not have a location in the
  // source code, so find an enclosing statement and use its location.
  if (!L.isValid()) {

    AnalysisDeclContext *ADC;
    if (LAC.is<const LocationContext*>())
      ADC = LAC.get<const LocationContext*>()->getAnalysisDeclContext();
    else
      ADC = LAC.get<AnalysisDeclContext*>();

    ParentMap &PM = ADC->getParentMap();

    const Stmt *Parent = S;
    do {
      Parent = PM.getParent(Parent);

      // In rare cases, we have implicit top-level expressions,
      // such as arguments for implicit member initializers.
      // In this case, fall back to the start of the body (even if we were
      // asked for the statement end location).
      if (!Parent) {
        const Stmt *Body = ADC->getBody();
        if (Body)
          L = Body->getLocStart();
        else
          L = ADC->getDecl()->getLocEnd();
        break;
      }

      L = UseEnd ? Parent->getLocEnd() : Parent->getLocStart();
    } while (!L.isValid());
  }

  return L;
}

static PathDiagnosticLocation
getLocationForCaller(const StackFrameContext *SFC,
                     const LocationContext *CallerCtx,
                     const SourceManager &SM) {
  const CFGBlock &Block = *SFC->getCallSiteBlock();
  CFGElement Source = Block[SFC->getIndex()];

  switch (Source.getKind()) {
  case CFGElement::Statement:
    return PathDiagnosticLocation(Source.castAs<CFGStmt>().getStmt(),
                                  SM, CallerCtx);
  case CFGElement::Initializer: {
    const CFGInitializer &Init = Source.castAs<CFGInitializer>();
    return PathDiagnosticLocation(Init.getInitializer()->getInit(),
                                  SM, CallerCtx);
  }
  case CFGElement::AutomaticObjectDtor: {
    const CFGAutomaticObjDtor &Dtor = Source.castAs<CFGAutomaticObjDtor>();
    return PathDiagnosticLocation::createEnd(Dtor.getTriggerStmt(),
                                             SM, CallerCtx);
  }
  case CFGElement::DeleteDtor: {
    const CFGDeleteDtor &Dtor = Source.castAs<CFGDeleteDtor>();
    return PathDiagnosticLocation(Dtor.getDeleteExpr(), SM, CallerCtx);
  }
  case CFGElement::BaseDtor:
  case CFGElement::MemberDtor: {
    const AnalysisDeclContext *CallerInfo = CallerCtx->getAnalysisDeclContext();
    if (const Stmt *CallerBody = CallerInfo->getBody())
      return PathDiagnosticLocation::createEnd(CallerBody, SM, CallerCtx);
    return PathDiagnosticLocation::create(CallerInfo->getDecl(), SM);
  }
  case CFGElement::TemporaryDtor:
  case CFGElement::NewAllocator:
    llvm_unreachable("not yet implemented!");
  }

  llvm_unreachable("Unknown CFGElement kind");
}


PathDiagnosticLocation
  PathDiagnosticLocation::createBegin(const Decl *D,
                                      const SourceManager &SM) {
  return PathDiagnosticLocation(D->getLocStart(), SM, SingleLocK);
}

PathDiagnosticLocation
  PathDiagnosticLocation::createBegin(const Stmt *S,
                                      const SourceManager &SM,
                                      LocationOrAnalysisDeclContext LAC) {
  return PathDiagnosticLocation(getValidSourceLocation(S, LAC),
                                SM, SingleLocK);
}


PathDiagnosticLocation
PathDiagnosticLocation::createEnd(const Stmt *S,
                                  const SourceManager &SM,
                                  LocationOrAnalysisDeclContext LAC) {
  if (const CompoundStmt *CS = dyn_cast<CompoundStmt>(S))
    return createEndBrace(CS, SM);
  return PathDiagnosticLocation(getValidSourceLocation(S, LAC, /*End=*/true),
                                SM, SingleLocK);
}

PathDiagnosticLocation
  PathDiagnosticLocation::createOperatorLoc(const BinaryOperator *BO,
                                            const SourceManager &SM) {
  return PathDiagnosticLocation(BO->getOperatorLoc(), SM, SingleLocK);
}

PathDiagnosticLocation
  PathDiagnosticLocation::createConditionalColonLoc(
                                            const ConditionalOperator *CO,
                                            const SourceManager &SM) {
  return PathDiagnosticLocation(CO->getColonLoc(), SM, SingleLocK);
}


PathDiagnosticLocation
  PathDiagnosticLocation::createMemberLoc(const MemberExpr *ME,
                                          const SourceManager &SM) {
  return PathDiagnosticLocation(ME->getMemberLoc(), SM, SingleLocK);
}

PathDiagnosticLocation
  PathDiagnosticLocation::createBeginBrace(const CompoundStmt *CS,
                                           const SourceManager &SM) {
  SourceLocation L = CS->getLBracLoc();
  return PathDiagnosticLocation(L, SM, SingleLocK);
}

PathDiagnosticLocation
  PathDiagnosticLocation::createEndBrace(const CompoundStmt *CS,
                                         const SourceManager &SM) {
  SourceLocation L = CS->getRBracLoc();
  return PathDiagnosticLocation(L, SM, SingleLocK);
}

PathDiagnosticLocation
  PathDiagnosticLocation::createDeclBegin(const LocationContext *LC,
                                          const SourceManager &SM) {
  // FIXME: Should handle CXXTryStmt if analyser starts supporting C++.
  if (const CompoundStmt *CS =
        dyn_cast_or_null<CompoundStmt>(LC->getDecl()->getBody()))
    if (!CS->body_empty()) {
      SourceLocation Loc = (*CS->body_begin())->getLocStart();
      return PathDiagnosticLocation(Loc, SM, SingleLocK);
    }

  return PathDiagnosticLocation();
}

PathDiagnosticLocation
  PathDiagnosticLocation::createDeclEnd(const LocationContext *LC,
                                        const SourceManager &SM) {
  SourceLocation L = LC->getDecl()->getBodyRBrace();
  return PathDiagnosticLocation(L, SM, SingleLocK);
}

PathDiagnosticLocation
  PathDiagnosticLocation::create(const ProgramPoint& P,
                                 const SourceManager &SMng) {

  const Stmt* S = nullptr;
  if (Optional<BlockEdge> BE = P.getAs<BlockEdge>()) {
    const CFGBlock *BSrc = BE->getSrc();
    S = BSrc->getTerminatorCondition();
  } else if (Optional<StmtPoint> SP = P.getAs<StmtPoint>()) {
    S = SP->getStmt();
    if (P.getAs<PostStmtPurgeDeadSymbols>())
      return PathDiagnosticLocation::createEnd(S, SMng, P.getLocationContext());
  } else if (Optional<PostInitializer> PIP = P.getAs<PostInitializer>()) {
    return PathDiagnosticLocation(PIP->getInitializer()->getSourceLocation(),
                                  SMng);
  } else if (Optional<PostImplicitCall> PIE = P.getAs<PostImplicitCall>()) {
    return PathDiagnosticLocation(PIE->getLocation(), SMng);
  } else if (Optional<CallEnter> CE = P.getAs<CallEnter>()) {
    return getLocationForCaller(CE->getCalleeContext(),
                                CE->getLocationContext(),
                                SMng);
  } else if (Optional<CallExitEnd> CEE = P.getAs<CallExitEnd>()) {
    return getLocationForCaller(CEE->getCalleeContext(),
                                CEE->getLocationContext(),
                                SMng);
  } else {
    llvm_unreachable("Unexpected ProgramPoint");
  }

  return PathDiagnosticLocation(S, SMng, P.getLocationContext());
}

const Stmt *PathDiagnosticLocation::getStmt(const ExplodedNode *N) {
  ProgramPoint P = N->getLocation();
  if (Optional<StmtPoint> SP = P.getAs<StmtPoint>())
    return SP->getStmt();
  if (Optional<BlockEdge> BE = P.getAs<BlockEdge>())
    return BE->getSrc()->getTerminator();
  if (Optional<CallEnter> CE = P.getAs<CallEnter>())
    return CE->getCallExpr();
  if (Optional<CallExitEnd> CEE = P.getAs<CallExitEnd>())
    return CEE->getCalleeContext()->getCallSite();
  if (Optional<PostInitializer> PIPP = P.getAs<PostInitializer>())
    return PIPP->getInitializer()->getInit();

  return nullptr;
}

const Stmt *PathDiagnosticLocation::getNextStmt(const ExplodedNode *N) {
  for (N = N->getFirstSucc(); N; N = N->getFirstSucc()) {
    if (const Stmt *S = getStmt(N)) {
      // Check if the statement is '?' or '&&'/'||'.  These are "merges",
      // not actual statement points.
      switch (S->getStmtClass()) {
        case Stmt::ChooseExprClass:
        case Stmt::BinaryConditionalOperatorClass:
        case Stmt::ConditionalOperatorClass:
          continue;
        case Stmt::BinaryOperatorClass: {
          BinaryOperatorKind Op = cast<BinaryOperator>(S)->getOpcode();
          if (Op == BO_LAnd || Op == BO_LOr)
            continue;
          break;
        }
        default:
          break;
      }
      // We found the statement, so return it.
      return S;
    }
  }

  return nullptr;
}

PathDiagnosticLocation
  PathDiagnosticLocation::createEndOfPath(const ExplodedNode *N,
                                          const SourceManager &SM) {
  assert(N && "Cannot create a location with a null node.");
  const Stmt *S = getStmt(N);

  if (!S) {
    // If this is an implicit call, return the implicit call point location.
    if (Optional<PreImplicitCall> PIE = N->getLocationAs<PreImplicitCall>())
      return PathDiagnosticLocation(PIE->getLocation(), SM);
    S = getNextStmt(N);
  }

  if (S) {
    ProgramPoint P = N->getLocation();
    const LocationContext *LC = N->getLocationContext();

    // For member expressions, return the location of the '.' or '->'.
    if (const MemberExpr *ME = dyn_cast<MemberExpr>(S))
      return PathDiagnosticLocation::createMemberLoc(ME, SM);

    // For binary operators, return the location of the operator.
    if (const BinaryOperator *B = dyn_cast<BinaryOperator>(S))
      return PathDiagnosticLocation::createOperatorLoc(B, SM);

    if (P.getAs<PostStmtPurgeDeadSymbols>())
      return PathDiagnosticLocation::createEnd(S, SM, LC);

    if (S->getLocStart().isValid())
      return PathDiagnosticLocation(S, SM, LC);
    return PathDiagnosticLocation(getValidSourceLocation(S, LC), SM);
  }

  return createDeclEnd(N->getLocationContext(), SM);
}

PathDiagnosticLocation PathDiagnosticLocation::createSingleLocation(
                                           const PathDiagnosticLocation &PDL) {
  FullSourceLoc L = PDL.asLocation();
  return PathDiagnosticLocation(L, L.getManager(), SingleLocK);
}

FullSourceLoc
  PathDiagnosticLocation::genLocation(SourceLocation L,
                                      LocationOrAnalysisDeclContext LAC) const {
  assert(isValid());
  // Note that we want a 'switch' here so that the compiler can warn us in
  // case we add more cases.
  switch (K) {
    case SingleLocK:
    case RangeK:
      break;
    case StmtK:
      // Defensive checking.
      if (!S)
        break;
      return FullSourceLoc(getValidSourceLocation(S, LAC),
                           const_cast<SourceManager&>(*SM));
    case DeclK:
      // Defensive checking.
      if (!D)
        break;
      return FullSourceLoc(D->getLocation(), const_cast<SourceManager&>(*SM));
  }

  return FullSourceLoc(L, const_cast<SourceManager&>(*SM));
}

PathDiagnosticRange
  PathDiagnosticLocation::genRange(LocationOrAnalysisDeclContext LAC) const {
  assert(isValid());
  // Note that we want a 'switch' here so that the compiler can warn us in
  // case we add more cases.
  switch (K) {
    case SingleLocK:
      return PathDiagnosticRange(SourceRange(Loc,Loc), true);
    case RangeK:
      break;
    case StmtK: {
      const Stmt *S = asStmt();
      switch (S->getStmtClass()) {
        default:
          break;
        case Stmt::DeclStmtClass: {
          const DeclStmt *DS = cast<DeclStmt>(S);
          if (DS->isSingleDecl()) {
            // Should always be the case, but we'll be defensive.
            return SourceRange(DS->getLocStart(),
                               DS->getSingleDecl()->getLocation());
          }
          break;
        }
          // FIXME: Provide better range information for different
          //  terminators.
        case Stmt::IfStmtClass:
        case Stmt::WhileStmtClass:
        case Stmt::DoStmtClass:
        case Stmt::ForStmtClass:
        case Stmt::ChooseExprClass:
        case Stmt::IndirectGotoStmtClass:
        case Stmt::SwitchStmtClass:
        case Stmt::BinaryConditionalOperatorClass:
        case Stmt::ConditionalOperatorClass:
        case Stmt::ObjCForCollectionStmtClass: {
          SourceLocation L = getValidSourceLocation(S, LAC);
          return SourceRange(L, L);
        }
      }
      SourceRange R = S->getSourceRange();
      if (R.isValid())
        return R;
      break;  
    }
    case DeclK:
      if (const ObjCMethodDecl *MD = dyn_cast<ObjCMethodDecl>(D))
        return MD->getSourceRange();
      if (const FunctionDecl *FD = dyn_cast<FunctionDecl>(D)) {
        if (Stmt *Body = FD->getBody())
          return Body->getSourceRange();
      }
      else {
        SourceLocation L = D->getLocation();
        return PathDiagnosticRange(SourceRange(L, L), true);
      }
  }

  return SourceRange(Loc,Loc);
}

void PathDiagnosticLocation::flatten() {
  if (K == StmtK) {
    K = RangeK;
    S = nullptr;
    D = nullptr;
  }
  else if (K == DeclK) {
    K = SingleLocK;
    S = nullptr;
    D = nullptr;
  }
}

//===----------------------------------------------------------------------===//
// Manipulation of PathDiagnosticCallPieces.
//===----------------------------------------------------------------------===//

PathDiagnosticCallPiece *
PathDiagnosticCallPiece::construct(const ExplodedNode *N,
                                   const CallExitEnd &CE,
                                   const SourceManager &SM) {
  const Decl *caller = CE.getLocationContext()->getDecl();
  PathDiagnosticLocation pos = getLocationForCaller(CE.getCalleeContext(),
                                                    CE.getLocationContext(),
                                                    SM);
  return new PathDiagnosticCallPiece(caller, pos);
}

PathDiagnosticCallPiece *
PathDiagnosticCallPiece::construct(PathPieces &path,
                                   const Decl *caller) {
  PathDiagnosticCallPiece *C = new PathDiagnosticCallPiece(path, caller);
  path.clear();
  path.push_front(C);
  return C;
}

void PathDiagnosticCallPiece::setCallee(const CallEnter &CE,
                                        const SourceManager &SM) {
  const StackFrameContext *CalleeCtx = CE.getCalleeContext();
  Callee = CalleeCtx->getDecl();

  callEnterWithin = PathDiagnosticLocation::createBegin(Callee, SM);
  callEnter = getLocationForCaller(CalleeCtx, CE.getLocationContext(), SM);
}

static inline void describeClass(raw_ostream &Out, const CXXRecordDecl *D,
                                 StringRef Prefix = StringRef()) {
  if (!D->getIdentifier())
    return;
  Out << Prefix << '\'' << *D << '\'';
}

static bool describeCodeDecl(raw_ostream &Out, const Decl *D,
                             bool ExtendedDescription,
                             StringRef Prefix = StringRef()) {
  if (!D)
    return false;

  if (isa<BlockDecl>(D)) {
    if (ExtendedDescription)
      Out << Prefix << "anonymous block";
    return ExtendedDescription;
  }

  if (const CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(D)) {
    Out << Prefix;
    if (ExtendedDescription && !MD->isUserProvided()) {
      if (MD->isExplicitlyDefaulted())
        Out << "defaulted ";
      else
        Out << "implicit ";
    }

    if (const CXXConstructorDecl *CD = dyn_cast<CXXConstructorDecl>(MD)) {
      if (CD->isDefaultConstructor())
        Out << "default ";
      else if (CD->isCopyConstructor())
        Out << "copy ";
      else if (CD->isMoveConstructor())
        Out << "move ";

      Out << "constructor";
      describeClass(Out, MD->getParent(), " for ");
      
    } else if (isa<CXXDestructorDecl>(MD)) {
      if (!MD->isUserProvided()) {
        Out << "destructor";
        describeClass(Out, MD->getParent(), " for ");
      } else {
        // Use ~Foo for explicitly-written destructors.
        Out << "'" << *MD << "'";
      }

    } else if (MD->isCopyAssignmentOperator()) {
        Out << "copy assignment operator";
        describeClass(Out, MD->getParent(), " for ");

    } else if (MD->isMoveAssignmentOperator()) {
        Out << "move assignment operator";
        describeClass(Out, MD->getParent(), " for ");

    } else {
      if (MD->getParent()->getIdentifier())
        Out << "'" << *MD->getParent() << "::" << *MD << "'";
      else
        Out << "'" << *MD << "'";
    }

    return true;
  }

  Out << Prefix << '\'' << cast<NamedDecl>(*D) << '\'';
  return true;
}

IntrusiveRefCntPtr<PathDiagnosticEventPiece>
PathDiagnosticCallPiece::getCallEnterEvent() const {
  if (!Callee)
    return nullptr;

  SmallString<256> buf;
  llvm::raw_svector_ostream Out(buf);

  Out << "Calling ";
  describeCodeDecl(Out, Callee, /*ExtendedDescription=*/true);

  assert(callEnter.asLocation().isValid());
  return new PathDiagnosticEventPiece(callEnter, Out.str());
}

IntrusiveRefCntPtr<PathDiagnosticEventPiece>
PathDiagnosticCallPiece::getCallEnterWithinCallerEvent() const {
  if (!callEnterWithin.asLocation().isValid())
    return nullptr;
  if (Callee->isImplicit() || !Callee->hasBody())
    return nullptr;
  if (const CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(Callee))
    if (MD->isDefaulted())
      return nullptr;

  SmallString<256> buf;
  llvm::raw_svector_ostream Out(buf);

  Out << "Entered call";
  describeCodeDecl(Out, Caller, /*ExtendedDescription=*/false, " from ");

  return new PathDiagnosticEventPiece(callEnterWithin, Out.str());
}

IntrusiveRefCntPtr<PathDiagnosticEventPiece>
PathDiagnosticCallPiece::getCallExitEvent() const {
  if (NoExit)
    return nullptr;

  SmallString<256> buf;
  llvm::raw_svector_ostream Out(buf);

  if (!CallStackMessage.empty()) {
    Out << CallStackMessage;
  } else {
    bool DidDescribe = describeCodeDecl(Out, Callee,
                                        /*ExtendedDescription=*/false,
                                        "Returning from ");
    if (!DidDescribe)
      Out << "Returning to caller";
  }

  assert(callReturn.asLocation().isValid());
  return new PathDiagnosticEventPiece(callReturn, Out.str());
}

static void compute_path_size(const PathPieces &pieces, unsigned &size) {
  for (PathPieces::const_iterator it = pieces.begin(),
                                  et = pieces.end(); it != et; ++it) {
    const PathDiagnosticPiece *piece = it->get();
    if (const PathDiagnosticCallPiece *cp = 
        dyn_cast<PathDiagnosticCallPiece>(piece)) {
      compute_path_size(cp->path, size);
    }
    else
      ++size;
  }
}

unsigned PathDiagnostic::full_size() {
  unsigned size = 0;
  compute_path_size(path, size);
  return size;
}

//===----------------------------------------------------------------------===//
// FoldingSet profiling methods.
//===----------------------------------------------------------------------===//

void PathDiagnosticLocation::Profile(llvm::FoldingSetNodeID &ID) const {
  ID.AddInteger(Range.getBegin().getRawEncoding());
  ID.AddInteger(Range.getEnd().getRawEncoding());
  ID.AddInteger(Loc.getRawEncoding());
  return;
}

void PathDiagnosticPiece::Profile(llvm::FoldingSetNodeID &ID) const {
  ID.AddInteger((unsigned) getKind());
  ID.AddString(str);
  // FIXME: Add profiling support for code hints.
  ID.AddInteger((unsigned) getDisplayHint());
  ArrayRef<SourceRange> Ranges = getRanges();
  for (ArrayRef<SourceRange>::iterator I = Ranges.begin(), E = Ranges.end();
                                        I != E; ++I) {
    ID.AddInteger(I->getBegin().getRawEncoding());
    ID.AddInteger(I->getEnd().getRawEncoding());
  }  
}

void PathDiagnosticCallPiece::Profile(llvm::FoldingSetNodeID &ID) const {
  PathDiagnosticPiece::Profile(ID);
  for (PathPieces::const_iterator it = path.begin(), 
       et = path.end(); it != et; ++it) {
    ID.Add(**it);
  }
}

void PathDiagnosticSpotPiece::Profile(llvm::FoldingSetNodeID &ID) const {
  PathDiagnosticPiece::Profile(ID);
  ID.Add(Pos);
}

void PathDiagnosticControlFlowPiece::Profile(llvm::FoldingSetNodeID &ID) const {
  PathDiagnosticPiece::Profile(ID);
  for (const_iterator I = begin(), E = end(); I != E; ++I)
    ID.Add(*I);
}

void PathDiagnosticMacroPiece::Profile(llvm::FoldingSetNodeID &ID) const {
  PathDiagnosticSpotPiece::Profile(ID);
  for (PathPieces::const_iterator I = subPieces.begin(), E = subPieces.end();
       I != E; ++I)
    ID.Add(**I);
}

void PathDiagnostic::Profile(llvm::FoldingSetNodeID &ID) const {
  ID.Add(getLocation());
  ID.AddString(BugType);
  ID.AddString(VerboseDesc);
  ID.AddString(Category);
}

void PathDiagnostic::FullProfile(llvm::FoldingSetNodeID &ID) const {
  Profile(ID);
  for (PathPieces::const_iterator I = path.begin(), E = path.end(); I != E; ++I)
    ID.Add(**I);
  for (meta_iterator I = meta_begin(), E = meta_end(); I != E; ++I)
    ID.AddString(*I);
}

StackHintGenerator::~StackHintGenerator() {}

std::string StackHintGeneratorForSymbol::getMessage(const ExplodedNode *N){
  ProgramPoint P = N->getLocation();
  CallExitEnd CExit = P.castAs<CallExitEnd>();

  // FIXME: Use CallEvent to abstract this over all calls.
  const Stmt *CallSite = CExit.getCalleeContext()->getCallSite();
  const CallExpr *CE = dyn_cast_or_null<CallExpr>(CallSite);
  if (!CE)
    return "";

  if (!N)
    return getMessageForSymbolNotFound();

  // Check if one of the parameters are set to the interesting symbol.
  ProgramStateRef State = N->getState();
  const LocationContext *LCtx = N->getLocationContext();
  unsigned ArgIndex = 0;
  for (CallExpr::const_arg_iterator I = CE->arg_begin(),
                                    E = CE->arg_end(); I != E; ++I, ++ArgIndex){
    SVal SV = State->getSVal(*I, LCtx);

    // Check if the variable corresponding to the symbol is passed by value.
    SymbolRef AS = SV.getAsLocSymbol();
    if (AS == Sym) {
      return getMessageForArg(*I, ArgIndex);
    }

    // Check if the parameter is a pointer to the symbol.
    if (Optional<loc::MemRegionVal> Reg = SV.getAs<loc::MemRegionVal>()) {
      SVal PSV = State->getSVal(Reg->getRegion());
      SymbolRef AS = PSV.getAsLocSymbol();
      if (AS == Sym) {
        return getMessageForArg(*I, ArgIndex);
      }
    }
  }

  // Check if we are returning the interesting symbol.
  SVal SV = State->getSVal(CE, LCtx);
  SymbolRef RetSym = SV.getAsLocSymbol();
  if (RetSym == Sym) {
    return getMessageForReturn(CE);
  }

  return getMessageForSymbolNotFound();
}

std::string StackHintGeneratorForSymbol::getMessageForArg(const Expr *ArgE,
                                                          unsigned ArgIndex) {
  // Printed parameters start at 1, not 0.
  ++ArgIndex;

  SmallString<200> buf;
  llvm::raw_svector_ostream os(buf);

  os << Msg << " via " << ArgIndex << llvm::getOrdinalSuffix(ArgIndex)
     << " parameter";

  return os.str();
}
