//===--- SemaType.cpp - Semantic Analysis for Types -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements type-related semantic analysis.
//
//===----------------------------------------------------------------------===//

#include "clang/Sema/SemaInternal.h"
#include "TypeLocBuilder.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTMutationListener.h"
#include "clang/AST/CXXInheritance.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Expr.h"
#include "clang/AST/TypeLoc.h"
#include "clang/AST/TypeLocVisitor.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Basic/PartialDiagnostic.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseDiagnostic.h"
#include "clang/Sema/DeclSpec.h"
#include "clang/Sema/DelayedDiagnostic.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/Scope.h"       // HLSL Change
#include "dxc/Support/Global.h"     // HLSL Change
#include "clang/Sema/SemaHLSL.h"    // HLSL Change
#include "clang/Sema/ScopeInfo.h"
#include "clang/Sema/Template.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/ErrorHandling.h"

using namespace clang;

enum TypeDiagSelector {
  TDS_Function,
  TDS_Pointer,
  TDS_ObjCObjOrBlock
};

/// isOmittedBlockReturnType - Return true if this declarator is missing a
/// return type because this is a omitted return type on a block literal.
static bool isOmittedBlockReturnType(const Declarator &D) {
  if (D.getContext() != Declarator::BlockLiteralContext ||
      D.getDeclSpec().hasTypeSpecifier())
    return false;

  if (D.getNumTypeObjects() == 0)
    return true;   // ^{ ... }

  if (D.getNumTypeObjects() == 1 &&
      D.getTypeObject(0).Kind == DeclaratorChunk::Function)
    return true;   // ^(int X, float Y) { ... }

  return false;
}

/// diagnoseBadTypeAttribute - Diagnoses a type attribute which
/// doesn't apply to the given type.
static void diagnoseBadTypeAttribute(Sema &S, const AttributeList &attr,
                                     QualType type) {
  TypeDiagSelector WhichType;
  bool useExpansionLoc = true;
  switch (attr.getKind()) {
  case AttributeList::AT_ObjCGC:        WhichType = TDS_Pointer; break;
  case AttributeList::AT_ObjCOwnership: WhichType = TDS_ObjCObjOrBlock; break;
  default:
    // Assume everything else was a function attribute.
    WhichType = TDS_Function;
    useExpansionLoc = false;
    break;
  }

  SourceLocation loc = attr.getLoc();
  StringRef name = attr.getName()->getName();

  // The GC attributes are usually written with macros;  special-case them.
  IdentifierInfo *II = attr.isArgIdent(0) ? attr.getArgAsIdent(0)->Ident
                                          : nullptr;
  if (useExpansionLoc && loc.isMacroID() && II) {
    if (II->isStr("strong")) {
      if (S.findMacroSpelling(loc, "__strong")) name = "__strong";
    } else if (II->isStr("weak")) {
      if (S.findMacroSpelling(loc, "__weak")) name = "__weak";
    }
  }

  S.Diag(loc, diag::warn_type_attribute_wrong_type) << name << WhichType
    << type;
}

// objc_gc applies to Objective-C pointers or, otherwise, to the
// smallest available pointer type (i.e. 'void*' in 'void**').
#define OBJC_POINTER_TYPE_ATTRS_CASELIST \
    case AttributeList::AT_ObjCGC: \
    case AttributeList::AT_ObjCOwnership

// Function type attributes.
#define FUNCTION_TYPE_ATTRS_CASELIST \
    case AttributeList::AT_NoReturn: \
    case AttributeList::AT_CDecl: \
    case AttributeList::AT_FastCall: \
    case AttributeList::AT_StdCall: \
    case AttributeList::AT_ThisCall: \
    case AttributeList::AT_Pascal: \
    case AttributeList::AT_VectorCall: \
    case AttributeList::AT_MSABI: \
    case AttributeList::AT_SysVABI: \
    case AttributeList::AT_Regparm: \
    case AttributeList::AT_Pcs: \
    case AttributeList::AT_IntelOclBicc

// Microsoft-specific type qualifiers.
#define MS_TYPE_ATTRS_CASELIST  \
    case AttributeList::AT_Ptr32: \
    case AttributeList::AT_Ptr64: \
    case AttributeList::AT_SPtr: \
    case AttributeList::AT_UPtr

// Nullability qualifiers.
#define NULLABILITY_TYPE_ATTRS_CASELIST         \
    case AttributeList::AT_TypeNonNull:         \
    case AttributeList::AT_TypeNullable:        \
    case AttributeList::AT_TypeNullUnspecified

namespace {
  /// An object which stores processing state for the entire
  /// GetTypeForDeclarator process.
  class TypeProcessingState {
    Sema &sema;

    /// The declarator being processed.
    Declarator &declarator;

    /// The index of the declarator chunk we're currently processing.
    /// May be the total number of valid chunks, indicating the
    /// DeclSpec.
    unsigned chunkIndex;

    /// Whether there are non-trivial modifications to the decl spec.
    bool trivial;

    /// Whether we saved the attributes in the decl spec.
    bool hasSavedAttrs;

    // HLSL Change: add Scope

    /// The scope for the declarator
    Scope* scope;

    /// The original set of attributes on the DeclSpec.
    SmallVector<AttributeList*, 2> savedAttrs;

    /// A list of attributes to diagnose the uselessness of when the
    /// processing is complete.
    SmallVector<AttributeList*, 2> ignoredTypeAttrs;

  public:
    TypeProcessingState(Sema &sema, Declarator &declarator, Scope* S = nullptr) // HLSL Change: add Scope
      : sema(sema), declarator(declarator),
        chunkIndex(declarator.getNumTypeObjects()),
        trivial(true), hasSavedAttrs(false),
        scope(S) {} // HLSL Change: add Scope

    // HLSL Change: add getScope
    Scope* getScope() const {
      return scope;
    }

    Sema &getSema() const {
      return sema;
    }

    Declarator &getDeclarator() const {
      return declarator;
    }

    bool isProcessingDeclSpec() const {
      return chunkIndex == declarator.getNumTypeObjects();
    }

    unsigned getCurrentChunkIndex() const {
      return chunkIndex;
    }

    void setCurrentChunkIndex(unsigned idx) {
      assert(idx <= declarator.getNumTypeObjects());
      chunkIndex = idx;
    }

    AttributeList *&getCurrentAttrListRef() const {
      if (isProcessingDeclSpec())
        return getMutableDeclSpec().getAttributes().getListRef();
      return declarator.getTypeObject(chunkIndex).getAttrListRef();
    }

    /// Save the current set of attributes on the DeclSpec.
    void saveDeclSpecAttrs() {
      // Don't try to save them multiple times.
      if (hasSavedAttrs) return;

      DeclSpec &spec = getMutableDeclSpec();
      for (AttributeList *attr = spec.getAttributes().getList(); attr;
             attr = attr->getNext())
        savedAttrs.push_back(attr);
      trivial &= savedAttrs.empty();
      hasSavedAttrs = true;
    }

    /// Record that we had nowhere to put the given type attribute.
    /// We will diagnose such attributes later.
    void addIgnoredTypeAttr(AttributeList &attr) {
      ignoredTypeAttrs.push_back(&attr);
    }

    /// Diagnose all the ignored type attributes, given that the
    /// declarator worked out to the given type.
    void diagnoseIgnoredTypeAttrs(QualType type) const {
      for (SmallVectorImpl<AttributeList*>::const_iterator
             i = ignoredTypeAttrs.begin(), e = ignoredTypeAttrs.end();
           i != e; ++i)
        diagnoseBadTypeAttribute(getSema(), **i, type);
    }

    ~TypeProcessingState() {
      if (trivial) return;

      restoreDeclSpecAttrs();
    }

  private:
    DeclSpec &getMutableDeclSpec() const {
      return const_cast<DeclSpec&>(declarator.getDeclSpec());
    }

    void restoreDeclSpecAttrs() {
      assert(hasSavedAttrs);

      if (savedAttrs.empty()) {
        getMutableDeclSpec().getAttributes().set(nullptr);
        return;
      }

      getMutableDeclSpec().getAttributes().set(savedAttrs[0]);
      for (unsigned i = 0, e = savedAttrs.size() - 1; i != e; ++i)
        savedAttrs[i]->setNext(savedAttrs[i+1]);
      savedAttrs.back()->setNext(nullptr);
    }
  };
}

static void spliceAttrIntoList(AttributeList &attr, AttributeList *&head) {
  attr.setNext(head);
  head = &attr;
}

static void spliceAttrOutOfList(AttributeList &attr, AttributeList *&head) {
  if (head == &attr) {
    head = attr.getNext();
    return;
  }

  AttributeList *cur = head;
  while (true) {
    assert(cur && cur->getNext() && "ran out of attrs?");
    if (cur->getNext() == &attr) {
      cur->setNext(attr.getNext());
      return;
    }
    cur = cur->getNext();
  }
}

static void moveAttrFromListToList(AttributeList &attr,
                                   AttributeList *&fromList,
                                   AttributeList *&toList) {
  spliceAttrOutOfList(attr, fromList);
  spliceAttrIntoList(attr, toList);
}

/// The location of a type attribute.
enum TypeAttrLocation {
  /// The attribute is in the decl-specifier-seq.
  TAL_DeclSpec,
  /// The attribute is part of a DeclaratorChunk.
  TAL_DeclChunk,
  /// The attribute is immediately after the declaration's name.
  TAL_DeclName
};

static void processTypeAttrs(TypeProcessingState &state,
                             QualType &type, TypeAttrLocation TAL,
                             AttributeList *attrs);

static bool handleFunctionTypeAttr(TypeProcessingState &state,
                                   AttributeList &attr,
                                   QualType &type);

static bool handleMSPointerTypeQualifierAttr(TypeProcessingState &state,
                                             AttributeList &attr,
                                             QualType &type);

static bool handleObjCGCTypeAttr(TypeProcessingState &state,
                                 AttributeList &attr, QualType &type);

static bool handleObjCOwnershipTypeAttr(TypeProcessingState &state,
                                       AttributeList &attr, QualType &type);

static bool handleObjCPointerTypeAttr(TypeProcessingState &state,
                                      AttributeList &attr, QualType &type) {
  if (attr.getKind() == AttributeList::AT_ObjCGC)
    return handleObjCGCTypeAttr(state, attr, type);
  assert(attr.getKind() == AttributeList::AT_ObjCOwnership);
  return handleObjCOwnershipTypeAttr(state, attr, type);
}

/// Given the index of a declarator chunk, check whether that chunk
/// directly specifies the return type of a function and, if so, find
/// an appropriate place for it.
///
/// \param i - a notional index which the search will start
///   immediately inside
///
/// \param onlyBlockPointers Whether we should only look into block
/// pointer types (vs. all pointer types).
static DeclaratorChunk *maybeMovePastReturnType(Declarator &declarator,
                                                unsigned i,
                                                bool onlyBlockPointers) {
  assert(i <= declarator.getNumTypeObjects());

  DeclaratorChunk *result = nullptr;

  // First, look inwards past parens for a function declarator.
  for (; i != 0; --i) {
    DeclaratorChunk &fnChunk = declarator.getTypeObject(i-1);
    switch (fnChunk.Kind) {
    case DeclaratorChunk::Paren:
      continue;

    // If we find anything except a function, bail out.
    case DeclaratorChunk::Pointer:
    case DeclaratorChunk::BlockPointer:
    case DeclaratorChunk::Array:
    case DeclaratorChunk::Reference:
    case DeclaratorChunk::MemberPointer:
      return result;

    // If we do find a function declarator, scan inwards from that,
    // looking for a (block-)pointer declarator.
    case DeclaratorChunk::Function:
      for (--i; i != 0; --i) {
        DeclaratorChunk &ptrChunk = declarator.getTypeObject(i-1);
        switch (ptrChunk.Kind) {
        case DeclaratorChunk::Paren:
        case DeclaratorChunk::Array:
        case DeclaratorChunk::Function:
        case DeclaratorChunk::Reference:
          continue;

        case DeclaratorChunk::MemberPointer:
        case DeclaratorChunk::Pointer:
          if (onlyBlockPointers)
            continue;

          LLVM_FALLTHROUGH; // HLSL Change

        case DeclaratorChunk::BlockPointer:
          result = &ptrChunk;
          goto continue_outer;
        }
        llvm_unreachable("bad declarator chunk kind");
      }

      // If we run out of declarators doing that, we're done.
      return result;
    }
    llvm_unreachable("bad declarator chunk kind");

    // Okay, reconsider from our new point.
  continue_outer: ;
  }

  // Ran out of chunks, bail out.
  return result;
}

/// Given that an objc_gc attribute was written somewhere on a
/// declaration *other* than on the declarator itself (for which, use
/// distributeObjCPointerTypeAttrFromDeclarator), and given that it
/// didn't apply in whatever position it was written in, try to move
/// it to a more appropriate position.
static void distributeObjCPointerTypeAttr(TypeProcessingState &state,
                                          AttributeList &attr,
                                          QualType type) {
  Declarator &declarator = state.getDeclarator();

  // Move it to the outermost normal or block pointer declarator.
  for (unsigned i = state.getCurrentChunkIndex(); i != 0; --i) {
    DeclaratorChunk &chunk = declarator.getTypeObject(i-1);
    switch (chunk.Kind) {
    case DeclaratorChunk::Pointer:
    case DeclaratorChunk::BlockPointer: {
      // But don't move an ARC ownership attribute to the return type
      // of a block.
      DeclaratorChunk *destChunk = nullptr;
      if (state.isProcessingDeclSpec() &&
          attr.getKind() == AttributeList::AT_ObjCOwnership)
        destChunk = maybeMovePastReturnType(declarator, i - 1,
                                            /*onlyBlockPointers=*/true);
      if (!destChunk) destChunk = &chunk;

      moveAttrFromListToList(attr, state.getCurrentAttrListRef(),
                             destChunk->getAttrListRef());
      return;
    }

    case DeclaratorChunk::Paren:
    case DeclaratorChunk::Array:
      continue;

    // We may be starting at the return type of a block.
    case DeclaratorChunk::Function:
      if (state.isProcessingDeclSpec() &&
          attr.getKind() == AttributeList::AT_ObjCOwnership) {
        if (DeclaratorChunk *dest = maybeMovePastReturnType(
                                      declarator, i,
                                      /*onlyBlockPointers=*/true)) {
          moveAttrFromListToList(attr, state.getCurrentAttrListRef(),
                                 dest->getAttrListRef());
          return;
        }
      }
      goto error;

    // Don't walk through these.
    case DeclaratorChunk::Reference:
    case DeclaratorChunk::MemberPointer:
      goto error;
    }
  }
 error:

  diagnoseBadTypeAttribute(state.getSema(), attr, type);
}

/// Distribute an objc_gc type attribute that was written on the
/// declarator.
static void
distributeObjCPointerTypeAttrFromDeclarator(TypeProcessingState &state,
                                            AttributeList &attr,
                                            QualType &declSpecType) {
  Declarator &declarator = state.getDeclarator();

  // objc_gc goes on the innermost pointer to something that's not a
  // pointer.
  unsigned innermost = -1U;
  bool considerDeclSpec = true;
  for (unsigned i = 0, e = declarator.getNumTypeObjects(); i != e; ++i) {
    DeclaratorChunk &chunk = declarator.getTypeObject(i);
    switch (chunk.Kind) {
    case DeclaratorChunk::Pointer:
    case DeclaratorChunk::BlockPointer:
      innermost = i;
      continue;

    case DeclaratorChunk::Reference:
    case DeclaratorChunk::MemberPointer:
    case DeclaratorChunk::Paren:
    case DeclaratorChunk::Array:
      continue;

    case DeclaratorChunk::Function:
      considerDeclSpec = false;
      goto done;
    }
  }
 done:

  // That might actually be the decl spec if we weren't blocked by
  // anything in the declarator.
  if (considerDeclSpec) {
    if (handleObjCPointerTypeAttr(state, attr, declSpecType)) {
      // Splice the attribute into the decl spec.  Prevents the
      // attribute from being applied multiple times and gives
      // the source-location-filler something to work with.
      state.saveDeclSpecAttrs();
      moveAttrFromListToList(attr, declarator.getAttrListRef(),
               declarator.getMutableDeclSpec().getAttributes().getListRef());
      return;
    }
  }

  // Otherwise, if we found an appropriate chunk, splice the attribute
  // into it.
  if (innermost != -1U) {
    moveAttrFromListToList(attr, declarator.getAttrListRef(),
                       declarator.getTypeObject(innermost).getAttrListRef());
    return;
  }

  // Otherwise, diagnose when we're done building the type.
  spliceAttrOutOfList(attr, declarator.getAttrListRef());
  state.addIgnoredTypeAttr(attr);
}

/// A function type attribute was written somewhere in a declaration
/// *other* than on the declarator itself or in the decl spec.  Given
/// that it didn't apply in whatever position it was written in, try
/// to move it to a more appropriate position.
static void distributeFunctionTypeAttr(TypeProcessingState &state,
                                       AttributeList &attr,
                                       QualType type) {
  Declarator &declarator = state.getDeclarator();

  // Try to push the attribute from the return type of a function to
  // the function itself.
  for (unsigned i = state.getCurrentChunkIndex(); i != 0; --i) {
    DeclaratorChunk &chunk = declarator.getTypeObject(i-1);
    switch (chunk.Kind) {
    case DeclaratorChunk::Function:
      moveAttrFromListToList(attr, state.getCurrentAttrListRef(),
                             chunk.getAttrListRef());
      return;

    case DeclaratorChunk::Paren:
    case DeclaratorChunk::Pointer:
    case DeclaratorChunk::BlockPointer:
    case DeclaratorChunk::Array:
    case DeclaratorChunk::Reference:
    case DeclaratorChunk::MemberPointer:
      continue;
    }
  }

  diagnoseBadTypeAttribute(state.getSema(), attr, type);
}

/// Try to distribute a function type attribute to the innermost
/// function chunk or type.  Returns true if the attribute was
/// distributed, false if no location was found.
static bool
distributeFunctionTypeAttrToInnermost(TypeProcessingState &state,
                                      AttributeList &attr,
                                      AttributeList *&attrList,
                                      QualType &declSpecType) {
  Declarator &declarator = state.getDeclarator();

  // Put it on the innermost function chunk, if there is one.
  for (unsigned i = 0, e = declarator.getNumTypeObjects(); i != e; ++i) {
    DeclaratorChunk &chunk = declarator.getTypeObject(i);
    if (chunk.Kind != DeclaratorChunk::Function) continue;

    moveAttrFromListToList(attr, attrList, chunk.getAttrListRef());
    return true;
  }

  return handleFunctionTypeAttr(state, attr, declSpecType);
}

/// A function type attribute was written in the decl spec.  Try to
/// apply it somewhere.
static void
distributeFunctionTypeAttrFromDeclSpec(TypeProcessingState &state,
                                       AttributeList &attr,
                                       QualType &declSpecType) {
  state.saveDeclSpecAttrs();

  // C++11 attributes before the decl specifiers actually appertain to
  // the declarators. Move them straight there. We don't support the
  // 'put them wherever you like' semantics we allow for GNU attributes.
  if (attr.isCXX11Attribute()) {
    moveAttrFromListToList(attr, state.getCurrentAttrListRef(),
                           state.getDeclarator().getAttrListRef());
    return;
  }

  // Try to distribute to the innermost.
  if (distributeFunctionTypeAttrToInnermost(state, attr,
                                            state.getCurrentAttrListRef(),
                                            declSpecType))
    return;

  // If that failed, diagnose the bad attribute when the declarator is
  // fully built.
  state.addIgnoredTypeAttr(attr);
}

/// A function type attribute was written on the declarator.  Try to
/// apply it somewhere.
static void
distributeFunctionTypeAttrFromDeclarator(TypeProcessingState &state,
                                         AttributeList &attr,
                                         QualType &declSpecType) {
  Declarator &declarator = state.getDeclarator();

  // Try to distribute to the innermost.
  if (distributeFunctionTypeAttrToInnermost(state, attr,
                                            declarator.getAttrListRef(),
                                            declSpecType))
    return;

  // If that failed, diagnose the bad attribute when the declarator is
  // fully built.
  spliceAttrOutOfList(attr, declarator.getAttrListRef());
  state.addIgnoredTypeAttr(attr);
}

/// \brief Given that there are attributes written on the declarator
/// itself, try to distribute any type attributes to the appropriate
/// declarator chunk.
///
/// These are attributes like the following:
///   int f ATTR;
///   int (f ATTR)();
/// but not necessarily this:
///   int f() ATTR;
static void distributeTypeAttrsFromDeclarator(TypeProcessingState &state,
                                              QualType &declSpecType) {
  // Collect all the type attributes from the declarator itself.
  assert(state.getDeclarator().getAttributes() && "declarator has no attrs!");
  AttributeList *attr = state.getDeclarator().getAttributes();
  AttributeList *next;
  do {
    next = attr->getNext();

    // Do not distribute C++11 attributes. They have strict rules for what
    // they appertain to.
    if (attr->isCXX11Attribute())
      continue;

    switch (attr->getKind()) {
    OBJC_POINTER_TYPE_ATTRS_CASELIST:
      distributeObjCPointerTypeAttrFromDeclarator(state, *attr, declSpecType);
      break;

    case AttributeList::AT_NSReturnsRetained:
      if (!state.getSema().getLangOpts().ObjCAutoRefCount)
        break;
      distributeFunctionTypeAttrFromDeclarator(state, *attr, declSpecType); // HLSL Change
      break; // HLSL Change - avoid fallthrough
    FUNCTION_TYPE_ATTRS_CASELIST:
      distributeFunctionTypeAttrFromDeclarator(state, *attr, declSpecType);
      break;

    MS_TYPE_ATTRS_CASELIST:
      // Microsoft type attributes cannot go after the declarator-id.
      continue;

    NULLABILITY_TYPE_ATTRS_CASELIST:
      // Nullability specifiers cannot go after the declarator-id.

    // Objective-C __kindof does not get distributed.
    case AttributeList::AT_ObjCKindOf:
      continue;

    default:
      break;
    }
  } while ((attr = next));
}

/// Add a synthetic '()' to a block-literal declarator if it is
/// required, given the return type.
static void maybeSynthesizeBlockSignature(TypeProcessingState &state,
                                          QualType declSpecType) {
  Declarator &declarator = state.getDeclarator();

  // First, check whether the declarator would produce a function,
  // i.e. whether the innermost semantic chunk is a function.
  if (declarator.isFunctionDeclarator()) {
    // If so, make that declarator a prototyped declarator.
    declarator.getFunctionTypeInfo().hasPrototype = true;
    return;
  }

  // If there are any type objects, the type as written won't name a
  // function, regardless of the decl spec type.  This is because a
  // block signature declarator is always an abstract-declarator, and
  // abstract-declarators can't just be parentheses chunks.  Therefore
  // we need to build a function chunk unless there are no type
  // objects and the decl spec type is a function.
  if (!declarator.getNumTypeObjects() && declSpecType->isFunctionType())
    return;

  // Note that there *are* cases with invalid declarators where
  // declarators consist solely of parentheses.  In general, these
  // occur only in failed efforts to make function declarators, so
  // faking up the function chunk is still the right thing to do.

  // Otherwise, we need to fake up a function declarator.
  SourceLocation loc = declarator.getLocStart();

  // ...and *prepend* it to the declarator.
  SourceLocation NoLoc;
  declarator.AddInnermostTypeInfo(DeclaratorChunk::getFunction(
      /*HasProto=*/true,
      /*IsAmbiguous=*/false,
      /*LParenLoc=*/NoLoc,
      /*ArgInfo=*/nullptr,
      /*NumArgs=*/0,
      /*EllipsisLoc=*/NoLoc,
      /*RParenLoc=*/NoLoc,
      /*TypeQuals=*/0,
      /*RefQualifierIsLvalueRef=*/true,
      /*RefQualifierLoc=*/NoLoc,
      /*ConstQualifierLoc=*/NoLoc,
      /*VolatileQualifierLoc=*/NoLoc,
      /*RestrictQualifierLoc=*/NoLoc,
      /*MutableLoc=*/NoLoc, EST_None,
      /*ESpecLoc=*/NoLoc,
      /*Exceptions=*/nullptr,
      /*ExceptionRanges=*/nullptr,
      /*NumExceptions=*/0,
      /*NoexceptExpr=*/nullptr,
      /*ExceptionSpecTokens=*/nullptr,
      loc, loc, declarator));

  // For consistency, make sure the state still has us as processing
  // the decl spec.
  assert(state.getCurrentChunkIndex() == declarator.getNumTypeObjects() - 1);
  state.setCurrentChunkIndex(declarator.getNumTypeObjects());
}

static void diagnoseAndRemoveTypeQualifiers(Sema &S, const DeclSpec &DS,
                                            unsigned &TypeQuals,
                                            QualType TypeSoFar,
                                            unsigned RemoveTQs,
                                            unsigned DiagID) {
  // If this occurs outside a template instantiation, warn the user about
  // it; they probably didn't mean to specify a redundant qualifier.
  typedef std::pair<DeclSpec::TQ, SourceLocation> QualLoc;
  for (QualLoc Qual : {QualLoc(DeclSpec::TQ_const, DS.getConstSpecLoc()),
                       QualLoc(DeclSpec::TQ_volatile, DS.getVolatileSpecLoc()),
                       QualLoc(DeclSpec::TQ_atomic, DS.getAtomicSpecLoc())}) {
    if (!(RemoveTQs & Qual.first))
      continue;

    if (S.ActiveTemplateInstantiations.empty()) {
      if (TypeQuals & Qual.first)
        S.Diag(Qual.second, DiagID)
          << DeclSpec::getSpecifierName(Qual.first) << TypeSoFar
          << FixItHint::CreateRemoval(Qual.second);
    }

    TypeQuals &= ~Qual.first;
  }
}

/// Apply Objective-C type arguments to the given type.
static QualType applyObjCTypeArgs(Sema &S, SourceLocation loc, QualType type,
                                  ArrayRef<TypeSourceInfo *> typeArgs,
                                  SourceRange typeArgsRange,
                                  bool failOnError = false) {
  // We can only apply type arguments to an Objective-C class type.
  const auto *objcObjectType = type->getAs<ObjCObjectType>();
  if (!objcObjectType || !objcObjectType->getInterface()) {
    S.Diag(loc, diag::err_objc_type_args_non_class)
      << type
      << typeArgsRange;

    if (failOnError)
      return QualType();
    return type;
  }

  // The class type must be parameterized.
  ObjCInterfaceDecl *objcClass = objcObjectType->getInterface();
  ObjCTypeParamList *typeParams = objcClass->getTypeParamList();
  if (!typeParams) {
    S.Diag(loc, diag::err_objc_type_args_non_parameterized_class)
      << objcClass->getDeclName()
      << FixItHint::CreateRemoval(typeArgsRange);

    if (failOnError)
      return QualType();

    return type;
  }

  // The type must not already be specialized.
  if (objcObjectType->isSpecialized()) {
    S.Diag(loc, diag::err_objc_type_args_specialized_class)
      << type
      << FixItHint::CreateRemoval(typeArgsRange);

    if (failOnError)
      return QualType();

    return type;
  }

  // Check the type arguments.
  SmallVector<QualType, 4> finalTypeArgs;
  unsigned numTypeParams = typeParams->size();
  bool anyPackExpansions = false;
  for (unsigned i = 0, n = typeArgs.size(); i != n; ++i) {
    TypeSourceInfo *typeArgInfo = typeArgs[i];
    QualType typeArg = typeArgInfo->getType();

    // Type arguments cannot explicitly specify nullability.
    if (auto nullability = AttributedType::stripOuterNullability(typeArg)) {
      SourceLocation nullabilityLoc
        = typeArgInfo->getTypeLoc().findNullabilityLoc();
      SourceLocation diagLoc = nullabilityLoc.isValid()? nullabilityLoc
        : typeArgInfo->getTypeLoc().getLocStart();
      S.Diag(diagLoc,
             diag::err_type_arg_explicit_nullability)
        << typeArg
        << FixItHint::CreateRemoval(nullabilityLoc);
    }

    finalTypeArgs.push_back(typeArg);

    if (typeArg->getAs<PackExpansionType>())
      anyPackExpansions = true;

    // Find the corresponding type parameter, if there is one.
    ObjCTypeParamDecl *typeParam = nullptr;
    if (!anyPackExpansions) {
      if (i < numTypeParams) {
        typeParam = typeParams->begin()[i];
      } else {
        // Too many arguments.
        S.Diag(loc, diag::err_objc_type_args_wrong_arity)
          << false
          << objcClass->getDeclName()
          << (unsigned)typeArgs.size()
          << numTypeParams;
        S.Diag(objcClass->getLocation(), diag::note_previous_decl)
          << objcClass;

        if (failOnError)
          return QualType();

        return type;
      }
    }

    // Objective-C object pointer types must be substitutable for the bounds.
    if (const auto *typeArgObjC = typeArg->getAs<ObjCObjectPointerType>()) {
      // If we don't have a type parameter to match against, assume
      // everything is fine. There was a prior pack expansion that
      // means we won't be able to match anything.
      if (!typeParam) {
        assert(anyPackExpansions && "Too many arguments?");
        continue;
      }

      // Retrieve the bound.
      QualType bound = typeParam->getUnderlyingType();
      const auto *boundObjC = bound->getAs<ObjCObjectPointerType>();

      // Determine whether the type argument is substitutable for the bound.
      if (typeArgObjC->isObjCIdType()) {
        // When the type argument is 'id', the only acceptable type
        // parameter bound is 'id'.
        if (boundObjC->isObjCIdType())
          continue;
      } else if (S.Context.canAssignObjCInterfaces(boundObjC, typeArgObjC)) {
        // Otherwise, we follow the assignability rules.
        continue;
      }

      // Diagnose the mismatch.
      S.Diag(typeArgInfo->getTypeLoc().getLocStart(),
             diag::err_objc_type_arg_does_not_match_bound)
        << typeArg << bound << typeParam->getDeclName();
      S.Diag(typeParam->getLocation(), diag::note_objc_type_param_here)
        << typeParam->getDeclName();

      if (failOnError)
        return QualType();

      return type;
    }

    // Block pointer types are permitted for unqualified 'id' bounds.
    if (typeArg->isBlockPointerType()) {
      // If we don't have a type parameter to match against, assume
      // everything is fine. There was a prior pack expansion that
      // means we won't be able to match anything.
      if (!typeParam) {
        assert(anyPackExpansions && "Too many arguments?");
        continue;
      }

      // Retrieve the bound.
      QualType bound = typeParam->getUnderlyingType();
      if (bound->isBlockCompatibleObjCPointerType(S.Context))
        continue;

      // Diagnose the mismatch.
      S.Diag(typeArgInfo->getTypeLoc().getLocStart(),
             diag::err_objc_type_arg_does_not_match_bound)
        << typeArg << bound << typeParam->getDeclName();
      S.Diag(typeParam->getLocation(), diag::note_objc_type_param_here)
        << typeParam->getDeclName();

      if (failOnError)
        return QualType();

      return type;
    }

    // Dependent types will be checked at instantiation time.
    if (typeArg->isDependentType()) {
      continue;
    }

    // Diagnose non-id-compatible type arguments.
    S.Diag(typeArgInfo->getTypeLoc().getLocStart(),
           diag::err_objc_type_arg_not_id_compatible)
      << typeArg
      << typeArgInfo->getTypeLoc().getSourceRange();

    if (failOnError)
      return QualType();

    return type;
  }

  // Make sure we didn't have the wrong number of arguments.
  if (!anyPackExpansions && finalTypeArgs.size() != numTypeParams) {
    S.Diag(loc, diag::err_objc_type_args_wrong_arity)
      << (typeArgs.size() < typeParams->size())
      << objcClass->getDeclName()
      << (unsigned)finalTypeArgs.size()
      << (unsigned)numTypeParams;
    S.Diag(objcClass->getLocation(), diag::note_previous_decl)
      << objcClass;

    if (failOnError)
      return QualType();

    return type;
  }

  // Success. Form the specialized type.
  return S.Context.getObjCObjectType(type, finalTypeArgs, { }, false);
}

/// Apply Objective-C protocol qualifiers to the given type.
static QualType applyObjCProtocolQualifiers(
                  Sema &S, SourceLocation loc, SourceRange range, QualType type,
                  ArrayRef<ObjCProtocolDecl *> protocols,
                  const SourceLocation *protocolLocs,
                  bool failOnError = false) {
  ASTContext &ctx = S.Context;
  if (const ObjCObjectType *objT = dyn_cast<ObjCObjectType>(type.getTypePtr())){
    // FIXME: Check for protocols to which the class type is already
    // known to conform.

    return ctx.getObjCObjectType(objT->getBaseType(),
                                 objT->getTypeArgsAsWritten(),
                                 protocols,
                                 objT->isKindOfTypeAsWritten());
  }

  if (type->isObjCObjectType()) {
    // Silently overwrite any existing protocol qualifiers.
    // TODO: determine whether that's the right thing to do.

    // FIXME: Check for protocols to which the class type is already
    // known to conform.
    return ctx.getObjCObjectType(type, { }, protocols, false);
  }

  // id<protocol-list>
  if (type->isObjCIdType()) {
    const ObjCObjectPointerType *objPtr = type->castAs<ObjCObjectPointerType>();
    type = ctx.getObjCObjectType(ctx.ObjCBuiltinIdTy, { }, protocols,
                                 objPtr->isKindOfType());
    return ctx.getObjCObjectPointerType(type);
  }

  // Class<protocol-list>
  if (type->isObjCClassType()) {
    const ObjCObjectPointerType *objPtr = type->castAs<ObjCObjectPointerType>();
    type = ctx.getObjCObjectType(ctx.ObjCBuiltinClassTy, { }, protocols,
                                 objPtr->isKindOfType());
    return ctx.getObjCObjectPointerType(type);
  }

  S.Diag(loc, diag::err_invalid_protocol_qualifiers)
    << range;

  if (failOnError)
    return QualType();

  return type;
}

QualType Sema::BuildObjCObjectType(QualType BaseType,
                                   SourceLocation Loc,
                                   SourceLocation TypeArgsLAngleLoc,
                                   ArrayRef<TypeSourceInfo *> TypeArgs,
                                   SourceLocation TypeArgsRAngleLoc,
                                   SourceLocation ProtocolLAngleLoc,
                                   ArrayRef<ObjCProtocolDecl *> Protocols,
                                   ArrayRef<SourceLocation> ProtocolLocs,
                                   SourceLocation ProtocolRAngleLoc,
                                   bool FailOnError) {
  QualType Result = BaseType;
  if (!TypeArgs.empty()) {
    Result = applyObjCTypeArgs(*this, Loc, Result, TypeArgs,
                               SourceRange(TypeArgsLAngleLoc,
                                           TypeArgsRAngleLoc),
                               FailOnError);
    if (FailOnError && Result.isNull())
      return QualType();
  }

  if (!Protocols.empty()) {
    Result = applyObjCProtocolQualifiers(*this, Loc,
                                         SourceRange(ProtocolLAngleLoc,
                                                     ProtocolRAngleLoc),
                                         Result, Protocols,
                                         ProtocolLocs.data(),
                                         FailOnError);
    if (FailOnError && Result.isNull())
      return QualType();
  }

  return Result;
}

