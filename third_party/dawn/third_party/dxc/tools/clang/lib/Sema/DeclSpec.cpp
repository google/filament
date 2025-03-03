//===--- SemaDeclSpec.cpp - Declaration Specifier Semantic Analysis -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements semantic analysis for declaration specifiers.
//
//===----------------------------------------------------------------------===//

#include "clang/Sema/DeclSpec.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/Expr.h"
#include "clang/AST/NestedNameSpecifier.h"
#include "clang/AST/TypeLoc.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseDiagnostic.h" // FIXME: remove this back-dependency!
#include "clang/Sema/LocInfoType.h"
#include "clang/Sema/ParsedTemplate.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/SemaDiagnostic.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/ErrorHandling.h"
#include <cstring>
using namespace clang;


static DiagnosticBuilder Diag(DiagnosticsEngine &D, SourceLocation Loc,
                              unsigned DiagID) {
  return D.Report(Loc, DiagID);
}


void UnqualifiedId::setTemplateId(TemplateIdAnnotation *TemplateId) {
  assert(TemplateId && "NULL template-id annotation?");
  Kind = IK_TemplateId;
  this->TemplateId = TemplateId;
  StartLocation = TemplateId->TemplateNameLoc;
  EndLocation = TemplateId->RAngleLoc;
}

void UnqualifiedId::setConstructorTemplateId(TemplateIdAnnotation *TemplateId) {
  assert(TemplateId && "NULL template-id annotation?");
  Kind = IK_ConstructorTemplateId;
  this->TemplateId = TemplateId;
  StartLocation = TemplateId->TemplateNameLoc;
  EndLocation = TemplateId->RAngleLoc;
}

void CXXScopeSpec::Extend(ASTContext &Context, SourceLocation TemplateKWLoc, 
                          TypeLoc TL, SourceLocation ColonColonLoc) {
  Builder.Extend(Context, TemplateKWLoc, TL, ColonColonLoc);
  if (Range.getBegin().isInvalid())
    Range.setBegin(TL.getBeginLoc());
  Range.setEnd(ColonColonLoc);

  assert(Range == Builder.getSourceRange() &&
         "NestedNameSpecifierLoc range computation incorrect");
}

void CXXScopeSpec::Extend(ASTContext &Context, IdentifierInfo *Identifier,
                          SourceLocation IdentifierLoc, 
                          SourceLocation ColonColonLoc) {
  Builder.Extend(Context, Identifier, IdentifierLoc, ColonColonLoc);
  
  if (Range.getBegin().isInvalid())
    Range.setBegin(IdentifierLoc);
  Range.setEnd(ColonColonLoc);
  
  assert(Range == Builder.getSourceRange() &&
         "NestedNameSpecifierLoc range computation incorrect");
}

void CXXScopeSpec::Extend(ASTContext &Context, NamespaceDecl *Namespace,
                          SourceLocation NamespaceLoc, 
                          SourceLocation ColonColonLoc) {
  Builder.Extend(Context, Namespace, NamespaceLoc, ColonColonLoc);
  
  if (Range.getBegin().isInvalid())
    Range.setBegin(NamespaceLoc);
  Range.setEnd(ColonColonLoc);

  assert(Range == Builder.getSourceRange() &&
         "NestedNameSpecifierLoc range computation incorrect");
}

void CXXScopeSpec::Extend(ASTContext &Context, NamespaceAliasDecl *Alias,
                          SourceLocation AliasLoc, 
                          SourceLocation ColonColonLoc) {
  Builder.Extend(Context, Alias, AliasLoc, ColonColonLoc);
  
  if (Range.getBegin().isInvalid())
    Range.setBegin(AliasLoc);
  Range.setEnd(ColonColonLoc);

  assert(Range == Builder.getSourceRange() &&
         "NestedNameSpecifierLoc range computation incorrect");
}

void CXXScopeSpec::MakeGlobal(ASTContext &Context, 
                              SourceLocation ColonColonLoc) {
  Builder.MakeGlobal(Context, ColonColonLoc);
  
  Range = SourceRange(ColonColonLoc);
  
  assert(Range == Builder.getSourceRange() &&
         "NestedNameSpecifierLoc range computation incorrect");
}

void CXXScopeSpec::MakeSuper(ASTContext &Context, CXXRecordDecl *RD,
                             SourceLocation SuperLoc,
                             SourceLocation ColonColonLoc) {
  Builder.MakeSuper(Context, RD, SuperLoc, ColonColonLoc);

  Range.setBegin(SuperLoc);
  Range.setEnd(ColonColonLoc);

  assert(Range == Builder.getSourceRange() &&
  "NestedNameSpecifierLoc range computation incorrect");
}

void CXXScopeSpec::MakeTrivial(ASTContext &Context, 
                               NestedNameSpecifier *Qualifier, SourceRange R) {
  Builder.MakeTrivial(Context, Qualifier, R);
  Range = R;
}

void CXXScopeSpec::Adopt(NestedNameSpecifierLoc Other) {
  if (!Other) {
    Range = SourceRange();
    Builder.Clear();
    return;
  }

  Range = Other.getSourceRange();
  Builder.Adopt(Other);
}

SourceLocation CXXScopeSpec::getLastQualifierNameLoc() const {
  if (!Builder.getRepresentation())
    return SourceLocation();
  return Builder.getTemporary().getLocalBeginLoc();
}

NestedNameSpecifierLoc 
CXXScopeSpec::getWithLocInContext(ASTContext &Context) const {
  if (!Builder.getRepresentation())
    return NestedNameSpecifierLoc();
  
  return Builder.getWithLocInContext(Context);
}

