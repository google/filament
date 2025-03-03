//===--- Sema.h - Semantic Analysis & AST Building --------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the Sema class, which performs semantic analysis and
// builds ASTs.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_SEMA_SEMA_H
#define LLVM_CLANG_SEMA_SEMA_H

#include "clang/AST/Attr.h"
#include "clang/AST/DeclarationName.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprObjC.h"
#include "clang/AST/ExternalASTSource.h"
#include "clang/AST/MangleNumberingContext.h"
#include "clang/AST/NSAPI.h"
#include "clang/AST/PrettyPrinter.h"
#include "clang/AST/TypeLoc.h"
#include "clang/Basic/ExpressionTraits.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/Module.h"
#include "clang/Basic/OpenMPKinds.h"
#include "clang/Basic/Specifiers.h"
#include "clang/Basic/TemplateKinds.h"
#include "clang/Basic/TypeTraits.h"
#include "clang/Sema/AnalysisBasedWarnings.h"
#include "clang/Sema/DeclSpec.h"
#include "clang/Sema/ExternalSemaSource.h"
#include "clang/Sema/IdentifierResolver.h"
#include "clang/Sema/LocInfoType.h"
#include "clang/Sema/ObjCMethodList.h"
#include "clang/Sema/Ownership.h"
#include "clang/Sema/Scope.h"
#include "clang/Sema/ScopeInfo.h"
#include "clang/Sema/TypoCorrection.h"
#include "clang/Sema/Weak.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/TinyPtrVector.h"
#include <deque>
#include <memory>
#include <string>
#include <vector>

// HLSL Change Starts

#include "llvm/Support/OacrIgnoreCond.h" // HLSL Change - all sema use is heavily language-dependant

namespace hlsl {
  struct UnusualAnnotation;
  class ShaderModel;
}
// HLSL Change Ends

namespace llvm {
  class APSInt;
  template <typename ValueT> struct DenseMapInfo;
  template <typename ValueT, typename ValueInfoT> class DenseSet;
  class SmallBitVector;
  class InlineAsmIdentifierInfo;
}

namespace clang {
  class ADLResult;
  class ASTConsumer;
  class ASTContext;
  class ASTMutationListener;
  class ASTReader;
  class ASTWriter;
  class ArrayType;
  class AttributeList;
  class BlockDecl;
  class CapturedDecl;
  class CXXBasePath;
  class CXXBasePaths;
  class CXXBindTemporaryExpr;
  typedef SmallVector<CXXBaseSpecifier*, 4> CXXCastPath;
  class CXXConstructorDecl;
  class CXXConversionDecl;
  class CXXDeleteExpr;
  class CXXDestructorDecl;
  class CXXFieldCollector;
  class CXXMemberCallExpr;
  class CXXMethodDecl;
  class CXXScopeSpec;
  class CXXTemporary;
  class CXXTryStmt;
  class CallExpr;
  class ClassTemplateDecl;
  class ClassTemplatePartialSpecializationDecl;
  class ClassTemplateSpecializationDecl;
  class VarTemplatePartialSpecializationDecl;
  class CodeCompleteConsumer;
  class CodeCompletionAllocator;
  class CodeCompletionTUInfo;
  class CodeCompletionResult;
  class Decl;
  class DeclAccessPair;
  class DeclContext;
  class DeclRefExpr;
  class DeclaratorDecl;
  class DeducedTemplateArgument;
  class DependentDiagnostic;
  class DesignatedInitExpr;
  class Designation;
  class EnableIfAttr;
  class EnumConstantDecl;
  class Expr;
  class ExtVectorType;
  class ExternalSemaSource;
  class FormatAttr;
  class FriendDecl;
  class FunctionDecl;
  class FunctionProtoType;
  class FunctionTemplateDecl;
  class ImplicitConversionSequence;
  class InitListExpr;
  class InitializationKind;
  class InitializationSequence;
  class InitializedEntity;
  class IntegerLiteral;
  class LabelStmt;
  class LambdaExpr;
  class LangOptions;
  class LocalInstantiationScope;
  class LookupResult;
  class MacroInfo;
  typedef ArrayRef<std::pair<IdentifierInfo *, SourceLocation>> ModuleIdPath;
  class ModuleLoader;
  class MultiLevelTemplateArgumentList;
  class NamedDecl;
  class ObjCCategoryDecl;
  class ObjCCategoryImplDecl;
  class ObjCCompatibleAliasDecl;
  class ObjCContainerDecl;
  class ObjCImplDecl;
  class ObjCImplementationDecl;
  class ObjCInterfaceDecl;
  class ObjCIvarDecl;
  template <class T> class ObjCList;
  class ObjCMessageExpr;
  class ObjCMethodDecl;
  class ObjCPropertyDecl;
  class ObjCProtocolDecl;
  class OMPThreadPrivateDecl;
  class OMPClause;
  class OverloadCandidateSet;
  class OverloadExpr;
  class ParenListExpr;
  class ParmVarDecl;
  class Preprocessor;
  class PseudoDestructorTypeStorage;
  class PseudoObjectExpr;
  class QualType;
  class StandardConversionSequence;
  class Stmt;
  class StringLiteral;
  class SwitchStmt;
  class TemplateArgument;
  class TemplateArgumentList;
  class TemplateArgumentLoc;
  class TemplateDecl;
  class TemplateParameterList;
  class TemplatePartialOrderingContext;
  class TemplateTemplateParmDecl;
  class Token;
  class TypeAliasDecl;
  class TypedefDecl;
  class TypedefNameDecl;
  class TypeLoc;
  class TypoCorrectionConsumer;
  class UnqualifiedId;
  class UnresolvedLookupExpr;
  class UnresolvedMemberExpr;
  class UnresolvedSetImpl;
  class UnresolvedSetIterator;
  class UsingDecl;
  class UsingShadowDecl;
  class ValueDecl;
  class VarDecl;
  class VarTemplateSpecializationDecl;
  class VisibilityAttr;
  class VisibleDeclConsumer;
  class IndirectFieldDecl;
  struct DeductionFailureInfo;
  class TemplateSpecCandidateSet;

  class CXXThisExpr; // HLSL Change

namespace sema {
  class AccessedEntity;
  class BlockScopeInfo;
  class CapturedRegionScopeInfo;
  class CapturingScopeInfo;
  class CompoundScopeInfo;
  class DelayedDiagnostic;
  class DelayedDiagnosticPool;
  class FunctionScopeInfo;
  class LambdaScopeInfo;
  class PossiblyUnreachableDiag;
  class TemplateDeductionInfo;
}

namespace threadSafety {
  class BeforeSet;
  void threadSafetyCleanup(BeforeSet* Cache);
}

// FIXME: No way to easily map from TemplateTypeParmTypes to
// TemplateTypeParmDecls, so we have this horrible PointerUnion.
typedef std::pair<llvm::PointerUnion<const TemplateTypeParmType*, NamedDecl*>,
                  SourceLocation> UnexpandedParameterPack;

/// Describes whether we've seen any nullability information for the given
/// file.
struct FileNullability {
  /// The first pointer declarator (of any pointer kind) in the file that does
  /// not have a corresponding nullability annotation.
  SourceLocation PointerLoc;

  /// Which kind of pointer declarator we saw.
  uint8_t PointerKind;

  /// Whether we saw any type nullability annotations in the given file.
  bool SawTypeNullability = false;
};

/// A mapping from file IDs to a record of whether we've seen nullability
/// information in that file.
class FileNullabilityMap {
  /// A mapping from file IDs to the nullability information for each file ID.
  llvm::DenseMap<FileID, FileNullability> Map;

  /// A single-element cache based on the file ID.
  struct {
    FileID File;
    FileNullability Nullability;
  } Cache;

public:
  FileNullability &operator[](FileID file) {
    // Check the single-element cache.
    if (file == Cache.File)
      return Cache.Nullability;

    // It's not in the single-element cache; flush the cache if we have one.
    if (!Cache.File.isInvalid()) {
      Map[Cache.File] = Cache.Nullability;
    }

    // Pull this entry into the cache.
    Cache.File = file;
    Cache.Nullability = Map[file];
    return Cache.Nullability;
  }
};

/// Sema - This implements semantic analysis and AST building for C.
class Sema {
  Sema(const Sema &) = delete;
  void operator=(const Sema &) = delete;

  ///\brief Source of additional semantic information.
  ExternalSemaSource *ExternalSource;

  ///\brief Whether Sema has generated a multiplexer and has to delete it.
  bool isMultiplexExternalSource;

  static bool mightHaveNonExternalLinkage(const DeclaratorDecl *FD);

  bool isVisibleSlow(const NamedDecl *D);

  bool shouldLinkPossiblyHiddenDecl(const NamedDecl *Old,
                                    const NamedDecl *New) {
    // We are about to link these. It is now safe to compute the linkage of
    // the new decl. If the new decl has external linkage, we will
    // link it with the hidden decl (which also has external linkage) and
    // it will keep having external linkage. If it has internal linkage, we
    // will not link it. Since it has no previous decls, it will remain
    // with internal linkage.
    if (getLangOpts().ModulesHideInternalLinkage)
      return isVisible(Old) || New->isExternallyVisible();
    return true;
  }

public:
  typedef OpaquePtr<DeclGroupRef> DeclGroupPtrTy;
  typedef OpaquePtr<TemplateName> TemplateTy;
  typedef OpaquePtr<QualType> TypeTy;

  OpenCLOptions OpenCLFeatures;
  FPOptions FPFeatures;

  const LangOptions &LangOpts;
  Preprocessor &PP;
  ASTContext &Context;
  ASTConsumer &Consumer;
  DiagnosticsEngine &Diags;
  SourceManager &SourceMgr;

  /// \brief Flag indicating whether or not to collect detailed statistics.
  bool CollectStats;

  /// \brief Code-completion consumer.
  CodeCompleteConsumer *CodeCompleter;

  /// CurContext - This is the current declaration context of parsing.
  DeclContext *CurContext;

  /// \brief Generally null except when we temporarily switch decl contexts,
  /// like in \see ActOnObjCTemporaryExitContainerContext.
  DeclContext *OriginalLexicalContext;

  /// VAListTagName - The declaration name corresponding to __va_list_tag.
  /// This is used as part of a hack to omit that class from ADL results.
  DeclarationName VAListTagName;

  /// PackContext - Manages the stack for \#pragma pack. An alignment
  /// of 0 indicates default alignment.
  void *PackContext; // Really a "PragmaPackStack*"

  bool MSStructPragmaOn; // True when \#pragma ms_struct on

  /// \brief Controls member pointer representation format under the MS ABI.
  LangOptions::PragmaMSPointersToMembersKind
      MSPointerToMemberRepresentationMethod;

  // HLSL Change Begin
  // The HLSL rewriter doesn't define a default matrix pack,
  // so we must preserve the lack of annotations to avoid changing semantics.
  bool HasDefaultMatrixPack = false;
  // Uses of #pragma pack_matrix change the default pack.
  bool DefaultMatrixPackRowMajor = false;
  // HLSL Change End.

  enum PragmaVtorDispKind {
    PVDK_Push,          ///< #pragma vtordisp(push, mode)
    PVDK_Set,           ///< #pragma vtordisp(mode)
    PVDK_Pop,           ///< #pragma vtordisp(pop)
    PVDK_Reset          ///< #pragma vtordisp()
  };

  enum PragmaMsStackAction {
    PSK_Reset,    // #pragma ()
    PSK_Set,      // #pragma ("name")
    PSK_Push,     // #pragma (push[, id])
    PSK_Push_Set, // #pragma (push[, id], "name")
    PSK_Pop,      // #pragma (pop[, id])
    PSK_Pop_Set,  // #pragma (pop[, id], "name")
  };

  /// \brief Whether to insert vtordisps prior to virtual bases in the Microsoft
  /// C++ ABI.  Possible values are 0, 1, and 2, which mean:
  ///
  /// 0: Suppress all vtordisps
  /// 1: Insert vtordisps in the presence of vbase overrides and non-trivial
  ///    structors
  /// 2: Always insert vtordisps to support RTTI on partially constructed
  ///    objects
  ///
  /// The stack always has at least one element in it.
  SmallVector<MSVtorDispAttr::Mode, 2> VtorDispModeStack;

  /// Stack of active SEH __finally scopes.  Can be empty.
  SmallVector<Scope*, 2> CurrentSEHFinally;

  /// \brief Source location for newly created implicit MSInheritanceAttrs
  SourceLocation ImplicitMSInheritanceAttrLoc;

  template<typename ValueType>
  struct PragmaStack {
    struct Slot {
      llvm::StringRef StackSlotLabel;
      ValueType Value;
      SourceLocation PragmaLocation;
      Slot(llvm::StringRef StackSlotLabel,
           ValueType Value,
           SourceLocation PragmaLocation)
        : StackSlotLabel(StackSlotLabel), Value(Value),
          PragmaLocation(PragmaLocation) {}
    };
    void Act(SourceLocation PragmaLocation,
             PragmaMsStackAction Action,
             llvm::StringRef StackSlotLabel,
             ValueType Value);
    explicit PragmaStack(const ValueType &Value)
      : CurrentValue(Value) {}
    SmallVector<Slot, 2> Stack;
    ValueType CurrentValue;
    SourceLocation CurrentPragmaLocation;
  };
  // FIXME: We should serialize / deserialize these if they occur in a PCH (but
  // we shouldn't do so if they're in a module).
  PragmaStack<StringLiteral *> DataSegStack;
  PragmaStack<StringLiteral *> BSSSegStack;
  PragmaStack<StringLiteral *> ConstSegStack;
  PragmaStack<StringLiteral *> CodeSegStack;

  /// A mapping that describes the nullability we've seen in each header file.
  FileNullabilityMap NullabilityMap;

  /// Last section used with #pragma init_seg.
  StringLiteral *CurInitSeg;
  SourceLocation CurInitSegLoc;

  /// VisContext - Manages the stack for \#pragma GCC visibility.
  void *VisContext; // Really a "PragmaVisStack*"

  /// \brief This represents the last location of a "#pragma clang optimize off"
  /// directive if such a directive has not been closed by an "on" yet. If
  /// optimizations are currently "on", this is set to an invalid location.
  SourceLocation OptimizeOffPragmaLocation;

  /// \brief Flag indicating if Sema is building a recovery call expression.
  ///
  /// This flag is used to avoid building recovery call expressions
  /// if Sema is already doing so, which would cause infinite recursions.
  bool IsBuildingRecoveryCallExpr;

  /// ExprNeedsCleanups - True if the current evaluation context
  /// requires cleanups to be run at its conclusion.
  bool ExprNeedsCleanups;

  /// ExprCleanupObjects - This is the stack of objects requiring
  /// cleanup that are created by the current full expression.  The
  /// element type here is ExprWithCleanups::Object.
  SmallVector<BlockDecl*, 8> ExprCleanupObjects;

  /// \brief Store a list of either DeclRefExprs or MemberExprs
  ///  that contain a reference to a variable (constant) that may or may not
  ///  be odr-used in this Expr, and we won't know until all lvalue-to-rvalue
  ///  and discarded value conversions have been applied to all subexpressions 
  ///  of the enclosing full expression.  This is cleared at the end of each 
  ///  full expression. 
  llvm::SmallPtrSet<Expr*, 2> MaybeODRUseExprs;

  /// \brief Stack containing information about each of the nested
  /// function, block, and method scopes that are currently active.
  ///
  /// This array is never empty.  Clients should ignore the first
  /// element, which is used to cache a single FunctionScopeInfo
  /// that's used to parse every top-level function.
  SmallVector<sema::FunctionScopeInfo *, 4> FunctionScopes;

  typedef LazyVector<TypedefNameDecl *, ExternalSemaSource,
                     &ExternalSemaSource::ReadExtVectorDecls, 2, 2>
    ExtVectorDeclsType;

  /// ExtVectorDecls - This is a list all the extended vector types. This allows
  /// us to associate a raw vector type with one of the ext_vector type names.
  /// This is only necessary for issuing pretty diagnostics.
  ExtVectorDeclsType ExtVectorDecls;

  /// FieldCollector - Collects CXXFieldDecls during parsing of C++ classes.
  std::unique_ptr<CXXFieldCollector> FieldCollector;

  typedef llvm::SmallSetVector<const NamedDecl*, 16> NamedDeclSetType;

  /// \brief Set containing all declared private fields that are not used.
  NamedDeclSetType UnusedPrivateFields;

  /// \brief Set containing all typedefs that are likely unused.
  llvm::SmallSetVector<const TypedefNameDecl *, 4>
      UnusedLocalTypedefNameCandidates;

  /// \brief Delete-expressions to be analyzed at the end of translation unit
  ///
  /// This list contains class members, and locations of delete-expressions
  /// that could not be proven as to whether they mismatch with new-expression
  /// used in initializer of the field.
  typedef std::pair<SourceLocation, bool> DeleteExprLoc;
  typedef llvm::SmallVector<DeleteExprLoc, 4> DeleteLocs;
  llvm::MapVector<FieldDecl *, DeleteLocs> DeleteExprs;

  typedef llvm::SmallPtrSet<const CXXRecordDecl*, 8> RecordDeclSetTy;

  /// PureVirtualClassDiagSet - a set of class declarations which we have
  /// emitted a list of pure virtual functions. Used to prevent emitting the
  /// same list more than once.
  std::unique_ptr<RecordDeclSetTy> PureVirtualClassDiagSet;

  /// ParsingInitForAutoVars - a set of declarations with auto types for which
  /// we are currently parsing the initializer.
  llvm::SmallPtrSet<const Decl*, 4> ParsingInitForAutoVars;

  /// \brief Look for a locally scoped extern "C" declaration by the given name.
  NamedDecl *findLocallyScopedExternCDecl(DeclarationName Name);

  typedef LazyVector<VarDecl *, ExternalSemaSource,
                     &ExternalSemaSource::ReadTentativeDefinitions, 2, 2>
    TentativeDefinitionsType;

  /// \brief All the tentative definitions encountered in the TU.
  TentativeDefinitionsType TentativeDefinitions;

  typedef LazyVector<const DeclaratorDecl *, ExternalSemaSource,
                     &ExternalSemaSource::ReadUnusedFileScopedDecls, 2, 2>
    UnusedFileScopedDeclsType;

  /// \brief The set of file scoped decls seen so far that have not been used
  /// and must warn if not used. Only contains the first declaration.
  UnusedFileScopedDeclsType UnusedFileScopedDecls;

  typedef LazyVector<CXXConstructorDecl *, ExternalSemaSource,
                     &ExternalSemaSource::ReadDelegatingConstructors, 2, 2>
    DelegatingCtorDeclsType;

  /// \brief All the delegating constructors seen so far in the file, used for
  /// cycle detection at the end of the TU.
  DelegatingCtorDeclsType DelegatingCtorDecls;

  /// \brief All the overriding functions seen during a class definition
  /// that had their exception spec checks delayed, plus the overridden
  /// function.
  SmallVector<std::pair<const CXXMethodDecl*, const CXXMethodDecl*>, 2>
    DelayedExceptionSpecChecks;

  /// \brief All the members seen during a class definition which were both
  /// explicitly defaulted and had explicitly-specified exception
  /// specifications, along with the function type containing their
  /// user-specified exception specification. Those exception specifications
  /// were overridden with the default specifications, but we still need to
  /// check whether they are compatible with the default specification, and
  /// we can't do that until the nesting set of class definitions is complete.
  SmallVector<std::pair<CXXMethodDecl*, const FunctionProtoType*>, 2>
    DelayedDefaultedMemberExceptionSpecs;

  typedef llvm::MapVector<const FunctionDecl *, LateParsedTemplate *>
      LateParsedTemplateMapT;
  LateParsedTemplateMapT LateParsedTemplateMap;

  /// \brief Callback to the parser to parse templated functions when needed.
  typedef void LateTemplateParserCB(void *P, LateParsedTemplate &LPT);
  typedef void LateTemplateParserCleanupCB(void *P);
  LateTemplateParserCB *LateTemplateParser;
  LateTemplateParserCleanupCB *LateTemplateParserCleanup;
  void *OpaqueParser;

  void SetLateTemplateParser(LateTemplateParserCB *LTP,
                             LateTemplateParserCleanupCB *LTPCleanup,
                             void *P) {
    LateTemplateParser = LTP;
    LateTemplateParserCleanup = LTPCleanup;
    OpaqueParser = P;
  }

  class DelayedDiagnostics;

  class DelayedDiagnosticsState {
    sema::DelayedDiagnosticPool *SavedPool;
    friend class Sema::DelayedDiagnostics;
  };
  typedef DelayedDiagnosticsState ParsingDeclState;
  typedef DelayedDiagnosticsState ProcessingContextState;

  /// A class which encapsulates the logic for delaying diagnostics
  /// during parsing and other processing.
  class DelayedDiagnostics {
    /// \brief The current pool of diagnostics into which delayed
    /// diagnostics should go.
    sema::DelayedDiagnosticPool *CurPool;

  public:
    DelayedDiagnostics() : CurPool(nullptr) {}

    /// Adds a delayed diagnostic.
    void add(const sema::DelayedDiagnostic &diag); // in DelayedDiagnostic.h

    /// Determines whether diagnostics should be delayed.
    bool shouldDelayDiagnostics() { return CurPool != nullptr; }

    /// Returns the current delayed-diagnostics pool.
    sema::DelayedDiagnosticPool *getCurrentPool() const {
      return CurPool;
    }

    /// Enter a new scope.  Access and deprecation diagnostics will be
    /// collected in this pool.
    DelayedDiagnosticsState push(sema::DelayedDiagnosticPool &pool) {
      DelayedDiagnosticsState state;
      state.SavedPool = CurPool;
      CurPool = &pool;
      return state;
    }

    /// Leave a delayed-diagnostic state that was previously pushed.
    /// Do not emit any of the diagnostics.  This is performed as part
    /// of the bookkeeping of popping a pool "properly".
    void popWithoutEmitting(DelayedDiagnosticsState state) {
      CurPool = state.SavedPool;
    }

    /// Enter a new scope where access and deprecation diagnostics are
    /// not delayed.
    DelayedDiagnosticsState pushUndelayed() {
      DelayedDiagnosticsState state;
      state.SavedPool = CurPool;
      CurPool = nullptr;
      return state;
    }

    /// Undo a previous pushUndelayed().
    void popUndelayed(DelayedDiagnosticsState state) {
      assert(CurPool == nullptr);
      CurPool = state.SavedPool;
    }
  } DelayedDiagnostics;

  /// A RAII object to temporarily push a declaration context.
  class ContextRAII {
  private:
    Sema &S;
    DeclContext *SavedContext;
    ProcessingContextState SavedContextState;
    QualType SavedCXXThisTypeOverride;

  public:
    ContextRAII(Sema &S, DeclContext *ContextToPush, bool NewThisContext = true)
      : S(S), SavedContext(S.CurContext),
        SavedContextState(S.DelayedDiagnostics.pushUndelayed()),
        SavedCXXThisTypeOverride(S.CXXThisTypeOverride)
    {
      assert(ContextToPush && "pushing null context");
      S.CurContext = ContextToPush;
      if (NewThisContext)
        S.CXXThisTypeOverride = QualType();
    }

    void pop() {
      if (!SavedContext) return;
      S.CurContext = SavedContext;
      S.DelayedDiagnostics.popUndelayed(SavedContextState);
      S.CXXThisTypeOverride = SavedCXXThisTypeOverride;
      SavedContext = nullptr;
    }

    ~ContextRAII() {
      pop();
    }
  };

  /// \brief RAII object to handle the state changes required to synthesize
  /// a function body.
  class SynthesizedFunctionScope {
    Sema &S;
    Sema::ContextRAII SavedContext;
    
  public:
    SynthesizedFunctionScope(Sema &S, DeclContext *DC)
      : S(S), SavedContext(S, DC) 
    {
      S.PushFunctionScope();
      S.PushExpressionEvaluationContext(Sema::PotentiallyEvaluated);
    }
    
    ~SynthesizedFunctionScope() {
      S.PopExpressionEvaluationContext();
      S.PopFunctionScopeInfo();
    }
  };

  /// WeakUndeclaredIdentifiers - Identifiers contained in
  /// \#pragma weak before declared. rare. may alias another
  /// identifier, declared or undeclared
  llvm::MapVector<IdentifierInfo *, WeakInfo> WeakUndeclaredIdentifiers;

  /// ExtnameUndeclaredIdentifiers - Identifiers contained in
  /// \#pragma redefine_extname before declared.  Used in Solaris system headers
  /// to define functions that occur in multiple standards to call the version
  /// in the currently selected standard.
  llvm::DenseMap<IdentifierInfo*,AsmLabelAttr*> ExtnameUndeclaredIdentifiers;


  /// \brief Load weak undeclared identifiers from the external source.
  void LoadExternalWeakUndeclaredIdentifiers();

  /// WeakTopLevelDecl - Translation-unit scoped declarations generated by
  /// \#pragma weak during processing of other Decls.
  /// I couldn't figure out a clean way to generate these in-line, so
  /// we store them here and handle separately -- which is a hack.
  /// It would be best to refactor this.
  SmallVector<Decl*,2> WeakTopLevelDecl;

  IdentifierResolver IdResolver;

  /// Translation Unit Scope - useful to Objective-C actions that need
  /// to lookup file scope declarations in the "ordinary" C decl namespace.
  /// For example, user-defined classes, built-in "id" type, etc.
  Scope *TUScope;

  /// \brief The C++ "std" namespace, where the standard library resides.
  LazyDeclPtr StdNamespace;

  /// \brief The C++ "std::bad_alloc" class, which is defined by the C++
  /// standard library.
  LazyDeclPtr StdBadAlloc;

  /// \brief The C++ "std::initializer_list" template, which is defined in
  /// \<initializer_list>.
  ClassTemplateDecl *StdInitializerList;

  /// \brief The C++ "type_info" declaration, which is defined in \<typeinfo>.
  RecordDecl *CXXTypeInfoDecl;

  /// \brief The MSVC "_GUID" struct, which is defined in MSVC header files.
  RecordDecl *MSVCGuidDecl;

  /// \brief Caches identifiers/selectors for NSFoundation APIs.
  // std::unique_ptr<NSAPI> NSAPIObj; // HLSL Change

  /// \brief The declaration of the Objective-C NSNumber class.
  ObjCInterfaceDecl *NSNumberDecl;

  /// \brief The declaration of the Objective-C NSValue class.
  ObjCInterfaceDecl *NSValueDecl;

  /// \brief Pointer to NSNumber type (NSNumber *).
  QualType NSNumberPointer;

  /// \brief Pointer to NSValue type (NSValue *).
  QualType NSValuePointer;

  /// \brief The Objective-C NSNumber methods used to create NSNumber literals.
  ObjCMethodDecl *NSNumberLiteralMethods[NSAPI::NumNSNumberLiteralMethods];

  /// \brief The declaration of the Objective-C NSString class.
  ObjCInterfaceDecl *NSStringDecl;

  /// \brief Pointer to NSString type (NSString *).
  QualType NSStringPointer;

  /// \brief The declaration of the stringWithUTF8String: method.
  ObjCMethodDecl *StringWithUTF8StringMethod;

  /// \brief The declaration of the valueWithBytes:objCType: method.
  ObjCMethodDecl *ValueWithBytesObjCTypeMethod;

  /// \brief The declaration of the Objective-C NSArray class.
  ObjCInterfaceDecl *NSArrayDecl;

  /// \brief The declaration of the arrayWithObjects:count: method.
  ObjCMethodDecl *ArrayWithObjectsMethod;

  /// \brief The declaration of the Objective-C NSDictionary class.
  ObjCInterfaceDecl *NSDictionaryDecl;

  /// \brief The declaration of the dictionaryWithObjects:forKeys:count: method.
  ObjCMethodDecl *DictionaryWithObjectsMethod;

  /// \brief id<NSCopying> type.
  QualType QIDNSCopying;

  /// \brief will hold 'respondsToSelector:'
  Selector RespondsToSelectorSel;

  /// \brief counter for internal MS Asm label names.
  unsigned MSAsmLabelNameCounter;

  /// A flag to remember whether the implicit forms of operator new and delete
  /// have been declared.
  bool GlobalNewDeleteDeclared;

  /// A flag to indicate that we're in a context that permits abstract
  /// references to fields.  This is really a 
  bool AllowAbstractFieldReference;

  /// \brief Describes how the expressions currently being parsed are
  /// evaluated at run-time, if at all.
  enum ExpressionEvaluationContext {
    /// \brief The current expression and its subexpressions occur within an
    /// unevaluated operand (C++11 [expr]p7), such as the subexpression of
    /// \c sizeof, where the type of the expression may be significant but
    /// no code will be generated to evaluate the value of the expression at
    /// run time.
    Unevaluated,

    /// \brief The current expression occurs within an unevaluated
    /// operand that unconditionally permits abstract references to
    /// fields, such as a SIZE operator in MS-style inline assembly.
    UnevaluatedAbstract,

    /// \brief The current context is "potentially evaluated" in C++11 terms,
    /// but the expression is evaluated at compile-time (like the values of
    /// cases in a switch statement).
    ConstantEvaluated,

    /// \brief The current expression is potentially evaluated at run time,
    /// which means that code may be generated to evaluate the value of the
    /// expression at run time.
    PotentiallyEvaluated,

    /// \brief The current expression is potentially evaluated, but any
    /// declarations referenced inside that expression are only used if
    /// in fact the current expression is used.
    ///
    /// This value is used when parsing default function arguments, for which
    /// we would like to provide diagnostics (e.g., passing non-POD arguments
    /// through varargs) but do not want to mark declarations as "referenced"
    /// until the default argument is used.
    PotentiallyEvaluatedIfUsed
  };

  /// \brief Data structure used to record current or nested
  /// expression evaluation contexts.
  struct ExpressionEvaluationContextRecord {
    /// \brief The expression evaluation context.
    ExpressionEvaluationContext Context;

    /// \brief Whether the enclosing context needed a cleanup.
    bool ParentNeedsCleanups;

    /// \brief Whether we are in a decltype expression.
    bool IsDecltype;

    /// \brief The number of active cleanup objects when we entered
    /// this expression evaluation context.
    unsigned NumCleanupObjects;

    /// \brief The number of typos encountered during this expression evaluation
    /// context (i.e. the number of TypoExprs created).
    unsigned NumTypos;

    llvm::SmallPtrSet<Expr*, 2> SavedMaybeODRUseExprs;

    /// \brief The lambdas that are present within this context, if it
    /// is indeed an unevaluated context.
    SmallVector<LambdaExpr *, 2> Lambdas;

    /// \brief The declaration that provides context for lambda expressions
    /// and block literals if the normal declaration context does not
    /// suffice, e.g., in a default function argument.
    Decl *ManglingContextDecl;

    /// \brief The context information used to mangle lambda expressions
    /// and block literals within this context.
    ///
    /// This mangling information is allocated lazily, since most contexts
    /// do not have lambda expressions or block literals.
    IntrusiveRefCntPtr<MangleNumberingContext> MangleNumbering;

    /// \brief If we are processing a decltype type, a set of call expressions
    /// for which we have deferred checking the completeness of the return type.
    SmallVector<CallExpr *, 8> DelayedDecltypeCalls;

    /// \brief If we are processing a decltype type, a set of temporary binding
    /// expressions for which we have deferred checking the destructor.
    SmallVector<CXXBindTemporaryExpr *, 8> DelayedDecltypeBinds;

    ExpressionEvaluationContextRecord(ExpressionEvaluationContext Context,
                                      unsigned NumCleanupObjects,
                                      bool ParentNeedsCleanups,
                                      Decl *ManglingContextDecl,
                                      bool IsDecltype)
      : Context(Context), ParentNeedsCleanups(ParentNeedsCleanups),
        IsDecltype(IsDecltype), NumCleanupObjects(NumCleanupObjects),
        NumTypos(0),
        ManglingContextDecl(ManglingContextDecl), MangleNumbering() { }

    /// \brief Retrieve the mangling numbering context, used to consistently
    /// number constructs like lambdas for mangling.
    MangleNumberingContext &getMangleNumberingContext(ASTContext &Ctx);

    bool isUnevaluated() const {
      return Context == Unevaluated || Context == UnevaluatedAbstract;
    }
  };

  /// A stack of expression evaluation contexts.
  SmallVector<ExpressionEvaluationContextRecord, 8> ExprEvalContexts;

  /// \brief Compute the mangling number context for a lambda expression or
  /// block literal.
  ///
  /// \param DC - The DeclContext containing the lambda expression or
  /// block literal.
  /// \param[out] ManglingContextDecl - Returns the ManglingContextDecl
  /// associated with the context, if relevant.
  MangleNumberingContext *getCurrentMangleNumberContext(
    const DeclContext *DC,
    Decl *&ManglingContextDecl);


  /// SpecialMemberOverloadResult - The overloading result for a special member
  /// function.
  ///
  /// This is basically a wrapper around PointerIntPair. The lowest bits of the
  /// integer are used to determine whether overload resolution succeeded.
  class SpecialMemberOverloadResult : public llvm::FastFoldingSetNode {
  public:
    enum Kind {
      NoMemberOrDeleted,
      Ambiguous,
      Success
    };

  private:
    llvm::PointerIntPair<CXXMethodDecl*, 2> Pair;

  public:
    SpecialMemberOverloadResult(const llvm::FoldingSetNodeID &ID)
      : FastFoldingSetNode(ID)
    {}

    CXXMethodDecl *getMethod() const { return Pair.getPointer(); }
    void setMethod(CXXMethodDecl *MD) { Pair.setPointer(MD); }

    Kind getKind() const { return static_cast<Kind>(Pair.getInt()); }
    void setKind(Kind K) { Pair.setInt(K); }
  };

  /// \brief A cache of special member function overload resolution results
  /// for C++ records.
  llvm::FoldingSet<SpecialMemberOverloadResult> SpecialMemberCache;

  /// \brief The kind of translation unit we are processing.
  ///
  /// When we're processing a complete translation unit, Sema will perform
  /// end-of-translation-unit semantic tasks (such as creating
  /// initializers for tentative definitions in C) once parsing has
  /// completed. Modules and precompiled headers perform different kinds of
  /// checks.
  TranslationUnitKind TUKind;

  llvm::BumpPtrAllocator BumpAlloc;

  /// \brief The number of SFINAE diagnostics that have been trapped.
  unsigned NumSFINAEErrors;

  typedef llvm::DenseMap<ParmVarDecl *, llvm::TinyPtrVector<ParmVarDecl *>>
    UnparsedDefaultArgInstantiationsMap;

  /// \brief A mapping from parameters with unparsed default arguments to the
  /// set of instantiations of each parameter.
  ///
  /// This mapping is a temporary data structure used when parsing
  /// nested class templates or nested classes of class templates,
  /// where we might end up instantiating an inner class before the
  /// default arguments of its methods have been parsed.
  UnparsedDefaultArgInstantiationsMap UnparsedDefaultArgInstantiations;

  // Contains the locations of the beginning of unparsed default
  // argument locations.
  llvm::DenseMap<ParmVarDecl *, SourceLocation> UnparsedDefaultArgLocs;

  /// UndefinedInternals - all the used, undefined objects which require a
  /// definition in this translation unit.
  llvm::DenseMap<NamedDecl *, SourceLocation> UndefinedButUsed;

  /// Obtain a sorted list of functions that are undefined but ODR-used.
  void getUndefinedButUsed(
      SmallVectorImpl<std::pair<NamedDecl *, SourceLocation> > &Undefined);

  /// Retrieves list of suspicious delete-expressions that will be checked at
  /// the end of translation unit.
  const llvm::MapVector<FieldDecl *, DeleteLocs> &
  getMismatchingDeleteExpressions() const;

  typedef std::pair<ObjCMethodList, ObjCMethodList> GlobalMethods;
  typedef llvm::DenseMap<Selector, GlobalMethods> GlobalMethodPool;

  /// Method Pool - allows efficient lookup when typechecking messages to "id".
  /// We need to maintain a list, since selectors can have differing signatures
  /// across classes. In Cocoa, this happens to be extremely uncommon (only 1%
  /// of selectors are "overloaded").
  /// At the head of the list it is recorded whether there were 0, 1, or >= 2
  /// methods inside categories with a particular selector.
  GlobalMethodPool MethodPool;

  /// Method selectors used in a \@selector expression. Used for implementation
  /// of -Wselector.
  llvm::MapVector<Selector, SourceLocation> ReferencedSelectors;

  /// Kinds of C++ special members.
  enum CXXSpecialMember {
    CXXDefaultConstructor,
    CXXCopyConstructor,
    CXXMoveConstructor,
    CXXCopyAssignment,
    CXXMoveAssignment,
    CXXDestructor,
    CXXInvalid
  };

  typedef std::pair<CXXRecordDecl*, CXXSpecialMember> SpecialMemberDecl;

  /// The C++ special members which we are currently in the process of
  /// declaring. If this process recursively triggers the declaration of the
  /// same special member, we should act as if it is not yet declared.
  llvm::SmallSet<SpecialMemberDecl, 4> SpecialMembersBeingDeclared;

  void ReadMethodPool(Selector Sel);

  /// Private Helper predicate to check for 'self'.
  bool isSelfExpr(Expr *RExpr);
  bool isSelfExpr(Expr *RExpr, const ObjCMethodDecl *Method);

  /// \brief Cause the active diagnostic on the DiagosticsEngine to be
  /// emitted. This is closely coupled to the SemaDiagnosticBuilder class and
  /// should not be used elsewhere.
  void EmitCurrentDiagnostic(unsigned DiagID);

  /// Records and restores the FP_CONTRACT state on entry/exit of compound
  /// statements.
  class FPContractStateRAII {
  public:
    FPContractStateRAII(Sema& S)
      : S(S), OldFPContractState(S.FPFeatures.fp_contract) {}
    ~FPContractStateRAII() {
      S.FPFeatures.fp_contract = OldFPContractState;
    }
  private:
    Sema& S;
    bool OldFPContractState : 1;
  };

  void addImplicitTypedef(StringRef Name, QualType T);

public:
  Sema(Preprocessor &pp, ASTContext &ctxt, ASTConsumer &consumer,
       TranslationUnitKind TUKind = TU_Complete,
       CodeCompleteConsumer *CompletionConsumer = nullptr);
  ~Sema();

  /// \brief Perform initialization that occurs after the parser has been
  /// initialized but before it parses anything.
  void Initialize();

  const LangOptions &getLangOpts() const { return LangOpts; }
  OpenCLOptions &getOpenCLOptions() { return OpenCLFeatures; }
  FPOptions     &getFPOptions() { return FPFeatures; }

  DiagnosticsEngine &getDiagnostics() const { return Diags; }
  SourceManager &getSourceManager() const { return SourceMgr; }
  Preprocessor &getPreprocessor() const { return PP; }
  ASTContext &getASTContext() const { return Context; }
  ASTConsumer &getASTConsumer() const { return Consumer; }
  ASTMutationListener *getASTMutationListener() const;
  ExternalSemaSource* getExternalSource() const { return ExternalSource; }

  ///\brief Registers an external source. If an external source already exists,
  /// creates a multiplex external source and appends to it.
  ///
  ///\param[in] E - A non-null external sema source.
  ///
  void addExternalSource(ExternalSemaSource *E);

  void PrintStats() const;

  /// \brief Helper class that creates diagnostics with optional
  /// template instantiation stacks.
  ///
  /// This class provides a wrapper around the basic DiagnosticBuilder
  /// class that emits diagnostics. SemaDiagnosticBuilder is
  /// responsible for emitting the diagnostic (as DiagnosticBuilder
  /// does) and, if the diagnostic comes from inside a template
  /// instantiation, printing the template instantiation stack as
  /// well.
  class SemaDiagnosticBuilder : public DiagnosticBuilder {
    Sema &SemaRef;
    unsigned DiagID;

  public:
    SemaDiagnosticBuilder(DiagnosticBuilder &DB, Sema &SemaRef, unsigned DiagID)
      : DiagnosticBuilder(DB), SemaRef(SemaRef), DiagID(DiagID) { }

    ~SemaDiagnosticBuilder() {
      // If we aren't active, there is nothing to do.
      if (!isActive()) return;

      // Otherwise, we need to emit the diagnostic. First flush the underlying
      // DiagnosticBuilder data, and clear the diagnostic builder itself so it
      // won't emit the diagnostic in its own destructor.
      //
      // This seems wasteful, in that as written the DiagnosticBuilder dtor will
      // do its own needless checks to see if the diagnostic needs to be
      // emitted. However, because we take care to ensure that the builder
      // objects never escape, a sufficiently smart compiler will be able to
      // eliminate that code.
      FlushCounts();
      Clear();

      // Dispatch to Sema to emit the diagnostic.
      SemaRef.EmitCurrentDiagnostic(DiagID);
    }

    /// Teach operator<< to produce an object of the correct type.
    template<typename T>
    friend const SemaDiagnosticBuilder &operator<<(
        const SemaDiagnosticBuilder &Diag, const T &Value) {
      const DiagnosticBuilder &BaseDiag = Diag;
      BaseDiag << Value;
      return Diag;
    }
  };

  /// \brief Emit a diagnostic.
  SemaDiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID) {
    DiagnosticBuilder DB = Diags.Report(Loc, DiagID);
    return SemaDiagnosticBuilder(DB, *this, DiagID);
  }

  /// \brief Emit a partial diagnostic.
  SemaDiagnosticBuilder Diag(SourceLocation Loc, const PartialDiagnostic& PD);

  /// \brief Build a partial diagnostic.
  PartialDiagnostic PDiag(unsigned DiagID = 0); // in SemaInternal.h

  bool findMacroSpelling(SourceLocation &loc, StringRef name);

  /// \brief Get a string to suggest for zero-initialization of a type.
  std::string
  getFixItZeroInitializerForType(QualType T, SourceLocation Loc) const;
  std::string getFixItZeroLiteralForType(QualType T, SourceLocation Loc) const;

  /// \brief Calls \c Lexer::getLocForEndOfToken()
  SourceLocation getLocForEndOfToken(SourceLocation Loc, unsigned Offset = 0);

  /// \brief Retrieve the module loader associated with the preprocessor.
  ModuleLoader &getModuleLoader() const;

  void emitAndClearUnusedLocalTypedefWarnings();

  void ActOnEndOfTranslationUnit();

  void CheckDelegatingCtorCycles();

  Scope *getScopeForContext(DeclContext *Ctx);

  void PushFunctionScope();
  void PushBlockScope(Scope *BlockScope, BlockDecl *Block);
  sema::LambdaScopeInfo *PushLambdaScope();

  /// \brief This is used to inform Sema what the current TemplateParameterDepth
  /// is during Parsing.  Currently it is used to pass on the depth
  /// when parsing generic lambda 'auto' parameters.
  void RecordParsingTemplateParameterDepth(unsigned Depth);
  
  void PushCapturedRegionScope(Scope *RegionScope, CapturedDecl *CD,
                               RecordDecl *RD,
                               CapturedRegionKind K);
  void
  PopFunctionScopeInfo(const sema::AnalysisBasedWarnings::Policy *WP = nullptr,
                       const Decl *D = nullptr,
                       const BlockExpr *blkExpr = nullptr);

  sema::FunctionScopeInfo *getCurFunction() const {
    return FunctionScopes.back();
  }
  
  sema::FunctionScopeInfo *getEnclosingFunction() const {
    if (FunctionScopes.empty())
      return nullptr;
    
    for (int e = FunctionScopes.size()-1; e >= 0; --e) {
      if (isa<sema::BlockScopeInfo>(FunctionScopes[e]))
        continue;
      return FunctionScopes[e];
    }
    return nullptr;
  }
  
  template <typename ExprT>
  void recordUseOfEvaluatedWeak(const ExprT *E, bool IsRead=true) {
    if (!isUnevaluatedContext())
      getCurFunction()->recordUseOfWeak(E, IsRead);
  }
  
  void PushCompoundScope();
  void PopCompoundScope();

  sema::CompoundScopeInfo &getCurCompoundScope() const;

  bool hasAnyUnrecoverableErrorsInThisFunction() const;

  /// \brief Retrieve the current block, if any.
  sema::BlockScopeInfo *getCurBlock();

  /// \brief Retrieve the current lambda scope info, if any.
  sema::LambdaScopeInfo *getCurLambda();

  /// \brief Retrieve the current generic lambda info, if any.
  sema::LambdaScopeInfo *getCurGenericLambda();

  /// \brief Retrieve the current captured region, if any.
  sema::CapturedRegionScopeInfo *getCurCapturedRegion();

  /// WeakTopLevelDeclDecls - access to \#pragma weak-generated Decls
  SmallVectorImpl<Decl *> &WeakTopLevelDecls() { return WeakTopLevelDecl; }

  void ActOnComment(SourceRange Comment);

  //===--------------------------------------------------------------------===//
  // Type Analysis / Processing: SemaType.cpp.
  //

  QualType BuildQualifiedType(QualType T, SourceLocation Loc, Qualifiers Qs,
                              const DeclSpec *DS = nullptr);
  QualType BuildQualifiedType(QualType T, SourceLocation Loc, unsigned CVRA,
                              const DeclSpec *DS = nullptr);
  QualType BuildPointerType(QualType T,
                            SourceLocation Loc, DeclarationName Entity);
  QualType BuildReferenceType(QualType T, bool LValueRef,
                              SourceLocation Loc, DeclarationName Entity);
  QualType BuildArrayType(QualType T, ArrayType::ArraySizeModifier ASM,
                          Expr *ArraySize, unsigned Quals,
                          SourceRange Brackets, DeclarationName Entity);
  QualType BuildExtVectorType(QualType T, Expr *ArraySize,
                              SourceLocation AttrLoc);

  bool CheckFunctionReturnType(QualType T, SourceLocation Loc);

  unsigned deduceWeakPropertyFromType(QualType T) {
    if ((getLangOpts().getGC() != LangOptions::NonGC &&
         T.isObjCGCWeak()) ||
        (getLangOpts().ObjCAutoRefCount &&
         T.getObjCLifetime() == Qualifiers::OCL_Weak))
        return ObjCDeclSpec::DQ_PR_weak;
    return 0;
  }


  /// \brief Build a function type.
  ///
  /// This routine checks the function type according to C++ rules and
  /// under the assumption that the result type and parameter types have
  /// just been instantiated from a template. It therefore duplicates
  /// some of the behavior of GetTypeForDeclarator, but in a much
  /// simpler form that is only suitable for this narrow use case.
  ///
  /// \param T The return type of the function.
  ///
  /// \param ParamTypes The parameter types of the function. This array
  /// will be modified to account for adjustments to the types of the
  /// function parameters.
  ///
  /// \param Loc The location of the entity whose type involves this
  /// function type or, if there is no such entity, the location of the
  /// type that will have function type.
  ///
  /// \param Entity The name of the entity that involves the function
  /// type, if known.
  ///
  /// \param EPI Extra information about the function type. Usually this will
  /// be taken from an existing function with the same prototype.
  ///
  /// \returns A suitable function type, if there are no errors. The
  /// unqualified type will always be a FunctionProtoType.
  /// Otherwise, returns a NULL type.
  // HLSL Change - FIX - We should move param mods to parameter QualTypes
  QualType BuildFunctionType(QualType T, MutableArrayRef<QualType> ParamTypes,
                             SourceLocation Loc, DeclarationName Entity,
                             const FunctionProtoType::ExtProtoInfo &EPI,
                             ArrayRef<hlsl::ParameterModifier> ParamMods);
  // HLSL Change - End

  QualType BuildMemberPointerType(QualType T, QualType Class,
                                  SourceLocation Loc,
                                  DeclarationName Entity);
  QualType BuildBlockPointerType(QualType T,
                                 SourceLocation Loc, DeclarationName Entity);
  QualType BuildParenType(QualType T);
  QualType BuildAtomicType(QualType T, SourceLocation Loc);

  TypeSourceInfo *GetTypeForDeclarator(Declarator &D, Scope *S);
  TypeSourceInfo *GetTypeForDeclaratorCast(Declarator &D, QualType FromTy);
  TypeSourceInfo *GetTypeSourceInfoForDeclarator(Declarator &D, QualType T,
                                               TypeSourceInfo *ReturnTypeInfo);

  /// \brief Package the given type and TSI into a ParsedType.
  ParsedType CreateParsedType(QualType T, TypeSourceInfo *TInfo);
  DeclarationNameInfo GetNameForDeclarator(Declarator &D);
  DeclarationNameInfo GetNameFromUnqualifiedId(const UnqualifiedId &Name);
  static QualType GetTypeFromParser(ParsedType Ty,
                                    TypeSourceInfo **TInfo = nullptr);
  CanThrowResult canThrow(const Expr *E);
  const FunctionProtoType *ResolveExceptionSpec(SourceLocation Loc,
                                                const FunctionProtoType *FPT);
  void UpdateExceptionSpec(FunctionDecl *FD,
                           const FunctionProtoType::ExceptionSpecInfo &ESI);
  bool CheckSpecifiedExceptionType(QualType &T, const SourceRange &Range);
  bool CheckDistantExceptionSpec(QualType T);
  bool CheckEquivalentExceptionSpec(FunctionDecl *Old, FunctionDecl *New);
  bool CheckEquivalentExceptionSpec(
      const FunctionProtoType *Old, SourceLocation OldLoc,
      const FunctionProtoType *New, SourceLocation NewLoc);
  bool CheckEquivalentExceptionSpec(
      const PartialDiagnostic &DiagID, const PartialDiagnostic & NoteID,
      const FunctionProtoType *Old, SourceLocation OldLoc,
      const FunctionProtoType *New, SourceLocation NewLoc,
      bool *MissingExceptionSpecification = nullptr,
      bool *MissingEmptyExceptionSpecification = nullptr,
      bool AllowNoexceptAllMatchWithNoSpec = false,
      bool IsOperatorNew = false);
  bool CheckExceptionSpecSubset(
      const PartialDiagnostic &DiagID, const PartialDiagnostic & NoteID,
      const FunctionProtoType *Superset, SourceLocation SuperLoc,
      const FunctionProtoType *Subset, SourceLocation SubLoc);
  bool CheckParamExceptionSpec(const PartialDiagnostic & NoteID,
      const FunctionProtoType *Target, SourceLocation TargetLoc,
      const FunctionProtoType *Source, SourceLocation SourceLoc);

  TypeResult ActOnTypeName(Scope *S, Declarator &D);

  /// \brief The parser has parsed the context-sensitive type 'instancetype'
  /// in an Objective-C message declaration. Return the appropriate type.
  ParsedType ActOnObjCInstanceType(SourceLocation Loc);

  /// \brief Abstract class used to diagnose incomplete types.
  struct TypeDiagnoser {
    bool Suppressed;

    TypeDiagnoser(bool Suppressed = false) : Suppressed(Suppressed) { }

    virtual void diagnose(Sema &S, SourceLocation Loc, QualType T) = 0;
    virtual ~TypeDiagnoser() {}
  };

  static int getPrintable(int I) { return I; }
  static unsigned getPrintable(unsigned I) { return I; }
  static bool getPrintable(bool B) { return B; }
  static const char * getPrintable(const char *S) { return S; }
  static StringRef getPrintable(StringRef S) { return S; }
  static const std::string &getPrintable(const std::string &S) { return S; }
  static const IdentifierInfo *getPrintable(const IdentifierInfo *II) {
    return II;
  }
  static DeclarationName getPrintable(DeclarationName N) { return N; }
  static QualType getPrintable(QualType T) { return T; }
  static SourceRange getPrintable(SourceRange R) { return R; }
  static SourceRange getPrintable(SourceLocation L) { return L; }
  static SourceRange getPrintable(const Expr *E) { return E->getSourceRange(); }
  static SourceRange getPrintable(TypeLoc TL) { return TL.getSourceRange();}

  template <typename... Ts> class BoundTypeDiagnoser : public TypeDiagnoser {
    unsigned DiagID;
    std::tuple<const Ts &...> Args;

    template <std::size_t... Is>
    void emit(const SemaDiagnosticBuilder &DB,
              llvm::index_sequence<Is...>) const {
      // Apply all tuple elements to the builder in order.
      bool Dummy[] = {(DB << getPrintable(std::get<Is>(Args)))...};
      (void)Dummy;
    }

  public:
    BoundTypeDiagnoser(unsigned DiagID, const Ts &...Args)
        : TypeDiagnoser(DiagID == 0), DiagID(DiagID), Args(Args...) {}

    void diagnose(Sema &S, SourceLocation Loc, QualType T) override {
      if (Suppressed)
        return;
      const SemaDiagnosticBuilder &DB = S.Diag(Loc, DiagID);
      emit(DB, llvm::index_sequence_for<Ts...>());
      DB << T;
    }
  };

private:
  bool RequireCompleteTypeImpl(SourceLocation Loc, QualType T,
                           TypeDiagnoser &Diagnoser);

  VisibleModuleSet VisibleModules;
  llvm::SmallVector<VisibleModuleSet, 16> VisibleModulesStack;

  Module *CachedFakeTopLevelModule;

public:
  /// \brief Get the module owning an entity.
  Module *getOwningModule(Decl *Entity);

  /// \brief Make a merged definition of an existing hidden definition \p ND
  /// visible at the specified location.
  void makeMergedDefinitionVisible(NamedDecl *ND, SourceLocation Loc);

  bool isModuleVisible(Module *M) { return VisibleModules.isVisible(M); }

  /// Determine whether a declaration is visible to name lookup.
  bool isVisible(const NamedDecl *D) {
    return !D->isHidden() || isVisibleSlow(D);
  }
  bool hasVisibleMergedDefinition(NamedDecl *Def);

  /// Determine if \p D has a visible definition. If not, suggest a declaration
  /// that should be made visible to expose the definition.
  bool hasVisibleDefinition(NamedDecl *D, NamedDecl **Suggested,
                            bool OnlyNeedComplete = false);
  bool hasVisibleDefinition(const NamedDecl *D) {
    NamedDecl *Hidden;
    return hasVisibleDefinition(const_cast<NamedDecl*>(D), &Hidden);
  }

  /// Determine if the template parameter \p D has a visible default argument.
  bool
  hasVisibleDefaultArgument(const NamedDecl *D,
                            llvm::SmallVectorImpl<Module *> *Modules = nullptr);

  bool RequireCompleteType(SourceLocation Loc, QualType T,
                           TypeDiagnoser &Diagnoser);
  bool RequireCompleteType(SourceLocation Loc, QualType T,
                           unsigned DiagID);

  template <typename... Ts>
  bool RequireCompleteType(SourceLocation Loc, QualType T, unsigned DiagID,
                           const Ts &...Args) {
    BoundTypeDiagnoser<Ts...> Diagnoser(DiagID, Args...);
    return RequireCompleteType(Loc, T, Diagnoser);
  }

  bool RequireCompleteExprType(Expr *E, TypeDiagnoser &Diagnoser);
  bool RequireCompleteExprType(Expr *E, unsigned DiagID);

  template <typename... Ts>
  bool RequireCompleteExprType(Expr *E, unsigned DiagID, const Ts &...Args) {
    BoundTypeDiagnoser<Ts...> Diagnoser(DiagID, Args...);
    return RequireCompleteExprType(E, Diagnoser);
  }

  bool RequireLiteralType(SourceLocation Loc, QualType T,
                          TypeDiagnoser &Diagnoser);
  bool RequireLiteralType(SourceLocation Loc, QualType T, unsigned DiagID);

  template <typename... Ts>
  bool RequireLiteralType(SourceLocation Loc, QualType T, unsigned DiagID,
                          const Ts &...Args) {
    BoundTypeDiagnoser<Ts...> Diagnoser(DiagID, Args...);
    return RequireLiteralType(Loc, T, Diagnoser);
  }

  QualType getElaboratedType(ElaboratedTypeKeyword Keyword,
                             const CXXScopeSpec &SS, QualType T);

  QualType BuildTypeofExprType(Expr *E, SourceLocation Loc);
  /// If AsUnevaluated is false, E is treated as though it were an evaluated
  /// context, such as when building a type for decltype(auto).
  QualType BuildDecltypeType(Expr *E, SourceLocation Loc,
                             bool AsUnevaluated = true);
  QualType BuildUnaryTransformType(QualType BaseType,
                                   UnaryTransformType::UTTKind UKind,
                                   SourceLocation Loc);

  //===--------------------------------------------------------------------===//
  // Symbol table / Decl tracking callbacks: SemaDecl.cpp.
  //

  /// List of decls defined in a function prototype. This contains EnumConstants
  /// that incorrectly end up in translation unit scope because there is no
  /// function to pin them on. ActOnFunctionDeclarator reads this list and patches
  /// them into the FunctionDecl.
  std::vector<NamedDecl*> DeclsInPrototypeScope;

  DeclGroupPtrTy ConvertDeclToDeclGroup(Decl *Ptr, Decl *OwnedType = nullptr);

  void DiagnoseUseOfUnimplementedSelectors();

  bool isSimpleTypeSpecifier(tok::TokenKind Kind) const;

  ParsedType getTypeName(const IdentifierInfo &II, SourceLocation NameLoc,
                         Scope *S, CXXScopeSpec *SS = nullptr,
                         bool isClassName = false,
                         bool HasTrailingDot = false,
                         ParsedType ObjectType = ParsedType(),
                         bool IsCtorOrDtorName = false,
                         bool WantNontrivialTypeSourceInfo = false,
                         IdentifierInfo **CorrectedII = nullptr);
  TypeSpecifierType isTagName(IdentifierInfo &II, Scope *S);
  bool isMicrosoftMissingTypename(const CXXScopeSpec *SS, Scope *S);
  void DiagnoseUnknownTypeName(IdentifierInfo *&II,
                               SourceLocation IILoc,
                               Scope *S,
                               CXXScopeSpec *SS,
                               ParsedType &SuggestedType,
                               bool AllowClassTemplates = false);

  /// \brief For compatibility with MSVC, we delay parsing of some default
  /// template type arguments until instantiation time.  Emits a warning and
  /// returns a synthesized DependentNameType that isn't really dependent on any
  /// other template arguments.
  ParsedType ActOnDelayedDefaultTemplateArg(const IdentifierInfo &II,
                                            SourceLocation NameLoc);

  /// \brief Describes the result of the name lookup and resolution performed
  /// by \c ClassifyName().
  enum NameClassificationKind {
    NC_Unknown,
    NC_Error,
    NC_Keyword,
    NC_Type,
    NC_Expression,
    NC_NestedNameSpecifier,
    NC_TypeTemplate,
    NC_VarTemplate,
    NC_FunctionTemplate
  };

  class NameClassification {
    NameClassificationKind Kind;
    ExprResult Expr;
    TemplateName Template;
    ParsedType Type;

    explicit NameClassification(NameClassificationKind Kind) : Kind(Kind) {}

  public:
    NameClassification(ExprResult Expr) : Kind(NC_Expression), Expr(Expr) {}

    NameClassification(ParsedType Type) : Kind(NC_Type), Type(Type) {}

    NameClassification(const IdentifierInfo *)
      : Kind(NC_Keyword) {
    }

    static NameClassification Error() {
      return NameClassification(NC_Error);
    }

    static NameClassification Unknown() {
      return NameClassification(NC_Unknown);
    }

    static NameClassification NestedNameSpecifier() {
      return NameClassification(NC_NestedNameSpecifier);
    }

    static NameClassification TypeTemplate(TemplateName Name) {
      NameClassification Result(NC_TypeTemplate);
      Result.Template = Name;
      return Result;
    }

    static NameClassification VarTemplate(TemplateName Name) {
      NameClassification Result(NC_VarTemplate);
      Result.Template = Name;
      return Result;
    }

    static NameClassification FunctionTemplate(TemplateName Name) {
      NameClassification Result(NC_FunctionTemplate);
      Result.Template = Name;
      return Result;
    }

    NameClassificationKind getKind() const { return Kind; }

    ParsedType getType() const {
      assert(Kind == NC_Type);
      return Type;
    }

    ExprResult getExpression() const {
      assert(Kind == NC_Expression);
      return Expr;
    }

    TemplateName getTemplateName() const {
      assert(Kind == NC_TypeTemplate || Kind == NC_FunctionTemplate ||
             Kind == NC_VarTemplate);
      return Template;
    }

    TemplateNameKind getTemplateNameKind() const {
      switch (Kind) {
      case NC_TypeTemplate:
        return TNK_Type_template;
      case NC_FunctionTemplate:
        return TNK_Function_template;
      case NC_VarTemplate:
        return TNK_Var_template;
      default:
        llvm_unreachable("unsupported name classification.");
      }
    }
  };

  /// \brief Perform name lookup on the given name, classifying it based on
  /// the results of name lookup and the following token.
  ///
  /// This routine is used by the parser to resolve identifiers and help direct
  /// parsing. When the identifier cannot be found, this routine will attempt
  /// to correct the typo and classify based on the resulting name.
  ///
  /// \param S The scope in which we're performing name lookup.
  ///
  /// \param SS The nested-name-specifier that precedes the name.
  ///
  /// \param Name The identifier. If typo correction finds an alternative name,
  /// this pointer parameter will be updated accordingly.
  ///
  /// \param NameLoc The location of the identifier.
  ///
  /// \param NextToken The token following the identifier. Used to help
  /// disambiguate the name.
  ///
  /// \param IsAddressOfOperand True if this name is the operand of a unary
  ///        address of ('&') expression, assuming it is classified as an
  ///        expression.
  ///
  /// \param CCC The correction callback, if typo correction is desired.
  NameClassification
  ClassifyName(Scope *S, CXXScopeSpec &SS, IdentifierInfo *&Name,
               SourceLocation NameLoc, const Token &NextToken,
               bool IsAddressOfOperand,
               std::unique_ptr<CorrectionCandidateCallback> CCC = nullptr);

  Decl *ActOnDeclarator(Scope *S, Declarator &D);

  NamedDecl *HandleDeclarator(Scope *S, Declarator &D,
                              MultiTemplateParamsArg TemplateParameterLists);
  void RegisterLocallyScopedExternCDecl(NamedDecl *ND, Scope *S);
  bool DiagnoseClassNameShadow(DeclContext *DC, DeclarationNameInfo Info);
  bool diagnoseQualifiedDeclaration(CXXScopeSpec &SS, DeclContext *DC,
                                    DeclarationName Name,
                                    SourceLocation Loc);
  void
  diagnoseIgnoredQualifiers(unsigned DiagID, unsigned Quals,
                            SourceLocation FallbackLoc,
                            SourceLocation ConstQualLoc = SourceLocation(),
                            SourceLocation VolatileQualLoc = SourceLocation(),
                            SourceLocation RestrictQualLoc = SourceLocation(),
                            SourceLocation AtomicQualLoc = SourceLocation());

  static bool adjustContextForLocalExternDecl(DeclContext *&DC);
  void DiagnoseFunctionSpecifiers(const DeclSpec &DS);
  void CheckShadow(Scope *S, VarDecl *D, const LookupResult& R);
  void CheckShadow(Scope *S, VarDecl *D);
  void CheckCastAlign(Expr *Op, QualType T, SourceRange TRange);
  void handleTagNumbering(const TagDecl *Tag, Scope *TagScope);
  void setTagNameForLinkagePurposes(TagDecl *TagFromDeclSpec,
                                    TypedefNameDecl *NewTD);
  void CheckTypedefForVariablyModifiedType(Scope *S, TypedefNameDecl *D);
  NamedDecl* ActOnTypedefDeclarator(Scope* S, Declarator& D, DeclContext* DC,
                                    TypeSourceInfo *TInfo,
                                    LookupResult &Previous);
  NamedDecl* ActOnTypedefNameDecl(Scope* S, DeclContext* DC, TypedefNameDecl *D,
                                  LookupResult &Previous, bool &Redeclaration);
  // HLSL Change Starts
  // This enumeration is used to determine whether a variable declaration
  // should shadow a prior declaration rather than merging.
  enum ShadowMergeState {
    ShadowMergeState_Disallowed,  // shadowing is not allowed
    ShadowMergeState_Possible,    // shadowing is possible (but may not occur)
    ShadowMergeState_Effective    // the declaration should shadow a prior one
  };
  // HLSL Change Ends
  NamedDecl *ActOnVariableDeclarator(Scope *S, Declarator &D, DeclContext *DC,
                                     TypeSourceInfo *TInfo,
                                     LookupResult &Previous,
                                     MultiTemplateParamsArg TemplateParamLists,
                                     bool &AddToScope,
                                     ShadowMergeState MergeState = ShadowMergeState_Disallowed); // HLSL Change - add merge state
  // Returns true if the variable declaration is a redeclaration
  bool CheckVariableDeclaration(VarDecl *NewVD, LookupResult &Previous, ShadowMergeState MergeState = ShadowMergeState_Disallowed); // HLSL Change - add merge state
  void CheckVariableDeclarationType(VarDecl *NewVD);
  void CheckCompleteVariableDeclaration(VarDecl *var);
  void MaybeSuggestAddingStaticToDecl(const FunctionDecl *D);

  NamedDecl* ActOnFunctionDeclarator(Scope* S, Declarator& D, DeclContext* DC,
                                     TypeSourceInfo *TInfo,
                                     LookupResult &Previous,
                                     MultiTemplateParamsArg TemplateParamLists,
                                     bool &AddToScope);
  bool AddOverriddenMethods(CXXRecordDecl *DC, CXXMethodDecl *MD);

  bool CheckConstexprFunctionDecl(const FunctionDecl *FD);
  bool CheckConstexprFunctionBody(const FunctionDecl *FD, Stmt *Body);

  void DiagnoseHiddenVirtualMethods(CXXMethodDecl *MD);
  void FindHiddenVirtualMethods(CXXMethodDecl *MD,
                          SmallVectorImpl<CXXMethodDecl*> &OverloadedMethods);
  void NoteHiddenVirtualMethods(CXXMethodDecl *MD,
                          SmallVectorImpl<CXXMethodDecl*> &OverloadedMethods);
  // Returns true if the function declaration is a redeclaration
  bool CheckFunctionDeclaration(Scope *S,
                                FunctionDecl *NewFD, LookupResult &Previous,
                                bool IsExplicitSpecialization);
  void CheckMain(FunctionDecl *FD, const DeclSpec &D);
  void CheckMSVCRTEntryPoint(FunctionDecl *FD);
  Decl *ActOnParamDeclarator(Scope *S, Declarator &D);
  ParmVarDecl *BuildParmVarDeclForTypedef(DeclContext *DC,
                                          SourceLocation Loc,
                                          QualType T);
  ParmVarDecl *CheckParameter(DeclContext *DC, SourceLocation StartLoc,
                              SourceLocation NameLoc, IdentifierInfo *Name,
                              QualType T, TypeSourceInfo *TSInfo,
                              StorageClass SCm, hlsl::ParameterModifier ParamMod); // HLSL Change
  void ActOnParamDefaultArgument(Decl *param,
                                 SourceLocation EqualLoc,
                                 Expr *defarg);
  void ActOnParamUnparsedDefaultArgument(Decl *param,
                                         SourceLocation EqualLoc,
                                         SourceLocation ArgLoc);
  void ActOnParamDefaultArgumentError(Decl *param, SourceLocation EqualLoc);
  bool SetParamDefaultArgument(ParmVarDecl *Param, Expr *DefaultArg,
                               SourceLocation EqualLoc);

  void AddInitializerToDecl(Decl *dcl, Expr *init, bool DirectInit,
                            bool TypeMayContainAuto);
  void ActOnUninitializedDecl(Decl *dcl, bool TypeMayContainAuto);
  void ActOnInitializerError(Decl *Dcl);
  void ActOnPureSpecifier(Decl *D, SourceLocation PureSpecLoc);
  void ActOnCXXForRangeDecl(Decl *D);
  StmtResult ActOnCXXForRangeIdentifier(Scope *S, SourceLocation IdentLoc,
                                        IdentifierInfo *Ident,
                                        ParsedAttributes &Attrs,
                                        SourceLocation AttrEnd);
  void SetDeclDeleted(Decl *dcl, SourceLocation DelLoc);
  void SetDeclDefaulted(Decl *dcl, SourceLocation DefaultLoc);
  void FinalizeDeclaration(Decl *D);
  DeclGroupPtrTy FinalizeDeclaratorGroup(Scope *S, const DeclSpec &DS,
                                         ArrayRef<Decl *> Group);
  DeclGroupPtrTy BuildDeclaratorGroup(MutableArrayRef<Decl *> Group,
                                      bool TypeMayContainAuto = true);

  /// Should be called on all declarations that might have attached
  /// documentation comments.
  void ActOnDocumentableDecl(Decl *D);
  void ActOnDocumentableDecls(ArrayRef<Decl *> Group);

  void ActOnFinishKNRParamDeclarations(Scope *S, Declarator &D,
                                       SourceLocation LocAfterDecls);
  void CheckForFunctionRedefinition(FunctionDecl *FD,
                                    const FunctionDecl *EffectiveDefinition =
                                        nullptr);
  Decl *ActOnStartOfFunctionDef(Scope *S, Declarator &D);
  Decl *ActOnStartOfFunctionDef(Scope *S, Decl *D);
  void ActOnStartOfObjCMethodDef(Scope *S, Decl *D);
  bool isObjCMethodDecl(Decl *D) {
    return D && isa<ObjCMethodDecl>(D);
  }

  /// \brief Determine whether we can delay parsing the body of a function or
  /// function template until it is used, assuming we don't care about emitting
  /// code for that function.
  ///
  /// This will be \c false if we may need the body of the function in the
  /// middle of parsing an expression (where it's impractical to switch to
  /// parsing a different function), for instance, if it's constexpr in C++11
  /// or has an 'auto' return type in C++14. These cases are essentially bugs.
  bool canDelayFunctionBody(const Declarator &D);

  /// \brief Determine whether we can skip parsing the body of a function
  /// definition, assuming we don't care about analyzing its body or emitting
  /// code for that function.
  ///
  /// This will be \c false only if we may need the body of the function in
  /// order to parse the rest of the program (for instance, if it is
  /// \c constexpr in C++11 or has an 'auto' return type in C++14).
  bool canSkipFunctionBody(Decl *D);

  void computeNRVO(Stmt *Body, sema::FunctionScopeInfo *Scope);
  Decl *ActOnFinishFunctionBody(Decl *Decl, Stmt *Body);
  Decl *ActOnFinishFunctionBody(Decl *Decl, Stmt *Body, bool IsInstantiation);
  Decl *ActOnSkippedFunctionBody(Decl *Decl);
  void ActOnFinishInlineMethodDef(CXXMethodDecl *D);

  /// ActOnFinishDelayedAttribute - Invoked when we have finished parsing an
  /// attribute for which parsing is delayed.
  void ActOnFinishDelayedAttribute(Scope *S, Decl *D, ParsedAttributes &Attrs);

  /// \brief Diagnose any unused parameters in the given sequence of
  /// ParmVarDecl pointers.
  void DiagnoseUnusedParameters(ParmVarDecl * const *Begin,
                                ParmVarDecl * const *End);

  /// \brief Diagnose whether the size of parameters or return value of a
  /// function or obj-c method definition is pass-by-value and larger than a
  /// specified threshold.
  void DiagnoseSizeOfParametersAndReturnValue(ParmVarDecl * const *Begin,
                                              ParmVarDecl * const *End,
                                              QualType ReturnTy,
                                              NamedDecl *D);

  void DiagnoseInvalidJumps(Stmt *Body);
  Decl *ActOnFileScopeAsmDecl(Expr *expr,
                              SourceLocation AsmLoc,
                              SourceLocation RParenLoc);

  /// \brief Handle a C++11 empty-declaration and attribute-declaration.
  Decl *ActOnEmptyDeclaration(Scope *S,
                              AttributeList *AttrList,
                              SourceLocation SemiLoc);

  /// \brief The parser has processed a module import declaration.
  ///
  /// \param AtLoc The location of the '@' symbol, if any.
  ///
  /// \param ImportLoc The location of the 'import' keyword.
  ///
  /// \param Path The module access path.
  DeclResult ActOnModuleImport(SourceLocation AtLoc, SourceLocation ImportLoc,
                               ModuleIdPath Path);

  /// \brief The parser has processed a module import translated from a
  /// #include or similar preprocessing directive.
  void ActOnModuleInclude(SourceLocation DirectiveLoc, Module *Mod);

  /// \brief The parsed has entered a submodule.
  void ActOnModuleBegin(SourceLocation DirectiveLoc, Module *Mod);
  /// \brief The parser has left a submodule.
  void ActOnModuleEnd(SourceLocation DirectiveLoc, Module *Mod);

  /// \brief Create an implicit import of the given module at the given
  /// source location, for error recovery, if possible.
  ///
  /// This routine is typically used when an entity found by name lookup
  /// is actually hidden within a module that we know about but the user
  /// has forgotten to import.
  void createImplicitModuleImportForErrorRecovery(SourceLocation Loc,
                                                  Module *Mod);

  /// Kinds of missing import. Note, the values of these enumerators correspond
  /// to %select values in diagnostics.
  enum class MissingImportKind {
    Declaration,
    Definition,
    DefaultArgument
  };

  /// \brief Diagnose that the specified declaration needs to be visible but
  /// isn't, and suggest a module import that would resolve the problem.
  void diagnoseMissingImport(SourceLocation Loc, NamedDecl *Decl,
                             bool NeedDefinition, bool Recover = true);
  void diagnoseMissingImport(SourceLocation Loc, NamedDecl *Decl,
                             SourceLocation DeclLoc, ArrayRef<Module *> Modules,
                             MissingImportKind MIK, bool Recover);

  /// \brief Retrieve a suitable printing policy.
  PrintingPolicy getPrintingPolicy() const {
    return getPrintingPolicy(Context, PP);
  }

  /// \brief Retrieve a suitable printing policy.
  static PrintingPolicy getPrintingPolicy(const ASTContext &Ctx,
                                          const Preprocessor &PP);

  /// Scope actions.
  void ActOnPopScope(SourceLocation Loc, Scope *S);
  void ActOnTranslationUnitScope(Scope *S);

  Decl *ParsedFreeStandingDeclSpec(Scope *S, AccessSpecifier AS,
                                   DeclSpec &DS);
  Decl *ParsedFreeStandingDeclSpec(Scope *S, AccessSpecifier AS,
                                   DeclSpec &DS,
                                   MultiTemplateParamsArg TemplateParams,
                                   bool IsExplicitInstantiation = false);

  Decl *BuildAnonymousStructOrUnion(Scope *S, DeclSpec &DS,
                                    AccessSpecifier AS,
                                    RecordDecl *Record,
                                    const PrintingPolicy &Policy);

  Decl *BuildMicrosoftCAnonymousStruct(Scope *S, DeclSpec &DS,
                                       RecordDecl *Record);

  bool isAcceptableTagRedeclaration(const TagDecl *Previous,
                                    TagTypeKind NewTag, bool isDefinition,
                                    SourceLocation NewTagLoc,
                                    const IdentifierInfo *Name);

  enum TagUseKind {
    TUK_Reference,   // Reference to a tag:  'struct foo *X;'
    TUK_Declaration, // Fwd decl of a tag:   'struct foo;'
    TUK_Definition,  // Definition of a tag: 'struct foo { int X; } Y;'
    TUK_Friend       // Friend declaration:  'friend struct foo;'
  };

  struct SkipBodyInfo {
    SkipBodyInfo() : ShouldSkip(false), Previous(nullptr) {}
    bool ShouldSkip;
    NamedDecl *Previous;
  };

  Decl *ActOnTag(Scope *S, unsigned TagSpec, TagUseKind TUK,
                 SourceLocation KWLoc, CXXScopeSpec &SS,
                 IdentifierInfo *Name, SourceLocation NameLoc,
                 AttributeList *Attr, AccessSpecifier AS,
                 SourceLocation ModulePrivateLoc,
                 MultiTemplateParamsArg TemplateParameterLists,
                 bool &OwnedDecl, bool &IsDependent,
                 SourceLocation ScopedEnumKWLoc,
                 bool ScopedEnumUsesClassTag, TypeResult UnderlyingType,
                 bool IsTypeSpecifier, SkipBodyInfo *SkipBody = nullptr);

  Decl *ActOnTemplatedFriendTag(Scope *S, SourceLocation FriendLoc,
                                unsigned TagSpec, SourceLocation TagLoc,
                                CXXScopeSpec &SS,
                                IdentifierInfo *Name, SourceLocation NameLoc,
                                AttributeList *Attr,
                                MultiTemplateParamsArg TempParamLists);

  TypeResult ActOnDependentTag(Scope *S,
                               unsigned TagSpec,
                               TagUseKind TUK,
                               const CXXScopeSpec &SS,
                               IdentifierInfo *Name,
                               SourceLocation TagLoc,
                               SourceLocation NameLoc);

  void ActOnDefs(Scope *S, Decl *TagD, SourceLocation DeclStart,
                 IdentifierInfo *ClassName,
                 SmallVectorImpl<Decl *> &Decls);
  Decl *ActOnField(Scope *S, Decl *TagD, SourceLocation DeclStart,
                   Declarator &D, Expr *BitfieldWidth);

  FieldDecl *HandleField(Scope *S, RecordDecl *TagD, SourceLocation DeclStart,
                         Declarator &D, Expr *BitfieldWidth,
                         InClassInitStyle InitStyle,
                         AccessSpecifier AS);
  MSPropertyDecl *HandleMSProperty(Scope *S, RecordDecl *TagD,
                                   SourceLocation DeclStart,
                                   Declarator &D, Expr *BitfieldWidth,
                                   InClassInitStyle InitStyle,
                                   AccessSpecifier AS,
                                   AttributeList *MSPropertyAttr);

  FieldDecl *CheckFieldDecl(DeclarationName Name, QualType T,
                            TypeSourceInfo *TInfo,
                            RecordDecl *Record, SourceLocation Loc,
                            bool Mutable, Expr *BitfieldWidth,
                            InClassInitStyle InitStyle,
                            SourceLocation TSSL,
                            AccessSpecifier AS, NamedDecl *PrevDecl,
                            Declarator *D = nullptr);

  bool CheckNontrivialField(FieldDecl *FD);
  void DiagnoseNontrivial(const CXXRecordDecl *Record, CXXSpecialMember CSM);
  bool SpecialMemberIsTrivial(CXXMethodDecl *MD, CXXSpecialMember CSM,
                              bool Diagnose = false);
  CXXSpecialMember getSpecialMember(const CXXMethodDecl *MD);
  void ActOnLastBitfield(SourceLocation DeclStart,
                         SmallVectorImpl<Decl *> &AllIvarDecls);
  Decl *ActOnIvar(Scope *S, SourceLocation DeclStart,
                  Declarator &D, Expr *BitfieldWidth,
                  tok::ObjCKeywordKind visibility);

  // This is used for both record definitions and ObjC interface declarations.
  void ActOnFields(Scope* S, SourceLocation RecLoc, Decl *TagDecl,
                   ArrayRef<Decl *> Fields,
                   SourceLocation LBrac, SourceLocation RBrac,
                   AttributeList *AttrList);

  /// ActOnTagStartDefinition - Invoked when we have entered the
  /// scope of a tag's definition (e.g., for an enumeration, class,
  /// struct, or union).
  void ActOnTagStartDefinition(Scope *S, Decl *TagDecl);

  typedef void *SkippedDefinitionContext;

  /// \brief Invoked when we enter a tag definition that we're skipping.
  SkippedDefinitionContext ActOnTagStartSkippedDefinition(Scope *S, Decl *TD);

  Decl *ActOnObjCContainerStartDefinition(Decl *IDecl);

  /// ActOnStartCXXMemberDeclarations - Invoked when we have parsed a
  /// C++ record definition's base-specifiers clause and are starting its
  /// member declarations.
  void ActOnStartCXXMemberDeclarations(Scope *S, Decl *TagDecl,
                                       SourceLocation FinalLoc,
                                       bool IsFinalSpelledSealed,
                                       SourceLocation LBraceLoc);

  /// ActOnTagFinishDefinition - Invoked once we have finished parsing
  /// the definition of a tag (enumeration, class, struct, or union).
  void ActOnTagFinishDefinition(Scope *S, Decl *TagDecl,
                                SourceLocation RBraceLoc);

  void ActOnTagFinishSkippedDefinition(SkippedDefinitionContext Context);

  void ActOnObjCContainerFinishDefinition();

  /// \brief Invoked when we must temporarily exit the objective-c container
  /// scope for parsing/looking-up C constructs.
  ///
  /// Must be followed by a call to \see ActOnObjCReenterContainerContext
  void ActOnObjCTemporaryExitContainerContext(DeclContext *DC);
  void ActOnObjCReenterContainerContext(DeclContext *DC);

  /// ActOnTagDefinitionError - Invoked when there was an unrecoverable
  /// error parsing the definition of a tag.
  void ActOnTagDefinitionError(Scope *S, Decl *TagDecl);

  EnumConstantDecl *CheckEnumConstant(EnumDecl *Enum,
                                      EnumConstantDecl *LastEnumConst,
                                      SourceLocation IdLoc,
                                      IdentifierInfo *Id,
                                      Expr *val);
  bool CheckEnumUnderlyingType(TypeSourceInfo *TI);
  bool CheckEnumRedeclaration(SourceLocation EnumLoc, bool IsScoped,
                              QualType EnumUnderlyingTy, const EnumDecl *Prev);

  /// Determine whether the body of an anonymous enumeration should be skipped.
  /// \param II The name of the first enumerator.
  SkipBodyInfo shouldSkipAnonEnumBody(Scope *S, IdentifierInfo *II,
                                      SourceLocation IILoc);

  Decl *ActOnEnumConstant(Scope *S, Decl *EnumDecl, Decl *LastEnumConstant,
                          SourceLocation IdLoc, IdentifierInfo *Id,
                          AttributeList *Attrs,
                          SourceLocation EqualLoc, Expr *Val);
  void ActOnEnumBody(SourceLocation EnumLoc, SourceLocation LBraceLoc,
                     SourceLocation RBraceLoc, Decl *EnumDecl,
                     ArrayRef<Decl *> Elements,
                     Scope *S, AttributeList *Attr);

  DeclContext *getContainingDC(DeclContext *DC);

  /// Set the current declaration context until it gets popped.
  void PushDeclContext(Scope *S, DeclContext *DC);
  void PopDeclContext();

  /// EnterDeclaratorContext - Used when we must lookup names in the context
  /// of a declarator's nested name specifier.
  void EnterDeclaratorContext(Scope *S, DeclContext *DC);
  void ExitDeclaratorContext(Scope *S);

  /// Push the parameters of D, which must be a function, into scope.
  void ActOnReenterFunctionContext(Scope* S, Decl* D);
  void ActOnExitFunctionContext();

  DeclContext *getFunctionLevelDeclContext();

  /// getCurFunctionDecl - If inside of a function body, this returns a pointer
  /// to the function decl for the function being parsed.  If we're currently
  /// in a 'block', this returns the containing context.
  FunctionDecl *getCurFunctionDecl();

  /// getCurMethodDecl - If inside of a method body, this returns a pointer to
  /// the method decl for the method being parsed.  If we're currently
  /// in a 'block', this returns the containing context.
  ObjCMethodDecl *getCurMethodDecl();

  /// getCurFunctionOrMethodDecl - Return the Decl for the current ObjC method
  /// or C function we're in, otherwise return null.  If we're currently
  /// in a 'block', this returns the containing context.
  NamedDecl *getCurFunctionOrMethodDecl();

  /// Add this decl to the scope shadowed decl chains.
  void PushOnScopeChains(NamedDecl *D, Scope *S, bool AddToContext = true);

  /// \brief Make the given externally-produced declaration visible at the
  /// top level scope.
  ///
  /// \param D The externally-produced declaration to push.
  ///
  /// \param Name The name of the externally-produced declaration.
  void pushExternalDeclIntoScope(NamedDecl *D, DeclarationName Name);

  /// isDeclInScope - If 'Ctx' is a function/method, isDeclInScope returns true
  /// if 'D' is in Scope 'S', otherwise 'S' is ignored and isDeclInScope returns
  /// true if 'D' belongs to the given declaration context.
  ///
  /// \param AllowInlineNamespace If \c true, allow the declaration to be in the
  ///        enclosing namespace set of the context, rather than contained
  ///        directly within it.
  bool isDeclInScope(NamedDecl *D, DeclContext *Ctx, Scope *S = nullptr,
                     bool AllowInlineNamespace = false);

  /// Finds the scope corresponding to the given decl context, if it
  /// happens to be an enclosing scope.  Otherwise return NULL.
  static Scope *getScopeForDeclContext(Scope *S, DeclContext *DC);

  /// Subroutines of ActOnDeclarator().
  TypedefDecl *ParseTypedefDecl(Scope *S, Declarator &D, QualType T,
                                TypeSourceInfo *TInfo);
  bool isIncompatibleTypedef(TypeDecl *Old, TypedefNameDecl *New);

  /// Attribute merging methods. Return true if a new attribute was added.
  AvailabilityAttr *mergeAvailabilityAttr(NamedDecl *D, SourceRange Range,
                                          IdentifierInfo *Platform,
                                          VersionTuple Introduced,
                                          VersionTuple Deprecated,
                                          VersionTuple Obsoleted,
                                          bool IsUnavailable,
                                          StringRef Message,
                                          bool Override,
                                          unsigned AttrSpellingListIndex);
  TypeVisibilityAttr *mergeTypeVisibilityAttr(Decl *D, SourceRange Range,
                                       TypeVisibilityAttr::VisibilityType Vis,
                                              unsigned AttrSpellingListIndex);
  VisibilityAttr *mergeVisibilityAttr(Decl *D, SourceRange Range,
                                      VisibilityAttr::VisibilityType Vis,
                                      unsigned AttrSpellingListIndex);
  DLLImportAttr *mergeDLLImportAttr(Decl *D, SourceRange Range,
                                    unsigned AttrSpellingListIndex);
  DLLExportAttr *mergeDLLExportAttr(Decl *D, SourceRange Range,
                                    unsigned AttrSpellingListIndex);
  MSInheritanceAttr *
  mergeMSInheritanceAttr(Decl *D, SourceRange Range, bool BestCase,
                         unsigned AttrSpellingListIndex,
                         MSInheritanceAttr::Spelling SemanticSpelling);
  FormatAttr *mergeFormatAttr(Decl *D, SourceRange Range,
                              IdentifierInfo *Format, int FormatIdx,
                              int FirstArg, unsigned AttrSpellingListIndex);
  SectionAttr *mergeSectionAttr(Decl *D, SourceRange Range, StringRef Name,
                                unsigned AttrSpellingListIndex);
  AlwaysInlineAttr *mergeAlwaysInlineAttr(Decl *D, SourceRange Range,
                                          IdentifierInfo *Ident,
                                          unsigned AttrSpellingListIndex);
  MinSizeAttr *mergeMinSizeAttr(Decl *D, SourceRange Range,
                                unsigned AttrSpellingListIndex);
  OptimizeNoneAttr *mergeOptimizeNoneAttr(Decl *D, SourceRange Range,
                                          unsigned AttrSpellingListIndex);

  /// \brief Describes the kind of merge to perform for availability
  /// attributes (including "deprecated", "unavailable", and "availability").
  enum AvailabilityMergeKind {
    /// \brief Don't merge availability attributes at all.
    AMK_None,
    /// \brief Merge availability attributes for a redeclaration, which requires
    /// an exact match.
    AMK_Redeclaration,
    /// \brief Merge availability attributes for an override, which requires
    /// an exact match or a weakening of constraints.
    AMK_Override
  };

  void mergeDeclAttributes(NamedDecl *New, Decl *Old,
                           AvailabilityMergeKind AMK = AMK_Redeclaration);
  void MergeTypedefNameDecl(TypedefNameDecl *New, LookupResult &OldDecls);
  bool MergeFunctionDecl(FunctionDecl *New, NamedDecl *&Old, Scope *S,
                         bool MergeTypeWithOld);
  bool MergeCompatibleFunctionDecls(FunctionDecl *New, FunctionDecl *Old,
                                    Scope *S, bool MergeTypeWithOld);
  void mergeObjCMethodDecls(ObjCMethodDecl *New, ObjCMethodDecl *Old);
  void MergeVarDecl(VarDecl *New, LookupResult &Previous, ShadowMergeState& MergeState); // HLSL Change - add merge state
  void MergeVarDeclTypes(VarDecl *New, VarDecl *Old, bool MergeTypeWithOld, ShadowMergeState& MergeState); // HLSL Change - add merge state
  void MergeVarDeclExceptionSpecs(VarDecl *New, VarDecl *Old);
  bool MergeCXXFunctionDecl(FunctionDecl *New, FunctionDecl *Old, Scope *S);

  // AssignmentAction - This is used by all the assignment diagnostic functions
  // to represent what is actually causing the operation
  enum AssignmentAction {
    AA_Assigning,
    AA_Passing,
    AA_Returning,
    AA_Converting,
    AA_Initializing,
    AA_Sending,
    AA_Casting,
    AA_Passing_CFAudited
  };

  /// C++ Overloading.
  enum OverloadKind {
    /// This is a legitimate overload: the existing declarations are
    /// functions or function templates with different signatures.
    Ovl_Overload,

    /// This is not an overload because the signature exactly matches
    /// an existing declaration.
    Ovl_Match,

    /// This is not an overload because the lookup results contain a
    /// non-function.
    Ovl_NonFunction
  };
  OverloadKind CheckOverload(Scope *S,
                             FunctionDecl *New,
                             const LookupResult &OldDecls,
                             NamedDecl *&OldDecl,
                             bool IsForUsingDecl);
  bool IsOverload(FunctionDecl *New, FunctionDecl *Old, bool IsForUsingDecl);

  /// \brief Checks availability of the function depending on the current
  /// function context.Inside an unavailable function,unavailability is ignored.
  ///
  /// \returns true if \p FD is unavailable and current context is inside
  /// an available function, false otherwise.
  bool isFunctionConsideredUnavailable(FunctionDecl *FD);

  ImplicitConversionSequence
  TryImplicitConversion(Expr *From, QualType ToType,
                        bool SuppressUserConversions,
                        bool AllowExplicit,
                        bool InOverloadResolution,
                        bool CStyle,
                        bool AllowObjCWritebackConversion);

  bool IsIntegralPromotion(Expr *From, QualType FromType, QualType ToType);
  bool IsFloatingPointPromotion(QualType FromType, QualType ToType);
  bool IsComplexPromotion(QualType FromType, QualType ToType);
  bool IsPointerConversion(Expr *From, QualType FromType, QualType ToType,
                           bool InOverloadResolution,
                           QualType& ConvertedType, bool &IncompatibleObjC);
  bool isObjCPointerConversion(QualType FromType, QualType ToType,
                               QualType& ConvertedType, bool &IncompatibleObjC);
  bool isObjCWritebackConversion(QualType FromType, QualType ToType,
                                 QualType &ConvertedType);
  bool IsBlockPointerConversion(QualType FromType, QualType ToType,
                                QualType& ConvertedType);
  bool FunctionParamTypesAreEqual(const FunctionProtoType *OldType,
                                  const FunctionProtoType *NewType,
                                  unsigned *ArgPos = nullptr);
  void HandleFunctionTypeMismatch(PartialDiagnostic &PDiag,
                                  QualType FromType, QualType ToType);

  void maybeExtendBlockObject(ExprResult &E);
  CastKind PrepareCastToObjCObjectPointer(ExprResult &E);
  bool CheckPointerConversion(Expr *From, QualType ToType,
                              CastKind &Kind,
                              CXXCastPath& BasePath,
                              bool IgnoreBaseAccess);
  bool IsMemberPointerConversion(Expr *From, QualType FromType, QualType ToType,
                                 bool InOverloadResolution,
                                 QualType &ConvertedType);
  bool CheckMemberPointerConversion(Expr *From, QualType ToType,
                                    CastKind &Kind,
                                    CXXCastPath &BasePath,
                                    bool IgnoreBaseAccess);
  bool IsQualificationConversion(QualType FromType, QualType ToType,
                                 bool CStyle, bool &ObjCLifetimeConversion);
  bool IsNoReturnConversion(QualType FromType, QualType ToType,
                            QualType &ResultTy);
  bool DiagnoseMultipleUserDefinedConversion(Expr *From, QualType ToType);
  bool isSameOrCompatibleFunctionType(CanQualType Param, CanQualType Arg);

  ExprResult PerformMoveOrCopyInitialization(const InitializedEntity &Entity,
                                             const VarDecl *NRVOCandidate,
                                             QualType ResultType,
                                             Expr *Value,
                                             bool AllowNRVO = true);

  bool CanPerformCopyInitialization(const InitializedEntity &Entity,
                                    ExprResult Init);
  ExprResult PerformCopyInitialization(const InitializedEntity &Entity,
                                       SourceLocation EqualLoc,
                                       ExprResult Init,
                                       bool TopLevelOfInitList = false,
                                       bool AllowExplicit = false);
  ExprResult PerformObjectArgumentInitialization(Expr *From,
                                                 NestedNameSpecifier *Qualifier,
                                                 NamedDecl *FoundDecl,
                                                 CXXMethodDecl *Method);

  ExprResult PerformContextuallyConvertToBool(Expr *From);
  ExprResult PerformContextuallyConvertToObjCPointer(Expr *From);

  /// Contexts in which a converted constant expression is required.
  enum CCEKind {
    CCEK_CaseValue,   ///< Expression in a case label.
    CCEK_Enumerator,  ///< Enumerator value with fixed underlying type.
    CCEK_TemplateArg, ///< Value of a non-type template parameter.
    CCEK_NewExpr      ///< Constant expression in a noptr-new-declarator.
  };
  ExprResult CheckConvertedConstantExpression(Expr *From, QualType T,
                                              llvm::APSInt &Value, CCEKind CCE);
  ExprResult CheckConvertedConstantExpression(Expr *From, QualType T,
                                              APValue &Value, CCEKind CCE);

  /// \brief Abstract base class used to perform a contextual implicit
  /// conversion from an expression to any type passing a filter.
  class ContextualImplicitConverter {
  public:
    bool Suppress;
    bool SuppressConversion;

    ContextualImplicitConverter(bool Suppress = false,
                                bool SuppressConversion = false)
        : Suppress(Suppress), SuppressConversion(SuppressConversion) {}

    /// \brief Determine whether the specified type is a valid destination type
    /// for this conversion.
    virtual bool match(QualType T) = 0;

    /// \brief Emits a diagnostic complaining that the expression does not have
    /// integral or enumeration type.
    virtual SemaDiagnosticBuilder
    diagnoseNoMatch(Sema &S, SourceLocation Loc, QualType T) = 0;

    /// \brief Emits a diagnostic when the expression has incomplete class type.
    virtual SemaDiagnosticBuilder
    diagnoseIncomplete(Sema &S, SourceLocation Loc, QualType T) = 0;

    /// \brief Emits a diagnostic when the only matching conversion function
    /// is explicit.
    virtual SemaDiagnosticBuilder diagnoseExplicitConv(
        Sema &S, SourceLocation Loc, QualType T, QualType ConvTy) = 0;

    /// \brief Emits a note for the explicit conversion function.
    virtual SemaDiagnosticBuilder
    noteExplicitConv(Sema &S, CXXConversionDecl *Conv, QualType ConvTy) = 0;

    /// \brief Emits a diagnostic when there are multiple possible conversion
    /// functions.
    virtual SemaDiagnosticBuilder
    diagnoseAmbiguous(Sema &S, SourceLocation Loc, QualType T) = 0;

    /// \brief Emits a note for one of the candidate conversions.
    virtual SemaDiagnosticBuilder
    noteAmbiguous(Sema &S, CXXConversionDecl *Conv, QualType ConvTy) = 0;

    /// \brief Emits a diagnostic when we picked a conversion function
    /// (for cases when we are not allowed to pick a conversion function).
    virtual SemaDiagnosticBuilder diagnoseConversion(
        Sema &S, SourceLocation Loc, QualType T, QualType ConvTy) = 0;

    virtual ~ContextualImplicitConverter() {}
  };

  class ICEConvertDiagnoser : public ContextualImplicitConverter {
    bool AllowScopedEnumerations;

  public:
    ICEConvertDiagnoser(bool AllowScopedEnumerations,
                        bool Suppress, bool SuppressConversion)
        : ContextualImplicitConverter(Suppress, SuppressConversion),
          AllowScopedEnumerations(AllowScopedEnumerations) {}

    /// Match an integral or (possibly scoped) enumeration type.
    bool match(QualType T) override;

    SemaDiagnosticBuilder
    diagnoseNoMatch(Sema &S, SourceLocation Loc, QualType T) override {
      return diagnoseNotInt(S, Loc, T);
    }

    /// \brief Emits a diagnostic complaining that the expression does not have
    /// integral or enumeration type.
    virtual SemaDiagnosticBuilder
    diagnoseNotInt(Sema &S, SourceLocation Loc, QualType T) = 0;
  };

  /// Perform a contextual implicit conversion.
  ExprResult PerformContextualImplicitConversion(
      SourceLocation Loc, Expr *FromE, ContextualImplicitConverter &Converter);


  enum ObjCSubscriptKind {
    OS_Array,
    OS_Dictionary,
    OS_Error
  };
  ObjCSubscriptKind CheckSubscriptingKind(Expr *FromE);

  // Note that LK_String is intentionally after the other literals, as
  // this is used for diagnostics logic.
  enum ObjCLiteralKind {
    LK_Array,
    LK_Dictionary,
    LK_Numeric,
    LK_Boxed,
    LK_String,
    LK_Block,
    LK_None
  };
  ObjCLiteralKind CheckLiteralKind(Expr *FromE);

  ExprResult PerformObjectMemberConversion(Expr *From,
                                           NestedNameSpecifier *Qualifier,
                                           NamedDecl *FoundDecl,
                                           NamedDecl *Member);

  // Members have to be NamespaceDecl* or TranslationUnitDecl*.
  // TODO: make this is a typesafe union.
  typedef llvm::SmallPtrSet<DeclContext   *, 16> AssociatedNamespaceSet;
  typedef llvm::SmallPtrSet<CXXRecordDecl *, 16> AssociatedClassSet;

  void AddOverloadCandidate(FunctionDecl *Function,
                            DeclAccessPair FoundDecl,
                            ArrayRef<Expr *> Args,
                            OverloadCandidateSet& CandidateSet,
                            bool SuppressUserConversions = false,
                            bool PartialOverloading = false,
                            bool AllowExplicit = false);
  void AddFunctionCandidates(const UnresolvedSetImpl &Functions,
                      ArrayRef<Expr *> Args,
                      OverloadCandidateSet &CandidateSet,
                      TemplateArgumentListInfo *ExplicitTemplateArgs = nullptr,
                      bool SuppressUserConversions = false,
                      bool PartialOverloading = false);
  void AddMethodCandidate(DeclAccessPair FoundDecl,
                          QualType ObjectType,
                          Expr::Classification ObjectClassification,
                          ArrayRef<Expr *> Args,
                          OverloadCandidateSet& CandidateSet,
                          bool SuppressUserConversion = false);
  void AddMethodCandidate(CXXMethodDecl *Method,
                          DeclAccessPair FoundDecl,
                          CXXRecordDecl *ActingContext, QualType ObjectType,
                          Expr::Classification ObjectClassification,
                          ArrayRef<Expr *> Args,
                          OverloadCandidateSet& CandidateSet,
                          bool SuppressUserConversions = false,
                          bool PartialOverloading = false);
  void AddMethodTemplateCandidate(FunctionTemplateDecl *MethodTmpl,
                                  DeclAccessPair FoundDecl,
                                  CXXRecordDecl *ActingContext,
                                 TemplateArgumentListInfo *ExplicitTemplateArgs,
                                  QualType ObjectType,
                                  Expr::Classification ObjectClassification,
                                  ArrayRef<Expr *> Args,
                                  OverloadCandidateSet& CandidateSet,
                                  bool SuppressUserConversions = false,
                                  bool PartialOverloading = false);
  void AddTemplateOverloadCandidate(FunctionTemplateDecl *FunctionTemplate,
                                    DeclAccessPair FoundDecl,
                                 TemplateArgumentListInfo *ExplicitTemplateArgs,
                                    ArrayRef<Expr *> Args,
                                    OverloadCandidateSet& CandidateSet,
                                    bool SuppressUserConversions = false,
                                    bool PartialOverloading = false);
  void AddConversionCandidate(CXXConversionDecl *Conversion,
                              DeclAccessPair FoundDecl,
                              CXXRecordDecl *ActingContext,
                              Expr *From, QualType ToType,
                              OverloadCandidateSet& CandidateSet,
                              bool AllowObjCConversionOnExplicit);
  void AddTemplateConversionCandidate(FunctionTemplateDecl *FunctionTemplate,
                                      DeclAccessPair FoundDecl,
                                      CXXRecordDecl *ActingContext,
                                      Expr *From, QualType ToType,
                                      OverloadCandidateSet &CandidateSet,
                                      bool AllowObjCConversionOnExplicit);
  void AddSurrogateCandidate(CXXConversionDecl *Conversion,
                             DeclAccessPair FoundDecl,
                             CXXRecordDecl *ActingContext,
                             const FunctionProtoType *Proto,
                             Expr *Object, ArrayRef<Expr *> Args,
                             OverloadCandidateSet& CandidateSet);
  void AddMemberOperatorCandidates(OverloadedOperatorKind Op,
                                   SourceLocation OpLoc, ArrayRef<Expr *> Args,
                                   OverloadCandidateSet& CandidateSet,
                                   SourceRange OpRange = SourceRange());
  void AddBuiltinCandidate(QualType ResultTy, QualType *ParamTys,
                           ArrayRef<Expr *> Args, 
                           OverloadCandidateSet& CandidateSet,
                           bool IsAssignmentOperator = false,
                           unsigned NumContextualBoolArguments = 0);
  void AddBuiltinOperatorCandidates(OverloadedOperatorKind Op,
                                    SourceLocation OpLoc, ArrayRef<Expr *> Args,
                                    OverloadCandidateSet& CandidateSet);
  void AddArgumentDependentLookupCandidates(DeclarationName Name,
                                            SourceLocation Loc,
                                            ArrayRef<Expr *> Args,
                                TemplateArgumentListInfo *ExplicitTemplateArgs,
                                            OverloadCandidateSet& CandidateSet,
                                            bool PartialOverloading = false);

  // Emit as a 'note' the specific overload candidate
  void NoteOverloadCandidate(FunctionDecl *Fn, QualType DestType = QualType());

  // Emit as a series of 'note's all template and non-templates
  // identified by the expression Expr
  void NoteAllOverloadCandidates(Expr* E, QualType DestType = QualType());

  /// Check the enable_if expressions on the given function. Returns the first
  /// failing attribute, or NULL if they were all successful.
  EnableIfAttr *CheckEnableIf(FunctionDecl *Function, ArrayRef<Expr *> Args,
                              bool MissingImplicitThis = false);

  // [PossiblyAFunctionType]  -->   [Return]
  // NonFunctionType --> NonFunctionType
  // R (A) --> R(A)
  // R (*)(A) --> R (A)
  // R (&)(A) --> R (A)
  // R (S::*)(A) --> R (A)
  QualType ExtractUnqualifiedFunctionType(QualType PossiblyAFunctionType);

  FunctionDecl *
  ResolveAddressOfOverloadedFunction(Expr *AddressOfExpr,
                                     QualType TargetType,
                                     bool Complain,
                                     DeclAccessPair &Found,
                                     bool *pHadMultipleCandidates = nullptr);

  FunctionDecl *
  ResolveSingleFunctionTemplateSpecialization(OverloadExpr *ovl,
                                              bool Complain = false,
                                              DeclAccessPair *Found = nullptr);

  bool ResolveAndFixSingleFunctionTemplateSpecialization(
                      ExprResult &SrcExpr,
                      bool DoFunctionPointerConverion = false,
                      bool Complain = false,
                      const SourceRange& OpRangeForComplaining = SourceRange(),
                      QualType DestTypeForComplaining = QualType(),
                      unsigned DiagIDForComplaining = 0);


  Expr *FixOverloadedFunctionReference(Expr *E,
                                       DeclAccessPair FoundDecl,
                                       FunctionDecl *Fn);
  ExprResult FixOverloadedFunctionReference(ExprResult,
                                            DeclAccessPair FoundDecl,
                                            FunctionDecl *Fn);

  void AddOverloadedCallCandidates(UnresolvedLookupExpr *ULE,
                                   ArrayRef<Expr *> Args,
                                   OverloadCandidateSet &CandidateSet,
                                   bool PartialOverloading = false);

  // An enum used to represent the different possible results of building a
  // range-based for loop.
  enum ForRangeStatus {
    FRS_Success,
    FRS_NoViableFunction,
    FRS_DiagnosticIssued
  };

  // An enum to represent whether something is dealing with a call to begin()
  // or a call to end() in a range-based for loop.
  enum BeginEndFunction {
    BEF_begin,
    BEF_end
  };

  ForRangeStatus BuildForRangeBeginEndCall(Scope *S, SourceLocation Loc,
                                           SourceLocation RangeLoc,
                                           VarDecl *Decl,
                                           BeginEndFunction BEF,
                                           const DeclarationNameInfo &NameInfo,
                                           LookupResult &MemberLookup,
                                           OverloadCandidateSet *CandidateSet,
                                           Expr *Range, ExprResult *CallExpr);

  ExprResult BuildOverloadedCallExpr(Scope *S, Expr *Fn,
                                     UnresolvedLookupExpr *ULE,
                                     SourceLocation LParenLoc,
                                     MultiExprArg Args,
                                     SourceLocation RParenLoc,
                                     Expr *ExecConfig,
                                     bool AllowTypoCorrection=true);

  bool buildOverloadedCallSet(Scope *S, Expr *Fn, UnresolvedLookupExpr *ULE,
                              MultiExprArg Args, SourceLocation RParenLoc,
                              OverloadCandidateSet *CandidateSet,
                              ExprResult *Result);

  ExprResult CreateOverloadedUnaryOp(SourceLocation OpLoc,
                                     unsigned Opc,
                                     const UnresolvedSetImpl &Fns,
                                     Expr *input);

  ExprResult CreateOverloadedBinOp(SourceLocation OpLoc,
                                   unsigned Opc,
                                   const UnresolvedSetImpl &Fns,
                                   Expr *LHS, Expr *RHS);

  ExprResult CreateOverloadedArraySubscriptExpr(SourceLocation LLoc,
                                                SourceLocation RLoc,
                                                Expr *Base,Expr *Idx);

  ExprResult
  BuildCallToMemberFunction(Scope *S, Expr *MemExpr,
                            SourceLocation LParenLoc,
                            MultiExprArg Args,
                            SourceLocation RParenLoc);
  ExprResult
  BuildCallToObjectOfClassType(Scope *S, Expr *Object, SourceLocation LParenLoc,
                               MultiExprArg Args,
                               SourceLocation RParenLoc);

  ExprResult BuildOverloadedArrowExpr(Scope *S, Expr *Base,
                                      SourceLocation OpLoc,
                                      bool *NoArrowOperatorFound = nullptr);

  /// CheckCallReturnType - Checks that a call expression's return type is
  /// complete. Returns true on failure. The location passed in is the location
  /// that best represents the call.
  bool CheckCallReturnType(QualType ReturnType, SourceLocation Loc,
                           CallExpr *CE, FunctionDecl *FD);

  /// Helpers for dealing with blocks and functions.
  bool CheckParmsForFunctionDef(ParmVarDecl *const *Param,
                                ParmVarDecl *const *ParamEnd,
                                bool CheckParameterNames);
  void CheckCXXDefaultArguments(FunctionDecl *FD);
  void CheckExtraCXXDefaultArguments(Declarator &D);
  Scope *getNonFieldDeclScope(Scope *S);

  /// \name Name lookup
  ///
  /// These routines provide name lookup that is used during semantic
  /// analysis to resolve the various kinds of names (identifiers,
  /// overloaded operator names, constructor names, etc.) into zero or
  /// more declarations within a particular scope. The major entry
  /// points are LookupName, which performs unqualified name lookup,
  /// and LookupQualifiedName, which performs qualified name lookup.
  ///
  /// All name lookup is performed based on some specific criteria,
  /// which specify what names will be visible to name lookup and how
  /// far name lookup should work. These criteria are important both
  /// for capturing language semantics (certain lookups will ignore
  /// certain names, for example) and for performance, since name
  /// lookup is often a bottleneck in the compilation of C++. Name
  /// lookup criteria is specified via the LookupCriteria enumeration.
  ///
  /// The results of name lookup can vary based on the kind of name
  /// lookup performed, the current language, and the translation
  /// unit. In C, for example, name lookup will either return nothing
  /// (no entity found) or a single declaration. In C++, name lookup
  /// can additionally refer to a set of overloaded functions or
  /// result in an ambiguity. All of the possible results of name
  /// lookup are captured by the LookupResult class, which provides
  /// the ability to distinguish among them.
  //@{

  /// @brief Describes the kind of name lookup to perform.
  enum LookupNameKind {
    /// Ordinary name lookup, which finds ordinary names (functions,
    /// variables, typedefs, etc.) in C and most kinds of names
    /// (functions, variables, members, types, etc.) in C++.
    LookupOrdinaryName = 0,
    /// Tag name lookup, which finds the names of enums, classes,
    /// structs, and unions.
    LookupTagName,
    /// Label name lookup.
    LookupLabel,
    /// Member name lookup, which finds the names of
    /// class/struct/union members.
    LookupMemberName,
    /// Look up of an operator name (e.g., operator+) for use with
    /// operator overloading. This lookup is similar to ordinary name
    /// lookup, but will ignore any declarations that are class members.
    LookupOperatorName,
    /// Look up of a name that precedes the '::' scope resolution
    /// operator in C++. This lookup completely ignores operator, object,
    /// function, and enumerator names (C++ [basic.lookup.qual]p1).
    LookupNestedNameSpecifierName,
    /// Look up a namespace name within a C++ using directive or
    /// namespace alias definition, ignoring non-namespace names (C++
    /// [basic.lookup.udir]p1).
    LookupNamespaceName,
    /// Look up all declarations in a scope with the given name,
    /// including resolved using declarations.  This is appropriate
    /// for checking redeclarations for a using declaration.
    LookupUsingDeclName,
    /// Look up an ordinary name that is going to be redeclared as a
    /// name with linkage. This lookup ignores any declarations that
    /// are outside of the current scope unless they have linkage. See
    /// C99 6.2.2p4-5 and C++ [basic.link]p6.
    LookupRedeclarationWithLinkage,
    /// Look up a friend of a local class. This lookup does not look
    /// outside the innermost non-class scope. See C++11 [class.friend]p11.
    LookupLocalFriendName,
    /// Look up the name of an Objective-C protocol.
    LookupObjCProtocolName,
    /// Look up implicit 'self' parameter of an objective-c method.
    LookupObjCImplicitSelfParam,
    /// \brief Look up any declaration with any name.
    LookupAnyName
  };

  /// \brief Specifies whether (or how) name lookup is being performed for a
  /// redeclaration (vs. a reference).
  enum RedeclarationKind {
    /// \brief The lookup is a reference to this name that is not for the
    /// purpose of redeclaring the name.
    NotForRedeclaration = 0,
    /// \brief The lookup results will be used for redeclaration of a name,
    /// if an entity by that name already exists.
    ForRedeclaration
  };

  /// \brief The possible outcomes of name lookup for a literal operator.
  enum LiteralOperatorLookupResult {
    /// \brief The lookup resulted in an error.
    LOLR_Error,
    /// \brief The lookup found a single 'cooked' literal operator, which
    /// expects a normal literal to be built and passed to it.
    LOLR_Cooked,
    /// \brief The lookup found a single 'raw' literal operator, which expects
    /// a string literal containing the spelling of the literal token.
    LOLR_Raw,
    /// \brief The lookup found an overload set of literal operator templates,
    /// which expect the characters of the spelling of the literal token to be
    /// passed as a non-type template argument pack.
    LOLR_Template,
    /// \brief The lookup found an overload set of literal operator templates,
    /// which expect the character type and characters of the spelling of the
    /// string literal token to be passed as template arguments.
    LOLR_StringTemplate
  };

  SpecialMemberOverloadResult *LookupSpecialMember(CXXRecordDecl *D,
                                                   CXXSpecialMember SM,
                                                   bool ConstArg,
                                                   bool VolatileArg,
                                                   bool RValueThis,
                                                   bool ConstThis,
                                                   bool VolatileThis);

  typedef std::function<void(const TypoCorrection &)> TypoDiagnosticGenerator;
  typedef std::function<ExprResult(Sema &, TypoExpr *, TypoCorrection)>
      TypoRecoveryCallback;

private:
  bool CppLookupName(LookupResult &R, Scope *S);

  struct TypoExprState {
    std::unique_ptr<TypoCorrectionConsumer> Consumer;
    TypoDiagnosticGenerator DiagHandler;
    TypoRecoveryCallback RecoveryHandler;
    TypoExprState();
    TypoExprState(TypoExprState&& other) LLVM_NOEXCEPT;
    TypoExprState& operator=(TypoExprState&& other) LLVM_NOEXCEPT;
  };

  /// \brief The set of unhandled TypoExprs and their associated state.
  llvm::MapVector<TypoExpr *, TypoExprState> DelayedTypos;

  /// \brief Creates a new TypoExpr AST node.
  TypoExpr *createDelayedTypo(std::unique_ptr<TypoCorrectionConsumer> TCC,
                              TypoDiagnosticGenerator TDG,
                              TypoRecoveryCallback TRC);

  // \brief The set of known/encountered (unique, canonicalized) NamespaceDecls.
  //
  // The boolean value will be true to indicate that the namespace was loaded
  // from an AST/PCH file, or false otherwise.
  llvm::MapVector<NamespaceDecl*, bool> KnownNamespaces;

  /// \brief Whether we have already loaded known namespaces from an extenal
  /// source.
  bool LoadedExternalKnownNamespaces;

  /// \brief Helper for CorrectTypo and CorrectTypoDelayed used to create and
  /// populate a new TypoCorrectionConsumer. Returns nullptr if typo correction
  /// should be skipped entirely.
  std::unique_ptr<TypoCorrectionConsumer>
  makeTypoCorrectionConsumer(const DeclarationNameInfo &Typo,
                             Sema::LookupNameKind LookupKind, Scope *S,
                             CXXScopeSpec *SS,
                             std::unique_ptr<CorrectionCandidateCallback> CCC,
                             DeclContext *MemberContext, bool EnteringContext,
                             const ObjCObjectPointerType *OPT,
                             bool ErrorRecovery);

  /// HLSL Change Begin - back ported from llvm-project/c601377b2376.
  bool addInstantiatedParametersToScope(
      FunctionDecl *Function, const FunctionDecl *PatternDecl,
      LocalInstantiationScope &Scope,
      const MultiLevelTemplateArgumentList &TemplateArgs);
  /// HLSL Change End - back ported from llvm-project/c601377b2376.

public:
  const TypoExprState &getTypoExprState(TypoExpr *TE) const;

  /// \brief Clears the state of the given TypoExpr.
  void clearDelayedTypo(TypoExpr *TE);

  /// \brief Look up a name, looking for a single declaration.  Return
  /// null if the results were absent, ambiguous, or overloaded.
  ///
  /// It is preferable to use the elaborated form and explicitly handle
  /// ambiguity and overloaded.
  NamedDecl *LookupSingleName(Scope *S, DeclarationName Name,
                              SourceLocation Loc,
                              LookupNameKind NameKind,
                              RedeclarationKind Redecl
                                = NotForRedeclaration);
  bool LookupName(LookupResult &R, Scope *S,
                  bool AllowBuiltinCreation = false);
  bool LookupQualifiedName(LookupResult &R, DeclContext *LookupCtx,
                           bool InUnqualifiedLookup = false);
  bool LookupQualifiedName(LookupResult &R, DeclContext *LookupCtx,
                           CXXScopeSpec &SS);
  bool LookupParsedName(LookupResult &R, Scope *S, CXXScopeSpec *SS,
                        bool AllowBuiltinCreation = false,
                        bool EnteringContext = false);
  ObjCProtocolDecl *LookupProtocol(IdentifierInfo *II, SourceLocation IdLoc,
                                   RedeclarationKind Redecl
                                     = NotForRedeclaration);
  bool LookupInSuper(LookupResult &R, CXXRecordDecl *Class);

  void LookupOverloadedOperatorName(OverloadedOperatorKind Op, Scope *S,
                                    QualType T1, QualType T2,
                                    UnresolvedSetImpl &Functions);
  void addOverloadedOperatorToUnresolvedSet(UnresolvedSetImpl &Functions,
                                            DeclAccessPair Operator,
                                            QualType T1, QualType T2);

  LabelDecl *LookupOrCreateLabel(IdentifierInfo *II, SourceLocation IdentLoc,
                                 SourceLocation GnuLabelLoc = SourceLocation());

  DeclContextLookupResult LookupConstructors(CXXRecordDecl *Class);
  CXXConstructorDecl *LookupDefaultConstructor(CXXRecordDecl *Class);
  CXXConstructorDecl *LookupCopyingConstructor(CXXRecordDecl *Class,
                                               unsigned Quals);
  CXXMethodDecl *LookupCopyingAssignment(CXXRecordDecl *Class, unsigned Quals,
                                         bool RValueThis, unsigned ThisQuals);
  CXXConstructorDecl *LookupMovingConstructor(CXXRecordDecl *Class,
                                              unsigned Quals);
  CXXMethodDecl *LookupMovingAssignment(CXXRecordDecl *Class, unsigned Quals,
                                        bool RValueThis, unsigned ThisQuals);
  CXXDestructorDecl *LookupDestructor(CXXRecordDecl *Class);

  bool checkLiteralOperatorId(const CXXScopeSpec &SS, const UnqualifiedId &Id);
  LiteralOperatorLookupResult LookupLiteralOperator(Scope *S, LookupResult &R,
                                                    ArrayRef<QualType> ArgTys,
                                                    bool AllowRaw,
                                                    bool AllowTemplate,
                                                    bool AllowStringTemplate);
  bool isKnownName(StringRef name);

  void ArgumentDependentLookup(DeclarationName Name, SourceLocation Loc,
                               ArrayRef<Expr *> Args, ADLResult &Functions);

  void LookupVisibleDecls(Scope *S, LookupNameKind Kind,
                          VisibleDeclConsumer &Consumer,
                          bool IncludeGlobalScope = true);
  void LookupVisibleDecls(DeclContext *Ctx, LookupNameKind Kind,
                          VisibleDeclConsumer &Consumer,
                          bool IncludeGlobalScope = true);

  enum CorrectTypoKind {
    CTK_NonError,     // CorrectTypo used in a non error recovery situation.
    CTK_ErrorRecovery // CorrectTypo used in normal error recovery.
  };

  TypoCorrection CorrectTypo(const DeclarationNameInfo &Typo,
                             Sema::LookupNameKind LookupKind,
                             Scope *S, CXXScopeSpec *SS,
                             std::unique_ptr<CorrectionCandidateCallback> CCC,
                             CorrectTypoKind Mode,
                             DeclContext *MemberContext = nullptr,
                             bool EnteringContext = false,
                             const ObjCObjectPointerType *OPT = nullptr,
                             bool RecordFailure = true);

  TypoExpr *CorrectTypoDelayed(const DeclarationNameInfo &Typo,
                               Sema::LookupNameKind LookupKind, Scope *S,
                               CXXScopeSpec *SS,
                               std::unique_ptr<CorrectionCandidateCallback> CCC,
                               TypoDiagnosticGenerator TDG,
                               TypoRecoveryCallback TRC, CorrectTypoKind Mode,
                               DeclContext *MemberContext = nullptr,
                               bool EnteringContext = false,
                               const ObjCObjectPointerType *OPT = nullptr);

  /// \brief Process any TypoExprs in the given Expr and its children,
  /// generating diagnostics as appropriate and returning a new Expr if there
  /// were typos that were all successfully corrected and ExprError if one or
  /// more typos could not be corrected.
  ///
  /// \param E The Expr to check for TypoExprs.
  ///
  /// \param InitDecl A VarDecl to avoid because the Expr being corrected is its
  /// initializer.
  ///
  /// \param Filter A function applied to a newly rebuilt Expr to determine if
  /// it is an acceptable/usable result from a single combination of typo
  /// corrections. As long as the filter returns ExprError, different
  /// combinations of corrections will be tried until all are exhausted.
  ExprResult
  CorrectDelayedTyposInExpr(Expr *E, VarDecl *InitDecl = nullptr,
                            llvm::function_ref<ExprResult(Expr *)> Filter =
                                [](Expr *E) -> ExprResult { return E; });

  ExprResult
  CorrectDelayedTyposInExpr(Expr *E,
                            llvm::function_ref<ExprResult(Expr *)> Filter) {
    return CorrectDelayedTyposInExpr(E, nullptr, Filter);
  }

  ExprResult
  CorrectDelayedTyposInExpr(ExprResult ER, VarDecl *InitDecl = nullptr,
                            llvm::function_ref<ExprResult(Expr *)> Filter =
                                [](Expr *E) -> ExprResult { return E; }) {
    return ER.isInvalid() ? ER : CorrectDelayedTyposInExpr(ER.get(), Filter);
  }

  ExprResult
  CorrectDelayedTyposInExpr(ExprResult ER,
                            llvm::function_ref<ExprResult(Expr *)> Filter) {
    return CorrectDelayedTyposInExpr(ER, nullptr, Filter);
  }

  void diagnoseTypo(const TypoCorrection &Correction,
                    const PartialDiagnostic &TypoDiag,
                    bool ErrorRecovery = true);

  void diagnoseTypo(const TypoCorrection &Correction,
                    const PartialDiagnostic &TypoDiag,
                    const PartialDiagnostic &PrevNote,
                    bool ErrorRecovery = true);

  void FindAssociatedClassesAndNamespaces(SourceLocation InstantiationLoc,
                                          ArrayRef<Expr *> Args,
                                   AssociatedNamespaceSet &AssociatedNamespaces,
                                   AssociatedClassSet &AssociatedClasses);

  void FilterLookupForScope(LookupResult &R, DeclContext *Ctx, Scope *S,
                            bool ConsiderLinkage, bool AllowInlineNamespace);

  void DiagnoseAmbiguousLookup(LookupResult &Result);
  //@}

  ObjCInterfaceDecl *getObjCInterfaceDecl(IdentifierInfo *&Id,
                                          SourceLocation IdLoc,
                                          bool TypoCorrection = false);
  NamedDecl *LazilyCreateBuiltin(IdentifierInfo *II, unsigned ID,
                                 Scope *S, bool ForRedeclaration,
                                 SourceLocation Loc);
  NamedDecl *ImplicitlyDefineFunction(SourceLocation Loc, IdentifierInfo &II,
                                      Scope *S);
  void AddKnownFunctionAttributes(FunctionDecl *FD);

  // More parsing and symbol table subroutines.

  void ProcessPragmaWeak(Scope *S, Decl *D);
  // Decl attributes - this routine is the top level dispatcher.
  void ProcessDeclAttributes(Scope *S, Decl *D, const Declarator &PD);
  void ProcessDeclAttributeList(Scope *S, Decl *D, const AttributeList *AL,
                                bool IncludeCXX11Attributes = true);
  bool ProcessAccessDeclAttributeList(AccessSpecDecl *ASDecl,
                                      const AttributeList *AttrList);

  void checkUnusedDeclAttributes(Declarator &D);

  /// Determine if type T is a valid subject for a nonnull and similar
  /// attributes. By default, we look through references (the behavior used by
  /// nonnull), but if the second parameter is true, then we treat a reference
  /// type as valid.
  bool isValidPointerAttrType(QualType T, bool RefOkay = false);

  bool CheckRegparmAttr(const AttributeList &attr, unsigned &value);
  bool CheckCallingConvAttr(const AttributeList &attr, CallingConv &CC, 
                            const FunctionDecl *FD = nullptr);
  bool CheckNoReturnAttr(const AttributeList &attr);
  bool checkStringLiteralArgumentAttr(const AttributeList &Attr,
                                      unsigned ArgNum, StringRef &Str,
                                      SourceLocation *ArgLocation = nullptr);
  bool checkSectionName(SourceLocation LiteralLoc, StringRef Str);
  void checkTargetAttr(SourceLocation LiteralLoc, StringRef Str);
  bool checkMSInheritanceAttrOnDefinition(
      CXXRecordDecl *RD, SourceRange Range, bool BestCase,
      MSInheritanceAttr::Spelling SemanticSpelling);

  void CheckAlignasUnderalignment(Decl *D);

  /// Adjust the calling convention of a method to be the ABI default if it
  /// wasn't specified explicitly.  This handles method types formed from
  /// function type typedefs and typename template arguments.
  void adjustMemberFunctionCC(QualType &T, bool IsStatic);

  // Check if there is an explicit attribute, but only look through parens.
  // The intent is to look for an attribute on the current declarator, but not
  // one that came from a typedef.
  bool hasExplicitCallingConv(QualType &T);

  /// Get the outermost AttributedType node that sets a calling convention.
  /// Valid types should not have multiple attributes with different CCs.
  const AttributedType *getCallingConvAttributedType(QualType T) const;

  /// Check whether a nullability type specifier can be added to the given
  /// type.
  ///
  /// \param type The type to which the nullability specifier will be
  /// added. On success, this type will be updated appropriately.
  ///
  /// \param nullability The nullability specifier to add.
  ///
  /// \param nullabilityLoc The location of the nullability specifier.
  ///
  /// \param isContextSensitive Whether this nullability specifier was
  /// written as a context-sensitive keyword (in an Objective-C
  /// method) or an Objective-C property attribute, rather than as an
  /// underscored type specifier.
  ///
  /// \returns true if nullability cannot be applied, false otherwise.
  bool checkNullabilityTypeSpecifier(QualType &type, NullabilityKind nullability,
                                     SourceLocation nullabilityLoc,
                                     bool isContextSensitive);

  /// \brief Stmt attributes - this routine is the top level dispatcher.
  StmtResult ProcessStmtAttributes(Stmt *Stmt, AttributeList *Attrs,
                                   SourceRange Range);

  void WarnConflictingTypedMethods(ObjCMethodDecl *Method,
                                   ObjCMethodDecl *MethodDecl,
                                   bool IsProtocolMethodDecl);

  void CheckConflictingOverridingMethod(ObjCMethodDecl *Method,
                                   ObjCMethodDecl *Overridden,
                                   bool IsProtocolMethodDecl);

  void ValidateShaderAttributes(Decl *D, const AttributeList *A);

  /// WarnExactTypedMethods - This routine issues a warning if method
  /// implementation declaration matches exactly that of its declaration.
  void WarnExactTypedMethods(ObjCMethodDecl *Method,
                             ObjCMethodDecl *MethodDecl,
                             bool IsProtocolMethodDecl);

  typedef llvm::SmallPtrSet<Selector, 8> SelectorSet;
  typedef llvm::DenseMap<Selector, ObjCMethodDecl*> ProtocolsMethodsMap;

  /// CheckImplementationIvars - This routine checks if the instance variables
  /// listed in the implelementation match those listed in the interface.
  void CheckImplementationIvars(ObjCImplementationDecl *ImpDecl,
                                ObjCIvarDecl **Fields, unsigned nIvars,
                                SourceLocation Loc);

  /// ImplMethodsVsClassMethods - This is main routine to warn if any method
  /// remains unimplemented in the class or category \@implementation.
  void ImplMethodsVsClassMethods(Scope *S, ObjCImplDecl* IMPDecl,
                                 ObjCContainerDecl* IDecl,
                                 bool IncompleteImpl = false);

  /// DiagnoseUnimplementedProperties - This routine warns on those properties
  /// which must be implemented by this implementation.
  void DiagnoseUnimplementedProperties(Scope *S, ObjCImplDecl* IMPDecl,
                                       ObjCContainerDecl *CDecl,
                                       bool SynthesizeProperties);

  /// Diagnose any null-resettable synthesized setters.
  void diagnoseNullResettableSynthesizedSetters(const ObjCImplDecl *impDecl);

  /// DefaultSynthesizeProperties - This routine default synthesizes all
  /// properties which must be synthesized in the class's \@implementation.
  void DefaultSynthesizeProperties (Scope *S, ObjCImplDecl* IMPDecl,
                                    ObjCInterfaceDecl *IDecl);
  void DefaultSynthesizeProperties(Scope *S, Decl *D);

  /// IvarBacksCurrentMethodAccessor - This routine returns 'true' if 'IV' is
  /// an ivar synthesized for 'Method' and 'Method' is a property accessor
  /// declared in class 'IFace'.
  bool IvarBacksCurrentMethodAccessor(ObjCInterfaceDecl *IFace,
                                      ObjCMethodDecl *Method, ObjCIvarDecl *IV);
  
  /// DiagnoseUnusedBackingIvarInAccessor - Issue an 'unused' warning if ivar which
  /// backs the property is not used in the property's accessor.
  void DiagnoseUnusedBackingIvarInAccessor(Scope *S,
                                           const ObjCImplementationDecl *ImplD);
  
  /// GetIvarBackingPropertyAccessor - If method is a property setter/getter and
  /// it property has a backing ivar, returns this ivar; otherwise, returns NULL.
  /// It also returns ivar's property on success.
  ObjCIvarDecl *GetIvarBackingPropertyAccessor(const ObjCMethodDecl *Method,
                                               const ObjCPropertyDecl *&PDecl) const;
  
  /// Called by ActOnProperty to handle \@property declarations in
  /// class extensions.
  ObjCPropertyDecl *HandlePropertyInClassExtension(Scope *S,
                      SourceLocation AtLoc,
                      SourceLocation LParenLoc,
                      FieldDeclarator &FD,
                      Selector GetterSel,
                      Selector SetterSel,
                      const bool isAssign,
                      const bool isReadWrite,
                      const unsigned Attributes,
                      const unsigned AttributesAsWritten,
                      bool *isOverridingProperty,
                      QualType T,
                      TypeSourceInfo *TSI,
                      tok::ObjCKeywordKind MethodImplKind);

  /// Called by ActOnProperty and HandlePropertyInClassExtension to
  /// handle creating the ObjcPropertyDecl for a category or \@interface.
  ObjCPropertyDecl *CreatePropertyDecl(Scope *S,
                                       ObjCContainerDecl *CDecl,
                                       SourceLocation AtLoc,
                                       SourceLocation LParenLoc,
                                       FieldDeclarator &FD,
                                       Selector GetterSel,
                                       Selector SetterSel,
                                       const bool isAssign,
                                       const bool isReadWrite,
                                       const unsigned Attributes,
                                       const unsigned AttributesAsWritten,
                                       QualType T,
                                       TypeSourceInfo *TSI,
                                       tok::ObjCKeywordKind MethodImplKind,
                                       DeclContext *lexicalDC = nullptr);

  /// AtomicPropertySetterGetterRules - This routine enforces the rule (via
  /// warning) when atomic property has one but not the other user-declared
  /// setter or getter.
  void AtomicPropertySetterGetterRules(ObjCImplDecl* IMPDecl,
                                       ObjCContainerDecl* IDecl);

  void DiagnoseOwningPropertyGetterSynthesis(const ObjCImplementationDecl *D);

  void DiagnoseMissingDesignatedInitOverrides(
                                          const ObjCImplementationDecl *ImplD,
                                          const ObjCInterfaceDecl *IFD);

  void DiagnoseDuplicateIvars(ObjCInterfaceDecl *ID, ObjCInterfaceDecl *SID);

  enum MethodMatchStrategy {
    MMS_loose,
    MMS_strict
  };

  /// MatchTwoMethodDeclarations - Checks if two methods' type match and returns
  /// true, or false, accordingly.
  bool MatchTwoMethodDeclarations(const ObjCMethodDecl *Method,
                                  const ObjCMethodDecl *PrevMethod,
                                  MethodMatchStrategy strategy = MMS_strict);

  /// MatchAllMethodDeclarations - Check methods declaraed in interface or
  /// or protocol against those declared in their implementations.
  void MatchAllMethodDeclarations(const SelectorSet &InsMap,
                                  const SelectorSet &ClsMap,
                                  SelectorSet &InsMapSeen,
                                  SelectorSet &ClsMapSeen,
                                  ObjCImplDecl* IMPDecl,
                                  ObjCContainerDecl* IDecl,
                                  bool &IncompleteImpl,
                                  bool ImmediateClass,
                                  bool WarnCategoryMethodImpl=false);

  /// CheckCategoryVsClassMethodMatches - Checks that methods implemented in
  /// category matches with those implemented in its primary class and
  /// warns each time an exact match is found.
  void CheckCategoryVsClassMethodMatches(ObjCCategoryImplDecl *CatIMP);

  /// \brief Add the given method to the list of globally-known methods.
  void addMethodToGlobalList(ObjCMethodList *List, ObjCMethodDecl *Method);

private:
  /// AddMethodToGlobalPool - Add an instance or factory method to the global
  /// pool. See descriptoin of AddInstanceMethodToGlobalPool.
  void AddMethodToGlobalPool(ObjCMethodDecl *Method, bool impl, bool instance);

  /// LookupMethodInGlobalPool - Returns the instance or factory method and
  /// optionally warns if there are multiple signatures.
  ObjCMethodDecl *LookupMethodInGlobalPool(Selector Sel, SourceRange R,
                                           bool receiverIdOrClass,
                                           bool instance);

public:
  /// \brief - Returns instance or factory methods in global method pool for
  /// given selector. If no such method or only one method found, function returns
  /// false; otherwise, it returns true
  bool CollectMultipleMethodsInGlobalPool(Selector Sel,
                                          SmallVectorImpl<ObjCMethodDecl*>& Methods,
                                          bool instance);
    
  bool AreMultipleMethodsInGlobalPool(Selector Sel, ObjCMethodDecl *BestMethod,
                                      SourceRange R,
                                      bool receiverIdOrClass);
      
  void DiagnoseMultipleMethodInGlobalPool(SmallVectorImpl<ObjCMethodDecl*> &Methods,
                                          Selector Sel, SourceRange R,
                                          bool receiverIdOrClass);

private:
  /// \brief - Returns a selector which best matches given argument list or
  /// nullptr if none could be found
  ObjCMethodDecl *SelectBestMethod(Selector Sel, MultiExprArg Args,
                                   bool IsInstance);
    

  /// \brief Record the typo correction failure and return an empty correction.
  TypoCorrection FailedCorrection(IdentifierInfo *Typo, SourceLocation TypoLoc,
                                  bool RecordFailure = true) {
    if (RecordFailure)
      TypoCorrectionFailures[Typo].insert(TypoLoc);
    return TypoCorrection();
  }

public:
  /// AddInstanceMethodToGlobalPool - All instance methods in a translation
  /// unit are added to a global pool. This allows us to efficiently associate
  /// a selector with a method declaraation for purposes of typechecking
  /// messages sent to "id" (where the class of the object is unknown).
  void AddInstanceMethodToGlobalPool(ObjCMethodDecl *Method, bool impl=false) {
    AddMethodToGlobalPool(Method, impl, /*instance*/true);
  }

  /// AddFactoryMethodToGlobalPool - Same as above, but for factory methods.
  void AddFactoryMethodToGlobalPool(ObjCMethodDecl *Method, bool impl=false) {
    AddMethodToGlobalPool(Method, impl, /*instance*/false);
  }

  /// AddAnyMethodToGlobalPool - Add any method, instance or factory to global
  /// pool.
  void AddAnyMethodToGlobalPool(Decl *D);

  /// LookupInstanceMethodInGlobalPool - Returns the method and warns if
  /// there are multiple signatures.
  ObjCMethodDecl *LookupInstanceMethodInGlobalPool(Selector Sel, SourceRange R,
                                                   bool receiverIdOrClass=false) {
    return LookupMethodInGlobalPool(Sel, R, receiverIdOrClass,
                                    /*instance*/true);
  }

  /// LookupFactoryMethodInGlobalPool - Returns the method and warns if
  /// there are multiple signatures.
  ObjCMethodDecl *LookupFactoryMethodInGlobalPool(Selector Sel, SourceRange R,
                                                  bool receiverIdOrClass=false) {
    return LookupMethodInGlobalPool(Sel, R, receiverIdOrClass,
                                    /*instance*/false);
  }

  const ObjCMethodDecl *SelectorsForTypoCorrection(Selector Sel,
                              QualType ObjectType=QualType());
  /// LookupImplementedMethodInGlobalPool - Returns the method which has an
  /// implementation.
  ObjCMethodDecl *LookupImplementedMethodInGlobalPool(Selector Sel);

  /// CollectIvarsToConstructOrDestruct - Collect those ivars which require
  /// initialization.
  void CollectIvarsToConstructOrDestruct(ObjCInterfaceDecl *OI,
                                  SmallVectorImpl<ObjCIvarDecl*> &Ivars);

  //===--------------------------------------------------------------------===//
  // Statement Parsing Callbacks: SemaStmt.cpp.
public:
  class FullExprArg {
  public:
    FullExprArg(Sema &actions) : E(nullptr) { }

    ExprResult release() {
      return E;
    }

    Expr *get() const { return E; }

    Expr *operator->() {
      return E;
    }

  private:
    // FIXME: No need to make the entire Sema class a friend when it's just
    // Sema::MakeFullExpr that needs access to the constructor below.
    friend class Sema;

    explicit FullExprArg(Expr *expr) : E(expr) {}

    Expr *E;
  };

  FullExprArg MakeFullExpr(Expr *Arg) {
    return MakeFullExpr(Arg, Arg ? Arg->getExprLoc() : SourceLocation());
  }
  FullExprArg MakeFullExpr(Expr *Arg, SourceLocation CC) {
    return FullExprArg(ActOnFinishFullExpr(Arg, CC).get());
  }
  FullExprArg MakeFullDiscardedValueExpr(Expr *Arg) {
    ExprResult FE =
      ActOnFinishFullExpr(Arg, Arg ? Arg->getExprLoc() : SourceLocation(),
                          /*DiscardedValue*/ true);
    return FullExprArg(FE.get());
  }

  StmtResult ActOnExprStmt(ExprResult Arg);
  StmtResult ActOnExprStmtError();

  StmtResult ActOnHlslDiscardStmt(SourceLocation Loc); // HLSL Change
  
  StmtResult ActOnNullStmt(SourceLocation SemiLoc,
                           bool HasLeadingEmptyMacro = false);

  void ActOnStartOfCompoundStmt();
  void ActOnFinishOfCompoundStmt();
  StmtResult ActOnCompoundStmt(SourceLocation L, SourceLocation R,
                               ArrayRef<Stmt *> Elts, bool isStmtExpr);

  /// \brief A RAII object to enter scope of a compound statement.
  class CompoundScopeRAII {
  public:
    CompoundScopeRAII(Sema &S): S(S) {
      S.ActOnStartOfCompoundStmt();
    }

    ~CompoundScopeRAII() {
      S.ActOnFinishOfCompoundStmt();
    }

  private:
    Sema &S;
  };

  /// An RAII helper that pops function a function scope on exit.
  struct FunctionScopeRAII {
    Sema &S;
    bool Active;
    FunctionScopeRAII(Sema &S) : S(S), Active(true) {}
    ~FunctionScopeRAII() {
      if (Active)
        S.PopFunctionScopeInfo();
    }
    void disable() { Active = false; }
  };

  StmtResult ActOnDeclStmt(DeclGroupPtrTy Decl,
                                   SourceLocation StartLoc,
                                   SourceLocation EndLoc);
  void ActOnForEachDeclStmt(DeclGroupPtrTy Decl);
  StmtResult ActOnForEachLValueExpr(Expr *E);
  StmtResult ActOnCaseStmt(SourceLocation CaseLoc, Expr *LHSVal,
                                   SourceLocation DotDotDotLoc, Expr *RHSVal,
                                   SourceLocation ColonLoc);
  void ActOnCaseStmtBody(Stmt *CaseStmt, Stmt *SubStmt);

  StmtResult ActOnDefaultStmt(SourceLocation DefaultLoc,
                                      SourceLocation ColonLoc,
                                      Stmt *SubStmt, Scope *CurScope);
  StmtResult ActOnLabelStmt(SourceLocation IdentLoc, LabelDecl *TheDecl,
                            SourceLocation ColonLoc, Stmt *SubStmt);

  StmtResult ActOnAttributedStmt(SourceLocation AttrLoc,
                                 ArrayRef<const Attr*> Attrs,
                                 Stmt *SubStmt);

  StmtResult ActOnIfStmt(SourceLocation IfLoc,
                         FullExprArg CondVal, Decl *CondVar,
                         Stmt *ThenVal,
                         SourceLocation ElseLoc, Stmt *ElseVal);
  StmtResult ActOnStartOfSwitchStmt(SourceLocation SwitchLoc,
                                            Expr *Cond,
                                            Decl *CondVar);
  StmtResult ActOnFinishSwitchStmt(SourceLocation SwitchLoc,
                                           Stmt *Switch, Stmt *Body);
  StmtResult ActOnWhileStmt(SourceLocation WhileLoc,
                            FullExprArg Cond,
                            Decl *CondVar, Stmt *Body);
  StmtResult ActOnDoStmt(SourceLocation DoLoc, Stmt *Body,
                                 SourceLocation WhileLoc,
                                 SourceLocation CondLParen, Expr *Cond,
                                 SourceLocation CondRParen);

  StmtResult ActOnForStmt(SourceLocation ForLoc,
                          SourceLocation LParenLoc,
                          Stmt *First, FullExprArg Second,
                          Decl *SecondVar,
                          FullExprArg Third,
                          SourceLocation RParenLoc,
                          Stmt *Body);
  ExprResult CheckObjCForCollectionOperand(SourceLocation forLoc,
                                           Expr *collection);
  StmtResult ActOnObjCForCollectionStmt(SourceLocation ForColLoc,
                                        Stmt *First, Expr *collection,
                                        SourceLocation RParenLoc);
  StmtResult FinishObjCForCollectionStmt(Stmt *ForCollection, Stmt *Body);

  enum BuildForRangeKind {
    /// Initial building of a for-range statement.
    BFRK_Build,
    /// Instantiation or recovery rebuild of a for-range statement. Don't
    /// attempt any typo-correction.
    BFRK_Rebuild,
    /// Determining whether a for-range statement could be built. Avoid any
    /// unnecessary or irreversible actions.
    BFRK_Check
  };

  StmtResult ActOnCXXForRangeStmt(SourceLocation ForLoc, Stmt *LoopVar,
                                  SourceLocation ColonLoc, Expr *Collection,
                                  SourceLocation RParenLoc,
                                  BuildForRangeKind Kind);
  StmtResult BuildCXXForRangeStmt(SourceLocation ForLoc,
                                  SourceLocation ColonLoc,
                                  Stmt *RangeDecl, Stmt *BeginEndDecl,
                                  Expr *Cond, Expr *Inc,
                                  Stmt *LoopVarDecl,
                                  SourceLocation RParenLoc,
                                  BuildForRangeKind Kind);
  StmtResult FinishCXXForRangeStmt(Stmt *ForRange, Stmt *Body);

  StmtResult ActOnGotoStmt(SourceLocation GotoLoc,
                           SourceLocation LabelLoc,
                           LabelDecl *TheDecl);
  StmtResult ActOnIndirectGotoStmt(SourceLocation GotoLoc,
                                   SourceLocation StarLoc,
                                   Expr *DestExp);
  StmtResult ActOnContinueStmt(SourceLocation ContinueLoc, Scope *CurScope);
  StmtResult ActOnBreakStmt(SourceLocation BreakLoc, Scope *CurScope);

  void ActOnCapturedRegionStart(SourceLocation Loc, Scope *CurScope,
                                CapturedRegionKind Kind, unsigned NumParams);
  typedef std::pair<StringRef, QualType> CapturedParamNameType;
  void ActOnCapturedRegionStart(SourceLocation Loc, Scope *CurScope,
                                CapturedRegionKind Kind,
                                ArrayRef<CapturedParamNameType> Params);
  StmtResult ActOnCapturedRegionEnd(Stmt *S);
  void ActOnCapturedRegionError();
  RecordDecl *CreateCapturedStmtRecordDecl(CapturedDecl *&CD,
                                           SourceLocation Loc,
                                           unsigned NumParams);
  VarDecl *getCopyElisionCandidate(QualType ReturnType, Expr *E,
                                   bool AllowFunctionParameters);
  bool isCopyElisionCandidate(QualType ReturnType, const VarDecl *VD,
                              bool AllowFunctionParameters);

  StmtResult ActOnReturnStmt(SourceLocation ReturnLoc, Expr *RetValExp,
                             Scope *CurScope);
  StmtResult BuildReturnStmt(SourceLocation ReturnLoc, Expr *RetValExp);
  StmtResult ActOnCapScopeReturnStmt(SourceLocation ReturnLoc, Expr *RetValExp);

  StmtResult ActOnGCCAsmStmt(SourceLocation AsmLoc, bool IsSimple,
                             bool IsVolatile, unsigned NumOutputs,
                             unsigned NumInputs, IdentifierInfo **Names,
                             MultiExprArg Constraints, MultiExprArg Exprs,
                             Expr *AsmString, MultiExprArg Clobbers,
                             SourceLocation RParenLoc);

  ExprResult LookupInlineAsmIdentifier(CXXScopeSpec &SS,
                                       SourceLocation TemplateKWLoc,
                                       UnqualifiedId &Id,
                                       llvm::InlineAsmIdentifierInfo &Info,
                                       bool IsUnevaluatedContext);
  bool LookupInlineAsmField(StringRef Base, StringRef Member,
                            unsigned &Offset, SourceLocation AsmLoc);
  StmtResult ActOnMSAsmStmt(SourceLocation AsmLoc, SourceLocation LBraceLoc,
                            ArrayRef<Token> AsmToks,
                            StringRef AsmString,
                            unsigned NumOutputs, unsigned NumInputs,
                            ArrayRef<StringRef> Constraints,
                            ArrayRef<StringRef> Clobbers,
                            ArrayRef<Expr*> Exprs,
                            SourceLocation EndLoc);
  LabelDecl *GetOrCreateMSAsmLabel(StringRef ExternalLabelName,
                                   SourceLocation Location,
                                   bool AlwaysCreate);

  VarDecl *BuildObjCExceptionDecl(TypeSourceInfo *TInfo, QualType ExceptionType,
                                  SourceLocation StartLoc,
                                  SourceLocation IdLoc, IdentifierInfo *Id,
                                  bool Invalid = false);

  Decl *ActOnObjCExceptionDecl(Scope *S, Declarator &D);

  StmtResult ActOnObjCAtCatchStmt(SourceLocation AtLoc, SourceLocation RParen,
                                  Decl *Parm, Stmt *Body);

  StmtResult ActOnObjCAtFinallyStmt(SourceLocation AtLoc, Stmt *Body);

  StmtResult ActOnObjCAtTryStmt(SourceLocation AtLoc, Stmt *Try,
                                MultiStmtArg Catch, Stmt *Finally);

  StmtResult BuildObjCAtThrowStmt(SourceLocation AtLoc, Expr *Throw);
  StmtResult ActOnObjCAtThrowStmt(SourceLocation AtLoc, Expr *Throw,
                                  Scope *CurScope);
  ExprResult ActOnObjCAtSynchronizedOperand(SourceLocation atLoc,
                                            Expr *operand);
  StmtResult ActOnObjCAtSynchronizedStmt(SourceLocation AtLoc,
                                         Expr *SynchExpr,
                                         Stmt *SynchBody);

  StmtResult ActOnObjCAutoreleasePoolStmt(SourceLocation AtLoc, Stmt *Body);

  VarDecl *BuildExceptionDeclaration(Scope *S, TypeSourceInfo *TInfo,
                                     SourceLocation StartLoc,
                                     SourceLocation IdLoc,
                                     IdentifierInfo *Id);

  Decl *ActOnExceptionDeclarator(Scope *S, Declarator &D);

  StmtResult ActOnCXXCatchBlock(SourceLocation CatchLoc,
                                Decl *ExDecl, Stmt *HandlerBlock);
  StmtResult ActOnCXXTryBlock(SourceLocation TryLoc, Stmt *TryBlock,
                              ArrayRef<Stmt *> Handlers);

  StmtResult ActOnSEHTryBlock(bool IsCXXTry, // try (true) or __try (false) ?
                              SourceLocation TryLoc, Stmt *TryBlock,
                              Stmt *Handler);
  StmtResult ActOnSEHExceptBlock(SourceLocation Loc,
                                 Expr *FilterExpr,
                                 Stmt *Block);
  void ActOnStartSEHFinallyBlock();
  void ActOnAbortSEHFinallyBlock();
  StmtResult ActOnFinishSEHFinallyBlock(SourceLocation Loc, Stmt *Block);
  StmtResult ActOnSEHLeaveStmt(SourceLocation Loc, Scope *CurScope);

  void DiagnoseReturnInConstructorExceptionHandler(CXXTryStmt *TryBlock);

  bool ShouldWarnIfUnusedFileScopedDecl(const DeclaratorDecl *D) const;

  /// \brief If it's a file scoped decl that must warn if not used, keep track
  /// of it.
  void MarkUnusedFileScopedDecl(const DeclaratorDecl *D);

  /// DiagnoseUnusedExprResult - If the statement passed in is an expression
  /// whose result is unused, warn.
  void DiagnoseUnusedExprResult(const Stmt *S);
  void DiagnoseUnusedNestedTypedefs(const RecordDecl *D);
  void DiagnoseUnusedDecl(const NamedDecl *ND);

  /// Emit \p DiagID if statement located on \p StmtLoc has a suspicious null
  /// statement as a \p Body, and it is located on the same line.
  ///
  /// This helps prevent bugs due to typos, such as:
  ///     if (condition);
  ///       do_stuff();
  void DiagnoseEmptyStmtBody(SourceLocation StmtLoc,
                             const Stmt *Body,
                             unsigned DiagID);

  /// Warn if a for/while loop statement \p S, which is followed by
  /// \p PossibleBody, has a suspicious null statement as a body.
  void DiagnoseEmptyLoopBody(const Stmt *S,
                             const Stmt *PossibleBody);

  /// Warn if a value is moved to itself.
  void DiagnoseSelfMove(const Expr *LHSExpr, const Expr *RHSExpr,
                        SourceLocation OpLoc);

  ParsingDeclState PushParsingDeclaration(sema::DelayedDiagnosticPool &pool) {
    return DelayedDiagnostics.push(pool);
  }
  void PopParsingDeclaration(ParsingDeclState state, Decl *decl);

  typedef ProcessingContextState ParsingClassState;
  ParsingClassState PushParsingClass() {
    return DelayedDiagnostics.pushUndelayed();
  }
  void PopParsingClass(ParsingClassState state) {
    DelayedDiagnostics.popUndelayed(state);
  }

  void redelayDiagnostics(sema::DelayedDiagnosticPool &pool);

  enum AvailabilityDiagnostic { AD_Deprecation, AD_Unavailable, AD_Partial };

  void EmitAvailabilityWarning(AvailabilityDiagnostic AD,
                               NamedDecl *D, StringRef Message,
                               SourceLocation Loc,
                               const ObjCInterfaceDecl *UnknownObjCClass,
                               const ObjCPropertyDecl  *ObjCProperty,
                               bool ObjCPropertyAccess);

  bool makeUnavailableInSystemHeader(SourceLocation loc,
                                     StringRef message);

  //===--------------------------------------------------------------------===//
  // Expression Parsing Callbacks: SemaExpr.cpp.

  bool CanUseDecl(NamedDecl *D);
  bool DiagnoseUseOfDecl(NamedDecl *D, SourceLocation Loc,
                         const ObjCInterfaceDecl *UnknownObjCClass=nullptr,
                         bool ObjCPropertyAccess=false);
  void NoteDeletedFunction(FunctionDecl *FD);
  std::string getDeletedOrUnavailableSuffix(const FunctionDecl *FD);
  bool DiagnosePropertyAccessorMismatch(ObjCPropertyDecl *PD,
                                        ObjCMethodDecl *Getter,
                                        SourceLocation Loc);
  void DiagnoseSentinelCalls(NamedDecl *D, SourceLocation Loc,
                             ArrayRef<Expr *> Args);

  void PushExpressionEvaluationContext(ExpressionEvaluationContext NewContext,
                                       Decl *LambdaContextDecl = nullptr,
                                       bool IsDecltype = false);
  enum ReuseLambdaContextDecl_t { ReuseLambdaContextDecl };
  void PushExpressionEvaluationContext(ExpressionEvaluationContext NewContext,
                                       ReuseLambdaContextDecl_t,
                                       bool IsDecltype = false);
  void PopExpressionEvaluationContext();

  void DiscardCleanupsInEvaluationContext();

  ExprResult TransformToPotentiallyEvaluated(Expr *E);
  ExprResult HandleExprEvaluationContextForTypeof(Expr *E);

  ExprResult ActOnConstantExpression(ExprResult Res);

  // Functions for marking a declaration referenced.  These functions also
  // contain the relevant logic for marking if a reference to a function or
  // variable is an odr-use (in the C++11 sense).  There are separate variants
  // for expressions referring to a decl; these exist because odr-use marking
  // needs to be delayed for some constant variables when we build one of the
  // named expressions.
  void MarkAnyDeclReferenced(SourceLocation Loc, Decl *D, bool OdrUse);
  void MarkFunctionReferenced(SourceLocation Loc, FunctionDecl *Func,
                              bool OdrUse = true);
  void MarkVariableReferenced(SourceLocation Loc, VarDecl *Var);
  void MarkDeclRefReferenced(DeclRefExpr *E);
  void MarkMemberReferenced(MemberExpr *E);

  void UpdateMarkingForLValueToRValue(Expr *E);
  void CleanupVarDeclMarking();

  enum TryCaptureKind {
    TryCapture_Implicit, TryCapture_ExplicitByVal, TryCapture_ExplicitByRef
  };

  /// \brief Try to capture the given variable.
  ///
  /// \param Var The variable to capture.
  ///
  /// \param Loc The location at which the capture occurs.
  ///
  /// \param Kind The kind of capture, which may be implicit (for either a
  /// block or a lambda), or explicit by-value or by-reference (for a lambda).
  ///
  /// \param EllipsisLoc The location of the ellipsis, if one is provided in
  /// an explicit lambda capture.
  ///
  /// \param BuildAndDiagnose Whether we are actually supposed to add the
  /// captures or diagnose errors. If false, this routine merely check whether
  /// the capture can occur without performing the capture itself or complaining
  /// if the variable cannot be captured.
  ///
  /// \param CaptureType Will be set to the type of the field used to capture
  /// this variable in the innermost block or lambda. Only valid when the
  /// variable can be captured.
  ///
  /// \param DeclRefType Will be set to the type of a reference to the capture
  /// from within the current scope. Only valid when the variable can be
  /// captured.
  ///
  /// \param FunctionScopeIndexToStopAt If non-null, it points to the index
  /// of the FunctionScopeInfo stack beyond which we do not attempt to capture.
  /// This is useful when enclosing lambdas must speculatively capture 
  /// variables that may or may not be used in certain specializations of
  /// a nested generic lambda.
  /// 
  /// \returns true if an error occurred (i.e., the variable cannot be
  /// captured) and false if the capture succeeded.
  bool tryCaptureVariable(VarDecl *Var, SourceLocation Loc, TryCaptureKind Kind,
                          SourceLocation EllipsisLoc, bool BuildAndDiagnose,
                          QualType &CaptureType,
                          QualType &DeclRefType, 
                          const unsigned *const FunctionScopeIndexToStopAt);

  /// \brief Try to capture the given variable.
  bool tryCaptureVariable(VarDecl *Var, SourceLocation Loc,
                          TryCaptureKind Kind = TryCapture_Implicit,
                          SourceLocation EllipsisLoc = SourceLocation());

  /// \brief Checks if the variable must be captured.
  bool NeedToCaptureVariable(VarDecl *Var, SourceLocation Loc);

  /// \brief Given a variable, determine the type that a reference to that
  /// variable will have in the given scope.
  QualType getCapturedDeclRefType(VarDecl *Var, SourceLocation Loc);

  void MarkDeclarationsReferencedInType(SourceLocation Loc, QualType T);
  void MarkDeclarationsReferencedInExpr(Expr *E,
                                        bool SkipLocalVariables = false);

  /// \brief Try to recover by turning the given expression into a
  /// call.  Returns true if recovery was attempted or an error was
  /// emitted; this may also leave the ExprResult invalid.
  bool tryToRecoverWithCall(ExprResult &E, const PartialDiagnostic &PD,
                            bool ForceComplain = false,
                            bool (*IsPlausibleResult)(QualType) = nullptr);

  /// \brief Figure out if an expression could be turned into a call.
  bool tryExprAsCall(Expr &E, QualType &ZeroArgCallReturnTy,
                     UnresolvedSetImpl &NonTemplateOverloads);

  /// \brief Conditionally issue a diagnostic based on the current
  /// evaluation context.
  ///
  /// \param Statement If Statement is non-null, delay reporting the
  /// diagnostic until the function body is parsed, and then do a basic
  /// reachability analysis to determine if the statement is reachable.
  /// If it is unreachable, the diagnostic will not be emitted.
  bool DiagRuntimeBehavior(SourceLocation Loc, const Stmt *Statement,
                           const PartialDiagnostic &PD);

  // Primary Expressions.
  SourceRange getExprRange(Expr *E) const;

  ExprResult ActOnIdExpression(
      Scope *S, CXXScopeSpec &SS, SourceLocation TemplateKWLoc,
      UnqualifiedId &Id, bool HasTrailingLParen, bool IsAddressOfOperand,
      std::unique_ptr<CorrectionCandidateCallback> CCC = nullptr,
      bool IsInlineAsmIdentifier = false, Token *KeywordReplacement = nullptr);

  void DecomposeUnqualifiedId(const UnqualifiedId &Id,
                              TemplateArgumentListInfo &Buffer,
                              DeclarationNameInfo &NameInfo,
                              const TemplateArgumentListInfo *&TemplateArgs);

  bool
  DiagnoseEmptyLookup(Scope *S, CXXScopeSpec &SS, LookupResult &R,
                      std::unique_ptr<CorrectionCandidateCallback> CCC,
                      TemplateArgumentListInfo *ExplicitTemplateArgs = nullptr,
                      ArrayRef<Expr *> Args = None, TypoExpr **Out = nullptr);

  ExprResult LookupInObjCMethod(LookupResult &LookUp, Scope *S,
                                IdentifierInfo *II,
                                bool AllowBuiltinCreation=false);

  ExprResult ActOnDependentIdExpression(const CXXScopeSpec &SS,
                                        SourceLocation TemplateKWLoc,
                                        const DeclarationNameInfo &NameInfo,
                                        bool isAddressOfOperand,
                                const TemplateArgumentListInfo *TemplateArgs);

  ExprResult BuildDeclRefExpr(ValueDecl *D, QualType Ty,
                              ExprValueKind VK,
                              SourceLocation Loc,
                              const CXXScopeSpec *SS = nullptr);
  ExprResult
  BuildDeclRefExpr(ValueDecl *D, QualType Ty, ExprValueKind VK,
                   const DeclarationNameInfo &NameInfo,
                   const CXXScopeSpec *SS = nullptr,
                   NamedDecl *FoundD = nullptr,
                   const TemplateArgumentListInfo *TemplateArgs = nullptr);
  ExprResult
  BuildAnonymousStructUnionMemberReference(
      const CXXScopeSpec &SS,
      SourceLocation nameLoc,
      IndirectFieldDecl *indirectField,
      DeclAccessPair FoundDecl = DeclAccessPair::make(nullptr, AS_none),
      Expr *baseObjectExpr = nullptr,
      SourceLocation opLoc = SourceLocation());

  ExprResult BuildPossibleImplicitMemberExpr(const CXXScopeSpec &SS,
                                             SourceLocation TemplateKWLoc,
                                             LookupResult &R,
                                const TemplateArgumentListInfo *TemplateArgs);
  ExprResult BuildImplicitMemberExpr(const CXXScopeSpec &SS,
                                     SourceLocation TemplateKWLoc,
                                     LookupResult &R,
                                const TemplateArgumentListInfo *TemplateArgs,
                                     bool IsDefiniteInstance);
  bool UseArgumentDependentLookup(const CXXScopeSpec &SS,
                                  const LookupResult &R,
                                  bool HasTrailingLParen);

  ExprResult BuildQualifiedDeclarationNameExpr(
      CXXScopeSpec &SS, const DeclarationNameInfo &NameInfo,
      bool IsAddressOfOperand, TypeSourceInfo **RecoveryTSI = nullptr);

  ExprResult BuildDependentDeclRefExpr(const CXXScopeSpec &SS,
                                       SourceLocation TemplateKWLoc,
                                const DeclarationNameInfo &NameInfo,
                                const TemplateArgumentListInfo *TemplateArgs);

  ExprResult BuildDeclarationNameExpr(const CXXScopeSpec &SS,
                                      LookupResult &R,
                                      bool NeedsADL,
                                      bool AcceptInvalidDecl = false);
  ExprResult BuildDeclarationNameExpr(
      const CXXScopeSpec &SS, const DeclarationNameInfo &NameInfo, NamedDecl *D,
      NamedDecl *FoundD = nullptr,
      const TemplateArgumentListInfo *TemplateArgs = nullptr,
      bool AcceptInvalidDecl = false);

  ExprResult BuildLiteralOperatorCall(LookupResult &R,
                      DeclarationNameInfo &SuffixInfo,
                      ArrayRef<Expr *> Args,
                      SourceLocation LitEndLoc,
                      TemplateArgumentListInfo *ExplicitTemplateArgs = nullptr);

  ExprResult BuildPredefinedExpr(SourceLocation Loc,
                                 PredefinedExpr::IdentType IT);
  ExprResult ActOnPredefinedExpr(SourceLocation Loc, tok::TokenKind Kind);
  ExprResult ActOnIntegerConstant(SourceLocation Loc, uint64_t Val);

  bool CheckLoopHintExpr(Expr *E, SourceLocation Loc);

  ExprResult ActOnNumericConstant(const Token &Tok, Scope *UDLScope = nullptr);
  ExprResult ActOnCharacterConstant(const Token &Tok,
                                    Scope *UDLScope = nullptr);
  ExprResult ActOnParenExpr(SourceLocation L, SourceLocation R, Expr *E);
  ExprResult ActOnParenListExpr(SourceLocation L,
                                SourceLocation R,
                                MultiExprArg Val);

  /// ActOnStringLiteral - The specified tokens were lexed as pasted string
  /// fragments (e.g. "foo" "bar" L"baz").
  ExprResult ActOnStringLiteral(ArrayRef<Token> StringToks,
                                Scope *UDLScope = nullptr);

  ExprResult ActOnGenericSelectionExpr(SourceLocation KeyLoc,
                                       SourceLocation DefaultLoc,
                                       SourceLocation RParenLoc,
                                       Expr *ControllingExpr,
                                       ArrayRef<ParsedType> ArgTypes,
                                       ArrayRef<Expr *> ArgExprs);
  ExprResult CreateGenericSelectionExpr(SourceLocation KeyLoc,
                                        SourceLocation DefaultLoc,
                                        SourceLocation RParenLoc,
                                        Expr *ControllingExpr,
                                        ArrayRef<TypeSourceInfo *> Types,
                                        ArrayRef<Expr *> Exprs);

  // Binary/Unary Operators.  'Tok' is the token for the operator.
  ExprResult CreateBuiltinUnaryOp(SourceLocation OpLoc, UnaryOperatorKind Opc,
                                  Expr *InputExpr);
  ExprResult BuildUnaryOp(Scope *S, SourceLocation OpLoc,
                          UnaryOperatorKind Opc, Expr *Input);
  ExprResult ActOnUnaryOp(Scope *S, SourceLocation OpLoc,
                          tok::TokenKind Op, Expr *Input);

  QualType CheckAddressOfOperand(ExprResult &Operand, SourceLocation OpLoc);

  ExprResult CreateUnaryExprOrTypeTraitExpr(TypeSourceInfo *TInfo,
                                            SourceLocation OpLoc,
                                            UnaryExprOrTypeTrait ExprKind,
                                            SourceRange R);
  ExprResult CreateUnaryExprOrTypeTraitExpr(Expr *E, SourceLocation OpLoc,
                                            UnaryExprOrTypeTrait ExprKind);
  ExprResult
    ActOnUnaryExprOrTypeTraitExpr(SourceLocation OpLoc,
                                  UnaryExprOrTypeTrait ExprKind,
                                  bool IsType, void *TyOrEx,
                                  const SourceRange &ArgRange);

  ExprResult CheckPlaceholderExpr(Expr *E);
  bool CheckVecStepExpr(Expr *E);

  // HLSL Change Begins
  bool CheckHLSLUnaryExprOrTypeTraitOperand(QualType ExprType, SourceLocation Loc,
                                            UnaryExprOrTypeTrait ExprKind);
  void DiagnoseHLSLDeclAttr(const Decl *D, const Attr *A);
  void DiagnoseGloballyCoherentMismatch(const Expr *SrcExpr,
                                        QualType TargetType,
                                        SourceLocation Loc);
  void CheckHLSLFunctionCall(FunctionDecl *FDecl, CallExpr *TheCall,
                             const FunctionProtoType *Proto);
  void DiagnoseReachableHLSLCall(CallExpr *CE, const hlsl::ShaderModel *SM,
                                 hlsl::DXIL::ShaderKind EntrySK,
                                 hlsl::DXIL::NodeLaunchType NodeLaunchTy,
                                 const FunctionDecl *EntryDecl,
                                 bool locallyVisited);
  // HLSL Change Ends

  bool CheckUnaryExprOrTypeTraitOperand(Expr *E, UnaryExprOrTypeTrait ExprKind);
  bool CheckUnaryExprOrTypeTraitOperand(QualType ExprType, SourceLocation OpLoc,
                                        SourceRange ExprRange,
                                        UnaryExprOrTypeTrait ExprKind);
  ExprResult ActOnSizeofParameterPackExpr(Scope *S,
                                          SourceLocation OpLoc,
                                          IdentifierInfo &Name,
                                          SourceLocation NameLoc,
                                          SourceLocation RParenLoc);
  ExprResult ActOnPostfixUnaryOp(Scope *S, SourceLocation OpLoc,
                                 tok::TokenKind Kind, Expr *Input);

  ExprResult ActOnArraySubscriptExpr(Scope *S, Expr *Base, SourceLocation LLoc,
                                     Expr *Idx, SourceLocation RLoc);
  ExprResult CreateBuiltinArraySubscriptExpr(Expr *Base, SourceLocation LLoc,
                                             Expr *Idx, SourceLocation RLoc);

  // This struct is for use by ActOnMemberAccess to allow
  // BuildMemberReferenceExpr to be able to reinvoke ActOnMemberAccess after
  // changing the access operator from a '.' to a '->' (to see if that is the
  // change needed to fix an error about an unknown member, e.g. when the class
  // defines a custom operator->).
  struct ActOnMemberAccessExtraArgs {
    Scope *S;
    UnqualifiedId &Id;
    Decl *ObjCImpDecl;
  };

  ExprResult BuildMemberReferenceExpr(
      Expr *Base, QualType BaseType, SourceLocation OpLoc, bool IsArrow,
      CXXScopeSpec &SS, SourceLocation TemplateKWLoc,
      NamedDecl *FirstQualifierInScope, const DeclarationNameInfo &NameInfo,
      const TemplateArgumentListInfo *TemplateArgs,
      ActOnMemberAccessExtraArgs *ExtraArgs = nullptr);

  ExprResult
  BuildMemberReferenceExpr(Expr *Base, QualType BaseType, SourceLocation OpLoc,
                           bool IsArrow, const CXXScopeSpec &SS,
                           SourceLocation TemplateKWLoc,
                           NamedDecl *FirstQualifierInScope, LookupResult &R,
                           const TemplateArgumentListInfo *TemplateArgs,
                           bool SuppressQualifierCheck = false,
                           ActOnMemberAccessExtraArgs *ExtraArgs = nullptr);

  ExprResult PerformMemberExprBaseConversion(Expr *Base, bool IsArrow);

  bool CheckQualifiedMemberReference(Expr *BaseExpr, QualType BaseType,
                                     const CXXScopeSpec &SS,
                                     const LookupResult &R);

  ExprResult ActOnDependentMemberExpr(Expr *Base, QualType BaseType,
                                      bool IsArrow, SourceLocation OpLoc,
                                      const CXXScopeSpec &SS,
                                      SourceLocation TemplateKWLoc,
                                      NamedDecl *FirstQualifierInScope,
                               const DeclarationNameInfo &NameInfo,
                               const TemplateArgumentListInfo *TemplateArgs);

  ExprResult ActOnMemberAccessExpr(Scope *S, Expr *Base,
                                   SourceLocation OpLoc,
                                   tok::TokenKind OpKind,
                                   CXXScopeSpec &SS,
                                   SourceLocation TemplateKWLoc,
                                   UnqualifiedId &Member,
                                   Decl *ObjCImpDecl);

  void ActOnDefaultCtorInitializers(Decl *CDtorDecl);
  bool ConvertArgumentsForCall(CallExpr *Call, Expr *Fn,
                               FunctionDecl *FDecl,
                               const FunctionProtoType *Proto,
                               ArrayRef<Expr *> Args,
                               SourceLocation RParenLoc,
                               bool ExecConfig = false);
  void CheckStaticArrayArgument(SourceLocation CallLoc,
                                ParmVarDecl *Param,
                                const Expr *ArgExpr);

  /// ActOnCallExpr - Handle a call to Fn with the specified array of arguments.
  /// This provides the location of the left/right parens and a list of comma
  /// locations.
  ExprResult ActOnCallExpr(Scope *S, Expr *Fn, SourceLocation LParenLoc,
                           MultiExprArg ArgExprs, SourceLocation RParenLoc,
                           Expr *ExecConfig = nullptr,
                           bool IsExecConfig = false);
  ExprResult BuildResolvedCallExpr(Expr *Fn, NamedDecl *NDecl,
                                   SourceLocation LParenLoc,
                                   ArrayRef<Expr *> Arg,
                                   SourceLocation RParenLoc,
                                   Expr *Config = nullptr,
                                   bool IsExecConfig = false);

  ExprResult ActOnCUDAExecConfigExpr(Scope *S, SourceLocation LLLLoc,
                                     MultiExprArg ExecConfig,
                                     SourceLocation GGGLoc);

  ExprResult ActOnCastExpr(Scope *S, SourceLocation LParenLoc,
                           Declarator &D, ParsedType &Ty,
                           SourceLocation RParenLoc, Expr *CastExpr);
  ExprResult BuildCStyleCastExpr(SourceLocation LParenLoc,
                                 TypeSourceInfo *Ty,
                                 SourceLocation RParenLoc,
                                 Expr *Op);
  CastKind PrepareScalarCast(ExprResult &src, QualType destType);

  /// \brief Build an altivec or OpenCL literal.
  ExprResult BuildVectorLiteral(SourceLocation LParenLoc,
                                SourceLocation RParenLoc, Expr *E,
                                TypeSourceInfo *TInfo);

  ExprResult MaybeConvertParenListExprToParenExpr(Scope *S, Expr *ME);

  ExprResult ActOnCompoundLiteral(SourceLocation LParenLoc,
                                  ParsedType Ty,
                                  SourceLocation RParenLoc,
                                  Expr *InitExpr);

  ExprResult BuildCompoundLiteralExpr(SourceLocation LParenLoc,
                                      TypeSourceInfo *TInfo,
                                      SourceLocation RParenLoc,
                                      Expr *LiteralExpr);

  ExprResult ActOnInitList(SourceLocation LBraceLoc,
                           MultiExprArg InitArgList,
                           SourceLocation RBraceLoc);

  ExprResult ActOnDesignatedInitializer(Designation &Desig,
                                        SourceLocation Loc,
                                        bool GNUSyntax,
                                        ExprResult Init);

private:
  static BinaryOperatorKind ConvertTokenKindToBinaryOpcode(tok::TokenKind Kind);

public:
  ExprResult ActOnBinOp(Scope *S, SourceLocation TokLoc,
                        tok::TokenKind Kind, Expr *LHSExpr, Expr *RHSExpr);
  ExprResult BuildBinOp(Scope *S, SourceLocation OpLoc,
                        BinaryOperatorKind Opc, Expr *LHSExpr, Expr *RHSExpr);
  ExprResult CreateBuiltinBinOp(SourceLocation OpLoc, BinaryOperatorKind Opc,
                                Expr *LHSExpr, Expr *RHSExpr);

  /// ActOnConditionalOp - Parse a ?: operation.  Note that 'LHS' may be null
  /// in the case of a the GNU conditional expr extension.
  ExprResult ActOnConditionalOp(SourceLocation QuestionLoc,
                                SourceLocation ColonLoc,
                                Expr *CondExpr, Expr *LHSExpr, Expr *RHSExpr);

  /// ActOnAddrLabel - Parse the GNU address of label extension: "&&foo".
  ExprResult ActOnAddrLabel(SourceLocation OpLoc, SourceLocation LabLoc,
                            LabelDecl *TheDecl);

  void ActOnStartStmtExpr();
  ExprResult ActOnStmtExpr(SourceLocation LPLoc, Stmt *SubStmt,
                           SourceLocation RPLoc); // "({..})"
  void ActOnStmtExprError();

  // __builtin_offsetof(type, identifier(.identifier|[expr])*)
  struct OffsetOfComponent {
    SourceLocation LocStart, LocEnd;
    bool isBrackets;  // true if [expr], false if .ident
    union {
      IdentifierInfo *IdentInfo;
      Expr *E;
    } U;
  };

  /// __builtin_offsetof(type, a.b[123][456].c)
  ExprResult BuildBuiltinOffsetOf(SourceLocation BuiltinLoc,
                                  TypeSourceInfo *TInfo,
                                  OffsetOfComponent *CompPtr,
                                  unsigned NumComponents,
                                  SourceLocation RParenLoc);
  ExprResult ActOnBuiltinOffsetOf(Scope *S,
                                  SourceLocation BuiltinLoc,
                                  SourceLocation TypeLoc,
                                  ParsedType ParsedArgTy,
                                  OffsetOfComponent *CompPtr,
                                  unsigned NumComponents,
                                  SourceLocation RParenLoc);

  // __builtin_choose_expr(constExpr, expr1, expr2)
  ExprResult ActOnChooseExpr(SourceLocation BuiltinLoc,
                             Expr *CondExpr, Expr *LHSExpr,
                             Expr *RHSExpr, SourceLocation RPLoc);

  // __builtin_va_arg(expr, type)
  ExprResult ActOnVAArg(SourceLocation BuiltinLoc, Expr *E, ParsedType Ty,
                        SourceLocation RPLoc);
  ExprResult BuildVAArgExpr(SourceLocation BuiltinLoc, Expr *E,
                            TypeSourceInfo *TInfo, SourceLocation RPLoc);

  // __null
  ExprResult ActOnGNUNullExpr(SourceLocation TokenLoc);

  bool CheckCaseExpression(Expr *E);

  /// \brief Describes the result of an "if-exists" condition check.
  enum IfExistsResult {
    /// \brief The symbol exists.
    IER_Exists,

    /// \brief The symbol does not exist.
    IER_DoesNotExist,

    /// \brief The name is a dependent name, so the results will differ
    /// from one instantiation to the next.
    IER_Dependent,

    /// \brief An error occurred.
    IER_Error
  };

  IfExistsResult
  CheckMicrosoftIfExistsSymbol(Scope *S, CXXScopeSpec &SS,
                               const DeclarationNameInfo &TargetNameInfo);

  IfExistsResult
  CheckMicrosoftIfExistsSymbol(Scope *S, SourceLocation KeywordLoc,
                               bool IsIfExists, CXXScopeSpec &SS,
                               UnqualifiedId &Name);

  StmtResult BuildMSDependentExistsStmt(SourceLocation KeywordLoc,
                                        bool IsIfExists,
                                        NestedNameSpecifierLoc QualifierLoc,
                                        DeclarationNameInfo NameInfo,
                                        Stmt *Nested);
  StmtResult ActOnMSDependentExistsStmt(SourceLocation KeywordLoc,
                                        bool IsIfExists,
                                        CXXScopeSpec &SS, UnqualifiedId &Name,
                                        Stmt *Nested);

  //===------------------------- "Block" Extension ------------------------===//

  /// ActOnBlockStart - This callback is invoked when a block literal is
  /// started.
  void ActOnBlockStart(SourceLocation CaretLoc, Scope *CurScope);

  /// ActOnBlockArguments - This callback allows processing of block arguments.
  /// If there are no arguments, this is still invoked.
  void ActOnBlockArguments(SourceLocation CaretLoc, Declarator &ParamInfo,
                           Scope *CurScope);

  /// ActOnBlockError - If there is an error parsing a block, this callback
  /// is invoked to pop the information about the block from the action impl.
  void ActOnBlockError(SourceLocation CaretLoc, Scope *CurScope);

  /// ActOnBlockStmtExpr - This is called when the body of a block statement
  /// literal was successfully completed.  ^(int x){...}
  ExprResult ActOnBlockStmtExpr(SourceLocation CaretLoc, Stmt *Body,
                                Scope *CurScope);

  //===---------------------------- Clang Extensions ----------------------===//

  /// __builtin_convertvector(...)
  ExprResult ActOnConvertVectorExpr(Expr *E, ParsedType ParsedDestTy,
                                    SourceLocation BuiltinLoc,
                                    SourceLocation RParenLoc);

  //===---------------------------- OpenCL Features -----------------------===//

  /// __builtin_astype(...)
  ExprResult ActOnAsTypeExpr(Expr *E, ParsedType ParsedDestTy,
                             SourceLocation BuiltinLoc,
                             SourceLocation RParenLoc);

  // HLSL Change Starts
  //===---------------------------- HLSL Features -------------------------===//

  /// cbuffer/tbuffer
  llvm::SmallVector<Decl*, 1> HLSLBuffers;
  Decl* ActOnStartHLSLBuffer(Scope* bufferScope, bool cbuffer, SourceLocation KwLoc,
    IdentifierInfo *Ident, SourceLocation IdentLoc,
    std::vector<hlsl::UnusualAnnotation *>& BufferAttributes,
    SourceLocation LBrace);
  void ActOnFinishHLSLBuffer(Decl *Dcl, SourceLocation RBrace);
  Decl* getActiveHLSLBuffer() const;
  bool IsOnHLSLBufferView();
  // HLSL Change Ends

  //===---------------------------- C++ Features --------------------------===//

  // Act on C++ namespaces
  Decl *ActOnStartNamespaceDef(Scope *S, SourceLocation InlineLoc,
                               SourceLocation NamespaceLoc,
                               SourceLocation IdentLoc,
                               IdentifierInfo *Ident,
                               SourceLocation LBrace,
                               AttributeList *AttrList);
  void ActOnFinishNamespaceDef(Decl *Dcl, SourceLocation RBrace);

  NamespaceDecl *getStdNamespace() const;
  NamespaceDecl *getOrCreateStdNamespace();

  CXXRecordDecl *getStdBadAlloc() const;

  /// \brief Tests whether Ty is an instance of std::initializer_list and, if
  /// it is and Element is not NULL, assigns the element type to Element.
  bool isStdInitializerList(QualType Ty, QualType *Element);

  /// \brief Looks for the std::initializer_list template and instantiates it
  /// with Element, or emits an error if it's not found.
  ///
  /// \returns The instantiated template, or null on error.
  QualType BuildStdInitializerList(QualType Element, SourceLocation Loc);

  /// \brief Determine whether Ctor is an initializer-list constructor, as
  /// defined in [dcl.init.list]p2.
  bool isInitListConstructor(const CXXConstructorDecl *Ctor);

  Decl *ActOnUsingDirective(Scope *CurScope,
                            SourceLocation UsingLoc,
                            SourceLocation NamespcLoc,
                            CXXScopeSpec &SS,
                            SourceLocation IdentLoc,
                            IdentifierInfo *NamespcName,
                            AttributeList *AttrList);

  void PushUsingDirective(Scope *S, UsingDirectiveDecl *UDir);

  Decl *ActOnNamespaceAliasDef(Scope *CurScope,
                               SourceLocation NamespaceLoc,
                               SourceLocation AliasLoc,
                               IdentifierInfo *Alias,
                               CXXScopeSpec &SS,
                               SourceLocation IdentLoc,
                               IdentifierInfo *Ident);

  void HideUsingShadowDecl(Scope *S, UsingShadowDecl *Shadow);
  bool CheckUsingShadowDecl(UsingDecl *UD, NamedDecl *Target,
                            const LookupResult &PreviousDecls,
                            UsingShadowDecl *&PrevShadow);
  UsingShadowDecl *BuildUsingShadowDecl(Scope *S, UsingDecl *UD,
                                        NamedDecl *Target,
                                        UsingShadowDecl *PrevDecl);

  bool CheckUsingDeclRedeclaration(SourceLocation UsingLoc,
                                   bool HasTypenameKeyword,
                                   const CXXScopeSpec &SS,
                                   SourceLocation NameLoc,
                                   const LookupResult &Previous);
  bool CheckUsingDeclQualifier(SourceLocation UsingLoc,
                               const CXXScopeSpec &SS,
                               const DeclarationNameInfo &NameInfo,
                               SourceLocation NameLoc);

  NamedDecl *BuildUsingDeclaration(Scope *S, AccessSpecifier AS,
                                   SourceLocation UsingLoc,
                                   CXXScopeSpec &SS,
                                   DeclarationNameInfo NameInfo,
                                   AttributeList *AttrList,
                                   bool IsInstantiation,
                                   bool HasTypenameKeyword,
                                   SourceLocation TypenameLoc);

  bool CheckInheritingConstructorUsingDecl(UsingDecl *UD);

  Decl *ActOnUsingDeclaration(Scope *CurScope,
                              AccessSpecifier AS,
                              bool HasUsingKeyword,
                              SourceLocation UsingLoc,
                              CXXScopeSpec &SS,
                              UnqualifiedId &Name,
                              AttributeList *AttrList,
                              bool HasTypenameKeyword,
                              SourceLocation TypenameLoc);
  Decl *ActOnAliasDeclaration(Scope *CurScope,
                              AccessSpecifier AS,
                              MultiTemplateParamsArg TemplateParams,
                              SourceLocation UsingLoc,
                              UnqualifiedId &Name,
                              AttributeList *AttrList,
                              TypeResult Type,
                              Decl *DeclFromDeclSpec);

  /// BuildCXXConstructExpr - Creates a complete call to a constructor,
  /// including handling of its default argument expressions.
  ///
  /// \param ConstructKind - a CXXConstructExpr::ConstructionKind
  ExprResult
  BuildCXXConstructExpr(SourceLocation ConstructLoc, QualType DeclInitType,
                        CXXConstructorDecl *Constructor, MultiExprArg Exprs,
                        bool HadMultipleCandidates, bool IsListInitialization,
                        bool IsStdInitListInitialization,
                        bool RequiresZeroInit, unsigned ConstructKind,
                        SourceRange ParenRange);

  // FIXME: Can we remove this and have the above BuildCXXConstructExpr check if
  // the constructor can be elidable?
  ExprResult
  BuildCXXConstructExpr(SourceLocation ConstructLoc, QualType DeclInitType,
                        CXXConstructorDecl *Constructor, bool Elidable,
                        MultiExprArg Exprs, bool HadMultipleCandidates,
                        bool IsListInitialization,
                        bool IsStdInitListInitialization, bool RequiresZeroInit,
                        unsigned ConstructKind, SourceRange ParenRange);

  ExprResult BuildCXXDefaultInitExpr(SourceLocation Loc, FieldDecl *Field);

  /// BuildCXXDefaultArgExpr - Creates a CXXDefaultArgExpr, instantiating
  /// the default expr if needed.
  ExprResult BuildCXXDefaultArgExpr(SourceLocation CallLoc,
                                    FunctionDecl *FD,
                                    ParmVarDecl *Param);

  /// FinalizeVarWithDestructor - Prepare for calling destructor on the
  /// constructed variable.
  void FinalizeVarWithDestructor(VarDecl *VD, const RecordType *DeclInitType);

  /// \brief Helper class that collects exception specifications for
  /// implicitly-declared special member functions.
  class ImplicitExceptionSpecification {
    // Pointer to allow copying
    Sema *Self;
    // We order exception specifications thus:
    // noexcept is the most restrictive, but is only used in C++11.
    // throw() comes next.
    // Then a throw(collected exceptions)
    // Finally no specification, which is expressed as noexcept(false).
    // throw(...) is used instead if any called function uses it.
    ExceptionSpecificationType ComputedEST;
    llvm::SmallPtrSet<CanQualType, 4> ExceptionsSeen;
    SmallVector<QualType, 4> Exceptions;

    void ClearExceptions() {
      ExceptionsSeen.clear();
      Exceptions.clear();
    }

  public:
    explicit ImplicitExceptionSpecification(Sema &Self)
      : Self(&Self), ComputedEST(EST_BasicNoexcept) {
      if (!Self.getLangOpts().CPlusPlus11)
        ComputedEST = EST_DynamicNone;
    }

    /// \brief Get the computed exception specification type.
    ExceptionSpecificationType getExceptionSpecType() const {
      assert(ComputedEST != EST_ComputedNoexcept &&
             "noexcept(expr) should not be a possible result");
      return ComputedEST;
    }

    /// \brief The number of exceptions in the exception specification.
    unsigned size() const { return Exceptions.size(); }

    /// \brief The set of exceptions in the exception specification.
    const QualType *data() const { return Exceptions.data(); }

    /// \brief Integrate another called method into the collected data.
    void CalledDecl(SourceLocation CallLoc, const CXXMethodDecl *Method);

    /// \brief Integrate an invoked expression into the collected data.
    void CalledExpr(Expr *E);

    /// \brief Overwrite an EPI's exception specification with this
    /// computed exception specification.
    FunctionProtoType::ExceptionSpecInfo getExceptionSpec() const {
      FunctionProtoType::ExceptionSpecInfo ESI;
      ESI.Type = getExceptionSpecType();
      if (ESI.Type == EST_Dynamic) {
        ESI.Exceptions = Exceptions;
      } else if (ESI.Type == EST_None) {
        /// C++11 [except.spec]p14:
        ///   The exception-specification is noexcept(false) if the set of
        ///   potential exceptions of the special member function contains "any"
        ESI.Type = EST_ComputedNoexcept;
        ESI.NoexceptExpr = Self->ActOnCXXBoolLiteral(SourceLocation(),
                                                     tok::kw_false).get();
      }
      return ESI;
    }
  };

  /// \brief Determine what sort of exception specification a defaulted
  /// copy constructor of a class will have.
  ImplicitExceptionSpecification
  ComputeDefaultedDefaultCtorExceptionSpec(SourceLocation Loc,
                                           CXXMethodDecl *MD);

  /// \brief Determine what sort of exception specification a defaulted
  /// default constructor of a class will have, and whether the parameter
  /// will be const.
  ImplicitExceptionSpecification
  ComputeDefaultedCopyCtorExceptionSpec(CXXMethodDecl *MD);

  /// \brief Determine what sort of exception specification a defautled
  /// copy assignment operator of a class will have, and whether the
  /// parameter will be const.
  ImplicitExceptionSpecification
  ComputeDefaultedCopyAssignmentExceptionSpec(CXXMethodDecl *MD);

  /// \brief Determine what sort of exception specification a defaulted move
  /// constructor of a class will have.
  ImplicitExceptionSpecification
  ComputeDefaultedMoveCtorExceptionSpec(CXXMethodDecl *MD);

  /// \brief Determine what sort of exception specification a defaulted move
  /// assignment operator of a class will have.
  ImplicitExceptionSpecification
  ComputeDefaultedMoveAssignmentExceptionSpec(CXXMethodDecl *MD);

  /// \brief Determine what sort of exception specification a defaulted
  /// destructor of a class will have.
  ImplicitExceptionSpecification
  ComputeDefaultedDtorExceptionSpec(CXXMethodDecl *MD);

  /// \brief Determine what sort of exception specification an inheriting
  /// constructor of a class will have.
  ImplicitExceptionSpecification
  ComputeInheritingCtorExceptionSpec(CXXConstructorDecl *CD);

  /// \brief Evaluate the implicit exception specification for a defaulted
  /// special member function.
  void EvaluateImplicitExceptionSpec(SourceLocation Loc, CXXMethodDecl *MD);

  /// \brief Check the given exception-specification and update the
  /// exception specification information with the results.
  void checkExceptionSpecification(bool IsTopLevel,
                                   ExceptionSpecificationType EST,
                                   ArrayRef<ParsedType> DynamicExceptions,
                                   ArrayRef<SourceRange> DynamicExceptionRanges,
                                   Expr *NoexceptExpr,
                                   SmallVectorImpl<QualType> &Exceptions,
                                   FunctionProtoType::ExceptionSpecInfo &ESI);

  /// \brief Determine if we're in a case where we need to (incorrectly) eagerly
  /// parse an exception specification to work around a libstdc++ bug.
  bool isLibstdcxxEagerExceptionSpecHack(const Declarator &D);

  /// \brief Add an exception-specification to the given member function
  /// (or member function template). The exception-specification was parsed
  /// after the method itself was declared.
  void actOnDelayedExceptionSpecification(Decl *Method,
         ExceptionSpecificationType EST,
         SourceRange SpecificationRange,
         ArrayRef<ParsedType> DynamicExceptions,
         ArrayRef<SourceRange> DynamicExceptionRanges,
         Expr *NoexceptExpr);

  /// \brief Determine if a special member function should have a deleted
  /// definition when it is defaulted.
  bool ShouldDeleteSpecialMember(CXXMethodDecl *MD, CXXSpecialMember CSM,
                                 bool Diagnose = false);

  /// \brief Declare the implicit default constructor for the given class.
  ///
  /// \param ClassDecl The class declaration into which the implicit
  /// default constructor will be added.
  ///
  /// \returns The implicitly-declared default constructor.
  CXXConstructorDecl *DeclareImplicitDefaultConstructor(
                                                     CXXRecordDecl *ClassDecl);

  /// DefineImplicitDefaultConstructor - Checks for feasibility of
  /// defining this constructor as the default constructor.
  void DefineImplicitDefaultConstructor(SourceLocation CurrentLocation,
                                        CXXConstructorDecl *Constructor);

  /// \brief Declare the implicit destructor for the given class.
  ///
  /// \param ClassDecl The class declaration into which the implicit
  /// destructor will be added.
  ///
  /// \returns The implicitly-declared destructor.
  CXXDestructorDecl *DeclareImplicitDestructor(CXXRecordDecl *ClassDecl);

  /// DefineImplicitDestructor - Checks for feasibility of
  /// defining this destructor as the default destructor.
  void DefineImplicitDestructor(SourceLocation CurrentLocation,
                                CXXDestructorDecl *Destructor);

  /// \brief Build an exception spec for destructors that don't have one.
  ///
  /// C++11 says that user-defined destructors with no exception spec get one
  /// that looks as if the destructor was implicitly declared.
  void AdjustDestructorExceptionSpec(CXXRecordDecl *ClassDecl,
                                     CXXDestructorDecl *Destructor);

  /// \brief Declare all inheriting constructors for the given class.
  ///
  /// \param ClassDecl The class declaration into which the inheriting
  /// constructors will be added.
  void DeclareInheritingConstructors(CXXRecordDecl *ClassDecl);

  /// \brief Define the specified inheriting constructor.
  void DefineInheritingConstructor(SourceLocation UseLoc,
                                   CXXConstructorDecl *Constructor);

  /// \brief Declare the implicit copy constructor for the given class.
  ///
  /// \param ClassDecl The class declaration into which the implicit
  /// copy constructor will be added.
  ///
  /// \returns The implicitly-declared copy constructor.
  CXXConstructorDecl *DeclareImplicitCopyConstructor(CXXRecordDecl *ClassDecl);

  /// DefineImplicitCopyConstructor - Checks for feasibility of
  /// defining this constructor as the copy constructor.
  void DefineImplicitCopyConstructor(SourceLocation CurrentLocation,
                                     CXXConstructorDecl *Constructor);

  /// \brief Declare the implicit move constructor for the given class.
  ///
  /// \param ClassDecl The Class declaration into which the implicit
  /// move constructor will be added.
  ///
  /// \returns The implicitly-declared move constructor, or NULL if it wasn't
  /// declared.
  CXXConstructorDecl *DeclareImplicitMoveConstructor(CXXRecordDecl *ClassDecl);

  /// DefineImplicitMoveConstructor - Checks for feasibility of
  /// defining this constructor as the move constructor.
  void DefineImplicitMoveConstructor(SourceLocation CurrentLocation,
                                     CXXConstructorDecl *Constructor);

  /// \brief Declare the implicit copy assignment operator for the given class.
  ///
  /// \param ClassDecl The class declaration into which the implicit
  /// copy assignment operator will be added.
  ///
  /// \returns The implicitly-declared copy assignment operator.
  CXXMethodDecl *DeclareImplicitCopyAssignment(CXXRecordDecl *ClassDecl);

  /// \brief Defines an implicitly-declared copy assignment operator.
  void DefineImplicitCopyAssignment(SourceLocation CurrentLocation,
                                    CXXMethodDecl *MethodDecl);

  /// \brief Declare the implicit move assignment operator for the given class.
  ///
  /// \param ClassDecl The Class declaration into which the implicit
  /// move assignment operator will be added.
  ///
  /// \returns The implicitly-declared move assignment operator, or NULL if it
  /// wasn't declared.
  CXXMethodDecl *DeclareImplicitMoveAssignment(CXXRecordDecl *ClassDecl);

  /// \brief Defines an implicitly-declared move assignment operator.
  void DefineImplicitMoveAssignment(SourceLocation CurrentLocation,
                                    CXXMethodDecl *MethodDecl);

  /// \brief Force the declaration of any implicitly-declared members of this
  /// class.
  void ForceDeclarationOfImplicitMembers(CXXRecordDecl *Class);

  /// \brief Determine whether the given function is an implicitly-deleted
  /// special member function.
  bool isImplicitlyDeleted(FunctionDecl *FD);

  /// \brief Check whether 'this' shows up in the type of a static member
  /// function after the (naturally empty) cv-qualifier-seq would be.
  ///
  /// \returns true if an error occurred.
  bool checkThisInStaticMemberFunctionType(CXXMethodDecl *Method);

  /// \brief Whether this' shows up in the exception specification of a static
  /// member function.
  bool checkThisInStaticMemberFunctionExceptionSpec(CXXMethodDecl *Method);

  /// \brief Check whether 'this' shows up in the attributes of the given
  /// static member function.
  ///
  /// \returns true if an error occurred.
  bool checkThisInStaticMemberFunctionAttributes(CXXMethodDecl *Method);

  /// MaybeBindToTemporary - If the passed in expression has a record type with
  /// a non-trivial destructor, this will return CXXBindTemporaryExpr. Otherwise
  /// it simply returns the passed in expression.
  ExprResult MaybeBindToTemporary(Expr *E);

  bool CompleteConstructorCall(CXXConstructorDecl *Constructor,
                               MultiExprArg ArgsPtr,
                               SourceLocation Loc,
                               SmallVectorImpl<Expr*> &ConvertedArgs,
                               bool AllowExplicit = false,
                               bool IsListInitialization = false);

  ParsedType getInheritingConstructorName(CXXScopeSpec &SS,
                                          SourceLocation NameLoc,
                                          IdentifierInfo &Name);

  ParsedType getDestructorName(SourceLocation TildeLoc,
                               IdentifierInfo &II, SourceLocation NameLoc,
                               Scope *S, CXXScopeSpec &SS,
                               ParsedType ObjectType,
                               bool EnteringContext);

  ParsedType getDestructorType(const DeclSpec& DS, ParsedType ObjectType);

  // Checks that reinterpret casts don't have undefined behavior.
  void CheckCompatibleReinterpretCast(QualType SrcType, QualType DestType,
                                      bool IsDereference, SourceRange Range);

  /// ActOnCXXNamedCast - Parse {dynamic,static,reinterpret,const}_cast's.
  ExprResult ActOnCXXNamedCast(SourceLocation OpLoc,
                               tok::TokenKind Kind,
                               SourceLocation LAngleBracketLoc,
                               Declarator &D,
                               SourceLocation RAngleBracketLoc,
                               SourceLocation LParenLoc,
                               Expr *E,
                               SourceLocation RParenLoc);

  ExprResult BuildCXXNamedCast(SourceLocation OpLoc,
                               tok::TokenKind Kind,
                               TypeSourceInfo *Ty,
                               Expr *E,
                               SourceRange AngleBrackets,
                               SourceRange Parens);

  ExprResult BuildCXXTypeId(QualType TypeInfoType,
                            SourceLocation TypeidLoc,
                            TypeSourceInfo *Operand,
                            SourceLocation RParenLoc);
  ExprResult BuildCXXTypeId(QualType TypeInfoType,
                            SourceLocation TypeidLoc,
                            Expr *Operand,
                            SourceLocation RParenLoc);

  /// ActOnCXXTypeid - Parse typeid( something ).
  ExprResult ActOnCXXTypeid(SourceLocation OpLoc,
                            SourceLocation LParenLoc, bool isType,
                            void *TyOrExpr,
                            SourceLocation RParenLoc);

  ExprResult BuildCXXUuidof(QualType TypeInfoType,
                            SourceLocation TypeidLoc,
                            TypeSourceInfo *Operand,
                            SourceLocation RParenLoc);
  ExprResult BuildCXXUuidof(QualType TypeInfoType,
                            SourceLocation TypeidLoc,
                            Expr *Operand,
                            SourceLocation RParenLoc);

  /// ActOnCXXUuidof - Parse __uuidof( something ).
  ExprResult ActOnCXXUuidof(SourceLocation OpLoc,
                            SourceLocation LParenLoc, bool isType,
                            void *TyOrExpr,
                            SourceLocation RParenLoc);

  /// \brief Handle a C++1z fold-expression: ( expr op ... op expr ).
  ExprResult ActOnCXXFoldExpr(SourceLocation LParenLoc, Expr *LHS,
                              tok::TokenKind Operator,
                              SourceLocation EllipsisLoc, Expr *RHS,
                              SourceLocation RParenLoc);
  ExprResult BuildCXXFoldExpr(SourceLocation LParenLoc, Expr *LHS,
                              BinaryOperatorKind Operator,
                              SourceLocation EllipsisLoc, Expr *RHS,
                              SourceLocation RParenLoc);
  ExprResult BuildEmptyCXXFoldExpr(SourceLocation EllipsisLoc,
                                   BinaryOperatorKind Operator);

  //// ActOnCXXThis -  Parse 'this' pointer.
  ExprResult ActOnCXXThis(SourceLocation loc);

  /// \brief Try to retrieve the type of the 'this' pointer.
  ///
  /// \returns The type of 'this', if possible. Otherwise, returns a NULL type.
  QualType getCurrentThisType();

  /// \brief When non-NULL, the C++ 'this' expression is allowed despite the
  /// current context not being a non-static member function. In such cases,
  /// this provides the type used for 'this'.
  QualType CXXThisTypeOverride;

  /// \brief RAII object used to temporarily allow the C++ 'this' expression
  /// to be used, with the given qualifiers on the current class type.
  class CXXThisScopeRAII {
    Sema &S;
    QualType OldCXXThisTypeOverride;
    bool Enabled;

  public:
    /// \brief Introduce a new scope where 'this' may be allowed (when enabled),
    /// using the given declaration (which is either a class template or a
    /// class) along with the given qualifiers.
    /// along with the qualifiers placed on '*this'.
    CXXThisScopeRAII(Sema &S, Decl *ContextDecl, unsigned CXXThisTypeQuals,
                     bool Enabled = true);

    ~CXXThisScopeRAII();
  };

  /// \brief Make sure the value of 'this' is actually available in the current
  /// context, if it is a potentially evaluated context.
  ///
  /// \param Loc The location at which the capture of 'this' occurs.
  ///
  /// \param Explicit Whether 'this' is explicitly captured in a lambda
  /// capture list.
  ///
  /// \param FunctionScopeIndexToStopAt If non-null, it points to the index
  /// of the FunctionScopeInfo stack beyond which we do not attempt to capture.
  /// This is useful when enclosing lambdas must speculatively capture 
  /// 'this' that may or may not be used in certain specializations of
  /// a nested generic lambda (depending on whether the name resolves to 
  /// a non-static member function or a static function).
  /// \return returns 'true' if failed, 'false' if success.
  bool CheckCXXThisCapture(SourceLocation Loc, bool Explicit = false, 
      bool BuildAndDiagnose = true,
      const unsigned *const FunctionScopeIndexToStopAt = nullptr);

  /// \brief Determine whether the given type is the type of *this that is used
  /// outside of the body of a member function for a type that is currently
  /// being defined.
  bool isThisOutsideMemberFunctionBody(QualType BaseType);

  /// ActOnCXXBoolLiteral - Parse {true,false} literals.
  ExprResult ActOnCXXBoolLiteral(SourceLocation OpLoc, tok::TokenKind Kind);


  /// ActOnObjCBoolLiteral - Parse {__objc_yes,__objc_no} literals.
  ExprResult ActOnObjCBoolLiteral(SourceLocation OpLoc, tok::TokenKind Kind);

  /// ActOnCXXNullPtrLiteral - Parse 'nullptr'.
  ExprResult ActOnCXXNullPtrLiteral(SourceLocation Loc);

  //// ActOnCXXThrow -  Parse throw expressions.
  ExprResult ActOnCXXThrow(Scope *S, SourceLocation OpLoc, Expr *expr);
  ExprResult BuildCXXThrow(SourceLocation OpLoc, Expr *Ex,
                           bool IsThrownVarInScope);
  bool CheckCXXThrowOperand(SourceLocation ThrowLoc, QualType ThrowTy, Expr *E);

  /// ActOnCXXTypeConstructExpr - Parse construction of a specified type.
  /// Can be interpreted either as function-style casting ("int(x)")
  /// or class type construction ("ClassType(x,y,z)")
  /// or creation of a value-initialized type ("int()").
  ExprResult ActOnCXXTypeConstructExpr(ParsedType TypeRep,
                                       SourceLocation LParenLoc,
                                       MultiExprArg Exprs,
                                       SourceLocation RParenLoc);

  ExprResult BuildCXXTypeConstructExpr(TypeSourceInfo *Type,
                                       SourceLocation LParenLoc,
                                       MultiExprArg Exprs,
                                       SourceLocation RParenLoc);

  /// ActOnCXXNew - Parsed a C++ 'new' expression.
  ExprResult ActOnCXXNew(SourceLocation StartLoc, bool UseGlobal,
                         SourceLocation PlacementLParen,
                         MultiExprArg PlacementArgs,
                         SourceLocation PlacementRParen,
                         SourceRange TypeIdParens, Declarator &D,
                         Expr *Initializer);
  ExprResult BuildCXXNew(SourceRange Range, bool UseGlobal,
                         SourceLocation PlacementLParen,
                         MultiExprArg PlacementArgs,
                         SourceLocation PlacementRParen,
                         SourceRange TypeIdParens,
                         QualType AllocType,
                         TypeSourceInfo *AllocTypeInfo,
                         Expr *ArraySize,
                         SourceRange DirectInitRange,
                         Expr *Initializer,
                         bool TypeMayContainAuto = true);

  bool CheckAllocatedType(QualType AllocType, SourceLocation Loc,
                          SourceRange R);
  bool FindAllocationFunctions(SourceLocation StartLoc, SourceRange Range,
                               bool UseGlobal, QualType AllocType, bool IsArray,
                               MultiExprArg PlaceArgs,
                               FunctionDecl *&OperatorNew,
                               FunctionDecl *&OperatorDelete);
  bool FindAllocationOverload(SourceLocation StartLoc, SourceRange Range,
                              DeclarationName Name, MultiExprArg Args,
                              DeclContext *Ctx,
                              bool AllowMissing, FunctionDecl *&Operator,
                              bool Diagnose = true);
  void DeclareGlobalNewDelete();
  void DeclareGlobalAllocationFunction(DeclarationName Name, QualType Return,
                                       QualType Param1,
                                       QualType Param2 = QualType(),
                                       bool addRestrictAttr = false);

  bool FindDeallocationFunction(SourceLocation StartLoc, CXXRecordDecl *RD,
                                DeclarationName Name, FunctionDecl* &Operator,
                                bool Diagnose = true);
  FunctionDecl *FindUsualDeallocationFunction(SourceLocation StartLoc,
                                              bool CanProvideSize,
                                              DeclarationName Name);

  /// ActOnCXXDelete - Parsed a C++ 'delete' expression
  ExprResult ActOnCXXDelete(SourceLocation StartLoc,
                            bool UseGlobal, bool ArrayForm,
                            Expr *Operand);

  DeclResult ActOnCXXConditionDeclaration(Scope *S, Declarator &D);
  ExprResult CheckConditionVariable(VarDecl *ConditionVar,
                                    SourceLocation StmtLoc,
                                    bool ConvertToBoolean);

  ExprResult ActOnNoexceptExpr(SourceLocation KeyLoc, SourceLocation LParen,
                               Expr *Operand, SourceLocation RParen);
  ExprResult BuildCXXNoexceptExpr(SourceLocation KeyLoc, Expr *Operand,
                                  SourceLocation RParen);

  /// \brief Parsed one of the type trait support pseudo-functions.
  ExprResult ActOnTypeTrait(TypeTrait Kind, SourceLocation KWLoc,
                            ArrayRef<ParsedType> Args,
                            SourceLocation RParenLoc);
  ExprResult BuildTypeTrait(TypeTrait Kind, SourceLocation KWLoc,
                            ArrayRef<TypeSourceInfo *> Args,
                            SourceLocation RParenLoc);

  /// ActOnArrayTypeTrait - Parsed one of the bianry type trait support
  /// pseudo-functions.
  ExprResult ActOnArrayTypeTrait(ArrayTypeTrait ATT,
                                 SourceLocation KWLoc,
                                 ParsedType LhsTy,
                                 Expr *DimExpr,
                                 SourceLocation RParen);

  ExprResult BuildArrayTypeTrait(ArrayTypeTrait ATT,
                                 SourceLocation KWLoc,
                                 TypeSourceInfo *TSInfo,
                                 Expr *DimExpr,
                                 SourceLocation RParen);

  /// ActOnExpressionTrait - Parsed one of the unary type trait support
  /// pseudo-functions.
  ExprResult ActOnExpressionTrait(ExpressionTrait OET,
                                  SourceLocation KWLoc,
                                  Expr *Queried,
                                  SourceLocation RParen);

  ExprResult BuildExpressionTrait(ExpressionTrait OET,
                                  SourceLocation KWLoc,
                                  Expr *Queried,
                                  SourceLocation RParen);

  ExprResult ActOnStartCXXMemberReference(Scope *S,
                                          Expr *Base,
                                          SourceLocation OpLoc,
                                          tok::TokenKind OpKind,
                                          ParsedType &ObjectType,
                                          bool &MayBePseudoDestructor);

  ExprResult BuildPseudoDestructorExpr(Expr *Base,
                                       SourceLocation OpLoc,
                                       tok::TokenKind OpKind,
                                       const CXXScopeSpec &SS,
                                       TypeSourceInfo *ScopeType,
                                       SourceLocation CCLoc,
                                       SourceLocation TildeLoc,
                                     PseudoDestructorTypeStorage DestroyedType);

  ExprResult ActOnPseudoDestructorExpr(Scope *S, Expr *Base,
                                       SourceLocation OpLoc,
                                       tok::TokenKind OpKind,
                                       CXXScopeSpec &SS,
                                       UnqualifiedId &FirstTypeName,
                                       SourceLocation CCLoc,
                                       SourceLocation TildeLoc,
                                       UnqualifiedId &SecondTypeName);

  ExprResult ActOnPseudoDestructorExpr(Scope *S, Expr *Base,
                                       SourceLocation OpLoc,
                                       tok::TokenKind OpKind,
                                       SourceLocation TildeLoc,
                                       const DeclSpec& DS);

  /// MaybeCreateExprWithCleanups - If the current full-expression
  /// requires any cleanups, surround it with a ExprWithCleanups node.
  /// Otherwise, just returns the passed-in expression.
  Expr *MaybeCreateExprWithCleanups(Expr *SubExpr);
  Stmt *MaybeCreateStmtWithCleanups(Stmt *SubStmt);
  ExprResult MaybeCreateExprWithCleanups(ExprResult SubExpr);

  ExprResult ActOnFinishFullExpr(Expr *Expr) {
    return ActOnFinishFullExpr(Expr, Expr ? Expr->getExprLoc()
                                          : SourceLocation());
  }
  ExprResult ActOnFinishFullExpr(Expr *Expr, SourceLocation CC,
                                 bool DiscardedValue = false,
                                 bool IsConstexpr = false,
                                 bool IsLambdaInitCaptureInitializer = false);
  StmtResult ActOnFinishFullStmt(Stmt *Stmt);

  // Marks SS invalid if it represents an incomplete type.
  bool RequireCompleteDeclContext(CXXScopeSpec &SS, DeclContext *DC);

  DeclContext *computeDeclContext(QualType T);
  DeclContext *computeDeclContext(const CXXScopeSpec &SS,
                                  bool EnteringContext = false);
  bool isDependentScopeSpecifier(const CXXScopeSpec &SS);
  CXXRecordDecl *getCurrentInstantiationOf(NestedNameSpecifier *NNS);

  /// \brief The parser has parsed a global nested-name-specifier '::'.
  ///
  /// \param CCLoc The location of the '::'.
  ///
  /// \param SS The nested-name-specifier, which will be updated in-place
  /// to reflect the parsed nested-name-specifier.
  ///
  /// \returns true if an error occurred, false otherwise.
  bool ActOnCXXGlobalScopeSpecifier(SourceLocation CCLoc, CXXScopeSpec &SS);

  /// \brief The parser has parsed a '__super' nested-name-specifier.
  ///
  /// \param SuperLoc The location of the '__super' keyword.
  ///
  /// \param ColonColonLoc The location of the '::'.
  ///
  /// \param SS The nested-name-specifier, which will be updated in-place
  /// to reflect the parsed nested-name-specifier.
  ///
  /// \returns true if an error occurred, false otherwise.
  bool ActOnSuperScopeSpecifier(SourceLocation SuperLoc,
                                SourceLocation ColonColonLoc, CXXScopeSpec &SS);

  bool isAcceptableNestedNameSpecifier(const NamedDecl *SD,
                                       bool *CanCorrect = nullptr);
  NamedDecl *FindFirstQualifierInScope(Scope *S, NestedNameSpecifier *NNS);

  bool isNonTypeNestedNameSpecifier(Scope *S, CXXScopeSpec &SS,
                                    SourceLocation IdLoc,
                                    IdentifierInfo &II,
                                    ParsedType ObjectType);

  bool BuildCXXNestedNameSpecifier(Scope *S,
                                   IdentifierInfo &Identifier,
                                   SourceLocation IdentifierLoc,
                                   SourceLocation CCLoc,
                                   QualType ObjectType,
                                   bool EnteringContext,
                                   CXXScopeSpec &SS,
                                   NamedDecl *ScopeLookupResult,
                                   bool ErrorRecoveryLookup,
                                   bool *IsCorrectedToColon = nullptr);

  /// \brief The parser has parsed a nested-name-specifier 'identifier::'.
  ///
  /// \param S The scope in which this nested-name-specifier occurs.
  ///
  /// \param Identifier The identifier preceding the '::'.
  ///
  /// \param IdentifierLoc The location of the identifier.
  ///
  /// \param CCLoc The location of the '::'.
  ///
  /// \param ObjectType The type of the object, if we're parsing
  /// nested-name-specifier in a member access expression.
  ///
  /// \param EnteringContext Whether we're entering the context nominated by
  /// this nested-name-specifier.
  ///
  /// \param SS The nested-name-specifier, which is both an input
  /// parameter (the nested-name-specifier before this type) and an
  /// output parameter (containing the full nested-name-specifier,
  /// including this new type).
  ///
  /// \param ErrorRecoveryLookup If true, then this method is called to improve
  /// error recovery. In this case do not emit error message.
  ///
  /// \param IsCorrectedToColon If not null, suggestions to replace '::' -> ':'
  /// are allowed.  The bool value pointed by this parameter is set to 'true'
  /// if the identifier is treated as if it was followed by ':', not '::'.
  ///
  /// \returns true if an error occurred, false otherwise.
  bool ActOnCXXNestedNameSpecifier(Scope *S,
                                   IdentifierInfo &Identifier,
                                   SourceLocation IdentifierLoc,
                                   SourceLocation CCLoc,
                                   ParsedType ObjectType,
                                   bool EnteringContext,
                                   CXXScopeSpec &SS,
                                   bool ErrorRecoveryLookup = false,
                                   bool *IsCorrectedToColon = nullptr);

  ExprResult ActOnDecltypeExpression(Expr *E);

  bool ActOnCXXNestedNameSpecifierDecltype(CXXScopeSpec &SS,
                                           const DeclSpec &DS,
                                           SourceLocation ColonColonLoc);

  bool IsInvalidUnlessNestedName(Scope *S, CXXScopeSpec &SS,
                                 IdentifierInfo &Identifier,
                                 SourceLocation IdentifierLoc,
                                 SourceLocation ColonLoc,
                                 ParsedType ObjectType,
                                 bool EnteringContext);

  /// \brief The parser has parsed a nested-name-specifier
  /// 'template[opt] template-name < template-args >::'.
  ///
  /// \param S The scope in which this nested-name-specifier occurs.
  ///
  /// \param SS The nested-name-specifier, which is both an input
  /// parameter (the nested-name-specifier before this type) and an
  /// output parameter (containing the full nested-name-specifier,
  /// including this new type).
  ///
  /// \param TemplateKWLoc the location of the 'template' keyword, if any.
  /// \param TemplateName the template name.
  /// \param TemplateNameLoc The location of the template name.
  /// \param LAngleLoc The location of the opening angle bracket  ('<').
  /// \param TemplateArgs The template arguments.
  /// \param RAngleLoc The location of the closing angle bracket  ('>').
  /// \param CCLoc The location of the '::'.
  ///
  /// \param EnteringContext Whether we're entering the context of the
  /// nested-name-specifier.
  ///
  ///
  /// \returns true if an error occurred, false otherwise.
  bool ActOnCXXNestedNameSpecifier(Scope *S,
                                   CXXScopeSpec &SS,
                                   SourceLocation TemplateKWLoc,
                                   TemplateTy TemplateName,
                                   SourceLocation TemplateNameLoc,
                                   SourceLocation LAngleLoc,
                                   ASTTemplateArgsPtr TemplateArgs,
                                   SourceLocation RAngleLoc,
                                   SourceLocation CCLoc,
                                   bool EnteringContext);

  /// \brief Given a C++ nested-name-specifier, produce an annotation value
  /// that the parser can use later to reconstruct the given
  /// nested-name-specifier.
  ///
  /// \param SS A nested-name-specifier.
  ///
  /// \returns A pointer containing all of the information in the
  /// nested-name-specifier \p SS.
  void *SaveNestedNameSpecifierAnnotation(CXXScopeSpec &SS);

  /// \brief Given an annotation pointer for a nested-name-specifier, restore
  /// the nested-name-specifier structure.
  ///
  /// \param Annotation The annotation pointer, produced by
  /// \c SaveNestedNameSpecifierAnnotation().
  ///
  /// \param AnnotationRange The source range corresponding to the annotation.
  ///
  /// \param SS The nested-name-specifier that will be updated with the contents
  /// of the annotation pointer.
  void RestoreNestedNameSpecifierAnnotation(void *Annotation,
                                            SourceRange AnnotationRange,
                                            CXXScopeSpec &SS);

  bool ShouldEnterDeclaratorScope(Scope *S, const CXXScopeSpec &SS);

  /// ActOnCXXEnterDeclaratorScope - Called when a C++ scope specifier (global
  /// scope or nested-name-specifier) is parsed, part of a declarator-id.
  /// After this method is called, according to [C++ 3.4.3p3], names should be
  /// looked up in the declarator-id's scope, until the declarator is parsed and
  /// ActOnCXXExitDeclaratorScope is called.
  /// The 'SS' should be a non-empty valid CXXScopeSpec.
  bool ActOnCXXEnterDeclaratorScope(Scope *S, CXXScopeSpec &SS);

  /// ActOnCXXExitDeclaratorScope - Called when a declarator that previously
  /// invoked ActOnCXXEnterDeclaratorScope(), is finished. 'SS' is the same
  /// CXXScopeSpec that was passed to ActOnCXXEnterDeclaratorScope as well.
  /// Used to indicate that names should revert to being looked up in the
  /// defining scope.
  void ActOnCXXExitDeclaratorScope(Scope *S, const CXXScopeSpec &SS);

  /// ActOnCXXEnterDeclInitializer - Invoked when we are about to parse an
  /// initializer for the declaration 'Dcl'.
  /// After this method is called, according to [C++ 3.4.1p13], if 'Dcl' is a
  /// static data member of class X, names should be looked up in the scope of
  /// class X.
  void ActOnCXXEnterDeclInitializer(Scope *S, Decl *Dcl);

  /// ActOnCXXExitDeclInitializer - Invoked after we are finished parsing an
  /// initializer for the declaration 'Dcl'.
  void ActOnCXXExitDeclInitializer(Scope *S, Decl *Dcl);

  /// \brief Create a new lambda closure type.
  CXXRecordDecl *createLambdaClosureType(SourceRange IntroducerRange,
                                         TypeSourceInfo *Info,
                                         bool KnownDependent, 
                                         LambdaCaptureDefault CaptureDefault);

  /// \brief Start the definition of a lambda expression.
  CXXMethodDecl *startLambdaDefinition(CXXRecordDecl *Class,
                                       SourceRange IntroducerRange,
                                       TypeSourceInfo *MethodType,
                                       SourceLocation EndLoc,
                                       ArrayRef<ParmVarDecl *> Params);

  /// \brief Endow the lambda scope info with the relevant properties.
  void buildLambdaScope(sema::LambdaScopeInfo *LSI, 
                        CXXMethodDecl *CallOperator,
                        SourceRange IntroducerRange,
                        LambdaCaptureDefault CaptureDefault,
                        SourceLocation CaptureDefaultLoc,
                        bool ExplicitParams,
                        bool ExplicitResultType,
                        bool Mutable);

  /// \brief Perform initialization analysis of the init-capture and perform
  /// any implicit conversions such as an lvalue-to-rvalue conversion if
  /// not being used to initialize a reference.
  QualType performLambdaInitCaptureInitialization(SourceLocation Loc, 
      bool ByRef, IdentifierInfo *Id, Expr *&Init);
  /// \brief Create a dummy variable within the declcontext of the lambda's
  ///  call operator, for name lookup purposes for a lambda init capture.
  ///  
  ///  CodeGen handles emission of lambda captures, ignoring these dummy
  ///  variables appropriately.
  VarDecl *createLambdaInitCaptureVarDecl(SourceLocation Loc, 
    QualType InitCaptureType, IdentifierInfo *Id, Expr *Init);

  /// \brief Build the implicit field for an init-capture.
  FieldDecl *buildInitCaptureField(sema::LambdaScopeInfo *LSI, VarDecl *Var);

  /// \brief Note that we have finished the explicit captures for the
  /// given lambda.
  void finishLambdaExplicitCaptures(sema::LambdaScopeInfo *LSI);

  /// \brief Introduce the lambda parameters into scope.
  void addLambdaParameters(CXXMethodDecl *CallOperator, Scope *CurScope);

  /// \brief Deduce a block or lambda's return type based on the return
  /// statements present in the body.
  void deduceClosureReturnType(sema::CapturingScopeInfo &CSI);

  /// ActOnStartOfLambdaDefinition - This is called just before we start
  /// parsing the body of a lambda; it analyzes the explicit captures and
  /// arguments, and sets up various data-structures for the body of the
  /// lambda.
  void ActOnStartOfLambdaDefinition(LambdaIntroducer &Intro,
                                    Declarator &ParamInfo, Scope *CurScope);

  /// ActOnLambdaError - If there is an error parsing a lambda, this callback
  /// is invoked to pop the information about the lambda.
  void ActOnLambdaError(SourceLocation StartLoc, Scope *CurScope,
                        bool IsInstantiation = false);

  /// ActOnLambdaExpr - This is called when the body of a lambda expression
  /// was successfully completed.
  ExprResult ActOnLambdaExpr(SourceLocation StartLoc, Stmt *Body,
                             Scope *CurScope);

  /// \brief Complete a lambda-expression having processed and attached the
  /// lambda body.
  ExprResult BuildLambdaExpr(SourceLocation StartLoc, SourceLocation EndLoc,
                             sema::LambdaScopeInfo *LSI);

  /// \brief Define the "body" of the conversion from a lambda object to a
  /// function pointer.
  ///
  /// This routine doesn't actually define a sensible body; rather, it fills
  /// in the initialization expression needed to copy the lambda object into
  /// the block, and IR generation actually generates the real body of the
  /// block pointer conversion.
  void DefineImplicitLambdaToFunctionPointerConversion(
         SourceLocation CurrentLoc, CXXConversionDecl *Conv);

  /// \brief Define the "body" of the conversion from a lambda object to a
  /// block pointer.
  ///
  /// This routine doesn't actually define a sensible body; rather, it fills
  /// in the initialization expression needed to copy the lambda object into
  /// the block, and IR generation actually generates the real body of the
  /// block pointer conversion.
  void DefineImplicitLambdaToBlockPointerConversion(SourceLocation CurrentLoc,
                                                    CXXConversionDecl *Conv);

  ExprResult BuildBlockForLambdaConversion(SourceLocation CurrentLocation,
                                           SourceLocation ConvLocation,
                                           CXXConversionDecl *Conv,
                                           Expr *Src);

  // ParseObjCStringLiteral - Parse Objective-C string literals.
  ExprResult ParseObjCStringLiteral(SourceLocation *AtLocs,
                                    Expr **Strings,
                                    unsigned NumStrings);

  ExprResult BuildObjCStringLiteral(SourceLocation AtLoc, StringLiteral *S);

  /// BuildObjCNumericLiteral - builds an ObjCBoxedExpr AST node for the
  /// numeric literal expression. Type of the expression will be "NSNumber *"
  /// or "id" if NSNumber is unavailable.
  ExprResult BuildObjCNumericLiteral(SourceLocation AtLoc, Expr *Number);
  ExprResult ActOnObjCBoolLiteral(SourceLocation AtLoc, SourceLocation ValueLoc,
                                  bool Value);
  ExprResult BuildObjCArrayLiteral(SourceRange SR, MultiExprArg Elements);

  /// BuildObjCBoxedExpr - builds an ObjCBoxedExpr AST node for the
  /// '@' prefixed parenthesized expression. The type of the expression will
  /// either be "NSNumber *", "NSString *" or "NSValue *" depending on the type
  /// of ValueType, which is allowed to be a built-in numeric type, "char *",
  /// "const char *" or C structure with attribute 'objc_boxable'.
  ExprResult BuildObjCBoxedExpr(SourceRange SR, Expr *ValueExpr);

  ExprResult BuildObjCSubscriptExpression(SourceLocation RB, Expr *BaseExpr,
                                          Expr *IndexExpr,
                                          ObjCMethodDecl *getterMethod,
                                          ObjCMethodDecl *setterMethod);

  ExprResult BuildObjCDictionaryLiteral(SourceRange SR,
                                        ObjCDictionaryElement *Elements,
                                        unsigned NumElements);

  ExprResult BuildObjCEncodeExpression(SourceLocation AtLoc,
                                  TypeSourceInfo *EncodedTypeInfo,
                                  SourceLocation RParenLoc);
  ExprResult BuildCXXMemberCallExpr(Expr *Exp, NamedDecl *FoundDecl,
                                    CXXConversionDecl *Method,
                                    bool HadMultipleCandidates);

  ExprResult ParseObjCEncodeExpression(SourceLocation AtLoc,
                                       SourceLocation EncodeLoc,
                                       SourceLocation LParenLoc,
                                       ParsedType Ty,
                                       SourceLocation RParenLoc);

  /// ParseObjCSelectorExpression - Build selector expression for \@selector
  ExprResult ParseObjCSelectorExpression(Selector Sel,
                                         SourceLocation AtLoc,
                                         SourceLocation SelLoc,
                                         SourceLocation LParenLoc,
                                         SourceLocation RParenLoc,
                                         bool WarnMultipleSelectors);

  /// ParseObjCProtocolExpression - Build protocol expression for \@protocol
  ExprResult ParseObjCProtocolExpression(IdentifierInfo * ProtocolName,
                                         SourceLocation AtLoc,
                                         SourceLocation ProtoLoc,
                                         SourceLocation LParenLoc,
                                         SourceLocation ProtoIdLoc,
                                         SourceLocation RParenLoc);

  //===--------------------------------------------------------------------===//
  // C++ Declarations
  //
  Decl *ActOnStartLinkageSpecification(Scope *S,
                                       SourceLocation ExternLoc,
                                       Expr *LangStr,
                                       SourceLocation LBraceLoc);
  Decl *ActOnFinishLinkageSpecification(Scope *S,
                                        Decl *LinkageSpec,
                                        SourceLocation RBraceLoc);


  //===--------------------------------------------------------------------===//
  // C++ Classes
  //
  bool isCurrentClassName(const IdentifierInfo &II, Scope *S,
                          const CXXScopeSpec *SS = nullptr);
  bool isCurrentClassNameTypo(IdentifierInfo *&II, const CXXScopeSpec *SS);

  bool ActOnAccessSpecifier(AccessSpecifier Access,
                            SourceLocation ASLoc,
                            SourceLocation ColonLoc,
                            AttributeList *Attrs = nullptr);

  NamedDecl *ActOnCXXMemberDeclarator(Scope *S, AccessSpecifier AS,
                                 Declarator &D,
                                 MultiTemplateParamsArg TemplateParameterLists,
                                 Expr *BitfieldWidth, const VirtSpecifiers &VS,
                                 InClassInitStyle InitStyle);

  void ActOnStartCXXInClassMemberInitializer();
  void ActOnFinishCXXInClassMemberInitializer(Decl *VarDecl,
                                              SourceLocation EqualLoc,
                                              Expr *Init);

  MemInitResult ActOnMemInitializer(Decl *ConstructorD,
                                    Scope *S,
                                    CXXScopeSpec &SS,
                                    IdentifierInfo *MemberOrBase,
                                    ParsedType TemplateTypeTy,
                                    const DeclSpec &DS,
                                    SourceLocation IdLoc,
                                    SourceLocation LParenLoc,
                                    ArrayRef<Expr *> Args,
                                    SourceLocation RParenLoc,
                                    SourceLocation EllipsisLoc);

  MemInitResult ActOnMemInitializer(Decl *ConstructorD,
                                    Scope *S,
                                    CXXScopeSpec &SS,
                                    IdentifierInfo *MemberOrBase,
                                    ParsedType TemplateTypeTy,
                                    const DeclSpec &DS,
                                    SourceLocation IdLoc,
                                    Expr *InitList,
                                    SourceLocation EllipsisLoc);

  MemInitResult BuildMemInitializer(Decl *ConstructorD,
                                    Scope *S,
                                    CXXScopeSpec &SS,
                                    IdentifierInfo *MemberOrBase,
                                    ParsedType TemplateTypeTy,
                                    const DeclSpec &DS,
                                    SourceLocation IdLoc,
                                    Expr *Init,
                                    SourceLocation EllipsisLoc);

  MemInitResult BuildMemberInitializer(ValueDecl *Member,
                                       Expr *Init,
                                       SourceLocation IdLoc);

  MemInitResult BuildBaseInitializer(QualType BaseType,
                                     TypeSourceInfo *BaseTInfo,
                                     Expr *Init,
                                     CXXRecordDecl *ClassDecl,
                                     SourceLocation EllipsisLoc);

  MemInitResult BuildDelegatingInitializer(TypeSourceInfo *TInfo,
                                           Expr *Init,
                                           CXXRecordDecl *ClassDecl);

  bool SetDelegatingInitializer(CXXConstructorDecl *Constructor,
                                CXXCtorInitializer *Initializer);

  bool SetCtorInitializers(CXXConstructorDecl *Constructor, bool AnyErrors,
                           ArrayRef<CXXCtorInitializer *> Initializers = None);

  void SetIvarInitializers(ObjCImplementationDecl *ObjCImplementation);


  /// MarkBaseAndMemberDestructorsReferenced - Given a record decl,
  /// mark all the non-trivial destructors of its members and bases as
  /// referenced.
  void MarkBaseAndMemberDestructorsReferenced(SourceLocation Loc,
                                              CXXRecordDecl *Record);

  /// \brief The list of classes whose vtables have been used within
  /// this translation unit, and the source locations at which the
  /// first use occurred.
  typedef std::pair<CXXRecordDecl*, SourceLocation> VTableUse;

  /// \brief The list of vtables that are required but have not yet been
  /// materialized.
  SmallVector<VTableUse, 16> VTableUses;

  /// \brief The set of classes whose vtables have been used within
  /// this translation unit, and a bit that will be true if the vtable is
  /// required to be emitted (otherwise, it should be emitted only if needed
  /// by code generation).
  llvm::DenseMap<CXXRecordDecl *, bool> VTablesUsed;

  /// \brief Load any externally-stored vtable uses.
  void LoadExternalVTableUses();

  /// \brief Note that the vtable for the given class was used at the
  /// given location.
  void MarkVTableUsed(SourceLocation Loc, CXXRecordDecl *Class,
                      bool DefinitionRequired = false);

  /// \brief Mark the exception specifications of all virtual member functions
  /// in the given class as needed.
  void MarkVirtualMemberExceptionSpecsNeeded(SourceLocation Loc,
                                             const CXXRecordDecl *RD);

  /// MarkVirtualMembersReferenced - Will mark all members of the given
  /// CXXRecordDecl referenced.
  void MarkVirtualMembersReferenced(SourceLocation Loc,
                                    const CXXRecordDecl *RD);

  /// \brief Define all of the vtables that have been used in this
  /// translation unit and reference any virtual members used by those
  /// vtables.
  ///
  /// \returns true if any work was done, false otherwise.
  bool DefineUsedVTables();

  void AddImplicitlyDeclaredMembersToClass(CXXRecordDecl *ClassDecl);

  void ActOnMemInitializers(Decl *ConstructorDecl,
                            SourceLocation ColonLoc,
                            ArrayRef<CXXCtorInitializer*> MemInits,
                            bool AnyErrors);

  void checkClassLevelDLLAttribute(CXXRecordDecl *Class);
  void propagateDLLAttrToBaseClassTemplate(
      CXXRecordDecl *Class, Attr *ClassAttr,
      ClassTemplateSpecializationDecl *BaseTemplateSpec,
      SourceLocation BaseLoc);
  void CheckCompletedCXXClass(CXXRecordDecl *Record);
  void ActOnFinishCXXMemberSpecification(Scope* S, SourceLocation RLoc,
                                         Decl *TagDecl,
                                         SourceLocation LBrac,
                                         SourceLocation RBrac,
                                         AttributeList *AttrList);
  void ActOnFinishCXXMemberDecls();
  void ActOnFinishCXXMemberDefaultArgs(Decl *D);

  void ActOnReenterCXXMethodParameter(Scope *S, ParmVarDecl *Param);
  unsigned ActOnReenterTemplateScope(Scope *S, Decl *Template);
  void ActOnStartDelayedMemberDeclarations(Scope *S, Decl *Record);
  void ActOnStartDelayedCXXMethodDeclaration(Scope *S, Decl *Method);
  void ActOnDelayedCXXMethodParameter(Scope *S, Decl *Param);
  void ActOnFinishDelayedMemberDeclarations(Scope *S, Decl *Record);
  void ActOnFinishDelayedCXXMethodDeclaration(Scope *S, Decl *Method);
  void ActOnFinishDelayedMemberInitializers(Decl *Record);
  void MarkAsLateParsedTemplate(FunctionDecl *FD, Decl *FnD,
                                CachedTokens &Toks);
  void UnmarkAsLateParsedTemplate(FunctionDecl *FD);
  bool IsInsideALocalClassWithinATemplateFunction();

  Decl *ActOnStaticAssertDeclaration(SourceLocation StaticAssertLoc,
                                     Expr *AssertExpr,
                                     Expr *AssertMessageExpr,
                                     SourceLocation RParenLoc);
  Decl *BuildStaticAssertDeclaration(SourceLocation StaticAssertLoc,
                                     Expr *AssertExpr,
                                     StringLiteral *AssertMessageExpr,
                                     SourceLocation RParenLoc,
                                     bool Failed);

  FriendDecl *CheckFriendTypeDecl(SourceLocation LocStart,
                                  SourceLocation FriendLoc,
                                  TypeSourceInfo *TSInfo);
  Decl *ActOnFriendTypeDecl(Scope *S, const DeclSpec &DS,
                            MultiTemplateParamsArg TemplateParams);
  NamedDecl *ActOnFriendFunctionDecl(Scope *S, Declarator &D,
                                     MultiTemplateParamsArg TemplateParams);

  QualType CheckConstructorDeclarator(Declarator &D, QualType R,
                                      StorageClass& SC);
  void CheckConstructor(CXXConstructorDecl *Constructor);
  QualType CheckDestructorDeclarator(Declarator &D, QualType R,
                                     StorageClass& SC);
  bool CheckDestructor(CXXDestructorDecl *Destructor);
  void CheckConversionDeclarator(Declarator &D, QualType &R,
                                 StorageClass& SC);
  Decl *ActOnConversionDeclarator(CXXConversionDecl *Conversion);

  void CheckExplicitlyDefaultedSpecialMember(CXXMethodDecl *MD);
  void CheckExplicitlyDefaultedMemberExceptionSpec(CXXMethodDecl *MD,
                                                   const FunctionProtoType *T);
  void CheckDelayedMemberExceptionSpecs();

  //===--------------------------------------------------------------------===//
  // C++ Derived Classes
  //

  /// ActOnBaseSpecifier - Parsed a base specifier
  CXXBaseSpecifier *CheckBaseSpecifier(CXXRecordDecl *Class,
                                       SourceRange SpecifierRange,
                                       bool Virtual, AccessSpecifier Access,
                                       TypeSourceInfo *TInfo,
                                       SourceLocation EllipsisLoc);

  BaseResult ActOnBaseSpecifier(Decl *classdecl,
                                SourceRange SpecifierRange,
                                ParsedAttributes &Attrs,
                                bool Virtual, AccessSpecifier Access,
                                ParsedType basetype,
                                SourceLocation BaseLoc,
                                SourceLocation EllipsisLoc);

  bool AttachBaseSpecifiers(CXXRecordDecl *Class, CXXBaseSpecifier **Bases,
                            unsigned NumBases);
  void ActOnBaseSpecifiers(Decl *ClassDecl, CXXBaseSpecifier **Bases,
                           unsigned NumBases);

  bool IsDerivedFrom(QualType Derived, QualType Base);
  bool IsDerivedFrom(QualType Derived, QualType Base, CXXBasePaths &Paths);

  // FIXME: I don't like this name.
  void BuildBasePathArray(const CXXBasePaths &Paths, CXXCastPath &BasePath);

  bool CheckDerivedToBaseConversion(QualType Derived, QualType Base,
                                    SourceLocation Loc, SourceRange Range,
                                    CXXCastPath *BasePath = nullptr,
                                    bool IgnoreAccess = false);
  bool CheckDerivedToBaseConversion(QualType Derived, QualType Base,
                                    unsigned InaccessibleBaseID,
                                    unsigned AmbigiousBaseConvID,
                                    SourceLocation Loc, SourceRange Range,
                                    DeclarationName Name,
                                    CXXCastPath *BasePath);

  std::string getAmbiguousPathsDisplayString(CXXBasePaths &Paths);

  bool CheckOverridingFunctionAttributes(const CXXMethodDecl *New,
                                         const CXXMethodDecl *Old);

  /// CheckOverridingFunctionReturnType - Checks whether the return types are
  /// covariant, according to C++ [class.virtual]p5.
  bool CheckOverridingFunctionReturnType(const CXXMethodDecl *New,
                                         const CXXMethodDecl *Old);

  /// CheckOverridingFunctionExceptionSpec - Checks whether the exception
  /// spec is a subset of base spec.
  bool CheckOverridingFunctionExceptionSpec(const CXXMethodDecl *New,
                                            const CXXMethodDecl *Old);

  bool CheckPureMethod(CXXMethodDecl *Method, SourceRange InitRange);

  /// CheckOverrideControl - Check C++11 override control semantics.
  void CheckOverrideControl(NamedDecl *D);
    
  /// DiagnoseAbsenceOfOverrideControl - Diagnose if 'override' keyword was
  /// not used in the declaration of an overriding method.
  void DiagnoseAbsenceOfOverrideControl(NamedDecl *D);

  /// CheckForFunctionMarkedFinal - Checks whether a virtual member function
  /// overrides a virtual member function marked 'final', according to
  /// C++11 [class.virtual]p4.
  bool CheckIfOverriddenFunctionIsMarkedFinal(const CXXMethodDecl *New,
                                              const CXXMethodDecl *Old);


  //===--------------------------------------------------------------------===//
  // C++ Access Control
  //

  enum AccessResult {
    AR_accessible,
    AR_inaccessible,
    AR_dependent,
    AR_delayed
  };

  bool SetMemberAccessSpecifier(NamedDecl *MemberDecl,
                                NamedDecl *PrevMemberDecl,
                                AccessSpecifier LexicalAS);

  AccessResult CheckUnresolvedMemberAccess(UnresolvedMemberExpr *E,
                                           DeclAccessPair FoundDecl);
  AccessResult CheckUnresolvedLookupAccess(UnresolvedLookupExpr *E,
                                           DeclAccessPair FoundDecl);
  AccessResult CheckAllocationAccess(SourceLocation OperatorLoc,
                                     SourceRange PlacementRange,
                                     CXXRecordDecl *NamingClass,
                                     DeclAccessPair FoundDecl,
                                     bool Diagnose = true);
  AccessResult CheckConstructorAccess(SourceLocation Loc,
                                      CXXConstructorDecl *D,
                                      const InitializedEntity &Entity,
                                      AccessSpecifier Access,
                                      bool IsCopyBindingRefToTemp = false);
  AccessResult CheckConstructorAccess(SourceLocation Loc,
                                      CXXConstructorDecl *D,
                                      const InitializedEntity &Entity,
                                      AccessSpecifier Access,
                                      const PartialDiagnostic &PDiag);
  AccessResult CheckDestructorAccess(SourceLocation Loc,
                                     CXXDestructorDecl *Dtor,
                                     const PartialDiagnostic &PDiag,
                                     QualType objectType = QualType());
  AccessResult CheckFriendAccess(NamedDecl *D);
  AccessResult CheckMemberAccess(SourceLocation UseLoc,
                                 CXXRecordDecl *NamingClass,
                                 DeclAccessPair Found);
  AccessResult CheckMemberOperatorAccess(SourceLocation Loc,
                                         Expr *ObjectExpr,
                                         Expr *ArgExpr,
                                         DeclAccessPair FoundDecl);
  AccessResult CheckAddressOfMemberAccess(Expr *OvlExpr,
                                          DeclAccessPair FoundDecl);
  AccessResult CheckBaseClassAccess(SourceLocation AccessLoc,
                                    QualType Base, QualType Derived,
                                    const CXXBasePath &Path,
                                    unsigned DiagID,
                                    bool ForceCheck = false,
                                    bool ForceUnprivileged = false);
  void CheckLookupAccess(const LookupResult &R);
  bool IsSimplyAccessible(NamedDecl *decl, DeclContext *Ctx);
  bool isSpecialMemberAccessibleForDeletion(CXXMethodDecl *decl,
                                            AccessSpecifier access,
                                            QualType objectType);

  void HandleDependentAccessCheck(const DependentDiagnostic &DD,
                         const MultiLevelTemplateArgumentList &TemplateArgs);
  void PerformDependentDiagnostics(const DeclContext *Pattern,
                        const MultiLevelTemplateArgumentList &TemplateArgs);

  void HandleDelayedAccessCheck(sema::DelayedDiagnostic &DD, Decl *Ctx);

  /// \brief When true, access checking violations are treated as SFINAE
  /// failures rather than hard errors.
  bool AccessCheckingSFINAE;

  enum AbstractDiagSelID {
    AbstractNone = -1,
    AbstractReturnType,
    AbstractParamType,
    AbstractVariableType,
    AbstractFieldType,
    AbstractIvarType,
    AbstractSynthesizedIvarType,
    AbstractArrayType
  };

  bool RequireNonAbstractType(SourceLocation Loc, QualType T,
                              TypeDiagnoser &Diagnoser);
  template <typename... Ts>
  bool RequireNonAbstractType(SourceLocation Loc, QualType T, unsigned DiagID,
                              const Ts &...Args) {
    BoundTypeDiagnoser<Ts...> Diagnoser(DiagID, Args...);
    return RequireNonAbstractType(Loc, T, Diagnoser);
  }

  void DiagnoseAbstractType(const CXXRecordDecl *RD);

  bool RequireNonAbstractType(SourceLocation Loc, QualType T, unsigned DiagID,
                              AbstractDiagSelID SelID = AbstractNone);

  //===--------------------------------------------------------------------===//
  // C++ Overloaded Operators [C++ 13.5]
  //

  bool CheckOverloadedOperatorDeclaration(FunctionDecl *FnDecl);

  bool CheckLiteralOperatorDeclaration(FunctionDecl *FnDecl);

  //===--------------------------------------------------------------------===//
  // C++ Templates [C++ 14]
  //
  void FilterAcceptableTemplateNames(LookupResult &R,
                                     bool AllowFunctionTemplates = true);
  bool hasAnyAcceptableTemplateNames(LookupResult &R,
                                     bool AllowFunctionTemplates = true);

  void LookupTemplateName(LookupResult &R, Scope *S, CXXScopeSpec &SS,
                          QualType ObjectType, bool EnteringContext,
                          bool &MemberOfUnknownSpecialization);

  TemplateNameKind isTemplateName(Scope *S,
                                  CXXScopeSpec &SS,
                                  bool hasTemplateKeyword,
                                  UnqualifiedId &Name,
                                  ParsedType ObjectType,
                                  bool EnteringContext,
                                  TemplateTy &Template,
                                  bool &MemberOfUnknownSpecialization);

  bool DiagnoseUnknownTemplateName(const IdentifierInfo &II,
                                   SourceLocation IILoc,
                                   Scope *S,
                                   const CXXScopeSpec *SS,
                                   TemplateTy &SuggestedTemplate,
                                   TemplateNameKind &SuggestedKind);

  void DiagnoseTemplateParameterShadow(SourceLocation Loc, Decl *PrevDecl);
  TemplateDecl *AdjustDeclIfTemplate(Decl *&Decl);

  Decl *ActOnTypeParameter(Scope *S, bool Typename,
                           SourceLocation EllipsisLoc,
                           SourceLocation KeyLoc,
                           IdentifierInfo *ParamName,
                           SourceLocation ParamNameLoc,
                           unsigned Depth, unsigned Position,
                           SourceLocation EqualLoc,
                           ParsedType DefaultArg);

  QualType CheckNonTypeTemplateParameterType(QualType T, SourceLocation Loc);
  Decl *ActOnNonTypeTemplateParameter(Scope *S, Declarator &D,
                                      unsigned Depth,
                                      unsigned Position,
                                      SourceLocation EqualLoc,
                                      Expr *DefaultArg);
  Decl *ActOnTemplateTemplateParameter(Scope *S,
                                       SourceLocation TmpLoc,
                                       TemplateParameterList *Params,
                                       SourceLocation EllipsisLoc,
                                       IdentifierInfo *ParamName,
                                       SourceLocation ParamNameLoc,
                                       unsigned Depth,
                                       unsigned Position,
                                       SourceLocation EqualLoc,
                                       ParsedTemplateArgument DefaultArg);

  TemplateParameterList *
  ActOnTemplateParameterList(unsigned Depth,
                             SourceLocation ExportLoc,
                             SourceLocation TemplateLoc,
                             SourceLocation LAngleLoc,
                             Decl **Params, unsigned NumParams,
                             SourceLocation RAngleLoc);

  /// \brief The context in which we are checking a template parameter list.
  enum TemplateParamListContext {
    TPC_ClassTemplate,
    TPC_VarTemplate,
    TPC_FunctionTemplate,
    TPC_ClassTemplateMember,
    TPC_FriendClassTemplate,
    TPC_FriendFunctionTemplate,
    TPC_FriendFunctionTemplateDefinition,
    TPC_TypeAliasTemplate
  };

  bool CheckTemplateParameterList(TemplateParameterList *NewParams,
                                  TemplateParameterList *OldParams,
                                  TemplateParamListContext TPC);
  TemplateParameterList *MatchTemplateParametersToScopeSpecifier(
      SourceLocation DeclStartLoc, SourceLocation DeclLoc,
      const CXXScopeSpec &SS, TemplateIdAnnotation *TemplateId,
      ArrayRef<TemplateParameterList *> ParamLists,
      bool IsFriend, bool &IsExplicitSpecialization, bool &Invalid);

  DeclResult CheckClassTemplate(Scope *S, unsigned TagSpec, TagUseKind TUK,
                                SourceLocation KWLoc, CXXScopeSpec &SS,
                                IdentifierInfo *Name, SourceLocation NameLoc,
                                AttributeList *Attr,
                                TemplateParameterList *TemplateParams,
                                AccessSpecifier AS,
                                SourceLocation ModulePrivateLoc,
                                SourceLocation FriendLoc,
                                unsigned NumOuterTemplateParamLists,
                            TemplateParameterList **OuterTemplateParamLists,
                                SkipBodyInfo *SkipBody = nullptr);

  void translateTemplateArguments(const ASTTemplateArgsPtr &In,
                                  TemplateArgumentListInfo &Out);

  void NoteAllFoundTemplates(TemplateName Name);

  QualType CheckTemplateIdType(TemplateName Template,
                               SourceLocation TemplateLoc,
                              TemplateArgumentListInfo &TemplateArgs);

  TypeResult
  ActOnTemplateIdType(CXXScopeSpec &SS, SourceLocation TemplateKWLoc,
                      TemplateTy Template, SourceLocation TemplateLoc,
                      SourceLocation LAngleLoc,
                      ASTTemplateArgsPtr TemplateArgs,
                      SourceLocation RAngleLoc,
                      bool IsCtorOrDtorName = false);

  /// \brief Parsed an elaborated-type-specifier that refers to a template-id,
  /// such as \c class T::template apply<U>.
  TypeResult ActOnTagTemplateIdType(TagUseKind TUK,
                                    TypeSpecifierType TagSpec,
                                    SourceLocation TagLoc,
                                    CXXScopeSpec &SS,
                                    SourceLocation TemplateKWLoc,
                                    TemplateTy TemplateD,
                                    SourceLocation TemplateLoc,
                                    SourceLocation LAngleLoc,
                                    ASTTemplateArgsPtr TemplateArgsIn,
                                    SourceLocation RAngleLoc);

  DeclResult ActOnVarTemplateSpecialization(
      Scope *S, Declarator &D, TypeSourceInfo *DI,
      SourceLocation TemplateKWLoc, TemplateParameterList *TemplateParams,
      StorageClass SC, bool IsPartialSpecialization);

  DeclResult CheckVarTemplateId(VarTemplateDecl *Template,
                                SourceLocation TemplateLoc,
                                SourceLocation TemplateNameLoc,
                                const TemplateArgumentListInfo &TemplateArgs);

  ExprResult CheckVarTemplateId(const CXXScopeSpec &SS,
                                const DeclarationNameInfo &NameInfo,
                                VarTemplateDecl *Template,
                                SourceLocation TemplateLoc,
                                const TemplateArgumentListInfo *TemplateArgs);

  ExprResult BuildTemplateIdExpr(const CXXScopeSpec &SS,
                                 SourceLocation TemplateKWLoc,
                                 LookupResult &R,
                                 bool RequiresADL,
                               const TemplateArgumentListInfo *TemplateArgs);

  ExprResult BuildQualifiedTemplateIdExpr(CXXScopeSpec &SS,
                                          SourceLocation TemplateKWLoc,
                               const DeclarationNameInfo &NameInfo,
                               const TemplateArgumentListInfo *TemplateArgs);

  TemplateNameKind ActOnDependentTemplateName(Scope *S,
                                              CXXScopeSpec &SS,
                                              SourceLocation TemplateKWLoc,
                                              UnqualifiedId &Name,
                                              ParsedType ObjectType,
                                              bool EnteringContext,
                                              TemplateTy &Template);

  DeclResult
  ActOnClassTemplateSpecialization(Scope *S, unsigned TagSpec, TagUseKind TUK,
                                   SourceLocation KWLoc,
                                   SourceLocation ModulePrivateLoc,
                                   TemplateIdAnnotation &TemplateId,
                                   AttributeList *Attr,
                                 MultiTemplateParamsArg TemplateParameterLists,
                                   SkipBodyInfo *SkipBody = nullptr);

  Decl *ActOnTemplateDeclarator(Scope *S,
                                MultiTemplateParamsArg TemplateParameterLists,
                                Declarator &D);

  Decl *ActOnStartOfFunctionTemplateDef(Scope *FnBodyScope,
                                  MultiTemplateParamsArg TemplateParameterLists,
                                        Declarator &D);

  bool
  CheckSpecializationInstantiationRedecl(SourceLocation NewLoc,
                                         TemplateSpecializationKind NewTSK,
                                         NamedDecl *PrevDecl,
                                         TemplateSpecializationKind PrevTSK,
                                         SourceLocation PrevPtOfInstantiation,
                                         bool &SuppressNew);

  bool CheckDependentFunctionTemplateSpecialization(FunctionDecl *FD,
                    const TemplateArgumentListInfo &ExplicitTemplateArgs,
                                                    LookupResult &Previous);

  bool CheckFunctionTemplateSpecialization(FunctionDecl *FD,
                         TemplateArgumentListInfo *ExplicitTemplateArgs,
                                           LookupResult &Previous);
  bool CheckMemberSpecialization(NamedDecl *Member, LookupResult &Previous);

  DeclResult
  ActOnExplicitInstantiation(Scope *S,
                             SourceLocation ExternLoc,
                             SourceLocation TemplateLoc,
                             unsigned TagSpec,
                             SourceLocation KWLoc,
                             const CXXScopeSpec &SS,
                             TemplateTy Template,
                             SourceLocation TemplateNameLoc,
                             SourceLocation LAngleLoc,
                             ASTTemplateArgsPtr TemplateArgs,
                             SourceLocation RAngleLoc,
                             AttributeList *Attr);

  DeclResult
  ActOnExplicitInstantiation(Scope *S,
                             SourceLocation ExternLoc,
                             SourceLocation TemplateLoc,
                             unsigned TagSpec,
                             SourceLocation KWLoc,
                             CXXScopeSpec &SS,
                             IdentifierInfo *Name,
                             SourceLocation NameLoc,
                             AttributeList *Attr);

  DeclResult ActOnExplicitInstantiation(Scope *S,
                                        SourceLocation ExternLoc,
                                        SourceLocation TemplateLoc,
                                        Declarator &D);

  TemplateArgumentLoc
  SubstDefaultTemplateArgumentIfAvailable(TemplateDecl *Template,
                                          SourceLocation TemplateLoc,
                                          SourceLocation RAngleLoc,
                                          Decl *Param,
                                          SmallVectorImpl<TemplateArgument>
                                            &Converted,
                                          bool &HasDefaultArg);

  /// \brief Specifies the context in which a particular template
  /// argument is being checked.
  enum CheckTemplateArgumentKind {
    /// \brief The template argument was specified in the code or was
    /// instantiated with some deduced template arguments.
    CTAK_Specified,

    /// \brief The template argument was deduced via template argument
    /// deduction.
    CTAK_Deduced,

    /// \brief The template argument was deduced from an array bound
    /// via template argument deduction.
    CTAK_DeducedFromArrayBound
  };

  bool CheckTemplateArgument(NamedDecl *Param,
                             TemplateArgumentLoc &Arg,
                             NamedDecl *Template,
                             SourceLocation TemplateLoc,
                             SourceLocation RAngleLoc,
                             unsigned ArgumentPackIndex,
                           SmallVectorImpl<TemplateArgument> &Converted,
                             CheckTemplateArgumentKind CTAK = CTAK_Specified);

  /// \brief Check that the given template arguments can be be provided to
  /// the given template, converting the arguments along the way.
  ///
  /// \param Template The template to which the template arguments are being
  /// provided.
  ///
  /// \param TemplateLoc The location of the template name in the source.
  ///
  /// \param TemplateArgs The list of template arguments. If the template is
  /// a template template parameter, this function may extend the set of
  /// template arguments to also include substituted, defaulted template
  /// arguments.
  ///
  /// \param PartialTemplateArgs True if the list of template arguments is
  /// intentionally partial, e.g., because we're checking just the initial
  /// set of template arguments.
  ///
  /// \param Converted Will receive the converted, canonicalized template
  /// arguments.
  ///
  /// \returns true if an error occurred, false otherwise.
  bool CheckTemplateArgumentList(TemplateDecl *Template,
                                 SourceLocation TemplateLoc,
                                 TemplateArgumentListInfo &TemplateArgs,
                                 bool PartialTemplateArgs,
                           SmallVectorImpl<TemplateArgument> &Converted);

  bool CheckTemplateTypeArgument(TemplateTypeParmDecl *Param,
                                 TemplateArgumentLoc &Arg,
                           SmallVectorImpl<TemplateArgument> &Converted);

  bool CheckTemplateArgument(TemplateTypeParmDecl *Param,
                             TypeSourceInfo *Arg);
  ExprResult CheckTemplateArgument(NonTypeTemplateParmDecl *Param,
                                   QualType InstantiatedParamType, Expr *Arg,
                                   TemplateArgument &Converted,
                               CheckTemplateArgumentKind CTAK = CTAK_Specified);
  bool CheckTemplateArgument(TemplateTemplateParmDecl *Param,
                             TemplateArgumentLoc &Arg,
                             unsigned ArgumentPackIndex);

  ExprResult
  BuildExpressionFromDeclTemplateArgument(const TemplateArgument &Arg,
                                          QualType ParamType,
                                          SourceLocation Loc);
  ExprResult
  BuildExpressionFromIntegralTemplateArgument(const TemplateArgument &Arg,
                                              SourceLocation Loc);

  /// \brief Enumeration describing how template parameter lists are compared
  /// for equality.
  enum TemplateParameterListEqualKind {
    /// \brief We are matching the template parameter lists of two templates
    /// that might be redeclarations.
    ///
    /// \code
    /// template<typename T> struct X;
    /// template<typename T> struct X;
    /// \endcode
    TPL_TemplateMatch,

    /// \brief We are matching the template parameter lists of two template
    /// template parameters as part of matching the template parameter lists
    /// of two templates that might be redeclarations.
    ///
    /// \code
    /// template<template<int I> class TT> struct X;
    /// template<template<int Value> class Other> struct X;
    /// \endcode
    TPL_TemplateTemplateParmMatch,

    /// \brief We are matching the template parameter lists of a template
    /// template argument against the template parameter lists of a template
    /// template parameter.
    ///
    /// \code
    /// template<template<int Value> class Metafun> struct X;
    /// template<int Value> struct integer_c;
    /// X<integer_c> xic;
    /// \endcode
    TPL_TemplateTemplateArgumentMatch
  };

  bool TemplateParameterListsAreEqual(TemplateParameterList *New,
                                      TemplateParameterList *Old,
                                      bool Complain,
                                      TemplateParameterListEqualKind Kind,
                                      SourceLocation TemplateArgLoc
                                        = SourceLocation());

  bool CheckTemplateDeclScope(Scope *S, TemplateParameterList *TemplateParams);

  /// \brief Called when the parser has parsed a C++ typename
  /// specifier, e.g., "typename T::type".
  ///
  /// \param S The scope in which this typename type occurs.
  /// \param TypenameLoc the location of the 'typename' keyword
  /// \param SS the nested-name-specifier following the typename (e.g., 'T::').
  /// \param II the identifier we're retrieving (e.g., 'type' in the example).
  /// \param IdLoc the location of the identifier.
  TypeResult
  ActOnTypenameType(Scope *S, SourceLocation TypenameLoc,
                    const CXXScopeSpec &SS, const IdentifierInfo &II,
                    SourceLocation IdLoc);

  /// \brief Called when the parser has parsed a C++ typename
  /// specifier that ends in a template-id, e.g.,
  /// "typename MetaFun::template apply<T1, T2>".
  ///
  /// \param S The scope in which this typename type occurs.
  /// \param TypenameLoc the location of the 'typename' keyword
  /// \param SS the nested-name-specifier following the typename (e.g., 'T::').
  /// \param TemplateLoc the location of the 'template' keyword, if any.
  /// \param TemplateName The template name.
  /// \param TemplateNameLoc The location of the template name.
  /// \param LAngleLoc The location of the opening angle bracket  ('<').
  /// \param TemplateArgs The template arguments.
  /// \param RAngleLoc The location of the closing angle bracket  ('>').
  TypeResult
  ActOnTypenameType(Scope *S, SourceLocation TypenameLoc,
                    const CXXScopeSpec &SS,
                    SourceLocation TemplateLoc,
                    TemplateTy TemplateName,
                    SourceLocation TemplateNameLoc,
                    SourceLocation LAngleLoc,
                    ASTTemplateArgsPtr TemplateArgs,
                    SourceLocation RAngleLoc);

  QualType CheckTypenameType(ElaboratedTypeKeyword Keyword,
                             SourceLocation KeywordLoc,
                             NestedNameSpecifierLoc QualifierLoc,
                             const IdentifierInfo &II,
                             SourceLocation IILoc);

  TypeSourceInfo *RebuildTypeInCurrentInstantiation(TypeSourceInfo *T,
                                                    SourceLocation Loc,
                                                    DeclarationName Name);
  bool RebuildNestedNameSpecifierInCurrentInstantiation(CXXScopeSpec &SS);

  ExprResult RebuildExprInCurrentInstantiation(Expr *E);
  bool RebuildTemplateParamsInCurrentInstantiation(
                                                TemplateParameterList *Params);

  std::string
  getTemplateArgumentBindingsText(const TemplateParameterList *Params,
                                  const TemplateArgumentList &Args);

  std::string
  getTemplateArgumentBindingsText(const TemplateParameterList *Params,
                                  const TemplateArgument *Args,
                                  unsigned NumArgs);

  //===--------------------------------------------------------------------===//
  // C++ Variadic Templates (C++0x [temp.variadic])
  //===--------------------------------------------------------------------===//

  /// Determine whether an unexpanded parameter pack might be permitted in this
  /// location. Useful for error recovery.
  bool isUnexpandedParameterPackPermitted();

  /// \brief The context in which an unexpanded parameter pack is
  /// being diagnosed.
  ///
  /// Note that the values of this enumeration line up with the first
  /// argument to the \c err_unexpanded_parameter_pack diagnostic.
  enum UnexpandedParameterPackContext {
    /// \brief An arbitrary expression.
    UPPC_Expression = 0,

    /// \brief The base type of a class type.
    UPPC_BaseType,

    /// \brief The type of an arbitrary declaration.
    UPPC_DeclarationType,

    /// \brief The type of a data member.
    UPPC_DataMemberType,

    /// \brief The size of a bit-field.
    UPPC_BitFieldWidth,

    /// \brief The expression in a static assertion.
    UPPC_StaticAssertExpression,

    /// \brief The fixed underlying type of an enumeration.
    UPPC_FixedUnderlyingType,

    /// \brief The enumerator value.
    UPPC_EnumeratorValue,

    /// \brief A using declaration.
    UPPC_UsingDeclaration,

    /// \brief A friend declaration.
    UPPC_FriendDeclaration,

    /// \brief A declaration qualifier.
    UPPC_DeclarationQualifier,

    /// \brief An initializer.
    UPPC_Initializer,

    /// \brief A default argument.
    UPPC_DefaultArgument,

    /// \brief The type of a non-type template parameter.
    UPPC_NonTypeTemplateParameterType,

    /// \brief The type of an exception.
    UPPC_ExceptionType,

    /// \brief Partial specialization.
    UPPC_PartialSpecialization,

    /// \brief Microsoft __if_exists.
    UPPC_IfExists,

    /// \brief Microsoft __if_not_exists.
    UPPC_IfNotExists,

    /// \brief Lambda expression.
    UPPC_Lambda,

    /// \brief Block expression,
    UPPC_Block
  };

  /// \brief Diagnose unexpanded parameter packs.
  ///
  /// \param Loc The location at which we should emit the diagnostic.
  ///
  /// \param UPPC The context in which we are diagnosing unexpanded
  /// parameter packs.
  ///
  /// \param Unexpanded the set of unexpanded parameter packs.
  ///
  /// \returns true if an error occurred, false otherwise.
  bool DiagnoseUnexpandedParameterPacks(SourceLocation Loc,
                                        UnexpandedParameterPackContext UPPC,
                                  ArrayRef<UnexpandedParameterPack> Unexpanded);

  /// \brief If the given type contains an unexpanded parameter pack,
  /// diagnose the error.
  ///
  /// \param Loc The source location where a diagnostc should be emitted.
  ///
  /// \param T The type that is being checked for unexpanded parameter
  /// packs.
  ///
  /// \returns true if an error occurred, false otherwise.
  bool DiagnoseUnexpandedParameterPack(SourceLocation Loc, TypeSourceInfo *T,
                                       UnexpandedParameterPackContext UPPC);

  /// \brief If the given expression contains an unexpanded parameter
  /// pack, diagnose the error.
  ///
  /// \param E The expression that is being checked for unexpanded
  /// parameter packs.
  ///
  /// \returns true if an error occurred, false otherwise.
  bool DiagnoseUnexpandedParameterPack(Expr *E,
                       UnexpandedParameterPackContext UPPC = UPPC_Expression);

  /// \brief If the given nested-name-specifier contains an unexpanded
  /// parameter pack, diagnose the error.
  ///
  /// \param SS The nested-name-specifier that is being checked for
  /// unexpanded parameter packs.
  ///
  /// \returns true if an error occurred, false otherwise.
  bool DiagnoseUnexpandedParameterPack(const CXXScopeSpec &SS,
                                       UnexpandedParameterPackContext UPPC);

  /// \brief If the given name contains an unexpanded parameter pack,
  /// diagnose the error.
  ///
  /// \param NameInfo The name (with source location information) that
  /// is being checked for unexpanded parameter packs.
  ///
  /// \returns true if an error occurred, false otherwise.
  bool DiagnoseUnexpandedParameterPack(const DeclarationNameInfo &NameInfo,
                                       UnexpandedParameterPackContext UPPC);

  /// \brief If the given template name contains an unexpanded parameter pack,
  /// diagnose the error.
  ///
  /// \param Loc The location of the template name.
  ///
  /// \param Template The template name that is being checked for unexpanded
  /// parameter packs.
  ///
  /// \returns true if an error occurred, false otherwise.
  bool DiagnoseUnexpandedParameterPack(SourceLocation Loc,
                                       TemplateName Template,
                                       UnexpandedParameterPackContext UPPC);

  /// \brief If the given template argument contains an unexpanded parameter
  /// pack, diagnose the error.
  ///
  /// \param Arg The template argument that is being checked for unexpanded
  /// parameter packs.
  ///
  /// \returns true if an error occurred, false otherwise.
  bool DiagnoseUnexpandedParameterPack(TemplateArgumentLoc Arg,
                                       UnexpandedParameterPackContext UPPC);

  /// \brief Collect the set of unexpanded parameter packs within the given
  /// template argument.
  ///
  /// \param Arg The template argument that will be traversed to find
  /// unexpanded parameter packs.
  void collectUnexpandedParameterPacks(TemplateArgument Arg,
                   SmallVectorImpl<UnexpandedParameterPack> &Unexpanded);

  /// \brief Collect the set of unexpanded parameter packs within the given
  /// template argument.
  ///
  /// \param Arg The template argument that will be traversed to find
  /// unexpanded parameter packs.
  void collectUnexpandedParameterPacks(TemplateArgumentLoc Arg,
                    SmallVectorImpl<UnexpandedParameterPack> &Unexpanded);

  /// \brief Collect the set of unexpanded parameter packs within the given
  /// type.
  ///
  /// \param T The type that will be traversed to find
  /// unexpanded parameter packs.
  void collectUnexpandedParameterPacks(QualType T,
                   SmallVectorImpl<UnexpandedParameterPack> &Unexpanded);

  /// \brief Collect the set of unexpanded parameter packs within the given
  /// type.
  ///
  /// \param TL The type that will be traversed to find
  /// unexpanded parameter packs.
  void collectUnexpandedParameterPacks(TypeLoc TL,
                   SmallVectorImpl<UnexpandedParameterPack> &Unexpanded);

  /// \brief Collect the set of unexpanded parameter packs within the given
  /// nested-name-specifier.
  ///
  /// \param SS The nested-name-specifier that will be traversed to find
  /// unexpanded parameter packs.
  void collectUnexpandedParameterPacks(CXXScopeSpec &SS,
                         SmallVectorImpl<UnexpandedParameterPack> &Unexpanded);

  /// \brief Collect the set of unexpanded parameter packs within the given
  /// name.
  ///
  /// \param NameInfo The name that will be traversed to find
  /// unexpanded parameter packs.
  void collectUnexpandedParameterPacks(const DeclarationNameInfo &NameInfo,
                         SmallVectorImpl<UnexpandedParameterPack> &Unexpanded);

  /// \brief Invoked when parsing a template argument followed by an
  /// ellipsis, which creates a pack expansion.
  ///
  /// \param Arg The template argument preceding the ellipsis, which
  /// may already be invalid.
  ///
  /// \param EllipsisLoc The location of the ellipsis.
  ParsedTemplateArgument ActOnPackExpansion(const ParsedTemplateArgument &Arg,
                                            SourceLocation EllipsisLoc);

  /// \brief Invoked when parsing a type followed by an ellipsis, which
  /// creates a pack expansion.
  ///
  /// \param Type The type preceding the ellipsis, which will become
  /// the pattern of the pack expansion.
  ///
  /// \param EllipsisLoc The location of the ellipsis.
  TypeResult ActOnPackExpansion(ParsedType Type, SourceLocation EllipsisLoc);

  /// \brief Construct a pack expansion type from the pattern of the pack
  /// expansion.
  TypeSourceInfo *CheckPackExpansion(TypeSourceInfo *Pattern,
                                     SourceLocation EllipsisLoc,
                                     Optional<unsigned> NumExpansions);

  /// \brief Construct a pack expansion type from the pattern of the pack
  /// expansion.
  QualType CheckPackExpansion(QualType Pattern,
                              SourceRange PatternRange,
                              SourceLocation EllipsisLoc,
                              Optional<unsigned> NumExpansions);

  /// \brief Invoked when parsing an expression followed by an ellipsis, which
  /// creates a pack expansion.
  ///
  /// \param Pattern The expression preceding the ellipsis, which will become
  /// the pattern of the pack expansion.
  ///
  /// \param EllipsisLoc The location of the ellipsis.
  ExprResult ActOnPackExpansion(Expr *Pattern, SourceLocation EllipsisLoc);

  /// \brief Invoked when parsing an expression followed by an ellipsis, which
  /// creates a pack expansion.
  ///
  /// \param Pattern The expression preceding the ellipsis, which will become
  /// the pattern of the pack expansion.
  ///
  /// \param EllipsisLoc The location of the ellipsis.
  ExprResult CheckPackExpansion(Expr *Pattern, SourceLocation EllipsisLoc,
                                Optional<unsigned> NumExpansions);

  /// \brief Determine whether we could expand a pack expansion with the
  /// given set of parameter packs into separate arguments by repeatedly
  /// transforming the pattern.
  ///
  /// \param EllipsisLoc The location of the ellipsis that identifies the
  /// pack expansion.
  ///
  /// \param PatternRange The source range that covers the entire pattern of
  /// the pack expansion.
  ///
  /// \param Unexpanded The set of unexpanded parameter packs within the
  /// pattern.
  ///
  /// \param ShouldExpand Will be set to \c true if the transformer should
  /// expand the corresponding pack expansions into separate arguments. When
  /// set, \c NumExpansions must also be set.
  ///
  /// \param RetainExpansion Whether the caller should add an unexpanded
  /// pack expansion after all of the expanded arguments. This is used
  /// when extending explicitly-specified template argument packs per
  /// C++0x [temp.arg.explicit]p9.
  ///
  /// \param NumExpansions The number of separate arguments that will be in
  /// the expanded form of the corresponding pack expansion. This is both an
  /// input and an output parameter, which can be set by the caller if the
  /// number of expansions is known a priori (e.g., due to a prior substitution)
  /// and will be set by the callee when the number of expansions is known.
  /// The callee must set this value when \c ShouldExpand is \c true; it may
  /// set this value in other cases.
  ///
  /// \returns true if an error occurred (e.g., because the parameter packs
  /// are to be instantiated with arguments of different lengths), false
  /// otherwise. If false, \c ShouldExpand (and possibly \c NumExpansions)
  /// must be set.
  bool CheckParameterPacksForExpansion(SourceLocation EllipsisLoc,
                                       SourceRange PatternRange,
                             ArrayRef<UnexpandedParameterPack> Unexpanded,
                             const MultiLevelTemplateArgumentList &TemplateArgs,
                                       bool &ShouldExpand,
                                       bool &RetainExpansion,
                                       Optional<unsigned> &NumExpansions);

  /// \brief Determine the number of arguments in the given pack expansion
  /// type.
  ///
  /// This routine assumes that the number of arguments in the expansion is
  /// consistent across all of the unexpanded parameter packs in its pattern.
  ///
  /// Returns an empty Optional if the type can't be expanded.
  Optional<unsigned> getNumArgumentsInExpansion(QualType T,
      const MultiLevelTemplateArgumentList &TemplateArgs);

  /// \brief Determine whether the given declarator contains any unexpanded
  /// parameter packs.
  ///
  /// This routine is used by the parser to disambiguate function declarators
  /// with an ellipsis prior to the ')', e.g.,
  ///
  /// \code
  ///   void f(T...);
  /// \endcode
  ///
  /// To determine whether we have an (unnamed) function parameter pack or
  /// a variadic function.
  ///
  /// \returns true if the declarator contains any unexpanded parameter packs,
  /// false otherwise.
  bool containsUnexpandedParameterPacks(Declarator &D);

  /// \brief Returns the pattern of the pack expansion for a template argument.
  ///
  /// \param OrigLoc The template argument to expand.
  ///
  /// \param Ellipsis Will be set to the location of the ellipsis.
  ///
  /// \param NumExpansions Will be set to the number of expansions that will
  /// be generated from this pack expansion, if known a priori.
  TemplateArgumentLoc getTemplateArgumentPackExpansionPattern(
      TemplateArgumentLoc OrigLoc,
      SourceLocation &Ellipsis,
      Optional<unsigned> &NumExpansions) const;

  //===--------------------------------------------------------------------===//
  // C++ Template Argument Deduction (C++ [temp.deduct])
  //===--------------------------------------------------------------------===//

  QualType adjustCCAndNoReturn(QualType ArgFunctionType, QualType FunctionType);

  /// \brief Describes the result of template argument deduction.
  ///
  /// The TemplateDeductionResult enumeration describes the result of
  /// template argument deduction, as returned from
  /// DeduceTemplateArguments(). The separate TemplateDeductionInfo
  /// structure provides additional information about the results of
  /// template argument deduction, e.g., the deduced template argument
  /// list (if successful) or the specific template parameters or
  /// deduced arguments that were involved in the failure.
  enum TemplateDeductionResult {
    /// \brief Template argument deduction was successful.
    TDK_Success = 0,
    /// \brief The declaration was invalid; do nothing.
    TDK_Invalid,
    /// \brief Template argument deduction exceeded the maximum template
    /// instantiation depth (which has already been diagnosed).
    TDK_InstantiationDepth,
    /// \brief Template argument deduction did not deduce a value
    /// for every template parameter.
    TDK_Incomplete,
    /// \brief Template argument deduction produced inconsistent
    /// deduced values for the given template parameter.
    TDK_Inconsistent,
    /// \brief Template argument deduction failed due to inconsistent
    /// cv-qualifiers on a template parameter type that would
    /// otherwise be deduced, e.g., we tried to deduce T in "const T"
    /// but were given a non-const "X".
    TDK_Underqualified,
    /// \brief Substitution of the deduced template argument values
    /// resulted in an error.
    TDK_SubstitutionFailure,
    /// \brief A non-depnedent component of the parameter did not match the
    /// corresponding component of the argument.
    TDK_NonDeducedMismatch,
    /// \brief When performing template argument deduction for a function
    /// template, there were too many call arguments.
    TDK_TooManyArguments,
    /// \brief When performing template argument deduction for a function
    /// template, there were too few call arguments.
    TDK_TooFewArguments,
    /// \brief The explicitly-specified template arguments were not valid
    /// template arguments for the given template.
    TDK_InvalidExplicitArguments,
    /// \brief The arguments included an overloaded function name that could
    /// not be resolved to a suitable function.
    TDK_FailedOverloadResolution,
    /// \brief Deduction failed; that's all we know.
    TDK_MiscellaneousDeductionFailure
  };

  TemplateDeductionResult
  DeduceTemplateArguments(ClassTemplatePartialSpecializationDecl *Partial,
                          const TemplateArgumentList &TemplateArgs,
                          sema::TemplateDeductionInfo &Info);

  TemplateDeductionResult
  DeduceTemplateArguments(VarTemplatePartialSpecializationDecl *Partial,
                          const TemplateArgumentList &TemplateArgs,
                          sema::TemplateDeductionInfo &Info);

  TemplateDeductionResult SubstituteExplicitTemplateArguments(
      FunctionTemplateDecl *FunctionTemplate,
      TemplateArgumentListInfo &ExplicitTemplateArgs,
      SmallVectorImpl<DeducedTemplateArgument> &Deduced,
      SmallVectorImpl<QualType> &ParamTypes, QualType *FunctionType,
      sema::TemplateDeductionInfo &Info);

  /// brief A function argument from which we performed template argument
  // deduction for a call.
  struct OriginalCallArg {
    OriginalCallArg(QualType OriginalParamType,
                    unsigned ArgIdx,
                    QualType OriginalArgType)
      : OriginalParamType(OriginalParamType), ArgIdx(ArgIdx),
        OriginalArgType(OriginalArgType) { }

    QualType OriginalParamType;
    unsigned ArgIdx;
    QualType OriginalArgType;
  };

  TemplateDeductionResult
  FinishTemplateArgumentDeduction(FunctionTemplateDecl *FunctionTemplate,
                      SmallVectorImpl<DeducedTemplateArgument> &Deduced,
                                  unsigned NumExplicitlySpecified,
                                  FunctionDecl *&Specialization,
                                  sema::TemplateDeductionInfo &Info,
           SmallVectorImpl<OriginalCallArg> const *OriginalCallArgs = nullptr,
                                  bool PartialOverloading = false);

  TemplateDeductionResult
  DeduceTemplateArguments(FunctionTemplateDecl *FunctionTemplate,
                          TemplateArgumentListInfo *ExplicitTemplateArgs,
                          ArrayRef<Expr *> Args,
                          FunctionDecl *&Specialization,
                          sema::TemplateDeductionInfo &Info,
                          bool PartialOverloading = false);

  TemplateDeductionResult
  DeduceTemplateArguments(FunctionTemplateDecl *FunctionTemplate,
                          TemplateArgumentListInfo *ExplicitTemplateArgs,
                          QualType ArgFunctionType,
                          FunctionDecl *&Specialization,
                          sema::TemplateDeductionInfo &Info,
                          bool InOverloadResolution = false);

  TemplateDeductionResult
  DeduceTemplateArguments(FunctionTemplateDecl *FunctionTemplate,
                          QualType ToType,
                          CXXConversionDecl *&Specialization,
                          sema::TemplateDeductionInfo &Info);

  TemplateDeductionResult
  DeduceTemplateArguments(FunctionTemplateDecl *FunctionTemplate,
                          TemplateArgumentListInfo *ExplicitTemplateArgs,
                          FunctionDecl *&Specialization,
                          sema::TemplateDeductionInfo &Info,
                          bool InOverloadResolution = false);

  /// \brief Substitute Replacement for \p auto in \p TypeWithAuto
  QualType SubstAutoType(QualType TypeWithAuto, QualType Replacement);
  /// \brief Substitute Replacement for auto in TypeWithAuto
  TypeSourceInfo* SubstAutoTypeSourceInfo(TypeSourceInfo *TypeWithAuto, 
                                          QualType Replacement);

  /// \brief Result type of DeduceAutoType.
  enum DeduceAutoResult {
    DAR_Succeeded,
    DAR_Failed,
    DAR_FailedAlreadyDiagnosed
  };

  DeduceAutoResult DeduceAutoType(TypeSourceInfo *AutoType, Expr *&Initializer,
                                  QualType &Result);
  DeduceAutoResult DeduceAutoType(TypeLoc AutoTypeLoc, Expr *&Initializer,
                                  QualType &Result);
  void DiagnoseAutoDeductionFailure(VarDecl *VDecl, Expr *Init);
  bool DeduceReturnType(FunctionDecl *FD, SourceLocation Loc,
                        bool Diagnose = true);

  TypeLoc getReturnTypeLoc(FunctionDecl *FD) const;

  bool DeduceFunctionTypeFromReturnExpr(FunctionDecl *FD,
                                        SourceLocation ReturnLoc,
                                        Expr *&RetExpr, AutoType *AT);

  FunctionTemplateDecl *getMoreSpecializedTemplate(FunctionTemplateDecl *FT1,
                                                   FunctionTemplateDecl *FT2,
                                                   SourceLocation Loc,
                                           TemplatePartialOrderingContext TPOC,
                                                   unsigned NumCallArguments1,
                                                   unsigned NumCallArguments2);
  UnresolvedSetIterator
  getMostSpecialized(UnresolvedSetIterator SBegin, UnresolvedSetIterator SEnd,
                     TemplateSpecCandidateSet &FailedCandidates,
                     SourceLocation Loc,
                     const PartialDiagnostic &NoneDiag,
                     const PartialDiagnostic &AmbigDiag,
                     const PartialDiagnostic &CandidateDiag,
                     bool Complain = true, QualType TargetType = QualType());

  ClassTemplatePartialSpecializationDecl *
  getMoreSpecializedPartialSpecialization(
                                  ClassTemplatePartialSpecializationDecl *PS1,
                                  ClassTemplatePartialSpecializationDecl *PS2,
                                  SourceLocation Loc);

  VarTemplatePartialSpecializationDecl *getMoreSpecializedPartialSpecialization(
      VarTemplatePartialSpecializationDecl *PS1,
      VarTemplatePartialSpecializationDecl *PS2, SourceLocation Loc);

  void MarkUsedTemplateParameters(const TemplateArgumentList &TemplateArgs,
                                  bool OnlyDeduced,
                                  unsigned Depth,
                                  llvm::SmallBitVector &Used);
  void MarkDeducedTemplateParameters(
                                  const FunctionTemplateDecl *FunctionTemplate,
                                  llvm::SmallBitVector &Deduced) {
    return MarkDeducedTemplateParameters(Context, FunctionTemplate, Deduced);
  }
  static void MarkDeducedTemplateParameters(ASTContext &Ctx,
                                  const FunctionTemplateDecl *FunctionTemplate,
                                  llvm::SmallBitVector &Deduced);

  //===--------------------------------------------------------------------===//
  // C++ Template Instantiation
  //

  MultiLevelTemplateArgumentList
  getTemplateInstantiationArgs(NamedDecl *D,
                               const TemplateArgumentList *Innermost = nullptr,
                               bool RelativeToPrimary = false,
                               const FunctionDecl *Pattern = nullptr);

  /// \brief A template instantiation that is currently in progress.
  struct ActiveTemplateInstantiation {
    /// \brief The kind of template instantiation we are performing
    enum InstantiationKind {
      /// We are instantiating a template declaration. The entity is
      /// the declaration we're instantiating (e.g., a CXXRecordDecl).
      TemplateInstantiation,

      /// We are instantiating a default argument for a template
      /// parameter. The Entity is the template, and
      /// TemplateArgs/NumTemplateArguments provides the template
      /// arguments as specified.
      /// FIXME: Use a TemplateArgumentList
      DefaultTemplateArgumentInstantiation,

      /// We are instantiating a default argument for a function.
      /// The Entity is the ParmVarDecl, and TemplateArgs/NumTemplateArgs
      /// provides the template arguments as specified.
      DefaultFunctionArgumentInstantiation,

      /// We are substituting explicit template arguments provided for
      /// a function template. The entity is a FunctionTemplateDecl.
      ExplicitTemplateArgumentSubstitution,

      /// We are substituting template argument determined as part of
      /// template argument deduction for either a class template
      /// partial specialization or a function template. The
      /// Entity is either a ClassTemplatePartialSpecializationDecl or
      /// a FunctionTemplateDecl.
      DeducedTemplateArgumentSubstitution,

      /// We are substituting prior template arguments into a new
      /// template parameter. The template parameter itself is either a
      /// NonTypeTemplateParmDecl or a TemplateTemplateParmDecl.
      PriorTemplateArgumentSubstitution,

      /// We are checking the validity of a default template argument that
      /// has been used when naming a template-id.
      DefaultTemplateArgumentChecking,

      /// We are instantiating the exception specification for a function
      /// template which was deferred until it was needed.
      ExceptionSpecInstantiation
    } Kind;

    /// \brief The point of instantiation within the source code.
    SourceLocation PointOfInstantiation;

    /// \brief The template (or partial specialization) in which we are
    /// performing the instantiation, for substitutions of prior template
    /// arguments.
    NamedDecl *Template;

    /// \brief The entity that is being instantiated.
    Decl *Entity;

    /// \brief The list of template arguments we are substituting, if they
    /// are not part of the entity.
    const TemplateArgument *TemplateArgs;

    /// \brief The number of template arguments in TemplateArgs.
    unsigned NumTemplateArgs;

    /// \brief The template deduction info object associated with the
    /// substitution or checking of explicit or deduced template arguments.
    sema::TemplateDeductionInfo *DeductionInfo;

    /// \brief The source range that covers the construct that cause
    /// the instantiation, e.g., the template-id that causes a class
    /// template instantiation.
    SourceRange InstantiationRange;

    ActiveTemplateInstantiation()
      : Kind(TemplateInstantiation), Template(nullptr), Entity(nullptr),
        TemplateArgs(nullptr), NumTemplateArgs(0), DeductionInfo(nullptr) {}

    /// \brief Determines whether this template is an actual instantiation
    /// that should be counted toward the maximum instantiation depth.
    bool isInstantiationRecord() const;

    friend bool operator==(const ActiveTemplateInstantiation &X,
                           const ActiveTemplateInstantiation &Y) {
      if (X.Kind != Y.Kind)
        return false;

      if (X.Entity != Y.Entity)
        return false;

      switch (X.Kind) {
      case TemplateInstantiation:
      case ExceptionSpecInstantiation:
        return true;

      case PriorTemplateArgumentSubstitution:
      case DefaultTemplateArgumentChecking:
        return X.Template == Y.Template && X.TemplateArgs == Y.TemplateArgs;

      case DefaultTemplateArgumentInstantiation:
      case ExplicitTemplateArgumentSubstitution:
      case DeducedTemplateArgumentSubstitution:
      case DefaultFunctionArgumentInstantiation:
        return X.TemplateArgs == Y.TemplateArgs;

      }

      llvm_unreachable("Invalid InstantiationKind!");
    }

    friend bool operator!=(const ActiveTemplateInstantiation &X,
                           const ActiveTemplateInstantiation &Y) {
      return !(X == Y);
    }
  };

  /// \brief List of active template instantiations.
  ///
  /// This vector is treated as a stack. As one template instantiation
  /// requires another template instantiation, additional
  /// instantiations are pushed onto the stack up to a
  /// user-configurable limit LangOptions::InstantiationDepth.
  SmallVector<ActiveTemplateInstantiation, 16>
    ActiveTemplateInstantiations;

  /// \brief Extra modules inspected when performing a lookup during a template
  /// instantiation. Computed lazily.
  SmallVector<Module*, 16> ActiveTemplateInstantiationLookupModules;

  /// \brief Cache of additional modules that should be used for name lookup
  /// within the current template instantiation. Computed lazily; use
  /// getLookupModules() to get a complete set.
  llvm::DenseSet<Module*> LookupModulesCache;

  /// \brief Get the set of additional modules that should be checked during
  /// name lookup. A module and its imports become visible when instanting a
  /// template defined within it.
  llvm::DenseSet<Module*> &getLookupModules();

  /// \brief Whether we are in a SFINAE context that is not associated with
  /// template instantiation.
  ///
  /// This is used when setting up a SFINAE trap (\c see SFINAETrap) outside
  /// of a template instantiation or template argument deduction.
  bool InNonInstantiationSFINAEContext;

  /// \brief The number of ActiveTemplateInstantiation entries in
  /// \c ActiveTemplateInstantiations that are not actual instantiations and,
  /// therefore, should not be counted as part of the instantiation depth.
  unsigned NonInstantiationEntries;

  /// \brief The last template from which a template instantiation
  /// error or warning was produced.
  ///
  /// This value is used to suppress printing of redundant template
  /// instantiation backtraces when there are multiple errors in the
  /// same instantiation. FIXME: Does this belong in Sema? It's tough
  /// to implement it anywhere else.
  ActiveTemplateInstantiation LastTemplateInstantiationErrorContext;

  /// \brief The current index into pack expansion arguments that will be
  /// used for substitution of parameter packs.
  ///
  /// The pack expansion index will be -1 to indicate that parameter packs
  /// should be instantiated as themselves. Otherwise, the index specifies
  /// which argument within the parameter pack will be used for substitution.
  int ArgumentPackSubstitutionIndex;

  /// \brief RAII object used to change the argument pack substitution index
  /// within a \c Sema object.
  ///
  /// See \c ArgumentPackSubstitutionIndex for more information.
  class ArgumentPackSubstitutionIndexRAII {
    Sema &Self;
    int OldSubstitutionIndex;

  public:
    ArgumentPackSubstitutionIndexRAII(Sema &Self, int NewSubstitutionIndex)
      : Self(Self), OldSubstitutionIndex(Self.ArgumentPackSubstitutionIndex) {
      Self.ArgumentPackSubstitutionIndex = NewSubstitutionIndex;
    }

    ~ArgumentPackSubstitutionIndexRAII() {
      Self.ArgumentPackSubstitutionIndex = OldSubstitutionIndex;
    }
  };

  friend class ArgumentPackSubstitutionRAII;

  /// \brief The stack of calls expression undergoing template instantiation.
  ///
  /// The top of this stack is used by a fixit instantiating unresolved
  /// function calls to fix the AST to match the textual change it prints.
  SmallVector<CallExpr *, 8> CallsUndergoingInstantiation;

  /// \brief For each declaration that involved template argument deduction, the
  /// set of diagnostics that were suppressed during that template argument
  /// deduction.
  ///
  /// FIXME: Serialize this structure to the AST file.
  typedef llvm::DenseMap<Decl *, SmallVector<PartialDiagnosticAt, 1> >
    SuppressedDiagnosticsMap;
  SuppressedDiagnosticsMap SuppressedDiagnostics;

  /// \brief A stack object to be created when performing template
  /// instantiation.
  ///
  /// Construction of an object of type \c InstantiatingTemplate
  /// pushes the current instantiation onto the stack of active
  /// instantiations. If the size of this stack exceeds the maximum
  /// number of recursive template instantiations, construction
  /// produces an error and evaluates true.
  ///
  /// Destruction of this object will pop the named instantiation off
  /// the stack.
  struct InstantiatingTemplate {
    /// \brief Note that we are instantiating a class template,
    /// function template, or a member thereof.
    InstantiatingTemplate(Sema &SemaRef, SourceLocation PointOfInstantiation,
                          Decl *Entity,
                          SourceRange InstantiationRange = SourceRange());

    struct ExceptionSpecification {};
    /// \brief Note that we are instantiating an exception specification
    /// of a function template.
    InstantiatingTemplate(Sema &SemaRef, SourceLocation PointOfInstantiation,
                          FunctionDecl *Entity, ExceptionSpecification,
                          SourceRange InstantiationRange = SourceRange());

    /// \brief Note that we are instantiating a default argument in a
    /// template-id.
    InstantiatingTemplate(Sema &SemaRef, SourceLocation PointOfInstantiation,
                          TemplateDecl *Template,
                          ArrayRef<TemplateArgument> TemplateArgs,
                          SourceRange InstantiationRange = SourceRange());

    /// \brief Note that we are instantiating a default argument in a
    /// template-id.
    InstantiatingTemplate(Sema &SemaRef, SourceLocation PointOfInstantiation,
                          FunctionTemplateDecl *FunctionTemplate,
                          ArrayRef<TemplateArgument> TemplateArgs,
                          ActiveTemplateInstantiation::InstantiationKind Kind,
                          sema::TemplateDeductionInfo &DeductionInfo,
                          SourceRange InstantiationRange = SourceRange());

    /// \brief Note that we are instantiating as part of template
    /// argument deduction for a class template partial
    /// specialization.
    InstantiatingTemplate(Sema &SemaRef, SourceLocation PointOfInstantiation,
                          ClassTemplatePartialSpecializationDecl *PartialSpec,
                          ArrayRef<TemplateArgument> TemplateArgs,
                          sema::TemplateDeductionInfo &DeductionInfo,
                          SourceRange InstantiationRange = SourceRange());

    /// \brief Note that we are instantiating as part of template
    /// argument deduction for a variable template partial
    /// specialization.
    InstantiatingTemplate(Sema &SemaRef, SourceLocation PointOfInstantiation,
                          VarTemplatePartialSpecializationDecl *PartialSpec,
                          ArrayRef<TemplateArgument> TemplateArgs,
                          sema::TemplateDeductionInfo &DeductionInfo,
                          SourceRange InstantiationRange = SourceRange());

    InstantiatingTemplate(Sema &SemaRef, SourceLocation PointOfInstantiation,
                          ParmVarDecl *Param,
                          ArrayRef<TemplateArgument> TemplateArgs,
                          SourceRange InstantiationRange = SourceRange());

    /// \brief Note that we are substituting prior template arguments into a
    /// non-type parameter.
    InstantiatingTemplate(Sema &SemaRef, SourceLocation PointOfInstantiation,
                          NamedDecl *Template,
                          NonTypeTemplateParmDecl *Param,
                          ArrayRef<TemplateArgument> TemplateArgs,
                          SourceRange InstantiationRange);

    /// \brief Note that we are substituting prior template arguments into a
    /// template template parameter.
    InstantiatingTemplate(Sema &SemaRef, SourceLocation PointOfInstantiation,
                          NamedDecl *Template,
                          TemplateTemplateParmDecl *Param,
                          ArrayRef<TemplateArgument> TemplateArgs,
                          SourceRange InstantiationRange);

    /// \brief Note that we are checking the default template argument
    /// against the template parameter for a given template-id.
    InstantiatingTemplate(Sema &SemaRef, SourceLocation PointOfInstantiation,
                          TemplateDecl *Template,
                          NamedDecl *Param,
                          ArrayRef<TemplateArgument> TemplateArgs,
                          SourceRange InstantiationRange);


    /// \brief Note that we have finished instantiating this template.
    void Clear();

    ~InstantiatingTemplate() { Clear(); }

    /// \brief Determines whether we have exceeded the maximum
    /// recursive template instantiations.
    bool isInvalid() const { return Invalid; }

  private:
    Sema &SemaRef;
    bool Invalid;
    bool SavedInNonInstantiationSFINAEContext;
    bool CheckInstantiationDepth(SourceLocation PointOfInstantiation,
                                 SourceRange InstantiationRange);

    InstantiatingTemplate(
        Sema &SemaRef, ActiveTemplateInstantiation::InstantiationKind Kind,
        SourceLocation PointOfInstantiation, SourceRange InstantiationRange,
        Decl *Entity, NamedDecl *Template = nullptr,
        ArrayRef<TemplateArgument> TemplateArgs = ArrayRef<TemplateArgument>(),
        sema::TemplateDeductionInfo *DeductionInfo = nullptr);

    InstantiatingTemplate(const InstantiatingTemplate&) = delete;

    InstantiatingTemplate&
    operator=(const InstantiatingTemplate&) = delete;
  };

  void PrintInstantiationStack();

  /// \brief Determines whether we are currently in a context where
  /// template argument substitution failures are not considered
  /// errors.
  ///
  /// \returns An empty \c Optional if we're not in a SFINAE context.
  /// Otherwise, contains a pointer that, if non-NULL, contains the nearest
  /// template-deduction context object, which can be used to capture
  /// diagnostics that will be suppressed.
  Optional<sema::TemplateDeductionInfo *> isSFINAEContext() const;

  /// \brief Determines whether we are currently in a context that
  /// is not evaluated as per C++ [expr] p5.
  bool isUnevaluatedContext() const {
    assert(!ExprEvalContexts.empty() &&
           "Must be in an expression evaluation context");
    return ExprEvalContexts.back().isUnevaluated();
  }

  /// \brief RAII class used to determine whether SFINAE has
  /// trapped any errors that occur during template argument
  /// deduction.
  class SFINAETrap {
    Sema &SemaRef;
    unsigned PrevSFINAEErrors;
    bool PrevInNonInstantiationSFINAEContext;
    bool PrevAccessCheckingSFINAE;

  public:
    explicit SFINAETrap(Sema &SemaRef, bool AccessCheckingSFINAE = false)
      : SemaRef(SemaRef), PrevSFINAEErrors(SemaRef.NumSFINAEErrors),
        PrevInNonInstantiationSFINAEContext(
                                      SemaRef.InNonInstantiationSFINAEContext),
        PrevAccessCheckingSFINAE(SemaRef.AccessCheckingSFINAE)
    {
      if (!SemaRef.isSFINAEContext())
        SemaRef.InNonInstantiationSFINAEContext = true;
      SemaRef.AccessCheckingSFINAE = AccessCheckingSFINAE;
    }

    ~SFINAETrap() {
      SemaRef.NumSFINAEErrors = PrevSFINAEErrors;
      SemaRef.InNonInstantiationSFINAEContext
        = PrevInNonInstantiationSFINAEContext;
      SemaRef.AccessCheckingSFINAE = PrevAccessCheckingSFINAE;
    }

    /// \brief Determine whether any SFINAE errors have been trapped.
    bool hasErrorOccurred() const {
      return SemaRef.NumSFINAEErrors > PrevSFINAEErrors;
    }
  };

  /// \brief RAII class used to indicate that we are performing provisional
  /// semantic analysis to determine the validity of a construct, so
  /// typo-correction and diagnostics in the immediate context (not within
  /// implicitly-instantiated templates) should be suppressed.
  class TentativeAnalysisScope {
    Sema &SemaRef;
    // FIXME: Using a SFINAETrap for this is a hack.
    SFINAETrap Trap;
    bool PrevDisableTypoCorrection;
  public:
    explicit TentativeAnalysisScope(Sema &SemaRef)
        : SemaRef(SemaRef), Trap(SemaRef, true),
          PrevDisableTypoCorrection(SemaRef.DisableTypoCorrection) {
      SemaRef.DisableTypoCorrection = true;
    }
    ~TentativeAnalysisScope() {
      SemaRef.DisableTypoCorrection = PrevDisableTypoCorrection;
    }
  };

  /// \brief The current instantiation scope used to store local
  /// variables.
  LocalInstantiationScope *CurrentInstantiationScope;

  /// \brief Tracks whether we are in a context where typo correction is
  /// disabled.
  bool DisableTypoCorrection;

  /// \brief The number of typos corrected by CorrectTypo.
  unsigned TyposCorrected;

  typedef llvm::SmallSet<SourceLocation, 2> SrcLocSet;
  typedef llvm::DenseMap<IdentifierInfo *, SrcLocSet> IdentifierSourceLocations;

  /// \brief A cache containing identifiers for which typo correction failed and
  /// their locations, so that repeated attempts to correct an identifier in a
  /// given location are ignored if typo correction already failed for it.
  IdentifierSourceLocations TypoCorrectionFailures;

  /// \brief Worker object for performing CFG-based warnings.
  sema::AnalysisBasedWarnings AnalysisWarnings;
  threadSafety::BeforeSet *ThreadSafetyDeclCache;

  /// \brief An entity for which implicit template instantiation is required.
  ///
  /// The source location associated with the declaration is the first place in
  /// the source code where the declaration was "used". It is not necessarily
  /// the point of instantiation (which will be either before or after the
  /// namespace-scope declaration that triggered this implicit instantiation),
  /// However, it is the location that diagnostics should generally refer to,
  /// because users will need to know what code triggered the instantiation.
  typedef std::pair<ValueDecl *, SourceLocation> PendingImplicitInstantiation;

  /// \brief The queue of implicit template instantiations that are required
  /// but have not yet been performed.
  std::deque<PendingImplicitInstantiation> PendingInstantiations;

  class SavePendingInstantiationsAndVTableUsesRAII {
  public:
    SavePendingInstantiationsAndVTableUsesRAII(Sema &S, bool Enabled)
        : S(S), Enabled(Enabled) {
      if (!Enabled) return;

      SavedPendingInstantiations.swap(S.PendingInstantiations);
      SavedVTableUses.swap(S.VTableUses);
    }

    ~SavePendingInstantiationsAndVTableUsesRAII() {
      if (!Enabled) return;

      // Restore the set of pending vtables.
      assert(S.VTableUses.empty() &&
             "VTableUses should be empty before it is discarded.");
      S.VTableUses.swap(SavedVTableUses);

      // Restore the set of pending implicit instantiations.
      assert(S.PendingInstantiations.empty() &&
             "PendingInstantiations should be empty before it is discarded.");
      S.PendingInstantiations.swap(SavedPendingInstantiations);
    }

  private:
    Sema &S;
    SmallVector<VTableUse, 16> SavedVTableUses;
    std::deque<PendingImplicitInstantiation> SavedPendingInstantiations;
    bool Enabled;
  };

  /// \brief The queue of implicit template instantiations that are required
  /// and must be performed within the current local scope.
  ///
  /// This queue is only used for member functions of local classes in
  /// templates, which must be instantiated in the same scope as their
  /// enclosing function, so that they can reference function-local
  /// types, static variables, enumerators, etc.
  std::deque<PendingImplicitInstantiation> PendingLocalImplicitInstantiations;

  class SavePendingLocalImplicitInstantiationsRAII {
  public:
    SavePendingLocalImplicitInstantiationsRAII(Sema &S): S(S) {
      SavedPendingLocalImplicitInstantiations.swap(
          S.PendingLocalImplicitInstantiations);
    }

    ~SavePendingLocalImplicitInstantiationsRAII() {
      assert(S.PendingLocalImplicitInstantiations.empty() &&
             "there shouldn't be any pending local implicit instantiations");
      SavedPendingLocalImplicitInstantiations.swap(
          S.PendingLocalImplicitInstantiations);
    }

  private:
    Sema &S;
    std::deque<PendingImplicitInstantiation>
    SavedPendingLocalImplicitInstantiations;
  };

  void PerformPendingInstantiations(bool LocalOnly = false);

  TypeSourceInfo *SubstType(TypeSourceInfo *T,
                            const MultiLevelTemplateArgumentList &TemplateArgs,
                            SourceLocation Loc, DeclarationName Entity);

  QualType SubstType(QualType T,
                     const MultiLevelTemplateArgumentList &TemplateArgs,
                     SourceLocation Loc, DeclarationName Entity);

  TypeSourceInfo *SubstType(TypeLoc TL,
                            const MultiLevelTemplateArgumentList &TemplateArgs,
                            SourceLocation Loc, DeclarationName Entity);

  TypeSourceInfo *SubstFunctionDeclType(TypeSourceInfo *T,
                            const MultiLevelTemplateArgumentList &TemplateArgs,
                                        SourceLocation Loc,
                                        DeclarationName Entity,
                                        CXXRecordDecl *ThisContext,
                                        unsigned ThisTypeQuals);
  void SubstExceptionSpec(FunctionDecl *New, const FunctionProtoType *Proto,
                          const MultiLevelTemplateArgumentList &Args);
  ParmVarDecl *SubstParmVarDecl(ParmVarDecl *D,
                            const MultiLevelTemplateArgumentList &TemplateArgs,
                                int indexAdjustment,
                                Optional<unsigned> NumExpansions,
                                bool ExpectParameterPack);
  bool SubstParmTypes(SourceLocation Loc,
                      ParmVarDecl **Params, unsigned NumParams,
                      const MultiLevelTemplateArgumentList &TemplateArgs,
                      SmallVectorImpl<QualType> &ParamTypes,
                      SmallVectorImpl<ParmVarDecl *> *OutParams = nullptr);
  /// HLSL Change Begin - back ported from llvm-project/4409a83c2935.
  bool SubstDefaultArgument(SourceLocation Loc, ParmVarDecl *Param,
                            const MultiLevelTemplateArgumentList &TemplateArgs,
                            bool ForCallExpr = false);
  /// HLSL Change End - back ported from llvm-project/4409a83c2935.

  ExprResult SubstExpr(Expr *E,
                       const MultiLevelTemplateArgumentList &TemplateArgs);

  /// \brief Substitute the given template arguments into a list of
  /// expressions, expanding pack expansions if required.
  ///
  /// \param Exprs The list of expressions to substitute into.
  ///
  /// \param NumExprs The number of expressions in \p Exprs.
  ///
  /// \param IsCall Whether this is some form of call, in which case
  /// default arguments will be dropped.
  ///
  /// \param TemplateArgs The set of template arguments to substitute.
  ///
  /// \param Outputs Will receive all of the substituted arguments.
  ///
  /// \returns true if an error occurred, false otherwise.
  bool SubstExprs(Expr **Exprs, unsigned NumExprs, bool IsCall,
                  const MultiLevelTemplateArgumentList &TemplateArgs,
                  SmallVectorImpl<Expr *> &Outputs);

  StmtResult SubstStmt(Stmt *S,
                       const MultiLevelTemplateArgumentList &TemplateArgs);

  Decl *SubstDecl(Decl *D, DeclContext *Owner,
                  const MultiLevelTemplateArgumentList &TemplateArgs);

  ExprResult SubstInitializer(Expr *E,
                       const MultiLevelTemplateArgumentList &TemplateArgs,
                       bool CXXDirectInit);

  bool
  SubstBaseSpecifiers(CXXRecordDecl *Instantiation,
                      CXXRecordDecl *Pattern,
                      const MultiLevelTemplateArgumentList &TemplateArgs);

  bool
  InstantiateClass(SourceLocation PointOfInstantiation,
                   CXXRecordDecl *Instantiation, CXXRecordDecl *Pattern,
                   const MultiLevelTemplateArgumentList &TemplateArgs,
                   TemplateSpecializationKind TSK,
                   bool Complain = true);

  bool InstantiateEnum(SourceLocation PointOfInstantiation,
                       EnumDecl *Instantiation, EnumDecl *Pattern,
                       const MultiLevelTemplateArgumentList &TemplateArgs,
                       TemplateSpecializationKind TSK);

  bool InstantiateInClassInitializer(
      SourceLocation PointOfInstantiation, FieldDecl *Instantiation,
      FieldDecl *Pattern, const MultiLevelTemplateArgumentList &TemplateArgs);

  struct LateInstantiatedAttribute {
    const Attr *TmplAttr;
    LocalInstantiationScope *Scope;
    Decl *NewDecl;

    LateInstantiatedAttribute(const Attr *A, LocalInstantiationScope *S,
                              Decl *D)
      : TmplAttr(A), Scope(S), NewDecl(D)
    { }
  };
  typedef SmallVector<LateInstantiatedAttribute, 16> LateInstantiatedAttrVec;

  void InstantiateAttrs(const MultiLevelTemplateArgumentList &TemplateArgs,
                        const Decl *Pattern, Decl *Inst,
                        LateInstantiatedAttrVec *LateAttrs = nullptr,
                        LocalInstantiationScope *OuterMostScope = nullptr);

  bool
  InstantiateClassTemplateSpecialization(SourceLocation PointOfInstantiation,
                           ClassTemplateSpecializationDecl *ClassTemplateSpec,
                           TemplateSpecializationKind TSK,
                           bool Complain = true);

  void InstantiateClassMembers(SourceLocation PointOfInstantiation,
                               CXXRecordDecl *Instantiation,
                            const MultiLevelTemplateArgumentList &TemplateArgs,
                               TemplateSpecializationKind TSK);

  void InstantiateClassTemplateSpecializationMembers(
                                          SourceLocation PointOfInstantiation,
                           ClassTemplateSpecializationDecl *ClassTemplateSpec,
                                                TemplateSpecializationKind TSK);

  NestedNameSpecifierLoc
  SubstNestedNameSpecifierLoc(NestedNameSpecifierLoc NNS,
                           const MultiLevelTemplateArgumentList &TemplateArgs);

  DeclarationNameInfo
  SubstDeclarationNameInfo(const DeclarationNameInfo &NameInfo,
                           const MultiLevelTemplateArgumentList &TemplateArgs);
  TemplateName
  SubstTemplateName(NestedNameSpecifierLoc QualifierLoc, TemplateName Name,
                    SourceLocation Loc,
                    const MultiLevelTemplateArgumentList &TemplateArgs);
  bool Subst(const TemplateArgumentLoc *Args, unsigned NumArgs,
             TemplateArgumentListInfo &Result,
             const MultiLevelTemplateArgumentList &TemplateArgs);

  void InstantiateExceptionSpec(SourceLocation PointOfInstantiation,
                                FunctionDecl *Function);
  void InstantiateFunctionDefinition(SourceLocation PointOfInstantiation,
                                     FunctionDecl *Function,
                                     bool Recursive = false,
                                     bool DefinitionRequired = false);
  VarTemplateSpecializationDecl *BuildVarTemplateInstantiation(
      VarTemplateDecl *VarTemplate, VarDecl *FromVar,
      const TemplateArgumentList &TemplateArgList,
      const TemplateArgumentListInfo &TemplateArgsInfo,
      SmallVectorImpl<TemplateArgument> &Converted,
      SourceLocation PointOfInstantiation, void *InsertPos,
      LateInstantiatedAttrVec *LateAttrs = nullptr,
      LocalInstantiationScope *StartingScope = nullptr);
  VarTemplateSpecializationDecl *CompleteVarTemplateSpecializationDecl(
      VarTemplateSpecializationDecl *VarSpec, VarDecl *PatternDecl,
      const MultiLevelTemplateArgumentList &TemplateArgs);
  void
  BuildVariableInstantiation(VarDecl *NewVar, VarDecl *OldVar,
                             const MultiLevelTemplateArgumentList &TemplateArgs,
                             LateInstantiatedAttrVec *LateAttrs,
                             DeclContext *Owner,
                             LocalInstantiationScope *StartingScope,
                             bool InstantiatingVarTemplate = false);
  void InstantiateVariableInitializer(
      VarDecl *Var, VarDecl *OldVar,
      const MultiLevelTemplateArgumentList &TemplateArgs);
  void InstantiateVariableDefinition(SourceLocation PointOfInstantiation,
                                     VarDecl *Var, bool Recursive = false,
                                     bool DefinitionRequired = false);
  void InstantiateStaticDataMemberDefinition(
                                     SourceLocation PointOfInstantiation,
                                     VarDecl *Var,
                                     bool Recursive = false,
                                     bool DefinitionRequired = false);

  void InstantiateMemInitializers(CXXConstructorDecl *New,
                                  const CXXConstructorDecl *Tmpl,
                            const MultiLevelTemplateArgumentList &TemplateArgs);

  NamedDecl *FindInstantiatedDecl(SourceLocation Loc, NamedDecl *D,
                          const MultiLevelTemplateArgumentList &TemplateArgs);
  DeclContext *FindInstantiatedContext(SourceLocation Loc, DeclContext *DC,
                          const MultiLevelTemplateArgumentList &TemplateArgs);

  // Objective-C declarations.
  enum ObjCContainerKind {
    OCK_None = -1,
    OCK_Interface = 0,
    OCK_Protocol,
    OCK_Category,
    OCK_ClassExtension,
    OCK_Implementation,
    OCK_CategoryImplementation
  };
  ObjCContainerKind getObjCContainerKind() const;

  DeclResult actOnObjCTypeParam(Scope *S,
                                ObjCTypeParamVariance variance,
                                SourceLocation varianceLoc,
                                unsigned index,
                                IdentifierInfo *paramName,
                                SourceLocation paramLoc,
                                SourceLocation colonLoc,
                                ParsedType typeBound);

  ObjCTypeParamList *actOnObjCTypeParamList(Scope *S, SourceLocation lAngleLoc,
                                            ArrayRef<Decl *> typeParams,
                                            SourceLocation rAngleLoc);
  void popObjCTypeParamList(Scope *S, ObjCTypeParamList *typeParamList);

  Decl *ActOnStartClassInterface(Scope *S,
                                 SourceLocation AtInterfaceLoc,
                                 IdentifierInfo *ClassName,
                                 SourceLocation ClassLoc,
                                 ObjCTypeParamList *typeParamList,
                                 IdentifierInfo *SuperName,
                                 SourceLocation SuperLoc,
                                 ArrayRef<ParsedType> SuperTypeArgs,
                                 SourceRange SuperTypeArgsRange,
                                 Decl * const *ProtoRefs,
                                 unsigned NumProtoRefs,
                                 const SourceLocation *ProtoLocs,
                                 SourceLocation EndProtoLoc,
                                 AttributeList *AttrList);
    
  void ActOnSuperClassOfClassInterface(Scope *S,
                                       SourceLocation AtInterfaceLoc,
                                       ObjCInterfaceDecl *IDecl,
                                       IdentifierInfo *ClassName,
                                       SourceLocation ClassLoc,
                                       IdentifierInfo *SuperName,
                                       SourceLocation SuperLoc,
                                       ArrayRef<ParsedType> SuperTypeArgs,
                                       SourceRange SuperTypeArgsRange);
  
  void ActOnTypedefedProtocols(SmallVectorImpl<Decl *> &ProtocolRefs,
                               IdentifierInfo *SuperName,
                               SourceLocation SuperLoc);

  Decl *ActOnCompatibilityAlias(
                    SourceLocation AtCompatibilityAliasLoc,
                    IdentifierInfo *AliasName,  SourceLocation AliasLocation,
                    IdentifierInfo *ClassName, SourceLocation ClassLocation);

  bool CheckForwardProtocolDeclarationForCircularDependency(
    IdentifierInfo *PName,
    SourceLocation &PLoc, SourceLocation PrevLoc,
    const ObjCList<ObjCProtocolDecl> &PList);

  Decl *ActOnStartProtocolInterface(
                    SourceLocation AtProtoInterfaceLoc,
                    IdentifierInfo *ProtocolName, SourceLocation ProtocolLoc,
                    Decl * const *ProtoRefNames, unsigned NumProtoRefs,
                    const SourceLocation *ProtoLocs,
                    SourceLocation EndProtoLoc,
                    AttributeList *AttrList);

  Decl *ActOnStartCategoryInterface(SourceLocation AtInterfaceLoc,
                                    IdentifierInfo *ClassName,
                                    SourceLocation ClassLoc,
                                    ObjCTypeParamList *typeParamList,
                                    IdentifierInfo *CategoryName,
                                    SourceLocation CategoryLoc,
                                    Decl * const *ProtoRefs,
                                    unsigned NumProtoRefs,
                                    const SourceLocation *ProtoLocs,
                                    SourceLocation EndProtoLoc);

  Decl *ActOnStartClassImplementation(
                    SourceLocation AtClassImplLoc,
                    IdentifierInfo *ClassName, SourceLocation ClassLoc,
                    IdentifierInfo *SuperClassname,
                    SourceLocation SuperClassLoc);

  Decl *ActOnStartCategoryImplementation(SourceLocation AtCatImplLoc,
                                         IdentifierInfo *ClassName,
                                         SourceLocation ClassLoc,
                                         IdentifierInfo *CatName,
                                         SourceLocation CatLoc);

  DeclGroupPtrTy ActOnFinishObjCImplementation(Decl *ObjCImpDecl,
                                               ArrayRef<Decl *> Decls);

  DeclGroupPtrTy ActOnForwardClassDeclaration(SourceLocation Loc,
                   IdentifierInfo **IdentList,
                   SourceLocation *IdentLocs,
                   ArrayRef<ObjCTypeParamList *> TypeParamLists,
                   unsigned NumElts);

  DeclGroupPtrTy ActOnForwardProtocolDeclaration(SourceLocation AtProtoclLoc,
                                        const IdentifierLocPair *IdentList,
                                        unsigned NumElts,
                                        AttributeList *attrList);

  void FindProtocolDeclaration(bool WarnOnDeclarations, bool ForObjCContainer,
                               const IdentifierLocPair *ProtocolId,
                               unsigned NumProtocols,
                               SmallVectorImpl<Decl *> &Protocols);

  /// Given a list of identifiers (and their locations), resolve the
  /// names to either Objective-C protocol qualifiers or type
  /// arguments, as appropriate.
  void actOnObjCTypeArgsOrProtocolQualifiers(
         Scope *S,
         ParsedType baseType,
         SourceLocation lAngleLoc,
         ArrayRef<IdentifierInfo *> identifiers,
         ArrayRef<SourceLocation> identifierLocs,
         SourceLocation rAngleLoc,
         SourceLocation &typeArgsLAngleLoc,
         SmallVectorImpl<ParsedType> &typeArgs,
         SourceLocation &typeArgsRAngleLoc,
         SourceLocation &protocolLAngleLoc,
         SmallVectorImpl<Decl *> &protocols,
         SourceLocation &protocolRAngleLoc,
         bool warnOnIncompleteProtocols);

  /// Build a an Objective-C protocol-qualified 'id' type where no
  /// base type was specified.
  TypeResult actOnObjCProtocolQualifierType(
               SourceLocation lAngleLoc,
               ArrayRef<Decl *> protocols,
               ArrayRef<SourceLocation> protocolLocs,
               SourceLocation rAngleLoc);

  /// Build a specialized and/or protocol-qualified Objective-C type.
  TypeResult actOnObjCTypeArgsAndProtocolQualifiers(
               Scope *S,
               SourceLocation Loc,
               ParsedType BaseType,
               SourceLocation TypeArgsLAngleLoc,
               ArrayRef<ParsedType> TypeArgs,
               SourceLocation TypeArgsRAngleLoc,
               SourceLocation ProtocolLAngleLoc,
               ArrayRef<Decl *> Protocols,
               ArrayRef<SourceLocation> ProtocolLocs,
               SourceLocation ProtocolRAngleLoc);

  /// Build an Objective-C object pointer type.
  QualType BuildObjCObjectType(QualType BaseType,
                               SourceLocation Loc,
                               SourceLocation TypeArgsLAngleLoc,
                               ArrayRef<TypeSourceInfo *> TypeArgs,
                               SourceLocation TypeArgsRAngleLoc,
                               SourceLocation ProtocolLAngleLoc,
                               ArrayRef<ObjCProtocolDecl *> Protocols,
                               ArrayRef<SourceLocation> ProtocolLocs,
                               SourceLocation ProtocolRAngleLoc,
                               bool FailOnError = false);

  /// Check the application of the Objective-C '__kindof' qualifier to
  /// the given type.
  bool checkObjCKindOfType(QualType &type, SourceLocation loc);

  /// Ensure attributes are consistent with type.
  /// \param [in, out] Attributes The attributes to check; they will
  /// be modified to be consistent with \p PropertyTy.
  void CheckObjCPropertyAttributes(Decl *PropertyPtrTy,
                                   SourceLocation Loc,
                                   unsigned &Attributes,
                                   bool propertyInPrimaryClass);

  /// Process the specified property declaration and create decls for the
  /// setters and getters as needed.
  /// \param property The property declaration being processed
  /// \param CD The semantic container for the property
  /// \param redeclaredProperty Declaration for property if redeclared
  ///        in class extension.
  /// \param lexicalDC Container for redeclaredProperty.
  void ProcessPropertyDecl(ObjCPropertyDecl *property,
                           ObjCContainerDecl *CD,
                           ObjCPropertyDecl *redeclaredProperty = nullptr,
                           ObjCContainerDecl *lexicalDC = nullptr);


  void DiagnosePropertyMismatch(ObjCPropertyDecl *Property,
                                ObjCPropertyDecl *SuperProperty,
                                const IdentifierInfo *Name,
                                bool OverridingProtocolProperty);

  void DiagnoseClassExtensionDupMethods(ObjCCategoryDecl *CAT,
                                        ObjCInterfaceDecl *ID);

  Decl *ActOnAtEnd(Scope *S, SourceRange AtEnd,
                   ArrayRef<Decl *> allMethods = None,
                   ArrayRef<DeclGroupPtrTy> allTUVars = None);

  Decl *ActOnProperty(Scope *S, SourceLocation AtLoc,
                      SourceLocation LParenLoc,
                      FieldDeclarator &FD, ObjCDeclSpec &ODS,
                      Selector GetterSel, Selector SetterSel,
                      bool *OverridingProperty,
                      tok::ObjCKeywordKind MethodImplKind,
                      DeclContext *lexicalDC = nullptr);

  Decl *ActOnPropertyImplDecl(Scope *S,
                              SourceLocation AtLoc,
                              SourceLocation PropertyLoc,
                              bool ImplKind,
                              IdentifierInfo *PropertyId,
                              IdentifierInfo *PropertyIvar,
                              SourceLocation PropertyIvarLoc);

  enum ObjCSpecialMethodKind {
    OSMK_None,
    OSMK_Alloc,
    OSMK_New,
    OSMK_Copy,
    OSMK_RetainingInit,
    OSMK_NonRetainingInit
  };

  struct ObjCArgInfo {
    IdentifierInfo *Name;
    SourceLocation NameLoc;
    // The Type is null if no type was specified, and the DeclSpec is invalid
    // in this case.
    ParsedType Type;
    ObjCDeclSpec DeclSpec;

    /// ArgAttrs - Attribute list for this argument.
    AttributeList *ArgAttrs;
  };

  Decl *ActOnMethodDeclaration(
    Scope *S,
    SourceLocation BeginLoc, // location of the + or -.
    SourceLocation EndLoc,   // location of the ; or {.
    tok::TokenKind MethodType,
    ObjCDeclSpec &ReturnQT, ParsedType ReturnType,
    ArrayRef<SourceLocation> SelectorLocs, Selector Sel,
    // optional arguments. The number of types/arguments is obtained
    // from the Sel.getNumArgs().
    ObjCArgInfo *ArgInfo,
    DeclaratorChunk::ParamInfo *CParamInfo, unsigned CNumArgs, // c-style args
    AttributeList *AttrList, tok::ObjCKeywordKind MethodImplKind,
    bool isVariadic, bool MethodDefinition);

  ObjCMethodDecl *LookupMethodInQualifiedType(Selector Sel,
                                              const ObjCObjectPointerType *OPT,
                                              bool IsInstance);
  ObjCMethodDecl *LookupMethodInObjectType(Selector Sel, QualType Ty,
                                           bool IsInstance);

  bool CheckARCMethodDecl(ObjCMethodDecl *method);
  bool inferObjCARCLifetime(ValueDecl *decl);

  ExprResult
  HandleExprPropertyRefExpr(const ObjCObjectPointerType *OPT,
                            Expr *BaseExpr,
                            SourceLocation OpLoc,
                            DeclarationName MemberName,
                            SourceLocation MemberLoc,
                            SourceLocation SuperLoc, QualType SuperType,
                            bool Super);

  ExprResult
  ActOnClassPropertyRefExpr(IdentifierInfo &receiverName,
                            IdentifierInfo &propertyName,
                            SourceLocation receiverNameLoc,
                            SourceLocation propertyNameLoc);

  ObjCMethodDecl *tryCaptureObjCSelf(SourceLocation Loc);

  /// \brief Describes the kind of message expression indicated by a message
  /// send that starts with an identifier.
  enum ObjCMessageKind {
    /// \brief The message is sent to 'super'.
    ObjCSuperMessage,
    /// \brief The message is an instance message.
    ObjCInstanceMessage,
    /// \brief The message is a class message, and the identifier is a type
    /// name.
    ObjCClassMessage
  };

  ObjCMessageKind getObjCMessageKind(Scope *S,
                                     IdentifierInfo *Name,
                                     SourceLocation NameLoc,
                                     bool IsSuper,
                                     bool HasTrailingDot,
                                     ParsedType &ReceiverType);

  ExprResult ActOnSuperMessage(Scope *S, SourceLocation SuperLoc,
                               Selector Sel,
                               SourceLocation LBracLoc,
                               ArrayRef<SourceLocation> SelectorLocs,
                               SourceLocation RBracLoc,
                               MultiExprArg Args);

  ExprResult BuildClassMessage(TypeSourceInfo *ReceiverTypeInfo,
                               QualType ReceiverType,
                               SourceLocation SuperLoc,
                               Selector Sel,
                               ObjCMethodDecl *Method,
                               SourceLocation LBracLoc,
                               ArrayRef<SourceLocation> SelectorLocs,
                               SourceLocation RBracLoc,
                               MultiExprArg Args,
                               bool isImplicit = false);

  ExprResult BuildClassMessageImplicit(QualType ReceiverType,
                                       bool isSuperReceiver,
                                       SourceLocation Loc,
                                       Selector Sel,
                                       ObjCMethodDecl *Method,
                                       MultiExprArg Args);

  ExprResult ActOnClassMessage(Scope *S,
                               ParsedType Receiver,
                               Selector Sel,
                               SourceLocation LBracLoc,
                               ArrayRef<SourceLocation> SelectorLocs,
                               SourceLocation RBracLoc,
                               MultiExprArg Args);

  ExprResult BuildInstanceMessage(Expr *Receiver,
                                  QualType ReceiverType,
                                  SourceLocation SuperLoc,
                                  Selector Sel,
                                  ObjCMethodDecl *Method,
                                  SourceLocation LBracLoc,
                                  ArrayRef<SourceLocation> SelectorLocs,
                                  SourceLocation RBracLoc,
                                  MultiExprArg Args,
                                  bool isImplicit = false);

  ExprResult BuildInstanceMessageImplicit(Expr *Receiver,
                                          QualType ReceiverType,
                                          SourceLocation Loc,
                                          Selector Sel,
                                          ObjCMethodDecl *Method,
                                          MultiExprArg Args);

  ExprResult ActOnInstanceMessage(Scope *S,
                                  Expr *Receiver,
                                  Selector Sel,
                                  SourceLocation LBracLoc,
                                  ArrayRef<SourceLocation> SelectorLocs,
                                  SourceLocation RBracLoc,
                                  MultiExprArg Args);

  ExprResult BuildObjCBridgedCast(SourceLocation LParenLoc,
                                  ObjCBridgeCastKind Kind,
                                  SourceLocation BridgeKeywordLoc,
                                  TypeSourceInfo *TSInfo,
                                  Expr *SubExpr);

  ExprResult ActOnObjCBridgedCast(Scope *S,
                                  SourceLocation LParenLoc,
                                  ObjCBridgeCastKind Kind,
                                  SourceLocation BridgeKeywordLoc,
                                  ParsedType Type,
                                  SourceLocation RParenLoc,
                                  Expr *SubExpr);
  
  void CheckTollFreeBridgeCast(QualType castType, Expr *castExpr);
  
  void CheckObjCBridgeRelatedCast(QualType castType, Expr *castExpr);
  
  bool CheckTollFreeBridgeStaticCast(QualType castType, Expr *castExpr,
                                     CastKind &Kind);
  
  bool checkObjCBridgeRelatedComponents(SourceLocation Loc,
                                        QualType DestType, QualType SrcType,
                                        ObjCInterfaceDecl *&RelatedClass,
                                        ObjCMethodDecl *&ClassMethod,
                                        ObjCMethodDecl *&InstanceMethod,
                                        TypedefNameDecl *&TDNDecl,
                                        bool CfToNs);
  
  bool CheckObjCBridgeRelatedConversions(SourceLocation Loc,
                                         QualType DestType, QualType SrcType,
                                         Expr *&SrcExpr);
  
  bool ConversionToObjCStringLiteralCheck(QualType DstType, Expr *&SrcExpr);
  
  bool checkInitMethod(ObjCMethodDecl *method, QualType receiverTypeIfCall);

  /// \brief Check whether the given new method is a valid override of the
  /// given overridden method, and set any properties that should be inherited.
  void CheckObjCMethodOverride(ObjCMethodDecl *NewMethod,
                               const ObjCMethodDecl *Overridden);

  /// \brief Describes the compatibility of a result type with its method.
  enum ResultTypeCompatibilityKind {
    RTC_Compatible,
    RTC_Incompatible,
    RTC_Unknown
  };

  void CheckObjCMethodOverrides(ObjCMethodDecl *ObjCMethod,
                                ObjCInterfaceDecl *CurrentClass,
                                ResultTypeCompatibilityKind RTC);

  enum PragmaOptionsAlignKind {
    POAK_Native,  // #pragma options align=native
    POAK_Natural, // #pragma options align=natural
    POAK_Packed,  // #pragma options align=packed
    POAK_Power,   // #pragma options align=power
    POAK_Mac68k,  // #pragma options align=mac68k
    POAK_Reset    // #pragma options align=reset
  };

  /// ActOnPragmaOptionsAlign - Called on well formed \#pragma options align.
  void ActOnPragmaOptionsAlign(PragmaOptionsAlignKind Kind,
                               SourceLocation PragmaLoc);

  enum PragmaPackKind {
    PPK_Default, // #pragma pack([n])
    PPK_Show,    // #pragma pack(show), only supported by MSVC.
    PPK_Push,    // #pragma pack(push, [identifier], [n])
    PPK_Pop      // #pragma pack(pop, [identifier], [n])
  };

  enum PragmaMSStructKind {
    PMSST_OFF,  // #pragms ms_struct off
    PMSST_ON    // #pragms ms_struct on
  };

  enum PragmaMSCommentKind {
    PCK_Unknown,
    PCK_Linker,   // #pragma comment(linker, ...)
    PCK_Lib,      // #pragma comment(lib, ...)
    PCK_Compiler, // #pragma comment(compiler, ...)
    PCK_ExeStr,   // #pragma comment(exestr, ...)
    PCK_User      // #pragma comment(user, ...)
  };

  /// ActOnPragmaPack - Called on well formed \#pragma pack(...).
  void ActOnPragmaPack(PragmaPackKind Kind,
                       IdentifierInfo *Name,
                       Expr *Alignment,
                       SourceLocation PragmaLoc,
                       SourceLocation LParenLoc,
                       SourceLocation RParenLoc);

  /// ActOnPragmaPackMatrix - Called on well formed \#pragma pack_matrix(...).
  void ActOnPragmaPackMatrix(bool bRowMajor, SourceLocation PragmaLoc);

  /// ActOnPragmaMSStruct - Called on well formed \#pragma ms_struct [on|off].
  void ActOnPragmaMSStruct(PragmaMSStructKind Kind);

  /// ActOnPragmaMSComment - Called on well formed
  /// \#pragma comment(kind, "arg").
  void ActOnPragmaMSComment(PragmaMSCommentKind Kind, StringRef Arg);

  /// ActOnPragmaMSPointersToMembers - called on well formed \#pragma
  /// pointers_to_members(representation method[, general purpose
  /// representation]).
  void ActOnPragmaMSPointersToMembers(
      LangOptions::PragmaMSPointersToMembersKind Kind,
      SourceLocation PragmaLoc);

  /// \brief Called on well formed \#pragma vtordisp().
  void ActOnPragmaMSVtorDisp(PragmaVtorDispKind Kind, SourceLocation PragmaLoc,
                             MSVtorDispAttr::Mode Value);

  enum PragmaSectionKind {
    PSK_DataSeg,
    PSK_BSSSeg,
    PSK_ConstSeg,
    PSK_CodeSeg,
  };

  bool UnifySection(StringRef SectionName,
                    int SectionFlags,
                    DeclaratorDecl *TheDecl);
  bool UnifySection(StringRef SectionName,
                    int SectionFlags,
                    SourceLocation PragmaSectionLocation);

  /// \brief Called on well formed \#pragma bss_seg/data_seg/const_seg/code_seg.
  void ActOnPragmaMSSeg(SourceLocation PragmaLocation,
                        PragmaMsStackAction Action,
                        llvm::StringRef StackSlotLabel,
                        StringLiteral *SegmentName,
                        llvm::StringRef PragmaName);

  /// \brief Called on well formed \#pragma section().
  void ActOnPragmaMSSection(SourceLocation PragmaLocation,
                            int SectionFlags, StringLiteral *SegmentName);

  /// \brief Called on well-formed \#pragma init_seg().
  void ActOnPragmaMSInitSeg(SourceLocation PragmaLocation,
                            StringLiteral *SegmentName);

  /// ActOnPragmaDetectMismatch - Call on well-formed \#pragma detect_mismatch
  void ActOnPragmaDetectMismatch(StringRef Name, StringRef Value);

  /// ActOnPragmaUnused - Called on well-formed '\#pragma unused'.
  void ActOnPragmaUnused(const Token &Identifier,
                         Scope *curScope,
                         SourceLocation PragmaLoc);

  /// ActOnPragmaVisibility - Called on well formed \#pragma GCC visibility... .
  void ActOnPragmaVisibility(const IdentifierInfo* VisType,
                             SourceLocation PragmaLoc);

  NamedDecl *DeclClonePragmaWeak(NamedDecl *ND, IdentifierInfo *II,
                                 SourceLocation Loc);
  void DeclApplyPragmaWeak(Scope *S, NamedDecl *ND, WeakInfo &W);

  /// ActOnPragmaWeakID - Called on well formed \#pragma weak ident.
  void ActOnPragmaWeakID(IdentifierInfo* WeakName,
                         SourceLocation PragmaLoc,
                         SourceLocation WeakNameLoc);

  /// ActOnPragmaRedefineExtname - Called on well formed
  /// \#pragma redefine_extname oldname newname.
  void ActOnPragmaRedefineExtname(IdentifierInfo* WeakName,
                                  IdentifierInfo* AliasName,
                                  SourceLocation PragmaLoc,
                                  SourceLocation WeakNameLoc,
                                  SourceLocation AliasNameLoc);

  /// ActOnPragmaWeakAlias - Called on well formed \#pragma weak ident = ident.
  void ActOnPragmaWeakAlias(IdentifierInfo* WeakName,
                            IdentifierInfo* AliasName,
                            SourceLocation PragmaLoc,
                            SourceLocation WeakNameLoc,
                            SourceLocation AliasNameLoc);

  /// ActOnPragmaFPContract - Called on well formed
  /// \#pragma {STDC,OPENCL} FP_CONTRACT
  void ActOnPragmaFPContract(tok::OnOffSwitch OOS);

  /// AddAlignmentAttributesForRecord - Adds any needed alignment attributes to
  /// a the record decl, to handle '\#pragma pack' and '\#pragma options align'.
  void AddAlignmentAttributesForRecord(RecordDecl *RD);

  /// AddMsStructLayoutForRecord - Adds ms_struct layout attribute to record.
  void AddMsStructLayoutForRecord(RecordDecl *RD);

  /// FreePackedContext - Deallocate and null out PackContext.
  void FreePackedContext();

  /// PushNamespaceVisibilityAttr - Note that we've entered a
  /// namespace with a visibility attribute.
  void PushNamespaceVisibilityAttr(const VisibilityAttr *Attr,
                                   SourceLocation Loc);

  /// AddPushedVisibilityAttribute - If '\#pragma GCC visibility' was used,
  /// add an appropriate visibility attribute.
  void AddPushedVisibilityAttribute(Decl *RD);

  /// PopPragmaVisibility - Pop the top element of the visibility stack; used
  /// for '\#pragma GCC visibility' and visibility attributes on namespaces.
  void PopPragmaVisibility(bool IsNamespaceEnd, SourceLocation EndLoc);

  /// FreeVisContext - Deallocate and null out VisContext.
  void FreeVisContext();

  /// AddCFAuditedAttribute - Check whether we're currently within
  /// '\#pragma clang arc_cf_code_audited' and, if so, consider adding
  /// the appropriate attribute.
  void AddCFAuditedAttribute(Decl *D);

  /// \brief Called on well formed \#pragma clang optimize.
  void ActOnPragmaOptimize(bool On, SourceLocation PragmaLoc);

  /// \brief Get the location for the currently active "\#pragma clang optimize
  /// off". If this location is invalid, then the state of the pragma is "on".
  SourceLocation getOptimizeOffPragmaLocation() const {
    return OptimizeOffPragmaLocation;
  }

  /// \brief Only called on function definitions; if there is a pragma in scope
  /// with the effect of a range-based optnone, consider marking the function
  /// with attribute optnone.
  void AddRangeBasedOptnone(FunctionDecl *FD);

  /// \brief Adds the 'optnone' attribute to the function declaration if there
  /// are no conflicts; Loc represents the location causing the 'optnone'
  /// attribute to be added (usually because of a pragma).
  void AddOptnoneAttributeIfNoConflicts(FunctionDecl *FD, SourceLocation Loc);

  /// AddAlignedAttr - Adds an aligned attribute to a particular declaration.
  void AddAlignedAttr(SourceRange AttrRange, Decl *D, Expr *E,
                      unsigned SpellingListIndex, bool IsPackExpansion);
  void AddAlignedAttr(SourceRange AttrRange, Decl *D, TypeSourceInfo *T,
                      unsigned SpellingListIndex, bool IsPackExpansion);

  /// AddAssumeAlignedAttr - Adds an assume_aligned attribute to a particular
  /// declaration.
  void AddAssumeAlignedAttr(SourceRange AttrRange, Decl *D, Expr *E, Expr *OE,
                            unsigned SpellingListIndex);

  /// AddAlignValueAttr - Adds an align_value attribute to a particular
  /// declaration.
  void AddAlignValueAttr(SourceRange AttrRange, Decl *D, Expr *E,
                         unsigned SpellingListIndex);

  /// AddLaunchBoundsAttr - Adds a launch_bounds attribute to a particular
  /// declaration.
  void AddLaunchBoundsAttr(SourceRange AttrRange, Decl *D, Expr *MaxThreads,
                           Expr *MinBlocks, unsigned SpellingListIndex);

  // OpenMP directives and clauses.
private:
  void *VarDataSharingAttributesStack;
  /// \brief Initialization of data-sharing attributes stack.
  void InitDataSharingAttributesStack();
  void DestroyDataSharingAttributesStack();
  ExprResult VerifyPositiveIntegerConstantInClause(Expr *Op,
                                                   OpenMPClauseKind CKind);
public:
  /// \brief Check if the specified variable is used in a private clause in
  /// Checks if the specified variable is used in one of the private
  /// clauses in OpenMP constructs.
  bool IsOpenMPCapturedVar(VarDecl *VD);

  /// OpenMP constructs.
  /// \param Level Relative level of nested OpenMP construct for that the check
  /// is performed.
  bool isOpenMPPrivateVar(VarDecl *VD, unsigned Level);

  ExprResult PerformOpenMPImplicitIntegerConversion(SourceLocation OpLoc,
                                                    Expr *Op);
  /// \brief Called on start of new data sharing attribute block.
  void StartOpenMPDSABlock(OpenMPDirectiveKind K,
                           const DeclarationNameInfo &DirName, Scope *CurScope,
                           SourceLocation Loc);
  /// \brief Start analysis of clauses.
  void StartOpenMPClause(OpenMPClauseKind K);
  /// \brief End analysis of clauses.
  void EndOpenMPClause();
  /// \brief Called on end of data sharing attribute block.
  void EndOpenMPDSABlock(Stmt *CurDirective);

  /// \brief Check if the current region is an OpenMP loop region and if it is,
  /// mark loop control variable, used in \p Init for loop initialization, as
  /// private by default.
  /// \param Init First part of the for loop.
  void ActOnOpenMPLoopInitialization(SourceLocation ForLoc, Stmt *Init);

  // OpenMP directives and clauses.
  /// \brief Called on correct id-expression from the '#pragma omp
  /// threadprivate'.
  ExprResult ActOnOpenMPIdExpression(Scope *CurScope,
                                     CXXScopeSpec &ScopeSpec,
                                     const DeclarationNameInfo &Id);
  /// \brief Called on well-formed '#pragma omp threadprivate'.
  DeclGroupPtrTy ActOnOpenMPThreadprivateDirective(
                                     SourceLocation Loc,
                                     ArrayRef<Expr *> VarList);
  /// \brief Builds a new OpenMPThreadPrivateDecl and checks its correctness.
  OMPThreadPrivateDecl *CheckOMPThreadPrivateDecl(
                                     SourceLocation Loc,
                                     ArrayRef<Expr *> VarList);

  /// \brief Initialization of captured region for OpenMP region.
  void ActOnOpenMPRegionStart(OpenMPDirectiveKind DKind, Scope *CurScope);
  /// \brief End of OpenMP region.
  ///
  /// \param S Statement associated with the current OpenMP region.
  /// \param Clauses List of clauses for the current OpenMP region.
  ///
  /// \returns Statement for finished OpenMP region.
  StmtResult ActOnOpenMPRegionEnd(StmtResult S, ArrayRef<OMPClause *> Clauses);
  StmtResult ActOnOpenMPExecutableDirective(
      OpenMPDirectiveKind Kind, const DeclarationNameInfo &DirName,
      OpenMPDirectiveKind CancelRegion, ArrayRef<OMPClause *> Clauses,
      Stmt *AStmt, SourceLocation StartLoc, SourceLocation EndLoc);
  /// \brief Called on well-formed '\#pragma omp parallel' after parsing
  /// of the  associated statement.
  StmtResult ActOnOpenMPParallelDirective(ArrayRef<OMPClause *> Clauses,
                                          Stmt *AStmt,
                                          SourceLocation StartLoc,
                                          SourceLocation EndLoc);
  /// \brief Called on well-formed '\#pragma omp simd' after parsing
  /// of the associated statement.
  StmtResult ActOnOpenMPSimdDirective(
      ArrayRef<OMPClause *> Clauses, Stmt *AStmt, SourceLocation StartLoc,
      SourceLocation EndLoc,
      llvm::DenseMap<VarDecl *, Expr *> &VarsWithImplicitDSA);
  /// \brief Called on well-formed '\#pragma omp for' after parsing
  /// of the associated statement.
  StmtResult ActOnOpenMPForDirective(
      ArrayRef<OMPClause *> Clauses, Stmt *AStmt, SourceLocation StartLoc,
      SourceLocation EndLoc,
      llvm::DenseMap<VarDecl *, Expr *> &VarsWithImplicitDSA);
  /// \brief Called on well-formed '\#pragma omp for simd' after parsing
  /// of the associated statement.
  StmtResult ActOnOpenMPForSimdDirective(
      ArrayRef<OMPClause *> Clauses, Stmt *AStmt, SourceLocation StartLoc,
      SourceLocation EndLoc,
      llvm::DenseMap<VarDecl *, Expr *> &VarsWithImplicitDSA);
  /// \brief Called on well-formed '\#pragma omp sections' after parsing
  /// of the associated statement.
  StmtResult ActOnOpenMPSectionsDirective(ArrayRef<OMPClause *> Clauses,
                                          Stmt *AStmt, SourceLocation StartLoc,
                                          SourceLocation EndLoc);
  /// \brief Called on well-formed '\#pragma omp section' after parsing of the
  /// associated statement.
  StmtResult ActOnOpenMPSectionDirective(Stmt *AStmt, SourceLocation StartLoc,
                                         SourceLocation EndLoc);
  /// \brief Called on well-formed '\#pragma omp single' after parsing of the
  /// associated statement.
  StmtResult ActOnOpenMPSingleDirective(ArrayRef<OMPClause *> Clauses,
                                        Stmt *AStmt, SourceLocation StartLoc,
                                        SourceLocation EndLoc);
  /// \brief Called on well-formed '\#pragma omp master' after parsing of the
  /// associated statement.
  StmtResult ActOnOpenMPMasterDirective(Stmt *AStmt, SourceLocation StartLoc,
                                        SourceLocation EndLoc);
  /// \brief Called on well-formed '\#pragma omp critical' after parsing of the
  /// associated statement.
  StmtResult ActOnOpenMPCriticalDirective(const DeclarationNameInfo &DirName,
                                          Stmt *AStmt, SourceLocation StartLoc,
                                          SourceLocation EndLoc);
  /// \brief Called on well-formed '\#pragma omp parallel for' after parsing
  /// of the  associated statement.
  StmtResult ActOnOpenMPParallelForDirective(
      ArrayRef<OMPClause *> Clauses, Stmt *AStmt, SourceLocation StartLoc,
      SourceLocation EndLoc,
      llvm::DenseMap<VarDecl *, Expr *> &VarsWithImplicitDSA);
  /// \brief Called on well-formed '\#pragma omp parallel for simd' after
  /// parsing of the  associated statement.
  StmtResult ActOnOpenMPParallelForSimdDirective(
      ArrayRef<OMPClause *> Clauses, Stmt *AStmt, SourceLocation StartLoc,
      SourceLocation EndLoc,
      llvm::DenseMap<VarDecl *, Expr *> &VarsWithImplicitDSA);
  /// \brief Called on well-formed '\#pragma omp parallel sections' after
  /// parsing of the  associated statement.
  StmtResult ActOnOpenMPParallelSectionsDirective(ArrayRef<OMPClause *> Clauses,
                                                  Stmt *AStmt,
                                                  SourceLocation StartLoc,
                                                  SourceLocation EndLoc);
  /// \brief Called on well-formed '\#pragma omp task' after parsing of the
  /// associated statement.
  StmtResult ActOnOpenMPTaskDirective(ArrayRef<OMPClause *> Clauses,
                                      Stmt *AStmt, SourceLocation StartLoc,
                                      SourceLocation EndLoc);
  /// \brief Called on well-formed '\#pragma omp taskyield'.
  StmtResult ActOnOpenMPTaskyieldDirective(SourceLocation StartLoc,
                                           SourceLocation EndLoc);
  /// \brief Called on well-formed '\#pragma omp barrier'.
  StmtResult ActOnOpenMPBarrierDirective(SourceLocation StartLoc,
                                         SourceLocation EndLoc);
  /// \brief Called on well-formed '\#pragma omp taskwait'.
  StmtResult ActOnOpenMPTaskwaitDirective(SourceLocation StartLoc,
                                          SourceLocation EndLoc);
  /// \brief Called on well-formed '\#pragma omp taskgroup'.
  StmtResult ActOnOpenMPTaskgroupDirective(Stmt *AStmt, SourceLocation StartLoc,
                                           SourceLocation EndLoc);
  /// \brief Called on well-formed '\#pragma omp flush'.
  StmtResult ActOnOpenMPFlushDirective(ArrayRef<OMPClause *> Clauses,
                                       SourceLocation StartLoc,
                                       SourceLocation EndLoc);
  /// \brief Called on well-formed '\#pragma omp ordered' after parsing of the
  /// associated statement.
  StmtResult ActOnOpenMPOrderedDirective(Stmt *AStmt, SourceLocation StartLoc,
                                         SourceLocation EndLoc);
  /// \brief Called on well-formed '\#pragma omp atomic' after parsing of the
  /// associated statement.
  StmtResult ActOnOpenMPAtomicDirective(ArrayRef<OMPClause *> Clauses,
                                        Stmt *AStmt, SourceLocation StartLoc,
                                        SourceLocation EndLoc);
  /// \brief Called on well-formed '\#pragma omp target' after parsing of the
  /// associated statement.
  StmtResult ActOnOpenMPTargetDirective(ArrayRef<OMPClause *> Clauses,
                                        Stmt *AStmt, SourceLocation StartLoc,
                                        SourceLocation EndLoc);
  /// \brief Called on well-formed '\#pragma omp teams' after parsing of the
  /// associated statement.
  StmtResult ActOnOpenMPTeamsDirective(ArrayRef<OMPClause *> Clauses,
                                       Stmt *AStmt, SourceLocation StartLoc,
                                       SourceLocation EndLoc);
  /// \brief Called on well-formed '\#pragma omp cancellation point'.
  StmtResult
  ActOnOpenMPCancellationPointDirective(SourceLocation StartLoc,
                                        SourceLocation EndLoc,
                                        OpenMPDirectiveKind CancelRegion);
  /// \brief Called on well-formed '\#pragma omp cancel'.
  StmtResult ActOnOpenMPCancelDirective(SourceLocation StartLoc,
                                        SourceLocation EndLoc,
                                        OpenMPDirectiveKind CancelRegion);

  OMPClause *ActOnOpenMPSingleExprClause(OpenMPClauseKind Kind,
                                         Expr *Expr,
                                         SourceLocation StartLoc,
                                         SourceLocation LParenLoc,
                                         SourceLocation EndLoc);
  /// \brief Called on well-formed 'if' clause.
  OMPClause *ActOnOpenMPIfClause(Expr *Condition, SourceLocation StartLoc,
                                 SourceLocation LParenLoc,
                                 SourceLocation EndLoc);
  /// \brief Called on well-formed 'final' clause.
  OMPClause *ActOnOpenMPFinalClause(Expr *Condition, SourceLocation StartLoc,
                                    SourceLocation LParenLoc,
                                    SourceLocation EndLoc);
  /// \brief Called on well-formed 'num_threads' clause.
  OMPClause *ActOnOpenMPNumThreadsClause(Expr *NumThreads,
                                         SourceLocation StartLoc,
                                         SourceLocation LParenLoc,
                                         SourceLocation EndLoc);
  /// \brief Called on well-formed 'safelen' clause.
  OMPClause *ActOnOpenMPSafelenClause(Expr *Length,
                                      SourceLocation StartLoc,
                                      SourceLocation LParenLoc,
                                      SourceLocation EndLoc);
  /// \brief Called on well-formed 'collapse' clause.
  OMPClause *ActOnOpenMPCollapseClause(Expr *NumForLoops,
                                       SourceLocation StartLoc,
                                       SourceLocation LParenLoc,
                                       SourceLocation EndLoc);

  OMPClause *ActOnOpenMPSimpleClause(OpenMPClauseKind Kind,
                                     unsigned Argument,
                                     SourceLocation ArgumentLoc,
                                     SourceLocation StartLoc,
                                     SourceLocation LParenLoc,
                                     SourceLocation EndLoc);
  /// \brief Called on well-formed 'default' clause.
  OMPClause *ActOnOpenMPDefaultClause(OpenMPDefaultClauseKind Kind,
                                      SourceLocation KindLoc,
                                      SourceLocation StartLoc,
                                      SourceLocation LParenLoc,
                                      SourceLocation EndLoc);
  /// \brief Called on well-formed 'proc_bind' clause.
  OMPClause *ActOnOpenMPProcBindClause(OpenMPProcBindClauseKind Kind,
                                       SourceLocation KindLoc,
                                       SourceLocation StartLoc,
                                       SourceLocation LParenLoc,
                                       SourceLocation EndLoc);

  OMPClause *ActOnOpenMPSingleExprWithArgClause(OpenMPClauseKind Kind,
                                                unsigned Argument, Expr *Expr,
                                                SourceLocation StartLoc,
                                                SourceLocation LParenLoc,
                                                SourceLocation ArgumentLoc,
                                                SourceLocation CommaLoc,
                                                SourceLocation EndLoc);
  /// \brief Called on well-formed 'schedule' clause.
  OMPClause *ActOnOpenMPScheduleClause(OpenMPScheduleClauseKind Kind,
                                       Expr *ChunkSize, SourceLocation StartLoc,
                                       SourceLocation LParenLoc,
                                       SourceLocation KindLoc,
                                       SourceLocation CommaLoc,
                                       SourceLocation EndLoc);

  OMPClause *ActOnOpenMPClause(OpenMPClauseKind Kind, SourceLocation StartLoc,
                               SourceLocation EndLoc);
  /// \brief Called on well-formed 'ordered' clause.
  OMPClause *ActOnOpenMPOrderedClause(SourceLocation StartLoc,
                                      SourceLocation EndLoc);
  /// \brief Called on well-formed 'nowait' clause.
  OMPClause *ActOnOpenMPNowaitClause(SourceLocation StartLoc,
                                     SourceLocation EndLoc);
  /// \brief Called on well-formed 'untied' clause.
  OMPClause *ActOnOpenMPUntiedClause(SourceLocation StartLoc,
                                     SourceLocation EndLoc);
  /// \brief Called on well-formed 'mergeable' clause.
  OMPClause *ActOnOpenMPMergeableClause(SourceLocation StartLoc,
                                        SourceLocation EndLoc);
  /// \brief Called on well-formed 'read' clause.
  OMPClause *ActOnOpenMPReadClause(SourceLocation StartLoc,
                                   SourceLocation EndLoc);
  /// \brief Called on well-formed 'write' clause.
  OMPClause *ActOnOpenMPWriteClause(SourceLocation StartLoc,
                                    SourceLocation EndLoc);
  /// \brief Called on well-formed 'update' clause.
  OMPClause *ActOnOpenMPUpdateClause(SourceLocation StartLoc,
                                     SourceLocation EndLoc);
  /// \brief Called on well-formed 'capture' clause.
  OMPClause *ActOnOpenMPCaptureClause(SourceLocation StartLoc,
                                      SourceLocation EndLoc);
  /// \brief Called on well-formed 'seq_cst' clause.
  OMPClause *ActOnOpenMPSeqCstClause(SourceLocation StartLoc,
                                     SourceLocation EndLoc);

  OMPClause *ActOnOpenMPVarListClause(
      OpenMPClauseKind Kind, ArrayRef<Expr *> Vars, Expr *TailExpr,
      SourceLocation StartLoc, SourceLocation LParenLoc,
      SourceLocation ColonLoc, SourceLocation EndLoc,
      CXXScopeSpec &ReductionIdScopeSpec,
      const DeclarationNameInfo &ReductionId, OpenMPDependClauseKind DepKind,
      SourceLocation DepLoc);
  /// \brief Called on well-formed 'private' clause.
  OMPClause *ActOnOpenMPPrivateClause(ArrayRef<Expr *> VarList,
                                      SourceLocation StartLoc,
                                      SourceLocation LParenLoc,
                                      SourceLocation EndLoc);
  /// \brief Called on well-formed 'firstprivate' clause.
  OMPClause *ActOnOpenMPFirstprivateClause(ArrayRef<Expr *> VarList,
                                           SourceLocation StartLoc,
                                           SourceLocation LParenLoc,
                                           SourceLocation EndLoc);
  /// \brief Called on well-formed 'lastprivate' clause.
  OMPClause *ActOnOpenMPLastprivateClause(ArrayRef<Expr *> VarList,
                                          SourceLocation StartLoc,
                                          SourceLocation LParenLoc,
                                          SourceLocation EndLoc);
  /// \brief Called on well-formed 'shared' clause.
  OMPClause *ActOnOpenMPSharedClause(ArrayRef<Expr *> VarList,
                                     SourceLocation StartLoc,
                                     SourceLocation LParenLoc,
                                     SourceLocation EndLoc);
  /// \brief Called on well-formed 'reduction' clause.
  OMPClause *
  ActOnOpenMPReductionClause(ArrayRef<Expr *> VarList, SourceLocation StartLoc,
                             SourceLocation LParenLoc, SourceLocation ColonLoc,
                             SourceLocation EndLoc,
                             CXXScopeSpec &ReductionIdScopeSpec,
                             const DeclarationNameInfo &ReductionId);
  /// \brief Called on well-formed 'linear' clause.
  OMPClause *ActOnOpenMPLinearClause(ArrayRef<Expr *> VarList,
                                     Expr *Step,
                                     SourceLocation StartLoc,
                                     SourceLocation LParenLoc,
                                     SourceLocation ColonLoc,
                                     SourceLocation EndLoc);
  /// \brief Called on well-formed 'aligned' clause.
  OMPClause *ActOnOpenMPAlignedClause(ArrayRef<Expr *> VarList,
                                      Expr *Alignment,
                                      SourceLocation StartLoc,
                                      SourceLocation LParenLoc,
                                      SourceLocation ColonLoc,
                                      SourceLocation EndLoc);
  /// \brief Called on well-formed 'copyin' clause.
  OMPClause *ActOnOpenMPCopyinClause(ArrayRef<Expr *> VarList,
                                     SourceLocation StartLoc,
                                     SourceLocation LParenLoc,
                                     SourceLocation EndLoc);
  /// \brief Called on well-formed 'copyprivate' clause.
  OMPClause *ActOnOpenMPCopyprivateClause(ArrayRef<Expr *> VarList,
                                          SourceLocation StartLoc,
                                          SourceLocation LParenLoc,
                                          SourceLocation EndLoc);
  /// \brief Called on well-formed 'flush' pseudo clause.
  OMPClause *ActOnOpenMPFlushClause(ArrayRef<Expr *> VarList,
                                    SourceLocation StartLoc,
                                    SourceLocation LParenLoc,
                                    SourceLocation EndLoc);
  /// \brief Called on well-formed 'depend' clause.
  OMPClause *
  ActOnOpenMPDependClause(OpenMPDependClauseKind DepKind, SourceLocation DepLoc,
                          SourceLocation ColonLoc, ArrayRef<Expr *> VarList,
                          SourceLocation StartLoc, SourceLocation LParenLoc,
                          SourceLocation EndLoc);

  /// \brief The kind of conversion being performed.
  enum CheckedConversionKind {
    /// \brief An implicit conversion.
    CCK_ImplicitConversion,
    /// \brief A C-style cast.
    CCK_CStyleCast,
    /// \brief A functional-style cast.
    CCK_FunctionalCast,
    /// \brief A cast other than a C-style cast.
    CCK_OtherCast
  };

  /// ImpCastExprToType - If Expr is not of type 'Type', insert an implicit
  /// cast.  If there is already an implicit cast, merge into the existing one.
  /// If isLvalue, the result of the cast is an lvalue.
  ExprResult ImpCastExprToType(Expr *E, QualType Type, CastKind CK,
                               ExprValueKind VK = VK_RValue,
                               const CXXCastPath *BasePath = nullptr,
                               CheckedConversionKind CCK
                                  = CCK_ImplicitConversion);

  /// ScalarTypeToBooleanCastKind - Returns the cast kind corresponding
  /// to the conversion from scalar type ScalarTy to the Boolean type.
  static CastKind ScalarTypeToBooleanCastKind(QualType ScalarTy);

  /// IgnoredValueConversions - Given that an expression's result is
  /// syntactically ignored, perform any conversions that are
  /// required.
  ExprResult IgnoredValueConversions(Expr *E);

  // UsualUnaryConversions - promotes integers (C99 6.3.1.1p2) and converts
  // functions and arrays to their respective pointers (C99 6.3.2.1).
  ExprResult UsualUnaryConversions(Expr *E);

  /// CallExprUnaryConversions - a special case of an unary conversion
  /// performed on a function designator of a call expression.
  ExprResult CallExprUnaryConversions(Expr *E);

  // DefaultFunctionArrayConversion - converts functions and arrays
  // to their respective pointers (C99 6.3.2.1).
  ExprResult DefaultFunctionArrayConversion(Expr *E);

  // DefaultFunctionArrayLvalueConversion - converts functions and
  // arrays to their respective pointers and performs the
  // lvalue-to-rvalue conversion.
  ExprResult DefaultFunctionArrayLvalueConversion(Expr *E);

  // DefaultLvalueConversion - performs lvalue-to-rvalue conversion on
  // the operand.  This is DefaultFunctionArrayLvalueConversion,
  // except that it assumes the operand isn't of function or array
  // type.
  ExprResult DefaultLvalueConversion(Expr *E);

  // DefaultArgumentPromotion (C99 6.5.2.2p6). Used for function calls that
  // do not have a prototype. Integer promotions are performed on each
  // argument, and arguments that have type float are promoted to double.
  ExprResult DefaultArgumentPromotion(Expr *E);

  // Used for emitting the right warning by DefaultVariadicArgumentPromotion
  enum VariadicCallType {
    VariadicFunction,
    VariadicBlock,
    VariadicMethod,
    VariadicConstructor,
    VariadicDoesNotApply
  };

  VariadicCallType getVariadicCallType(FunctionDecl *FDecl,
                                       const FunctionProtoType *Proto,
                                       Expr *Fn);

  // Used for determining in which context a type is allowed to be passed to a
  // vararg function.
  enum VarArgKind {
    VAK_Valid,
    VAK_ValidInCXX11,
    VAK_Undefined,
    VAK_MSVCUndefined,
    VAK_Invalid
  };

  // Determines which VarArgKind fits an expression.
  VarArgKind isValidVarArgType(const QualType &Ty);

  /// Check to see if the given expression is a valid argument to a variadic
  /// function, issuing a diagnostic if not.
  void checkVariadicArgument(const Expr *E, VariadicCallType CT);

  /// Check to see if a given expression could have '.c_str()' called on it.
  bool hasCStrMethod(const Expr *E);

  /// GatherArgumentsForCall - Collector argument expressions for various
  /// form of call prototypes.
  bool GatherArgumentsForCall(SourceLocation CallLoc, FunctionDecl *FDecl,
                              const FunctionProtoType *Proto,
                              unsigned FirstParam, ArrayRef<Expr *> Args,
                              SmallVectorImpl<Expr *> &AllArgs,
                              VariadicCallType CallType = VariadicDoesNotApply,
                              bool AllowExplicit = false,
                              bool IsListInitialization = false);

  // DefaultVariadicArgumentPromotion - Like DefaultArgumentPromotion, but
  // will create a runtime trap if the resulting type is not a POD type.
  ExprResult DefaultVariadicArgumentPromotion(Expr *E, VariadicCallType CT,
                                              FunctionDecl *FDecl);

  // UsualArithmeticConversions - performs the UsualUnaryConversions on it's
  // operands and then handles various conversions that are common to binary
  // operators (C99 6.3.1.8). If both operands aren't arithmetic, this
  // routine returns the first non-arithmetic type found. The client is
  // responsible for emitting appropriate error diagnostics.
  QualType UsualArithmeticConversions(ExprResult &LHS, ExprResult &RHS,
                                      bool IsCompAssign = false);

  /// AssignConvertType - All of the 'assignment' semantic checks return this
  /// enum to indicate whether the assignment was allowed.  These checks are
  /// done for simple assignments, as well as initialization, return from
  /// function, argument passing, etc.  The query is phrased in terms of a
  /// source and destination type.
  enum AssignConvertType {
    /// Compatible - the types are compatible according to the standard.
    Compatible,

    /// PointerToInt - The assignment converts a pointer to an int, which we
    /// accept as an extension.
    PointerToInt,

    /// IntToPointer - The assignment converts an int to a pointer, which we
    /// accept as an extension.
    IntToPointer,

    /// FunctionVoidPointer - The assignment is between a function pointer and
    /// void*, which the standard doesn't allow, but we accept as an extension.
    FunctionVoidPointer,

    /// IncompatiblePointer - The assignment is between two pointers types that
    /// are not compatible, but we accept them as an extension.
    IncompatiblePointer,

    /// IncompatiblePointer - The assignment is between two pointers types which
    /// point to integers which have a different sign, but are otherwise
    /// identical. This is a subset of the above, but broken out because it's by
    /// far the most common case of incompatible pointers.
    IncompatiblePointerSign,

    /// CompatiblePointerDiscardsQualifiers - The assignment discards
    /// c/v/r qualifiers, which we accept as an extension.
    CompatiblePointerDiscardsQualifiers,

    /// IncompatiblePointerDiscardsQualifiers - The assignment
    /// discards qualifiers that we don't permit to be discarded,
    /// like address spaces.
    IncompatiblePointerDiscardsQualifiers,

    /// IncompatibleNestedPointerQualifiers - The assignment is between two
    /// nested pointer types, and the qualifiers other than the first two
    /// levels differ e.g. char ** -> const char **, but we accept them as an
    /// extension.
    IncompatibleNestedPointerQualifiers,

    /// IncompatibleVectors - The assignment is between two vector types that
    /// have the same size, which we accept as an extension.
    IncompatibleVectors,

    /// IntToBlockPointer - The assignment converts an int to a block
    /// pointer. We disallow this.
    IntToBlockPointer,

    /// IncompatibleBlockPointer - The assignment is between two block
    /// pointers types that are not compatible.
    IncompatibleBlockPointer,

    /// IncompatibleObjCQualifiedId - The assignment is between a qualified
    /// id type and something else (that is incompatible with it). For example,
    /// "id <XXX>" = "Foo *", where "Foo *" doesn't implement the XXX protocol.
    IncompatibleObjCQualifiedId,

    /// IncompatibleObjCWeakRef - Assigning a weak-unavailable object to an
    /// object with __weak qualifier.
    IncompatibleObjCWeakRef,

    /// Incompatible - We reject this conversion outright, it is invalid to
    /// represent it in the AST.
    Incompatible
  };

  /// DiagnoseAssignmentResult - Emit a diagnostic, if required, for the
  /// assignment conversion type specified by ConvTy.  This returns true if the
  /// conversion was invalid or false if the conversion was accepted.
  bool DiagnoseAssignmentResult(AssignConvertType ConvTy,
                                SourceLocation Loc,
                                QualType DstType, QualType SrcType,
                                Expr *SrcExpr, AssignmentAction Action,
                                bool *Complained = nullptr);

  /// IsValueInFlagEnum - Determine if a value is allowed as part of a flag
  /// enum. If AllowMask is true, then we also allow the complement of a valid
  /// value, to be used as a mask.
  bool IsValueInFlagEnum(const EnumDecl *ED, const llvm::APInt &Val,
                         bool AllowMask) const;

  /// DiagnoseAssignmentEnum - Warn if assignment to enum is a constant
  /// integer not in the range of enum values.
  void DiagnoseAssignmentEnum(QualType DstType, QualType SrcType,
                              Expr *SrcExpr);

  /// CheckAssignmentConstraints - Perform type checking for assignment,
  /// argument passing, variable initialization, and function return values.
  /// C99 6.5.16.
  AssignConvertType CheckAssignmentConstraints(SourceLocation Loc,
                                               QualType LHSType,
                                               QualType RHSType);

  /// Check assignment constraints and prepare for a conversion of the
  /// RHS to the LHS type.
  AssignConvertType CheckAssignmentConstraints(QualType LHSType,
                                               ExprResult &RHS,
                                               CastKind &Kind);

  // CheckSingleAssignmentConstraints - Currently used by
  // CheckAssignmentOperands, and ActOnReturnStmt. Prior to type checking,
  // this routine performs the default function/array converions.
  AssignConvertType CheckSingleAssignmentConstraints(QualType LHSType,
                                                     ExprResult &RHS,
                                                     bool Diagnose = true,
                                                     bool DiagnoseCFAudited = false);

  // \brief If the lhs type is a transparent union, check whether we
  // can initialize the transparent union with the given expression.
  AssignConvertType CheckTransparentUnionArgumentConstraints(QualType ArgType,
                                                             ExprResult &RHS);

  bool IsStringLiteralToNonConstPointerConversion(Expr *From, QualType ToType);

  bool CheckExceptionSpecCompatibility(Expr *From, QualType ToType);

  ExprResult PerformImplicitConversion(Expr *From, QualType ToType,
                                       AssignmentAction Action,
                                       bool AllowExplicit = false);
  ExprResult PerformImplicitConversion(Expr *From, QualType ToType,
                                       AssignmentAction Action,
                                       bool AllowExplicit,
                                       ImplicitConversionSequence& ICS);
  ExprResult PerformImplicitConversion(Expr *From, QualType ToType,
                                       const ImplicitConversionSequence& ICS,
                                       AssignmentAction Action,
                                       CheckedConversionKind CCK
                                          = CCK_ImplicitConversion);
  ExprResult PerformImplicitConversion(Expr *From, QualType ToType,
                                       const StandardConversionSequence& SCS,
                                       AssignmentAction Action,
                                       CheckedConversionKind CCK);

  /// the following "Check" methods will return a valid/converted QualType
  /// or a null QualType (indicating an error diagnostic was issued).

  /// type checking binary operators (subroutines of CreateBuiltinBinOp).
  QualType InvalidOperands(SourceLocation Loc, ExprResult &LHS,
                           ExprResult &RHS);
  QualType CheckPointerToMemberOperands( // C++ 5.5
    ExprResult &LHS, ExprResult &RHS, ExprValueKind &VK,
    SourceLocation OpLoc, bool isIndirect);
  QualType CheckMultiplyDivideOperands( // C99 6.5.5
    ExprResult &LHS, ExprResult &RHS, SourceLocation Loc, bool IsCompAssign,
    bool IsDivide);
  QualType CheckRemainderOperands( // C99 6.5.5
    ExprResult &LHS, ExprResult &RHS, SourceLocation Loc,
    bool IsCompAssign = false);
  QualType CheckAdditionOperands( // C99 6.5.6
    ExprResult &LHS, ExprResult &RHS, SourceLocation Loc, unsigned Opc,
    QualType* CompLHSTy = nullptr);
  QualType CheckSubtractionOperands( // C99 6.5.6
    ExprResult &LHS, ExprResult &RHS, SourceLocation Loc,
    QualType* CompLHSTy = nullptr);
  QualType CheckShiftOperands( // C99 6.5.7
    ExprResult &LHS, ExprResult &RHS, SourceLocation Loc, unsigned Opc,
    bool IsCompAssign = false);
  QualType CheckCompareOperands( // C99 6.5.8/9
    ExprResult &LHS, ExprResult &RHS, SourceLocation Loc, unsigned OpaqueOpc,
                                bool isRelational);
  QualType CheckBitwiseOperands( // C99 6.5.[10...12]
    ExprResult &LHS, ExprResult &RHS, SourceLocation Loc,
    bool IsCompAssign = false);
  QualType CheckLogicalOperands( // C99 6.5.[13,14]
    ExprResult &LHS, ExprResult &RHS, SourceLocation Loc, unsigned Opc);
  // CheckAssignmentOperands is used for both simple and compound assignment.
  // For simple assignment, pass both expressions and a null converted type.
  // For compound assignment, pass both expressions and the converted type.
  QualType CheckAssignmentOperands( // C99 6.5.16.[1,2]
    Expr *LHSExpr, ExprResult &RHS, SourceLocation Loc, QualType CompoundType);

  ExprResult checkPseudoObjectIncDec(Scope *S, SourceLocation OpLoc,
                                     UnaryOperatorKind Opcode, Expr *Op);
  ExprResult checkPseudoObjectAssignment(Scope *S, SourceLocation OpLoc,
                                         BinaryOperatorKind Opcode,
                                         Expr *LHS, Expr *RHS);
  ExprResult checkPseudoObjectRValue(Expr *E);
  Expr *recreateSyntacticForm(PseudoObjectExpr *E);

  QualType CheckConditionalOperands( // C99 6.5.15
    ExprResult &Cond, ExprResult &LHS, ExprResult &RHS,
    ExprValueKind &VK, ExprObjectKind &OK, SourceLocation QuestionLoc);
  QualType CXXCheckConditionalOperands( // C++ 5.16
    ExprResult &cond, ExprResult &lhs, ExprResult &rhs,
    ExprValueKind &VK, ExprObjectKind &OK, SourceLocation questionLoc);
  QualType FindCompositePointerType(SourceLocation Loc, Expr *&E1, Expr *&E2,
                                    bool *NonStandardCompositeType = nullptr);
  QualType FindCompositePointerType(SourceLocation Loc,
                                    ExprResult &E1, ExprResult &E2,
                                    bool *NonStandardCompositeType = nullptr) {
    Expr *E1Tmp = E1.get(), *E2Tmp = E2.get();
    QualType Composite = FindCompositePointerType(Loc, E1Tmp, E2Tmp,
                                                  NonStandardCompositeType);
    E1 = E1Tmp;
    E2 = E2Tmp;
    return Composite;
  }

  QualType FindCompositeObjCPointerType(ExprResult &LHS, ExprResult &RHS,
                                        SourceLocation QuestionLoc);

  bool DiagnoseConditionalForNull(Expr *LHSExpr, Expr *RHSExpr,
                                  SourceLocation QuestionLoc);

  void DiagnoseAlwaysNonNullPointer(Expr *E,
                                    Expr::NullPointerConstantKind NullType,
                                    bool IsEqual, SourceRange Range);

  /// type checking for vector binary operators.
  QualType CheckVectorOperands(ExprResult &LHS, ExprResult &RHS,
                               SourceLocation Loc, bool IsCompAssign,
                               bool AllowBothBool, bool AllowBoolConversion);
  QualType GetSignedVectorType(QualType V);
  QualType CheckVectorCompareOperands(ExprResult &LHS, ExprResult &RHS,
                                      SourceLocation Loc, bool isRelational);
  QualType CheckVectorLogicalOperands(ExprResult &LHS, ExprResult &RHS,
                                      SourceLocation Loc);

  bool isLaxVectorConversion(QualType srcType, QualType destType);

  /// type checking declaration initializers (C99 6.7.8)
  bool CheckForConstantInitializer(Expr *e, QualType t);

  // type checking C++ declaration initializers (C++ [dcl.init]).

  /// ReferenceCompareResult - Expresses the result of comparing two
  /// types (cv1 T1 and cv2 T2) to determine their compatibility for the
  /// purposes of initialization by reference (C++ [dcl.init.ref]p4).
  enum ReferenceCompareResult {
    /// Ref_Incompatible - The two types are incompatible, so direct
    /// reference binding is not possible.
    Ref_Incompatible = 0,
    /// Ref_Related - The two types are reference-related, which means
    /// that their unqualified forms (T1 and T2) are either the same
    /// or T1 is a base class of T2.
    Ref_Related,
    /// Ref_Compatible_With_Added_Qualification - The two types are
    /// reference-compatible with added qualification, meaning that
    /// they are reference-compatible and the qualifiers on T1 (cv1)
    /// are greater than the qualifiers on T2 (cv2).
    Ref_Compatible_With_Added_Qualification,
    /// Ref_Compatible - The two types are reference-compatible and
    /// have equivalent qualifiers (cv1 == cv2).
    Ref_Compatible
  };

  ReferenceCompareResult CompareReferenceRelationship(SourceLocation Loc,
                                                      QualType T1, QualType T2,
                                                      bool &DerivedToBase,
                                                      bool &ObjCConversion,
                                                bool &ObjCLifetimeConversion);

  ExprResult checkUnknownAnyCast(SourceRange TypeRange, QualType CastType,
                                 Expr *CastExpr, CastKind &CastKind,
                                 ExprValueKind &VK, CXXCastPath &Path);

  /// \brief Force an expression with unknown-type to an expression of the
  /// given type.
  ExprResult forceUnknownAnyToType(Expr *E, QualType ToType);

  /// \brief Type-check an expression that's being passed to an
  /// __unknown_anytype parameter.
  ExprResult checkUnknownAnyArg(SourceLocation callLoc,
                                Expr *result, QualType &paramType);

  // CheckVectorCast - check type constraints for vectors.
  // Since vectors are an extension, there are no C standard reference for this.
  // We allow casting between vectors and integer datatypes of the same size.
  // returns true if the cast is invalid
  bool CheckVectorCast(SourceRange R, QualType VectorTy, QualType Ty,
                       CastKind &Kind);

  // CheckExtVectorCast - check type constraints for extended vectors.
  // Since vectors are an extension, there are no C standard reference for this.
  // We allow casting between vectors and integer datatypes of the same size,
  // or vectors and the element type of that vector.
  // returns the cast expr
  ExprResult CheckExtVectorCast(SourceRange R, QualType DestTy, Expr *CastExpr,
                                CastKind &Kind);

  ExprResult BuildCXXFunctionalCastExpr(TypeSourceInfo *TInfo,
                                        SourceLocation LParenLoc,
                                        Expr *CastExpr,
                                        SourceLocation RParenLoc);

  enum ARCConversionResult { ACR_okay, ACR_unbridged };

  /// \brief Checks for invalid conversions and casts between
  /// retainable pointers and other pointer kinds.
  ARCConversionResult CheckObjCARCConversion(SourceRange castRange,
                                             QualType castType, Expr *&op,
                                             CheckedConversionKind CCK,
                                             bool DiagnoseCFAudited = false,
                                             BinaryOperatorKind Opc = BO_PtrMemD
                                             );

  Expr *stripARCUnbridgedCast(Expr *e);
  void diagnoseARCUnbridgedCast(Expr *e);

  bool CheckObjCARCUnavailableWeakConversion(QualType castType,
                                             QualType ExprType);

  /// checkRetainCycles - Check whether an Objective-C message send
  /// might create an obvious retain cycle.
  void checkRetainCycles(ObjCMessageExpr *msg);
  void checkRetainCycles(Expr *receiver, Expr *argument);
  void checkRetainCycles(VarDecl *Var, Expr *Init);

  /// checkUnsafeAssigns - Check whether +1 expr is being assigned
  /// to weak/__unsafe_unretained type.
  bool checkUnsafeAssigns(SourceLocation Loc, QualType LHS, Expr *RHS);

  /// checkUnsafeExprAssigns - Check whether +1 expr is being assigned
  /// to weak/__unsafe_unretained expression.
  void checkUnsafeExprAssigns(SourceLocation Loc, Expr *LHS, Expr *RHS);

  /// CheckMessageArgumentTypes - Check types in an Obj-C message send.
  /// \param Method - May be null.
  /// \param [out] ReturnType - The return type of the send.
  /// \return true iff there were any incompatible types.
  bool CheckMessageArgumentTypes(QualType ReceiverType,
                                 MultiExprArg Args, Selector Sel,
                                 ArrayRef<SourceLocation> SelectorLocs,
                                 ObjCMethodDecl *Method, bool isClassMessage,
                                 bool isSuperMessage,
                                 SourceLocation lbrac, SourceLocation rbrac,
                                 SourceRange RecRange,
                                 QualType &ReturnType, ExprValueKind &VK);

  /// \brief Determine the result of a message send expression based on
  /// the type of the receiver, the method expected to receive the message,
  /// and the form of the message send.
  QualType getMessageSendResultType(QualType ReceiverType,
                                    ObjCMethodDecl *Method,
                                    bool isClassMessage, bool isSuperMessage);

  /// \brief If the given expression involves a message send to a method
  /// with a related result type, emit a note describing what happened.
  void EmitRelatedResultTypeNote(const Expr *E);

  /// \brief Given that we had incompatible pointer types in a return
  /// statement, check whether we're in a method with a related result
  /// type, and if so, emit a note describing what happened.
  void EmitRelatedResultTypeNoteForReturn(QualType destType);

  /// CheckBooleanCondition - Diagnose problems involving the use of
  /// the given expression as a boolean condition (e.g. in an if
  /// statement).  Also performs the standard function and array
  /// decays, possibly changing the input variable.
  ///
  /// \param Loc - A location associated with the condition, e.g. the
  /// 'if' keyword.
  /// \return true iff there were any errors
  ExprResult CheckBooleanCondition(Expr *E, SourceLocation Loc);

  ExprResult ActOnBooleanCondition(Scope *S, SourceLocation Loc,
                                   Expr *SubExpr);

  /// DiagnoseAssignmentAsCondition - Given that an expression is
  /// being used as a boolean condition, warn if it's an assignment.
  void DiagnoseAssignmentAsCondition(Expr *E);

  /// \brief Redundant parentheses over an equality comparison can indicate
  /// that the user intended an assignment used as condition.
  void DiagnoseEqualityWithExtraParens(ParenExpr *ParenE);

  /// CheckCXXBooleanCondition - Returns true if conversion to bool is invalid.
  ExprResult CheckCXXBooleanCondition(Expr *CondExpr);

  /// ConvertIntegerToTypeWarnOnOverflow - Convert the specified APInt to have
  /// the specified width and sign.  If an overflow occurs, detect it and emit
  /// the specified diagnostic.
  void ConvertIntegerToTypeWarnOnOverflow(llvm::APSInt &OldVal,
                                          unsigned NewWidth, bool NewSign,
                                          SourceLocation Loc, unsigned DiagID);

  /// Checks that the Objective-C declaration is declared in the global scope.
  /// Emits an error and marks the declaration as invalid if it's not declared
  /// in the global scope.
  bool CheckObjCDeclScope(Decl *D);

  /// \brief Abstract base class used for diagnosing integer constant
  /// expression violations.
  class VerifyICEDiagnoser {
  public:
    bool Suppress;

    VerifyICEDiagnoser(bool Suppress = false) : Suppress(Suppress) { }

    virtual void diagnoseNotICE(Sema &S, SourceLocation Loc, SourceRange SR) =0;
    virtual void diagnoseFold(Sema &S, SourceLocation Loc, SourceRange SR);
    virtual ~VerifyICEDiagnoser() { }
  };

  /// VerifyIntegerConstantExpression - Verifies that an expression is an ICE,
  /// and reports the appropriate diagnostics. Returns false on success.
  /// Can optionally return the value of the expression.
  ExprResult VerifyIntegerConstantExpression(Expr *E, llvm::APSInt *Result,
                                             VerifyICEDiagnoser &Diagnoser,
                                             bool AllowFold = true);
  ExprResult VerifyIntegerConstantExpression(Expr *E, llvm::APSInt *Result,
                                             unsigned DiagID,
                                             bool AllowFold = true);
  ExprResult VerifyIntegerConstantExpression(Expr *E,
                                             llvm::APSInt *Result = nullptr);

  /// VerifyBitField - verifies that a bit field expression is an ICE and has
  /// the correct width, and that the field type is valid.
  /// Returns false on success.
  /// Can optionally return whether the bit-field is of width 0
  ExprResult VerifyBitField(SourceLocation FieldLoc, IdentifierInfo *FieldName,
                            QualType FieldTy, bool IsMsStruct,
                            Expr *BitWidth, bool *ZeroWidth = nullptr);

  enum CUDAFunctionTarget {
    CFT_Device,
    CFT_Global,
    CFT_Host,
    CFT_HostDevice,
    CFT_InvalidTarget
  };

  CUDAFunctionTarget IdentifyCUDATarget(const FunctionDecl *D);

  bool CheckCUDATarget(const FunctionDecl *Caller, const FunctionDecl *Callee);

  /// Given a implicit special member, infer its CUDA target from the
  /// calls it needs to make to underlying base/field special members.
  /// \param ClassDecl the class for which the member is being created.
  /// \param CSM the kind of special member.
  /// \param MemberDecl the special member itself.
  /// \param ConstRHS true if this is a copy operation with a const object on
  ///        its RHS.
  /// \param Diagnose true if this call should emit diagnostics.
  /// \return true if there was an error inferring.
  /// The result of this call is implicit CUDA target attribute(s) attached to
  /// the member declaration.
  bool inferCUDATargetForImplicitSpecialMember(CXXRecordDecl *ClassDecl,
                                               CXXSpecialMember CSM,
                                               CXXMethodDecl *MemberDecl,
                                               bool ConstRHS,
                                               bool Diagnose);

  /// \name Code completion
  //@{
  /// \brief Describes the context in which code completion occurs.
  enum ParserCompletionContext {
    /// \brief Code completion occurs at top-level or namespace context.
    PCC_Namespace,
    /// \brief Code completion occurs within a class, struct, or union.
    PCC_Class,
    /// \brief Code completion occurs within an Objective-C interface, protocol,
    /// or category.
    PCC_ObjCInterface,
    /// \brief Code completion occurs within an Objective-C implementation or
    /// category implementation
    PCC_ObjCImplementation,
    /// \brief Code completion occurs within the list of instance variables
    /// in an Objective-C interface, protocol, category, or implementation.
    PCC_ObjCInstanceVariableList,
    /// \brief Code completion occurs following one or more template
    /// headers.
    PCC_Template,
    /// \brief Code completion occurs following one or more template
    /// headers within a class.
    PCC_MemberTemplate,
    /// \brief Code completion occurs within an expression.
    PCC_Expression,
    /// \brief Code completion occurs within a statement, which may
    /// also be an expression or a declaration.
    PCC_Statement,
    /// \brief Code completion occurs at the beginning of the
    /// initialization statement (or expression) in a for loop.
    PCC_ForInit,
    /// \brief Code completion occurs within the condition of an if,
    /// while, switch, or for statement.
    PCC_Condition,
    /// \brief Code completion occurs within the body of a function on a
    /// recovery path, where we do not have a specific handle on our position
    /// in the grammar.
    PCC_RecoveryInFunction,
    /// \brief Code completion occurs where only a type is permitted.
    PCC_Type,
    /// \brief Code completion occurs in a parenthesized expression, which
    /// might also be a type cast.
    PCC_ParenthesizedExpression,
    /// \brief Code completion occurs within a sequence of declaration
    /// specifiers within a function, method, or block.
    PCC_LocalDeclarationSpecifiers
  };

  void CodeCompleteModuleImport(SourceLocation ImportLoc, ModuleIdPath Path);
  void CodeCompleteOrdinaryName(Scope *S,
                                ParserCompletionContext CompletionContext);
  void CodeCompleteDeclSpec(Scope *S, DeclSpec &DS,
                            bool AllowNonIdentifiers,
                            bool AllowNestedNameSpecifiers);

  struct CodeCompleteExpressionData;
  void CodeCompleteExpression(Scope *S,
                              const CodeCompleteExpressionData &Data);
  void CodeCompleteMemberReferenceExpr(Scope *S, Expr *Base,
                                       SourceLocation OpLoc,
                                       bool IsArrow);
  void CodeCompletePostfixExpression(Scope *S, ExprResult LHS);
  void CodeCompleteTag(Scope *S, unsigned TagSpec);
  void CodeCompleteTypeQualifiers(DeclSpec &DS);
  void CodeCompleteCase(Scope *S);
  void CodeCompleteCall(Scope *S, Expr *Fn, ArrayRef<Expr *> Args);
  void CodeCompleteConstructor(Scope *S, QualType Type, SourceLocation Loc,
                               ArrayRef<Expr *> Args);
  void CodeCompleteInitializer(Scope *S, Decl *D);
  void CodeCompleteReturn(Scope *S);
  void CodeCompleteAfterIf(Scope *S);
  void CodeCompleteAssignmentRHS(Scope *S, Expr *LHS);

  void CodeCompleteQualifiedId(Scope *S, CXXScopeSpec &SS,
                               bool EnteringContext);
  void CodeCompleteUsing(Scope *S);
  void CodeCompleteUsingDirective(Scope *S);
  void CodeCompleteNamespaceDecl(Scope *S);
  void CodeCompleteNamespaceAliasDecl(Scope *S);
  void CodeCompleteOperatorName(Scope *S);
  void CodeCompleteConstructorInitializer(
                                Decl *Constructor,
                                ArrayRef<CXXCtorInitializer *> Initializers);

  void CodeCompleteLambdaIntroducer(Scope *S, LambdaIntroducer &Intro,
                                    bool AfterAmpersand);

  void CodeCompleteObjCAtDirective(Scope *S);
  void CodeCompleteObjCAtVisibility(Scope *S);
  void CodeCompleteObjCAtStatement(Scope *S);
  void CodeCompleteObjCAtExpression(Scope *S);
  void CodeCompleteObjCPropertyFlags(Scope *S, ObjCDeclSpec &ODS);
  void CodeCompleteObjCPropertyGetter(Scope *S);
  void CodeCompleteObjCPropertySetter(Scope *S);
  void CodeCompleteObjCPassingType(Scope *S, ObjCDeclSpec &DS,
                                   bool IsParameter);
  void CodeCompleteObjCMessageReceiver(Scope *S);
  void CodeCompleteObjCSuperMessage(Scope *S, SourceLocation SuperLoc,
                                    ArrayRef<IdentifierInfo *> SelIdents,
                                    bool AtArgumentExpression);
  void CodeCompleteObjCClassMessage(Scope *S, ParsedType Receiver,
                                    ArrayRef<IdentifierInfo *> SelIdents,
                                    bool AtArgumentExpression,
                                    bool IsSuper = false);
  void CodeCompleteObjCInstanceMessage(Scope *S, Expr *Receiver,
                                       ArrayRef<IdentifierInfo *> SelIdents,
                                       bool AtArgumentExpression,
                                       ObjCInterfaceDecl *Super = nullptr);
  void CodeCompleteObjCForCollection(Scope *S,
                                     DeclGroupPtrTy IterationVar);
  void CodeCompleteObjCSelector(Scope *S,
                                ArrayRef<IdentifierInfo *> SelIdents);
  void CodeCompleteObjCProtocolReferences(IdentifierLocPair *Protocols,
                                          unsigned NumProtocols);
  void CodeCompleteObjCProtocolDecl(Scope *S);
  void CodeCompleteObjCInterfaceDecl(Scope *S);
  void CodeCompleteObjCSuperclass(Scope *S,
                                  IdentifierInfo *ClassName,
                                  SourceLocation ClassNameLoc);
  void CodeCompleteObjCImplementationDecl(Scope *S);
  void CodeCompleteObjCInterfaceCategory(Scope *S,
                                         IdentifierInfo *ClassName,
                                         SourceLocation ClassNameLoc);
  void CodeCompleteObjCImplementationCategory(Scope *S,
                                              IdentifierInfo *ClassName,
                                              SourceLocation ClassNameLoc);
  void CodeCompleteObjCPropertyDefinition(Scope *S);
  void CodeCompleteObjCPropertySynthesizeIvar(Scope *S,
                                              IdentifierInfo *PropertyName);
  void CodeCompleteObjCMethodDecl(Scope *S,
                                  bool IsInstanceMethod,
                                  ParsedType ReturnType);
  void CodeCompleteObjCMethodDeclSelector(Scope *S,
                                          bool IsInstanceMethod,
                                          bool AtParameterName,
                                          ParsedType ReturnType,
                                          ArrayRef<IdentifierInfo *> SelIdents);
  void CodeCompletePreprocessorDirective(bool InConditional);
  void CodeCompleteInPreprocessorConditionalExclusion(Scope *S);
  void CodeCompletePreprocessorMacroName(bool IsDefinition);
  void CodeCompletePreprocessorExpression();
  void CodeCompletePreprocessorMacroArgument(Scope *S,
                                             IdentifierInfo *Macro,
                                             MacroInfo *MacroInfo,
                                             unsigned Argument);
  void CodeCompleteNaturalLanguage();
  void GatherGlobalCodeCompletions(CodeCompletionAllocator &Allocator,
                                   CodeCompletionTUInfo &CCTUInfo,
                  SmallVectorImpl<CodeCompletionResult> &Results);
  //@}

  //===--------------------------------------------------------------------===//
  // Extra semantic analysis beyond the C type system

public:
  SourceLocation getLocationOfStringLiteralByte(const StringLiteral *SL,
                                                unsigned ByteNo) const;

private:
  void CheckArrayAccess(const Expr *BaseExpr, const Expr *IndexExpr,
                        const ArraySubscriptExpr *ASE=nullptr,
                        bool AllowOnePastEnd=true, bool IndexNegated=false);
  // HLSL Change Starts - checking array subscript access to vector or matrix member
  void CheckHLSLArrayAccess(const Expr *expr);
  bool CheckHLSLIntrinsicCall(FunctionDecl *FDecl, CallExpr *TheCall);
  bool CheckHLSLFunctionCall(FunctionDecl *FDecl, CallExpr *TheCall);
  // HLSL Change ends
  void CheckArrayAccess(const Expr *E);
  // Used to grab the relevant information from a FormatAttr and a
  // FunctionDeclaration.
  struct FormatStringInfo {
    unsigned FormatIdx;
    unsigned FirstDataArg;
    bool HasVAListArg;
  };

  bool getFormatStringInfo(const FormatAttr *Format, bool IsCXXMember,
                           FormatStringInfo *FSI);
  bool CheckFunctionCall(FunctionDecl *FDecl, CallExpr *TheCall,
                         const FunctionProtoType *Proto);
  bool CheckObjCMethodCall(ObjCMethodDecl *Method, SourceLocation loc,
                           ArrayRef<const Expr *> Args);
  bool CheckPointerCall(NamedDecl *NDecl, CallExpr *TheCall,
                        const FunctionProtoType *Proto);
  bool CheckOtherCall(CallExpr *TheCall, const FunctionProtoType *Proto);
  void CheckConstructorCall(FunctionDecl *FDecl,
                            ArrayRef<const Expr *> Args,
                            const FunctionProtoType *Proto,
                            SourceLocation Loc);

  void checkCall(NamedDecl *FDecl, const FunctionProtoType *Proto,
                 ArrayRef<const Expr *> Args, bool IsMemberFunction, 
                 SourceLocation Loc, SourceRange Range, 
                 VariadicCallType CallType);

  bool CheckObjCString(Expr *Arg);

  ExprResult CheckBuiltinFunctionCall(FunctionDecl *FDecl,
		                      unsigned BuiltinID, CallExpr *TheCall);

  bool CheckARMBuiltinExclusiveCall(unsigned BuiltinID, CallExpr *TheCall,
                                    unsigned MaxWidth);
  bool CheckNeonBuiltinFunctionCall(unsigned BuiltinID, CallExpr *TheCall);
  bool CheckARMBuiltinFunctionCall(unsigned BuiltinID, CallExpr *TheCall);

  bool CheckAArch64BuiltinFunctionCall(unsigned BuiltinID, CallExpr *TheCall);
  bool CheckMipsBuiltinFunctionCall(unsigned BuiltinID, CallExpr *TheCall);
  bool CheckSystemZBuiltinFunctionCall(unsigned BuiltinID, CallExpr *TheCall);
  bool CheckX86BuiltinFunctionCall(unsigned BuiltinID, CallExpr *TheCall);
  bool CheckPPCBuiltinFunctionCall(unsigned BuiltinID, CallExpr *TheCall);
  
  bool SemaBuiltinVAStart(CallExpr *TheCall);
  bool SemaBuiltinVAStartARM(CallExpr *Call);
  bool SemaBuiltinUnorderedCompare(CallExpr *TheCall);
  bool SemaBuiltinFPClassification(CallExpr *TheCall, unsigned NumArgs);

public:
  // Used by C++ template instantiation.
  ExprResult SemaBuiltinShuffleVector(CallExpr *TheCall);
  ExprResult SemaConvertVectorExpr(Expr *E, TypeSourceInfo *TInfo,
                                   SourceLocation BuiltinLoc,
                                   SourceLocation RParenLoc);

private:
  bool SemaBuiltinPrefetch(CallExpr *TheCall);
  bool SemaBuiltinAssume(CallExpr *TheCall);
  bool SemaBuiltinAssumeAligned(CallExpr *TheCall);
  bool SemaBuiltinLongjmp(CallExpr *TheCall);
  bool SemaBuiltinSetjmp(CallExpr *TheCall);
  ExprResult SemaBuiltinAtomicOverloaded(ExprResult TheCallResult);
  ExprResult SemaAtomicOpsOverloaded(ExprResult TheCallResult,
                                     AtomicExpr::AtomicOp Op);
  bool SemaBuiltinConstantArg(CallExpr *TheCall, int ArgNum,
                              llvm::APSInt &Result);
  bool SemaBuiltinConstantArgRange(CallExpr *TheCall, int ArgNum,
                                   int Low, int High);
  bool SemaBuiltinARMSpecialReg(unsigned BuiltinID, CallExpr *TheCall,
                                int ArgNum, unsigned ExpectedFieldNum,
                                bool AllowName);
  bool SemaBuiltinCpuSupports(CallExpr *TheCall);
public:
  enum FormatStringType {
    FST_Scanf,
    FST_Printf,
    FST_NSString,
    FST_Strftime,
    FST_Strfmon,
    FST_Kprintf,
    FST_FreeBSDKPrintf,
    FST_OSTrace,
    FST_Unknown
  };
  static FormatStringType GetFormatStringType(const FormatAttr *Format);

  void CheckFormatString(const StringLiteral *FExpr, const Expr *OrigFormatExpr,
                         ArrayRef<const Expr *> Args, bool HasVAListArg,
                         unsigned format_idx, unsigned firstDataArg,
                         FormatStringType Type, bool inFunctionCall,
                         VariadicCallType CallType,
                         llvm::SmallBitVector &CheckedVarArgs);
  
  bool FormatStringHasSArg(const StringLiteral *FExpr);
  
  bool GetFormatNSStringIdx(const FormatAttr *Format, unsigned &Idx);

private:
  bool CheckFormatArguments(const FormatAttr *Format,
                            ArrayRef<const Expr *> Args,
                            bool IsCXXMember,
                            VariadicCallType CallType,
                            SourceLocation Loc, SourceRange Range,
                            llvm::SmallBitVector &CheckedVarArgs);
  bool CheckFormatArguments(ArrayRef<const Expr *> Args,
                            bool HasVAListArg, unsigned format_idx,
                            unsigned firstDataArg, FormatStringType Type,
                            VariadicCallType CallType,
                            SourceLocation Loc, SourceRange range,
                            llvm::SmallBitVector &CheckedVarArgs);

  void CheckAbsoluteValueFunction(const CallExpr *Call,
                                  const FunctionDecl *FDecl,
                                  IdentifierInfo *FnInfo);

  void CheckMemaccessArguments(const CallExpr *Call,
                               unsigned BId,
                               IdentifierInfo *FnName);

  void CheckStrlcpycatArguments(const CallExpr *Call,
                                IdentifierInfo *FnName);

  void CheckStrncatArguments(const CallExpr *Call,
                             IdentifierInfo *FnName);

  void CheckReturnValExpr(Expr *RetValExp, QualType lhsType,
                          SourceLocation ReturnLoc,
                          bool isObjCMethod = false,
                          const AttrVec *Attrs = nullptr,
                          const FunctionDecl *FD = nullptr);

  void CheckFloatComparison(SourceLocation Loc, Expr* LHS, Expr* RHS);
  void CheckImplicitConversions(Expr *E, SourceLocation CC = SourceLocation());
  void CheckBoolLikeConversion(Expr *E, SourceLocation CC);
  void CheckForIntOverflow(Expr *E);
  void CheckUnsequencedOperations(Expr *E);

  /// \brief Perform semantic checks on a completed expression. This will either
  /// be a full-expression or a default argument expression.
  void CheckCompletedExpr(Expr *E, SourceLocation CheckLoc = SourceLocation(),
                          bool IsConstexpr = false);

  void CheckBitFieldInitialization(SourceLocation InitLoc, FieldDecl *Field,
                                   Expr *Init);

  /// \brief Check if the given expression contains 'break' or 'continue'
  /// statement that produces control flow different from GCC.
  void CheckBreakContinueBinding(Expr *E);

  /// \brief Check whether receiver is mutable ObjC container which
  /// attempts to add itself into the container
  void CheckObjCCircularContainer(ObjCMessageExpr *Message);

  void AnalyzeDeleteExprMismatch(const CXXDeleteExpr *DE);
  void AnalyzeDeleteExprMismatch(FieldDecl *Field, SourceLocation DeleteLoc,
                                 bool DeleteWasArrayForm);
public:
  /// \brief Register a magic integral constant to be used as a type tag.
  void RegisterTypeTagForDatatype(const IdentifierInfo *ArgumentKind,
                                  uint64_t MagicValue, QualType Type,
                                  bool LayoutCompatible, bool MustBeNull);

  struct TypeTagData {
    TypeTagData() {}

    TypeTagData(QualType Type, bool LayoutCompatible, bool MustBeNull) :
        Type(Type), LayoutCompatible(LayoutCompatible),
        MustBeNull(MustBeNull)
    {}

    QualType Type;

    /// If true, \c Type should be compared with other expression's types for
    /// layout-compatibility.
    unsigned LayoutCompatible : 1;
    unsigned MustBeNull : 1;
  };

  /// A pair of ArgumentKind identifier and magic value.  This uniquely
  /// identifies the magic value.
  typedef std::pair<const IdentifierInfo *, uint64_t> TypeTagMagicValue;

private:
  /// \brief A map from magic value to type information.
  std::unique_ptr<llvm::DenseMap<TypeTagMagicValue, TypeTagData>>
      TypeTagForDatatypeMagicValues;

  /// \brief Peform checks on a call of a function with argument_with_type_tag
  /// or pointer_with_type_tag attributes.
  void CheckArgumentWithTypeTag(const ArgumentWithTypeTagAttr *Attr,
                                const Expr * const *ExprArgs);

  /// \brief The parser's current scope.
  ///
  /// The parser maintains this state here.
  Scope *CurScope;

  mutable IdentifierInfo *Ident_super;
  mutable IdentifierInfo *Ident___float128;

  // HLSL Change Starts
  bool DiagnoseHLSLDecl(Declarator& D, DeclContext* DC, Expr *BitWidth, TypeSourceInfo* TInfo, bool isParameter);
  void TransferUnusualAttributes(Declarator& D, NamedDecl* NewDecl);
  // HLSL Change Ends

  /// Nullability type specifiers.
  IdentifierInfo *Ident__Nonnull = nullptr;
  IdentifierInfo *Ident__Nullable = nullptr;
  IdentifierInfo *Ident__Null_unspecified = nullptr;

  IdentifierInfo *Ident_NSError = nullptr;

protected:
  friend class Parser;
  friend class InitializationSequence;
  friend class ASTReader;
  friend class ASTDeclReader;
  friend class ASTWriter;

public:
  /// Retrieve the keyword associated
  IdentifierInfo *getNullabilityKeyword(NullabilityKind nullability);

  /// The struct behind the CFErrorRef pointer.
  RecordDecl *CFError = nullptr;

  /// Retrieve the identifier "NSError".
  IdentifierInfo *getNSErrorIdent();

  /// \brief Retrieve the parser's current scope.
  ///
  /// This routine must only be used when it is certain that semantic analysis
  /// and the parser are in precisely the same context, which is not the case
  /// when, e.g., we are performing any kind of template instantiation.
  /// Therefore, the only safe places to use this scope are in the parser
  /// itself and in routines directly invoked from the parser and *never* from
  /// template substitution or instantiation.
  Scope *getCurScope() const { return CurScope; }

  void incrementMSManglingNumber() const {
    return CurScope->incrementMSManglingNumber();
  }

  IdentifierInfo *getSuperIdentifier() const;
  IdentifierInfo *getFloat128Identifier() const;

  Decl *getObjCDeclContext() const;

  DeclContext *getCurLexicalContext() const {
    return OriginalLexicalContext ? OriginalLexicalContext : CurContext;
  }

  AvailabilityResult getCurContextAvailability() const;
  
  const DeclContext *getCurObjCLexicalContext() const {
    const DeclContext *DC = getCurLexicalContext();
    // A category implicitly has the attribute of the interface.
    if (const ObjCCategoryDecl *CatD = dyn_cast<ObjCCategoryDecl>(DC))
      DC = CatD->getClassInterface();
    return DC;
  }

  /// \brief To be used for checking whether the arguments being passed to
  /// function exceeds the number of parameters expected for it.
  static bool TooManyArguments(size_t NumParams, size_t NumArgs,
                               bool PartialOverloading = false) {
    // We check whether we're just after a comma in code-completion.
    if (NumArgs > 0 && PartialOverloading)
      return NumArgs + 1 > NumParams; // If so, we view as an extra argument.
    return NumArgs > NumParams;
  }

  // HLSL Change Begin - adjust this from T* to T&-like
  CXXThisExpr *genereateHLSLThis(SourceLocation Loc, QualType ThisType,
                                bool isImplicit);
  QualType getHLSLDefaultSpecialization(TemplateDecl *Decl);
  // HLSL Change End - adjust this from T* to T&-like

  void DiagnoseSemanticDecl(hlsl::SemanticDecl *Decl); // HLSL Change
};

/// \brief RAII object that enters a new expression evaluation context.
class EnterExpressionEvaluationContext {
  Sema &Actions;

public:
  EnterExpressionEvaluationContext(Sema &Actions,
                                   Sema::ExpressionEvaluationContext NewContext,
                                   Decl *LambdaContextDecl = nullptr,
                                   bool IsDecltype = false)
    : Actions(Actions) {
    Actions.PushExpressionEvaluationContext(NewContext, LambdaContextDecl,
                                            IsDecltype);
  }
  EnterExpressionEvaluationContext(Sema &Actions,
                                   Sema::ExpressionEvaluationContext NewContext,
                                   Sema::ReuseLambdaContextDecl_t,
                                   bool IsDecltype = false)
    : Actions(Actions) {
    Actions.PushExpressionEvaluationContext(NewContext, 
                                            Sema::ReuseLambdaContextDecl,
                                            IsDecltype);
  }

  ~EnterExpressionEvaluationContext() {
    Actions.PopExpressionEvaluationContext();
  }
};

DeductionFailureInfo
MakeDeductionFailureInfo(ASTContext &Context, Sema::TemplateDeductionResult TDK,
                         sema::TemplateDeductionInfo &Info);

/// \brief Contains a late templated function.
/// Will be parsed at the end of the translation unit, used by Sema & Parser.
struct LateParsedTemplate {
  CachedTokens Toks;
  /// \brief The template function declaration to be late parsed.
  Decl *D;
};

} // end namespace clang

#endif