TypeResult Sema::actOnObjCProtocolQualifierType(
             SourceLocation lAngleLoc,
             ArrayRef<Decl *> protocols,
             ArrayRef<SourceLocation> protocolLocs,
             SourceLocation rAngleLoc) {
  // Form id<protocol-list>.
  QualType Result = Context.getObjCObjectType(
                      Context.ObjCBuiltinIdTy, { },
                      llvm::makeArrayRef(
                        (ObjCProtocolDecl * const *)protocols.data(),
                        protocols.size()),
                      false);
  Result = Context.getObjCObjectPointerType(Result);

  TypeSourceInfo *ResultTInfo = Context.CreateTypeSourceInfo(Result);
  TypeLoc ResultTL = ResultTInfo->getTypeLoc();

  auto ObjCObjectPointerTL = ResultTL.castAs<ObjCObjectPointerTypeLoc>();
  ObjCObjectPointerTL.setStarLoc(SourceLocation()); // implicit

  auto ObjCObjectTL = ObjCObjectPointerTL.getPointeeLoc()
                        .castAs<ObjCObjectTypeLoc>();
  ObjCObjectTL.setHasBaseTypeAsWritten(false);
  ObjCObjectTL.getBaseLoc().initialize(Context, SourceLocation());

  // No type arguments.
  ObjCObjectTL.setTypeArgsLAngleLoc(SourceLocation());
  ObjCObjectTL.setTypeArgsRAngleLoc(SourceLocation());

  // Fill in protocol qualifiers.
  ObjCObjectTL.setProtocolLAngleLoc(lAngleLoc);
  ObjCObjectTL.setProtocolRAngleLoc(rAngleLoc);
  for (unsigned i = 0, n = protocols.size(); i != n; ++i)
    ObjCObjectTL.setProtocolLoc(i, protocolLocs[i]);

  // We're done. Return the completed type to the parser.
  return CreateParsedType(Result, ResultTInfo);
}

TypeResult Sema::actOnObjCTypeArgsAndProtocolQualifiers(
             Scope *S,
             SourceLocation Loc,
             ParsedType BaseType,
             SourceLocation TypeArgsLAngleLoc,
             ArrayRef<ParsedType> TypeArgs,
             SourceLocation TypeArgsRAngleLoc,
             SourceLocation ProtocolLAngleLoc,
             ArrayRef<Decl *> Protocols,
             ArrayRef<SourceLocation> ProtocolLocs,
             SourceLocation ProtocolRAngleLoc) {
  TypeSourceInfo *BaseTypeInfo = nullptr;
  QualType T = GetTypeFromParser(BaseType, &BaseTypeInfo);
  if (T.isNull())
    return true;

  // Handle missing type-source info.
  if (!BaseTypeInfo)
    BaseTypeInfo = Context.getTrivialTypeSourceInfo(T, Loc);

  // Extract type arguments.
  SmallVector<TypeSourceInfo *, 4> ActualTypeArgInfos;
  for (unsigned i = 0, n = TypeArgs.size(); i != n; ++i) {
    TypeSourceInfo *TypeArgInfo = nullptr;
    QualType TypeArg = GetTypeFromParser(TypeArgs[i], &TypeArgInfo);
    if (TypeArg.isNull()) {
      ActualTypeArgInfos.clear();
      break;
    }
    
    assert(TypeArgInfo && "No type source info?");
    ActualTypeArgInfos.push_back(TypeArgInfo);
  }

  // Build the object type.
  QualType Result = BuildObjCObjectType(
      T, BaseTypeInfo->getTypeLoc().getSourceRange().getBegin(),
      TypeArgsLAngleLoc, ActualTypeArgInfos, TypeArgsRAngleLoc,
      ProtocolLAngleLoc,
      llvm::makeArrayRef((ObjCProtocolDecl * const *)Protocols.data(),
                         Protocols.size()),
      ProtocolLocs, ProtocolRAngleLoc,
      /*FailOnError=*/false);

  if (Result == T)
    return BaseType;
    
  // Create source information for this type.
  TypeSourceInfo *ResultTInfo = Context.CreateTypeSourceInfo(Result);
  TypeLoc ResultTL = ResultTInfo->getTypeLoc();

  // For id<Proto1, Proto2> or Class<Proto1, Proto2>, we'll have an
  // object pointer type. Fill in source information for it.
  if (auto ObjCObjectPointerTL = ResultTL.getAs<ObjCObjectPointerTypeLoc>()) {
    // The '*' is implicit.
    ObjCObjectPointerTL.setStarLoc(SourceLocation());
    ResultTL = ObjCObjectPointerTL.getPointeeLoc();
  }

  auto ObjCObjectTL = ResultTL.castAs<ObjCObjectTypeLoc>();

  // Type argument information.
  if (ObjCObjectTL.getNumTypeArgs() > 0) {
    assert(ObjCObjectTL.getNumTypeArgs() == ActualTypeArgInfos.size());
    ObjCObjectTL.setTypeArgsLAngleLoc(TypeArgsLAngleLoc);
    ObjCObjectTL.setTypeArgsRAngleLoc(TypeArgsRAngleLoc);
    for (unsigned i = 0, n = ActualTypeArgInfos.size(); i != n; ++i)
      ObjCObjectTL.setTypeArgTInfo(i, ActualTypeArgInfos[i]);
  } else {
    ObjCObjectTL.setTypeArgsLAngleLoc(SourceLocation());
    ObjCObjectTL.setTypeArgsRAngleLoc(SourceLocation());
  }

  // Protocol qualifier information.
  if (ObjCObjectTL.getNumProtocols() > 0) {
    assert(ObjCObjectTL.getNumProtocols() == Protocols.size());
    ObjCObjectTL.setProtocolLAngleLoc(ProtocolLAngleLoc);
    ObjCObjectTL.setProtocolRAngleLoc(ProtocolRAngleLoc);
    for (unsigned i = 0, n = Protocols.size(); i != n; ++i)
      ObjCObjectTL.setProtocolLoc(i, ProtocolLocs[i]);
  } else {
    ObjCObjectTL.setProtocolLAngleLoc(SourceLocation());
    ObjCObjectTL.setProtocolRAngleLoc(SourceLocation());
  }

  // Base type.
  ObjCObjectTL.setHasBaseTypeAsWritten(true);
  if (ObjCObjectTL.getType() == T)
    ObjCObjectTL.getBaseLoc().initializeFullCopy(BaseTypeInfo->getTypeLoc());
  else
    ObjCObjectTL.getBaseLoc().initialize(Context, Loc);

  // We're done. Return the completed type to the parser.
  return CreateParsedType(Result, ResultTInfo);
}

/// \brief Convert the specified declspec to the appropriate type
/// object.
/// \param state Specifies the declarator containing the declaration specifier
/// to be converted, along with other associated processing state.
/// \returns The type described by the declaration specifiers.  This function
/// never returns null.
static QualType ConvertDeclSpecToType(TypeProcessingState &state) {
  // FIXME: Should move the logic from DeclSpec::Finish to here for validity
  // checking.

  Sema &S = state.getSema();
  Declarator &declarator = state.getDeclarator();
  const DeclSpec &DS = declarator.getDeclSpec();
  SourceLocation DeclLoc = declarator.getIdentifierLoc();
  if (DeclLoc.isInvalid())
    DeclLoc = DS.getLocStart();

  ASTContext &Context = S.Context;

  QualType Result;
  switch (DS.getTypeSpecType()) {
  case DeclSpec::TST_void:
    Result = Context.VoidTy;
    break;
  case DeclSpec::TST_char:
    if (DS.getTypeSpecSign() == DeclSpec::TSS_unspecified)
      Result = Context.CharTy;
    else if (DS.getTypeSpecSign() == DeclSpec::TSS_signed)
      Result = Context.SignedCharTy;
    else {
      assert(DS.getTypeSpecSign() == DeclSpec::TSS_unsigned &&
             "Unknown TSS value");
      Result = Context.UnsignedCharTy;
    }
    break;
  case DeclSpec::TST_wchar:
    if (DS.getTypeSpecSign() == DeclSpec::TSS_unspecified)
      Result = Context.WCharTy;
    else if (DS.getTypeSpecSign() == DeclSpec::TSS_signed) {
      S.Diag(DS.getTypeSpecSignLoc(), diag::ext_invalid_sign_spec)
        << DS.getSpecifierName(DS.getTypeSpecType(),
                               Context.getPrintingPolicy());
      Result = Context.getSignedWCharType();
    } else {
      assert(DS.getTypeSpecSign() == DeclSpec::TSS_unsigned &&
        "Unknown TSS value");
      S.Diag(DS.getTypeSpecSignLoc(), diag::ext_invalid_sign_spec)
        << DS.getSpecifierName(DS.getTypeSpecType(),
                               Context.getPrintingPolicy());
      Result = Context.getUnsignedWCharType();
    }
    break;
  case DeclSpec::TST_char16:
      assert(DS.getTypeSpecSign() == DeclSpec::TSS_unspecified &&
        "Unknown TSS value");
      Result = Context.Char16Ty;
    break;
  case DeclSpec::TST_char32:
      assert(DS.getTypeSpecSign() == DeclSpec::TSS_unspecified &&
        "Unknown TSS value");
      Result = Context.Char32Ty;
    break;
  case DeclSpec::TST_unspecified:
    // If this is a missing declspec in a block literal return context, then it
    // is inferred from the return statements inside the block.
    // The declspec is always missing in a lambda expr context; it is either
    // specified with a trailing return type or inferred.
    if (S.getLangOpts().CPlusPlus14 &&
        declarator.getContext() == Declarator::LambdaExprContext) {
      // In C++1y, a lambda's implicit return type is 'auto'.
      Result = Context.getAutoDeductType();
      break;
    } else if (declarator.getContext() == Declarator::LambdaExprContext ||
               isOmittedBlockReturnType(declarator)) {
      Result = Context.DependentTy;
      break;
    }

    // Unspecified typespec defaults to int in C90.  However, the C90 grammar
    // [C90 6.5] only allows a decl-spec if there was *some* type-specifier,
    // type-qualifier, or storage-class-specifier.  If not, emit an extwarn.
    // Note that the one exception to this is function definitions, which are
    // allowed to be completely missing a declspec.  This is handled in the
    // parser already though by it pretending to have seen an 'int' in this
    // case.
    if (S.getLangOpts().ImplicitInt) {
      // In C89 mode, we only warn if there is a completely missing declspec
      // when one is not allowed.
      if (DS.isEmpty()) {
        S.Diag(DeclLoc, diag::ext_missing_declspec)
          << DS.getSourceRange()
        << FixItHint::CreateInsertion(DS.getLocStart(), "int");
      }
    } else if (!DS.hasTypeSpecifier()) {
      // C99 and C++ require a type specifier.  For example, C99 6.7.2p2 says:
      // "At least one type specifier shall be given in the declaration
      // specifiers in each declaration, and in the specifier-qualifier list in
      // each struct declaration and type name."
      if (S.getLangOpts().CPlusPlus) {
        S.Diag(DeclLoc, S.getLangOpts().HLSL ? diag::err_hlsl_missing_type_specifier : diag::err_missing_type_specifier)  // HLSL Change
          << DS.getSourceRange();

        // When this occurs in C++ code, often something is very broken with the
        // value being declared, poison it as invalid so we don't get chains of
        // errors.
        declarator.setInvalidType(true);
      } else {
        S.Diag(DeclLoc, diag::ext_missing_type_specifier)
          << DS.getSourceRange();
      }
    }

    LLVM_FALLTHROUGH; // HLSL CHANGE
  case DeclSpec::TST_int: {
    if (DS.getTypeSpecSign() != DeclSpec::TSS_unsigned) {
      switch (DS.getTypeSpecWidth()) {
      case DeclSpec::TSW_unspecified: Result = Context.IntTy; break;
      case DeclSpec::TSW_short:       Result = Context.ShortTy; break;
      case DeclSpec::TSW_long:        Result = Context.LongTy; break;
      case DeclSpec::TSW_longlong:
        Result = Context.LongLongTy;

        // 'long long' is a C99 or C++11 feature.
        if (!S.getLangOpts().C99) {
          if (S.getLangOpts().CPlusPlus)
            S.Diag(DS.getTypeSpecWidthLoc(),
                   S.getLangOpts().CPlusPlus11 ?
                   diag::warn_cxx98_compat_longlong : diag::ext_cxx11_longlong);
          else
            S.Diag(DS.getTypeSpecWidthLoc(), diag::ext_c99_longlong);
        }
        break;
      }
    } else {
      switch (DS.getTypeSpecWidth()) {
      case DeclSpec::TSW_unspecified: Result = Context.UnsignedIntTy; break;
      case DeclSpec::TSW_short:       Result = Context.UnsignedShortTy; break;
      case DeclSpec::TSW_long:        Result = Context.UnsignedLongTy; break;
      case DeclSpec::TSW_longlong:
        Result = Context.UnsignedLongLongTy;

        // 'long long' is a C99 or C++11 feature.
        if (!S.getLangOpts().C99) {
          if (S.getLangOpts().CPlusPlus)
            S.Diag(DS.getTypeSpecWidthLoc(),
                   S.getLangOpts().CPlusPlus11 ?
                   diag::warn_cxx98_compat_longlong : diag::ext_cxx11_longlong);
          else
            S.Diag(DS.getTypeSpecWidthLoc(), diag::ext_c99_longlong);
        }
        break;
      }
    }
    break;
  }
  case DeclSpec::TST_int128:
    if (!S.Context.getTargetInfo().hasInt128Type())
      S.Diag(DS.getTypeSpecTypeLoc(), diag::err_int128_unsupported);
    if (DS.getTypeSpecSign() == DeclSpec::TSS_unsigned)
      Result = Context.UnsignedInt128Ty;
    else
      Result = Context.Int128Ty;
    break;
  case DeclSpec::TST_half: Result = Context.HalfTy; break;
  case DeclSpec::TST_float: Result = Context.FloatTy; break;
    // HLSL Change Starts
  case DeclSpec::TST_halffloat: Result = Context.HalfFloatTy; break;
  case DeclSpec::TST_min16float: Result = Context.Min16FloatTy; break;
  case DeclSpec::TST_min16int: Result = Context.Min16IntTy; break;
  case DeclSpec::TST_min16uint: Result = Context.Min16UIntTy; break;
  case DeclSpec::TST_min10float: Result = Context.Min10FloatTy; break;
  case DeclSpec::TST_min12int: Result = Context.Min12IntTy; break;
  case DeclSpec::TST_int8_4packed: Result = Context.Int8_4PackedTy; break;
  case DeclSpec::TST_uint8_4packed: Result = Context.UInt8_4PackedTy; break;
    // HLSL Change Ends
  case DeclSpec::TST_double:
    if (DS.getTypeSpecWidth() == DeclSpec::TSW_long)
      Result = Context.LongDoubleTy;
    else
      Result = Context.DoubleTy;

    if (S.getLangOpts().OpenCL &&
        !((S.getLangOpts().OpenCLVersion >= 120) ||
          S.getOpenCLOptions().cl_khr_fp64)) {
      S.Diag(DS.getTypeSpecTypeLoc(), diag::err_type_requires_extension)
          << Result << "cl_khr_fp64";
      declarator.setInvalidType(true);
    }
    break;
  case DeclSpec::TST_bool: Result = Context.BoolTy; break; // _Bool or bool
  case DeclSpec::TST_decimal32:    // _Decimal32
  case DeclSpec::TST_decimal64:    // _Decimal64
  case DeclSpec::TST_decimal128:   // _Decimal128
    S.Diag(DS.getTypeSpecTypeLoc(), diag::err_decimal_unsupported);
    Result = Context.IntTy;
    declarator.setInvalidType(true);
    break;
  case DeclSpec::TST_class:
  case DeclSpec::TST_enum:
  case DeclSpec::TST_union:
  case DeclSpec::TST_struct:
  case DeclSpec::TST_interface: {
    TypeDecl *D = dyn_cast_or_null<TypeDecl>(DS.getRepAsDecl());
    if (!D) {
      // This can happen in C++ with ambiguous lookups.
      Result = Context.IntTy;
      declarator.setInvalidType(true);
      break;
    }

    // If the type is deprecated or unavailable, diagnose it.
    S.DiagnoseUseOfDecl(D, DS.getTypeSpecTypeNameLoc());

    assert(DS.getTypeSpecWidth() == 0 && DS.getTypeSpecComplex() == 0 &&
           DS.getTypeSpecSign() == 0 && "No qualifiers on tag names!");

    // TypeQuals handled by caller.
    Result = Context.getTypeDeclType(D);

    // In both C and C++, make an ElaboratedType.
    ElaboratedTypeKeyword Keyword
      = ElaboratedType::getKeywordForTypeSpec(DS.getTypeSpecType());
    Result = S.getElaboratedType(Keyword, DS.getTypeSpecScope(), Result);
    break;
  }
  case DeclSpec::TST_typename: {
    assert(DS.getTypeSpecWidth() == 0 && DS.getTypeSpecComplex() == 0 &&
           "Can't handle qualifiers on typedef names yet!");
    Result = S.GetTypeFromParser(DS.getRepAsType());
    if (Result.isNull()) {
      declarator.setInvalidType(true);
    } else if (S.getLangOpts().OpenCL) {
      if (const AtomicType *AT = Result->getAs<AtomicType>()) {
        const BuiltinType *BT = AT->getValueType()->getAs<BuiltinType>();
        bool NoExtTypes = BT && (BT->getKind() == BuiltinType::Int ||
                                 BT->getKind() == BuiltinType::UInt ||
                                 BT->getKind() == BuiltinType::Float);
        if (!S.getOpenCLOptions().cl_khr_int64_base_atomics && !NoExtTypes) {
          S.Diag(DS.getTypeSpecTypeLoc(), diag::err_type_requires_extension)
              << Result << "cl_khr_int64_base_atomics";
          declarator.setInvalidType(true);
        }
        if (!S.getOpenCLOptions().cl_khr_int64_extended_atomics &&
            !NoExtTypes) {
          S.Diag(DS.getTypeSpecTypeLoc(), diag::err_type_requires_extension)
              << Result << "cl_khr_int64_extended_atomics";
          declarator.setInvalidType(true);
        }
        if (!S.getOpenCLOptions().cl_khr_fp64 && BT &&
            BT->getKind() == BuiltinType::Double) {
          S.Diag(DS.getTypeSpecTypeLoc(), diag::err_type_requires_extension)
              << Result << "cl_khr_fp64";
          declarator.setInvalidType(true);
        }
      }
    }

    // TypeQuals handled by caller.
    break;
  }
  case DeclSpec::TST_typeofType:
    // FIXME: Preserve type source info.
    Result = S.GetTypeFromParser(DS.getRepAsType());
    assert(!Result.isNull() && "Didn't get a type for typeof?");
    if (!Result->isDependentType())
      if (const TagType *TT = Result->getAs<TagType>())
        S.DiagnoseUseOfDecl(TT->getDecl(), DS.getTypeSpecTypeLoc());
    // TypeQuals handled by caller.
    Result = Context.getTypeOfType(Result);
    break;
  case DeclSpec::TST_typeofExpr: {
    Expr *E = DS.getRepAsExpr();
    assert(E && "Didn't get an expression for typeof?");
    // TypeQuals handled by caller.
    Result = S.BuildTypeofExprType(E, DS.getTypeSpecTypeLoc());
    if (Result.isNull()) {
      Result = Context.IntTy;
      declarator.setInvalidType(true);
    }
    break;
  }
  case DeclSpec::TST_decltype: {
    Expr *E = DS.getRepAsExpr();
    assert(E && "Didn't get an expression for decltype?");
    // TypeQuals handled by caller.
    Result = S.BuildDecltypeType(E, DS.getTypeSpecTypeLoc());
    if (Result.isNull()) {
      Result = Context.IntTy;
      declarator.setInvalidType(true);
    }
    break;
  }
  case DeclSpec::TST_underlyingType:
    Result = S.GetTypeFromParser(DS.getRepAsType());
    assert(!Result.isNull() && "Didn't get a type for __underlying_type?");
    Result = S.BuildUnaryTransformType(Result,
                                       UnaryTransformType::EnumUnderlyingType,
                                       DS.getTypeSpecTypeLoc());
    if (Result.isNull()) {
      Result = Context.IntTy;
      declarator.setInvalidType(true);
    }
    break;

  case DeclSpec::TST_auto:
    // TypeQuals handled by caller.
    // If auto is mentioned in a lambda parameter context, convert it to a 
    // template parameter type immediately, with the appropriate depth and 
    // index, and update sema's state (LambdaScopeInfo) for the current lambda 
    // being analyzed (which tracks the invented type template parameter).
    if (declarator.getContext() == Declarator::LambdaExprParameterContext) {
      sema::LambdaScopeInfo *LSI = S.getCurLambda();
      assert(LSI && "No LambdaScopeInfo on the stack!");
      const unsigned TemplateParameterDepth = LSI->AutoTemplateParameterDepth;
      const unsigned AutoParameterPosition = LSI->AutoTemplateParams.size();
      const bool IsParameterPack = declarator.hasEllipsis();

      // Turns out we must create the TemplateTypeParmDecl here to 
      // retrieve the corresponding template parameter type. 
      TemplateTypeParmDecl *CorrespondingTemplateParam =
        TemplateTypeParmDecl::Create(Context, 
        // Temporarily add to the TranslationUnit DeclContext.  When the 
        // associated TemplateParameterList is attached to a template
        // declaration (such as FunctionTemplateDecl), the DeclContext 
        // for each template parameter gets updated appropriately via
        // a call to AdoptTemplateParameterList. 
        Context.getTranslationUnitDecl(), 
        /*KeyLoc*/ SourceLocation(), 
        /*NameLoc*/ declarator.getLocStart(),  
        TemplateParameterDepth, 
        AutoParameterPosition,  // our template param index 
        /* Identifier*/ nullptr, false, IsParameterPack);
      LSI->AutoTemplateParams.push_back(CorrespondingTemplateParam);
      // Replace the 'auto' in the function parameter with this invented 
      // template type parameter.
      Result = QualType(CorrespondingTemplateParam->getTypeForDecl(), 0);
    } else {
      Result = Context.getAutoType(QualType(), /*decltype(auto)*/false, false);
    }
    break;

  case DeclSpec::TST_decltype_auto:
    Result = Context.getAutoType(QualType(), 
                                 /*decltype(auto)*/true, 
                                 /*IsDependent*/   false);
    break;

  case DeclSpec::TST_unknown_anytype:
    Result = Context.UnknownAnyTy;
    break;

  case DeclSpec::TST_atomic:
    Result = S.GetTypeFromParser(DS.getRepAsType());
    assert(!Result.isNull() && "Didn't get a type for _Atomic?");
    Result = S.BuildAtomicType(Result, DS.getTypeSpecTypeLoc());
    if (Result.isNull()) {
      Result = Context.IntTy;
      declarator.setInvalidType(true);
    }
    break;

  case DeclSpec::TST_error:
    Result = Context.IntTy;
    declarator.setInvalidType(true);
    break;
  }

  // Handle complex types.
  if (DS.getTypeSpecComplex() == DeclSpec::TSC_complex) {
    if (S.getLangOpts().Freestanding)
      S.Diag(DS.getTypeSpecComplexLoc(), diag::ext_freestanding_complex);
    Result = Context.getComplexType(Result);
  } else if (DS.isTypeAltiVecVector()) {
    unsigned typeSize = static_cast<unsigned>(Context.getTypeSize(Result));
    assert(typeSize > 0 && "type size for vector must be greater than 0 bits");
    VectorType::VectorKind VecKind = VectorType::AltiVecVector;
    if (DS.isTypeAltiVecPixel())
      VecKind = VectorType::AltiVecPixel;
    else if (DS.isTypeAltiVecBool())
      VecKind = VectorType::AltiVecBool;
    Result = Context.getVectorType(Result, 128/typeSize, VecKind);
  }

  // FIXME: Imaginary.
  if (DS.getTypeSpecComplex() == DeclSpec::TSC_imaginary)
    S.Diag(DS.getTypeSpecComplexLoc(), diag::err_imaginary_not_supported);

  // Before we process any type attributes, synthesize a block literal
  // function declarator if necessary.
  if (declarator.getContext() == Declarator::BlockLiteralContext)
    maybeSynthesizeBlockSignature(state, Result);

  // Apply any type attributes from the decl spec.  This may cause the
  // list of type attributes to be temporarily saved while the type
  // attributes are pushed around.
  if (AttributeList *attrs = DS.getAttributes().getList())
    processTypeAttrs(state, Result, TAL_DeclSpec, attrs);

  // Apply const/volatile/restrict qualifiers to T.
  if (unsigned TypeQuals = DS.getTypeQualifiers()) {
    // Warn about CV qualifiers on function types.
    // C99 6.7.3p8:
    //   If the specification of a function type includes any type qualifiers,
    //   the behavior is undefined.
    // C++11 [dcl.fct]p7:
    //   The effect of a cv-qualifier-seq in a function declarator is not the
    //   same as adding cv-qualification on top of the function type. In the
    //   latter case, the cv-qualifiers are ignored.
    if (TypeQuals && Result->isFunctionType()) {
      diagnoseAndRemoveTypeQualifiers(
          S, DS, TypeQuals, Result, DeclSpec::TQ_const | DeclSpec::TQ_volatile,
          S.getLangOpts().CPlusPlus
              ? diag::warn_typecheck_function_qualifiers_ignored
              : diag::warn_typecheck_function_qualifiers_unspecified);
      // No diagnostic for 'restrict' or '_Atomic' applied to a
      // function type; we'll diagnose those later, in BuildQualifiedType.
    }

    // C++11 [dcl.ref]p1:
    //   Cv-qualified references are ill-formed except when the
    //   cv-qualifiers are introduced through the use of a typedef-name
    //   or decltype-specifier, in which case the cv-qualifiers are ignored.
    //
    // There don't appear to be any other contexts in which a cv-qualified
    // reference type could be formed, so the 'ill-formed' clause here appears
    // to never happen.
    if (TypeQuals && Result->isReferenceType()) {
      diagnoseAndRemoveTypeQualifiers(
          S, DS, TypeQuals, Result,
          DeclSpec::TQ_const | DeclSpec::TQ_volatile | DeclSpec::TQ_atomic,
          diag::warn_typecheck_reference_qualifiers);
    }

    // C90 6.5.3 constraints: "The same type qualifier shall not appear more
    // than once in the same specifier-list or qualifier-list, either directly
    // or via one or more typedefs."
    if (!S.getLangOpts().C99 && !S.getLangOpts().CPlusPlus
        && TypeQuals & Result.getCVRQualifiers()) {
      if (TypeQuals & DeclSpec::TQ_const && Result.isConstQualified()) {
        S.Diag(DS.getConstSpecLoc(), diag::ext_duplicate_declspec)
          << "const";
      }

      if (TypeQuals & DeclSpec::TQ_volatile && Result.isVolatileQualified()) {
        S.Diag(DS.getVolatileSpecLoc(), diag::ext_duplicate_declspec)
          << "volatile";
      }

      // C90 doesn't have restrict nor _Atomic, so it doesn't force us to
      // produce a warning in this case.
    }

    QualType Qualified = S.BuildQualifiedType(Result, DeclLoc, TypeQuals, &DS);

    // If adding qualifiers fails, just use the unqualified type.
    if (Qualified.isNull())
      declarator.setInvalidType(true);
    else
      Result = Qualified;
  }

  assert(!Result.isNull() && "This function should not return a null type");
  return Result;
}

static std::string getPrintableNameForEntity(DeclarationName Entity) {
  if (Entity)
    return Entity.getAsString();

  return "type name";
}

QualType Sema::BuildQualifiedType(QualType T, SourceLocation Loc,
                                  Qualifiers Qs, const DeclSpec *DS) {
  if (T.isNull())
    return QualType();

  // Enforce C99 6.7.3p2: "Types other than pointer types derived from
  // object or incomplete types shall not be restrict-qualified."
  if (Qs.hasRestrict()) {
    unsigned DiagID = 0;
    QualType ProblemTy;

    if (T->isAnyPointerType() || T->isReferenceType() ||
        T->isMemberPointerType()) {
      QualType EltTy;
      if (T->isObjCObjectPointerType())
        EltTy = T;
      else if (const MemberPointerType *PTy = T->getAs<MemberPointerType>())
        EltTy = PTy->getPointeeType();
      else
        EltTy = T->getPointeeType();

      // If we have a pointer or reference, the pointee must have an object
      // incomplete type.
      if (!EltTy->isIncompleteOrObjectType()) {
        DiagID = diag::err_typecheck_invalid_restrict_invalid_pointee;
        ProblemTy = EltTy;
      }
    } else if (!T->isDependentType()) {
      DiagID = diag::err_typecheck_invalid_restrict_not_pointer;
      ProblemTy = T;
    }

    if (DiagID) {
      Diag(DS ? DS->getRestrictSpecLoc() : Loc, DiagID) << ProblemTy;
      Qs.removeRestrict();
    }
  }

  return Context.getQualifiedType(T, Qs);
}

QualType Sema::BuildQualifiedType(QualType T, SourceLocation Loc,
                                  unsigned CVRA, const DeclSpec *DS) {
  if (T.isNull())
    return QualType();

  // Convert from DeclSpec::TQ to Qualifiers::TQ by just dropping TQ_atomic.
  unsigned CVR = CVRA & ~DeclSpec::TQ_atomic;

  // C11 6.7.3/5:
  //   If the same qualifier appears more than once in the same
  //   specifier-qualifier-list, either directly or via one or more typedefs,
  //   the behavior is the same as if it appeared only once.
  //
  // It's not specified what happens when the _Atomic qualifier is applied to
  // a type specified with the _Atomic specifier, but we assume that this
  // should be treated as if the _Atomic qualifier appeared multiple times.
  if (CVRA & DeclSpec::TQ_atomic && !T->isAtomicType()) {
    // C11 6.7.3/5:
    //   If other qualifiers appear along with the _Atomic qualifier in a
    //   specifier-qualifier-list, the resulting type is the so-qualified
    //   atomic type.
    //
    // Don't need to worry about array types here, since _Atomic can't be
    // applied to such types.
    SplitQualType Split = T.getSplitUnqualifiedType();
    T = BuildAtomicType(QualType(Split.Ty, 0),
                        DS ? DS->getAtomicSpecLoc() : Loc);
    if (T.isNull())
      return T;
    Split.Quals.addCVRQualifiers(CVR);
    return BuildQualifiedType(T, Loc, Split.Quals);
  }

  return BuildQualifiedType(T, Loc, Qualifiers::fromCVRMask(CVR), DS);
}

/// \brief Build a paren type including \p T.
QualType Sema::BuildParenType(QualType T) {
  return Context.getParenType(T);
}

/// Given that we're building a pointer or reference to the given
static QualType inferARCLifetimeForPointee(Sema &S, QualType type,
                                           SourceLocation loc,
                                           bool isReference) {
  // Bail out if retention is unrequired or already specified.
  if (!type->isObjCLifetimeType() ||
      type.getObjCLifetime() != Qualifiers::OCL_None)
    return type;

  Qualifiers::ObjCLifetime implicitLifetime = Qualifiers::OCL_None;

  // If the object type is const-qualified, we can safely use
  // __unsafe_unretained.  This is safe (because there are no read
  // barriers), and it'll be safe to coerce anything but __weak* to
  // the resulting type.
  if (type.isConstQualified()) {
    implicitLifetime = Qualifiers::OCL_ExplicitNone;

  // Otherwise, check whether the static type does not require
  // retaining.  This currently only triggers for Class (possibly
  // protocol-qualifed, and arrays thereof).
  } else if (type->isObjCARCImplicitlyUnretainedType()) {
    implicitLifetime = Qualifiers::OCL_ExplicitNone;

  // If we are in an unevaluated context, like sizeof, skip adding a
  // qualification.
  } else if (S.isUnevaluatedContext()) {
    return type;

  // If that failed, give an error and recover using __strong.  __strong
  // is the option most likely to prevent spurious second-order diagnostics,
  // like when binding a reference to a field.
  } else {
    // These types can show up in private ivars in system headers, so
    // we need this to not be an error in those cases.  Instead we
    // want to delay.
    if (S.DelayedDiagnostics.shouldDelayDiagnostics()) {
      S.DelayedDiagnostics.add(
          sema::DelayedDiagnostic::makeForbiddenType(loc,
              diag::err_arc_indirect_no_ownership, type, isReference));
    } else {
      S.Diag(loc, diag::err_arc_indirect_no_ownership) << type << isReference;
    }
    implicitLifetime = Qualifiers::OCL_Strong;
  }
  assert(implicitLifetime && "didn't infer any lifetime!");

  Qualifiers qs;
  qs.addObjCLifetime(implicitLifetime);
  return S.Context.getQualifiedType(type, qs);
}

static std::string getFunctionQualifiersAsString(const FunctionProtoType *FnTy){
  std::string Quals =
    Qualifiers::fromCVRMask(FnTy->getTypeQuals()).getAsString();

  switch (FnTy->getRefQualifier()) {
  case RQ_None:
    break;

  case RQ_LValue:
    if (!Quals.empty())
      Quals += ' ';
    Quals += '&';
    break;

  case RQ_RValue:
    if (!Quals.empty())
      Quals += ' ';
    Quals += "&&";
    break;
  }

  return Quals;
}

namespace {
/// Kinds of declarator that cannot contain a qualified function type.
///
/// C++98 [dcl.fct]p4 / C++11 [dcl.fct]p6:
///     a function type with a cv-qualifier or a ref-qualifier can only appear
///     at the topmost level of a type.
///
/// Parens and member pointers are permitted. We don't diagnose array and
/// function declarators, because they don't allow function types at all.
///
/// The values of this enum are used in diagnostics.
enum QualifiedFunctionKind { QFK_BlockPointer, QFK_Pointer, QFK_Reference };
}

/// Check whether the type T is a qualified function type, and if it is,
/// diagnose that it cannot be contained within the given kind of declarator.
static bool checkQualifiedFunction(Sema &S, QualType T, SourceLocation Loc,
                                   QualifiedFunctionKind QFK) {
  // Does T refer to a function type with a cv-qualifier or a ref-qualifier?
  const FunctionProtoType *FPT = T->getAs<FunctionProtoType>();
  if (!FPT || (FPT->getTypeQuals() == 0 && FPT->getRefQualifier() == RQ_None))
    return false;

  S.Diag(Loc, diag::err_compound_qualified_function_type)
    << QFK << isa<FunctionType>(T.IgnoreParens()) << T
    << getFunctionQualifiersAsString(FPT);
  return true;
}