/// DeclaratorChunk::getFunction - Return a DeclaratorChunk for a function.
/// "TheDeclarator" is the declarator that this will be added to.
DeclaratorChunk DeclaratorChunk::getFunction(bool hasProto,
                                             bool isAmbiguous,
                                             SourceLocation LParenLoc,
                                             ParamInfo *Params,
                                             unsigned NumParams,
                                             SourceLocation EllipsisLoc,
                                             SourceLocation RParenLoc,
                                             unsigned TypeQuals,
                                             bool RefQualifierIsLvalueRef,
                                             SourceLocation RefQualifierLoc,
                                             SourceLocation ConstQualifierLoc,
                                             SourceLocation
                                                 VolatileQualifierLoc,
                                             SourceLocation
                                                 RestrictQualifierLoc,
                                             SourceLocation MutableLoc,
                                             ExceptionSpecificationType
                                                 ESpecType,
                                             SourceLocation ESpecLoc,
                                             ParsedType *Exceptions,
                                             SourceRange *ExceptionRanges,
                                             unsigned NumExceptions,
                                             Expr *NoexceptExpr,
                                             CachedTokens *ExceptionSpecTokens,
                                             SourceLocation LocalRangeBegin,
                                             SourceLocation LocalRangeEnd,
                                             Declarator &TheDeclarator,
                                             TypeResult TrailingReturnType) {
  assert(!(TypeQuals & DeclSpec::TQ_atomic) &&
         "function cannot have _Atomic qualifier");

  DeclaratorChunk I;
  I.Kind                        = Function;
  I.Loc                         = LocalRangeBegin;
  I.EndLoc                      = LocalRangeEnd;
  I.Fun.AttrList                = nullptr;
  I.Fun.hasPrototype            = hasProto;
  I.Fun.isVariadic              = EllipsisLoc.isValid();
  I.Fun.isAmbiguous             = isAmbiguous;
  I.Fun.LParenLoc               = LParenLoc.getRawEncoding();
  I.Fun.EllipsisLoc             = EllipsisLoc.getRawEncoding();
  I.Fun.RParenLoc               = RParenLoc.getRawEncoding();
  I.Fun.DeleteParams            = false;
  I.Fun.TypeQuals               = TypeQuals;
  I.Fun.NumParams               = NumParams;
  I.Fun.Params                  = nullptr;
  I.Fun.RefQualifierIsLValueRef = RefQualifierIsLvalueRef;
  I.Fun.RefQualifierLoc         = RefQualifierLoc.getRawEncoding();
  I.Fun.ConstQualifierLoc       = ConstQualifierLoc.getRawEncoding();
  I.Fun.VolatileQualifierLoc    = VolatileQualifierLoc.getRawEncoding();
  I.Fun.RestrictQualifierLoc    = RestrictQualifierLoc.getRawEncoding();
  I.Fun.MutableLoc              = MutableLoc.getRawEncoding();
  I.Fun.ExceptionSpecType       = ESpecType;
  I.Fun.ExceptionSpecLoc        = ESpecLoc.getRawEncoding();
  I.Fun.NumExceptions           = 0;
  I.Fun.Exceptions              = nullptr;
  I.Fun.NoexceptExpr            = nullptr;
  I.Fun.HasTrailingReturnType   = TrailingReturnType.isUsable() ||
                                  TrailingReturnType.isInvalid();
  I.Fun.TrailingReturnType      = TrailingReturnType.get();

  assert(I.Fun.TypeQuals == TypeQuals && "bitfield overflow");
  assert(I.Fun.ExceptionSpecType == ESpecType && "bitfield overflow");

  // new[] a parameter array if needed.
  if (NumParams) {
    // If the 'InlineParams' in Declarator is unused and big enough, put our
    // parameter list there (in an effort to avoid new/delete traffic).  If it
    // is already used (consider a function returning a function pointer) or too
    // small (function with too many parameters), go to the heap.
    if (!TheDeclarator.InlineParamsUsed &&
        NumParams <= llvm::array_lengthof(TheDeclarator.InlineParams)) {
      I.Fun.Params = TheDeclarator.InlineParams;
      I.Fun.DeleteParams = false;
      TheDeclarator.InlineParamsUsed = true;
    } else {
      I.Fun.Params = new DeclaratorChunk::ParamInfo[NumParams];
      I.Fun.DeleteParams = true;
    }
    memcpy(I.Fun.Params, Params, sizeof(Params[0]) * NumParams);
  }

  // Check what exception specification information we should actually store.
  switch (ESpecType) {
  default: break; // By default, save nothing.
  case EST_Dynamic:
    // new[] an exception array if needed
    if (NumExceptions) {
      I.Fun.NumExceptions = NumExceptions;
      I.Fun.Exceptions = new DeclaratorChunk::TypeAndRange[NumExceptions];
      for (unsigned i = 0; i != NumExceptions; ++i) {
        I.Fun.Exceptions[i].Ty = Exceptions[i];
        I.Fun.Exceptions[i].Range = ExceptionRanges[i];
      }
    }
    break;

  case EST_ComputedNoexcept:
    I.Fun.NoexceptExpr = NoexceptExpr;
    break;

  case EST_Unparsed:
    I.Fun.ExceptionSpecTokens = ExceptionSpecTokens;
    break;
  }
  return I;
}

bool Declarator::isDeclarationOfFunction() const {
  for (unsigned i = 0, i_end = DeclTypeInfo.size(); i < i_end; ++i) {
    switch (DeclTypeInfo[i].Kind) {
    case DeclaratorChunk::Function:
      return true;
    case DeclaratorChunk::Paren:
      continue;
    case DeclaratorChunk::Pointer:
    case DeclaratorChunk::Reference:
    case DeclaratorChunk::Array:
    case DeclaratorChunk::BlockPointer:
    case DeclaratorChunk::MemberPointer:
      return false;
    }
    llvm_unreachable("Invalid type chunk");
  }
  
  switch (DS.getTypeSpecType()) {
    case TST_atomic:
    case TST_auto:
    case TST_bool:
    case TST_char:
    case TST_char16:
    case TST_char32:
    case TST_class:
    case TST_decimal128:
    case TST_decimal32:
    case TST_decimal64:
    case TST_double:
    case TST_enum:
    case TST_error:
    case TST_float:
    case TST_half:
    case TST_int:
    case TST_int128:
    case TST_struct:
    case TST_interface:
    case TST_union:
    case TST_unknown_anytype:
    case TST_unspecified:
    case TST_void:
    case TST_wchar:
    // HLSL Change Starts
    case TST_halffloat:
    case TST_min16float:
    case TST_min16int:
    case TST_min16uint:
    case TST_min10float:
    case TST_min12int:
    case TST_int8_4packed:
    case TST_uint8_4packed:
    // HLSL Change Ends
      return false;

    case TST_decltype_auto:
      // This must have an initializer, so can't be a function declaration,
      // even if the initializer has function type.
      return false;

    case TST_decltype:
    case TST_typeofExpr:
      if (Expr *E = DS.getRepAsExpr())
        return E->getType()->isFunctionType();
      return false;
     
    case TST_underlyingType:
    case TST_typename:
    case TST_typeofType: {
      QualType QT = DS.getRepAsType().get();
      if (QT.isNull())
        return false;
      
      if (const LocInfoType *LIT = dyn_cast<LocInfoType>(QT))
        QT = LIT->getType();

      if (QT.isNull())
        return false;
        
      return QT->isFunctionType();
    }
  }

  llvm_unreachable("Invalid TypeSpecType!");
}

