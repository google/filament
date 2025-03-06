//===--- ASTCommon.cpp - Common stuff for ASTReader/ASTWriter----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines common functions that both ASTReader and ASTWriter use.
//
//===----------------------------------------------------------------------===//

#include "ASTCommon.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclObjC.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Serialization/ASTDeserializationListener.h"
#include "llvm/ADT/StringExtras.h"

using namespace clang;

// Give ASTDeserializationListener's VTable a home.
ASTDeserializationListener::~ASTDeserializationListener() { }

serialization::TypeIdx
serialization::TypeIdxFromBuiltin(const BuiltinType *BT) {
  unsigned ID = 0;
  switch (BT->getKind()) {
  case BuiltinType::Void:       ID = PREDEF_TYPE_VOID_ID;       break;
  case BuiltinType::Bool:       ID = PREDEF_TYPE_BOOL_ID;       break;
  case BuiltinType::Char_U:     ID = PREDEF_TYPE_CHAR_U_ID;     break;
  case BuiltinType::UChar:      ID = PREDEF_TYPE_UCHAR_ID;      break;
  case BuiltinType::UShort:     ID = PREDEF_TYPE_USHORT_ID;     break;
  case BuiltinType::UInt:       ID = PREDEF_TYPE_UINT_ID;       break;
  case BuiltinType::ULong:      ID = PREDEF_TYPE_ULONG_ID;      break;
  case BuiltinType::ULongLong:  ID = PREDEF_TYPE_ULONGLONG_ID;  break;
  case BuiltinType::UInt128:    ID = PREDEF_TYPE_UINT128_ID;    break;
  case BuiltinType::Char_S:     ID = PREDEF_TYPE_CHAR_S_ID;     break;
  case BuiltinType::SChar:      ID = PREDEF_TYPE_SCHAR_ID;      break;
  case BuiltinType::WChar_S:
  case BuiltinType::WChar_U:    ID = PREDEF_TYPE_WCHAR_ID;      break;
  case BuiltinType::Short:      ID = PREDEF_TYPE_SHORT_ID;      break;
  case BuiltinType::Int:        ID = PREDEF_TYPE_INT_ID;        break;
  case BuiltinType::Long:       ID = PREDEF_TYPE_LONG_ID;       break;
  case BuiltinType::LongLong:   ID = PREDEF_TYPE_LONGLONG_ID;   break;
  case BuiltinType::Int128:     ID = PREDEF_TYPE_INT128_ID;     break;
  case BuiltinType::Half:       ID = PREDEF_TYPE_HALF_ID;       break;
  case BuiltinType::Float:      ID = PREDEF_TYPE_FLOAT_ID;      break;
  case BuiltinType::Double:     ID = PREDEF_TYPE_DOUBLE_ID;     break;
  case BuiltinType::LongDouble: ID = PREDEF_TYPE_LONGDOUBLE_ID; break;
  case BuiltinType::NullPtr:    ID = PREDEF_TYPE_NULLPTR_ID;    break;
  case BuiltinType::Char16:     ID = PREDEF_TYPE_CHAR16_ID;     break;
  case BuiltinType::Char32:     ID = PREDEF_TYPE_CHAR32_ID;     break;
  case BuiltinType::Overload:   ID = PREDEF_TYPE_OVERLOAD_ID;   break;
  case BuiltinType::BoundMember:ID = PREDEF_TYPE_BOUND_MEMBER;  break;
  case BuiltinType::PseudoObject:ID = PREDEF_TYPE_PSEUDO_OBJECT;break;
  case BuiltinType::Dependent:  ID = PREDEF_TYPE_DEPENDENT_ID;  break;
  case BuiltinType::UnknownAny: ID = PREDEF_TYPE_UNKNOWN_ANY;   break;
  case BuiltinType::ARCUnbridgedCast:
                                ID = PREDEF_TYPE_ARC_UNBRIDGED_CAST; break;
  case BuiltinType::ObjCId:     ID = PREDEF_TYPE_OBJC_ID;       break;
  case BuiltinType::ObjCClass:  ID = PREDEF_TYPE_OBJC_CLASS;    break;
  case BuiltinType::ObjCSel:    ID = PREDEF_TYPE_OBJC_SEL;      break;
  case BuiltinType::OCLImage1d:       ID = PREDEF_TYPE_IMAGE1D_ID;      break;
  case BuiltinType::OCLImage1dArray:  ID = PREDEF_TYPE_IMAGE1D_ARR_ID;  break;
  case BuiltinType::OCLImage1dBuffer: ID = PREDEF_TYPE_IMAGE1D_BUFF_ID; break;
  case BuiltinType::OCLImage2d:       ID = PREDEF_TYPE_IMAGE2D_ID;      break;
  case BuiltinType::OCLImage2dArray:  ID = PREDEF_TYPE_IMAGE2D_ARR_ID;  break;
  case BuiltinType::OCLImage3d:       ID = PREDEF_TYPE_IMAGE3D_ID;      break;
  case BuiltinType::OCLSampler:       ID = PREDEF_TYPE_SAMPLER_ID;      break;
  case BuiltinType::OCLEvent:         ID = PREDEF_TYPE_EVENT_ID;        break;
  case BuiltinType::BuiltinFn:
                                ID = PREDEF_TYPE_BUILTIN_FN; break;

  }

  return TypeIdx(ID);
}

unsigned serialization::ComputeHash(Selector Sel) {
  unsigned N = Sel.getNumArgs();
  if (N == 0)
    ++N;
  unsigned R = 5381;
  for (unsigned I = 0; I != N; ++I)
    if (IdentifierInfo *II = Sel.getIdentifierInfoForSlot(I))
      R = llvm::HashString(II->getName(), R);
  return R;
}