/// \brief Build a pointer type.
///
/// \param T The type to which we'll be building a pointer.
///
/// \param Loc The location of the entity whose type involves this
/// pointer type or, if there is no such entity, the location of the
/// type that will have pointer type.
///
/// \param Entity The name of the entity that involves the pointer
/// type, if known.
///
/// \returns A suitable pointer type, if there are no
/// errors. Otherwise, returns a NULL type.
QualType Sema::BuildPointerType(QualType T,
                                SourceLocation Loc, DeclarationName Entity) {
  if (T->isReferenceType()) {
    // C++ 8.3.2p4: There shall be no ... pointers to references ...
    Diag(Loc, diag::err_illegal_decl_pointer_to_reference)
      << getPrintableNameForEntity(Entity) << T;
    return QualType();
  }

  if (checkQualifiedFunction(*this, T, Loc, QFK_Pointer))
    return QualType();

  assert(!T->isObjCObjectType() && "Should build ObjCObjectPointerType");

  // In ARC, it is forbidden to build pointers to unqualified pointers.
  if (getLangOpts().ObjCAutoRefCount)
    T = inferARCLifetimeForPointee(*this, T, Loc, /*reference*/ false);

  // Build the pointer type.
  return Context.getPointerType(T);
}

/// \brief Build a reference type.
///
/// \param T The type to which we'll be building a reference.
///
/// \param Loc The location of the entity whose type involves this
/// reference type or, if there is no such entity, the location of the
/// type that will have reference type.
///
/// \param Entity The name of the entity that involves the reference
/// type, if known.
///
/// \returns A suitable reference type, if there are no
/// errors. Otherwise, returns a NULL type.
QualType Sema::BuildReferenceType(QualType T, bool SpelledAsLValue,
                                  SourceLocation Loc,
                                  DeclarationName Entity) {
  assert(Context.getCanonicalType(T) != Context.OverloadTy &&
         "Unresolved overloaded function type");

  // C++0x [dcl.ref]p6:
  //   If a typedef (7.1.3), a type template-parameter (14.3.1), or a
  //   decltype-specifier (7.1.6.2) denotes a type TR that is a reference to a
  //   type T, an attempt to create the type "lvalue reference to cv TR" creates
  //   the type "lvalue reference to T", while an attempt to create the type
  //   "rvalue reference to cv TR" creates the type TR.
  bool LValueRef = SpelledAsLValue || T->getAs<LValueReferenceType>();

  // C++ [dcl.ref]p4: There shall be no references to references.
  //
  // According to C++ DR 106, references to references are only
  // diagnosed when they are written directly (e.g., "int & &"),
  // but not when they happen via a typedef:
  //
  //   typedef int& intref;
  //   typedef intref& intref2;
  //
  // Parser::ParseDeclaratorInternal diagnoses the case where
  // references are written directly; here, we handle the
  // collapsing of references-to-references as described in C++0x.
  // DR 106 and 540 introduce reference-collapsing into C++98/03.

  // C++ [dcl.ref]p1:
  //   A declarator that specifies the type "reference to cv void"
  //   is ill-formed.
  if (T->isVoidType()) {
    Diag(Loc, diag::err_reference_to_void);
    return QualType();
  }

  if (checkQualifiedFunction(*this, T, Loc, QFK_Reference))
    return QualType();

  // In ARC, it is forbidden to build references to unqualified pointers.
  if (getLangOpts().ObjCAutoRefCount)
    T = inferARCLifetimeForPointee(*this, T, Loc, /*reference*/ true);

  // Handle restrict on references.
  if (LValueRef)
    return Context.getLValueReferenceType(T, SpelledAsLValue);
  return Context.getRValueReferenceType(T);
}

/// Check whether the specified array size makes the array type a VLA.  If so,
/// return true, if not, return the size of the array in SizeVal.
static bool isArraySizeVLA(Sema &S, Expr *ArraySize, llvm::APSInt &SizeVal) {
  // If the size is an ICE, it certainly isn't a VLA. If we're in a GNU mode
  // (like gnu99, but not c99) accept any evaluatable value as an extension.
  class VLADiagnoser : public Sema::VerifyICEDiagnoser {
  public:
    VLADiagnoser() : Sema::VerifyICEDiagnoser(true) {}

    void diagnoseNotICE(Sema &S, SourceLocation Loc, SourceRange SR) override {
    }

    void diagnoseFold(Sema &S, SourceLocation Loc, SourceRange SR) override {
      S.Diag(Loc, diag::ext_vla_folded_to_constant) << SR;
    }
  } Diagnoser;

  return S.VerifyIntegerConstantExpression(ArraySize, &SizeVal, Diagnoser,
                                           S.LangOpts.GNUMode).isInvalid();
}


/// \brief Build an array type.
///
/// \param T The type of each element in the array.
///
/// \param ASM C99 array size modifier (e.g., '*', 'static').
///
/// \param ArraySize Expression describing the size of the array.
///
/// \param Brackets The range from the opening '[' to the closing ']'.
///
/// \param Entity The name of the entity that involves the array
/// type, if known.
///
/// \returns A suitable array type, if there are no errors. Otherwise,
/// returns a NULL type.
QualType Sema::BuildArrayType(QualType T, ArrayType::ArraySizeModifier ASM,
                              Expr *ArraySize, unsigned Quals,
                              SourceRange Brackets, DeclarationName Entity) {

  SourceLocation Loc = Brackets.getBegin();
  if (getLangOpts().CPlusPlus) {
    // C++ [dcl.array]p1:
    //   T is called the array element type; this type shall not be a reference
    //   type, the (possibly cv-qualified) type void, a function type or an
    //   abstract class type.
    //
    // C++ [dcl.array]p3:
    //   When several "array of" specifications are adjacent, [...] only the
    //   first of the constant expressions that specify the bounds of the arrays
    //   may be omitted.
    //
    // Note: function types are handled in the common path with C.
    if (T->isReferenceType()) {
      Diag(Loc, diag::err_illegal_decl_array_of_references)
      << getPrintableNameForEntity(Entity) << T;
      return QualType();
    }

    if (T->isVoidType() || T->isIncompleteArrayType()) {
      Diag(Loc, diag::err_illegal_decl_array_incomplete_type) << T;
      return QualType();
    }

    // HLSL Change Start
    // no arrays of strings in HLSL
    if (getLangOpts().HLSL && hlsl::IsStringType(T)) {
      static const unsigned selectArrayIdx = 0;
      Diag(Loc, diag::err_hlsl_unsupported_string_decl) << selectArrayIdx;
      return QualType();
    }
    // HLSL Change End

    if (RequireNonAbstractType(Brackets.getBegin(), T,
                               diag::err_array_of_abstract_type))
      return QualType();

    // Mentioning a member pointer type for an array type causes us to lock in
    // an inheritance model, even if it's inside an unused typedef.
    if (Context.getTargetInfo().getCXXABI().isMicrosoft())
      if (const MemberPointerType *MPTy = T->getAs<MemberPointerType>())
        if (!MPTy->getClass()->isDependentType())
          RequireCompleteType(Loc, T, 0);

  } else {
    // C99 6.7.5.2p1: If the element type is an incomplete or function type,
    // reject it (e.g. void ary[7], struct foo ary[7], void ary[7]())
    if (RequireCompleteType(Loc, T,
                            diag::err_illegal_decl_array_incomplete_type))
      return QualType();
  }

  if (T->isFunctionType()) {
    Diag(Loc, diag::err_illegal_decl_array_of_functions)
      << getPrintableNameForEntity(Entity) << T;
    return QualType();
  }

  if (const RecordType *EltTy = T->getAs<RecordType>()) {
    // If the element type is a struct or union that contains a variadic
    // array, accept it as a GNU extension: C99 6.7.2.1p2.
    if (EltTy->getDecl()->hasFlexibleArrayMember())
      Diag(Loc, diag::ext_flexible_array_in_array) << T;
  } else if (T->isObjCObjectType()) {
    Diag(Loc, diag::err_objc_array_of_interfaces) << T;
    return QualType();
  }

  // Do placeholder conversions on the array size expression.
  if (ArraySize && ArraySize->hasPlaceholderType()) {
    ExprResult Result = CheckPlaceholderExpr(ArraySize);
    if (Result.isInvalid()) return QualType();
    ArraySize = Result.get();
  }

  // Do lvalue-to-rvalue conversions on the array size expression.
  if (ArraySize && !ArraySize->isRValue()) {
    ExprResult Result = DefaultLvalueConversion(ArraySize);
    if (Result.isInvalid())
      return QualType();

    ArraySize = Result.get();
  }

  // C99 6.7.5.2p1: The size expression shall have integer type.
  // C++11 allows contextual conversions to such types.
  if (!getLangOpts().CPlusPlus11 &&
      ArraySize && !ArraySize->isTypeDependent() &&
      !ArraySize->getType()->isIntegralOrUnscopedEnumerationType()) {
    Diag(ArraySize->getLocStart(), diag::err_array_size_non_int)
      << ArraySize->getType() << ArraySize->getSourceRange();
    return QualType();
  }

  llvm::APSInt ConstVal(Context.getTypeSize(Context.getSizeType()));
  if (!ArraySize) {
    if (ASM == ArrayType::Star)
      T = Context.getVariableArrayType(T, nullptr, ASM, Quals, Brackets);
    else
      T = Context.getIncompleteArrayType(T, ASM, Quals);
  } else if (ArraySize->isTypeDependent() || ArraySize->isValueDependent()) {
    T = Context.getDependentSizedArrayType(T, ArraySize, ASM, Quals, Brackets);
  } else if ((!T->isDependentType() && !T->isIncompleteType() &&
              !T->isConstantSizeType()) ||
             isArraySizeVLA(*this, ArraySize, ConstVal)) {
    // Even in C++11, don't allow contextual conversions in the array bound
    // of a VLA.
    if (getLangOpts().CPlusPlus11 &&
        !ArraySize->getType()->isIntegralOrUnscopedEnumerationType()) {
      Diag(ArraySize->getLocStart(), diag::err_array_size_non_int)
        << ArraySize->getType() << ArraySize->getSourceRange();
      return QualType();
    }

    // C99: an array with an element type that has a non-constant-size is a VLA.
    // C99: an array with a non-ICE size is a VLA.  We accept any expression
    // that we can fold to a non-zero positive value as an extension.
    T = Context.getVariableArrayType(T, ArraySize, ASM, Quals, Brackets);
  } else {
    // C99 6.7.5.2p1: If the expression is a constant expression, it shall
    // have a value greater than zero.
    if (ConstVal.isSigned() && ConstVal.isNegative()) {
      if (Entity)
        Diag(ArraySize->getLocStart(), diag::err_decl_negative_array_size)
          << getPrintableNameForEntity(Entity) << ArraySize->getSourceRange();
      else
        Diag(ArraySize->getLocStart(), diag::err_typecheck_negative_array_size)
          << ArraySize->getSourceRange();
      return QualType();
    }
    if (ConstVal == 0) {
      // GCC accepts zero sized static arrays. We allow them when
      // we're not in a SFINAE context.
      Diag(ArraySize->getLocStart(),
           isSFINAEContext()? diag::err_typecheck_zero_array_size
                            : diag::ext_typecheck_zero_array_size)
        << ArraySize->getSourceRange();

      if (ASM == ArrayType::Static) {
        Diag(ArraySize->getLocStart(),
             diag::warn_typecheck_zero_static_array_size)
          << ArraySize->getSourceRange();
        ASM = ArrayType::Normal;
      }
    } else if (!T->isDependentType() && !T->isVariablyModifiedType() &&
               !T->isIncompleteType() && !T->isUndeducedType()) {
      // Is the array too large?
      unsigned ActiveSizeBits
        = ConstantArrayType::getNumAddressingBits(Context, T, ConstVal);
      if (ActiveSizeBits > ConstantArrayType::getMaxSizeBits(Context)) {
        Diag(ArraySize->getLocStart(), diag::err_array_too_large)
          << ConstVal.toString(10)
          << ArraySize->getSourceRange();
        return QualType();
      }
    }

    T = Context.getConstantArrayType(T, ConstVal, ASM, Quals);
  }

  // HLSL Change Starts
  if (getLangOpts().HLSL && T->isVariableArrayType()) {
    Diag(Loc, diag::err_hlsl_vla);
    return QualType();
  }
  // HLSL Change Stops

  // OpenCL v1.2 s6.9.d: variable length arrays are not supported.
  if (getLangOpts().OpenCL && T->isVariableArrayType()) {
    Diag(Loc, diag::err_opencl_vla);
    return QualType();
  }
  // If this is not C99, extwarn about VLA's and C99 array size modifiers.
  if (!getLangOpts().C99) {
    if (T->isVariableArrayType()) {
      // Prohibit the use of non-POD types in VLAs.
      QualType BaseT = Context.getBaseElementType(T);
      if (!T->isDependentType() &&
          !RequireCompleteType(Loc, BaseT, 0) &&
          !BaseT.isPODType(Context) &&
          !BaseT->isObjCLifetimeType()) {
        Diag(Loc, diag::err_vla_non_pod)
          << BaseT;
        return QualType();
      }
      // Prohibit the use of VLAs during template argument deduction.
      else if (isSFINAEContext()) {
        Diag(Loc, diag::err_vla_in_sfinae);
        return QualType();
      }
      // Just extwarn about VLAs.
      else
        Diag(Loc, diag::ext_vla);
    } else if ((ASM != ArrayType::Normal || Quals != 0) && ! getLangOpts().HLSL) // HLSL Change: already fails in HLSL
      Diag(Loc,
           getLangOpts().CPlusPlus? diag::err_c99_array_usage_cxx
                                     : diag::ext_c99_array_usage) << ASM;
  }

  if (T->isVariableArrayType()) {
    // Warn about VLAs for -Wvla.
    Diag(Loc, diag::warn_vla_used);
  }

  return T;
}

/// \brief Build an ext-vector type.
///
/// Run the required checks for the extended vector type.
QualType Sema::BuildExtVectorType(QualType T, Expr *ArraySize,
                                  SourceLocation AttrLoc) {
  // unlike gcc's vector_size attribute, we do not allow vectors to be defined
  // in conjunction with complex types (pointers, arrays, functions, etc.).
  if (!T->isDependentType() &&
      !T->isIntegerType() && !T->isRealFloatingType()) {
    Diag(AttrLoc, diag::err_attribute_invalid_vector_type) << T;
    return QualType();
  }

  if (!ArraySize->isTypeDependent() && !ArraySize->isValueDependent()) {
    llvm::APSInt vecSize(32);
    if (!ArraySize->isIntegerConstantExpr(vecSize, Context)) {
      Diag(AttrLoc, diag::err_attribute_argument_type)
        << "ext_vector_type" << AANT_ArgumentIntegerConstant
        << ArraySize->getSourceRange();
      return QualType();
    }

    // unlike gcc's vector_size attribute, the size is specified as the
    // number of elements, not the number of bytes.
    unsigned vectorSize = static_cast<unsigned>(vecSize.getZExtValue());

    if (vectorSize == 0) {
      Diag(AttrLoc, diag::err_attribute_zero_size)
      << ArraySize->getSourceRange();
      return QualType();
    }

    if (VectorType::isVectorSizeTooLarge(vectorSize)) {
      Diag(AttrLoc, diag::err_attribute_size_too_large)
        << ArraySize->getSourceRange();
      return QualType();
    }

    return Context.getExtVectorType(T, vectorSize);
  }

  return Context.getDependentSizedExtVectorType(T, ArraySize, AttrLoc);
}

bool Sema::CheckFunctionReturnType(QualType T, SourceLocation Loc) {
  if (T->isArrayType() || T->isFunctionType()) {
    Diag(Loc, diag::err_func_returning_array_function)
      << T->isFunctionType() << T;
    return true;
  }

  // Functions cannot return half FP.
  if (T->isHalfType() && !getLangOpts().HalfArgsAndReturns) {
    Diag(Loc, diag::err_parameters_retval_cannot_have_fp16_type) << 1 <<
      FixItHint::CreateInsertion(Loc, "*");
    return true;
  }

  // Methods cannot return interface types. All ObjC objects are
  // passed by reference.
  if (T->isObjCObjectType()) {
    Diag(Loc, diag::err_object_cannot_be_passed_returned_by_value) << 0 << T;
    return 0;
  }

  return false;
}

// HLSL Change - FIX - We should move param mods to parameter QualTypes
QualType Sema::BuildFunctionType(QualType T,
                                 MutableArrayRef<QualType> ParamTypes,
                                 SourceLocation Loc, DeclarationName Entity,
                                 const FunctionProtoType::ExtProtoInfo &EPI,
                                 ArrayRef<hlsl::ParameterModifier> ParamMods) {
// HLSL Change - End
  bool Invalid = false;

  Invalid |= CheckFunctionReturnType(T, Loc);

  for (unsigned Idx = 0, Cnt = ParamTypes.size(); Idx < Cnt; ++Idx) {
    // FIXME: Loc is too inprecise here, should use proper locations for args.
    QualType ParamType = Context.getAdjustedParameterType(ParamTypes[Idx]);
    if (ParamType->isVoidType()) {
      Diag(Loc, diag::err_param_with_void_type);
      Invalid = true;
    } else if (ParamType->isHalfType() && !getLangOpts().HalfArgsAndReturns) {
      // Disallow half FP arguments.
      Diag(Loc, diag::err_parameters_retval_cannot_have_fp16_type) << 0 <<
        FixItHint::CreateInsertion(Loc, "*");
      Invalid = true;
    }

    ParamTypes[Idx] = ParamType;
  }

  if (Invalid)
    return QualType();

  // HLSL Change - FIX - We should move param mods to parameter QualTypes
  return Context.getFunctionType(T, ParamTypes, EPI, ParamMods);
  // HLSL Change End
}

/// \brief Build a member pointer type \c T Class::*.
///
/// \param T the type to which the member pointer refers.
/// \param Class the class type into which the member pointer points.
/// \param Loc the location where this type begins
/// \param Entity the name of the entity that will have this member pointer type
///
/// \returns a member pointer type, if successful, or a NULL type if there was
/// an error.
QualType Sema::BuildMemberPointerType(QualType T, QualType Class,
                                      SourceLocation Loc,
                                      DeclarationName Entity) {
  // Verify that we're not building a pointer to pointer to function with
  // exception specification.
  if (CheckDistantExceptionSpec(T)) {
    Diag(Loc, diag::err_distant_exception_spec);
    return QualType();
  }

  // C++ 8.3.3p3: A pointer to member shall not point to ... a member
  //   with reference type, or "cv void."
  if (T->isReferenceType()) {
    Diag(Loc, diag::err_illegal_decl_mempointer_to_reference)
      << getPrintableNameForEntity(Entity) << T;
    return QualType();
  }

  if (T->isVoidType()) {
    Diag(Loc, diag::err_illegal_decl_mempointer_to_void)
      << getPrintableNameForEntity(Entity);
    return QualType();
  }

  if (!Class->isDependentType() && !Class->isRecordType()) {
    Diag(Loc, diag::err_mempointer_in_nonclass_type) << Class;
    return QualType();
  }

  // Adjust the default free function calling convention to the default method
  // calling convention.
  if (T->isFunctionType())
    adjustMemberFunctionCC(T, /*IsStatic=*/false);

  return Context.getMemberPointerType(T, Class.getTypePtr());
}

/// \brief Build a block pointer type.
///
/// \param T The type to which we'll be building a block pointer.
///
/// \param Loc The source location, used for diagnostics.
///
/// \param Entity The name of the entity that involves the block pointer
/// type, if known.
///
/// \returns A suitable block pointer type, if there are no
/// errors. Otherwise, returns a NULL type.
QualType Sema::BuildBlockPointerType(QualType T,
                                     SourceLocation Loc,
                                     DeclarationName Entity) {
  if (!T->isFunctionType()) {
    Diag(Loc, diag::err_nonfunction_block_type);
    return QualType();
  }

  if (checkQualifiedFunction(*this, T, Loc, QFK_BlockPointer))
    return QualType();

  return Context.getBlockPointerType(T);
}

QualType Sema::GetTypeFromParser(ParsedType Ty, TypeSourceInfo **TInfo) {
  QualType QT = Ty.get();
  if (QT.isNull()) {
    if (TInfo) *TInfo = nullptr;
    return QualType();
  }

  TypeSourceInfo *DI = nullptr;
  if (const LocInfoType *LIT = dyn_cast<LocInfoType>(QT)) {
    QT = LIT->getType();
    DI = LIT->getTypeSourceInfo();
  }

  if (TInfo) *TInfo = DI;
  return QT;
}

static void transferARCOwnershipToDeclaratorChunk(TypeProcessingState &state,
                                            Qualifiers::ObjCLifetime ownership,
                                            unsigned chunkIndex);

/// Given that this is the declaration of a parameter under ARC,
/// attempt to infer attributes and such for pointer-to-whatever
/// types.
static void inferARCWriteback(TypeProcessingState &state,
                              QualType &declSpecType) {
  Sema &S = state.getSema();
  Declarator &declarator = state.getDeclarator();

  // TODO: should we care about decl qualifiers?

  // Check whether the declarator has the expected form.  We walk
  // from the inside out in order to make the block logic work.
  unsigned outermostPointerIndex = 0;
  bool isBlockPointer = false;
  unsigned numPointers = 0;
  for (unsigned i = 0, e = declarator.getNumTypeObjects(); i != e; ++i) {
    unsigned chunkIndex = i;
    DeclaratorChunk &chunk = declarator.getTypeObject(chunkIndex);
    switch (chunk.Kind) {
    case DeclaratorChunk::Paren:
      // Ignore parens.
      break;

    case DeclaratorChunk::Reference:
    case DeclaratorChunk::Pointer:
      // Count the number of pointers.  Treat references
      // interchangeably as pointers; if they're mis-ordered, normal
      // type building will discover that.
      outermostPointerIndex = chunkIndex;
      numPointers++;
      break;

    case DeclaratorChunk::BlockPointer:
      // If we have a pointer to block pointer, that's an acceptable
      // indirect reference; anything else is not an application of
      // the rules.
      if (numPointers != 1) return;
      numPointers++;
      outermostPointerIndex = chunkIndex;
      isBlockPointer = true;

      // We don't care about pointer structure in return values here.
      goto done;

    case DeclaratorChunk::Array: // suppress if written (id[])?
    case DeclaratorChunk::Function:
    case DeclaratorChunk::MemberPointer:
      return;
    }
  }
 done:

  // If we have *one* pointer, then we want to throw the qualifier on
  // the declaration-specifiers, which means that it needs to be a
  // retainable object type.
  if (numPointers == 1) {
    // If it's not a retainable object type, the rule doesn't apply.
    if (!declSpecType->isObjCRetainableType()) return;

    // If it already has lifetime, don't do anything.
    if (declSpecType.getObjCLifetime()) return;

    // Otherwise, modify the type in-place.
    Qualifiers qs;

    if (declSpecType->isObjCARCImplicitlyUnretainedType())
      qs.addObjCLifetime(Qualifiers::OCL_ExplicitNone);
    else
      qs.addObjCLifetime(Qualifiers::OCL_Autoreleasing);
    declSpecType = S.Context.getQualifiedType(declSpecType, qs);

  // If we have *two* pointers, then we want to throw the qualifier on
  // the outermost pointer.
  } else if (numPointers == 2) {
    // If we don't have a block pointer, we need to check whether the
    // declaration-specifiers gave us something that will turn into a
    // retainable object pointer after we slap the first pointer on it.
    if (!isBlockPointer && !declSpecType->isObjCObjectType())
      return;

    // Look for an explicit lifetime attribute there.
    DeclaratorChunk &chunk = declarator.getTypeObject(outermostPointerIndex);
    if (chunk.Kind != DeclaratorChunk::Pointer &&
        chunk.Kind != DeclaratorChunk::BlockPointer)
      return;
    for (const AttributeList *attr = chunk.getAttrs(); attr;
           attr = attr->getNext())
      if (attr->getKind() == AttributeList::AT_ObjCOwnership)
        return;

    transferARCOwnershipToDeclaratorChunk(state, Qualifiers::OCL_Autoreleasing,
                                          outermostPointerIndex);

  // Any other number of pointers/references does not trigger the rule.
  } else return;

  // TODO: mark whether we did this inference?
}

void Sema::diagnoseIgnoredQualifiers(unsigned DiagID, unsigned Quals,
                                     SourceLocation FallbackLoc,
                                     SourceLocation ConstQualLoc,
                                     SourceLocation VolatileQualLoc,
                                     SourceLocation RestrictQualLoc,
                                     SourceLocation AtomicQualLoc) {
  if (!Quals)
    return;

  struct Qual {
    unsigned Mask;
    const char *Name;
    SourceLocation Loc;
  } const QualKinds[4] = {
    { DeclSpec::TQ_const, "const", ConstQualLoc },
    { DeclSpec::TQ_volatile, "volatile", VolatileQualLoc },
    { DeclSpec::TQ_restrict, "restrict", RestrictQualLoc },
    { DeclSpec::TQ_atomic, "_Atomic", AtomicQualLoc }
  };

  SmallString<32> QualStr;
  unsigned NumQuals = 0;
  SourceLocation Loc;
  FixItHint FixIts[4];

  // Build a string naming the redundant qualifiers.
  for (unsigned I = 0; I != 4; ++I) {
    if (Quals & QualKinds[I].Mask) {
      if (!QualStr.empty()) QualStr += ' ';
      QualStr += QualKinds[I].Name;

      // If we have a location for the qualifier, offer a fixit.
      SourceLocation QualLoc = QualKinds[I].Loc;
      if (!QualLoc.isInvalid()) {
        FixIts[NumQuals] = FixItHint::CreateRemoval(QualLoc);
        if (Loc.isInvalid() ||
            getSourceManager().isBeforeInTranslationUnit(QualLoc, Loc))
          Loc = QualLoc;
      }

      ++NumQuals;
    }
  }

  Diag(Loc.isInvalid() ? FallbackLoc : Loc, DiagID)
    << QualStr << NumQuals << FixIts[0] << FixIts[1] << FixIts[2] << FixIts[3];
}

// Diagnose pointless type qualifiers on the return type of a function.
static void diagnoseRedundantReturnTypeQualifiers(Sema &S, QualType RetTy,
                                                  Declarator &D,
                                                  unsigned FunctionChunkIndex) {
  if (D.getTypeObject(FunctionChunkIndex).Fun.hasTrailingReturnType()) {
    // FIXME: TypeSourceInfo doesn't preserve location information for
    // qualifiers.
    S.diagnoseIgnoredQualifiers(diag::warn_qual_return_type,
                                RetTy.getLocalCVRQualifiers(),
                                D.getIdentifierLoc());
    return;
  }

  for (unsigned OuterChunkIndex = FunctionChunkIndex + 1,
                End = D.getNumTypeObjects();
       OuterChunkIndex != End; ++OuterChunkIndex) {
    DeclaratorChunk &OuterChunk = D.getTypeObject(OuterChunkIndex);
    switch (OuterChunk.Kind) {
    case DeclaratorChunk::Paren:
      continue;

    case DeclaratorChunk::Pointer: {
      DeclaratorChunk::PointerTypeInfo &PTI = OuterChunk.Ptr;
      S.diagnoseIgnoredQualifiers(
          diag::warn_qual_return_type,
          PTI.TypeQuals,
          SourceLocation(),
          SourceLocation::getFromRawEncoding(PTI.ConstQualLoc),
          SourceLocation::getFromRawEncoding(PTI.VolatileQualLoc),
          SourceLocation::getFromRawEncoding(PTI.RestrictQualLoc),
          SourceLocation::getFromRawEncoding(PTI.AtomicQualLoc));
      return;
    }

    case DeclaratorChunk::Function:
    case DeclaratorChunk::BlockPointer:
    case DeclaratorChunk::Reference:
    case DeclaratorChunk::Array:
    case DeclaratorChunk::MemberPointer:
      // FIXME: We can't currently provide an accurate source location and a
      // fix-it hint for these.
      unsigned AtomicQual = RetTy->isAtomicType() ? DeclSpec::TQ_atomic : 0;
      S.diagnoseIgnoredQualifiers(diag::warn_qual_return_type,
                                  RetTy.getCVRQualifiers() | AtomicQual,
                                  D.getIdentifierLoc());
      return;
    }

    llvm_unreachable("unknown declarator chunk kind");
  }

  // If the qualifiers come from a conversion function type, don't diagnose
  // them -- they're not necessarily redundant, since such a conversion
  // operator can be explicitly called as "x.operator const int()".
  if (D.getName().getKind() == UnqualifiedId::IK_ConversionFunctionId)
    return;

  // Just parens all the way out to the decl specifiers. Diagnose any qualifiers
  // which are present there.
  S.diagnoseIgnoredQualifiers(diag::warn_qual_return_type,
                              D.getDeclSpec().getTypeQualifiers(),
                              D.getIdentifierLoc(),
                              D.getDeclSpec().getConstSpecLoc(),
                              D.getDeclSpec().getVolatileSpecLoc(),
                              D.getDeclSpec().getRestrictSpecLoc(),
                              D.getDeclSpec().getAtomicSpecLoc());
}

static QualType GetDeclSpecTypeForDeclarator(TypeProcessingState &state,
                                             TypeSourceInfo *&ReturnTypeInfo) {
  Sema &SemaRef = state.getSema();
  Declarator &D = state.getDeclarator();
  QualType T;
  ReturnTypeInfo = nullptr;

  // The TagDecl owned by the DeclSpec.
  TagDecl *OwnedTagDecl = nullptr;

  bool ContainsPlaceholderType = false;

  switch (D.getName().getKind()) {
  case UnqualifiedId::IK_ImplicitSelfParam:
  case UnqualifiedId::IK_OperatorFunctionId:
  case UnqualifiedId::IK_Identifier:
  case UnqualifiedId::IK_LiteralOperatorId:
  case UnqualifiedId::IK_TemplateId:
    T = ConvertDeclSpecToType(state);
    ContainsPlaceholderType = D.getDeclSpec().containsPlaceholderType();

    if (!D.isInvalidType() && D.getDeclSpec().isTypeSpecOwned()) {
      OwnedTagDecl = cast<TagDecl>(D.getDeclSpec().getRepAsDecl());
      // Owned declaration is embedded in declarator.
      OwnedTagDecl->setEmbeddedInDeclarator(true);
    }
    break;

  case UnqualifiedId::IK_ConstructorName:
  case UnqualifiedId::IK_ConstructorTemplateId:
  case UnqualifiedId::IK_DestructorName:
    // Constructors and destructors don't have return types. Use
    // "void" instead.
    T = SemaRef.Context.VoidTy;
    if (AttributeList *attrs = D.getDeclSpec().getAttributes().getList())
      processTypeAttrs(state, T, TAL_DeclSpec, attrs);
    break;

  case UnqualifiedId::IK_ConversionFunctionId:
    // The result type of a conversion function is the type that it
    // converts to.
    T = SemaRef.GetTypeFromParser(D.getName().ConversionFunctionId,
                                  &ReturnTypeInfo);
    ContainsPlaceholderType = T->getContainedAutoType();
    break;
  }

  if (D.getAttributes())
    distributeTypeAttrsFromDeclarator(state, T);

  // C++11 [dcl.spec.auto]p5: reject 'auto' if it is not in an allowed context.
  // In C++11, a function declarator using 'auto' must have a trailing return
  // type (this is checked later) and we can skip this. In other languages
  // using auto, we need to check regardless.
  // C++14 In generic lambdas allow 'auto' in their parameters.
  if (ContainsPlaceholderType &&
      (!SemaRef.getLangOpts().CPlusPlus11 || !D.isFunctionDeclarator())) {
    int Error = -1;

    switch (D.getContext()) {
    case Declarator::KNRTypeListContext:
      llvm_unreachable("K&R type lists aren't allowed in C++");
    case Declarator::LambdaExprContext:
      llvm_unreachable("Can't specify a type specifier in lambda grammar");
    case Declarator::ObjCParameterContext:
    case Declarator::ObjCResultContext:
    case Declarator::PrototypeContext:
      Error = 0;  
      break;
    case Declarator::LambdaExprParameterContext:
      if (!(SemaRef.getLangOpts().CPlusPlus14 
              && D.getDeclSpec().getTypeSpecType() == DeclSpec::TST_auto))
        Error = 14;
      break;
    case Declarator::MemberContext:
      if (D.getDeclSpec().getStorageClassSpec() == DeclSpec::SCS_static)
        break;
      switch (cast<TagDecl>(SemaRef.CurContext)->getTagKind()) {
      case TTK_Enum: llvm_unreachable("unhandled tag kind");
      case TTK_Struct: Error = 1; /* Struct member */ break;
      case TTK_Union:  Error = 2; /* Union member */ break;
      case TTK_Class:  Error = 3; /* Class member */ break;
      case TTK_Interface: Error = 4; /* Interface member */ break;
      }
      break;
    case Declarator::CXXCatchContext:
    case Declarator::ObjCCatchContext:
      Error = 5; // Exception declaration
      break;
    case Declarator::TemplateParamContext:
      Error = 6; // Template parameter
      break;
    case Declarator::BlockLiteralContext:
      Error = 7; // Block literal
      break;
    case Declarator::TemplateTypeArgContext:
      Error = 8; // Template type argument
      break;
    case Declarator::AliasDeclContext:
    case Declarator::AliasTemplateContext:
      Error = 10; // Type alias
      break;
    case Declarator::TrailingReturnContext:
      if (!SemaRef.getLangOpts().CPlusPlus14)
        Error = 11; // Function return type
      break;
    case Declarator::ConversionIdContext:
      if (!SemaRef.getLangOpts().CPlusPlus14)
        Error = 12; // conversion-type-id
      break;
    case Declarator::TypeNameContext:
      Error = 13; // Generic
      break;
    case Declarator::FileContext:
    case Declarator::BlockContext:
    case Declarator::ForContext:
    case Declarator::ConditionContext:
    case Declarator::CXXNewContext:
      break;
    }

    if (D.getDeclSpec().getStorageClassSpec() == DeclSpec::SCS_typedef)
      Error = 9;

    // In Objective-C it is an error to use 'auto' on a function declarator.
    if (D.isFunctionDeclarator())
      Error = 11;

    // C++11 [dcl.spec.auto]p2: 'auto' is always fine if the declarator
    // contains a trailing return type. That is only legal at the outermost
    // level. Check all declarator chunks (outermost first) anyway, to give
    // better diagnostics.
    if (SemaRef.getLangOpts().CPlusPlus11 && Error != -1) {
      for (unsigned i = 0, e = D.getNumTypeObjects(); i != e; ++i) {
        unsigned chunkIndex = e - i - 1;
        state.setCurrentChunkIndex(chunkIndex);
        DeclaratorChunk &DeclType = D.getTypeObject(chunkIndex);
        if (DeclType.Kind == DeclaratorChunk::Function) {
          const DeclaratorChunk::FunctionTypeInfo &FTI = DeclType.Fun;
          if (FTI.hasTrailingReturnType()) {
            Error = -1;
            break;
          }
        }
      }
    }

    SourceRange AutoRange = D.getDeclSpec().getTypeSpecTypeLoc();
    if (D.getName().getKind() == UnqualifiedId::IK_ConversionFunctionId)
      AutoRange = D.getName().getSourceRange();

    if (Error != -1) {
      const bool IsDeclTypeAuto = 
          D.getDeclSpec().getTypeSpecType() == DeclSpec::TST_decltype_auto;
      SemaRef.Diag(AutoRange.getBegin(), diag::err_auto_not_allowed)
        << IsDeclTypeAuto << Error << AutoRange;
      T = SemaRef.Context.IntTy;
      D.setInvalidType(true);
    } else
      SemaRef.Diag(AutoRange.getBegin(),
                   diag::warn_cxx98_compat_auto_type_specifier)
        << AutoRange;
  }

  if (SemaRef.getLangOpts().CPlusPlus &&
      OwnedTagDecl && OwnedTagDecl->isCompleteDefinition()) {
    // Check the contexts where C++ forbids the declaration of a new class
    // or enumeration in a type-specifier-seq.
    switch (D.getContext()) {
    case Declarator::TrailingReturnContext:
      // Class and enumeration definitions are syntactically not allowed in
      // trailing return types.
      llvm_unreachable("parser should not have allowed this");
      break;
    case Declarator::FileContext:
    case Declarator::MemberContext:
    case Declarator::BlockContext:
    case Declarator::ForContext:
    case Declarator::BlockLiteralContext:
    case Declarator::LambdaExprContext:
      // C++11 [dcl.type]p3:
      //   A type-specifier-seq shall not define a class or enumeration unless
      //   it appears in the type-id of an alias-declaration (7.1.3) that is not
      //   the declaration of a template-declaration.
    case Declarator::AliasDeclContext:
      break;
    case Declarator::AliasTemplateContext:
      SemaRef.Diag(OwnedTagDecl->getLocation(),
             diag::err_type_defined_in_alias_template)
        << SemaRef.Context.getTypeDeclType(OwnedTagDecl);
      D.setInvalidType(true);
      break;
    case Declarator::TypeNameContext:
    case Declarator::ConversionIdContext:
    case Declarator::TemplateParamContext:
    case Declarator::CXXNewContext:
    case Declarator::CXXCatchContext:
    case Declarator::ObjCCatchContext:
    case Declarator::TemplateTypeArgContext:
      SemaRef.Diag(OwnedTagDecl->getLocation(),
             diag::err_type_defined_in_type_specifier)
        << SemaRef.Context.getTypeDeclType(OwnedTagDecl);
      D.setInvalidType(true);
      break;
    case Declarator::PrototypeContext:
    case Declarator::LambdaExprParameterContext:
    case Declarator::ObjCParameterContext:
    case Declarator::ObjCResultContext:
    case Declarator::KNRTypeListContext:
      // C++ [dcl.fct]p6:
      //   Types shall not be defined in return or parameter types.
      SemaRef.Diag(OwnedTagDecl->getLocation(),
                   diag::err_type_defined_in_param_type)
        << SemaRef.Context.getTypeDeclType(OwnedTagDecl);
      D.setInvalidType(true);
      break;
    case Declarator::ConditionContext:
      // C++ 6.4p2:
      // The type-specifier-seq shall not contain typedef and shall not declare
      // a new class or enumeration.
      SemaRef.Diag(OwnedTagDecl->getLocation(),
                   diag::err_type_defined_in_condition);
      D.setInvalidType(true);
      break;
    }
  }

  assert(!T.isNull() && "This function should not return a null type");
  return T;
}