bool Declarator::isStaticMember() {
  assert(getContext() == MemberContext);
  return getDeclSpec().getStorageClassSpec() == DeclSpec::SCS_static ||
         (getName().Kind == UnqualifiedId::IK_OperatorFunctionId &&
          CXXMethodDecl::isStaticOverloadedOperator(
              getName().OperatorFunctionId.Operator));
}

bool DeclSpec::hasTagDefinition() const {
  if (!TypeSpecOwned)
    return false;
  return cast<TagDecl>(getRepAsDecl())->isCompleteDefinition();
}

/// getParsedSpecifiers - Return a bitmask of which flavors of specifiers this
/// declaration specifier includes.
///
unsigned DeclSpec::getParsedSpecifiers() const {
  unsigned Res = 0;
  if (StorageClassSpec != SCS_unspecified ||
      ThreadStorageClassSpec != TSCS_unspecified)
    Res |= PQ_StorageClassSpecifier;

  if (TypeQualifiers != TQ_unspecified)
    Res |= PQ_TypeQualifier;

  if (hasTypeSpecifier())
    Res |= PQ_TypeSpecifier;

  if (FS_inline_specified || FS_virtual_specified || FS_explicit_specified ||
      FS_noreturn_specified || FS_forceinline_specified)
    Res |= PQ_FunctionSpecifier;
  return Res;
}

template <class T> static bool BadSpecifier(T TNew, T TPrev,
                                            const char *&PrevSpec,
                                            unsigned &DiagID,
                                            bool IsExtension = true) {
  PrevSpec = DeclSpec::getSpecifierName(TPrev);
  if (TNew != TPrev)
    DiagID = diag::err_invalid_decl_spec_combination;
  else
    DiagID = IsExtension ? diag::ext_duplicate_declspec : 
                           diag::warn_duplicate_declspec;    
  return true;
}

const char *DeclSpec::getSpecifierName(DeclSpec::SCS S) {
  switch (S) {
  case DeclSpec::SCS_unspecified: return "unspecified";
  case DeclSpec::SCS_typedef:     return "typedef";
  case DeclSpec::SCS_extern:      return "extern";
  case DeclSpec::SCS_static:      return "static";
  case DeclSpec::SCS_auto:        return "auto";
  case DeclSpec::SCS_register:    return "register";
  case DeclSpec::SCS_private_extern: return "__private_extern__";
  case DeclSpec::SCS_mutable:     return "mutable";
  }
  llvm_unreachable("Unknown typespec!");
}

const char *DeclSpec::getSpecifierName(DeclSpec::TSCS S) {
  switch (S) {
  case DeclSpec::TSCS_unspecified:   return "unspecified";
  case DeclSpec::TSCS___thread:      return "__thread";
  case DeclSpec::TSCS_thread_local:  return "thread_local";
  case DeclSpec::TSCS__Thread_local: return "_Thread_local";
  }
  llvm_unreachable("Unknown typespec!");
}

const char *DeclSpec::getSpecifierName(TSW W) {
  switch (W) {
  case TSW_unspecified: return "unspecified";
  case TSW_short:       return "short";
  case TSW_long:        return "long";
  case TSW_longlong:    return "long long";
  }
  llvm_unreachable("Unknown typespec!");
}

const char *DeclSpec::getSpecifierName(TSC C) {
  switch (C) {
  case TSC_unspecified: return "unspecified";
  case TSC_imaginary:   return "imaginary";
  case TSC_complex:     return "complex";
  }
  llvm_unreachable("Unknown typespec!");
}


const char *DeclSpec::getSpecifierName(TSS S) {
  switch (S) {
  case TSS_unspecified: return "unspecified";
  case TSS_signed:      return "signed";
  case TSS_unsigned:    return "unsigned";
  }
  llvm_unreachable("Unknown typespec!");
}

const char *DeclSpec::getSpecifierName(DeclSpec::TST T,
                                       const PrintingPolicy &Policy) {
  switch (T) {
  case DeclSpec::TST_unspecified: return "unspecified";
  case DeclSpec::TST_void:        return "void";
  case DeclSpec::TST_char:        return "char";
  case DeclSpec::TST_wchar:       return Policy.MSWChar ? "__wchar_t" : "wchar_t";
  case DeclSpec::TST_char16:      return "char16_t";
  case DeclSpec::TST_char32:      return "char32_t";
  case DeclSpec::TST_int:         return "int";
  case DeclSpec::TST_int128:      return "__int128";
  // HLSL Change Starts
  case DeclSpec::TST_min16float:  return "min16float";
  case DeclSpec::TST_min16int:    return "min16int";
  case DeclSpec::TST_min16uint:   return "min16uint";
  case DeclSpec::TST_min10float:  return "min10float";
  case DeclSpec::TST_min12int:    return "min12int";
  case DeclSpec::TST_halffloat:
  case DeclSpec::TST_int8_4packed:  return "int8_t4_packed";
  case DeclSpec::TST_uint8_4packed: return "uint8_t4_packed";
    // HLSL Change Ends
  case DeclSpec::TST_half:        return "half";
  case DeclSpec::TST_float:       return "float";
  case DeclSpec::TST_double:      return "double";
  case DeclSpec::TST_bool:        return Policy.Bool ? "bool" : "_Bool";
  case DeclSpec::TST_decimal32:   return "_Decimal32";
  case DeclSpec::TST_decimal64:   return "_Decimal64";
  case DeclSpec::TST_decimal128:  return "_Decimal128";
  case DeclSpec::TST_enum:        return "enum";
  case DeclSpec::TST_class:       return "class";
  case DeclSpec::TST_union:       return "union";
  case DeclSpec::TST_struct:      return "struct";
  case DeclSpec::TST_interface:   return "__interface";
  case DeclSpec::TST_typename:    return "type-name";
  case DeclSpec::TST_typeofType:
  case DeclSpec::TST_typeofExpr:  return "typeof";
  case DeclSpec::TST_auto:        return "auto";
  case DeclSpec::TST_decltype:    return "(decltype)";
  case DeclSpec::TST_decltype_auto: return "decltype(auto)";
  case DeclSpec::TST_underlyingType: return "__underlying_type";
  case DeclSpec::TST_unknown_anytype: return "__unknown_anytype";
  case DeclSpec::TST_atomic: return "_Atomic";
  case DeclSpec::TST_error:       return "(error)";
  }
  llvm_unreachable("Unknown typespec!");
}