const DeclContext *
serialization::getDefinitiveDeclContext(const DeclContext *DC) {
  switch (DC->getDeclKind()) {
  // These entities may have multiple definitions.
  case Decl::TranslationUnit:
  case Decl::ExternCContext:
  case Decl::Namespace:
  case Decl::LinkageSpec:
    return nullptr;

  // C/C++ tag types can only be defined in one place.
  case Decl::Enum:
  case Decl::Record:
    if (const TagDecl *Def = cast<TagDecl>(DC)->getDefinition())
      return Def;
    return nullptr;

  // FIXME: These can be defined in one place... except special member
  // functions and out-of-line definitions.
  case Decl::CXXRecord:
  case Decl::ClassTemplateSpecialization:
  case Decl::ClassTemplatePartialSpecialization:
    return nullptr;

  // Each function, method, and block declaration is its own DeclContext.
  case Decl::Function:
  case Decl::CXXMethod:
  case Decl::CXXConstructor:
  case Decl::CXXDestructor:
  case Decl::CXXConversion:
  case Decl::ObjCMethod:
  case Decl::Block:
  case Decl::Captured:
    // Objective C categories, category implementations, and class
    // implementations can only be defined in one place.
  case Decl::ObjCCategory:
  case Decl::ObjCCategoryImpl:
  case Decl::ObjCImplementation:
    return DC;

  case Decl::ObjCProtocol:
    if (const ObjCProtocolDecl *Def
          = cast<ObjCProtocolDecl>(DC)->getDefinition())
      return Def;
    return nullptr;

  // FIXME: These are defined in one place, but properties in class extensions
  // end up being back-patched into the main interface. See
  // Sema::HandlePropertyInClassExtension for the offending code.
  case Decl::ObjCInterface:
    return nullptr;

  default:
    llvm_unreachable("Unhandled DeclContext in AST reader");
  }
  
  llvm_unreachable("Unhandled decl kind");
}

bool serialization::isRedeclarableDeclKind(unsigned Kind) {
  switch (static_cast<Decl::Kind>(Kind)) {
  case Decl::TranslationUnit:
  case Decl::ExternCContext:
    // Special case of a "merged" declaration.
    return true;

  case Decl::Namespace:
  case Decl::NamespaceAlias:
  case Decl::Typedef:
  case Decl::TypeAlias:
  case Decl::Enum:
  case Decl::Record:
  case Decl::CXXRecord:
  case Decl::ClassTemplateSpecialization:
  case Decl::ClassTemplatePartialSpecialization:
  case Decl::VarTemplateSpecialization:
  case Decl::VarTemplatePartialSpecialization:
  case Decl::Function:
  case Decl::CXXMethod:
  case Decl::CXXConstructor:
  case Decl::CXXDestructor:
  case Decl::CXXConversion:
  case Decl::UsingShadow:
  case Decl::Var:
  case Decl::FunctionTemplate:
  case Decl::ClassTemplate:
  case Decl::VarTemplate:
  case Decl::TypeAliasTemplate:
  case Decl::ObjCProtocol:
  case Decl::ObjCInterface:
  case Decl::Empty:
    return true;

  // Never redeclarable.
  case Decl::UsingDirective:
  case Decl::Label:
  case Decl::UnresolvedUsingTypename:
  case Decl::TemplateTypeParm:
  case Decl::EnumConstant:
  case Decl::UnresolvedUsingValue:
  case Decl::IndirectField:
  case Decl::Field:
  case Decl::MSProperty:
  case Decl::ObjCIvar:
  case Decl::ObjCAtDefsField:
  case Decl::NonTypeTemplateParm:
  case Decl::TemplateTemplateParm:
  case Decl::Using:
  case Decl::ObjCMethod:
  case Decl::ObjCCategory:
  case Decl::ObjCCategoryImpl:
  case Decl::ObjCImplementation:
  case Decl::ObjCProperty:
  case Decl::ObjCCompatibleAlias:
  case Decl::LinkageSpec:
  case Decl::ObjCPropertyImpl:
  case Decl::FileScopeAsm:
  case Decl::AccessSpec:
  case Decl::Friend:
  case Decl::FriendTemplate:
  case Decl::StaticAssert:
  case Decl::Block:
  case Decl::Captured:
  case Decl::ClassScopeFunctionSpecialization:
  case Decl::Import:
  case Decl::OMPThreadPrivate:
    return false;

  // These indirectly derive from Redeclarable<T> but are not actually
  // redeclarable.
  case Decl::ImplicitParam:
  case Decl::ParmVar:
  case Decl::ObjCTypeParam:
    return false;
  }

  llvm_unreachable("Unhandled declaration kind");
}

bool serialization::needsAnonymousDeclarationNumber(const NamedDecl *D) {
  // Friend declarations in dependent contexts aren't anonymous in the usual
  // sense, but they cannot be found by name lookup in their semantic context
  // (or indeed in any context), so we treat them as anonymous.
  //
  // This doesn't apply to friend tag decls; Sema makes those available to name
  // lookup in the surrounding context.
  if (D->getFriendObjectKind() &&
      D->getLexicalDeclContext()->isDependentContext() && !isa<TagDecl>(D)) {
    // For function templates and class templates, the template is numbered and
    // not its pattern.
    if (auto *FD = dyn_cast<FunctionDecl>(D))
      return !FD->getDescribedFunctionTemplate();
    if (auto *RD = dyn_cast<CXXRecordDecl>(D))
      return !RD->getDescribedClassTemplate();
    return true;
  }

  // Otherwise, we only care about anonymous class members.
  if (D->getDeclName() || !isa<CXXRecordDecl>(D->getLexicalDeclContext()))
    return false;
  return isa<TagDecl>(D) || isa<FieldDecl>(D);
}