/// Produce an appropriate diagnostic for an ambiguity between a function
/// declarator and a C++ direct-initializer.
static void warnAboutAmbiguousFunction(Sema &S, Declarator &D,
                                       DeclaratorChunk &DeclType, QualType RT) {
  const DeclaratorChunk::FunctionTypeInfo &FTI = DeclType.Fun;
  assert(FTI.isAmbiguous && "no direct-initializer / function ambiguity");

  // If the return type is void there is no ambiguity.
  if (RT->isVoidType())
    return;

  // An initializer for a non-class type can have at most one argument.
  if (!RT->isRecordType() && FTI.NumParams > 1)
    return;

  // An initializer for a reference must have exactly one argument.
  if (RT->isReferenceType() && FTI.NumParams != 1)
    return;

  // Only warn if this declarator is declaring a function at block scope, and
  // doesn't have a storage class (such as 'extern') specified.
  if (!D.isFunctionDeclarator() ||
      D.getFunctionDefinitionKind() != FDK_Declaration ||
      !S.CurContext->isFunctionOrMethod() ||
      D.getDeclSpec().getStorageClassSpec()
        != DeclSpec::SCS_unspecified)
    return;

  // Inside a condition, a direct initializer is not permitted. We allow one to
  // be parsed in order to give better diagnostics in condition parsing.
  if (D.getContext() == Declarator::ConditionContext)
    return;

  SourceRange ParenRange(DeclType.Loc, DeclType.EndLoc);

  S.Diag(DeclType.Loc,
         FTI.NumParams ? diag::warn_parens_disambiguated_as_function_declaration
                       : diag::warn_empty_parens_are_function_decl)
      << ParenRange;

  // If the declaration looks like:
  //   T var1,
  //   f();
  // and name lookup finds a function named 'f', then the ',' was
  // probably intended to be a ';'.
  if (!D.isFirstDeclarator() && D.getIdentifier()) {
    FullSourceLoc Comma(D.getCommaLoc(), S.SourceMgr);
    FullSourceLoc Name(D.getIdentifierLoc(), S.SourceMgr);
    if (Comma.getFileID() != Name.getFileID() ||
        Comma.getSpellingLineNumber() != Name.getSpellingLineNumber()) {
      LookupResult Result(S, D.getIdentifier(), SourceLocation(),
                          Sema::LookupOrdinaryName);
      if (S.LookupName(Result, S.getCurScope()))
        S.Diag(D.getCommaLoc(), diag::note_empty_parens_function_call)
          << FixItHint::CreateReplacement(D.getCommaLoc(), ";")
          << D.getIdentifier();
    }
  }

  if (FTI.NumParams > 0) {
    // For a declaration with parameters, eg. "T var(T());", suggest adding
    // parens around the first parameter to turn the declaration into a
    // variable declaration.
    SourceRange Range = FTI.Params[0].Param->getSourceRange();
    SourceLocation B = Range.getBegin();
    SourceLocation E = S.getLocForEndOfToken(Range.getEnd());
    // FIXME: Maybe we should suggest adding braces instead of parens
    // in C++11 for classes that don't have an initializer_list constructor.
    S.Diag(B, diag::note_additional_parens_for_variable_declaration)
      << FixItHint::CreateInsertion(B, "(")
      << FixItHint::CreateInsertion(E, ")");
  } else {
    // For a declaration without parameters, eg. "T var();", suggest replacing
    // the parens with an initializer to turn the declaration into a variable
    // declaration.
    const CXXRecordDecl *RD = RT->getAsCXXRecordDecl();

    // Empty parens mean value-initialization, and no parens mean
    // default initialization. These are equivalent if the default
    // constructor is user-provided or if zero-initialization is a
    // no-op.
    if (RD && RD->hasDefinition() &&
        (RD->isEmpty() || RD->hasUserProvidedDefaultConstructor()))
      S.Diag(DeclType.Loc, diag::note_empty_parens_default_ctor)
        << FixItHint::CreateRemoval(ParenRange);
    else {
      std::string Init =
          S.getFixItZeroInitializerForType(RT, ParenRange.getBegin());
      if (Init.empty() && S.LangOpts.CPlusPlus11)
        Init = "{}";
      if (!Init.empty())
        S.Diag(DeclType.Loc, diag::note_empty_parens_zero_initialize)
          << FixItHint::CreateReplacement(ParenRange, Init);
    }
  }
}

/// Helper for figuring out the default CC for a function declarator type.  If
/// this is the outermost chunk, then we can determine the CC from the
/// declarator context.  If not, then this could be either a member function
/// type or normal function type.
static CallingConv
getCCForDeclaratorChunk(Sema &S, Declarator &D,
                        const DeclaratorChunk::FunctionTypeInfo &FTI,
                        unsigned ChunkIndex) {
  assert(D.getTypeObject(ChunkIndex).Kind == DeclaratorChunk::Function);

  bool IsCXXInstanceMethod = false;

  if (S.getLangOpts().CPlusPlus) {
    // Look inwards through parentheses to see if this chunk will form a
    // member pointer type or if we're the declarator.  Any type attributes
    // between here and there will override the CC we choose here.
    unsigned I = ChunkIndex;
    bool FoundNonParen = false;
    while (I && !FoundNonParen) {
      --I;
      if (D.getTypeObject(I).Kind != DeclaratorChunk::Paren)
        FoundNonParen = true;
    }

    if (FoundNonParen) {
      // If we're not the declarator, we're a regular function type unless we're
      // in a member pointer.
      IsCXXInstanceMethod =
          D.getTypeObject(I).Kind == DeclaratorChunk::MemberPointer;
    } else if (D.getContext() == Declarator::LambdaExprContext) {
      // This can only be a call operator for a lambda, which is an instance
      // method.
      IsCXXInstanceMethod = true;
    } else {
      // We're the innermost decl chunk, so must be a function declarator.
      assert(D.isFunctionDeclarator());

      // If we're inside a record, we're declaring a method, but it could be
      // explicitly or implicitly static.
      IsCXXInstanceMethod =
          D.isFirstDeclarationOfMember() &&
          D.getDeclSpec().getStorageClassSpec() != DeclSpec::SCS_typedef &&
          !D.isStaticMember();
    }
  }

  CallingConv CC = S.Context.getDefaultCallingConvention(FTI.isVariadic,
                                                         IsCXXInstanceMethod);

  // Attribute AT_OpenCLKernel affects the calling convention only on
  // the SPIR target, hence it cannot be treated as a calling
  // convention attribute. This is the simplest place to infer
  // "spir_kernel" for OpenCL kernels on SPIR.
  if (CC == CC_SpirFunction) {
    for (const AttributeList *Attr = D.getDeclSpec().getAttributes().getList();
         Attr; Attr = Attr->getNext()) {
      if (Attr->getKind() == AttributeList::AT_OpenCLKernel) {
        CC = CC_SpirKernel;
        break;
      }
    }
  }

  return CC;
}

namespace {
  /// A simple notion of pointer kinds, which matches up with the various
  /// pointer declarators.
  enum class SimplePointerKind {
    Pointer,
    BlockPointer,
    MemberPointer,
  };
}

IdentifierInfo *Sema::getNullabilityKeyword(NullabilityKind nullability) {
  switch (nullability) {
  case NullabilityKind::NonNull:
    if (!Ident__Nonnull)
      Ident__Nonnull = PP.getIdentifierInfo("_Nonnull");
    return Ident__Nonnull;

  case NullabilityKind::Nullable:
    if (!Ident__Nullable)
      Ident__Nullable = PP.getIdentifierInfo("_Nullable");
    return Ident__Nullable;

  case NullabilityKind::Unspecified:
    if (!Ident__Null_unspecified)
      Ident__Null_unspecified = PP.getIdentifierInfo("_Null_unspecified");
    return Ident__Null_unspecified;
  }
  llvm_unreachable("Unknown nullability kind.");
}

/// Retrieve the identifier "NSError".
IdentifierInfo *Sema::getNSErrorIdent() {
  if (!Ident_NSError)
    Ident_NSError = PP.getIdentifierInfo("NSError");

  return Ident_NSError;
}

/// Check whether there is a nullability attribute of any kind in the given
/// attribute list.
static bool hasNullabilityAttr(const AttributeList *attrs) {
  for (const AttributeList *attr = attrs; attr;
       attr = attr->getNext()) {
    if (attr->getKind() == AttributeList::AT_TypeNonNull ||
        attr->getKind() == AttributeList::AT_TypeNullable ||
        attr->getKind() == AttributeList::AT_TypeNullUnspecified)
      return true;
  }

  return false;
}

namespace {
  /// Describes the kind of a pointer a declarator describes.
  enum class PointerDeclaratorKind {
    // Not a pointer.
    NonPointer,
    // Single-level pointer.
    SingleLevelPointer,
    // Multi-level pointer (of any pointer kind).
    MultiLevelPointer,
    // CFFooRef*
    MaybePointerToCFRef,
    // CFErrorRef*
    CFErrorRefPointer,
    // NSError**
    NSErrorPointerPointer,
  };
}

/// Classify the given declarator, whose type-specified is \c type, based on
/// what kind of pointer it refers to.
///
/// This is used to determine the default nullability.
static PointerDeclaratorKind classifyPointerDeclarator(Sema &S,
                                                       QualType type,
                                                       Declarator &declarator) {
  unsigned numNormalPointers = 0;

  // For any dependent type, we consider it a non-pointer.
  if (type->isDependentType())
    return PointerDeclaratorKind::NonPointer;

  // Look through the declarator chunks to identify pointers.
  for (unsigned i = 0, n = declarator.getNumTypeObjects(); i != n; ++i) {
    DeclaratorChunk &chunk = declarator.getTypeObject(i);
    switch (chunk.Kind) {
    case DeclaratorChunk::Array:
    case DeclaratorChunk::Function:
      break;

    case DeclaratorChunk::BlockPointer:
    case DeclaratorChunk::MemberPointer:
      return numNormalPointers > 0 ? PointerDeclaratorKind::MultiLevelPointer
                                   : PointerDeclaratorKind::SingleLevelPointer;

    case DeclaratorChunk::Paren:
    case DeclaratorChunk::Reference:
      continue;

    case DeclaratorChunk::Pointer:
      ++numNormalPointers;
      if (numNormalPointers > 2)
        return PointerDeclaratorKind::MultiLevelPointer;
      continue;
    }
  }

  // Then, dig into the type specifier itself.
  unsigned numTypeSpecifierPointers = 0;
  do {
    // Decompose normal pointers.
    if (auto ptrType = type->getAs<PointerType>()) {
      ++numNormalPointers;

      if (numNormalPointers > 2)
        return PointerDeclaratorKind::MultiLevelPointer;

      type = ptrType->getPointeeType();
      ++numTypeSpecifierPointers;
      continue;
    }

    // Decompose block pointers.
    if (type->getAs<BlockPointerType>()) {
      return numNormalPointers > 0 ? PointerDeclaratorKind::MultiLevelPointer
                                   : PointerDeclaratorKind::SingleLevelPointer;
    }

    // Decompose member pointers.
    if (type->getAs<MemberPointerType>()) {
      return numNormalPointers > 0 ? PointerDeclaratorKind::MultiLevelPointer
                                   : PointerDeclaratorKind::SingleLevelPointer;
    }

    // Look at Objective-C object pointers.
    if (auto objcObjectPtr = type->getAs<ObjCObjectPointerType>()) {
      ++numNormalPointers;
      ++numTypeSpecifierPointers;

      // If this is NSError**, report that.
      if (auto objcClassDecl = objcObjectPtr->getInterfaceDecl()) {
        if (objcClassDecl->getIdentifier() == S.getNSErrorIdent() &&
            numNormalPointers == 2 && numTypeSpecifierPointers < 2) {
          return PointerDeclaratorKind::NSErrorPointerPointer;
        }
      }

      break;
    }

    // Look at Objective-C class types.
    if (auto objcClass = type->getAs<ObjCInterfaceType>()) {
      if (objcClass->getInterface()->getIdentifier() == S.getNSErrorIdent()) {
        if (numNormalPointers == 2 && numTypeSpecifierPointers < 2)
          return PointerDeclaratorKind::NSErrorPointerPointer;;
      }

      break;
    }

    // If at this point we haven't seen a pointer, we won't see one.
    if (numNormalPointers == 0)
      return PointerDeclaratorKind::NonPointer;

    if (auto recordType = type->getAs<RecordType>()) {
      RecordDecl *recordDecl = recordType->getDecl();

      bool isCFError = false;
      if (S.CFError) {
        // If we already know about CFError, test it directly.
        isCFError = (S.CFError == recordDecl);
      } else {
        // Check whether this is CFError, which we identify based on its bridge
        // to NSError.
        if (recordDecl->getTagKind() == TTK_Struct && numNormalPointers > 0) {
          if (auto bridgeAttr = recordDecl->getAttr<ObjCBridgeAttr>()) {
            if (bridgeAttr->getBridgedType() == S.getNSErrorIdent()) {
              S.CFError = recordDecl;
              isCFError = true;
            }
          }
        }
      }

      // If this is CFErrorRef*, report it as such.
      if (isCFError && numNormalPointers == 2 && numTypeSpecifierPointers < 2) {
        return PointerDeclaratorKind::CFErrorRefPointer;
      }
      break;
    }

    break;
  } while (true);


  switch (numNormalPointers) {
  case 0:
    return PointerDeclaratorKind::NonPointer;

  case 1:
    return PointerDeclaratorKind::SingleLevelPointer;

  case 2:
    return PointerDeclaratorKind::MaybePointerToCFRef;

  default:
    return PointerDeclaratorKind::MultiLevelPointer;
  }
}

static FileID getNullabilityCompletenessCheckFileID(Sema &S,
                                                    SourceLocation loc) {
  // If we're anywhere in a function, method, or closure context, don't perform
  // completeness checks.
  for (DeclContext *ctx = S.CurContext; ctx; ctx = ctx->getParent()) {
    if (ctx->isFunctionOrMethod())
      return FileID();

    if (ctx->isFileContext())
      break;
  }

  // We only care about the expansion location.
  loc = S.SourceMgr.getExpansionLoc(loc);
  FileID file = S.SourceMgr.getFileID(loc);
  if (file.isInvalid())
    return FileID();

  // Retrieve file information.
  bool invalid = false;
  const SrcMgr::SLocEntry &sloc = S.SourceMgr.getSLocEntry(file, &invalid);
  if (invalid || !sloc.isFile())
    return FileID();

  // We don't want to perform completeness checks on the main file or in
  // system headers.
  const SrcMgr::FileInfo &fileInfo = sloc.getFile();
  if (fileInfo.getIncludeLoc().isInvalid())
    return FileID();
  if (fileInfo.getFileCharacteristic() != SrcMgr::C_User &&
      S.Diags.getSuppressSystemWarnings()) {
    return FileID();
  }

  return file;
}

/// Check for consistent use of nullability.
static void checkNullabilityConsistency(TypeProcessingState &state,
                                        SimplePointerKind pointerKind,
                                        SourceLocation pointerLoc) {
  Sema &S = state.getSema();

  // Determine which file we're performing consistency checking for.
  FileID file = getNullabilityCompletenessCheckFileID(S, pointerLoc);
  if (file.isInvalid())
    return;

  // If we haven't seen any type nullability in this file, we won't warn now
  // about anything.
  FileNullability &fileNullability = S.NullabilityMap[file];
  if (!fileNullability.SawTypeNullability) {
    // If this is the first pointer declarator in the file, record it.
    if (fileNullability.PointerLoc.isInvalid() &&
        !S.Context.getDiagnostics().isIgnored(diag::warn_nullability_missing,
                                              pointerLoc)) {
      fileNullability.PointerLoc = pointerLoc;
      fileNullability.PointerKind = static_cast<unsigned>(pointerKind);
    }

    return;
  }

  // Complain about missing nullability.
  S.Diag(pointerLoc, diag::warn_nullability_missing)
    << static_cast<unsigned>(pointerKind);
}