const char *DeclSpec::getSpecifierName(TQ T) {
  switch (T) {
  case DeclSpec::TQ_unspecified: return "unspecified";
  case DeclSpec::TQ_const:       return "const";
  case DeclSpec::TQ_restrict:    return "restrict";
  case DeclSpec::TQ_volatile:    return "volatile";
  case DeclSpec::TQ_atomic:      return "_Atomic";
  }
  llvm_unreachable("Unknown typespec!");
}

bool DeclSpec::SetStorageClassSpec(Sema &S, SCS SC, SourceLocation Loc,
                                   const char *&PrevSpec,
                                   unsigned &DiagID,
                                   const PrintingPolicy &Policy) {
  // OpenCL v1.1 s6.8g: "The extern, static, auto and register storage-class
  // specifiers are not supported.
  // It seems sensible to prohibit private_extern too
  // The cl_clang_storage_class_specifiers extension enables support for
  // these storage-class specifiers.
  // OpenCL v1.2 s6.8 changes this to "The auto and register storage-class
  // specifiers are not supported."
  if (S.getLangOpts().OpenCL &&
      !S.getOpenCLOptions().cl_clang_storage_class_specifiers) {
    switch (SC) {
    case SCS_extern:
    case SCS_private_extern:
    case SCS_static:
        if (S.getLangOpts().OpenCLVersion < 120) {
          DiagID   = diag::err_opencl_unknown_type_specifier;
          PrevSpec = getSpecifierName(SC);
          return true;
        }
        break;
    case SCS_auto:
    case SCS_register:
      DiagID   = diag::err_opencl_unknown_type_specifier;
      PrevSpec = getSpecifierName(SC);
      return true;
    default:
      break;
    }
  }

  if (StorageClassSpec != SCS_unspecified) {
    // Maybe this is an attempt to use C++11 'auto' outside of C++11 mode.
    bool isInvalid = true;
    if (TypeSpecType == TST_unspecified && S.getLangOpts().CPlusPlus) {
      if (SC == SCS_auto)
        return SetTypeSpecType(TST_auto, Loc, PrevSpec, DiagID, Policy);
      if (StorageClassSpec == SCS_auto) {
        isInvalid = SetTypeSpecType(TST_auto, StorageClassSpecLoc,
                                    PrevSpec, DiagID, Policy);
        assert(!isInvalid && "auto SCS -> TST recovery failed");
      }
    }

    // Changing storage class is allowed only if the previous one
    // was the 'extern' that is part of a linkage specification and
    // the new storage class is 'typedef'.
    if (isInvalid &&
        !(SCS_extern_in_linkage_spec &&
          StorageClassSpec == SCS_extern &&
          SC == SCS_typedef))
      return BadSpecifier(SC, (SCS)StorageClassSpec, PrevSpec, DiagID);
  }
  StorageClassSpec = SC;
  StorageClassSpecLoc = Loc;
  assert((unsigned)SC == StorageClassSpec && "SCS constants overflow bitfield");
  return false;
}

bool DeclSpec::SetStorageClassSpecThread(TSCS TSC, SourceLocation Loc,
                                         const char *&PrevSpec,
                                         unsigned &DiagID) {
  if (ThreadStorageClassSpec != TSCS_unspecified)
    return BadSpecifier(TSC, (TSCS)ThreadStorageClassSpec, PrevSpec, DiagID);

  ThreadStorageClassSpec = TSC;
  ThreadStorageClassSpecLoc = Loc;
  return false;
}


/// These methods set the specified attribute of the DeclSpec, but return true
/// and ignore the request if invalid (e.g. "extern" then "auto" is
/// specified).
bool DeclSpec::SetTypeSpecWidth(TSW W, SourceLocation Loc,
                                const char *&PrevSpec,
                                unsigned &DiagID,
                                const PrintingPolicy &Policy) {
  // Overwrite TSWLoc only if TypeSpecWidth was unspecified, so that
  // for 'long long' we will keep the source location of the first 'long'.
  if (TypeSpecWidth == TSW_unspecified)
    TSWLoc = Loc;
  // Allow turning long -> long long.
  else if (W != TSW_longlong || TypeSpecWidth != TSW_long)
    return BadSpecifier(W, (TSW)TypeSpecWidth, PrevSpec, DiagID);
  TypeSpecWidth = W;
  return false;
}

bool DeclSpec::SetTypeSpecComplex(TSC C, SourceLocation Loc,
                                  const char *&PrevSpec,
                                  unsigned &DiagID) {
  if (TypeSpecComplex != TSC_unspecified)
    return BadSpecifier(C, (TSC)TypeSpecComplex, PrevSpec, DiagID);
  TypeSpecComplex = C;
  TSCLoc = Loc;
  return false;
}

bool DeclSpec::SetTypeSpecSign(TSS S, SourceLocation Loc,
                               const char *&PrevSpec,
                               unsigned &DiagID) {
  if (TypeSpecSign != TSS_unspecified)
    return BadSpecifier(S, (TSS)TypeSpecSign, PrevSpec, DiagID);
  TypeSpecSign = S;
  TSSLoc = Loc;
  return false;
}

bool DeclSpec::SetTypeSpecType(TST T, SourceLocation Loc,
                               const char *&PrevSpec,
                               unsigned &DiagID,
                               ParsedType Rep,
                               const PrintingPolicy &Policy) {
  return SetTypeSpecType(T, Loc, Loc, PrevSpec, DiagID, Rep, Policy);
}

bool DeclSpec::SetTypeSpecType(TST T, SourceLocation TagKwLoc,
                               SourceLocation TagNameLoc,
                               const char *&PrevSpec,
                               unsigned &DiagID,
                               ParsedType Rep,
                               const PrintingPolicy &Policy) {
  assert(isTypeRep(T) && "T does not store a type");
  assert(Rep && "no type provided!");
  if (TypeSpecType != TST_unspecified) {
    PrevSpec = DeclSpec::getSpecifierName((TST) TypeSpecType, Policy);
    DiagID = diag::err_invalid_decl_spec_combination;
    return true;
  }
  TypeSpecType = T;
  TypeRep = Rep;
  TSTLoc = TagKwLoc;
  TSTNameLoc = TagNameLoc;
  TypeSpecOwned = false;
  return false;
}

bool DeclSpec::SetTypeSpecType(TST T, SourceLocation Loc,
                               const char *&PrevSpec,
                               unsigned &DiagID,
                               Expr *Rep,
                               const PrintingPolicy &Policy) {
  assert(isExprRep(T) && "T does not store an expr");
  assert(Rep && "no expression provided!");
  if (TypeSpecType != TST_unspecified) {
    PrevSpec = DeclSpec::getSpecifierName((TST) TypeSpecType, Policy);
    DiagID = diag::err_invalid_decl_spec_combination;
    return true;
  }
  TypeSpecType = T;
  ExprRep = Rep;
  TSTLoc = Loc;
  TSTNameLoc = Loc;
  TypeSpecOwned = false;
  return false;
}

