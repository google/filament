//===--- ExternalSemaSource.h - External Sema Interface ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the ExternalSemaSource interface.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_CLANG_SEMA_EXTERNALSEMASOURCE_H
#define LLVM_CLANG_SEMA_EXTERNALSEMASOURCE_H

#include "clang/AST/ExternalASTSource.h"
#include "clang/AST/Type.h"
#include "clang/Sema/TypoCorrection.h"
#include "clang/Sema/Weak.h"
#include "llvm/ADT/MapVector.h"
#include <utility>

namespace llvm {
template <class T, unsigned n> class SmallSetVector;
}

namespace clang {

class CXXConstructorDecl;
class CXXDeleteExpr;
class CXXRecordDecl;
class DeclaratorDecl;
class LookupResult;
struct ObjCMethodList;
class Scope;
class Sema;
class TypedefNameDecl;
class ValueDecl;
class VarDecl;
struct LateParsedTemplate;
class OverloadCandidateSet; // HLSL Change
class UnresolvedLookupExpr; // HLSL Change

/// \brief A simple structure that captures a vtable use for the purposes of
/// the \c ExternalSemaSource.
struct ExternalVTableUse {
  CXXRecordDecl *Record;
  SourceLocation Location;
  bool DefinitionRequired;
};
  
/// \brief An abstract interface that should be implemented by
/// external AST sources that also provide information for semantic
/// analysis.
class ExternalSemaSource : public ExternalASTSource {
public:
  ExternalSemaSource() {
    ExternalASTSource::SemaSource = true;
  }

  ~ExternalSemaSource() override;

  /// \brief Initialize the semantic source with the Sema instance
  /// being used to perform semantic analysis on the abstract syntax
  /// tree.
  virtual void InitializeSema(Sema &S) {}

  /// \brief Inform the semantic consumer that Sema is no longer available.
  virtual void ForgetSema() {}

  /// \brief Load the contents of the global method pool for a given
  /// selector.
  virtual void ReadMethodPool(Selector Sel);

  /// \brief Load the set of namespaces that are known to the external source,
  /// which will be used during typo correction.
  virtual void ReadKnownNamespaces(
                           SmallVectorImpl<NamespaceDecl *> &Namespaces);

  /// \brief Load the set of used but not defined functions or variables with
  /// internal linkage, or used but not defined internal functions.
  virtual void ReadUndefinedButUsed(
                         llvm::DenseMap<NamedDecl*, SourceLocation> &Undefined);

  virtual void ReadMismatchingDeleteExpressions(llvm::MapVector<
      FieldDecl *, llvm::SmallVector<std::pair<SourceLocation, bool>, 4>> &);

  /// \brief Do last resort, unqualified lookup on a LookupResult that
  /// Sema cannot find.
  ///
  /// \param R a LookupResult that is being recovered.
  ///
  /// \param S the Scope of the identifier occurrence.
  ///
  /// \return true to tell Sema to recover using the LookupResult.
  virtual bool LookupUnqualified(LookupResult &R, Scope *S) { return false; }

  /// \brief Read the set of tentative definitions known to the external Sema
  /// source.
  ///
  /// The external source should append its own tentative definitions to the
  /// given vector of tentative definitions. Note that this routine may be
  /// invoked multiple times; the external source should take care not to
  /// introduce the same declarations repeatedly.
  virtual void ReadTentativeDefinitions(
                                  SmallVectorImpl<VarDecl *> &TentativeDefs) {}
  
  /// \brief Read the set of unused file-scope declarations known to the
  /// external Sema source.
  ///
  /// The external source should append its own unused, filed-scope to the
  /// given vector of declarations. Note that this routine may be
  /// invoked multiple times; the external source should take care not to
  /// introduce the same declarations repeatedly.
  virtual void ReadUnusedFileScopedDecls(
                 SmallVectorImpl<const DeclaratorDecl *> &Decls) {}
  
  /// \brief Read the set of delegating constructors known to the
  /// external Sema source.
  ///
  /// The external source should append its own delegating constructors to the
  /// given vector of declarations. Note that this routine may be
  /// invoked multiple times; the external source should take care not to
  /// introduce the same declarations repeatedly.
  virtual void ReadDelegatingConstructors(
                 SmallVectorImpl<CXXConstructorDecl *> &Decls) {}

  /// \brief Read the set of ext_vector type declarations known to the
  /// external Sema source.
  ///
  /// The external source should append its own ext_vector type declarations to
  /// the given vector of declarations. Note that this routine may be
  /// invoked multiple times; the external source should take care not to
  /// introduce the same declarations repeatedly.
  virtual void ReadExtVectorDecls(SmallVectorImpl<TypedefNameDecl *> &Decls) {}