static TypeSourceInfo *GetFullTypeForDeclarator(TypeProcessingState &state,
                                                QualType declSpecType,
                                                TypeSourceInfo *TInfo) {
  // The TypeSourceInfo that this function returns will not be a null type.
  // If there is an error, this function will fill in a dummy type as fallback.
  QualType T = declSpecType;
  Declarator &D = state.getDeclarator();
  Sema &S = state.getSema();
  ASTContext &Context = S.Context;
  const LangOptions &LangOpts = S.getLangOpts();

  // The name we're declaring, if any.
  DeclarationName Name;
  if (D.getIdentifier())
    Name = D.getIdentifier();

  // Does this declaration declare a typedef-name?
  bool IsTypedefName =
    D.getDeclSpec().getStorageClassSpec() == DeclSpec::SCS_typedef ||
    D.getContext() == Declarator::AliasDeclContext ||
    D.getContext() == Declarator::AliasTemplateContext;

  // Does T refer to a function type with a cv-qualifier or a ref-qualifier?
  bool IsQualifiedFunction = T->isFunctionProtoType() &&
      (T->castAs<FunctionProtoType>()->getTypeQuals() != 0 ||
       T->castAs<FunctionProtoType>()->getRefQualifier() != RQ_None);

  // If T is 'decltype(auto)', the only declarators we can have are parens
  // and at most one function declarator if this is a function declaration.
  if (const AutoType *AT = T->getAs<AutoType>()) {
    if (AT->isDecltypeAuto()) {
      for (unsigned I = 0, E = D.getNumTypeObjects(); I != E; ++I) {
        unsigned Index = E - I - 1;
        DeclaratorChunk &DeclChunk = D.getTypeObject(Index);
        unsigned DiagId = diag::err_decltype_auto_compound_type;
        unsigned DiagKind = 0;
        switch (DeclChunk.Kind) {
        case DeclaratorChunk::Paren:
          continue;
        case DeclaratorChunk::Function: {
          unsigned FnIndex;
          if (D.isFunctionDeclarationContext() &&
              D.isFunctionDeclarator(FnIndex) && FnIndex == Index)
            continue;
          DiagId = diag::err_decltype_auto_function_declarator_not_declaration;
          break;
        }
        case DeclaratorChunk::Pointer:
        case DeclaratorChunk::BlockPointer:
        case DeclaratorChunk::MemberPointer:
          DiagKind = 0;
          break;
        case DeclaratorChunk::Reference:
          DiagKind = 1;
          break;
        case DeclaratorChunk::Array:
          DiagKind = 2;
          break;
        }

        S.Diag(DeclChunk.Loc, DiagId) << DiagKind;
        D.setInvalidType(true);
        break;
      }
    }
  }

  // Determine whether we should infer _Nonnull on pointer types.
  Optional<NullabilityKind> inferNullability;
  bool inferNullabilityCS = false;
  bool inferNullabilityInnerOnly = false;
  bool inferNullabilityInnerOnlyComplete = false;

  // Are we in an assume-nonnull region?
  bool inAssumeNonNullRegion = false;
  if (S.PP.getPragmaAssumeNonNullLoc().isValid() &&
      !state.getDeclarator().isObjCWeakProperty() &&
      !S.deduceWeakPropertyFromType(T)) {
    inAssumeNonNullRegion = true;
    // Determine which file we saw the assume-nonnull region in.
    FileID file = getNullabilityCompletenessCheckFileID(
                    S, S.PP.getPragmaAssumeNonNullLoc());
    if (!file.isInvalid()) {
      FileNullability &fileNullability = S.NullabilityMap[file];

      // If we haven't seen any type nullability before, now we have.
      if (!fileNullability.SawTypeNullability) {
        if (fileNullability.PointerLoc.isValid()) {
          S.Diag(fileNullability.PointerLoc, diag::warn_nullability_missing)
            << static_cast<unsigned>(fileNullability.PointerKind);
        }

        fileNullability.SawTypeNullability = true;
      }
    }
  }

  // Whether to complain about missing nullability specifiers or not.
  enum {
    /// Never complain.
    CAMN_No,
    /// Complain on the inner pointers (but not the outermost
    /// pointer).
    CAMN_InnerPointers,
    /// Complain about any pointers that don't have nullability
    /// specified or inferred.
    CAMN_Yes
  } complainAboutMissingNullability = CAMN_No;
  unsigned NumPointersRemaining = 0;

  if (IsTypedefName) {
    // For typedefs, we do not infer any nullability (the default),
    // and we only complain about missing nullability specifiers on
    // inner pointers.
    complainAboutMissingNullability = CAMN_InnerPointers;

    if (T->canHaveNullability() && !T->getNullability(S.Context)) {
      ++NumPointersRemaining;
    }

    for (unsigned i = 0, n = D.getNumTypeObjects(); i != n; ++i) {
      DeclaratorChunk &chunk = D.getTypeObject(i);
      switch (chunk.Kind) {
      case DeclaratorChunk::Array:
      case DeclaratorChunk::Function:
        break;

      case DeclaratorChunk::BlockPointer:
      case DeclaratorChunk::MemberPointer:
        ++NumPointersRemaining;
        break;

      case DeclaratorChunk::Paren:
      case DeclaratorChunk::Reference:
        continue;

      case DeclaratorChunk::Pointer:
        ++NumPointersRemaining;
        continue;
      }
    }
  } else {
    bool isFunctionOrMethod = false;
    switch (auto context = state.getDeclarator().getContext()) {
    case Declarator::ObjCParameterContext:
    case Declarator::ObjCResultContext:
    case Declarator::PrototypeContext:
    case Declarator::TrailingReturnContext:
      isFunctionOrMethod = true;
      LLVM_FALLTHROUGH; // HLSL Change

    case Declarator::MemberContext:
      if (state.getDeclarator().isObjCIvar() && !isFunctionOrMethod) {
        complainAboutMissingNullability = CAMN_No;
        break;
      }
      LLVM_FALLTHROUGH; // HLSL Change

    case Declarator::FileContext:
    case Declarator::KNRTypeListContext:
      complainAboutMissingNullability = CAMN_Yes;

      // Nullability inference depends on the type and declarator.
      switch (classifyPointerDeclarator(S, T, D)) {
      case PointerDeclaratorKind::NonPointer:
      case PointerDeclaratorKind::MultiLevelPointer:
        // Cannot infer nullability.
        break;

      case PointerDeclaratorKind::SingleLevelPointer:
        // Infer _Nonnull if we are in an assumes-nonnull region.
        if (inAssumeNonNullRegion) {
          inferNullability = NullabilityKind::NonNull;
          inferNullabilityCS = (context == Declarator::ObjCParameterContext ||
                                context == Declarator::ObjCResultContext);
        }
        break;

      case PointerDeclaratorKind::CFErrorRefPointer:
      case PointerDeclaratorKind::NSErrorPointerPointer:
        // Within a function or method signature, infer _Nullable at both
        // levels.
        if (isFunctionOrMethod && inAssumeNonNullRegion)
          inferNullability = NullabilityKind::Nullable;
        break;

      case PointerDeclaratorKind::MaybePointerToCFRef:
        if (isFunctionOrMethod) {
          // On pointer-to-pointer parameters marked cf_returns_retained or
          // cf_returns_not_retained, if the outer pointer is explicit then
          // infer the inner pointer as _Nullable.
          auto hasCFReturnsAttr = [](const AttributeList *NextAttr) -> bool {
            while (NextAttr) {
              if (NextAttr->getKind() == AttributeList::AT_CFReturnsRetained ||
                  NextAttr->getKind() == AttributeList::AT_CFReturnsNotRetained)
                return true;
              NextAttr = NextAttr->getNext();
            }
            return false;
          };
          if (const auto *InnermostChunk = D.getInnermostNonParenChunk()) {
            if (hasCFReturnsAttr(D.getAttributes()) ||
                hasCFReturnsAttr(InnermostChunk->getAttrs()) ||
                hasCFReturnsAttr(D.getDeclSpec().getAttributes().getList())) {
              inferNullability = NullabilityKind::Nullable;
              inferNullabilityInnerOnly = true;
            }
          }
        }
        break;
      }
      break;

    case Declarator::ConversionIdContext:
      complainAboutMissingNullability = CAMN_Yes;
      break;

    case Declarator::AliasDeclContext:
    case Declarator::AliasTemplateContext:
    case Declarator::BlockContext:
    case Declarator::BlockLiteralContext:
    case Declarator::ConditionContext:
    case Declarator::CXXCatchContext:
    case Declarator::CXXNewContext:
    case Declarator::ForContext:
    case Declarator::LambdaExprContext:
    case Declarator::LambdaExprParameterContext:
    case Declarator::ObjCCatchContext:
    case Declarator::TemplateParamContext:
    case Declarator::TemplateTypeArgContext:
    case Declarator::TypeNameContext:
      // Don't infer in these contexts.
      break;
    }
  }

  // Local function that checks the nullability for a given pointer declarator.
  // Returns true if _Nonnull was inferred.
  auto inferPointerNullability = [&](SimplePointerKind pointerKind,
                                     SourceLocation pointerLoc,
                                     AttributeList *&attrs) -> AttributeList * {
    // We've seen a pointer.
    if (NumPointersRemaining > 0)
      --NumPointersRemaining;

    // If a nullability attribute is present, there's nothing to do.
    if (hasNullabilityAttr(attrs))
      return nullptr;

    // If we're supposed to infer nullability, do so now.
    if (inferNullability && !inferNullabilityInnerOnlyComplete) {
      AttributeList::Syntax syntax
        = inferNullabilityCS ? AttributeList::AS_ContextSensitiveKeyword
                             : AttributeList::AS_Keyword;
      AttributeList *nullabilityAttr = state.getDeclarator().getAttributePool()
                                         .create(
                                           S.getNullabilityKeyword(
                                             *inferNullability),
                                           SourceRange(pointerLoc),
                                           nullptr, SourceLocation(),
                                           nullptr, 0, syntax);

      spliceAttrIntoList(*nullabilityAttr, attrs);

      if (inferNullabilityCS) {
        state.getDeclarator().getMutableDeclSpec().getObjCQualifiers()
          ->setObjCDeclQualifier(ObjCDeclSpec::DQ_CSNullability);
      }

      if (inferNullabilityInnerOnly)
        inferNullabilityInnerOnlyComplete = true;
      return nullabilityAttr;
    }

    // If we're supposed to complain about missing nullability, do so
    // now if it's truly missing.
    switch (complainAboutMissingNullability) {
    case CAMN_No:
      break;

    case CAMN_InnerPointers:
      if (NumPointersRemaining == 0)
        break;
      LLVM_FALLTHROUGH; // HLSL Change

    case CAMN_Yes:
      checkNullabilityConsistency(state, pointerKind, pointerLoc);
    }
    return nullptr;
  };

  // If the type itself could have nullability but does not, infer pointer
  // nullability and perform consistency checking.
  if (T->canHaveNullability() && S.ActiveTemplateInstantiations.empty() &&
      !T->getNullability(S.Context)) {
    SimplePointerKind pointerKind = SimplePointerKind::Pointer;
    if (T->isBlockPointerType())
      pointerKind = SimplePointerKind::BlockPointer;
    else if (T->isMemberPointerType())
      pointerKind = SimplePointerKind::MemberPointer;

    if (auto *attr = inferPointerNullability(
                       pointerKind, D.getDeclSpec().getTypeSpecTypeLoc(),
                       D.getMutableDeclSpec().getAttributes().getListRef())) {
      T = Context.getAttributedType(
            AttributedType::getNullabilityAttrKind(*inferNullability), T, T);
      attr->setUsedAsTypeAttr();
    }
  }

  // Walk the DeclTypeInfo, building the recursive type as we go.
  // DeclTypeInfos are ordered from the identifier out, which is
  // opposite of what we want :).
  for (unsigned i = 0, e = D.getNumTypeObjects(); i != e; ++i) {
    unsigned chunkIndex = e - i - 1;
    state.setCurrentChunkIndex(chunkIndex);
    DeclaratorChunk &DeclType = D.getTypeObject(chunkIndex);
    IsQualifiedFunction &= DeclType.Kind == DeclaratorChunk::Paren;
    switch (DeclType.Kind) {
    case DeclaratorChunk::Paren:
      T = S.BuildParenType(T);
      break;
    case DeclaratorChunk::BlockPointer:
      // If blocks are disabled, emit an error.
      if (!LangOpts.Blocks)
        S.Diag(DeclType.Loc, diag::err_blocks_disable);

      // Handle pointer nullability.
      inferPointerNullability(SimplePointerKind::BlockPointer,
                              DeclType.Loc, DeclType.getAttrListRef());

      T = S.BuildBlockPointerType(T, D.getIdentifierLoc(), Name);
      if (DeclType.Cls.TypeQuals)
        T = S.BuildQualifiedType(T, DeclType.Loc, DeclType.Cls.TypeQuals);
      break;
    case DeclaratorChunk::Pointer:
      // Verify that we're not building a pointer to pointer to function with
      // exception specification.
      if (LangOpts.CPlusPlus && S.CheckDistantExceptionSpec(T)) {
        S.Diag(D.getIdentifierLoc(), diag::err_distant_exception_spec);
        D.setInvalidType(true);
        // Build the type anyway.
      }

      // Handle pointer nullability
      inferPointerNullability(SimplePointerKind::Pointer, DeclType.Loc,
                              DeclType.getAttrListRef());

      if (LangOpts.ObjC1 && T->getAs<ObjCObjectType>()) {
        T = Context.getObjCObjectPointerType(T);
        if (DeclType.Ptr.TypeQuals)
          T = S.BuildQualifiedType(T, DeclType.Loc, DeclType.Ptr.TypeQuals);
        break;
      }
      T = S.BuildPointerType(T, DeclType.Loc, Name);
      if (DeclType.Ptr.TypeQuals)
        T = S.BuildQualifiedType(T, DeclType.Loc, DeclType.Ptr.TypeQuals);

      break;
    case DeclaratorChunk::Reference: {
      // Verify that we're not building a reference to pointer to function with
      // exception specification.
      if (LangOpts.CPlusPlus && S.CheckDistantExceptionSpec(T)) {
        S.Diag(D.getIdentifierLoc(), diag::err_distant_exception_spec);
        D.setInvalidType(true);
        // Build the type anyway.
      }
      T = S.BuildReferenceType(T, DeclType.Ref.LValueRef, DeclType.Loc, Name);

      if (DeclType.Ref.HasRestrict)
        T = S.BuildQualifiedType(T, DeclType.Loc, Qualifiers::Restrict);
      break;
    }
    case DeclaratorChunk::Array: {
      // Verify that we're not building an array of pointers to function with
      // exception specification.
      if (LangOpts.CPlusPlus && S.CheckDistantExceptionSpec(T)) {
        S.Diag(D.getIdentifierLoc(), diag::err_distant_exception_spec);
        D.setInvalidType(true);
        // Build the type anyway.
      }
      DeclaratorChunk::ArrayTypeInfo &ATI = DeclType.Arr;
      Expr *ArraySize = static_cast<Expr*>(ATI.NumElts);
      ArrayType::ArraySizeModifier ASM;
      if (ATI.isStar)
        ASM = ArrayType::Star;
      else if (ATI.hasStatic)
        ASM = ArrayType::Static;
      else
        ASM = ArrayType::Normal;
      if (ASM == ArrayType::Star && !D.isPrototypeContext()) {
        // FIXME: This check isn't quite right: it allows star in prototypes
        // for function definitions, and disallows some edge cases detailed
        // in http://gcc.gnu.org/ml/gcc-patches/2009-02/msg00133.html
        if (!S.getLangOpts().HLSL) // HLSL Change: this already errors at parse time
        S.Diag(DeclType.Loc, diag::err_array_star_outside_prototype);
        ASM = ArrayType::Normal;
        D.setInvalidType(true);
      }

      // C99 6.7.5.2p1: The optional type qualifiers and the keyword static
      // shall appear only in a declaration of a function parameter with an
      // array type, ...
      if (!S.getLangOpts().HLSL) // HLSL Change - these already error at parse time
      if (ASM == ArrayType::Static || ATI.TypeQuals) {
        if (!(D.isPrototypeContext() ||
              D.getContext() == Declarator::KNRTypeListContext)) {
          S.Diag(DeclType.Loc, diag::err_array_static_outside_prototype) <<
              (ASM == ArrayType::Static ? "'static'" : "type qualifier");
          // Remove the 'static' and the type qualifiers.
          if (ASM == ArrayType::Static)
            ASM = ArrayType::Normal;
          ATI.TypeQuals = 0;
          D.setInvalidType(true);
        }

        // C99 6.7.5.2p1: ... and then only in the outermost array type
        // derivation.
        unsigned x = chunkIndex;
        while (x != 0) {
          // Walk outwards along the declarator chunks.
          x--;
          const DeclaratorChunk &DC = D.getTypeObject(x);
          switch (DC.Kind) {
          case DeclaratorChunk::Paren:
            continue;
          case DeclaratorChunk::Array:
          case DeclaratorChunk::Pointer:
          case DeclaratorChunk::Reference:
          case DeclaratorChunk::MemberPointer:
            S.Diag(DeclType.Loc, diag::err_array_static_not_outermost) <<
              (ASM == ArrayType::Static ? "'static'" : "type qualifier");
            if (ASM == ArrayType::Static)
              ASM = ArrayType::Normal;
            ATI.TypeQuals = 0;
            D.setInvalidType(true);
            break;
          case DeclaratorChunk::Function:
          case DeclaratorChunk::BlockPointer:
            // These are invalid anyway, so just ignore.
            break;
          }
        }
      }
      const AutoType *AT = T->getContainedAutoType();
      // Allow arrays of auto if we are a generic lambda parameter.
      // i.e. [](auto (&array)[5]) { return array[0]; }; OK
      if (AT && D.getContext() != Declarator::LambdaExprParameterContext) {
        // We've already diagnosed this for decltype(auto).
        if (!AT->isDecltypeAuto())
          S.Diag(DeclType.Loc, diag::err_illegal_decl_array_of_auto)
            << getPrintableNameForEntity(Name) << T;
        T = QualType();
        break;
      }

      T = S.BuildArrayType(T, ASM, ArraySize, ATI.TypeQuals,
                           SourceRange(DeclType.Loc, DeclType.EndLoc), Name);
      break;
    }
    case DeclaratorChunk::Function: {
      // If the function declarator has a prototype (i.e. it is not () and
      // does not have a K&R-style identifier list), then the arguments are part
      // of the type, otherwise the argument list is ().
      const DeclaratorChunk::FunctionTypeInfo &FTI = DeclType.Fun;
      IsQualifiedFunction = FTI.TypeQuals || FTI.hasRefQualifier();

      // Check for auto functions and trailing return type and adjust the
      // return type accordingly.
      if (!D.isInvalidType()) {
        // trailing-return-type is only required if we're declaring a function,
        // and not, for instance, a pointer to a function.
        if (D.getDeclSpec().containsPlaceholderType() &&
            !FTI.hasTrailingReturnType() && chunkIndex == 0 &&
            !S.getLangOpts().CPlusPlus14) {
          S.Diag(D.getDeclSpec().getTypeSpecTypeLoc(),
                 D.getDeclSpec().getTypeSpecType() == DeclSpec::TST_auto
                     ? diag::err_auto_missing_trailing_return
                     : diag::err_deduced_return_type);
          T = Context.IntTy;
          D.setInvalidType(true);
        } else if (FTI.hasTrailingReturnType()) {
          // T must be exactly 'auto' at this point. See CWG issue 681.
          if (isa<ParenType>(T)) {
            S.Diag(D.getDeclSpec().getTypeSpecTypeLoc(),
                 diag::err_trailing_return_in_parens)
              << T << D.getDeclSpec().getSourceRange();
            D.setInvalidType(true);
          } else if (D.getContext() != Declarator::LambdaExprContext &&
                     (T.hasQualifiers() || !isa<AutoType>(T) ||
                      cast<AutoType>(T)->isDecltypeAuto())) {
            S.Diag(D.getDeclSpec().getTypeSpecTypeLoc(),
                 diag::err_trailing_return_without_auto)
              << T << D.getDeclSpec().getSourceRange();
            D.setInvalidType(true);
          }
          T = S.GetTypeFromParser(FTI.getTrailingReturnType(), &TInfo);
          if (T.isNull()) {
            // An error occurred parsing the trailing return type.
            T = Context.IntTy;
            D.setInvalidType(true);
          }
        }
      }

      // C99 6.7.5.3p1: The return type may not be a function or array type.
      // For conversion functions, we'll diagnose this particular error later.
      if (((T->isArrayType() && !S.getLangOpts().HLSL) || T->isFunctionType()) && // HLSL Change - Allow array return types
          (D.getName().getKind() != UnqualifiedId::IK_ConversionFunctionId)) {
        unsigned diagID = diag::err_func_returning_array_function;
        // Last processing chunk in block context means this function chunk
        // represents the block.
        if (chunkIndex == 0 &&
            D.getContext() == Declarator::BlockLiteralContext)
          diagID = diag::err_block_returning_array_function;
        S.Diag(DeclType.Loc, diagID) << T->isFunctionType() << T;
        T = Context.IntTy;
        D.setInvalidType(true);
      }

      // Do not allow returning half FP value.
      // FIXME: This really should be in BuildFunctionType.
      if (T->isHalfType()) {
        if (S.getLangOpts().OpenCL) {
          if (!S.getOpenCLOptions().cl_khr_fp16) {
            S.Diag(D.getIdentifierLoc(), diag::err_opencl_half_return) << T;
            D.setInvalidType(true);
          } 
        } else if (!S.getLangOpts().HalfArgsAndReturns) {
          S.Diag(D.getIdentifierLoc(),
            diag::err_parameters_retval_cannot_have_fp16_type) << 1;
          D.setInvalidType(true);
        }
      }

      // Methods cannot return interface types. All ObjC objects are
      // passed by reference.
      if (T->isObjCObjectType()) {
        SourceLocation DiagLoc, FixitLoc;
        if (TInfo) {
          DiagLoc = TInfo->getTypeLoc().getLocStart();
          FixitLoc = S.getLocForEndOfToken(TInfo->getTypeLoc().getLocEnd());
        } else {
          DiagLoc = D.getDeclSpec().getTypeSpecTypeLoc();
          FixitLoc = S.getLocForEndOfToken(D.getDeclSpec().getLocEnd());
        }
        S.Diag(DiagLoc, diag::err_object_cannot_be_passed_returned_by_value)
          << 0 << T
          << FixItHint::CreateInsertion(FixitLoc, "*");

        T = Context.getObjCObjectPointerType(T);
        if (TInfo) {
          TypeLocBuilder TLB;
          TLB.pushFullCopy(TInfo->getTypeLoc());
          ObjCObjectPointerTypeLoc TLoc = TLB.push<ObjCObjectPointerTypeLoc>(T);
          TLoc.setStarLoc(FixitLoc);
          TInfo = TLB.getTypeSourceInfo(Context, T);
        }

        D.setInvalidType(true);
      }

      // cv-qualifiers on return types are pointless except when the type is a
      // class type in C++.
      if ((T.getCVRQualifiers() || T->isAtomicType()) &&
          !(S.getLangOpts().CPlusPlus &&
            (T->isDependentType() || T->isRecordType()))) {
        if (T->isVoidType() && !S.getLangOpts().CPlusPlus &&
            D.getFunctionDefinitionKind() == FDK_Definition) {
          // [6.9.1/3] qualified void return is invalid on a C
          // function definition.  Apparently ok on declarations and
          // in C++ though (!)
          S.Diag(DeclType.Loc, diag::err_func_returning_qualified_void) << T;
        } else
          diagnoseRedundantReturnTypeQualifiers(S, T, D, chunkIndex);
      }

      // Objective-C ARC ownership qualifiers are ignored on the function
      // return type (by type canonicalization). Complain if this attribute
      // was written here.
      if (!LangOpts.HLSL && T.getQualifiers().hasObjCLifetime()) { // HLSL Change - no checks needed
        SourceLocation AttrLoc;
        if (chunkIndex + 1 < D.getNumTypeObjects()) {
          DeclaratorChunk ReturnTypeChunk = D.getTypeObject(chunkIndex + 1);
          for (const AttributeList *Attr = ReturnTypeChunk.getAttrs();
               Attr; Attr = Attr->getNext()) {
            if (Attr->getKind() == AttributeList::AT_ObjCOwnership) {
              AttrLoc = Attr->getLoc();
              break;
            }
          }
        }
        if (AttrLoc.isInvalid()) {
          for (const AttributeList *Attr
                 = D.getDeclSpec().getAttributes().getList();
               Attr; Attr = Attr->getNext()) {
            if (Attr->getKind() == AttributeList::AT_ObjCOwnership) {
              AttrLoc = Attr->getLoc();
              break;
            }
          }
        }

        if (AttrLoc.isValid()) {
          // The ownership attributes are almost always written via
          // the predefined
          // __strong/__weak/__autoreleasing/__unsafe_unretained.
          if (AttrLoc.isMacroID())
            AttrLoc = S.SourceMgr.getImmediateExpansionRange(AttrLoc).first;

          S.Diag(AttrLoc, diag::warn_arc_lifetime_result_type)
            << T.getQualifiers().getObjCLifetime();
        }
      }

      if (LangOpts.CPlusPlus && D.getDeclSpec().hasTagDefinition()) {
        // C++ [dcl.fct]p6:
        //   Types shall not be defined in return or parameter types.
        TagDecl *Tag = cast<TagDecl>(D.getDeclSpec().getRepAsDecl());
        S.Diag(Tag->getLocation(), diag::err_type_defined_in_result_type)
          << Context.getTypeDeclType(Tag);
      }

      // Exception specs are not allowed in typedefs. Complain, but add it
      // anyway.
      if (IsTypedefName && FTI.getExceptionSpecType())
        S.Diag(FTI.getExceptionSpecLoc(), diag::err_exception_spec_in_typedef)
          << (D.getContext() == Declarator::AliasDeclContext ||
              D.getContext() == Declarator::AliasTemplateContext);

      // If we see "T var();" or "T var(T());" at block scope, it is probably
      // an attempt to initialize a variable, not a function declaration.
      if (FTI.isAmbiguous)
        warnAboutAmbiguousFunction(S, D, DeclType, T);

      FunctionType::ExtInfo EI(getCCForDeclaratorChunk(S, D, FTI, chunkIndex));

      if (!FTI.NumParams && !FTI.isVariadic && !LangOpts.CPlusPlus) {
        // Simple void foo(), where the incoming T is the result type.
        T = Context.getFunctionNoProtoType(T, EI);
      } else {
        // We allow a zero-parameter variadic function in C if the
        // function is marked with the "overloadable" attribute. Scan
        // for this attribute now.
        if (!FTI.NumParams && FTI.isVariadic && !LangOpts.CPlusPlus) {
          bool Overloadable = false;
          for (const AttributeList *Attrs = D.getAttributes();
               Attrs; Attrs = Attrs->getNext()) {
            if (Attrs->getKind() == AttributeList::AT_Overloadable) {
              Overloadable = true;
              break;
            }
          }

          if (!Overloadable)
            S.Diag(FTI.getEllipsisLoc(), diag::err_ellipsis_first_param);
        }

        if (FTI.NumParams && FTI.Params[0].Param == nullptr) {
          // C99 6.7.5.3p3: Reject int(x,y,z) when it's not a function
          // definition.
          S.Diag(FTI.Params[0].IdentLoc,
                 diag::err_ident_list_in_fn_declaration);
          D.setInvalidType(true);
          // Recover by creating a K&R-style function type.
          T = Context.getFunctionNoProtoType(T, EI);
          break;
        }

        FunctionProtoType::ExtProtoInfo EPI;
        EPI.ExtInfo = EI;
        EPI.Variadic = FTI.isVariadic;
        EPI.HasTrailingReturn = FTI.hasTrailingReturnType();
        EPI.TypeQuals = FTI.TypeQuals;
        EPI.RefQualifier = !FTI.hasRefQualifier()? RQ_None
                    : FTI.RefQualifierIsLValueRef? RQ_LValue
                    : RQ_RValue;

        // Otherwise, we have a function with a parameter list that is
        // potentially variadic.
        SmallVector<QualType, 16> ParamTys;
        ParamTys.reserve(FTI.NumParams);

        // HLSL Change Starts
        SmallVector<hlsl::ParameterModifier, 16> ParamMods;
        ParamMods.reserve(FTI.NumParams);
        // HLSL Change Ends

        SmallVector<bool, 16> ConsumedParameters;
        ConsumedParameters.reserve(FTI.NumParams);
        bool HasAnyConsumedParameters = false;

        for (unsigned i = 0, e = FTI.NumParams; i != e; ++i) {
          ParmVarDecl *Param = cast<ParmVarDecl>(FTI.Params[i].Param);
          QualType ParamTy = Param->getType();
          assert(!ParamTy.isNull() && "Couldn't parse type?");

          // Look for 'void'.  void is allowed only as a single parameter to a
          // function with no other parameters (C99 6.7.5.3p10).  We record
          // int(void) as a FunctionProtoType with an empty parameter list.
          if (ParamTy->isVoidType()) {
            // If this is something like 'float(int, void)', reject it.  'void'
            // is an incomplete type (C99 6.2.5p19) and function decls cannot
            // have parameters of incomplete type.
            if (FTI.NumParams != 1 || FTI.isVariadic) {
              S.Diag(DeclType.Loc, diag::err_void_only_param);
              ParamTy = Context.IntTy;
              Param->setType(ParamTy);
            } else if (FTI.Params[i].Ident) {
              // Reject, but continue to parse 'int(void abc)'.
              S.Diag(FTI.Params[i].IdentLoc, diag::err_param_with_void_type);
              ParamTy = Context.IntTy;
              Param->setType(ParamTy);
            } else {
              // Reject, but continue to parse 'float(const void)'.
              if (ParamTy.hasQualifiers())
                S.Diag(DeclType.Loc, diag::err_void_param_qualified);

              // Do not add 'void' to the list.
              break;
            }
          } else if (ParamTy->isHalfType()) {
            // Disallow half FP parameters.
            // FIXME: This really should be in BuildFunctionType.
            if (S.getLangOpts().OpenCL) {
              if (!S.getOpenCLOptions().cl_khr_fp16) {
                S.Diag(Param->getLocation(),
                  diag::err_opencl_half_param) << ParamTy;
                D.setInvalidType();
                Param->setInvalidDecl();
              }
            } else if (!S.getLangOpts().HalfArgsAndReturns) {
              S.Diag(Param->getLocation(),
                diag::err_parameters_retval_cannot_have_fp16_type) << 0;
              D.setInvalidType();
            }
          } else if (!FTI.hasPrototype) {
            if (ParamTy->isPromotableIntegerType()) {
              ParamTy = Context.getPromotedIntegerType(ParamTy);
              Param->setKNRPromoted(true);
            } else if (const BuiltinType* BTy = ParamTy->getAs<BuiltinType>()) {
              if (BTy->getKind() == BuiltinType::Float) {
                ParamTy = Context.DoubleTy;
                Param->setKNRPromoted(true);
              }
            }
          }

          if (LangOpts.ObjCAutoRefCount) {
            bool Consumed = Param->hasAttr<NSConsumedAttr>();
            ConsumedParameters.push_back(Consumed);
            HasAnyConsumedParameters |= Consumed;
          }

          ParamTys.push_back(ParamTy);
          ParamMods.push_back(Param->getParamModifiers());
        }

        if (HasAnyConsumedParameters)
          EPI.ConsumedParameters = ConsumedParameters.data();

        SmallVector<QualType, 4> Exceptions;
        SmallVector<ParsedType, 2> DynamicExceptions;
        SmallVector<SourceRange, 2> DynamicExceptionRanges;
        Expr *NoexceptExpr = nullptr;

        if (FTI.getExceptionSpecType() == EST_Dynamic) {
          // FIXME: It's rather inefficient to have to split into two vectors
          // here.
          unsigned N = FTI.NumExceptions;
          DynamicExceptions.reserve(N);
          DynamicExceptionRanges.reserve(N);
          for (unsigned I = 0; I != N; ++I) {
            DynamicExceptions.push_back(FTI.Exceptions[I].Ty);
            DynamicExceptionRanges.push_back(FTI.Exceptions[I].Range);
          }
        } else if (FTI.getExceptionSpecType() == EST_ComputedNoexcept) {
          NoexceptExpr = FTI.NoexceptExpr;
        }

        S.checkExceptionSpecification(D.isFunctionDeclarationContext(),
                                      FTI.getExceptionSpecType(),
                                      DynamicExceptions,
                                      DynamicExceptionRanges,
                                      NoexceptExpr,
                                      Exceptions,
                                      EPI.ExceptionSpec);

        T = Context.getFunctionType(T, ParamTys, EPI, ParamMods);
      }

      break;
    }
    case DeclaratorChunk::MemberPointer:
      // The scope spec must refer to a class, or be dependent.
      CXXScopeSpec &SS = DeclType.Mem.Scope();
      QualType ClsType;

      // Handle pointer nullability.
      inferPointerNullability(SimplePointerKind::MemberPointer,
                              DeclType.Loc, DeclType.getAttrListRef());

      if (SS.isInvalid()) {
        // Avoid emitting extra errors if we already errored on the scope.
        D.setInvalidType(true);
      } else if (S.isDependentScopeSpecifier(SS) ||
                 dyn_cast_or_null<CXXRecordDecl>(S.computeDeclContext(SS))) {
        NestedNameSpecifier *NNS = SS.getScopeRep();
        NestedNameSpecifier *NNSPrefix = NNS->getPrefix();
        switch (NNS->getKind()) {
        case NestedNameSpecifier::Identifier:
          ClsType = Context.getDependentNameType(ETK_None, NNSPrefix,
                                                 NNS->getAsIdentifier());
          break;

        case NestedNameSpecifier::Namespace:
        case NestedNameSpecifier::NamespaceAlias:
        case NestedNameSpecifier::Global:
        case NestedNameSpecifier::Super:
          llvm_unreachable("Nested-name-specifier must name a type");

        case NestedNameSpecifier::TypeSpec:
        case NestedNameSpecifier::TypeSpecWithTemplate:
          ClsType = QualType(NNS->getAsType(), 0);
          // Note: if the NNS has a prefix and ClsType is a nondependent
          // TemplateSpecializationType, then the NNS prefix is NOT included
          // in ClsType; hence we wrap ClsType into an ElaboratedType.
          // NOTE: in particular, no wrap occurs if ClsType already is an
          // Elaborated, DependentName, or DependentTemplateSpecialization.
          if (NNSPrefix && isa<TemplateSpecializationType>(NNS->getAsType()))
            ClsType = Context.getElaboratedType(ETK_None, NNSPrefix, ClsType);
          break;
        }
      } else {
        S.Diag(DeclType.Mem.Scope().getBeginLoc(),
             diag::err_illegal_decl_mempointer_in_nonclass)
          << (D.getIdentifier() ? D.getIdentifier()->getName() : "type name")
          << DeclType.Mem.Scope().getRange();
        D.setInvalidType(true);
      }

      if (!ClsType.isNull())
        T = S.BuildMemberPointerType(T, ClsType, DeclType.Loc,
                                     D.getIdentifier());
      if (T.isNull()) {
        T = Context.IntTy;
        D.setInvalidType(true);
      } else if (DeclType.Mem.TypeQuals) {
        T = S.BuildQualifiedType(T, DeclType.Loc, DeclType.Mem.TypeQuals);
      }
      break;
    }

    if (T.isNull()) {
      D.setInvalidType(true);
      T = Context.IntTy;
    }

    // See if there are any attributes on this declarator chunk.
    if (AttributeList *attrs = const_cast<AttributeList*>(DeclType.getAttrs()))
      processTypeAttrs(state, T, TAL_DeclChunk, attrs);
  }

  assert(!T.isNull() && "T must not be null after this point");

  if (LangOpts.CPlusPlus && T->isFunctionType()) {
    const FunctionProtoType *FnTy = T->getAs<FunctionProtoType>();
    assert(FnTy && "Why oh why is there not a FunctionProtoType here?");

    // C++ 8.3.5p4:
    //   A cv-qualifier-seq shall only be part of the function type
    //   for a nonstatic member function, the function type to which a pointer
    //   to member refers, or the top-level function type of a function typedef
    //   declaration.
    //
    // Core issue 547 also allows cv-qualifiers on function types that are
    // top-level template type arguments.
    bool FreeFunction;
    if (!D.getCXXScopeSpec().isSet()) {
      FreeFunction = ((D.getContext() != Declarator::MemberContext &&
                       D.getContext() != Declarator::LambdaExprContext) ||
                      D.getDeclSpec().isFriendSpecified());
    } else {
      DeclContext *DC = S.computeDeclContext(D.getCXXScopeSpec());
      FreeFunction = (DC && !DC->isRecord());
    }

    // C++11 [dcl.fct]p6 (w/DR1417):
    // An attempt to specify a function type with a cv-qualifier-seq or a
    // ref-qualifier (including by typedef-name) is ill-formed unless it is:
    //  - the function type for a non-static member function,
    //  - the function type to which a pointer to member refers,
    //  - the top-level function type of a function typedef declaration or
    //    alias-declaration,
    //  - the type-id in the default argument of a type-parameter, or
    //  - the type-id of a template-argument for a type-parameter
    //
    // FIXME: Checking this here is insufficient. We accept-invalid on:
    //
    //   template<typename T> struct S { void f(T); };
    //   S<int() const> s;
    //
    // ... for instance.
    if (IsQualifiedFunction &&
        !(!FreeFunction &&
          D.getDeclSpec().getStorageClassSpec() != DeclSpec::SCS_static) &&
        !IsTypedefName &&
        D.getContext() != Declarator::TemplateTypeArgContext) {
      SourceLocation Loc = D.getLocStart();
      SourceRange RemovalRange;
      unsigned I;
      if (D.isFunctionDeclarator(I)) {
        SmallVector<SourceLocation, 4> RemovalLocs;
        const DeclaratorChunk &Chunk = D.getTypeObject(I);
        assert(Chunk.Kind == DeclaratorChunk::Function);
        if (Chunk.Fun.hasRefQualifier())
          RemovalLocs.push_back(Chunk.Fun.getRefQualifierLoc());
        if (Chunk.Fun.TypeQuals & Qualifiers::Const)
          RemovalLocs.push_back(Chunk.Fun.getConstQualifierLoc());
        if (Chunk.Fun.TypeQuals & Qualifiers::Volatile)
          RemovalLocs.push_back(Chunk.Fun.getVolatileQualifierLoc());
        if (Chunk.Fun.TypeQuals & Qualifiers::Restrict)
          RemovalLocs.push_back(Chunk.Fun.getRestrictQualifierLoc());
        if (!RemovalLocs.empty()) {
          std::sort(RemovalLocs.begin(), RemovalLocs.end(),
                    BeforeThanCompare<SourceLocation>(S.getSourceManager()));
          RemovalRange = SourceRange(RemovalLocs.front(), RemovalLocs.back());
          Loc = RemovalLocs.front();
        }
      }

      S.Diag(Loc, diag::err_invalid_qualified_function_type)
        << FreeFunction << D.isFunctionDeclarator() << T
        << getFunctionQualifiersAsString(FnTy)
        << FixItHint::CreateRemoval(RemovalRange);

      // Strip the cv-qualifiers and ref-qualifiers from the type.
      FunctionProtoType::ExtProtoInfo EPI = FnTy->getExtProtoInfo();
      EPI.TypeQuals = 0;
      EPI.RefQualifier = RQ_None;

      T = Context.getFunctionType(FnTy->getReturnType(), FnTy->getParamTypes(),
                                  EPI, FnTy->getParamMods());
      // Rebuild any parens around the identifier in the function type.
      for (unsigned i = 0, e = D.getNumTypeObjects(); i != e; ++i) {
        if (D.getTypeObject(i).Kind != DeclaratorChunk::Paren)
          break;
        T = S.BuildParenType(T);
      }
    }
  }

  // Apply any undistributed attributes from the declarator.
  if (AttributeList *attrs = D.getAttributes())
    processTypeAttrs(state, T, TAL_DeclName, attrs);

  // Diagnose any ignored type attributes.
  state.diagnoseIgnoredTypeAttrs(T);

  // C++0x [dcl.constexpr]p9:
  //  A constexpr specifier used in an object declaration declares the object
  //  as const.
  if (D.getDeclSpec().isConstexprSpecified() && T->isObjectType()) {
    T.addConst();
  }

  // If there was an ellipsis in the declarator, the declaration declares a
  // parameter pack whose type may be a pack expansion type.
  if (D.hasEllipsis()) {
    // C++0x [dcl.fct]p13:
    //   A declarator-id or abstract-declarator containing an ellipsis shall
    //   only be used in a parameter-declaration. Such a parameter-declaration
    //   is a parameter pack (14.5.3). [...]
    switch (D.getContext()) {
    case Declarator::PrototypeContext:
    case Declarator::LambdaExprParameterContext:
      // C++0x [dcl.fct]p13:
      //   [...] When it is part of a parameter-declaration-clause, the
      //   parameter pack is a function parameter pack (14.5.3). The type T
      //   of the declarator-id of the function parameter pack shall contain
      //   a template parameter pack; each template parameter pack in T is
      //   expanded by the function parameter pack.
      //
      // We represent function parameter packs as function parameters whose
      // type is a pack expansion.
      if (!T->containsUnexpandedParameterPack()) {
        S.Diag(D.getEllipsisLoc(),
             diag::err_function_parameter_pack_without_parameter_packs)
          << T <<  D.getSourceRange();
        D.setEllipsisLoc(SourceLocation());
      } else {
        T = Context.getPackExpansionType(T, None);
      }
      break;
    case Declarator::TemplateParamContext:
      // C++0x [temp.param]p15:
      //   If a template-parameter is a [...] is a parameter-declaration that
      //   declares a parameter pack (8.3.5), then the template-parameter is a
      //   template parameter pack (14.5.3).
      //
      // Note: core issue 778 clarifies that, if there are any unexpanded
      // parameter packs in the type of the non-type template parameter, then
      // it expands those parameter packs.
      // HLSL Change Starts
      if (LangOpts.HLSL) {
        S.Diag(D.getEllipsisLoc(), diag::err_hlsl_variadic_templates);
        break;
      }
      // HLSL Change Ends
      if (T->containsUnexpandedParameterPack())
        T = Context.getPackExpansionType(T, None);
      else
        S.Diag(D.getEllipsisLoc(),
               LangOpts.CPlusPlus11
                 ? diag::warn_cxx98_compat_variadic_templates
                 : diag::ext_variadic_templates);
      break;

    case Declarator::FileContext:
    case Declarator::KNRTypeListContext:
    case Declarator::ObjCParameterContext:  // FIXME: special diagnostic here?
    case Declarator::ObjCResultContext:     // FIXME: special diagnostic here?
    case Declarator::TypeNameContext:
    case Declarator::CXXNewContext:
    case Declarator::AliasDeclContext:
    case Declarator::AliasTemplateContext:
    case Declarator::MemberContext:
    case Declarator::BlockContext:
    case Declarator::ForContext:
    case Declarator::ConditionContext:
    case Declarator::CXXCatchContext:
    case Declarator::ObjCCatchContext:
    case Declarator::BlockLiteralContext:
    case Declarator::LambdaExprContext:
    case Declarator::ConversionIdContext:
    case Declarator::TrailingReturnContext:
    case Declarator::TemplateTypeArgContext:
      // FIXME: We may want to allow parameter packs in block-literal contexts
      // in the future.
      S.Diag(D.getEllipsisLoc(),
             diag::err_ellipsis_in_declarator_not_parameter);
      D.setEllipsisLoc(SourceLocation());
      break;
    }
  }

  assert(!T.isNull() && "T must not be null at the end of this function");
  if (D.isInvalidType())
    return Context.getTrivialTypeSourceInfo(T);

  return S.GetTypeSourceInfoForDeclarator(D, T, TInfo);
}

/// GetTypeForDeclarator - Convert the type for the specified
/// declarator to Type instances.
///
/// The result of this call will never be null, but the associated
/// type may be a null type if there's an unrecoverable error.
TypeSourceInfo *Sema::GetTypeForDeclarator(Declarator &D, Scope *S) {
  // Determine the type of the declarator. Not all forms of declarator
  // have a type.

  TypeProcessingState state(*this, D, S);   // MS Change - add scope

  TypeSourceInfo *ReturnTypeInfo = nullptr;
  QualType T = GetDeclSpecTypeForDeclarator(state, ReturnTypeInfo);

  if (D.isPrototypeContext() && getLangOpts().ObjCAutoRefCount)
    inferARCWriteback(state, T);

  // HLSL changes begin
  // If there is no explicit pack orientation on matrix types, but there is file-level
  // default orientation set by #pragma pack_matrix, apply it here.
  // There is no default if rewriting (in the absence of #pragma pack_matrix), since
  // it is agnostic to default orientation and we want to preserve the lack of annotation.
  // For codegen, it'd be nice to annotate everything here, but it causes error
  // messages to have pack orientation added to types, so we handle it through
  // the codegen option's default packing orientation flag.
  bool defaultRowMajor;
  if (getLangOpts().HLSL && hlsl::IsHLSLMatType(T) && !hlsl::HasHLSLMatOrientation(T)
    && D.getDeclSpec().TryGetDefaultMatrixPackRowMajor(defaultRowMajor)) {
    AttributedType::Kind AttributeKind = defaultRowMajor
      ? AttributedType::attr_hlsl_row_major : AttributedType::attr_hlsl_column_major;
    T = Context.getAttributedType(AttributeKind, T, T);
  }
  // HLSL changes end

  return GetFullTypeForDeclarator(state, T, ReturnTypeInfo);
}

static void transferARCOwnershipToDeclSpec(Sema &S,
                                           QualType &declSpecTy,
                                           Qualifiers::ObjCLifetime ownership) {
  if (declSpecTy->isObjCRetainableType() &&
      declSpecTy.getObjCLifetime() == Qualifiers::OCL_None) {
    Qualifiers qs;
    qs.addObjCLifetime(ownership);
    declSpecTy = S.Context.getQualifiedType(declSpecTy, qs);
  }
}

static void transferARCOwnershipToDeclaratorChunk(TypeProcessingState &state,
                                            Qualifiers::ObjCLifetime ownership,
                                            unsigned chunkIndex) {
  Sema &S = state.getSema();
  Declarator &D = state.getDeclarator();

  // Look for an explicit lifetime attribute.
  DeclaratorChunk &chunk = D.getTypeObject(chunkIndex);
  for (const AttributeList *attr = chunk.getAttrs(); attr;
         attr = attr->getNext())
    if (attr->getKind() == AttributeList::AT_ObjCOwnership)
      return;

  const char *attrStr = nullptr;
  switch (ownership) {
  case Qualifiers::OCL_None: llvm_unreachable("no ownership!");
  case Qualifiers::OCL_ExplicitNone: attrStr = "none"; break;
  case Qualifiers::OCL_Strong: attrStr = "strong"; break;
  case Qualifiers::OCL_Weak: attrStr = "weak"; break;
  case Qualifiers::OCL_Autoreleasing: attrStr = "autoreleasing"; break;
  }

  IdentifierLoc *Arg = new (S.Context) IdentifierLoc;
  Arg->Ident = &S.Context.Idents.get(attrStr);
  Arg->Loc = SourceLocation();

  ArgsUnion Args(Arg);

  // If there wasn't one, add one (with an invalid source location
  // so that we don't make an AttributedType for it).
  AttributeList *attr = D.getAttributePool()
    .create(&S.Context.Idents.get("objc_ownership"), SourceLocation(),
            /*scope*/ nullptr, SourceLocation(),
            /*args*/ &Args, 1, AttributeList::AS_GNU);
  spliceAttrIntoList(*attr, chunk.getAttrListRef());

  // TODO: mark whether we did this inference?
}

/// \brief Used for transferring ownership in casts resulting in l-values.
static void transferARCOwnership(TypeProcessingState &state,
                                 QualType &declSpecTy,
                                 Qualifiers::ObjCLifetime ownership) {
  Sema &S = state.getSema();
  Declarator &D = state.getDeclarator();

  int inner = -1;
  bool hasIndirection = false;
  for (unsigned i = 0, e = D.getNumTypeObjects(); i != e; ++i) {
    DeclaratorChunk &chunk = D.getTypeObject(i);
    switch (chunk.Kind) {
    case DeclaratorChunk::Paren:
      // Ignore parens.
      break;

    case DeclaratorChunk::Array:
    case DeclaratorChunk::Reference:
    case DeclaratorChunk::Pointer:
      if (inner != -1)
        hasIndirection = true;
      inner = i;
      break;

    case DeclaratorChunk::BlockPointer:
      if (inner != -1)
        transferARCOwnershipToDeclaratorChunk(state, ownership, i);
      return;

    case DeclaratorChunk::Function:
    case DeclaratorChunk::MemberPointer:
      return;
    }
  }

  if (inner == -1)
    return;

  DeclaratorChunk &chunk = D.getTypeObject(inner);
  if (chunk.Kind == DeclaratorChunk::Pointer) {
    if (declSpecTy->isObjCRetainableType())
      return transferARCOwnershipToDeclSpec(S, declSpecTy, ownership);
    if (declSpecTy->isObjCObjectType() && hasIndirection)
      return transferARCOwnershipToDeclaratorChunk(state, ownership, inner);
  } else {
    assert(chunk.Kind == DeclaratorChunk::Array ||
           chunk.Kind == DeclaratorChunk::Reference);
    return transferARCOwnershipToDeclSpec(S, declSpecTy, ownership);
  }
}

TypeSourceInfo *Sema::GetTypeForDeclaratorCast(Declarator &D, QualType FromTy) {
  TypeProcessingState state(*this, D);

  TypeSourceInfo *ReturnTypeInfo = nullptr;
  QualType declSpecTy = GetDeclSpecTypeForDeclarator(state, ReturnTypeInfo);

  if (getLangOpts().ObjCAutoRefCount) {
    Qualifiers::ObjCLifetime ownership = Context.getInnerObjCOwnership(FromTy);
    if (ownership != Qualifiers::OCL_None)
      transferARCOwnership(state, declSpecTy, ownership);
  }

  return GetFullTypeForDeclarator(state, declSpecTy, ReturnTypeInfo);
}