bool DeclSpec::SetTypeSpecType(TST T, SourceLocation Loc,
                               const char *&PrevSpec,
                               unsigned &DiagID,
                               Decl *Rep, bool Owned,
                               const PrintingPolicy &Policy) {
  return SetTypeSpecType(T, Loc, Loc, PrevSpec, DiagID, Rep, Owned, Policy);
}

bool DeclSpec::SetTypeSpecType(TST T, SourceLocation TagKwLoc,
                               SourceLocation TagNameLoc,
                               const char *&PrevSpec,
                               unsigned &DiagID,
                               Decl *Rep, bool Owned,
                               const PrintingPolicy &Policy) {
  assert(isDeclRep(T) && "T does not store a decl");
  // Unlike the other cases, we don't assert that we actually get a decl.

  if (TypeSpecType != TST_unspecified) {
    PrevSpec = DeclSpec::getSpecifierName((TST) TypeSpecType, Policy);
    DiagID = diag::err_invalid_decl_spec_combination;
    return true;
  }
  TypeSpecType = T;
  DeclRep = Rep;
  TSTLoc = TagKwLoc;
  TSTNameLoc = TagNameLoc;
  TypeSpecOwned = Owned && Rep != nullptr;
  return false;
}

bool DeclSpec::SetTypeSpecType(TST T, SourceLocation Loc,
                               const char *&PrevSpec,
                               unsigned &DiagID,
                               const PrintingPolicy &Policy) {
  assert(!isDeclRep(T) && !isTypeRep(T) && !isExprRep(T) &&
         "rep required for these type-spec kinds!");
  if (TypeSpecType != TST_unspecified) {
    PrevSpec = DeclSpec::getSpecifierName((TST) TypeSpecType, Policy);
    DiagID = diag::err_invalid_decl_spec_combination;
    return true;
  }
  TSTLoc = Loc;
  TSTNameLoc = Loc;
  if (TypeAltiVecVector && (T == TST_bool) && !TypeAltiVecBool) {
    TypeAltiVecBool = true;
    return false;
  }
  TypeSpecType = T;
  TypeSpecOwned = false;
  return false;
}

bool DeclSpec::SetTypeAltiVecVector(bool isAltiVecVector, SourceLocation Loc,
                          const char *&PrevSpec, unsigned &DiagID,
                          const PrintingPolicy &Policy) {
  if (TypeSpecType != TST_unspecified) {
    PrevSpec = DeclSpec::getSpecifierName((TST) TypeSpecType, Policy);
    DiagID = diag::err_invalid_vector_decl_spec_combination;
    return true;
  }
  TypeAltiVecVector = isAltiVecVector;
  AltiVecLoc = Loc;
  return false;
}

bool DeclSpec::SetTypeAltiVecPixel(bool isAltiVecPixel, SourceLocation Loc,
                          const char *&PrevSpec, unsigned &DiagID,
                          const PrintingPolicy &Policy) {
  if (!TypeAltiVecVector || TypeAltiVecPixel ||
      (TypeSpecType != TST_unspecified)) {
    PrevSpec = DeclSpec::getSpecifierName((TST) TypeSpecType, Policy);
    DiagID = diag::err_invalid_pixel_decl_spec_combination;
    return true;
  }
  TypeAltiVecPixel = isAltiVecPixel;
  TSTLoc = Loc;
  TSTNameLoc = Loc;
  return false;
}

bool DeclSpec::SetTypeAltiVecBool(bool isAltiVecBool, SourceLocation Loc,
                                  const char *&PrevSpec, unsigned &DiagID,
                                  const PrintingPolicy &Policy) {
  if (!TypeAltiVecVector || TypeAltiVecBool ||
      (TypeSpecType != TST_unspecified)) {
    PrevSpec = DeclSpec::getSpecifierName((TST) TypeSpecType, Policy);
    DiagID = diag::err_invalid_vector_bool_decl_spec;
    return true;
  }
  TypeAltiVecBool = isAltiVecBool;
  TSTLoc = Loc;
  TSTNameLoc = Loc;
  return false;
}

bool DeclSpec::SetTypeSpecError() {
  TypeSpecType = TST_error;
  TypeSpecOwned = false;
  TSTLoc = SourceLocation();
  TSTNameLoc = SourceLocation();
  return false;
}

bool DeclSpec::SetTypeQual(TQ T, SourceLocation Loc, const char *&PrevSpec,
                           unsigned &DiagID, const LangOptions &Lang) {
  // Duplicates are permitted in C99 onwards, but are not permitted in C89 or
  // C++.  However, since this is likely not what the user intended, we will
  // always warn.  We do not need to set the qualifier's location since we
  // already have it.
  if (TypeQualifiers & T) {
    bool IsExtension = true;
    if (Lang.C99)
      IsExtension = false;
    return BadSpecifier(T, T, PrevSpec, DiagID, IsExtension);
  }
  TypeQualifiers |= T;

  switch (T) {
  case TQ_unspecified: break;
  case TQ_const:    TQ_constLoc = Loc; return false;
  case TQ_restrict: TQ_restrictLoc = Loc; return false;
  case TQ_volatile: TQ_volatileLoc = Loc; return false;
  case TQ_atomic:   TQ_atomicLoc = Loc; return false;
  }

  llvm_unreachable("Unknown type qualifier!");
}

bool DeclSpec::setFunctionSpecInline(SourceLocation Loc, const char *&PrevSpec,
                                     unsigned &DiagID) {
  // 'inline inline' is ok.  However, since this is likely not what the user
  // intended, we will always warn, similar to duplicates of type qualifiers.
  if (FS_inline_specified) {
    DiagID = diag::warn_duplicate_declspec;
    PrevSpec = "inline";
    return true;
  }
  FS_inline_specified = true;
  FS_inlineLoc = Loc;
  return false;
}

bool DeclSpec::setFunctionSpecForceInline(SourceLocation Loc, const char *&PrevSpec,
                                          unsigned &DiagID) {
  if (FS_forceinline_specified) {
    DiagID = diag::warn_duplicate_declspec;
    PrevSpec = "__forceinline";
    return true;
  }
  FS_forceinline_specified = true;
  FS_forceinlineLoc = Loc;
  return false;
}