  /// \brief Read the set of potentially unused typedefs known to the source.
  ///
  /// The external source should append its own potentially unused local
  /// typedefs to the given vector of declarations. Note that this routine may
  /// be invoked multiple times; the external source should take care not to
  /// introduce the same declarations repeatedly.
  virtual void ReadUnusedLocalTypedefNameCandidates(
      llvm::SmallSetVector<const TypedefNameDecl *, 4> &Decls) {};

  /// \brief Read the set of referenced selectors known to the
  /// external Sema source.
  ///
  /// The external source should append its own referenced selectors to the 
  /// given vector of selectors. Note that this routine 
  /// may be invoked multiple times; the external source should take care not 
  /// to introduce the same selectors repeatedly.
  virtual void ReadReferencedSelectors(
                 SmallVectorImpl<std::pair<Selector, SourceLocation> > &Sels) {}

  /// \brief Read the set of weak, undeclared identifiers known to the
  /// external Sema source.
  ///
  /// The external source should append its own weak, undeclared identifiers to
  /// the given vector. Note that this routine may be invoked multiple times; 
  /// the external source should take care not to introduce the same identifiers
  /// repeatedly.
  virtual void ReadWeakUndeclaredIdentifiers(
                 SmallVectorImpl<std::pair<IdentifierInfo *, WeakInfo> > &WI) {}

  /// \brief Read the set of used vtables known to the external Sema source.
  ///
  /// The external source should append its own used vtables to the given
  /// vector. Note that this routine may be invoked multiple times; the external
  /// source should take care not to introduce the same vtables repeatedly.
  virtual void ReadUsedVTables(SmallVectorImpl<ExternalVTableUse> &VTables) {}

  /// \brief Read the set of pending instantiations known to the external
  /// Sema source.
  ///
  /// The external source should append its own pending instantiations to the
  /// given vector. Note that this routine may be invoked multiple times; the
  /// external source should take care not to introduce the same instantiations
  /// repeatedly.
  virtual void ReadPendingInstantiations(
                 SmallVectorImpl<std::pair<ValueDecl *, 
                                           SourceLocation> > &Pending) {}

  /// \brief Read the set of late parsed template functions for this source.
  ///
  /// The external source should insert its own late parsed template functions
  /// into the map. Note that this routine may be invoked multiple times; the
  /// external source should take care not to introduce the same map entries
  /// repeatedly.
  virtual void ReadLateParsedTemplates(
      llvm::MapVector<const FunctionDecl *, LateParsedTemplate *> &LPTMap) {}

  /// \copydoc Sema::CorrectTypo
  /// \note LookupKind must correspond to a valid Sema::LookupNameKind
  ///
  /// ExternalSemaSource::CorrectTypo is always given the first chance to
  /// correct a typo (really, to offer suggestions to repair a failed lookup).
  /// It will even be called when SpellChecking is turned off or after a
  /// fatal error has already been detected.
  virtual TypoCorrection CorrectTypo(const DeclarationNameInfo &Typo,
                                     int LookupKind, Scope *S, CXXScopeSpec *SS,
                                     CorrectionCandidateCallback &CCC,
                                     DeclContext *MemberContext,
                                     bool EnteringContext,
                                     const ObjCObjectPointerType *OPT) {
    return TypoCorrection();
  }

  // HLSL Change Starts
  // ExternalSemaSource::AddOverloadedCallCandidates is given the chance to
  // add call candidates to the given expression. It returns 'true'
  // if standard overload search should be suppressed; false otherwise.
  virtual bool AddOverloadedCallCandidates(UnresolvedLookupExpr *ULE,
    ArrayRef<Expr *> Args,
    OverloadCandidateSet &CandidateSet,
    bool PartialOverloading)
  {
    return false;
  }

  // HLSL Change Ends


  /// \brief Produces a diagnostic note if the external source contains a
  /// complete definition for \p T.
  ///
  /// \param Loc the location at which a complete type was required but not
  /// provided
  ///
  /// \param T the \c QualType that should have been complete at \p Loc
  ///
  /// \return true if a diagnostic was produced, false otherwise.
  virtual bool MaybeDiagnoseMissingCompleteType(SourceLocation Loc,
                                                QualType T) {
    return false;
  }

  // isa/cast/dyn_cast support
  static bool classof(const ExternalASTSource *Source) {
    return Source->SemaSource;
  }
}; 

} // end namespace clang

#endif