/// Map an AttributedType::Kind to an AttributeList::Kind.
static AttributeList::Kind getAttrListKind(AttributedType::Kind kind) {
  switch (kind) {
  case AttributedType::attr_address_space:
    return AttributeList::AT_AddressSpace;
  case AttributedType::attr_regparm:
    return AttributeList::AT_Regparm;
  case AttributedType::attr_vector_size:
    return AttributeList::AT_VectorSize;
  case AttributedType::attr_neon_vector_type:
    return AttributeList::AT_NeonVectorType;
  case AttributedType::attr_neon_polyvector_type:
    return AttributeList::AT_NeonPolyVectorType;
  case AttributedType::attr_objc_gc:
    return AttributeList::AT_ObjCGC;
  case AttributedType::attr_objc_ownership:
    return AttributeList::AT_ObjCOwnership;
  case AttributedType::attr_noreturn:
    return AttributeList::AT_NoReturn;
  case AttributedType::attr_cdecl:
    return AttributeList::AT_CDecl;
  case AttributedType::attr_fastcall:
    return AttributeList::AT_FastCall;
  case AttributedType::attr_stdcall:
    return AttributeList::AT_StdCall;
  case AttributedType::attr_thiscall:
    return AttributeList::AT_ThisCall;
  case AttributedType::attr_pascal:
    return AttributeList::AT_Pascal;
  case AttributedType::attr_vectorcall:
    return AttributeList::AT_VectorCall;
  case AttributedType::attr_pcs:
  case AttributedType::attr_pcs_vfp:
    return AttributeList::AT_Pcs;
  case AttributedType::attr_inteloclbicc:
    return AttributeList::AT_IntelOclBicc;
  case AttributedType::attr_ms_abi:
    return AttributeList::AT_MSABI;
  case AttributedType::attr_sysv_abi:
    return AttributeList::AT_SysVABI;
  case AttributedType::attr_ptr32:
    return AttributeList::AT_Ptr32;
  case AttributedType::attr_ptr64:
    return AttributeList::AT_Ptr64;
  case AttributedType::attr_sptr:
    return AttributeList::AT_SPtr;
  case AttributedType::attr_uptr:
    return AttributeList::AT_UPtr;
  case AttributedType::attr_nonnull:
    return AttributeList::AT_TypeNonNull;
  case AttributedType::attr_nullable:
    return AttributeList::AT_TypeNullable;
  case AttributedType::attr_null_unspecified:
    return AttributeList::AT_TypeNullUnspecified;
  case AttributedType::attr_objc_kindof:
    return AttributeList::AT_ObjCKindOf;
  // HLSL Change Begins
  case AttributedType::attr_hlsl_unorm:
    return AttributeList::AT_HLSLUnorm;
  case AttributedType::attr_hlsl_snorm:
    return AttributeList::AT_HLSLSnorm;
  case AttributedType::attr_hlsl_row_major:
    return AttributeList::AT_HLSLRowMajor;
  case AttributedType::attr_hlsl_column_major:
    return AttributeList::AT_HLSLColumnMajor;
  case AttributedType::attr_hlsl_globallycoherent:
    return AttributeList::AT_HLSLGloballyCoherent;
  // HLSL Change Ends
  }
  llvm_unreachable("unexpected attribute kind!");
}

static void fillAttributedTypeLoc(AttributedTypeLoc TL,
                                  const AttributeList *attrs,
                                  const AttributeList *DeclAttrs = nullptr) {

  // HLSL changes begin
  // Don't fill the location info for matrix orientation attributes
  if (TL.getAttrKind() == AttributedType::attr_hlsl_row_major ||
      TL.getAttrKind() == AttributedType::attr_hlsl_column_major)
    return;
  // HLSL changes end

  // DeclAttrs and attrs cannot be both empty.
  assert((attrs || DeclAttrs) &&
         "no type attributes in the expected location!");

  AttributeList::Kind parsedKind = getAttrListKind(TL.getAttrKind());
  // Try to search for an attribute of matching kind in attrs list.
  while (attrs && attrs->getKind() != parsedKind)
    attrs = attrs->getNext();
  if (!attrs) {
    // No matching type attribute in attrs list found.
    // Try searching through C++11 attributes in the declarator attribute list.
    while (DeclAttrs && (!DeclAttrs->isCXX11Attribute() ||
                         DeclAttrs->getKind() != parsedKind))
      DeclAttrs = DeclAttrs->getNext();
    attrs = DeclAttrs;
  }

  assert(attrs && "no matching type attribute in expected location!");

  TL.setAttrNameLoc(attrs->getLoc());
  if (TL.hasAttrExprOperand()) {
    assert(attrs->isArgExpr(0) && "mismatched attribute operand kind");
    TL.setAttrExprOperand(attrs->getArgAsExpr(0));
  } else if (TL.hasAttrEnumOperand()) {
    assert((attrs->isArgIdent(0) || attrs->isArgExpr(0)) &&
           "unexpected attribute operand kind");
    if (attrs->isArgIdent(0))
      TL.setAttrEnumOperandLoc(attrs->getArgAsIdent(0)->Loc);
    else
      TL.setAttrEnumOperandLoc(attrs->getArgAsExpr(0)->getExprLoc());
  }

  // FIXME: preserve this information to here.
  if (TL.hasAttrOperand())
    TL.setAttrOperandParensRange(SourceRange());
}

namespace {
  class TypeSpecLocFiller : public TypeLocVisitor<TypeSpecLocFiller> {
    ASTContext &Context;
    const DeclSpec &DS;

  public:
    TypeSpecLocFiller(ASTContext &Context, const DeclSpec &DS)
      : Context(Context), DS(DS) {}

    void VisitAttributedTypeLoc(AttributedTypeLoc TL) {
      fillAttributedTypeLoc(TL, DS.getAttributes().getList());
      Visit(TL.getModifiedLoc());
    }
    void VisitQualifiedTypeLoc(QualifiedTypeLoc TL) {
      Visit(TL.getUnqualifiedLoc());
    }
    void VisitTypedefTypeLoc(TypedefTypeLoc TL) {
      TL.setNameLoc(DS.getTypeSpecTypeLoc());
    }
    void VisitObjCInterfaceTypeLoc(ObjCInterfaceTypeLoc TL) {
      TL.setNameLoc(DS.getTypeSpecTypeLoc());
      // FIXME. We should have DS.getTypeSpecTypeEndLoc(). But, it requires
      // addition field. What we have is good enough for dispay of location
      // of 'fixit' on interface name.
      TL.setNameEndLoc(DS.getLocEnd());
    }
    void VisitObjCObjectTypeLoc(ObjCObjectTypeLoc TL) {
      TypeSourceInfo *RepTInfo = nullptr;
      Sema::GetTypeFromParser(DS.getRepAsType(), &RepTInfo);
      TL.copy(RepTInfo->getTypeLoc());
    }
    void VisitObjCObjectPointerTypeLoc(ObjCObjectPointerTypeLoc TL) {
      TypeSourceInfo *RepTInfo = nullptr;
      Sema::GetTypeFromParser(DS.getRepAsType(), &RepTInfo);
      TL.copy(RepTInfo->getTypeLoc());
    }
    void VisitTemplateSpecializationTypeLoc(TemplateSpecializationTypeLoc TL) {
      TypeSourceInfo *TInfo = nullptr;
      Sema::GetTypeFromParser(DS.getRepAsType(), &TInfo);

      // If we got no declarator info from previous Sema routines,
      // just fill with the typespec loc.
      if (!TInfo) {
        TL.initialize(Context, DS.getTypeSpecTypeNameLoc());
        return;
      }

      TypeLoc OldTL = TInfo->getTypeLoc();
      if (TInfo->getType()->getAs<ElaboratedType>()) {
        ElaboratedTypeLoc ElabTL = OldTL.castAs<ElaboratedTypeLoc>();
        TemplateSpecializationTypeLoc NamedTL = ElabTL.getNamedTypeLoc()
            .castAs<TemplateSpecializationTypeLoc>();
        TL.copy(NamedTL);
      } else {
        TL.copy(OldTL.castAs<TemplateSpecializationTypeLoc>());
        assert(TL.getRAngleLoc() == OldTL.castAs<TemplateSpecializationTypeLoc>().getRAngleLoc());
      }
        
    }
    void VisitTypeOfExprTypeLoc(TypeOfExprTypeLoc TL) {
      assert(DS.getTypeSpecType() == DeclSpec::TST_typeofExpr);
      TL.setTypeofLoc(DS.getTypeSpecTypeLoc());
      TL.setParensRange(DS.getTypeofParensRange());
    }
    void VisitTypeOfTypeLoc(TypeOfTypeLoc TL) {
      assert(DS.getTypeSpecType() == DeclSpec::TST_typeofType);
      TL.setTypeofLoc(DS.getTypeSpecTypeLoc());
      TL.setParensRange(DS.getTypeofParensRange());
      assert(DS.getRepAsType());
      TypeSourceInfo *TInfo = nullptr;
      Sema::GetTypeFromParser(DS.getRepAsType(), &TInfo);
      TL.setUnderlyingTInfo(TInfo);
    }
    void VisitUnaryTransformTypeLoc(UnaryTransformTypeLoc TL) {
      // FIXME: This holds only because we only have one unary transform.
      assert(DS.getTypeSpecType() == DeclSpec::TST_underlyingType);
      TL.setKWLoc(DS.getTypeSpecTypeLoc());
      TL.setParensRange(DS.getTypeofParensRange());
      assert(DS.getRepAsType());
      TypeSourceInfo *TInfo = nullptr;
      Sema::GetTypeFromParser(DS.getRepAsType(), &TInfo);
      TL.setUnderlyingTInfo(TInfo);
    }
    void VisitBuiltinTypeLoc(BuiltinTypeLoc TL) {
      // By default, use the source location of the type specifier.
      TL.setBuiltinLoc(DS.getTypeSpecTypeLoc());
      if (TL.needsExtraLocalData()) {
        // Set info for the written builtin specifiers.
        TL.getWrittenBuiltinSpecs() = DS.getWrittenBuiltinSpecs();
        // Try to have a meaningful source location.
        if (TL.getWrittenSignSpec() != TSS_unspecified)
          // Sign spec loc overrides the others (e.g., 'unsigned long').
          TL.setBuiltinLoc(DS.getTypeSpecSignLoc());
        else if (TL.getWrittenWidthSpec() != TSW_unspecified)
          // Width spec loc overrides type spec loc (e.g., 'short int').
          TL.setBuiltinLoc(DS.getTypeSpecWidthLoc());
      }
    }
    void VisitElaboratedTypeLoc(ElaboratedTypeLoc TL) {
      ElaboratedTypeKeyword Keyword
        = TypeWithKeyword::getKeywordForTypeSpec(DS.getTypeSpecType());
      if (DS.getTypeSpecType() == TST_typename) {
        TypeSourceInfo *TInfo = nullptr;
        Sema::GetTypeFromParser(DS.getRepAsType(), &TInfo);
        if (TInfo) {
          TL.copy(TInfo->getTypeLoc().castAs<ElaboratedTypeLoc>());
          return;
        }
      }
      TL.setElaboratedKeywordLoc(Keyword != ETK_None
                                 ? DS.getTypeSpecTypeLoc()
                                 : SourceLocation());
      const CXXScopeSpec& SS = DS.getTypeSpecScope();
      TL.setQualifierLoc(SS.getWithLocInContext(Context));
      Visit(TL.getNextTypeLoc().getUnqualifiedLoc());
    }
    void VisitDependentNameTypeLoc(DependentNameTypeLoc TL) {
      assert(DS.getTypeSpecType() == TST_typename);
      TypeSourceInfo *TInfo = nullptr;
      Sema::GetTypeFromParser(DS.getRepAsType(), &TInfo);
      assert(TInfo);
      TL.copy(TInfo->getTypeLoc().castAs<DependentNameTypeLoc>());
    }
    void VisitDependentTemplateSpecializationTypeLoc(
                                 DependentTemplateSpecializationTypeLoc TL) {
      assert(DS.getTypeSpecType() == TST_typename);
      TypeSourceInfo *TInfo = nullptr;
      Sema::GetTypeFromParser(DS.getRepAsType(), &TInfo);
      assert(TInfo);
      TL.copy(
          TInfo->getTypeLoc().castAs<DependentTemplateSpecializationTypeLoc>());
    }
    void VisitTagTypeLoc(TagTypeLoc TL) {
      TL.setNameLoc(DS.getTypeSpecTypeNameLoc());
    }
    void VisitAtomicTypeLoc(AtomicTypeLoc TL) {
      // An AtomicTypeLoc can come from either an _Atomic(...) type specifier
      // or an _Atomic qualifier.
      if (DS.getTypeSpecType() == DeclSpec::TST_atomic) {
        TL.setKWLoc(DS.getTypeSpecTypeLoc());
        TL.setParensRange(DS.getTypeofParensRange());

        TypeSourceInfo *TInfo = nullptr;
        Sema::GetTypeFromParser(DS.getRepAsType(), &TInfo);
        assert(TInfo);
        TL.getValueLoc().initializeFullCopy(TInfo->getTypeLoc());
      } else {
        TL.setKWLoc(DS.getAtomicSpecLoc());
        // No parens, to indicate this was spelled as an _Atomic qualifier.
        TL.setParensRange(SourceRange());
        Visit(TL.getValueLoc());
      }
    }

    void VisitTypeLoc(TypeLoc TL) {
      // FIXME: add other typespec types and change this to an assert.
      TL.initialize(Context, DS.getTypeSpecTypeLoc());
    }
  };

  class DeclaratorLocFiller : public TypeLocVisitor<DeclaratorLocFiller> {
    ASTContext &Context;
    const DeclaratorChunk &Chunk;

  public:
    DeclaratorLocFiller(ASTContext &Context, const DeclaratorChunk &Chunk)
      : Context(Context), Chunk(Chunk) {}

    void VisitQualifiedTypeLoc(QualifiedTypeLoc TL) {
      llvm_unreachable("qualified type locs not expected here!");
    }
    void VisitDecayedTypeLoc(DecayedTypeLoc TL) {
      llvm_unreachable("decayed type locs not expected here!");
    }

    void VisitAttributedTypeLoc(AttributedTypeLoc TL) {
      fillAttributedTypeLoc(TL, Chunk.getAttrs());
    }
    void VisitAdjustedTypeLoc(AdjustedTypeLoc TL) {
      // nothing
    }
    void VisitBlockPointerTypeLoc(BlockPointerTypeLoc TL) {
      assert(Chunk.Kind == DeclaratorChunk::BlockPointer);
      TL.setCaretLoc(Chunk.Loc);
    }
    void VisitPointerTypeLoc(PointerTypeLoc TL) {
      assert(Chunk.Kind == DeclaratorChunk::Pointer);
      TL.setStarLoc(Chunk.Loc);
    }
    void VisitObjCObjectPointerTypeLoc(ObjCObjectPointerTypeLoc TL) {
      assert(Chunk.Kind == DeclaratorChunk::Pointer);
      TL.setStarLoc(Chunk.Loc);
    }
    void VisitMemberPointerTypeLoc(MemberPointerTypeLoc TL) {
      assert(Chunk.Kind == DeclaratorChunk::MemberPointer);
      const CXXScopeSpec& SS = Chunk.Mem.Scope();
      NestedNameSpecifierLoc NNSLoc = SS.getWithLocInContext(Context);

      const Type* ClsTy = TL.getClass();
      QualType ClsQT = QualType(ClsTy, 0);
      TypeSourceInfo *ClsTInfo = Context.CreateTypeSourceInfo(ClsQT, 0);
      // Now copy source location info into the type loc component.
      TypeLoc ClsTL = ClsTInfo->getTypeLoc();
      switch (NNSLoc.getNestedNameSpecifier()->getKind()) {
      case NestedNameSpecifier::Identifier:
        assert(isa<DependentNameType>(ClsTy) && "Unexpected TypeLoc");
        {
          DependentNameTypeLoc DNTLoc = ClsTL.castAs<DependentNameTypeLoc>();
          DNTLoc.setElaboratedKeywordLoc(SourceLocation());
          DNTLoc.setQualifierLoc(NNSLoc.getPrefix());
          DNTLoc.setNameLoc(NNSLoc.getLocalBeginLoc());
        }
        break;

      case NestedNameSpecifier::TypeSpec:
      case NestedNameSpecifier::TypeSpecWithTemplate:
        if (isa<ElaboratedType>(ClsTy)) {
          ElaboratedTypeLoc ETLoc = ClsTL.castAs<ElaboratedTypeLoc>();
          ETLoc.setElaboratedKeywordLoc(SourceLocation());
          ETLoc.setQualifierLoc(NNSLoc.getPrefix());
          TypeLoc NamedTL = ETLoc.getNamedTypeLoc();
          NamedTL.initializeFullCopy(NNSLoc.getTypeLoc());
        } else {
          ClsTL.initializeFullCopy(NNSLoc.getTypeLoc());
        }
        break;

      case NestedNameSpecifier::Namespace:
      case NestedNameSpecifier::NamespaceAlias:
      case NestedNameSpecifier::Global:
      case NestedNameSpecifier::Super:
        llvm_unreachable("Nested-name-specifier must name a type");
      }

      // Finally fill in MemberPointerLocInfo fields.
      TL.setStarLoc(Chunk.Loc);
      TL.setClassTInfo(ClsTInfo);
    }
    void VisitLValueReferenceTypeLoc(LValueReferenceTypeLoc TL) {
      assert(Chunk.Kind == DeclaratorChunk::Reference);
      // 'Amp' is misleading: this might have been originally
      /// spelled with AmpAmp.
      TL.setAmpLoc(Chunk.Loc);
    }
    void VisitRValueReferenceTypeLoc(RValueReferenceTypeLoc TL) {
      assert(Chunk.Kind == DeclaratorChunk::Reference);
      assert(!Chunk.Ref.LValueRef);
      TL.setAmpAmpLoc(Chunk.Loc);
    }
    void VisitArrayTypeLoc(ArrayTypeLoc TL) {
      assert(Chunk.Kind == DeclaratorChunk::Array);
      TL.setLBracketLoc(Chunk.Loc);
      TL.setRBracketLoc(Chunk.EndLoc);
      TL.setSizeExpr(static_cast<Expr*>(Chunk.Arr.NumElts));
    }
    void VisitFunctionTypeLoc(FunctionTypeLoc TL) {
      assert(Chunk.Kind == DeclaratorChunk::Function);
      TL.setLocalRangeBegin(Chunk.Loc);
      TL.setLocalRangeEnd(Chunk.EndLoc);

      const DeclaratorChunk::FunctionTypeInfo &FTI = Chunk.Fun;
      TL.setLParenLoc(FTI.getLParenLoc());
      TL.setRParenLoc(FTI.getRParenLoc());
      for (unsigned i = 0, e = TL.getNumParams(), tpi = 0; i != e; ++i) {
        ParmVarDecl *Param = cast<ParmVarDecl>(FTI.Params[i].Param);
        TL.setParam(tpi++, Param);
      }
      // FIXME: exception specs
    }
    void VisitParenTypeLoc(ParenTypeLoc TL) {
      assert(Chunk.Kind == DeclaratorChunk::Paren);
      TL.setLParenLoc(Chunk.Loc);
      TL.setRParenLoc(Chunk.EndLoc);
    }

    void VisitTypeLoc(TypeLoc TL) {
      llvm_unreachable("unsupported TypeLoc kind in declarator!");
    }
  };
}

static void fillAtomicQualLoc(AtomicTypeLoc ATL, const DeclaratorChunk &Chunk) {
  SourceLocation Loc;
  switch (Chunk.Kind) {
  case DeclaratorChunk::Function:
  case DeclaratorChunk::Array:
  case DeclaratorChunk::Paren:
    llvm_unreachable("cannot be _Atomic qualified");

  case DeclaratorChunk::Pointer:
    Loc = SourceLocation::getFromRawEncoding(Chunk.Ptr.AtomicQualLoc);
    break;

  case DeclaratorChunk::BlockPointer:
  case DeclaratorChunk::Reference:
  case DeclaratorChunk::MemberPointer:
    // FIXME: Provide a source location for the _Atomic keyword.
    break;
  }

  ATL.setKWLoc(Loc);
  ATL.setParensRange(SourceRange());
}

/// \brief Create and instantiate a TypeSourceInfo with type source information.
///
/// \param T QualType referring to the type as written in source code.
///
/// \param ReturnTypeInfo For declarators whose return type does not show
/// up in the normal place in the declaration specifiers (such as a C++
/// conversion function), this pointer will refer to a type source information
/// for that return type.
TypeSourceInfo *
Sema::GetTypeSourceInfoForDeclarator(Declarator &D, QualType T,
                                     TypeSourceInfo *ReturnTypeInfo) {
  TypeSourceInfo *TInfo = Context.CreateTypeSourceInfo(T);
  UnqualTypeLoc CurrTL = TInfo->getTypeLoc().getUnqualifiedLoc();
  const AttributeList *DeclAttrs = D.getAttributes();

  // Handle parameter packs whose type is a pack expansion.
  if (isa<PackExpansionType>(T)) {
    CurrTL.castAs<PackExpansionTypeLoc>().setEllipsisLoc(D.getEllipsisLoc());
    CurrTL = CurrTL.getNextTypeLoc().getUnqualifiedLoc();
  }

  for (unsigned i = 0, e = D.getNumTypeObjects(); i != e; ++i) {
    // An AtomicTypeLoc might be produced by an atomic qualifier in this
    // declarator chunk.
    if (AtomicTypeLoc ATL = CurrTL.getAs<AtomicTypeLoc>()) {
      fillAtomicQualLoc(ATL, D.getTypeObject(i));
      CurrTL = ATL.getValueLoc().getUnqualifiedLoc();
    }

    while (AttributedTypeLoc TL = CurrTL.getAs<AttributedTypeLoc>()) {
      fillAttributedTypeLoc(TL, D.getTypeObject(i).getAttrs(), DeclAttrs);
      CurrTL = TL.getNextTypeLoc().getUnqualifiedLoc();
    }

    // FIXME: Ordering here?
    while (AdjustedTypeLoc TL = CurrTL.getAs<AdjustedTypeLoc>())
      CurrTL = TL.getNextTypeLoc().getUnqualifiedLoc();

    DeclaratorLocFiller(Context, D.getTypeObject(i)).Visit(CurrTL);
    CurrTL = CurrTL.getNextTypeLoc().getUnqualifiedLoc();
  }

  // If we have different source information for the return type, use
  // that.  This really only applies to C++ conversion functions.
  if (ReturnTypeInfo) {
    TypeLoc TL = ReturnTypeInfo->getTypeLoc();
    assert(TL.getFullDataSize() == CurrTL.getFullDataSize());
    memcpy(CurrTL.getOpaqueData(), TL.getOpaqueData(), TL.getFullDataSize());
  } else {
    TypeSpecLocFiller(Context, D.getDeclSpec()).Visit(CurrTL);
  }

  return TInfo;
}

/// \brief Create a LocInfoType to hold the given QualType and TypeSourceInfo.
ParsedType Sema::CreateParsedType(QualType T, TypeSourceInfo *TInfo) {
  // FIXME: LocInfoTypes are "transient", only needed for passing to/from Parser
  // and Sema during declaration parsing. Try deallocating/caching them when
  // it's appropriate, instead of allocating them and keeping them around.
  LocInfoType *LocT = (LocInfoType*)BumpAlloc.Allocate(sizeof(LocInfoType),
                                                       TypeAlignment);
  new (LocT) LocInfoType(T, TInfo);
  assert(LocT->getTypeClass() != T->getTypeClass() &&
         "LocInfoType's TypeClass conflicts with an existing Type class");
  return ParsedType::make(QualType(LocT, 0));
}

void LocInfoType::getAsStringInternal(std::string &Str,
                                      const PrintingPolicy &Policy) const {
  llvm_unreachable("LocInfoType leaked into the type system; an opaque TypeTy*"
         " was used directly instead of getting the QualType through"
         " GetTypeFromParser");
}

TypeResult Sema::ActOnTypeName(Scope *S, Declarator &D) {
  // C99 6.7.6: Type names have no identifier.  This is already validated by
  // the parser.
  assert(D.getIdentifier() == nullptr &&
         "Type name should have no identifier!");

  TypeSourceInfo *TInfo = GetTypeForDeclarator(D, S);
  QualType T = TInfo->getType();
  if (D.isInvalidType())
    return true;

  // Make sure there are no unused decl attributes on the declarator.
  // We don't want to do this for ObjC parameters because we're going
  // to apply them to the actual parameter declaration.
  // Likewise, we don't want to do this for alias declarations, because
  // we are actually going to build a declaration from this eventually.
  if (D.getContext() != Declarator::ObjCParameterContext &&
      D.getContext() != Declarator::AliasDeclContext &&
      D.getContext() != Declarator::AliasTemplateContext)
    checkUnusedDeclAttributes(D);

  if (getLangOpts().CPlusPlus) {
    // Check that there are no default arguments (C++ only).
    CheckExtraCXXDefaultArguments(D);
  }

  return CreateParsedType(T, TInfo);
}

ParsedType Sema::ActOnObjCInstanceType(SourceLocation Loc) {
  QualType T = Context.getObjCInstanceType();
  TypeSourceInfo *TInfo = Context.getTrivialTypeSourceInfo(T, Loc);
  return CreateParsedType(T, TInfo);
}


//===----------------------------------------------------------------------===//
// Type Attribute Processing
//===----------------------------------------------------------------------===//

/// HandleAddressSpaceTypeAttribute - Process an address_space attribute on the
/// specified type.  The attribute contains 1 argument, the id of the address
/// space for the type.
static void HandleAddressSpaceTypeAttribute(QualType &Type,
                                            const AttributeList &Attr, Sema &S){

  // If this type is already address space qualified, reject it.
  // ISO/IEC TR 18037 S5.3 (amending C99 6.7.3): "No type shall be qualified by
  // qualifiers for two or more different address spaces."
  if (Type.getAddressSpace()) {
    S.Diag(Attr.getLoc(), diag::err_attribute_address_multiple_qualifiers);
    Attr.setInvalid();
    return;
  }

  // ISO/IEC TR 18037 S5.3 (amending C99 6.7.3): "A function type shall not be
  // qualified by an address-space qualifier."
  if (Type->isFunctionType()) {
    S.Diag(Attr.getLoc(), diag::err_attribute_address_function_type);
    Attr.setInvalid();
    return;
  }

  unsigned ASIdx;
  if (Attr.getKind() == AttributeList::AT_AddressSpace) {
    // Check the attribute arguments.
    if (Attr.getNumArgs() != 1) {
      S.Diag(Attr.getLoc(), diag::err_attribute_wrong_number_arguments)
        << Attr.getName() << 1;
      Attr.setInvalid();
      return;
    }
    Expr *ASArgExpr = static_cast<Expr *>(Attr.getArgAsExpr(0));
    llvm::APSInt addrSpace(32);
    if (ASArgExpr->isTypeDependent() || ASArgExpr->isValueDependent() ||
        !ASArgExpr->isIntegerConstantExpr(addrSpace, S.Context)) {
      S.Diag(Attr.getLoc(), diag::err_attribute_argument_type)
        << Attr.getName() << AANT_ArgumentIntegerConstant
        << ASArgExpr->getSourceRange();
      Attr.setInvalid();
      return;
    }

    // Bounds checking.
    if (addrSpace.isSigned()) {
      if (addrSpace.isNegative()) {
        S.Diag(Attr.getLoc(), diag::err_attribute_address_space_negative)
          << ASArgExpr->getSourceRange();
        Attr.setInvalid();
        return;
      }
      addrSpace.setIsSigned(false);
    }
    llvm::APSInt max(addrSpace.getBitWidth());
    max = Qualifiers::MaxAddressSpace;
    if (addrSpace > max) {
      S.Diag(Attr.getLoc(), diag::err_attribute_address_space_too_high)
        << int(Qualifiers::MaxAddressSpace) << ASArgExpr->getSourceRange();
      Attr.setInvalid();
      return;
    }
    ASIdx = static_cast<unsigned>(addrSpace.getZExtValue());
  } else {
    // The keyword-based type attributes imply which address space to use.
    switch (Attr.getKind()) {
    case AttributeList::AT_OpenCLGlobalAddressSpace:
      ASIdx = LangAS::opencl_global; break;
    case AttributeList::AT_OpenCLLocalAddressSpace:
      ASIdx = LangAS::opencl_local; break;
    case AttributeList::AT_OpenCLConstantAddressSpace:
      ASIdx = LangAS::opencl_constant; break;
    case AttributeList::AT_OpenCLGenericAddressSpace:
      ASIdx = LangAS::opencl_generic; break;
    default:
      assert(Attr.getKind() == AttributeList::AT_OpenCLPrivateAddressSpace);
      ASIdx = 0; break;
    }
  }
  
  Type = S.Context.getAddrSpaceQualType(Type, ASIdx);
}

/// Does this type have a "direct" ownership qualifier?  That is,
/// is it written like "__strong id", as opposed to something like
/// "typeof(foo)", where that happens to be strong?
static bool hasDirectOwnershipQualifier(QualType type) {
  // Fast path: no qualifier at all.
  assert(type.getQualifiers().hasObjCLifetime());

  while (true) {
    // __strong id
    if (const AttributedType *attr = dyn_cast<AttributedType>(type)) {
      if (attr->getAttrKind() == AttributedType::attr_objc_ownership)
        return true;

      type = attr->getModifiedType();

    // X *__strong (...)
    } else if (const ParenType *paren = dyn_cast<ParenType>(type)) {
      type = paren->getInnerType();

    // That's it for things we want to complain about.  In particular,
    // we do not want to look through typedefs, typeof(expr),
    // typeof(type), or any other way that the type is somehow
    // abstracted.
    } else {

      return false;
    }
  }
}

/// handleObjCOwnershipTypeAttr - Process an objc_ownership
/// attribute on the specified type.
///
/// Returns 'true' if the attribute was handled.
static bool handleObjCOwnershipTypeAttr(TypeProcessingState &state,
                                       AttributeList &attr,
                                       QualType &type) {
  bool NonObjCPointer = false;

  if (!type->isDependentType() && !type->isUndeducedType()) {
    if (const PointerType *ptr = type->getAs<PointerType>()) {
      QualType pointee = ptr->getPointeeType();
      if (pointee->isObjCRetainableType() || pointee->isPointerType())
        return false;
      // It is important not to lose the source info that there was an attribute
      // applied to non-objc pointer. We will create an attributed type but
      // its type will be the same as the original type.
      NonObjCPointer = true;
    } else if (!type->isObjCRetainableType()) {
      return false;
    }

    // Don't accept an ownership attribute in the declspec if it would
    // just be the return type of a block pointer.
    if (state.isProcessingDeclSpec()) {
      Declarator &D = state.getDeclarator();
      if (maybeMovePastReturnType(D, D.getNumTypeObjects(),
                                  /*onlyBlockPointers=*/true))
        return false;
    }
  }

  Sema &S = state.getSema();
  SourceLocation AttrLoc = attr.getLoc();
  if (AttrLoc.isMacroID())
    AttrLoc = S.getSourceManager().getImmediateExpansionRange(AttrLoc).first;

  if (!attr.isArgIdent(0)) {
    S.Diag(AttrLoc, diag::err_attribute_argument_type)
      << attr.getName() << AANT_ArgumentString;
    attr.setInvalid();
    return true;
  }

  // Consume lifetime attributes without further comment outside of
  // ARC mode.
  if (!S.getLangOpts().ObjCAutoRefCount)
    return true;

  IdentifierInfo *II = attr.getArgAsIdent(0)->Ident;
  Qualifiers::ObjCLifetime lifetime;
  if (II->isStr("none"))
    lifetime = Qualifiers::OCL_ExplicitNone;
  else if (II->isStr("strong"))
    lifetime = Qualifiers::OCL_Strong;
  else if (II->isStr("weak"))
    lifetime = Qualifiers::OCL_Weak;
  else if (II->isStr("autoreleasing"))
    lifetime = Qualifiers::OCL_Autoreleasing;
  else {
    S.Diag(AttrLoc, diag::warn_attribute_type_not_supported)
      << attr.getName() << II;
    attr.setInvalid();
    return true;
  }

  SplitQualType underlyingType = type.split();

  // Check for redundant/conflicting ownership qualifiers.
  if (Qualifiers::ObjCLifetime previousLifetime
        = type.getQualifiers().getObjCLifetime()) {
    // If it's written directly, that's an error.
    if (hasDirectOwnershipQualifier(type)) {
      S.Diag(AttrLoc, diag::err_attr_objc_ownership_redundant)
        << type;
      return true;
    }

    // Otherwise, if the qualifiers actually conflict, pull sugar off
    // until we reach a type that is directly qualified.
    if (previousLifetime != lifetime) {
      // This should always terminate: the canonical type is
      // qualified, so some bit of sugar must be hiding it.
      while (!underlyingType.Quals.hasObjCLifetime()) {
        underlyingType = underlyingType.getSingleStepDesugaredType();
      }
      underlyingType.Quals.removeObjCLifetime();
    }
  }

  underlyingType.Quals.addObjCLifetime(lifetime);

  if (NonObjCPointer) {
    StringRef name = attr.getName()->getName();
    switch (lifetime) {
    case Qualifiers::OCL_None:
    case Qualifiers::OCL_ExplicitNone:
      break;
    case Qualifiers::OCL_Strong: name = "__strong"; break;
    case Qualifiers::OCL_Weak: name = "__weak"; break;
    case Qualifiers::OCL_Autoreleasing: name = "__autoreleasing"; break;
    }
    S.Diag(AttrLoc, diag::warn_type_attribute_wrong_type) << name
      << TDS_ObjCObjOrBlock << type;
  }

  QualType origType = type;
  if (!NonObjCPointer)
    type = S.Context.getQualifiedType(underlyingType);

  // If we have a valid source location for the attribute, use an
  // AttributedType instead.
  if (AttrLoc.isValid())
    type = S.Context.getAttributedType(AttributedType::attr_objc_ownership,
                                       origType, type);

  // Forbid __weak if the runtime doesn't support it.
  if (lifetime == Qualifiers::OCL_Weak &&
      !S.getLangOpts().ObjCARCWeak && !NonObjCPointer) {

    // Actually, delay this until we know what we're parsing.
    if (S.DelayedDiagnostics.shouldDelayDiagnostics()) {
      S.DelayedDiagnostics.add(
          sema::DelayedDiagnostic::makeForbiddenType(
              S.getSourceManager().getExpansionLoc(AttrLoc),
              diag::err_arc_weak_no_runtime, type, /*ignored*/ 0));
    } else {
      S.Diag(AttrLoc, diag::err_arc_weak_no_runtime);
    }

    attr.setInvalid();
    return true;
  }

  // Forbid __weak for class objects marked as
  // objc_arc_weak_reference_unavailable
  if (lifetime == Qualifiers::OCL_Weak) {
    if (const ObjCObjectPointerType *ObjT =
          type->getAs<ObjCObjectPointerType>()) {
      if (ObjCInterfaceDecl *Class = ObjT->getInterfaceDecl()) {
        if (Class->isArcWeakrefUnavailable()) {
            S.Diag(AttrLoc, diag::err_arc_unsupported_weak_class);
            S.Diag(ObjT->getInterfaceDecl()->getLocation(),
                   diag::note_class_declared);
        }
      }
    }
  }

  return true;
}

/// handleObjCGCTypeAttr - Process the __attribute__((objc_gc)) type
/// attribute on the specified type.  Returns true to indicate that
/// the attribute was handled, false to indicate that the type does
/// not permit the attribute.
static bool handleObjCGCTypeAttr(TypeProcessingState &state,
                                 AttributeList &attr,
                                 QualType &type) {
  Sema &S = state.getSema();

  // Delay if this isn't some kind of pointer.
  if (!type->isPointerType() &&
      !type->isObjCObjectPointerType() &&
      !type->isBlockPointerType())
    return false;

  if (type.getObjCGCAttr() != Qualifiers::GCNone) {
    S.Diag(attr.getLoc(), diag::err_attribute_multiple_objc_gc);
    attr.setInvalid();
    return true;
  }
  
  // Check the attribute arguments.
  if (!attr.isArgIdent(0)) {
    S.Diag(attr.getLoc(), diag::err_attribute_argument_type)
      << attr.getName() << AANT_ArgumentString;
    attr.setInvalid();
    return true;
  }
  Qualifiers::GC GCAttr;
  if (attr.getNumArgs() > 1) {
    S.Diag(attr.getLoc(), diag::err_attribute_wrong_number_arguments)
      << attr.getName() << 1;
    attr.setInvalid();
    return true;
  }

  IdentifierInfo *II = attr.getArgAsIdent(0)->Ident;
  if (II->isStr("weak"))
    GCAttr = Qualifiers::Weak;
  else if (II->isStr("strong"))
    GCAttr = Qualifiers::Strong;
  else {
    S.Diag(attr.getLoc(), diag::warn_attribute_type_not_supported)
      << attr.getName() << II;
    attr.setInvalid();
    return true;
  }

  QualType origType = type;
  type = S.Context.getObjCGCQualType(origType, GCAttr);

  // Make an attributed type to preserve the source information.
  if (attr.getLoc().isValid())
    type = S.Context.getAttributedType(AttributedType::attr_objc_gc,
                                       origType, type);

  return true;
}