bool DeclSpec::setFunctionSpecVirtual(SourceLocation Loc,
                                      const char *&PrevSpec,
                                      unsigned &DiagID) {
  // 'virtual virtual' is ok, but warn as this is likely not what the user
  // intended.
  if (FS_virtual_specified) {
    DiagID = diag::warn_duplicate_declspec;
    PrevSpec = "virtual";
    return true;
  }
  FS_virtual_specified = true;
  FS_virtualLoc = Loc;
  return false;
}

bool DeclSpec::setFunctionSpecExplicit(SourceLocation Loc,
                                       const char *&PrevSpec,
                                       unsigned &DiagID) {
  // 'explicit explicit' is ok, but warn as this is likely not what the user
  // intended.
  if (FS_explicit_specified) {
    DiagID = diag::warn_duplicate_declspec;
    PrevSpec = "explicit";
    return true;
  }
  FS_explicit_specified = true;
  FS_explicitLoc = Loc;
  return false;
}

bool DeclSpec::setFunctionSpecNoreturn(SourceLocation Loc,
                                       const char *&PrevSpec,
                                       unsigned &DiagID) {
  // '_Noreturn _Noreturn' is ok, but warn as this is likely not what the user
  // intended.
  if (FS_noreturn_specified) {
    DiagID = diag::warn_duplicate_declspec;
    PrevSpec = "_Noreturn";
    return true;
  }
  FS_noreturn_specified = true;
  FS_noreturnLoc = Loc;
  return false;
}

bool DeclSpec::SetFriendSpec(SourceLocation Loc, const char *&PrevSpec,
                             unsigned &DiagID) {
  if (Friend_specified) {
    PrevSpec = "friend";
    // Keep the later location, so that we can later diagnose ill-formed
    // declarations like 'friend class X friend;'. Per [class.friend]p3,
    // 'friend' must be the first token in a friend declaration that is
    // not a function declaration.
    FriendLoc = Loc;
    DiagID = diag::warn_duplicate_declspec;
    return true;
  }

  Friend_specified = true;
  FriendLoc = Loc;
  return false;
}

bool DeclSpec::setModulePrivateSpec(SourceLocation Loc, const char *&PrevSpec,
                                    unsigned &DiagID) {
  if (isModulePrivateSpecified()) {
    PrevSpec = "__module_private__";
    DiagID = diag::ext_duplicate_declspec;
    return true;
  }
  
  ModulePrivateLoc = Loc;
  return false;
}

bool DeclSpec::SetConstexprSpec(SourceLocation Loc, const char *&PrevSpec,
                                unsigned &DiagID) {
  // 'constexpr constexpr' is ok, but warn as this is likely not what the user
  // intended.
  if (Constexpr_specified) {
    DiagID = diag::warn_duplicate_declspec;
    PrevSpec = "constexpr";
    return true;
  }
  Constexpr_specified = true;
  ConstexprLoc = Loc;
  return false;
}

bool DeclSpec::SetConceptSpec(SourceLocation Loc, const char *&PrevSpec,
                              unsigned &DiagID) {
  if (Concept_specified) {
    DiagID = diag::ext_duplicate_declspec;
    PrevSpec = "concept";
    return true;
  }
  Concept_specified = true;
  ConceptLoc = Loc;
  return false;
}

void DeclSpec::SaveWrittenBuiltinSpecs() {
  writtenBS.Sign = getTypeSpecSign();
  writtenBS.Width = getTypeSpecWidth();
  writtenBS.Type = getTypeSpecType();
  // Search the list of attributes for the presence of a mode attribute.
  writtenBS.ModeAttr = false;
  AttributeList* attrs = getAttributes().getList();
  while (attrs) {
    if (attrs->getKind() == AttributeList::AT_Mode) {
      writtenBS.ModeAttr = true;
      break;
    }
    attrs = attrs->getNext();
  }
}

/// Finish - This does final analysis of the declspec, rejecting things like
/// "_Imaginary" (lacking an FP type).  This returns a diagnostic to issue or
/// diag::NUM_DIAGNOSTICS if there is no error.  After calling this method,
/// DeclSpec is guaranteed self-consistent, even if an error occurred.
void DeclSpec::Finish(DiagnosticsEngine &D, Preprocessor &PP, const PrintingPolicy &Policy) {
  // Before possibly changing their values, save specs as written.
  SaveWrittenBuiltinSpecs();

  // Check the type specifier components first.

  // If decltype(auto) is used, no other type specifiers are permitted.
  if (TypeSpecType == TST_decltype_auto &&
      (TypeSpecWidth != TSW_unspecified ||
       TypeSpecComplex != TSC_unspecified ||
       TypeSpecSign != TSS_unspecified ||
       TypeAltiVecVector || TypeAltiVecPixel || TypeAltiVecBool ||
       TypeQualifiers)) {
    const unsigned NumLocs = 8;
    SourceLocation ExtraLocs[NumLocs] = {
      TSWLoc, TSCLoc, TSSLoc, AltiVecLoc,
      TQ_constLoc, TQ_restrictLoc, TQ_volatileLoc, TQ_atomicLoc
    };
    FixItHint Hints[NumLocs];
    SourceLocation FirstLoc;
    for (unsigned I = 0; I != NumLocs; ++I) {
      if (!ExtraLocs[I].isInvalid()) {
        if (FirstLoc.isInvalid() ||
            PP.getSourceManager().isBeforeInTranslationUnit(ExtraLocs[I],
                                                            FirstLoc))
          FirstLoc = ExtraLocs[I];
        Hints[I] = FixItHint::CreateRemoval(ExtraLocs[I]);
      }
    }
    TypeSpecWidth = TSW_unspecified;
    TypeSpecComplex = TSC_unspecified;
    TypeSpecSign = TSS_unspecified;
    TypeAltiVecVector = TypeAltiVecPixel = TypeAltiVecBool = false;
    TypeQualifiers = 0;
    Diag(D, TSTLoc, diag::err_decltype_auto_cannot_be_combined)
      << Hints[0] << Hints[1] << Hints[2] << Hints[3]
      << Hints[4] << Hints[5] << Hints[6] << Hints[7];
  }

  // Validate and finalize AltiVec vector declspec.
  if (TypeAltiVecVector) {
    if (TypeAltiVecBool) {
      // Sign specifiers are not allowed with vector bool. (PIM 2.1)
      if (TypeSpecSign != TSS_unspecified) {
        Diag(D, TSSLoc, diag::err_invalid_vector_bool_decl_spec)
          << getSpecifierName((TSS)TypeSpecSign);
      }

      // Only char/int are valid with vector bool. (PIM 2.1)
      if (((TypeSpecType != TST_unspecified) && (TypeSpecType != TST_char) &&
           (TypeSpecType != TST_int)) || TypeAltiVecPixel) {
        Diag(D, TSTLoc, diag::err_invalid_vector_bool_decl_spec)
          << (TypeAltiVecPixel ? "__pixel" :
                                 getSpecifierName((TST)TypeSpecType, Policy));
      }

      // Only 'short' and 'long long' are valid with vector bool. (PIM 2.1)
      if ((TypeSpecWidth != TSW_unspecified) && (TypeSpecWidth != TSW_short) &&
          (TypeSpecWidth != TSW_longlong))
        Diag(D, TSWLoc, diag::err_invalid_vector_bool_decl_spec)
          << getSpecifierName((TSW)TypeSpecWidth);

      // vector bool long long requires VSX support or ZVector.
      if ((TypeSpecWidth == TSW_longlong) &&
          (!PP.getTargetInfo().hasFeature("vsx")) &&
          (!PP.getTargetInfo().hasFeature("power8-vector")) &&
          !PP.getLangOpts().ZVector)
        Diag(D, TSTLoc, diag::err_invalid_vector_long_long_decl_spec);

      // Elements of vector bool are interpreted as unsigned. (PIM 2.1)
      if ((TypeSpecType == TST_char) || (TypeSpecType == TST_int) ||
          (TypeSpecWidth != TSW_unspecified))
        TypeSpecSign = TSS_unsigned;
    } else if (TypeSpecType == TST_double) {
      // vector long double and vector long long double are never allowed.
      // vector double is OK for Power7 and later, and ZVector.
      if (TypeSpecWidth == TSW_long || TypeSpecWidth == TSW_longlong)
        Diag(D, TSWLoc, diag::err_invalid_vector_long_double_decl_spec);
      else if (!PP.getTargetInfo().hasFeature("vsx") &&
               !PP.getLangOpts().ZVector)
        Diag(D, TSTLoc, diag::err_invalid_vector_double_decl_spec);
    } else if (TypeSpecType == TST_float) {
      // vector float is unsupported for ZVector.
      if (PP.getLangOpts().ZVector)
        Diag(D, TSTLoc, diag::err_invalid_vector_float_decl_spec);
    } else if (TypeSpecWidth == TSW_long) {
      // vector long is unsupported for ZVector and deprecated for AltiVec.
      if (PP.getLangOpts().ZVector)
        Diag(D, TSWLoc, diag::err_invalid_vector_long_decl_spec);
      else
        Diag(D, TSWLoc, diag::warn_vector_long_decl_spec_combination)
          << getSpecifierName((TST)TypeSpecType, Policy);
    }

    if (TypeAltiVecPixel) {
      //TODO: perform validation
      TypeSpecType = TST_int;
      TypeSpecSign = TSS_unsigned;
      TypeSpecWidth = TSW_short;
      TypeSpecOwned = false;
    }
  }

  // signed/unsigned are only valid with int/char/wchar_t.
  if (TypeSpecSign != TSS_unspecified) {
    // HLSL Change starts - signed/unsigned are not complete type specifiers.
    #if 0
    if (TypeSpecType == TST_unspecified)
      TypeSpecType = TST_int;
    #endif
    // shorthand vectors and matrices can have signed/unsigned specifiers.
    // If other typenames are used with signed/unsigned, it is already diagnosed by hlsl external source
    if (TypeSpecType != TST_int && TypeSpecType != TST_int128 &&
        TypeSpecType != TST_char && TypeSpecType != TST_wchar &&
        TypeSpecType != TST_typename) {
      Diag(D, TSSLoc, diag::err_invalid_sign_spec)
        << getSpecifierName((TST)TypeSpecType, Policy);
      // signed double -> double.
      TypeSpecSign = TSS_unspecified;
    }
    // HLSL Change end
  }

  // Validate the width of the type.
  switch (TypeSpecWidth) {
  case TSW_unspecified: break;
  case TSW_short:    // short int
  case TSW_longlong: // long long int
    if (TypeSpecType == TST_unspecified)
      TypeSpecType = TST_int; // short -> short int, long long -> long long int.
    else if (TypeSpecType != TST_int) {
      Diag(D, TSWLoc,
           TypeSpecWidth == TSW_short ? diag::err_invalid_short_spec
                                      : diag::err_invalid_longlong_spec)
        <<  getSpecifierName((TST)TypeSpecType, Policy);
      TypeSpecType = TST_int;
      TypeSpecOwned = false;
    }
    break;
  case TSW_long:  // long double, long int
    if (TypeSpecType == TST_unspecified)
      TypeSpecType = TST_int;  // long -> long int.
    else if (TypeSpecType != TST_int && TypeSpecType != TST_double) {
      Diag(D, TSWLoc, diag::err_invalid_long_spec)
        << getSpecifierName((TST)TypeSpecType, Policy);
      TypeSpecType = TST_int;
      TypeSpecOwned = false;
    }
    break;
  }

  // TODO: if the implementation does not implement _Complex or _Imaginary,
  // disallow their use.  Need information about the backend.
  if (TypeSpecComplex != TSC_unspecified) {
    if (TypeSpecType == TST_unspecified) {
      Diag(D, TSCLoc, diag::ext_plain_complex)
        << FixItHint::CreateInsertion(
                              PP.getLocForEndOfToken(getTypeSpecComplexLoc()),
                                                 " double");
      TypeSpecType = TST_double;   // _Complex -> _Complex double.
    } else if (TypeSpecType == TST_int || TypeSpecType == TST_char) {
      // Note that this intentionally doesn't include _Complex _Bool.
      if (!PP.getLangOpts().CPlusPlus)
        Diag(D, TSTLoc, diag::ext_integer_complex);
    } else if (TypeSpecType != TST_float && TypeSpecType != TST_double) {
      Diag(D, TSCLoc, diag::err_invalid_complex_spec)
        << getSpecifierName((TST)TypeSpecType, Policy);
      TypeSpecComplex = TSC_unspecified;
    }
  }

  // C11 6.7.1/3, C++11 [dcl.stc]p1, GNU TLS: __thread, thread_local and
  // _Thread_local can only appear with the 'static' and 'extern' storage class
  // specifiers. We also allow __private_extern__ as an extension.
  if (ThreadStorageClassSpec != TSCS_unspecified) {
    switch (StorageClassSpec) {
    case SCS_unspecified:
    case SCS_extern:
    case SCS_private_extern:
    case SCS_static:
      break;
    default:
      if (PP.getSourceManager().isBeforeInTranslationUnit(
            getThreadStorageClassSpecLoc(), getStorageClassSpecLoc()))
        Diag(D, getStorageClassSpecLoc(),
             diag::err_invalid_decl_spec_combination)
          << DeclSpec::getSpecifierName(getThreadStorageClassSpec())
          << SourceRange(getThreadStorageClassSpecLoc());
      else
        Diag(D, getThreadStorageClassSpecLoc(),
             diag::err_invalid_decl_spec_combination)
          << DeclSpec::getSpecifierName(getStorageClassSpec())
          << SourceRange(getStorageClassSpecLoc());
      // Discard the thread storage class specifier to recover.
      ThreadStorageClassSpec = TSCS_unspecified;
      ThreadStorageClassSpecLoc = SourceLocation();
    }
  }

  // If no type specifier was provided and we're parsing a language where
  // the type specifier is not optional, but we got 'auto' as a storage
  // class specifier, then assume this is an attempt to use C++0x's 'auto'
  // type specifier.
  if (PP.getLangOpts().CPlusPlus &&
      TypeSpecType == TST_unspecified && StorageClassSpec == SCS_auto) {
    TypeSpecType = TST_auto;
    StorageClassSpec = SCS_unspecified;
    TSTLoc = TSTNameLoc = StorageClassSpecLoc;
    StorageClassSpecLoc = SourceLocation();
  }
  // Diagnose if we've recovered from an ill-formed 'auto' storage class
  // specifier in a pre-C++11 dialect of C++.
  if (!PP.getLangOpts().CPlusPlus11 && TypeSpecType == TST_auto)
    Diag(D, TSTLoc, diag::ext_auto_type_specifier);
  if (PP.getLangOpts().CPlusPlus && !PP.getLangOpts().CPlusPlus11 &&
      StorageClassSpec == SCS_auto)
    Diag(D, StorageClassSpecLoc, diag::warn_auto_storage_class)
      << FixItHint::CreateRemoval(StorageClassSpecLoc);
  if (TypeSpecType == TST_char16 || TypeSpecType == TST_char32)
    Diag(D, TSTLoc, diag::warn_cxx98_compat_unicode_type)
      << (TypeSpecType == TST_char16 ? "char16_t" : "char32_t");
  if (Constexpr_specified)
    Diag(D, ConstexprLoc, diag::warn_cxx98_compat_constexpr);

  // C++ [class.friend]p6:
  //   No storage-class-specifier shall appear in the decl-specifier-seq
  //   of a friend declaration.
  if (isFriendSpecified() &&
      (getStorageClassSpec() || getThreadStorageClassSpec())) {
    SmallString<32> SpecName;
    SourceLocation SCLoc;
    FixItHint StorageHint, ThreadHint;

    if (DeclSpec::SCS SC = getStorageClassSpec()) {
      SpecName = getSpecifierName(SC);
      SCLoc = getStorageClassSpecLoc();
      StorageHint = FixItHint::CreateRemoval(SCLoc);
    }

    if (DeclSpec::TSCS TSC = getThreadStorageClassSpec()) {
      if (!SpecName.empty()) SpecName += " ";
      SpecName += getSpecifierName(TSC);
      SCLoc = getThreadStorageClassSpecLoc();
      ThreadHint = FixItHint::CreateRemoval(SCLoc);
    }

    Diag(D, SCLoc, diag::err_friend_decl_spec)
      << SpecName << StorageHint << ThreadHint;

    ClearStorageClassSpecs();
  }

  // C++11 [dcl.fct.spec]p5:
  //   The virtual specifier shall be used only in the initial
  //   declaration of a non-static class member function;
  // C++11 [dcl.fct.spec]p6:
  //   The explicit specifier shall be used only in the declaration of
  //   a constructor or conversion function within its class
  //   definition;
  if (isFriendSpecified() && (isVirtualSpecified() || isExplicitSpecified())) {
    StringRef Keyword;
    SourceLocation SCLoc;

    if (isVirtualSpecified()) {
      Keyword = "virtual";
      SCLoc = getVirtualSpecLoc();
    } else {
      Keyword = "explicit";
      SCLoc = getExplicitSpecLoc();
    }

    FixItHint Hint = FixItHint::CreateRemoval(SCLoc);
    Diag(D, SCLoc, diag::err_friend_decl_spec)
      << Keyword << Hint;

    FS_virtual_specified = FS_explicit_specified = false;
    FS_virtualLoc = FS_explicitLoc = SourceLocation();
  }

  assert(!TypeSpecOwned || isDeclRep((TST) TypeSpecType));

  // Okay, now we can infer the real type.

  // TODO: return "auto function" and other bad things based on the real type.

  // 'data definition has no type or storage class'?
}