namespace {
  /// A helper class to unwrap a type down to a function for the
  /// purposes of applying attributes there.
  ///
  /// Use:
  ///   FunctionTypeUnwrapper unwrapped(SemaRef, T);
  ///   if (unwrapped.isFunctionType()) {
  ///     const FunctionType *fn = unwrapped.get();
  ///     // change fn somehow
  ///     T = unwrapped.wrap(fn);
  ///   }
  struct FunctionTypeUnwrapper {
    enum WrapKind {
      Desugar,
      Parens,
      Pointer,
      BlockPointer,
      Reference,
      MemberPointer
    };

    QualType Original;
    const FunctionType *Fn;
    SmallVector<unsigned char /*WrapKind*/, 8> Stack;

    FunctionTypeUnwrapper(Sema &S, QualType T) : Original(T) {
      while (true) {
        const Type *Ty = T.getTypePtr();
        if (isa<FunctionType>(Ty)) {
          Fn = cast<FunctionType>(Ty);
          return;
        } else if (isa<ParenType>(Ty)) {
          T = cast<ParenType>(Ty)->getInnerType();
          Stack.push_back(Parens);
        } else if (isa<PointerType>(Ty)) {
          T = cast<PointerType>(Ty)->getPointeeType();
          Stack.push_back(Pointer);
        } else if (isa<BlockPointerType>(Ty)) {
          T = cast<BlockPointerType>(Ty)->getPointeeType();
          Stack.push_back(BlockPointer);
        } else if (isa<MemberPointerType>(Ty)) {
          T = cast<MemberPointerType>(Ty)->getPointeeType();
          Stack.push_back(MemberPointer);
        } else if (isa<ReferenceType>(Ty)) {
          T = cast<ReferenceType>(Ty)->getPointeeType();
          Stack.push_back(Reference);
        } else {
          const Type *DTy = Ty->getUnqualifiedDesugaredType();
          if (Ty == DTy) {
            Fn = nullptr;
            return;
          }

          T = QualType(DTy, 0);
          Stack.push_back(Desugar);
        }
      }
    }

    bool isFunctionType() const { return (Fn != nullptr); }
    const FunctionType *get() const { return Fn; }

    QualType wrap(Sema &S, const FunctionType *New) {
      // If T wasn't modified from the unwrapped type, do nothing.
      if (New == get()) return Original;

      Fn = New;
      return wrap(S.Context, Original, 0);
    }

  private:
    QualType wrap(ASTContext &C, QualType Old, unsigned I) {
      if (I == Stack.size())
        return C.getQualifiedType(Fn, Old.getQualifiers());

      // Build up the inner type, applying the qualifiers from the old
      // type to the new type.
      SplitQualType SplitOld = Old.split();

      // As a special case, tail-recurse if there are no qualifiers.
      if (SplitOld.Quals.empty())
        return wrap(C, SplitOld.Ty, I);
      return C.getQualifiedType(wrap(C, SplitOld.Ty, I), SplitOld.Quals);
    }

    QualType wrap(ASTContext &C, const Type *Old, unsigned I) {
      if (I == Stack.size()) return QualType(Fn, 0);

      switch (static_cast<WrapKind>(Stack[I++])) {
      case Desugar:
        // This is the point at which we potentially lose source
        // information.
        return wrap(C, Old->getUnqualifiedDesugaredType(), I);

      case Parens: {
        QualType New = wrap(C, cast<ParenType>(Old)->getInnerType(), I);
        return C.getParenType(New);
      }

      case Pointer: {
        QualType New = wrap(C, cast<PointerType>(Old)->getPointeeType(), I);
        return C.getPointerType(New);
      }

      case BlockPointer: {
        QualType New = wrap(C, cast<BlockPointerType>(Old)->getPointeeType(),I);
        return C.getBlockPointerType(New);
      }

      case MemberPointer: {
        const MemberPointerType *OldMPT = cast<MemberPointerType>(Old);
        QualType New = wrap(C, OldMPT->getPointeeType(), I);
        return C.getMemberPointerType(New, OldMPT->getClass());
      }

      case Reference: {
        const ReferenceType *OldRef = cast<ReferenceType>(Old);
        QualType New = wrap(C, OldRef->getPointeeType(), I);
        if (isa<LValueReferenceType>(OldRef))
          return C.getLValueReferenceType(New, OldRef->isSpelledAsLValue());
        else
          return C.getRValueReferenceType(New);
      }
      }

      llvm_unreachable("unknown wrapping kind");
    }
  };
}

static bool handleMSPointerTypeQualifierAttr(TypeProcessingState &State,
                                             AttributeList &Attr,
                                             QualType &Type) {
  Sema &S = State.getSema();

  AttributeList::Kind Kind = Attr.getKind();
  QualType Desugared = Type;
  const AttributedType *AT = dyn_cast<AttributedType>(Type);
  while (AT) {
    AttributedType::Kind CurAttrKind = AT->getAttrKind();

    // You cannot specify duplicate type attributes, so if the attribute has
    // already been applied, flag it.
    if (getAttrListKind(CurAttrKind) == Kind) {
      S.Diag(Attr.getLoc(), diag::warn_duplicate_attribute_exact)
        << Attr.getName();
      return true;
    }

    // You cannot have both __sptr and __uptr on the same type, nor can you
    // have __ptr32 and __ptr64.
    if ((CurAttrKind == AttributedType::attr_ptr32 &&
         Kind == AttributeList::AT_Ptr64) ||
        (CurAttrKind == AttributedType::attr_ptr64 &&
         Kind == AttributeList::AT_Ptr32)) {
      S.Diag(Attr.getLoc(), diag::err_attributes_are_not_compatible)
        << "'__ptr32'" << "'__ptr64'";
      return true;
    } else if ((CurAttrKind == AttributedType::attr_sptr &&
                Kind == AttributeList::AT_UPtr) ||
               (CurAttrKind == AttributedType::attr_uptr &&
                Kind == AttributeList::AT_SPtr)) {
      S.Diag(Attr.getLoc(), diag::err_attributes_are_not_compatible)
        << "'__sptr'" << "'__uptr'";
      return true;
    }
    
    Desugared = AT->getEquivalentType();
    AT = dyn_cast<AttributedType>(Desugared);
  }

  // Pointer type qualifiers can only operate on pointer types, but not
  // pointer-to-member types.
  if (!isa<PointerType>(Desugared)) {
    S.Diag(Attr.getLoc(), Type->isMemberPointerType() ?
                          diag::err_attribute_no_member_pointers :
                          diag::err_attribute_pointers_only) << Attr.getName();
    return true;
  }

  AttributedType::Kind TAK;
  switch (Kind) {
  default: llvm_unreachable("Unknown attribute kind");
  case AttributeList::AT_Ptr32: TAK = AttributedType::attr_ptr32; break;
  case AttributeList::AT_Ptr64: TAK = AttributedType::attr_ptr64; break;
  case AttributeList::AT_SPtr: TAK = AttributedType::attr_sptr; break;
  case AttributeList::AT_UPtr: TAK = AttributedType::attr_uptr; break;
  }

  Type = S.Context.getAttributedType(TAK, Type, Type);
  return false;
}

bool Sema::checkNullabilityTypeSpecifier(QualType &type,
                                         NullabilityKind nullability,
                                         SourceLocation nullabilityLoc,
                                         bool isContextSensitive) {
  // We saw a nullability type specifier. If this is the first one for
  // this file, note that.
  FileID file = getNullabilityCompletenessCheckFileID(*this, nullabilityLoc);
  if (!file.isInvalid()) {
    FileNullability &fileNullability = NullabilityMap[file];
    if (!fileNullability.SawTypeNullability) {
      // If we have already seen a pointer declarator without a nullability
      // annotation, complain about it.
      if (fileNullability.PointerLoc.isValid()) {
        Diag(fileNullability.PointerLoc, diag::warn_nullability_missing)
          << static_cast<unsigned>(fileNullability.PointerKind);
      }

      fileNullability.SawTypeNullability = true;
    }
  }

  // Check for existing nullability attributes on the type.
  QualType desugared = type;
  while (auto attributed = dyn_cast<AttributedType>(desugared.getTypePtr())) {
    // Check whether there is already a null
    if (auto existingNullability = attributed->getImmediateNullability()) {
      // Duplicated nullability.
      if (nullability == *existingNullability) {
        Diag(nullabilityLoc, diag::warn_nullability_duplicate)
          << DiagNullabilityKind(nullability, isContextSensitive)
          << FixItHint::CreateRemoval(nullabilityLoc);

        break;
      } 

      // Conflicting nullability.
      Diag(nullabilityLoc, diag::err_nullability_conflicting)
        << DiagNullabilityKind(nullability, isContextSensitive)
        << DiagNullabilityKind(*existingNullability, false);
      return true;
    }

    desugared = attributed->getModifiedType();
  }

  // If there is already a different nullability specifier, complain.
  // This (unlike the code above) looks through typedefs that might
  // have nullability specifiers on them, which means we cannot
  // provide a useful Fix-It.
  if (auto existingNullability = desugared->getNullability(Context)) {
    if (nullability != *existingNullability) {
      Diag(nullabilityLoc, diag::err_nullability_conflicting)
        << DiagNullabilityKind(nullability, isContextSensitive)
        << DiagNullabilityKind(*existingNullability, false);

      // Try to find the typedef with the existing nullability specifier.
      if (auto typedefType = desugared->getAs<TypedefType>()) {
        TypedefNameDecl *typedefDecl = typedefType->getDecl();
        QualType underlyingType = typedefDecl->getUnderlyingType();
        if (auto typedefNullability
              = AttributedType::stripOuterNullability(underlyingType)) {
          if (*typedefNullability == *existingNullability) {
            Diag(typedefDecl->getLocation(), diag::note_nullability_here)
              << DiagNullabilityKind(*existingNullability, false);
          }
        }
      }

      return true;
    }
  }

  // If this definitely isn't a pointer type, reject the specifier.
  if (!desugared->canHaveNullability()) {
    Diag(nullabilityLoc, diag::err_nullability_nonpointer)
      << DiagNullabilityKind(nullability, isContextSensitive) << type;
    return true;
  }
  
  // For the context-sensitive keywords/Objective-C property
  // attributes, require that the type be a single-level pointer.
  if (isContextSensitive) {
    // Make sure that the pointee isn't itself a pointer type.
    QualType pointeeType = desugared->getPointeeType();
    if (pointeeType->isAnyPointerType() ||
        pointeeType->isObjCObjectPointerType() ||
        pointeeType->isMemberPointerType()) {
      Diag(nullabilityLoc, diag::err_nullability_cs_multilevel)
        << DiagNullabilityKind(nullability, true)
        << type;
      Diag(nullabilityLoc, diag::note_nullability_type_specifier)
        << DiagNullabilityKind(nullability, false)
        << type
        << FixItHint::CreateReplacement(nullabilityLoc,
                                        getNullabilitySpelling(nullability));
      return true;
    }
  }

  // Form the attributed type.
  type = Context.getAttributedType(
           AttributedType::getNullabilityAttrKind(nullability), type, type);
  return false;
}

bool Sema::checkObjCKindOfType(QualType &type, SourceLocation loc) {
  // Find out if it's an Objective-C object or object pointer type;
  const ObjCObjectPointerType *ptrType = type->getAs<ObjCObjectPointerType>();
  const ObjCObjectType *objType = ptrType ? ptrType->getObjectType() 
                                          : type->getAs<ObjCObjectType>();

  // If not, we can't apply __kindof.
  if (!objType) {
    // FIXME: Handle dependent types that aren't yet object types.
    Diag(loc, diag::err_objc_kindof_nonobject)
      << type;
    return true;
  }

  // Rebuild the "equivalent" type, which pushes __kindof down into
  // the object type.
  QualType equivType = Context.getObjCObjectType(objType->getBaseType(),
                                                 objType->getTypeArgsAsWritten(),
                                                 objType->getProtocols(),
                                                 /*isKindOf=*/true);

  // If we started with an object pointer type, rebuild it.
  if (ptrType) {
    equivType = Context.getObjCObjectPointerType(equivType);
    if (auto nullability = type->getNullability(Context)) {
      auto attrKind = AttributedType::getNullabilityAttrKind(*nullability);
      equivType = Context.getAttributedType(attrKind, equivType, equivType);
    }
  }

  // Build the attributed type to record where __kindof occurred.
  type = Context.getAttributedType(AttributedType::attr_objc_kindof, 
                                   type,
                                   equivType);

  return false;
}

/// Map a nullability attribute kind to a nullability kind.
static NullabilityKind mapNullabilityAttrKind(AttributeList::Kind kind) {
  switch (kind) {
  case AttributeList::AT_TypeNonNull:
    return NullabilityKind::NonNull;

  case AttributeList::AT_TypeNullable:
    return NullabilityKind::Nullable;

  case AttributeList::AT_TypeNullUnspecified:
    return NullabilityKind::Unspecified;

  default:
    llvm_unreachable("not a nullability attribute kind");
  }
}

/// Distribute a nullability type attribute that cannot be applied to
/// the type specifier to a pointer, block pointer, or member pointer
/// declarator, complaining if necessary.
///
/// \returns true if the nullability annotation was distributed, false
/// otherwise.
static bool distributeNullabilityTypeAttr(TypeProcessingState &state,
                                          QualType type,
                                          AttributeList &attr) {
  Declarator &declarator = state.getDeclarator();

  /// Attempt to move the attribute to the specified chunk.
  auto moveToChunk = [&](DeclaratorChunk &chunk, bool inFunction) -> bool {
    // If there is already a nullability attribute there, don't add
    // one.
    if (hasNullabilityAttr(chunk.getAttrListRef()))
      return false;

    // Complain about the nullability qualifier being in the wrong
    // place.
    enum {
      PK_Pointer,
      PK_BlockPointer,
      PK_MemberPointer,
      PK_FunctionPointer,
      PK_MemberFunctionPointer,
    } pointerKind
      = chunk.Kind == DeclaratorChunk::Pointer ? (inFunction ? PK_FunctionPointer
                                                             : PK_Pointer)
        : chunk.Kind == DeclaratorChunk::BlockPointer ? PK_BlockPointer
        : inFunction? PK_MemberFunctionPointer : PK_MemberPointer;

    auto diag = state.getSema().Diag(attr.getLoc(),
                                     diag::warn_nullability_declspec)
      << DiagNullabilityKind(mapNullabilityAttrKind(attr.getKind()),
                             attr.isContextSensitiveKeywordAttribute())
      << type
      << static_cast<unsigned>(pointerKind);

    // FIXME: MemberPointer chunks don't carry the location of the *.
    if (chunk.Kind != DeclaratorChunk::MemberPointer) {
      diag << FixItHint::CreateRemoval(attr.getLoc())
           << FixItHint::CreateInsertion(
                state.getSema().getPreprocessor()
                  .getLocForEndOfToken(chunk.Loc),
                " " + attr.getName()->getName().str() + " ");
    }

    moveAttrFromListToList(attr, state.getCurrentAttrListRef(),
                           chunk.getAttrListRef());
    return true;
  };

  // Move it to the outermost pointer, member pointer, or block
  // pointer declarator.
  for (unsigned i = state.getCurrentChunkIndex(); i != 0; --i) {
    DeclaratorChunk &chunk = declarator.getTypeObject(i-1);
    switch (chunk.Kind) {
    case DeclaratorChunk::Pointer:
    case DeclaratorChunk::BlockPointer:
    case DeclaratorChunk::MemberPointer:
      return moveToChunk(chunk, false);

    case DeclaratorChunk::Paren:
    case DeclaratorChunk::Array:
      continue;

    case DeclaratorChunk::Function:
      // Try to move past the return type to a function/block/member
      // function pointer.
      if (DeclaratorChunk *dest = maybeMovePastReturnType(
                                    declarator, i,
                                    /*onlyBlockPointers=*/false)) {
        return moveToChunk(*dest, true);
      }

      return false;
      
    // Don't walk through these.
    case DeclaratorChunk::Reference:
      return false;
    }
  }

  return false;
}

// HLSL Change Starts
static bool isHLSLTypeAttr(AttributeList::Kind Kind) {
  switch (Kind) {
  case AttributeList::AT_HLSLRowMajor:
  case AttributeList::AT_HLSLColumnMajor:
  case AttributeList::AT_HLSLSnorm:
  case AttributeList::AT_HLSLUnorm:
  case AttributeList::AT_HLSLGloballyCoherent:
    return true;
  default:
    // Only meant to catch attr handled by handleHLSLTypeAttr, ignore the rest
    return false;
  }
}

static bool handleHLSLTypeAttr(TypeProcessingState &State,
                               AttributeList &Attr,
                               QualType &Type) {
  // Return true on error
  Sema &S = State.getSema();

  AttributeList::Kind Kind = Attr.getKind();

  // Check for attributes on incorrect types
  if ((Kind == AttributeList::AT_HLSLColumnMajor ||
       Kind == AttributeList::AT_HLSLRowMajor) &&
      !hlsl::IsMatrixType(&S, Type)) {
    S.Diag(Attr.getLoc(), diag::err_hlsl_matrix_layout_wrong_type) << Attr.getName() << Attr.getRange();
    return true;
  } else if ((Kind == AttributeList::AT_HLSLSnorm ||
              Kind == AttributeList::AT_HLSLUnorm) &&
              !hlsl::GetOriginalElementType(&S, Type)->isFloatingType()) {
    S.Diag(Attr.getLoc(), diag::err_hlsl_norm_float_only) << Attr.getRange();
    return true;
  }

  const AttributedType *pMatrixOrientation = nullptr;
  const AttributedType *pNorm = nullptr;
  const AttributedType *pGLC = nullptr;
  hlsl::GetHLSLAttributedTypes(&S, Type, &pMatrixOrientation, &pNorm, &pGLC);

  if (pMatrixOrientation &&
    (Kind == AttributeList::AT_HLSLColumnMajor ||
    Kind == AttributeList::AT_HLSLRowMajor))
  {
    AttributedType::Kind CurAttrKind = pMatrixOrientation->getAttrKind();
    if (Kind == getAttrListKind(CurAttrKind))
    {
      S.Diag(Attr.getLoc(), diag::warn_duplicate_attribute_exact)
        << Attr.getName() << Attr.getRange();
    } else {
      S.Diag(Attr.getLoc(), diag::err_attributes_are_not_compatible)
        << "'row_major'" << "'column_major'" << Attr.getRange();
    }
    return true;
  }

  if (pNorm &&
    (Kind == AttributeList::AT_HLSLSnorm ||
    Kind == AttributeList::AT_HLSLUnorm))
  {
    AttributedType::Kind CurAttrKind = pNorm->getAttrKind();
    if (Kind == getAttrListKind(CurAttrKind))
    {
      S.Diag(Attr.getLoc(), diag::warn_duplicate_attribute_exact)
        << Attr.getName() << Attr.getRange();
    } else {
      S.Diag(Attr.getLoc(), diag::err_attributes_are_not_compatible)
        << "'unorm'" << "'snorm'" << Attr.getRange();
    }
    return true;
  }

  if (pGLC && Kind == AttributeList::AT_HLSLGloballyCoherent) {
    AttributedType::Kind CurAttrKind = pGLC->getAttrKind();
    if (Kind == getAttrListKind(CurAttrKind)) {
      S.Diag(Attr.getLoc(), diag::warn_duplicate_attribute_exact)
          << Attr.getName() << Attr.getRange();
    }
  }

  AttributedType::Kind TAK;
  switch (Kind) {
  default: llvm_unreachable("Unknown attribute kind");
  case AttributeList::AT_HLSLRowMajor:    TAK = AttributedType::attr_hlsl_row_major; break;
  case AttributeList::AT_HLSLColumnMajor: TAK = AttributedType::attr_hlsl_column_major; break;
  case AttributeList::AT_HLSLUnorm:       TAK = AttributedType::attr_hlsl_unorm; break;
  case AttributeList::AT_HLSLSnorm:       TAK = AttributedType::attr_hlsl_snorm; break;
  case AttributeList::AT_HLSLGloballyCoherent:
    TAK = AttributedType::attr_hlsl_globallycoherent; break;
  }

  Type = S.Context.getAttributedType(TAK, Type, Type);
  return false;
}
// HLSL Change Ends

static AttributedType::Kind getCCTypeAttrKind(AttributeList &Attr) {
  assert(!Attr.isInvalid());
  switch (Attr.getKind()) {
  default:
    llvm_unreachable("not a calling convention attribute");
  case AttributeList::AT_CDecl:
    return AttributedType::attr_cdecl;
  case AttributeList::AT_FastCall:
    return AttributedType::attr_fastcall;
  case AttributeList::AT_StdCall:
    return AttributedType::attr_stdcall;
  case AttributeList::AT_ThisCall:
    return AttributedType::attr_thiscall;
  case AttributeList::AT_Pascal:
    return AttributedType::attr_pascal;
  case AttributeList::AT_VectorCall:
    return AttributedType::attr_vectorcall;
  case AttributeList::AT_Pcs: {
    // The attribute may have had a fixit applied where we treated an
    // identifier as a string literal.  The contents of the string are valid,
    // but the form may not be.
    StringRef Str;
    if (Attr.isArgExpr(0))
      Str = cast<StringLiteral>(Attr.getArgAsExpr(0))->getString();
    else
      Str = Attr.getArgAsIdent(0)->Ident->getName();
    return llvm::StringSwitch<AttributedType::Kind>(Str)
        .Case("aapcs", AttributedType::attr_pcs)
        .Case("aapcs-vfp", AttributedType::attr_pcs_vfp);
  }
  case AttributeList::AT_IntelOclBicc:
    return AttributedType::attr_inteloclbicc;
  case AttributeList::AT_MSABI:
    return AttributedType::attr_ms_abi;
  case AttributeList::AT_SysVABI:
    return AttributedType::attr_sysv_abi;
  }
  llvm_unreachable("unexpected attribute kind!");
}

/// Process an individual function attribute.  Returns true to
/// indicate that the attribute was handled, false if it wasn't.
static bool handleFunctionTypeAttr(TypeProcessingState &state,
                                   AttributeList &attr,
                                   QualType &type) {
  Sema &S = state.getSema();

  FunctionTypeUnwrapper unwrapped(S, type);

  if (attr.getKind() == AttributeList::AT_NoReturn) {
    if (S.CheckNoReturnAttr(attr))
      return true;

    // Delay if this is not a function type.
    if (!unwrapped.isFunctionType())
      return false;

    // Otherwise we can process right away.
    FunctionType::ExtInfo EI = unwrapped.get()->getExtInfo().withNoReturn(true);
    type = unwrapped.wrap(S, S.Context.adjustFunctionType(unwrapped.get(), EI));
    return true;
  }

  // ns_returns_retained is not always a type attribute, but if we got
  // here, we're treating it as one right now.
  if (attr.getKind() == AttributeList::AT_NSReturnsRetained) {
    assert(S.getLangOpts().ObjCAutoRefCount &&
           "ns_returns_retained treated as type attribute in non-ARC");
    if (attr.getNumArgs()) return true;

    // Delay if this is not a function type.
    if (!unwrapped.isFunctionType())
      return false;

    FunctionType::ExtInfo EI
      = unwrapped.get()->getExtInfo().withProducesResult(true);
    type = unwrapped.wrap(S, S.Context.adjustFunctionType(unwrapped.get(), EI));
    return true;
  }

  if (attr.getKind() == AttributeList::AT_Regparm) {
    unsigned value;
    if (S.CheckRegparmAttr(attr, value))
      return true;

    // Delay if this is not a function type.
    if (!unwrapped.isFunctionType())
      return false;

    // Diagnose regparm with fastcall.
    const FunctionType *fn = unwrapped.get();
    CallingConv CC = fn->getCallConv();
    if (CC == CC_X86FastCall) {
      S.Diag(attr.getLoc(), diag::err_attributes_are_not_compatible)
        << FunctionType::getNameForCallConv(CC)
        << "regparm";
      attr.setInvalid();
      return true;
    }

    FunctionType::ExtInfo EI =
      unwrapped.get()->getExtInfo().withRegParm(value);
    type = unwrapped.wrap(S, S.Context.adjustFunctionType(unwrapped.get(), EI));
    return true;
  }

  // Delay if the type didn't work out to a function.
  if (!unwrapped.isFunctionType()) return false;

  // Otherwise, a calling convention.
  CallingConv CC;
  if (S.CheckCallingConvAttr(attr, CC))
    return true;

  const FunctionType *fn = unwrapped.get();
  CallingConv CCOld = fn->getCallConv();
  AttributedType::Kind CCAttrKind = getCCTypeAttrKind(attr);

  if (CCOld != CC) {
    // Error out on when there's already an attribute on the type
    // and the CCs don't match.
    const AttributedType *AT = S.getCallingConvAttributedType(type);
    if (AT && AT->getAttrKind() != CCAttrKind) {
      S.Diag(attr.getLoc(), diag::err_attributes_are_not_compatible)
        << FunctionType::getNameForCallConv(CC)
        << FunctionType::getNameForCallConv(CCOld);
      attr.setInvalid();
      return true;
    }
  }

  // Diagnose use of callee-cleanup calling convention on variadic functions.
  if (!supportsVariadicCall(CC)) {
    const FunctionProtoType *FnP = dyn_cast<FunctionProtoType>(fn);
    if (FnP && FnP->isVariadic()) {
      unsigned DiagID = diag::err_cconv_varargs;
      // stdcall and fastcall are ignored with a warning for GCC and MS
      // compatibility.
      if (CC == CC_X86StdCall || CC == CC_X86FastCall)
        DiagID = diag::warn_cconv_varargs;

      S.Diag(attr.getLoc(), DiagID) << FunctionType::getNameForCallConv(CC);
      attr.setInvalid();
      return true;
    }
  }

  // Also diagnose fastcall with regparm.
  if (CC == CC_X86FastCall && fn->getHasRegParm()) {
    S.Diag(attr.getLoc(), diag::err_attributes_are_not_compatible)
        << "regparm" << FunctionType::getNameForCallConv(CC_X86FastCall);
    attr.setInvalid();
    return true;
  }

  // Modify the CC from the wrapped function type, wrap it all back, and then
  // wrap the whole thing in an AttributedType as written.  The modified type
  // might have a different CC if we ignored the attribute.
  FunctionType::ExtInfo EI = unwrapped.get()->getExtInfo().withCallingConv(CC);
  QualType Equivalent =
      unwrapped.wrap(S, S.Context.adjustFunctionType(unwrapped.get(), EI));
  type = S.Context.getAttributedType(CCAttrKind, type, Equivalent);
  return true;
}

bool Sema::hasExplicitCallingConv(QualType &T) {
  QualType R = T.IgnoreParens();
  while (const AttributedType *AT = dyn_cast<AttributedType>(R)) {
    if (AT->isCallingConv())
      return true;
    R = AT->getModifiedType().IgnoreParens();
  }
  return false;
}

void Sema::adjustMemberFunctionCC(QualType &T, bool IsStatic) {
  FunctionTypeUnwrapper Unwrapped(*this, T);
  const FunctionType *FT = Unwrapped.get();
  bool IsVariadic = (isa<FunctionProtoType>(FT) &&
                     cast<FunctionProtoType>(FT)->isVariadic());

  // Only adjust types with the default convention.  For example, on Windows we
  // should adjust a __cdecl type to __thiscall for instance methods, and a
  // __thiscall type to __cdecl for static methods.
  CallingConv CurCC = FT->getCallConv();
  CallingConv FromCC =
      Context.getDefaultCallingConvention(IsVariadic, IsStatic);
  CallingConv ToCC = Context.getDefaultCallingConvention(IsVariadic, !IsStatic);
  if (CurCC != FromCC || FromCC == ToCC)
    return;

  if (hasExplicitCallingConv(T))
    return;

  FT = Context.adjustFunctionType(FT, FT->getExtInfo().withCallingConv(ToCC));
  QualType Wrapped = Unwrapped.wrap(*this, FT);
  T = Context.getAdjustedType(T, Wrapped);
}

/// HandleVectorSizeAttribute - this attribute is only applicable to integral
/// and float scalars, although arrays, pointers, and function return values are
/// allowed in conjunction with this construct. Aggregates with this attribute
/// are invalid, even if they are of the same size as a corresponding scalar.
/// The raw attribute should contain precisely 1 argument, the vector size for
/// the variable, measured in bytes. If curType and rawAttr are well formed,
/// this routine will return a new vector type.
static void HandleVectorSizeAttr(QualType& CurType, const AttributeList &Attr,
                                 Sema &S) {
  // Check the attribute arguments.
  if (Attr.getNumArgs() != 1) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_number_arguments)
      << Attr.getName() << 1;
    Attr.setInvalid();
    return;
  }
  Expr *sizeExpr = static_cast<Expr *>(Attr.getArgAsExpr(0));
  llvm::APSInt vecSize(32);
  if (sizeExpr->isTypeDependent() || sizeExpr->isValueDependent() ||
      !sizeExpr->isIntegerConstantExpr(vecSize, S.Context)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_argument_type)
      << Attr.getName() << AANT_ArgumentIntegerConstant
      << sizeExpr->getSourceRange();
    Attr.setInvalid();
    return;
  }
  // The base type must be integer (not Boolean or enumeration) or float, and
  // can't already be a vector.
  if (!CurType->isBuiltinType() || CurType->isBooleanType() ||
      (!CurType->isIntegerType() && !CurType->isRealFloatingType())) {
    S.Diag(Attr.getLoc(), diag::err_attribute_invalid_vector_type) << CurType;
    Attr.setInvalid();
    return;
  }
  unsigned typeSize = static_cast<unsigned>(S.Context.getTypeSize(CurType));
  // vecSize is specified in bytes - convert to bits.
  unsigned vectorSize = static_cast<unsigned>(vecSize.getZExtValue() * 8);

  // the vector size needs to be an integral multiple of the type size.
  if (vectorSize % typeSize) {
    S.Diag(Attr.getLoc(), diag::err_attribute_invalid_size)
      << sizeExpr->getSourceRange();
    Attr.setInvalid();
    return;
  }
  if (VectorType::isVectorSizeTooLarge(vectorSize / typeSize)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_size_too_large)
      << sizeExpr->getSourceRange();
    Attr.setInvalid();
    return;
  }
  if (vectorSize == 0) {
    S.Diag(Attr.getLoc(), diag::err_attribute_zero_size)
      << sizeExpr->getSourceRange();
    Attr.setInvalid();
    return;
  }

  // Success! Instantiate the vector type, the number of elements is > 0, and
  // not required to be a power of 2, unlike GCC.
  CurType = S.Context.getVectorType(CurType, vectorSize/typeSize,
                                    VectorType::GenericVector);
}

/// \brief Process the OpenCL-like ext_vector_type attribute when it occurs on
/// a type.
static void HandleExtVectorTypeAttr(QualType &CurType,
                                    const AttributeList &Attr,
                                    Sema &S) {
  // check the attribute arguments.
  if (Attr.getNumArgs() != 1) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_number_arguments)
      << Attr.getName() << 1;
    return;
  }

  Expr *sizeExpr;

  // Special case where the argument is a template id.
  if (Attr.isArgIdent(0)) {
    CXXScopeSpec SS;
    SourceLocation TemplateKWLoc;
    UnqualifiedId id;
    id.setIdentifier(Attr.getArgAsIdent(0)->Ident, Attr.getLoc());

    ExprResult Size = S.ActOnIdExpression(S.getCurScope(), SS, TemplateKWLoc,
                                          id, false, false);
    if (Size.isInvalid())
      return;

    sizeExpr = Size.get();
  } else {
    sizeExpr = Attr.getArgAsExpr(0);
  }

  // Create the vector type.
  QualType T = S.BuildExtVectorType(CurType, sizeExpr, Attr.getLoc());
  if (!T.isNull())
    CurType = T;
}

static bool isPermittedNeonBaseType(QualType &Ty,
                                    VectorType::VectorKind VecKind, Sema &S) {
  const BuiltinType *BTy = Ty->getAs<BuiltinType>();
  if (!BTy)
    return false;

  llvm::Triple Triple = S.Context.getTargetInfo().getTriple();

  // Signed poly is mathematically wrong, but has been baked into some ABIs by
  // now.
  bool IsPolyUnsigned = Triple.getArch() == llvm::Triple::aarch64 ||
                        Triple.getArch() == llvm::Triple::aarch64_be;
  if (VecKind == VectorType::NeonPolyVector) {
    if (IsPolyUnsigned) {
      // AArch64 polynomial vectors are unsigned and support poly64.
      return BTy->getKind() == BuiltinType::UChar ||
             BTy->getKind() == BuiltinType::UShort ||
             BTy->getKind() == BuiltinType::ULong ||
             BTy->getKind() == BuiltinType::ULongLong;
    } else {
      // AArch32 polynomial vector are signed.
      return BTy->getKind() == BuiltinType::SChar ||
             BTy->getKind() == BuiltinType::Short;
    }
  }

  // Non-polynomial vector types: the usual suspects are allowed, as well as
  // float64_t on AArch64.
  bool Is64Bit = Triple.getArch() == llvm::Triple::aarch64 ||
                 Triple.getArch() == llvm::Triple::aarch64_be;

  if (Is64Bit && BTy->getKind() == BuiltinType::Double)
    return true;

  return BTy->getKind() == BuiltinType::SChar ||
         BTy->getKind() == BuiltinType::UChar ||
         BTy->getKind() == BuiltinType::Short ||
         BTy->getKind() == BuiltinType::UShort ||
         BTy->getKind() == BuiltinType::Int ||
         BTy->getKind() == BuiltinType::UInt ||
         BTy->getKind() == BuiltinType::Long ||
         BTy->getKind() == BuiltinType::ULong ||
         BTy->getKind() == BuiltinType::LongLong ||
         BTy->getKind() == BuiltinType::ULongLong ||
         BTy->getKind() == BuiltinType::Float ||
         BTy->getKind() == BuiltinType::Half;
}