bool DeclSpec::isMissingDeclaratorOk() {
  TST tst = getTypeSpecType();
  return isDeclRep(tst) && getRepAsDecl() != nullptr &&
    StorageClassSpec != DeclSpec::SCS_typedef;
}

void UnqualifiedId::setOperatorFunctionId(SourceLocation OperatorLoc, 
                                          OverloadedOperatorKind Op,
                                          SourceLocation SymbolLocations[3]) {
  Kind = IK_OperatorFunctionId;
  StartLocation = OperatorLoc;
  EndLocation = OperatorLoc;
  OperatorFunctionId.Operator = Op;
  for (unsigned I = 0; I != 3; ++I) {
    OperatorFunctionId.SymbolLocations[I] = SymbolLocations[I].getRawEncoding();
    
    if (SymbolLocations[I].isValid())
      EndLocation = SymbolLocations[I];
  }
}

bool VirtSpecifiers::SetSpecifier(Specifier VS, SourceLocation Loc,
                                  const char *&PrevSpec) {
  if (!FirstLocation.isValid())
    FirstLocation = Loc;
  LastLocation = Loc;
  LastSpecifier = VS;
  
  if (Specifiers & VS) {
    PrevSpec = getSpecifierName(VS);
    return true;
  }

  Specifiers |= VS;

  switch (VS) {
  default: llvm_unreachable("Unknown specifier!");
  case VS_Override: VS_overrideLoc = Loc; break;
  case VS_Sealed:
  case VS_Final:    VS_finalLoc = Loc; break;
  }

  return false;
}

const char *VirtSpecifiers::getSpecifierName(Specifier VS) {
  switch (VS) {
  default: llvm_unreachable("Unknown specifier");
  case VS_Override: return "override";
  case VS_Final: return "final";
  case VS_Sealed: return "sealed";
  }
}