/// HandleNeonVectorTypeAttr - The "neon_vector_type" and
/// "neon_polyvector_type" attributes are used to create vector types that
/// are mangled according to ARM's ABI.  Otherwise, these types are identical
/// to those created with the "vector_size" attribute.  Unlike "vector_size"
/// the argument to these Neon attributes is the number of vector elements,
/// not the vector size in bytes.  The vector width and element type must
/// match one of the standard Neon vector types.
static void HandleNeonVectorTypeAttr(QualType& CurType,
                                     const AttributeList &Attr, Sema &S,
                                     VectorType::VectorKind VecKind) {
  // Target must have NEON
  if (!S.Context.getTargetInfo().hasFeature("neon")) {
    S.Diag(Attr.getLoc(), diag::err_attribute_unsupported) << Attr.getName();
    Attr.setInvalid();
    return;
  }
  // Check the attribute arguments.
  if (Attr.getNumArgs() != 1) {
    S.Diag(Attr.getLoc(), diag::err_attribute_wrong_number_arguments)
      << Attr.getName() << 1;
    Attr.setInvalid();
    return;
  }
  // The number of elements must be an ICE.
  Expr *numEltsExpr = static_cast<Expr *>(Attr.getArgAsExpr(0));
  llvm::APSInt numEltsInt(32);
  if (numEltsExpr->isTypeDependent() || numEltsExpr->isValueDependent() ||
      !numEltsExpr->isIntegerConstantExpr(numEltsInt, S.Context)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_argument_type)
      << Attr.getName() << AANT_ArgumentIntegerConstant
      << numEltsExpr->getSourceRange();
    Attr.setInvalid();
    return;
  }
  // Only certain element types are supported for Neon vectors.
  if (!isPermittedNeonBaseType(CurType, VecKind, S)) {
    S.Diag(Attr.getLoc(), diag::err_attribute_invalid_vector_type) << CurType;
    Attr.setInvalid();
    return;
  }

  // The total size of the vector must be 64 or 128 bits.
  unsigned typeSize = static_cast<unsigned>(S.Context.getTypeSize(CurType));
  unsigned numElts = static_cast<unsigned>(numEltsInt.getZExtValue());
  unsigned vecSize = typeSize * numElts;
  if (vecSize != 64 && vecSize != 128) {
    S.Diag(Attr.getLoc(), diag::err_attribute_bad_neon_vector_size) << CurType;
    Attr.setInvalid();
    return;
  }

  CurType = S.Context.getVectorType(CurType, numElts, VecKind);
}

static void processTypeAttrs(TypeProcessingState &state, QualType &type,
                             TypeAttrLocation TAL, AttributeList *attrs) {
  // Scan through and apply attributes to this type where it makes sense.  Some
  // attributes (such as __address_space__, __vector_size__, etc) apply to the
  // type, but others can be present in the type specifiers even though they
  // apply to the decl.  Here we apply type attributes and ignore the rest.

  AttributeList *next;
  do {
    AttributeList &attr = *attrs;
    next = attr.getNext();

    // Skip attributes that were marked to be invalid.
    if (attr.isInvalid())
      continue;

    if (attr.isCXX11Attribute()) {
      // [[gnu::...]] attributes are treated as declaration attributes, so may
      // not appertain to a DeclaratorChunk, even if we handle them as type
      // attributes.
      if (attr.getScopeName() && attr.getScopeName()->isStr("gnu")) {
        if (TAL == TAL_DeclChunk) {
          state.getSema().Diag(attr.getLoc(),
                               diag::warn_cxx11_gnu_attribute_on_type)
              << attr.getName();
          continue;
        }
      // HLSL Change Starts
      } else if (isHLSLTypeAttr(attr.getKind())) {
        if (!handleHLSLTypeAttr(state, attr, type))
          attr.setUsedAsTypeAttr();
        else
          attr.setInvalid(true);
        continue;
      // HLSL Change Ends
      } else if (TAL != TAL_DeclChunk) {
        // Otherwise, only consider type processing for a C++11 attribute if
        // it's actually been applied to a type.
        continue;
      }
    }

    // If this is an attribute we can handle, do so now,
    // otherwise, add it to the FnAttrs list for rechaining.
    switch (attr.getKind()) {
    default:
      // A C++11 attribute on a declarator chunk must appertain to a type.
      if (attr.isCXX11Attribute() && TAL == TAL_DeclChunk) {
        state.getSema().Diag(attr.getLoc(), diag::err_attribute_not_type_attr)
          << attr.getName();
        attr.setUsedAsTypeAttr();
      }
      break;

    case AttributeList::UnknownAttribute:
      if (attr.isCXX11Attribute() && TAL == TAL_DeclChunk)
        state.getSema().Diag(attr.getLoc(),
                             diag::warn_unknown_attribute_ignored)
          << attr.getName();
      break;

    case AttributeList::IgnoredAttribute:
      break;

    case AttributeList::AT_MayAlias:
      // FIXME: This attribute needs to actually be handled, but if we ignore
      // it it breaks large amounts of Linux software.
      attr.setUsedAsTypeAttr();
      break;
    case AttributeList::AT_OpenCLPrivateAddressSpace:
    case AttributeList::AT_OpenCLGlobalAddressSpace:
    case AttributeList::AT_OpenCLLocalAddressSpace:
    case AttributeList::AT_OpenCLConstantAddressSpace:
    case AttributeList::AT_OpenCLGenericAddressSpace:
    case AttributeList::AT_AddressSpace:
      HandleAddressSpaceTypeAttribute(type, attr, state.getSema());
      attr.setUsedAsTypeAttr();
      break;
    OBJC_POINTER_TYPE_ATTRS_CASELIST:
      if (!handleObjCPointerTypeAttr(state, attr, type))
        distributeObjCPointerTypeAttr(state, attr, type);
      attr.setUsedAsTypeAttr();
      break;
    case AttributeList::AT_VectorSize:
      HandleVectorSizeAttr(type, attr, state.getSema());
      attr.setUsedAsTypeAttr();
      break;
    case AttributeList::AT_ExtVectorType:
      HandleExtVectorTypeAttr(type, attr, state.getSema());
      attr.setUsedAsTypeAttr();
      break;
    case AttributeList::AT_NeonVectorType:
      HandleNeonVectorTypeAttr(type, attr, state.getSema(),
                               VectorType::NeonVector);
      attr.setUsedAsTypeAttr();
      break;
    case AttributeList::AT_NeonPolyVectorType:
      HandleNeonVectorTypeAttr(type, attr, state.getSema(),
                               VectorType::NeonPolyVector);
      attr.setUsedAsTypeAttr();
      break;
    case AttributeList::AT_OpenCLImageAccess:
      // FIXME: there should be some type checking happening here, I would
      // imagine, but the original handler's checking was entirely superfluous.
      attr.setUsedAsTypeAttr();
      break;

    MS_TYPE_ATTRS_CASELIST:
      if (!handleMSPointerTypeQualifierAttr(state, attr, type))
        attr.setUsedAsTypeAttr();
      break;


    NULLABILITY_TYPE_ATTRS_CASELIST:
      // Either add nullability here or try to distribute it.  We
      // don't want to distribute the nullability specifier past any
      // dependent type, because that complicates the user model.
      if (type->canHaveNullability() || type->isDependentType() ||
          !distributeNullabilityTypeAttr(state, type, attr)) {
        if (state.getSema().checkNullabilityTypeSpecifier(
              type,
              mapNullabilityAttrKind(attr.getKind()),
              attr.getLoc(),
              attr.isContextSensitiveKeywordAttribute())) {
          attr.setInvalid();
        }

        attr.setUsedAsTypeAttr();
      }
      break;

    case AttributeList::AT_ObjCKindOf:
      // '__kindof' must be part of the decl-specifiers.
      switch (TAL) {
      case TAL_DeclSpec:
        break;

      case TAL_DeclChunk:
      case TAL_DeclName:
        state.getSema().Diag(attr.getLoc(),
                             diag::err_objc_kindof_wrong_position)
          << FixItHint::CreateRemoval(attr.getLoc())
          << FixItHint::CreateInsertion(
               state.getDeclarator().getDeclSpec().getLocStart(), "__kindof ");
        break;
      }

      // Apply it regardless.
      if (state.getSema().checkObjCKindOfType(type, attr.getLoc()))
        attr.setInvalid();
      attr.setUsedAsTypeAttr();
      break;

    case AttributeList::AT_NSReturnsRetained:
      // Begin HLSL Change - avoid dead-code fallthrough
      if (state.getSema().getLangOpts().ObjCAutoRefCount) {
        attr.setUsedAsTypeAttr();

        // Never process function type attributes as part of the
        // declaration-specifiers.
        if (TAL == TAL_DeclSpec)
          distributeFunctionTypeAttrFromDeclSpec(state, attr, type);

        // Otherwise, handle the possible delays.
        else if (!handleFunctionTypeAttr(state, attr, type))
          distributeFunctionTypeAttr(state, attr, type);
      }
      break;
      // End HLSL Change - avoid dead-code fallthrough

    FUNCTION_TYPE_ATTRS_CASELIST:
      attr.setUsedAsTypeAttr();

      // Never process function type attributes as part of the
      // declaration-specifiers.
      if (TAL == TAL_DeclSpec)
        distributeFunctionTypeAttrFromDeclSpec(state, attr, type);

      // Otherwise, handle the possible delays.
      else if (!handleFunctionTypeAttr(state, attr, type))
        distributeFunctionTypeAttr(state, attr, type);
      break;
    }
  } while ((attrs = next));
}

/// \brief Ensure that the type of the given expression is complete.
///
/// This routine checks whether the expression \p E has a complete type. If the
/// expression refers to an instantiable construct, that instantiation is
/// performed as needed to complete its type. Furthermore
/// Sema::RequireCompleteType is called for the expression's type (or in the
/// case of a reference type, the referred-to type).
///
/// \param E The expression whose type is required to be complete.
/// \param Diagnoser The object that will emit a diagnostic if the type is
/// incomplete.
///
/// \returns \c true if the type of \p E is incomplete and diagnosed, \c false
/// otherwise.
bool Sema::RequireCompleteExprType(Expr *E, TypeDiagnoser &Diagnoser){
  QualType T = E->getType();

  // Fast path the case where the type is already complete.
  if (!T->isIncompleteType())
    // FIXME: The definition might not be visible.
    return false;

  // Incomplete array types may be completed by the initializer attached to
  // their definitions. For static data members of class templates and for
  // variable templates, we need to instantiate the definition to get this
  // initializer and complete the type.
  if (T->isIncompleteArrayType()) {
    if (DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(E->IgnoreParens())) {
      if (VarDecl *Var = dyn_cast<VarDecl>(DRE->getDecl())) {
        if (isTemplateInstantiation(Var->getTemplateSpecializationKind())) {
          SourceLocation PointOfInstantiation = E->getExprLoc();

          if (MemberSpecializationInfo *MSInfo =
                  Var->getMemberSpecializationInfo()) {
            // If we don't already have a point of instantiation, this is it.
            if (MSInfo->getPointOfInstantiation().isInvalid()) {
              MSInfo->setPointOfInstantiation(PointOfInstantiation);

              // This is a modification of an existing AST node. Notify
              // listeners.
              if (ASTMutationListener *L = getASTMutationListener())
                L->StaticDataMemberInstantiated(Var);
            }
          } else {
            VarTemplateSpecializationDecl *VarSpec =
                cast<VarTemplateSpecializationDecl>(Var);
            if (VarSpec->getPointOfInstantiation().isInvalid())
              VarSpec->setPointOfInstantiation(PointOfInstantiation);
          }

          InstantiateVariableDefinition(PointOfInstantiation, Var);

          // Update the type to the newly instantiated definition's type both
          // here and within the expression.
          if (VarDecl *Def = Var->getDefinition()) {
            DRE->setDecl(Def);
            T = Def->getType();
            DRE->setType(T);
            E->setType(T);
          }

          // We still go on to try to complete the type independently, as it
          // may also require instantiations or diagnostics if it remains
          // incomplete.
        }
      }
    }
  }

  // FIXME: Are there other cases which require instantiating something other
  // than the type to complete the type of an expression?

  // Look through reference types and complete the referred type.
  if (const ReferenceType *Ref = T->getAs<ReferenceType>())
    T = Ref->getPointeeType();

  return RequireCompleteType(E->getExprLoc(), T, Diagnoser);
}

namespace {
  struct TypeDiagnoserDiag : Sema::TypeDiagnoser {
    unsigned DiagID;

    TypeDiagnoserDiag(unsigned DiagID)
      : Sema::TypeDiagnoser(DiagID == 0), DiagID(DiagID) {}

    void diagnose(Sema &S, SourceLocation Loc, QualType T) override {
      if (Suppressed) return;
      S.Diag(Loc, DiagID) << T;
    }
  };
}

bool Sema::RequireCompleteExprType(Expr *E, unsigned DiagID) {
  TypeDiagnoserDiag Diagnoser(DiagID);
  return RequireCompleteExprType(E, Diagnoser);
}

/// @brief Ensure that the type T is a complete type.
///
/// This routine checks whether the type @p T is complete in any
/// context where a complete type is required. If @p T is a complete
/// type, returns false. If @p T is a class template specialization,
/// this routine then attempts to perform class template
/// instantiation. If instantiation fails, or if @p T is incomplete
/// and cannot be completed, issues the diagnostic @p diag (giving it
/// the type @p T) and returns true.
///
/// @param Loc  The location in the source that the incomplete type
/// diagnostic should refer to.
///
/// @param T  The type that this routine is examining for completeness.
///
/// @returns @c true if @p T is incomplete and a diagnostic was emitted,
/// @c false otherwise.
bool Sema::RequireCompleteType(SourceLocation Loc, QualType T,
                               TypeDiagnoser &Diagnoser) {
  if (RequireCompleteTypeImpl(Loc, T, Diagnoser))
    return true;
  if (const TagType *Tag = T->getAs<TagType>()) {
    if (!Tag->getDecl()->isCompleteDefinitionRequired()) {
      Tag->getDecl()->setCompleteDefinitionRequired();
      Consumer.HandleTagDeclRequiredDefinition(Tag->getDecl());
    }
  }
  return false;
}

/// \brief Determine whether there is any declaration of \p D that was ever a
///        definition (perhaps before module merging) and is currently visible.
/// \param D The definition of the entity.
/// \param Suggested Filled in with the declaration that should be made visible
///        in order to provide a definition of this entity.
/// \param OnlyNeedComplete If \c true, we only need the type to be complete,
///        not defined. This only matters for enums with a fixed underlying
///        type, since in all other cases, a type is complete if and only if it
///        is defined.
bool Sema::hasVisibleDefinition(NamedDecl *D, NamedDecl **Suggested,
                                bool OnlyNeedComplete) {
  // Easy case: if we don't have modules, all declarations are visible.
  if (!getLangOpts().Modules && !getLangOpts().ModulesLocalVisibility)
    return true;

  // If this definition was instantiated from a template, map back to the
  // pattern from which it was instantiated.
  if (isa<TagDecl>(D) && cast<TagDecl>(D)->isBeingDefined()) {
    // We're in the middle of defining it; this definition should be treated
    // as visible.
    return true;
  } else if (auto *RD = dyn_cast<CXXRecordDecl>(D)) {
    if (auto *Pattern = RD->getTemplateInstantiationPattern())
      RD = Pattern;
    D = RD->getDefinition();
  } else if (auto *ED = dyn_cast<EnumDecl>(D)) {
    while (auto *NewED = ED->getInstantiatedFromMemberEnum())
      ED = NewED;
    if (OnlyNeedComplete && ED->isFixed()) {
      // If the enum has a fixed underlying type, and we're only looking for a
      // complete type (not a definition), any visible declaration of it will
      // do.
      *Suggested = nullptr;
      for (auto *Redecl : ED->redecls()) {
        if (isVisible(Redecl))
          return true;
        if (Redecl->isThisDeclarationADefinition() ||
            (Redecl->isCanonicalDecl() && !*Suggested))
          *Suggested = Redecl;
      }
      return false;
    }
    D = ED->getDefinition();
  }
  assert(D && "missing definition for pattern of instantiated definition");

  *Suggested = D;
  if (isVisible(D))
    return true;

  // The external source may have additional definitions of this type that are
  // visible, so complete the redeclaration chain now and ask again.
  if (auto *Source = Context.getExternalSource()) {
    Source->CompleteRedeclChain(D);
    return isVisible(D);
  }

  return false;
}

/// Locks in the inheritance model for the given class and all of its bases.
static void assignInheritanceModel(Sema &S, CXXRecordDecl *RD) {
  RD = RD->getMostRecentDecl();
  if (!RD->hasAttr<MSInheritanceAttr>()) {
    MSInheritanceAttr::Spelling IM;

    switch (S.MSPointerToMemberRepresentationMethod) {
    case LangOptions::PPTMK_BestCase:
      IM = RD->calculateInheritanceModel();
      break;
    case LangOptions::PPTMK_FullGeneralitySingleInheritance:
      IM = MSInheritanceAttr::Keyword_single_inheritance;
      break;
    case LangOptions::PPTMK_FullGeneralityMultipleInheritance:
      IM = MSInheritanceAttr::Keyword_multiple_inheritance;
      break;
    case LangOptions::PPTMK_FullGeneralityVirtualInheritance:
      IM = MSInheritanceAttr::Keyword_unspecified_inheritance;
      break;
    }

    RD->addAttr(MSInheritanceAttr::CreateImplicit(
        S.getASTContext(), IM,
        /*BestCase=*/S.MSPointerToMemberRepresentationMethod ==
            LangOptions::PPTMK_BestCase,
        S.ImplicitMSInheritanceAttrLoc.isValid()
            ? S.ImplicitMSInheritanceAttrLoc
            : RD->getSourceRange()));
  }
}

/// \brief The implementation of RequireCompleteType
bool Sema::RequireCompleteTypeImpl(SourceLocation Loc, QualType T,
                                   TypeDiagnoser &Diagnoser) {
  // FIXME: Add this assertion to make sure we always get instantiation points.
  //  assert(!Loc.isInvalid() && "Invalid location in RequireCompleteType");
  // FIXME: Add this assertion to help us flush out problems with
  // checking for dependent types and type-dependent expressions.
  //
  //  assert(!T->isDependentType() &&
  //         "Can't ask whether a dependent type is complete");

  // If we have a complete type, we're done.
  NamedDecl *Def = nullptr;
  if (!T->isIncompleteType(&Def)) {
    // If we know about the definition but it is not visible, complain.
    NamedDecl *SuggestedDef = nullptr;
    if (!Diagnoser.Suppressed && Def &&
        !hasVisibleDefinition(Def, &SuggestedDef, /*OnlyNeedComplete*/true))
      diagnoseMissingImport(Loc, SuggestedDef, /*NeedDefinition*/true);

    // We lock in the inheritance model once somebody has asked us to ensure
    // that a pointer-to-member type is complete.
    if (Context.getTargetInfo().getCXXABI().isMicrosoft()) {
      if (const MemberPointerType *MPTy = T->getAs<MemberPointerType>()) {
        if (!MPTy->getClass()->isDependentType()) {
          RequireCompleteType(Loc, QualType(MPTy->getClass(), 0), 0);
          assignInheritanceModel(*this, MPTy->getMostRecentCXXRecordDecl());
        }
      }
    }

    return false;
  }

  const TagType *Tag = T->getAs<TagType>();
  const ObjCInterfaceType *IFace = T->getAs<ObjCInterfaceType>();

  // If there's an unimported definition of this type in a module (for
  // instance, because we forward declared it, then imported the definition),
  // import that definition now.
  //
  // FIXME: What about other cases where an import extends a redeclaration
  // chain for a declaration that can be accessed through a mechanism other
  // than name lookup (eg, referenced in a template, or a variable whose type
  // could be completed by the module)?
  if (Tag || IFace) {
    NamedDecl *D =
        Tag ? static_cast<NamedDecl *>(Tag->getDecl()) : IFace->getDecl();

    // Avoid diagnosing invalid decls as incomplete.
    if (D->isInvalidDecl())
      return true;

    // Give the external AST source a chance to complete the type.
    if (auto *Source = Context.getExternalSource()) {
      if (Tag)
        Source->CompleteType(Tag->getDecl());
      else
        Source->CompleteType(IFace->getDecl());

      // If the external source completed the type, go through the motions
      // again to ensure we're allowed to use the completed type.
      if (!T->isIncompleteType())
        return RequireCompleteTypeImpl(Loc, T, Diagnoser);
    }
  }

  // If we have a class template specialization or a class member of a
  // class template specialization, or an array with known size of such,
  // try to instantiate it.
  QualType MaybeTemplate = T;
  while (const ConstantArrayType *Array
           = Context.getAsConstantArrayType(MaybeTemplate))
    MaybeTemplate = Array->getElementType();

  // HLSL Change - Begin
  // In HLSL we don't decay arrays to pointers, so we need to require complete
  // types for array elements.
  if (LangOpts.HLSL) {
    if (const TagType *Tag = MaybeTemplate->getAs<TagType>()) {
      if (auto *Source = Context.getExternalSource())
        Source->CompleteType(Tag->getDecl());
    }
  }
  // HLSL Change - End
  if (const RecordType *Record = MaybeTemplate->getAs<RecordType>()) {
    if (ClassTemplateSpecializationDecl *ClassTemplateSpec
          = dyn_cast<ClassTemplateSpecializationDecl>(Record->getDecl())) {
      if (ClassTemplateSpec->getSpecializationKind() == TSK_Undeclared)
        return InstantiateClassTemplateSpecialization(Loc, ClassTemplateSpec,
                                                      TSK_ImplicitInstantiation,
                                            /*Complain=*/!Diagnoser.Suppressed);
    } else if (CXXRecordDecl *Rec
                 = dyn_cast<CXXRecordDecl>(Record->getDecl())) {
      CXXRecordDecl *Pattern = Rec->getInstantiatedFromMemberClass();
      if (!Rec->isBeingDefined() && Pattern) {
        MemberSpecializationInfo *MSI = Rec->getMemberSpecializationInfo();
        assert(MSI && "Missing member specialization information?");
        // This record was instantiated from a class within a template.
        if (MSI->getTemplateSpecializationKind() != TSK_ExplicitSpecialization)
          return InstantiateClass(Loc, Rec, Pattern,
                                  getTemplateInstantiationArgs(Rec),
                                  TSK_ImplicitInstantiation,
                                  /*Complain=*/!Diagnoser.Suppressed);
      }
    }
  }

  if (Diagnoser.Suppressed)
    return true;

  // We have an incomplete type. Produce a diagnostic.
  if (Ident___float128 &&
      T == Context.getTypeDeclType(Context.getFloat128StubType())) {
    Diag(Loc, diag::err_typecheck_decl_incomplete_type___float128);
    return true;
  }

  Diagnoser.diagnose(*this, Loc, T);

  // If the type was a forward declaration of a class/struct/union
  // type, produce a note.
  if (Tag && !Tag->getDecl()->isInvalidDecl())
    Diag(Tag->getDecl()->getLocation(),
         Tag->isBeingDefined() ? diag::note_type_being_defined
                               : diag::note_forward_declaration)
      << QualType(Tag, 0);

  // If the Objective-C class was a forward declaration, produce a note.
  if (IFace && !IFace->getDecl()->isInvalidDecl())
    Diag(IFace->getDecl()->getLocation(), diag::note_forward_class);

  // If we have external information that we can use to suggest a fix,
  // produce a note.
  if (ExternalSource)
    ExternalSource->MaybeDiagnoseMissingCompleteType(Loc, T);

  return true;
}

bool Sema::RequireCompleteType(SourceLocation Loc, QualType T,
                               unsigned DiagID) {
  TypeDiagnoserDiag Diagnoser(DiagID);
  return RequireCompleteType(Loc, T, Diagnoser);
}

/// \brief Get diagnostic %select index for tag kind for
/// literal type diagnostic message.
/// WARNING: Indexes apply to particular diagnostics only!
///
/// \returns diagnostic %select index.
static unsigned getLiteralDiagFromTagKind(TagTypeKind Tag) {
  switch (Tag) {
  case TTK_Struct: return 0;
  case TTK_Interface: return 1;
  case TTK_Class:  return 2;
  default: llvm_unreachable("Invalid tag kind for literal type diagnostic!");
  }
}

/// @brief Ensure that the type T is a literal type.
///
/// This routine checks whether the type @p T is a literal type. If @p T is an
/// incomplete type, an attempt is made to complete it. If @p T is a literal
/// type, or @p AllowIncompleteType is true and @p T is an incomplete type,
/// returns false. Otherwise, this routine issues the diagnostic @p PD (giving
/// it the type @p T), along with notes explaining why the type is not a
/// literal type, and returns true.
///
/// @param Loc  The location in the source that the non-literal type
/// diagnostic should refer to.
///
/// @param T  The type that this routine is examining for literalness.
///
/// @param Diagnoser Emits a diagnostic if T is not a literal type.
///
/// @returns @c true if @p T is not a literal type and a diagnostic was emitted,
/// @c false otherwise.
bool Sema::RequireLiteralType(SourceLocation Loc, QualType T,
                              TypeDiagnoser &Diagnoser) {
  assert(!T->isDependentType() && "type should not be dependent");

  QualType ElemType = Context.getBaseElementType(T);
  RequireCompleteType(Loc, ElemType, 0);

  if (T->isLiteralType(Context))
    return false;

  if (Diagnoser.Suppressed)
    return true;

  Diagnoser.diagnose(*this, Loc, T);

  if (T->isVariableArrayType())
    return true;

  const RecordType *RT = ElemType->getAs<RecordType>();
  if (!RT)
    return true;

  const CXXRecordDecl *RD = cast<CXXRecordDecl>(RT->getDecl());

  // A partially-defined class type can't be a literal type, because a literal
  // class type must have a trivial destructor (which can't be checked until
  // the class definition is complete).
  if (!RD->isCompleteDefinition()) {
    RequireCompleteType(Loc, ElemType, diag::note_non_literal_incomplete, T);
    return true;
  }

  // If the class has virtual base classes, then it's not an aggregate, and
  // cannot have any constexpr constructors or a trivial default constructor,
  // so is non-literal. This is better to diagnose than the resulting absence
  // of constexpr constructors.
  if (RD->getNumVBases()) {
    Diag(RD->getLocation(), diag::note_non_literal_virtual_base)
      << getLiteralDiagFromTagKind(RD->getTagKind()) << RD->getNumVBases();
    for (const auto &I : RD->vbases())
      Diag(I.getLocStart(), diag::note_constexpr_virtual_base_here)
          << I.getSourceRange();
  } else if (!RD->isAggregate() && !RD->hasConstexprNonCopyMoveConstructor() &&
             !RD->hasTrivialDefaultConstructor()) {
    Diag(RD->getLocation(), diag::note_non_literal_no_constexpr_ctors) << RD;
  } else if (RD->hasNonLiteralTypeFieldsOrBases()) {
    for (const auto &I : RD->bases()) {
      if (!I.getType()->isLiteralType(Context)) {
        Diag(I.getLocStart(),
             diag::note_non_literal_base_class)
          << RD << I.getType() << I.getSourceRange();
        return true;
      }
    }
    for (const auto *I : RD->fields()) {
      if (!I->getType()->isLiteralType(Context) ||
          I->getType().isVolatileQualified()) {
        Diag(I->getLocation(), diag::note_non_literal_field)
          << RD << I << I->getType()
          << I->getType().isVolatileQualified();
        return true;
      }
    }
  } else if (!RD->hasTrivialDestructor()) {
    // All fields and bases are of literal types, so have trivial destructors.
    // If this class's destructor is non-trivial it must be user-declared.
    CXXDestructorDecl *Dtor = RD->getDestructor();
    assert(Dtor && "class has literal fields and bases but no dtor?");
    if (!Dtor)
      return true;

    Diag(Dtor->getLocation(), Dtor->isUserProvided() ?
         diag::note_non_literal_user_provided_dtor :
         diag::note_non_literal_nontrivial_dtor) << RD;
    if (!Dtor->isUserProvided())
      SpecialMemberIsTrivial(Dtor, CXXDestructor, /*Diagnose*/true);
  }

  return true;
}

bool Sema::RequireLiteralType(SourceLocation Loc, QualType T, unsigned DiagID) {
  TypeDiagnoserDiag Diagnoser(DiagID);
  return RequireLiteralType(Loc, T, Diagnoser);
}

/// \brief Retrieve a version of the type 'T' that is elaborated by Keyword
/// and qualified by the nested-name-specifier contained in SS.
QualType Sema::getElaboratedType(ElaboratedTypeKeyword Keyword,
                                 const CXXScopeSpec &SS, QualType T) {
  if (T.isNull())
    return T;
  NestedNameSpecifier *NNS;
  if (SS.isValid())
    NNS = SS.getScopeRep();
  else {
    if (Keyword == ETK_None)
      return T;
    NNS = nullptr;
  }
  return Context.getElaboratedType(Keyword, NNS, T);
}

QualType Sema::BuildTypeofExprType(Expr *E, SourceLocation Loc) {
  ExprResult ER = CheckPlaceholderExpr(E);
  if (ER.isInvalid()) return QualType();
  E = ER.get();

  if (!E->isTypeDependent()) {
    QualType T = E->getType();
    if (const TagType *TT = T->getAs<TagType>())
      DiagnoseUseOfDecl(TT->getDecl(), E->getExprLoc());
  }
  return Context.getTypeOfExprType(E);
}

/// getDecltypeForExpr - Given an expr, will return the decltype for
/// that expression, according to the rules in C++11
/// [dcl.type.simple]p4 and C++11 [expr.lambda.prim]p18.
static QualType getDecltypeForExpr(Sema &S, Expr *E) {
  if (E->isTypeDependent())
    return S.Context.DependentTy;

  // C++11 [dcl.type.simple]p4:
  //   The type denoted by decltype(e) is defined as follows:
  //
  //     - if e is an unparenthesized id-expression or an unparenthesized class
  //       member access (5.2.5), decltype(e) is the type of the entity named
  //       by e. If there is no such entity, or if e names a set of overloaded
  //       functions, the program is ill-formed;
  //
  // We apply the same rules for Objective-C ivar and property references.
  if (const DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(E)) {
    if (const ValueDecl *VD = dyn_cast<ValueDecl>(DRE->getDecl()))
      return VD->getType();
  } else if (const MemberExpr *ME = dyn_cast<MemberExpr>(E)) {
    if (const FieldDecl *FD = dyn_cast<FieldDecl>(ME->getMemberDecl()))
      return FD->getType();
  } else if (const ObjCIvarRefExpr *IR = dyn_cast<ObjCIvarRefExpr>(E)) {
    return IR->getDecl()->getType();
  } else if (const ObjCPropertyRefExpr *PR = dyn_cast<ObjCPropertyRefExpr>(E)) {
    if (PR->isExplicitProperty())
      return PR->getExplicitProperty()->getType();
  } else if (auto *PE = dyn_cast<PredefinedExpr>(E)) {
    return PE->getType();
  }
  
  // C++11 [expr.lambda.prim]p18:
  //   Every occurrence of decltype((x)) where x is a possibly
  //   parenthesized id-expression that names an entity of automatic
  //   storage duration is treated as if x were transformed into an
  //   access to a corresponding data member of the closure type that
  //   would have been declared if x were an odr-use of the denoted
  //   entity.
  using namespace sema;
  if (S.getCurLambda()) {
    if (isa<ParenExpr>(E)) {
      if (DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(E->IgnoreParens())) {
        if (VarDecl *Var = dyn_cast<VarDecl>(DRE->getDecl())) {
          QualType T = S.getCapturedDeclRefType(Var, DRE->getLocation());
          if (!T.isNull())
            return S.Context.getLValueReferenceType(T);
        }
      }
    }
  }


  // C++11 [dcl.type.simple]p4:
  //   [...]
  QualType T = E->getType();
  switch (E->getValueKind()) {
  //     - otherwise, if e is an xvalue, decltype(e) is T&&, where T is the
  //       type of e;
  case VK_XValue: T = S.Context.getRValueReferenceType(T); break;
  //     - otherwise, if e is an lvalue, decltype(e) is T&, where T is the
  //       type of e;
  case VK_LValue: T = S.Context.getLValueReferenceType(T); break;
  //  - otherwise, decltype(e) is the type of e.
  case VK_RValue: break;
  }

  return T;
}

QualType Sema::BuildDecltypeType(Expr *E, SourceLocation Loc,
                                 bool AsUnevaluated) {
  ExprResult ER = CheckPlaceholderExpr(E);
  if (ER.isInvalid()) return QualType();
  E = ER.get();

  if (AsUnevaluated && ActiveTemplateInstantiations.empty() &&
      E->HasSideEffects(Context, false)) {
    // The expression operand for decltype is in an unevaluated expression
    // context, so side effects could result in unintended consequences.
    Diag(E->getExprLoc(), diag::warn_side_effects_unevaluated_context);
  }

  return Context.getDecltypeType(E, getDecltypeForExpr(*this, E));
}

QualType Sema::BuildUnaryTransformType(QualType BaseType,
                                       UnaryTransformType::UTTKind UKind,
                                       SourceLocation Loc) {
  switch (UKind) {
  case UnaryTransformType::EnumUnderlyingType:
    if (!BaseType->isDependentType() && !BaseType->isEnumeralType()) {
      Diag(Loc, diag::err_only_enums_have_underlying_types);
      return QualType();
    } else {
      QualType Underlying = BaseType;
      if (!BaseType->isDependentType()) {
        // The enum could be incomplete if we're parsing its definition or
        // recovering from an error.
        NamedDecl *FwdDecl = nullptr;
        if (BaseType->isIncompleteType(&FwdDecl)) {
          Diag(Loc, diag::err_underlying_type_of_incomplete_enum) << BaseType;
          Diag(FwdDecl->getLocation(), diag::note_forward_declaration) << FwdDecl;
          return QualType();
        }

        EnumDecl *ED = BaseType->getAs<EnumType>()->getDecl();
        assert(ED && "EnumType has no EnumDecl");

        DiagnoseUseOfDecl(ED, Loc);

        Underlying = ED->getIntegerType();
        assert(!Underlying.isNull());
      }
      return Context.getUnaryTransformType(BaseType, Underlying,
                                        UnaryTransformType::EnumUnderlyingType);
    }
  }
  llvm_unreachable("unknown unary transform type");
}

QualType Sema::BuildAtomicType(QualType T, SourceLocation Loc) {
  if (!T->isDependentType()) {
    // FIXME: It isn't entirely clear whether incomplete atomic types
    // are allowed or not; for simplicity, ban them for the moment.
    if (RequireCompleteType(Loc, T, diag::err_atomic_specifier_bad_type, 0))
      return QualType();

    int DisallowedKind = -1;
    if (T->isArrayType())
      DisallowedKind = 1;
    else if (T->isFunctionType())
      DisallowedKind = 2;
    else if (T->isReferenceType())
      DisallowedKind = 3;
    else if (T->isAtomicType())
      DisallowedKind = 4;
    else if (T.hasQualifiers())
      DisallowedKind = 5;
    else if (!T.isTriviallyCopyableType(Context))
      // Some other non-trivially-copyable type (probably a C++ class)
      DisallowedKind = 6;

    if (DisallowedKind != -1) {
      Diag(Loc, diag::err_atomic_specifier_bad_type) << DisallowedKind << T;
      return QualType();
    }

    // FIXME: Do we need any handling for ARC here?
  }

  // Build the pointer type.
  return Context.getAtomicType(T);
}
