//===--- ASTWriter.cpp - AST File Writer ----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the ASTWriter class, which writes AST files.
//
//===----------------------------------------------------------------------===//

#include "clang/Serialization/ASTWriter.h"
#include "ASTCommon.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclContextInternals.h"
#include "clang/AST/DeclFriend.h"
#include "clang/AST/DeclLookups.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/Type.h"
#include "clang/AST/TypeLocVisitor.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/FileSystemStatCache.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/SourceManagerInternals.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/Version.h"
#include "clang/Basic/VersionTuple.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/HeaderSearchOptions.h"
#include "clang/Lex/MacroInfo.h"
#include "clang/Lex/PreprocessingRecord.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Sema/IdentifierResolver.h"
#include "clang/Sema/Sema.h"
#include "clang/Serialization/ASTReader.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Bitcode/BitstreamWriter.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/OnDiskHashTable.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Process.h"
#include <algorithm>
#include <cstdio>
#include <string.h>
#include <utility>
using namespace clang;
using namespace clang::serialization;

template <typename T, typename Allocator>
static StringRef bytes(const std::vector<T, Allocator> &v) {
  if (v.empty()) return StringRef();
  return StringRef(reinterpret_cast<const char*>(&v[0]),
                         sizeof(T) * v.size());
}

template <typename T>
static StringRef bytes(const SmallVectorImpl<T> &v) {
  return StringRef(reinterpret_cast<const char*>(v.data()),
                         sizeof(T) * v.size());
}

//===----------------------------------------------------------------------===//
// Type serialization
//===----------------------------------------------------------------------===//

namespace {
  class ASTTypeWriter {
    ASTWriter &Writer;
    ASTWriter::RecordDataImpl &Record;

  public:
    /// \brief Type code that corresponds to the record generated.
    TypeCode Code;
    /// \brief Abbreviation to use for the record, if any.
    unsigned AbbrevToUse;

    ASTTypeWriter(ASTWriter &Writer, ASTWriter::RecordDataImpl &Record)
      : Writer(Writer), Record(Record), Code(TYPE_EXT_QUAL) { }

    void VisitArrayType(const ArrayType *T);
    void VisitFunctionType(const FunctionType *T);
    void VisitTagType(const TagType *T);

#define TYPE(Class, Base) void Visit##Class##Type(const Class##Type *T);
#define ABSTRACT_TYPE(Class, Base)
#include "clang/AST/TypeNodes.def"
  };
}

void ASTTypeWriter::VisitBuiltinType(const BuiltinType *T) {
  llvm_unreachable("Built-in types are never serialized");
}

void ASTTypeWriter::VisitComplexType(const ComplexType *T) {
  Writer.AddTypeRef(T->getElementType(), Record);
  Code = TYPE_COMPLEX;
}

void ASTTypeWriter::VisitPointerType(const PointerType *T) {
  Writer.AddTypeRef(T->getPointeeType(), Record);
  Code = TYPE_POINTER;
}

void ASTTypeWriter::VisitDecayedType(const DecayedType *T) {
  Writer.AddTypeRef(T->getOriginalType(), Record);
  Code = TYPE_DECAYED;
}

void ASTTypeWriter::VisitAdjustedType(const AdjustedType *T) {
  Writer.AddTypeRef(T->getOriginalType(), Record);
  Writer.AddTypeRef(T->getAdjustedType(), Record);
  Code = TYPE_ADJUSTED;
}

void ASTTypeWriter::VisitBlockPointerType(const BlockPointerType *T) {
  Writer.AddTypeRef(T->getPointeeType(), Record);
  Code = TYPE_BLOCK_POINTER;
}

void ASTTypeWriter::VisitLValueReferenceType(const LValueReferenceType *T) {
  Writer.AddTypeRef(T->getPointeeTypeAsWritten(), Record);
  Record.push_back(T->isSpelledAsLValue());
  Code = TYPE_LVALUE_REFERENCE;
}

void ASTTypeWriter::VisitRValueReferenceType(const RValueReferenceType *T) {
  Writer.AddTypeRef(T->getPointeeTypeAsWritten(), Record);
  Code = TYPE_RVALUE_REFERENCE;
}

void ASTTypeWriter::VisitMemberPointerType(const MemberPointerType *T) {
  Writer.AddTypeRef(T->getPointeeType(), Record);
  Writer.AddTypeRef(QualType(T->getClass(), 0), Record);
  Code = TYPE_MEMBER_POINTER;
}

void ASTTypeWriter::VisitArrayType(const ArrayType *T) {
  Writer.AddTypeRef(T->getElementType(), Record);
  Record.push_back(T->getSizeModifier()); // FIXME: stable values
  Record.push_back(T->getIndexTypeCVRQualifiers()); // FIXME: stable values
}

void ASTTypeWriter::VisitConstantArrayType(const ConstantArrayType *T) {
  VisitArrayType(T);
  Writer.AddAPInt(T->getSize(), Record);
  Code = TYPE_CONSTANT_ARRAY;
}

void ASTTypeWriter::VisitIncompleteArrayType(const IncompleteArrayType *T) {
  VisitArrayType(T);
  Code = TYPE_INCOMPLETE_ARRAY;
}

void ASTTypeWriter::VisitVariableArrayType(const VariableArrayType *T) {
  VisitArrayType(T);
  Writer.AddSourceLocation(T->getLBracketLoc(), Record);
  Writer.AddSourceLocation(T->getRBracketLoc(), Record);
  Writer.AddStmt(T->getSizeExpr());
  Code = TYPE_VARIABLE_ARRAY;
}

void ASTTypeWriter::VisitVectorType(const VectorType *T) {
  Writer.AddTypeRef(T->getElementType(), Record);
  Record.push_back(T->getNumElements());
  Record.push_back(T->getVectorKind());
  Code = TYPE_VECTOR;
}

void ASTTypeWriter::VisitExtVectorType(const ExtVectorType *T) {
  VisitVectorType(T);
  Code = TYPE_EXT_VECTOR;
}

void ASTTypeWriter::VisitFunctionType(const FunctionType *T) {
  Writer.AddTypeRef(T->getReturnType(), Record);
  FunctionType::ExtInfo C = T->getExtInfo();
  Record.push_back(C.getNoReturn());
  Record.push_back(C.getHasRegParm());
  Record.push_back(C.getRegParm());
  // FIXME: need to stabilize encoding of calling convention...
  Record.push_back(C.getCC());
  Record.push_back(C.getProducesResult());

  if (C.getHasRegParm() || C.getRegParm() || C.getProducesResult())
    AbbrevToUse = 0;
}

void ASTTypeWriter::VisitFunctionNoProtoType(const FunctionNoProtoType *T) {
  VisitFunctionType(T);
  Code = TYPE_FUNCTION_NO_PROTO;
}

static void addExceptionSpec(ASTWriter &Writer, const FunctionProtoType *T,
                             ASTWriter::RecordDataImpl &Record) {
  Record.push_back(T->getExceptionSpecType());
  if (T->getExceptionSpecType() == EST_Dynamic) {
    Record.push_back(T->getNumExceptions());
    for (unsigned I = 0, N = T->getNumExceptions(); I != N; ++I)
      Writer.AddTypeRef(T->getExceptionType(I), Record);
  } else if (T->getExceptionSpecType() == EST_ComputedNoexcept) {
    Writer.AddStmt(T->getNoexceptExpr());
  } else if (T->getExceptionSpecType() == EST_Uninstantiated) {
    Writer.AddDeclRef(T->getExceptionSpecDecl(), Record);
    Writer.AddDeclRef(T->getExceptionSpecTemplate(), Record);
  } else if (T->getExceptionSpecType() == EST_Unevaluated) {
    Writer.AddDeclRef(T->getExceptionSpecDecl(), Record);
  }
}

void ASTTypeWriter::VisitFunctionProtoType(const FunctionProtoType *T) {
  VisitFunctionType(T);

  Record.push_back(T->isVariadic());
  Record.push_back(T->hasTrailingReturn());
  Record.push_back(T->getTypeQuals());
  Record.push_back(static_cast<unsigned>(T->getRefQualifier()));
  addExceptionSpec(Writer, T, Record);

  Record.push_back(T->getNumParams());
  for (unsigned I = 0, N = T->getNumParams(); I != N; ++I)
    Writer.AddTypeRef(T->getParamType(I), Record);

  if (T->isVariadic() || T->hasTrailingReturn() || T->getTypeQuals() ||
      T->getRefQualifier() || T->getExceptionSpecType() != EST_None)
    AbbrevToUse = 0;

  Code = TYPE_FUNCTION_PROTO;
}

void ASTTypeWriter::VisitUnresolvedUsingType(const UnresolvedUsingType *T) {
  Writer.AddDeclRef(T->getDecl(), Record);
  Code = TYPE_UNRESOLVED_USING;
}

void ASTTypeWriter::VisitTypedefType(const TypedefType *T) {
  Writer.AddDeclRef(T->getDecl(), Record);
  assert(!T->isCanonicalUnqualified() && "Invalid typedef ?");
  Writer.AddTypeRef(T->getCanonicalTypeInternal(), Record);
  Code = TYPE_TYPEDEF;
}

void ASTTypeWriter::VisitTypeOfExprType(const TypeOfExprType *T) {
  Writer.AddStmt(T->getUnderlyingExpr());
  Code = TYPE_TYPEOF_EXPR;
}

void ASTTypeWriter::VisitTypeOfType(const TypeOfType *T) {
  Writer.AddTypeRef(T->getUnderlyingType(), Record);
  Code = TYPE_TYPEOF;
}

void ASTTypeWriter::VisitDecltypeType(const DecltypeType *T) {
  Writer.AddTypeRef(T->getUnderlyingType(), Record);
  Writer.AddStmt(T->getUnderlyingExpr());
  Code = TYPE_DECLTYPE;
}

void ASTTypeWriter::VisitUnaryTransformType(const UnaryTransformType *T) {
  Writer.AddTypeRef(T->getBaseType(), Record);
  Writer.AddTypeRef(T->getUnderlyingType(), Record);
  Record.push_back(T->getUTTKind());
  Code = TYPE_UNARY_TRANSFORM;
}

void ASTTypeWriter::VisitAutoType(const AutoType *T) {
  Writer.AddTypeRef(T->getDeducedType(), Record);
  Record.push_back(T->isDecltypeAuto());
  if (T->getDeducedType().isNull())
    Record.push_back(T->isDependentType());
  Code = TYPE_AUTO;
}

void ASTTypeWriter::VisitTagType(const TagType *T) {
  Record.push_back(T->isDependentType());
  Writer.AddDeclRef(T->getDecl()->getCanonicalDecl(), Record);
  assert(!T->isBeingDefined() &&
         "Cannot serialize in the middle of a type definition");
}

void ASTTypeWriter::VisitRecordType(const RecordType *T) {
  VisitTagType(T);
  Code = TYPE_RECORD;
}

void ASTTypeWriter::VisitEnumType(const EnumType *T) {
  VisitTagType(T);
  Code = TYPE_ENUM;
}

void ASTTypeWriter::VisitAttributedType(const AttributedType *T) {
  Writer.AddTypeRef(T->getModifiedType(), Record);
  Writer.AddTypeRef(T->getEquivalentType(), Record);
  Record.push_back(T->getAttrKind());
  Code = TYPE_ATTRIBUTED;
}

void
ASTTypeWriter::VisitSubstTemplateTypeParmType(
                                        const SubstTemplateTypeParmType *T) {
  Writer.AddTypeRef(QualType(T->getReplacedParameter(), 0), Record);
  Writer.AddTypeRef(T->getReplacementType(), Record);
  Code = TYPE_SUBST_TEMPLATE_TYPE_PARM;
}

void
ASTTypeWriter::VisitSubstTemplateTypeParmPackType(
                                      const SubstTemplateTypeParmPackType *T) {
  Writer.AddTypeRef(QualType(T->getReplacedParameter(), 0), Record);
  Writer.AddTemplateArgument(T->getArgumentPack(), Record);
  Code = TYPE_SUBST_TEMPLATE_TYPE_PARM_PACK;
}

void
ASTTypeWriter::VisitTemplateSpecializationType(
                                       const TemplateSpecializationType *T) {
  Record.push_back(T->isDependentType());
  Writer.AddTemplateName(T->getTemplateName(), Record);
  Record.push_back(T->getNumArgs());
  for (TemplateSpecializationType::iterator ArgI = T->begin(), ArgE = T->end();
         ArgI != ArgE; ++ArgI)
    Writer.AddTemplateArgument(*ArgI, Record);
  Writer.AddTypeRef(T->isTypeAlias() ? T->getAliasedType() :
                    T->isCanonicalUnqualified() ? QualType()
                                                : T->getCanonicalTypeInternal(),
                    Record);
  Code = TYPE_TEMPLATE_SPECIALIZATION;
}

void
ASTTypeWriter::VisitDependentSizedArrayType(const DependentSizedArrayType *T) {
  VisitArrayType(T);
  Writer.AddStmt(T->getSizeExpr());
  Writer.AddSourceRange(T->getBracketsRange(), Record);
  Code = TYPE_DEPENDENT_SIZED_ARRAY;
}

void
ASTTypeWriter::VisitDependentSizedExtVectorType(
                                        const DependentSizedExtVectorType *T) {
  // FIXME: Serialize this type (C++ only)
  llvm_unreachable("Cannot serialize dependent sized extended vector types");
}

void
ASTTypeWriter::VisitTemplateTypeParmType(const TemplateTypeParmType *T) {
  Record.push_back(T->getDepth());
  Record.push_back(T->getIndex());
  Record.push_back(T->isParameterPack());
  Writer.AddDeclRef(T->getDecl(), Record);
  Code = TYPE_TEMPLATE_TYPE_PARM;
}

void
ASTTypeWriter::VisitDependentNameType(const DependentNameType *T) {
  Record.push_back(T->getKeyword());
  Writer.AddNestedNameSpecifier(T->getQualifier(), Record);
  Writer.AddIdentifierRef(T->getIdentifier(), Record);
  Writer.AddTypeRef(T->isCanonicalUnqualified() ? QualType()
                                                : T->getCanonicalTypeInternal(),
                    Record);
  Code = TYPE_DEPENDENT_NAME;
}

void
ASTTypeWriter::VisitDependentTemplateSpecializationType(
                                const DependentTemplateSpecializationType *T) {
  Record.push_back(T->getKeyword());
  Writer.AddNestedNameSpecifier(T->getQualifier(), Record);
  Writer.AddIdentifierRef(T->getIdentifier(), Record);
  Record.push_back(T->getNumArgs());
  for (DependentTemplateSpecializationType::iterator
         I = T->begin(), E = T->end(); I != E; ++I)
    Writer.AddTemplateArgument(*I, Record);
  Code = TYPE_DEPENDENT_TEMPLATE_SPECIALIZATION;
}

void ASTTypeWriter::VisitPackExpansionType(const PackExpansionType *T) {
  Writer.AddTypeRef(T->getPattern(), Record);
  if (Optional<unsigned> NumExpansions = T->getNumExpansions())
    Record.push_back(*NumExpansions + 1);
  else
    Record.push_back(0);
  Code = TYPE_PACK_EXPANSION;
}

void ASTTypeWriter::VisitParenType(const ParenType *T) {
  Writer.AddTypeRef(T->getInnerType(), Record);
  Code = TYPE_PAREN;
}

void ASTTypeWriter::VisitElaboratedType(const ElaboratedType *T) {
  Record.push_back(T->getKeyword());
  Writer.AddNestedNameSpecifier(T->getQualifier(), Record);
  Writer.AddTypeRef(T->getNamedType(), Record);
  Code = TYPE_ELABORATED;
}

void ASTTypeWriter::VisitInjectedClassNameType(const InjectedClassNameType *T) {
  Writer.AddDeclRef(T->getDecl()->getCanonicalDecl(), Record);
  Writer.AddTypeRef(T->getInjectedSpecializationType(), Record);
  Code = TYPE_INJECTED_CLASS_NAME;
}

void ASTTypeWriter::VisitObjCInterfaceType(const ObjCInterfaceType *T) {
  Writer.AddDeclRef(T->getDecl()->getCanonicalDecl(), Record);
  Code = TYPE_OBJC_INTERFACE;
}

void ASTTypeWriter::VisitObjCObjectType(const ObjCObjectType *T) {
  Writer.AddTypeRef(T->getBaseType(), Record);
  Record.push_back(T->getTypeArgsAsWritten().size());
  for (auto TypeArg : T->getTypeArgsAsWritten())
    Writer.AddTypeRef(TypeArg, Record);
  Record.push_back(T->getNumProtocols());
  for (const auto *I : T->quals())
    Writer.AddDeclRef(I, Record);
  Record.push_back(T->isKindOfTypeAsWritten());
  Code = TYPE_OBJC_OBJECT;
}

void
ASTTypeWriter::VisitObjCObjectPointerType(const ObjCObjectPointerType *T) {
  Writer.AddTypeRef(T->getPointeeType(), Record);
  Code = TYPE_OBJC_OBJECT_POINTER;
}

void
ASTTypeWriter::VisitAtomicType(const AtomicType *T) {
  Writer.AddTypeRef(T->getValueType(), Record);
  Code = TYPE_ATOMIC;
}

namespace {

class TypeLocWriter : public TypeLocVisitor<TypeLocWriter> {
  ASTWriter &Writer;
  ASTWriter::RecordDataImpl &Record;

public:
  TypeLocWriter(ASTWriter &Writer, ASTWriter::RecordDataImpl &Record)
    : Writer(Writer), Record(Record) { }

#define ABSTRACT_TYPELOC(CLASS, PARENT)
#define TYPELOC(CLASS, PARENT) \
    void Visit##CLASS##TypeLoc(CLASS##TypeLoc TyLoc);
#include "clang/AST/TypeLocNodes.def"

  void VisitArrayTypeLoc(ArrayTypeLoc TyLoc);
  void VisitFunctionTypeLoc(FunctionTypeLoc TyLoc);
};

}

void TypeLocWriter::VisitQualifiedTypeLoc(QualifiedTypeLoc TL) {
  // nothing to do
}
void TypeLocWriter::VisitBuiltinTypeLoc(BuiltinTypeLoc TL) {
  Writer.AddSourceLocation(TL.getBuiltinLoc(), Record);
  if (TL.needsExtraLocalData()) {
    Record.push_back(TL.getWrittenTypeSpec());
    Record.push_back(TL.getWrittenSignSpec());
    Record.push_back(TL.getWrittenWidthSpec());
    Record.push_back(TL.hasModeAttr());
  }
}
void TypeLocWriter::VisitComplexTypeLoc(ComplexTypeLoc TL) {
  Writer.AddSourceLocation(TL.getNameLoc(), Record);
}
void TypeLocWriter::VisitPointerTypeLoc(PointerTypeLoc TL) {
  Writer.AddSourceLocation(TL.getStarLoc(), Record);
}
void TypeLocWriter::VisitDecayedTypeLoc(DecayedTypeLoc TL) {
  // nothing to do
}
void TypeLocWriter::VisitAdjustedTypeLoc(AdjustedTypeLoc TL) {
  // nothing to do
}
void TypeLocWriter::VisitBlockPointerTypeLoc(BlockPointerTypeLoc TL) {
  Writer.AddSourceLocation(TL.getCaretLoc(), Record);
}
void TypeLocWriter::VisitLValueReferenceTypeLoc(LValueReferenceTypeLoc TL) {
  Writer.AddSourceLocation(TL.getAmpLoc(), Record);
}
void TypeLocWriter::VisitRValueReferenceTypeLoc(RValueReferenceTypeLoc TL) {
  Writer.AddSourceLocation(TL.getAmpAmpLoc(), Record);
}
void TypeLocWriter::VisitMemberPointerTypeLoc(MemberPointerTypeLoc TL) {
  Writer.AddSourceLocation(TL.getStarLoc(), Record);
  Writer.AddTypeSourceInfo(TL.getClassTInfo(), Record);
}
void TypeLocWriter::VisitArrayTypeLoc(ArrayTypeLoc TL) {
  Writer.AddSourceLocation(TL.getLBracketLoc(), Record);
  Writer.AddSourceLocation(TL.getRBracketLoc(), Record);
  Record.push_back(TL.getSizeExpr() ? 1 : 0);
  if (TL.getSizeExpr())
    Writer.AddStmt(TL.getSizeExpr());
}
void TypeLocWriter::VisitConstantArrayTypeLoc(ConstantArrayTypeLoc TL) {
  VisitArrayTypeLoc(TL);
}
void TypeLocWriter::VisitIncompleteArrayTypeLoc(IncompleteArrayTypeLoc TL) {
  VisitArrayTypeLoc(TL);
}
void TypeLocWriter::VisitVariableArrayTypeLoc(VariableArrayTypeLoc TL) {
  VisitArrayTypeLoc(TL);
}
void TypeLocWriter::VisitDependentSizedArrayTypeLoc(
                                            DependentSizedArrayTypeLoc TL) {
  VisitArrayTypeLoc(TL);
}
void TypeLocWriter::VisitDependentSizedExtVectorTypeLoc(
                                        DependentSizedExtVectorTypeLoc TL) {
  Writer.AddSourceLocation(TL.getNameLoc(), Record);
}
void TypeLocWriter::VisitVectorTypeLoc(VectorTypeLoc TL) {
  Writer.AddSourceLocation(TL.getNameLoc(), Record);
}
void TypeLocWriter::VisitExtVectorTypeLoc(ExtVectorTypeLoc TL) {
  Writer.AddSourceLocation(TL.getNameLoc(), Record);
}
void TypeLocWriter::VisitFunctionTypeLoc(FunctionTypeLoc TL) {
  Writer.AddSourceLocation(TL.getLocalRangeBegin(), Record);
  Writer.AddSourceLocation(TL.getLParenLoc(), Record);
  Writer.AddSourceLocation(TL.getRParenLoc(), Record);
  Writer.AddSourceLocation(TL.getLocalRangeEnd(), Record);
  for (unsigned i = 0, e = TL.getNumParams(); i != e; ++i)
    Writer.AddDeclRef(TL.getParam(i), Record);
}
void TypeLocWriter::VisitFunctionProtoTypeLoc(FunctionProtoTypeLoc TL) {
  VisitFunctionTypeLoc(TL);
}
void TypeLocWriter::VisitFunctionNoProtoTypeLoc(FunctionNoProtoTypeLoc TL) {
  VisitFunctionTypeLoc(TL);
}
void TypeLocWriter::VisitUnresolvedUsingTypeLoc(UnresolvedUsingTypeLoc TL) {
  Writer.AddSourceLocation(TL.getNameLoc(), Record);
}
void TypeLocWriter::VisitTypedefTypeLoc(TypedefTypeLoc TL) {
  Writer.AddSourceLocation(TL.getNameLoc(), Record);
}
void TypeLocWriter::VisitTypeOfExprTypeLoc(TypeOfExprTypeLoc TL) {
  Writer.AddSourceLocation(TL.getTypeofLoc(), Record);
  Writer.AddSourceLocation(TL.getLParenLoc(), Record);
  Writer.AddSourceLocation(TL.getRParenLoc(), Record);
}
void TypeLocWriter::VisitTypeOfTypeLoc(TypeOfTypeLoc TL) {
  Writer.AddSourceLocation(TL.getTypeofLoc(), Record);
  Writer.AddSourceLocation(TL.getLParenLoc(), Record);
  Writer.AddSourceLocation(TL.getRParenLoc(), Record);
  Writer.AddTypeSourceInfo(TL.getUnderlyingTInfo(), Record);
}
void TypeLocWriter::VisitDecltypeTypeLoc(DecltypeTypeLoc TL) {
  Writer.AddSourceLocation(TL.getNameLoc(), Record);
}
void TypeLocWriter::VisitUnaryTransformTypeLoc(UnaryTransformTypeLoc TL) {
  Writer.AddSourceLocation(TL.getKWLoc(), Record);
  Writer.AddSourceLocation(TL.getLParenLoc(), Record);
  Writer.AddSourceLocation(TL.getRParenLoc(), Record);
  Writer.AddTypeSourceInfo(TL.getUnderlyingTInfo(), Record);
}
void TypeLocWriter::VisitAutoTypeLoc(AutoTypeLoc TL) {
  Writer.AddSourceLocation(TL.getNameLoc(), Record);
}
void TypeLocWriter::VisitRecordTypeLoc(RecordTypeLoc TL) {
  Writer.AddSourceLocation(TL.getNameLoc(), Record);
}
void TypeLocWriter::VisitEnumTypeLoc(EnumTypeLoc TL) {
  Writer.AddSourceLocation(TL.getNameLoc(), Record);
}
void TypeLocWriter::VisitAttributedTypeLoc(AttributedTypeLoc TL) {
  Writer.AddSourceLocation(TL.getAttrNameLoc(), Record);
  if (TL.hasAttrOperand()) {
    SourceRange range = TL.getAttrOperandParensRange();
    Writer.AddSourceLocation(range.getBegin(), Record);
    Writer.AddSourceLocation(range.getEnd(), Record);
  }
  if (TL.hasAttrExprOperand()) {
    Expr *operand = TL.getAttrExprOperand();
    Record.push_back(operand ? 1 : 0);
    if (operand) Writer.AddStmt(operand);
  } else if (TL.hasAttrEnumOperand()) {
    Writer.AddSourceLocation(TL.getAttrEnumOperandLoc(), Record);
  }
}
void TypeLocWriter::VisitTemplateTypeParmTypeLoc(TemplateTypeParmTypeLoc TL) {
  Writer.AddSourceLocation(TL.getNameLoc(), Record);
}
void TypeLocWriter::VisitSubstTemplateTypeParmTypeLoc(
                                            SubstTemplateTypeParmTypeLoc TL) {
  Writer.AddSourceLocation(TL.getNameLoc(), Record);
}
void TypeLocWriter::VisitSubstTemplateTypeParmPackTypeLoc(
                                          SubstTemplateTypeParmPackTypeLoc TL) {
  Writer.AddSourceLocation(TL.getNameLoc(), Record);
}
void TypeLocWriter::VisitTemplateSpecializationTypeLoc(
                                           TemplateSpecializationTypeLoc TL) {
  Writer.AddSourceLocation(TL.getTemplateKeywordLoc(), Record);
  Writer.AddSourceLocation(TL.getTemplateNameLoc(), Record);
  Writer.AddSourceLocation(TL.getLAngleLoc(), Record);
  Writer.AddSourceLocation(TL.getRAngleLoc(), Record);
  for (unsigned i = 0, e = TL.getNumArgs(); i != e; ++i)
    Writer.AddTemplateArgumentLocInfo(TL.getArgLoc(i).getArgument().getKind(),
                                      TL.getArgLoc(i).getLocInfo(), Record);
}
void TypeLocWriter::VisitParenTypeLoc(ParenTypeLoc TL) {
  Writer.AddSourceLocation(TL.getLParenLoc(), Record);
  Writer.AddSourceLocation(TL.getRParenLoc(), Record);
}
void TypeLocWriter::VisitElaboratedTypeLoc(ElaboratedTypeLoc TL) {
  Writer.AddSourceLocation(TL.getElaboratedKeywordLoc(), Record);
  Writer.AddNestedNameSpecifierLoc(TL.getQualifierLoc(), Record);
}
void TypeLocWriter::VisitInjectedClassNameTypeLoc(InjectedClassNameTypeLoc TL) {
  Writer.AddSourceLocation(TL.getNameLoc(), Record);
}
void TypeLocWriter::VisitDependentNameTypeLoc(DependentNameTypeLoc TL) {
  Writer.AddSourceLocation(TL.getElaboratedKeywordLoc(), Record);
  Writer.AddNestedNameSpecifierLoc(TL.getQualifierLoc(), Record);
  Writer.AddSourceLocation(TL.getNameLoc(), Record);
}
void TypeLocWriter::VisitDependentTemplateSpecializationTypeLoc(
       DependentTemplateSpecializationTypeLoc TL) {
  Writer.AddSourceLocation(TL.getElaboratedKeywordLoc(), Record);
  Writer.AddNestedNameSpecifierLoc(TL.getQualifierLoc(), Record);
  Writer.AddSourceLocation(TL.getTemplateKeywordLoc(), Record);
  Writer.AddSourceLocation(TL.getTemplateNameLoc(), Record);
  Writer.AddSourceLocation(TL.getLAngleLoc(), Record);
  Writer.AddSourceLocation(TL.getRAngleLoc(), Record);
  for (unsigned I = 0, E = TL.getNumArgs(); I != E; ++I)
    Writer.AddTemplateArgumentLocInfo(TL.getArgLoc(I).getArgument().getKind(),
                                      TL.getArgLoc(I).getLocInfo(), Record);
}
void TypeLocWriter::VisitPackExpansionTypeLoc(PackExpansionTypeLoc TL) {
  Writer.AddSourceLocation(TL.getEllipsisLoc(), Record);
}
void TypeLocWriter::VisitObjCInterfaceTypeLoc(ObjCInterfaceTypeLoc TL) {
  Writer.AddSourceLocation(TL.getNameLoc(), Record);
}
void TypeLocWriter::VisitObjCObjectTypeLoc(ObjCObjectTypeLoc TL) {
  Record.push_back(TL.hasBaseTypeAsWritten());
  Writer.AddSourceLocation(TL.getTypeArgsLAngleLoc(), Record);
  Writer.AddSourceLocation(TL.getTypeArgsRAngleLoc(), Record);
  for (unsigned i = 0, e = TL.getNumTypeArgs(); i != e; ++i)
    Writer.AddTypeSourceInfo(TL.getTypeArgTInfo(i), Record);
  Writer.AddSourceLocation(TL.getProtocolLAngleLoc(), Record);
  Writer.AddSourceLocation(TL.getProtocolRAngleLoc(), Record);
  for (unsigned i = 0, e = TL.getNumProtocols(); i != e; ++i)
    Writer.AddSourceLocation(TL.getProtocolLoc(i), Record);
}
void TypeLocWriter::VisitObjCObjectPointerTypeLoc(ObjCObjectPointerTypeLoc TL) {
  Writer.AddSourceLocation(TL.getStarLoc(), Record);
}
void TypeLocWriter::VisitAtomicTypeLoc(AtomicTypeLoc TL) {
  Writer.AddSourceLocation(TL.getKWLoc(), Record);
  Writer.AddSourceLocation(TL.getLParenLoc(), Record);
  Writer.AddSourceLocation(TL.getRParenLoc(), Record);
}

void ASTWriter::WriteTypeAbbrevs() {
  using namespace llvm;

  BitCodeAbbrev *Abv;

  // Abbreviation for TYPE_EXT_QUAL
  Abv = new BitCodeAbbrev();
  Abv->Add(BitCodeAbbrevOp(serialization::TYPE_EXT_QUAL));
  Abv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6));   // Type
  Abv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 3));   // Quals
  TypeExtQualAbbrev = Stream.EmitAbbrev(Abv);

  // Abbreviation for TYPE_FUNCTION_PROTO
  Abv = new BitCodeAbbrev();
  Abv->Add(BitCodeAbbrevOp(serialization::TYPE_FUNCTION_PROTO));
  // FunctionType
  Abv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6));   // ReturnType
  Abv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1)); // NoReturn
  Abv->Add(BitCodeAbbrevOp(0));                         // HasRegParm
  Abv->Add(BitCodeAbbrevOp(0));                         // RegParm
  Abv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 4)); // CC
  Abv->Add(BitCodeAbbrevOp(0));                         // ProducesResult
  // FunctionProtoType
  Abv->Add(BitCodeAbbrevOp(0));                         // IsVariadic
  Abv->Add(BitCodeAbbrevOp(0));                         // HasTrailingReturn
  Abv->Add(BitCodeAbbrevOp(0));                         // TypeQuals
  Abv->Add(BitCodeAbbrevOp(0));                         // RefQualifier
  Abv->Add(BitCodeAbbrevOp(EST_None));                  // ExceptionSpec
  Abv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6));   // NumParams
  Abv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Array));
  Abv->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6));   // Params
  TypeFunctionProtoAbbrev = Stream.EmitAbbrev(Abv);
}

//===----------------------------------------------------------------------===//
// ASTWriter Implementation
//===----------------------------------------------------------------------===//

static void EmitBlockID(unsigned ID, const char *Name,
                        llvm::BitstreamWriter &Stream,
                        ASTWriter::RecordDataImpl &Record) {
  Record.clear();
  Record.push_back(ID);
  Stream.EmitRecord(llvm::bitc::BLOCKINFO_CODE_SETBID, Record);

  // Emit the block name if present.
  if (!Name || Name[0] == 0)
    return;
  Record.clear();
  while (*Name)
    Record.push_back(*Name++);
  Stream.EmitRecord(llvm::bitc::BLOCKINFO_CODE_BLOCKNAME, Record);
}

static void EmitRecordID(unsigned ID, const char *Name,
                         llvm::BitstreamWriter &Stream,
                         ASTWriter::RecordDataImpl &Record) {
  Record.clear();
  Record.push_back(ID);
  while (*Name)
    Record.push_back(*Name++);
  Stream.EmitRecord(llvm::bitc::BLOCKINFO_CODE_SETRECORDNAME, Record);
}

static void AddStmtsExprs(llvm::BitstreamWriter &Stream,
                          ASTWriter::RecordDataImpl &Record) {
#define RECORD(X) EmitRecordID(X, #X, Stream, Record)
  RECORD(STMT_STOP);
  RECORD(STMT_NULL_PTR);
  RECORD(STMT_REF_PTR);
  RECORD(STMT_NULL);
  RECORD(STMT_COMPOUND);
  RECORD(STMT_CASE);
  RECORD(STMT_DEFAULT);
  RECORD(STMT_LABEL);
  RECORD(STMT_ATTRIBUTED);
  RECORD(STMT_IF);
  RECORD(STMT_SWITCH);
  RECORD(STMT_WHILE);
  RECORD(STMT_DO);
  RECORD(STMT_FOR);
  RECORD(STMT_GOTO);
  RECORD(STMT_INDIRECT_GOTO);
  RECORD(STMT_CONTINUE);
  RECORD(STMT_BREAK);
  RECORD(STMT_RETURN);
  RECORD(STMT_DECL);
  RECORD(STMT_GCCASM);
  RECORD(STMT_MSASM);
  RECORD(EXPR_PREDEFINED);
  RECORD(EXPR_DECL_REF);
  RECORD(EXPR_INTEGER_LITERAL);
  RECORD(EXPR_FLOATING_LITERAL);
  RECORD(EXPR_IMAGINARY_LITERAL);
  RECORD(EXPR_STRING_LITERAL);
  RECORD(EXPR_CHARACTER_LITERAL);
  RECORD(EXPR_PAREN);
  RECORD(EXPR_PAREN_LIST);
  RECORD(EXPR_UNARY_OPERATOR);
  RECORD(EXPR_SIZEOF_ALIGN_OF);
  RECORD(EXPR_ARRAY_SUBSCRIPT);
  RECORD(EXPR_CALL);
  RECORD(EXPR_MEMBER);
  RECORD(EXPR_BINARY_OPERATOR);
  RECORD(EXPR_COMPOUND_ASSIGN_OPERATOR);
  RECORD(EXPR_CONDITIONAL_OPERATOR);
  RECORD(EXPR_IMPLICIT_CAST);
  RECORD(EXPR_CSTYLE_CAST);
  RECORD(EXPR_COMPOUND_LITERAL);
  RECORD(EXPR_EXT_VECTOR_ELEMENT);
  RECORD(EXPR_INIT_LIST);
  RECORD(EXPR_DESIGNATED_INIT);
  RECORD(EXPR_DESIGNATED_INIT_UPDATE);
  RECORD(EXPR_IMPLICIT_VALUE_INIT);
  RECORD(EXPR_NO_INIT);
  RECORD(EXPR_VA_ARG);
  RECORD(EXPR_ADDR_LABEL);
  RECORD(EXPR_STMT);
  RECORD(EXPR_CHOOSE);
  RECORD(EXPR_GNU_NULL);
  RECORD(EXPR_SHUFFLE_VECTOR);
  RECORD(EXPR_BLOCK);
  RECORD(EXPR_GENERIC_SELECTION);
  RECORD(EXPR_OBJC_STRING_LITERAL);
  RECORD(EXPR_OBJC_BOXED_EXPRESSION);
  RECORD(EXPR_OBJC_ARRAY_LITERAL);
  RECORD(EXPR_OBJC_DICTIONARY_LITERAL);
  RECORD(EXPR_OBJC_ENCODE);
  RECORD(EXPR_OBJC_SELECTOR_EXPR);
  RECORD(EXPR_OBJC_PROTOCOL_EXPR);
  RECORD(EXPR_OBJC_IVAR_REF_EXPR);
  RECORD(EXPR_OBJC_PROPERTY_REF_EXPR);
  RECORD(EXPR_OBJC_KVC_REF_EXPR);
  RECORD(EXPR_OBJC_MESSAGE_EXPR);
  RECORD(STMT_OBJC_FOR_COLLECTION);
  RECORD(STMT_OBJC_CATCH);
  RECORD(STMT_OBJC_FINALLY);
  RECORD(STMT_OBJC_AT_TRY);
  RECORD(STMT_OBJC_AT_SYNCHRONIZED);
  RECORD(STMT_OBJC_AT_THROW);
  RECORD(EXPR_OBJC_BOOL_LITERAL);
  RECORD(STMT_CXX_CATCH);
  RECORD(STMT_CXX_TRY);
  RECORD(STMT_CXX_FOR_RANGE);
  RECORD(EXPR_CXX_OPERATOR_CALL);
  RECORD(EXPR_CXX_MEMBER_CALL);
  RECORD(EXPR_CXX_CONSTRUCT);
  RECORD(EXPR_CXX_TEMPORARY_OBJECT);
  RECORD(EXPR_CXX_STATIC_CAST);
  RECORD(EXPR_CXX_DYNAMIC_CAST);
  RECORD(EXPR_CXX_REINTERPRET_CAST);
  RECORD(EXPR_CXX_CONST_CAST);
  RECORD(EXPR_CXX_FUNCTIONAL_CAST);
  RECORD(EXPR_USER_DEFINED_LITERAL);
  RECORD(EXPR_CXX_STD_INITIALIZER_LIST);
  RECORD(EXPR_CXX_BOOL_LITERAL);
  RECORD(EXPR_CXX_NULL_PTR_LITERAL);
  RECORD(EXPR_CXX_TYPEID_EXPR);
  RECORD(EXPR_CXX_TYPEID_TYPE);
  RECORD(EXPR_CXX_THIS);
  RECORD(EXPR_CXX_THROW);
  RECORD(EXPR_CXX_DEFAULT_ARG);
  RECORD(EXPR_CXX_DEFAULT_INIT);
  RECORD(EXPR_CXX_BIND_TEMPORARY);
  RECORD(EXPR_CXX_SCALAR_VALUE_INIT);
  RECORD(EXPR_CXX_NEW);
  RECORD(EXPR_CXX_DELETE);
  RECORD(EXPR_CXX_PSEUDO_DESTRUCTOR);
  RECORD(EXPR_EXPR_WITH_CLEANUPS);
  RECORD(EXPR_CXX_DEPENDENT_SCOPE_MEMBER);
  RECORD(EXPR_CXX_DEPENDENT_SCOPE_DECL_REF);
  RECORD(EXPR_CXX_UNRESOLVED_CONSTRUCT);
  RECORD(EXPR_CXX_UNRESOLVED_MEMBER);
  RECORD(EXPR_CXX_UNRESOLVED_LOOKUP);
  RECORD(EXPR_CXX_EXPRESSION_TRAIT);
  RECORD(EXPR_CXX_NOEXCEPT);
  RECORD(EXPR_OPAQUE_VALUE);
  RECORD(EXPR_BINARY_CONDITIONAL_OPERATOR);
  RECORD(EXPR_TYPE_TRAIT);
  RECORD(EXPR_ARRAY_TYPE_TRAIT);
  RECORD(EXPR_PACK_EXPANSION);
  RECORD(EXPR_SIZEOF_PACK);
  RECORD(EXPR_SUBST_NON_TYPE_TEMPLATE_PARM);
  RECORD(EXPR_SUBST_NON_TYPE_TEMPLATE_PARM_PACK);
  RECORD(EXPR_FUNCTION_PARM_PACK);
  RECORD(EXPR_MATERIALIZE_TEMPORARY);
  RECORD(EXPR_CUDA_KERNEL_CALL);
  RECORD(EXPR_CXX_UUIDOF_EXPR);
  RECORD(EXPR_CXX_UUIDOF_TYPE);
  RECORD(EXPR_LAMBDA);
#undef RECORD
}

void ASTWriter::WriteBlockInfoBlock() {
  RecordData Record;
  Stream.EnterSubblock(llvm::bitc::BLOCKINFO_BLOCK_ID, 3);

#define BLOCK(X) EmitBlockID(X ## _ID, #X, Stream, Record)
#define RECORD(X) EmitRecordID(X, #X, Stream, Record)

  // Control Block.
  BLOCK(CONTROL_BLOCK);
  RECORD(METADATA);
  RECORD(SIGNATURE);
  RECORD(MODULE_NAME);
  RECORD(MODULE_MAP_FILE);
  RECORD(IMPORTS);
  RECORD(KNOWN_MODULE_FILES);
  RECORD(LANGUAGE_OPTIONS);
  RECORD(TARGET_OPTIONS);
  RECORD(ORIGINAL_FILE);
  RECORD(ORIGINAL_PCH_DIR);
  RECORD(ORIGINAL_FILE_ID);
  RECORD(INPUT_FILE_OFFSETS);
  RECORD(DIAGNOSTIC_OPTIONS);
  RECORD(FILE_SYSTEM_OPTIONS);
  RECORD(HEADER_SEARCH_OPTIONS);
  RECORD(PREPROCESSOR_OPTIONS);

  BLOCK(INPUT_FILES_BLOCK);
  RECORD(INPUT_FILE);

  // AST Top-Level Block.
  BLOCK(AST_BLOCK);
  RECORD(TYPE_OFFSET);
  RECORD(DECL_OFFSET);
  RECORD(IDENTIFIER_OFFSET);
  RECORD(IDENTIFIER_TABLE);
  RECORD(EAGERLY_DESERIALIZED_DECLS);
  RECORD(SPECIAL_TYPES);
  RECORD(STATISTICS);
  RECORD(TENTATIVE_DEFINITIONS);
  RECORD(UNUSED_FILESCOPED_DECLS);
  RECORD(SELECTOR_OFFSETS);
  RECORD(METHOD_POOL);
  RECORD(PP_COUNTER_VALUE);
  RECORD(SOURCE_LOCATION_OFFSETS);
  RECORD(SOURCE_LOCATION_PRELOADS);
  RECORD(EXT_VECTOR_DECLS);
  RECORD(PPD_ENTITIES_OFFSETS);
  RECORD(REFERENCED_SELECTOR_POOL);
  RECORD(TU_UPDATE_LEXICAL);
  RECORD(LOCAL_REDECLARATIONS_MAP);
  RECORD(SEMA_DECL_REFS);
  RECORD(WEAK_UNDECLARED_IDENTIFIERS);
  RECORD(PENDING_IMPLICIT_INSTANTIATIONS);
  RECORD(DECL_REPLACEMENTS);
  RECORD(UPDATE_VISIBLE);
  RECORD(DECL_UPDATE_OFFSETS);
  RECORD(DECL_UPDATES);
  RECORD(CXX_BASE_SPECIFIER_OFFSETS);
  RECORD(DIAG_PRAGMA_MAPPINGS);
  RECORD(CUDA_SPECIAL_DECL_REFS);
  RECORD(HEADER_SEARCH_TABLE);
  RECORD(FP_PRAGMA_OPTIONS);
  RECORD(OPENCL_EXTENSIONS);
  RECORD(DELEGATING_CTORS);
  RECORD(KNOWN_NAMESPACES);
  RECORD(UNDEFINED_BUT_USED);
  RECORD(MODULE_OFFSET_MAP);
  RECORD(SOURCE_MANAGER_LINE_TABLE);
  RECORD(OBJC_CATEGORIES_MAP);
  RECORD(FILE_SORTED_DECLS);
  RECORD(IMPORTED_MODULES);
  RECORD(LOCAL_REDECLARATIONS);
  RECORD(OBJC_CATEGORIES);
  RECORD(MACRO_OFFSET);
  RECORD(LATE_PARSED_TEMPLATE);
  RECORD(OPTIMIZE_PRAGMA_OPTIONS);

  // SourceManager Block.
  BLOCK(SOURCE_MANAGER_BLOCK);
  RECORD(SM_SLOC_FILE_ENTRY);
  RECORD(SM_SLOC_BUFFER_ENTRY);
  RECORD(SM_SLOC_BUFFER_BLOB);
  RECORD(SM_SLOC_EXPANSION_ENTRY);

  // Preprocessor Block.
  BLOCK(PREPROCESSOR_BLOCK);
  RECORD(PP_MACRO_DIRECTIVE_HISTORY);
  RECORD(PP_MACRO_FUNCTION_LIKE);
  RECORD(PP_MACRO_OBJECT_LIKE);
  RECORD(PP_MODULE_MACRO);
  RECORD(PP_TOKEN);

  // Decls and Types block.
  BLOCK(DECLTYPES_BLOCK);
  RECORD(TYPE_EXT_QUAL);
  RECORD(TYPE_COMPLEX);
  RECORD(TYPE_POINTER);
  RECORD(TYPE_BLOCK_POINTER);
  RECORD(TYPE_LVALUE_REFERENCE);
  RECORD(TYPE_RVALUE_REFERENCE);
  RECORD(TYPE_MEMBER_POINTER);
  RECORD(TYPE_CONSTANT_ARRAY);
  RECORD(TYPE_INCOMPLETE_ARRAY);
  RECORD(TYPE_VARIABLE_ARRAY);
  RECORD(TYPE_VECTOR);
  RECORD(TYPE_EXT_VECTOR);
  RECORD(TYPE_FUNCTION_NO_PROTO);
  RECORD(TYPE_FUNCTION_PROTO);
  RECORD(TYPE_TYPEDEF);
  RECORD(TYPE_TYPEOF_EXPR);
  RECORD(TYPE_TYPEOF);
  RECORD(TYPE_RECORD);
  RECORD(TYPE_ENUM);
  RECORD(TYPE_OBJC_INTERFACE);
  RECORD(TYPE_OBJC_OBJECT_POINTER);
  RECORD(TYPE_DECLTYPE);
  RECORD(TYPE_ELABORATED);
  RECORD(TYPE_SUBST_TEMPLATE_TYPE_PARM);
  RECORD(TYPE_UNRESOLVED_USING);
  RECORD(TYPE_INJECTED_CLASS_NAME);
  RECORD(TYPE_OBJC_OBJECT);
  RECORD(TYPE_TEMPLATE_TYPE_PARM);
  RECORD(TYPE_TEMPLATE_SPECIALIZATION);
  RECORD(TYPE_DEPENDENT_NAME);
  RECORD(TYPE_DEPENDENT_TEMPLATE_SPECIALIZATION);
  RECORD(TYPE_DEPENDENT_SIZED_ARRAY);
  RECORD(TYPE_PAREN);
  RECORD(TYPE_PACK_EXPANSION);
  RECORD(TYPE_ATTRIBUTED);
  RECORD(TYPE_SUBST_TEMPLATE_TYPE_PARM_PACK);
  RECORD(TYPE_AUTO);
  RECORD(TYPE_UNARY_TRANSFORM);
  RECORD(TYPE_ATOMIC);
  RECORD(TYPE_DECAYED);
  RECORD(TYPE_ADJUSTED);
  RECORD(DECL_TYPEDEF);
  RECORD(DECL_TYPEALIAS);
  RECORD(DECL_ENUM);
  RECORD(DECL_RECORD);
  RECORD(DECL_ENUM_CONSTANT);
  RECORD(DECL_FUNCTION);
  RECORD(DECL_OBJC_METHOD);
  RECORD(DECL_OBJC_INTERFACE);
  RECORD(DECL_OBJC_PROTOCOL);
  RECORD(DECL_OBJC_IVAR);
  RECORD(DECL_OBJC_AT_DEFS_FIELD);
  RECORD(DECL_OBJC_CATEGORY);
  RECORD(DECL_OBJC_CATEGORY_IMPL);
  RECORD(DECL_OBJC_IMPLEMENTATION);
  RECORD(DECL_OBJC_COMPATIBLE_ALIAS);
  RECORD(DECL_OBJC_PROPERTY);
  RECORD(DECL_OBJC_PROPERTY_IMPL);
  RECORD(DECL_FIELD);
  RECORD(DECL_MS_PROPERTY);
  RECORD(DECL_VAR);
  RECORD(DECL_IMPLICIT_PARAM);
  RECORD(DECL_PARM_VAR);
  RECORD(DECL_FILE_SCOPE_ASM);
  RECORD(DECL_BLOCK);
  RECORD(DECL_CONTEXT_LEXICAL);
  RECORD(DECL_CONTEXT_VISIBLE);
  RECORD(DECL_NAMESPACE);
  RECORD(DECL_NAMESPACE_ALIAS);
  RECORD(DECL_USING);
  RECORD(DECL_USING_SHADOW);
  RECORD(DECL_USING_DIRECTIVE);
  RECORD(DECL_UNRESOLVED_USING_VALUE);
  RECORD(DECL_UNRESOLVED_USING_TYPENAME);
  RECORD(DECL_LINKAGE_SPEC);
  RECORD(DECL_CXX_RECORD);
  RECORD(DECL_CXX_METHOD);
  RECORD(DECL_CXX_CONSTRUCTOR);
  RECORD(DECL_CXX_DESTRUCTOR);
  RECORD(DECL_CXX_CONVERSION);
  RECORD(DECL_ACCESS_SPEC);
  RECORD(DECL_FRIEND);
  RECORD(DECL_FRIEND_TEMPLATE);
  RECORD(DECL_CLASS_TEMPLATE);
  RECORD(DECL_CLASS_TEMPLATE_SPECIALIZATION);
  RECORD(DECL_CLASS_TEMPLATE_PARTIAL_SPECIALIZATION);
  RECORD(DECL_VAR_TEMPLATE);
  RECORD(DECL_VAR_TEMPLATE_SPECIALIZATION);
  RECORD(DECL_VAR_TEMPLATE_PARTIAL_SPECIALIZATION);
  RECORD(DECL_FUNCTION_TEMPLATE);
  RECORD(DECL_TEMPLATE_TYPE_PARM);
  RECORD(DECL_NON_TYPE_TEMPLATE_PARM);
  RECORD(DECL_TEMPLATE_TEMPLATE_PARM);
  RECORD(DECL_STATIC_ASSERT);
  RECORD(DECL_CXX_BASE_SPECIFIERS);
  RECORD(DECL_INDIRECTFIELD);
  RECORD(DECL_EXPANDED_NON_TYPE_TEMPLATE_PARM_PACK);
  
  // Statements and Exprs can occur in the Decls and Types block.
  AddStmtsExprs(Stream, Record);

  BLOCK(PREPROCESSOR_DETAIL_BLOCK);
  RECORD(PPD_MACRO_EXPANSION);
  RECORD(PPD_MACRO_DEFINITION);
  RECORD(PPD_INCLUSION_DIRECTIVE);
  
#undef RECORD
#undef BLOCK
  Stream.ExitBlock();
}

/// \brief Prepares a path for being written to an AST file by converting it
/// to an absolute path and removing nested './'s.
///
/// \return \c true if the path was changed.
static bool cleanPathForOutput(FileManager &FileMgr,
                               SmallVectorImpl<char> &Path) {
  bool Changed = false;

  if (!llvm::sys::path::is_absolute(StringRef(Path.data(), Path.size()))) {
    llvm::sys::fs::make_absolute(Path);
    Changed = true;
  }

  return Changed | FileMgr.removeDotPaths(Path);
}

/// \brief Adjusts the given filename to only write out the portion of the
/// filename that is not part of the system root directory.
///
/// \param Filename the file name to adjust.
///
/// \param BaseDir When non-NULL, the PCH file is a relocatable AST file and
/// the returned filename will be adjusted by this root directory.
///
/// \returns either the original filename (if it needs no adjustment) or the
/// adjusted filename (which points into the @p Filename parameter).
static const char *
adjustFilenameForRelocatableAST(const char *Filename, StringRef BaseDir) {
  assert(Filename && "No file name to adjust?");

  if (BaseDir.empty())
    return Filename;

  // Verify that the filename and the system root have the same prefix.
  unsigned Pos = 0;
  for (; Filename[Pos] && Pos < BaseDir.size(); ++Pos)
    if (Filename[Pos] != BaseDir[Pos])
      return Filename; // Prefixes don't match.

  // We hit the end of the filename before we hit the end of the system root.
  if (!Filename[Pos])
    return Filename;

  // If there's not a path separator at the end of the base directory nor
  // immediately after it, then this isn't within the base directory.
  if (!llvm::sys::path::is_separator(Filename[Pos])) {
    if (!llvm::sys::path::is_separator(BaseDir.back()))
      return Filename;
  } else {
    // If the file name has a '/' at the current position, skip over the '/'.
    // We distinguish relative paths from absolute paths by the
    // absence of '/' at the beginning of relative paths.
    //
    // FIXME: This is wrong. We distinguish them by asking if the path is
    // absolute, which isn't the same thing. And there might be multiple '/'s
    // in a row. Use a better mechanism to indicate whether we have emitted an
    // absolute or relative path.
    ++Pos;
  }

  return Filename + Pos;
}

static ASTFileSignature getSignature() {
  while (1) {
    if (ASTFileSignature S = llvm::sys::Process::GetRandomNumber())
      return S;
    // Rely on GetRandomNumber to eventually return non-zero...
  }
}

/// \brief Write the control block.
void ASTWriter::WriteControlBlock(Preprocessor &PP, ASTContext &Context,
                                  StringRef isysroot,
                                  const std::string &OutputFile) {
  using namespace llvm;
  Stream.EnterSubblock(CONTROL_BLOCK_ID, 5);
  RecordData Record;
  
  // Metadata
  BitCodeAbbrev *MetadataAbbrev = new BitCodeAbbrev();
  MetadataAbbrev->Add(BitCodeAbbrevOp(METADATA));
  MetadataAbbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 16)); // Major
  MetadataAbbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 16)); // Minor
  MetadataAbbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 16)); // Clang maj.
  MetadataAbbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 16)); // Clang min.
  MetadataAbbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1)); // Relocatable
  MetadataAbbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1)); // Errors
  MetadataAbbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // SVN branch/tag
  unsigned MetadataAbbrevCode = Stream.EmitAbbrev(MetadataAbbrev);
  Record.push_back(METADATA);
  Record.push_back(VERSION_MAJOR);
  Record.push_back(VERSION_MINOR);
  Record.push_back(CLANG_VERSION_MAJOR);
  Record.push_back(CLANG_VERSION_MINOR);
  assert((!WritingModule || isysroot.empty()) &&
         "writing module as a relocatable PCH?");
  Record.push_back(!isysroot.empty());
  Record.push_back(ASTHasCompilerErrors);
  Stream.EmitRecordWithBlob(MetadataAbbrevCode, Record,
                            getClangFullRepositoryVersion());

  if (WritingModule) {
    // For implicit modules we output a signature that we can use to ensure
    // duplicate module builds don't collide in the cache as their output order
    // is non-deterministic.
    // FIXME: Remove this when output is deterministic.
    if (Context.getLangOpts().ImplicitModules) {
      Record.clear();
      Record.push_back(getSignature());
      Stream.EmitRecord(SIGNATURE, Record);
    }

    // Module name
    BitCodeAbbrev *Abbrev = new BitCodeAbbrev();
    Abbrev->Add(BitCodeAbbrevOp(MODULE_NAME));
    Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // Name
    unsigned AbbrevCode = Stream.EmitAbbrev(Abbrev);
    RecordData Record;
    Record.push_back(MODULE_NAME);
    Stream.EmitRecordWithBlob(AbbrevCode, Record, WritingModule->Name);
  }

  if (WritingModule && WritingModule->Directory) {
    // Module directory.
    BitCodeAbbrev *Abbrev = new BitCodeAbbrev();
    Abbrev->Add(BitCodeAbbrevOp(MODULE_DIRECTORY));
    Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // Directory
    unsigned AbbrevCode = Stream.EmitAbbrev(Abbrev);
    RecordData Record;
    Record.push_back(MODULE_DIRECTORY);

    SmallString<128> BaseDir(WritingModule->Directory->getName());
    cleanPathForOutput(Context.getSourceManager().getFileManager(), BaseDir);
    Stream.EmitRecordWithBlob(AbbrevCode, Record, BaseDir);

    // Write out all other paths relative to the base directory if possible.
    BaseDirectory.assign(BaseDir.begin(), BaseDir.end());
  } else if (!isysroot.empty()) {
    // Write out paths relative to the sysroot if possible.
    BaseDirectory = isysroot;
  }

  // Module map file
  if (WritingModule) {
    Record.clear();

    auto &Map = PP.getHeaderSearchInfo().getModuleMap();

    // Primary module map file.
    AddPath(Map.getModuleMapFileForUniquing(WritingModule)->getName(), Record);

    // Additional module map files.
    if (auto *AdditionalModMaps =
            Map.getAdditionalModuleMapFiles(WritingModule)) {
      Record.push_back(AdditionalModMaps->size());
      for (const FileEntry *F : *AdditionalModMaps)
        AddPath(F->getName(), Record);
    } else {
      Record.push_back(0);
    }

    Stream.EmitRecord(MODULE_MAP_FILE, Record);
  }

  // Imports
  if (Chain) {
    serialization::ModuleManager &Mgr = Chain->getModuleManager();
    Record.clear();

    for (auto *M : Mgr) {
      // Skip modules that weren't directly imported.
      if (!M->isDirectlyImported())
        continue;

      Record.push_back((unsigned)M->Kind); // FIXME: Stable encoding
      AddSourceLocation(M->ImportLoc, Record);
      Record.push_back(M->File->getSize());
      Record.push_back(M->File->getModificationTime());
      Record.push_back(M->Signature);
      AddPath(M->FileName, Record);
    }
    Stream.EmitRecord(IMPORTS, Record);

    // Also emit a list of known module files that were not imported,
    // but are made available by this module.
    // FIXME: Should we also include a signature here?
    Record.clear();
    for (auto *E : Mgr.getAdditionalKnownModuleFiles())
      AddPath(E->getName(), Record);
    if (!Record.empty())
      Stream.EmitRecord(KNOWN_MODULE_FILES, Record);
  }

  // Language options.
  Record.clear();
  const LangOptions &LangOpts = Context.getLangOpts();
#define LANGOPT(Name, Bits, Default, Description) \
  Record.push_back(LangOpts.Name);
#define LANGOPT_BOOL(Name, Default, Description) \
  Record.push_back(LangOpts.Name);
#define ENUM_LANGOPT(Name, Type, Bits, Default, Description) \
  Record.push_back(static_cast<unsigned>(LangOpts.get##Name()));
#include "clang/Basic/LangOptions.fixed.def"  
#define SANITIZER(NAME, ID)                                                    \
  Record.push_back(LangOpts.Sanitize.has(SanitizerKind::ID));
#include "clang/Basic/Sanitizers.def"

  Record.push_back(LangOpts.ModuleFeatures.size());
  for (StringRef Feature : LangOpts.ModuleFeatures)
    AddString(Feature, Record);

  Record.push_back((unsigned) LangOpts.ObjCRuntime.getKind());
  AddVersionTuple(LangOpts.ObjCRuntime.getVersion(), Record);

  AddString(LangOpts.CurrentModule, Record);

  // Comment options.
  Record.push_back(LangOpts.CommentOpts.BlockCommandNames.size());
  for (CommentOptions::BlockCommandNamesTy::const_iterator
           I = LangOpts.CommentOpts.BlockCommandNames.begin(),
           IEnd = LangOpts.CommentOpts.BlockCommandNames.end();
       I != IEnd; ++I) {
    AddString(*I, Record);
  }
  Record.push_back(LangOpts.CommentOpts.ParseAllComments);

  Stream.EmitRecord(LANGUAGE_OPTIONS, Record);

  // Target options.
  Record.clear();
  const TargetInfo &Target = Context.getTargetInfo();
  const TargetOptions &TargetOpts = Target.getTargetOpts();
  AddString(TargetOpts.Triple, Record);
  AddString(TargetOpts.CPU, Record);
  AddString(TargetOpts.ABI, Record);
  Record.push_back(TargetOpts.FeaturesAsWritten.size());
  for (unsigned I = 0, N = TargetOpts.FeaturesAsWritten.size(); I != N; ++I) {
    AddString(TargetOpts.FeaturesAsWritten[I], Record);
  }
  Record.push_back(TargetOpts.Features.size());
  for (unsigned I = 0, N = TargetOpts.Features.size(); I != N; ++I) {
    AddString(TargetOpts.Features[I], Record);
  }
  Stream.EmitRecord(TARGET_OPTIONS, Record);

  // Diagnostic options.
  Record.clear();
  const DiagnosticOptions &DiagOpts
    = Context.getDiagnostics().getDiagnosticOptions();
#define DIAGOPT(Name, Bits, Default) Record.push_back(DiagOpts.Name);
#define ENUM_DIAGOPT(Name, Type, Bits, Default) \
  Record.push_back(static_cast<unsigned>(DiagOpts.get##Name()));
#include "clang/Basic/DiagnosticOptions.def"
  Record.push_back(DiagOpts.Warnings.size());
  for (unsigned I = 0, N = DiagOpts.Warnings.size(); I != N; ++I)
    AddString(DiagOpts.Warnings[I], Record);
  Record.push_back(DiagOpts.Remarks.size());
  for (unsigned I = 0, N = DiagOpts.Remarks.size(); I != N; ++I)
    AddString(DiagOpts.Remarks[I], Record);
  // Note: we don't serialize the log or serialization file names, because they
  // are generally transient files and will almost always be overridden.
  Stream.EmitRecord(DIAGNOSTIC_OPTIONS, Record);

  // File system options.
  Record.clear();
  const FileSystemOptions &FSOpts
    = Context.getSourceManager().getFileManager().getFileSystemOptions();
  AddString(FSOpts.WorkingDir, Record);
  Stream.EmitRecord(FILE_SYSTEM_OPTIONS, Record);

  // Header search options.
  Record.clear();
  const HeaderSearchOptions &HSOpts
    = PP.getHeaderSearchInfo().getHeaderSearchOpts();
  AddString(HSOpts.Sysroot, Record);

  // Include entries.
  Record.push_back(HSOpts.UserEntries.size());
  for (unsigned I = 0, N = HSOpts.UserEntries.size(); I != N; ++I) {
    const HeaderSearchOptions::Entry &Entry = HSOpts.UserEntries[I];
    AddString(Entry.Path, Record);
    Record.push_back(static_cast<unsigned>(Entry.Group));
    Record.push_back(Entry.IsFramework);
    Record.push_back(Entry.IgnoreSysRoot);
  }

  // System header prefixes.
  Record.push_back(HSOpts.SystemHeaderPrefixes.size());
  for (unsigned I = 0, N = HSOpts.SystemHeaderPrefixes.size(); I != N; ++I) {
    AddString(HSOpts.SystemHeaderPrefixes[I].Prefix, Record);
    Record.push_back(HSOpts.SystemHeaderPrefixes[I].IsSystemHeader);
  }

  AddString(HSOpts.ResourceDir, Record);
  AddString(HSOpts.ModuleCachePath, Record);
  AddString(HSOpts.ModuleUserBuildPath, Record);
  Record.push_back(HSOpts.DisableModuleHash);
  Record.push_back(HSOpts.UseBuiltinIncludes);
  Record.push_back(HSOpts.UseStandardSystemIncludes);
  Record.push_back(HSOpts.UseStandardCXXIncludes);
  Record.push_back(HSOpts.UseLibcxx);
  // Write out the specific module cache path that contains the module files.
  AddString(PP.getHeaderSearchInfo().getModuleCachePath(), Record);
  Stream.EmitRecord(HEADER_SEARCH_OPTIONS, Record);

  // Preprocessor options.
  Record.clear();
  const PreprocessorOptions &PPOpts = PP.getPreprocessorOpts();

  // Macro definitions.
  Record.push_back(PPOpts.Macros.size());
  for (unsigned I = 0, N = PPOpts.Macros.size(); I != N; ++I) {
    AddString(PPOpts.Macros[I].first, Record);
    Record.push_back(PPOpts.Macros[I].second);
  }

  // Includes
  Record.push_back(PPOpts.Includes.size());
  for (unsigned I = 0, N = PPOpts.Includes.size(); I != N; ++I)
    AddString(PPOpts.Includes[I], Record);

  // Macro includes
  Record.push_back(PPOpts.MacroIncludes.size());
  for (unsigned I = 0, N = PPOpts.MacroIncludes.size(); I != N; ++I)
    AddString(PPOpts.MacroIncludes[I], Record);

  Record.push_back(PPOpts.UsePredefines);
  // Detailed record is important since it is used for the module cache hash.
  Record.push_back(PPOpts.DetailedRecord);
  AddString(PPOpts.ImplicitPCHInclude, Record);
  AddString(PPOpts.ImplicitPTHInclude, Record);
  Record.push_back(static_cast<unsigned>(PPOpts.ObjCXXARCStandardLibrary));
  Stream.EmitRecord(PREPROCESSOR_OPTIONS, Record);

  // Original file name and file ID
  SourceManager &SM = Context.getSourceManager();
  if (const FileEntry *MainFile = SM.getFileEntryForID(SM.getMainFileID())) {
    BitCodeAbbrev *FileAbbrev = new BitCodeAbbrev();
    FileAbbrev->Add(BitCodeAbbrevOp(ORIGINAL_FILE));
    FileAbbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6)); // File ID
    FileAbbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // File name
    unsigned FileAbbrevCode = Stream.EmitAbbrev(FileAbbrev);

    Record.clear();
    Record.push_back(ORIGINAL_FILE);
    Record.push_back(SM.getMainFileID().getOpaqueValue());
    EmitRecordWithPath(FileAbbrevCode, Record, MainFile->getName());
  }

  Record.clear();
  Record.push_back(SM.getMainFileID().getOpaqueValue());
  Stream.EmitRecord(ORIGINAL_FILE_ID, Record);

  // Original PCH directory
  if (!OutputFile.empty() && OutputFile != "-") {
    BitCodeAbbrev *Abbrev = new BitCodeAbbrev();
    Abbrev->Add(BitCodeAbbrevOp(ORIGINAL_PCH_DIR));
    Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // File name
    unsigned AbbrevCode = Stream.EmitAbbrev(Abbrev);

    SmallString<128> OutputPath(OutputFile);

    llvm::sys::fs::make_absolute(OutputPath);
    StringRef origDir = llvm::sys::path::parent_path(OutputPath);

    RecordData Record;
    Record.push_back(ORIGINAL_PCH_DIR);
    Stream.EmitRecordWithBlob(AbbrevCode, Record, origDir);
  }

  WriteInputFiles(Context.SourceMgr,
                  PP.getHeaderSearchInfo().getHeaderSearchOpts(),
                  PP.getLangOpts().Modules);
  Stream.ExitBlock();
}

namespace  {
  /// \brief An input file.
  struct InputFileEntry {
    const FileEntry *File;
    bool IsSystemFile;
    bool BufferOverridden;
  };
}

void ASTWriter::WriteInputFiles(SourceManager &SourceMgr,
                                HeaderSearchOptions &HSOpts,
                                bool Modules) {
  using namespace llvm;
  Stream.EnterSubblock(INPUT_FILES_BLOCK_ID, 4);
  RecordData Record;
  
  // Create input-file abbreviation.
  BitCodeAbbrev *IFAbbrev = new BitCodeAbbrev();
  IFAbbrev->Add(BitCodeAbbrevOp(INPUT_FILE));
  IFAbbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6)); // ID
  IFAbbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 12)); // Size
  IFAbbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 32)); // Modification time
  IFAbbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1)); // Overridden
  IFAbbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // File name
  unsigned IFAbbrevCode = Stream.EmitAbbrev(IFAbbrev);

  // Get all ContentCache objects for files, sorted by whether the file is a
  // system one or not. System files go at the back, users files at the front.
  std::deque<InputFileEntry> SortedFiles;
  for (unsigned I = 1, N = SourceMgr.local_sloc_entry_size(); I != N; ++I) {
    // Get this source location entry.
    const SrcMgr::SLocEntry *SLoc = &SourceMgr.getLocalSLocEntry(I);
    assert(&SourceMgr.getSLocEntry(FileID::get(I)) == SLoc);

    // We only care about file entries that were not overridden.
    if (!SLoc->isFile())
      continue;
    const SrcMgr::ContentCache *Cache = SLoc->getFile().getContentCache();
    if (!Cache->OrigEntry)
      continue;

    InputFileEntry Entry;
    Entry.File = Cache->OrigEntry;
    Entry.IsSystemFile = Cache->IsSystemFile;
    Entry.BufferOverridden = Cache->BufferOverridden;
    if (Cache->IsSystemFile)
      SortedFiles.push_back(Entry);
    else
      SortedFiles.push_front(Entry);
  }

  unsigned UserFilesNum = 0;
  // Write out all of the input files.
  std::vector<uint64_t> InputFileOffsets;
  for (std::deque<InputFileEntry>::iterator
         I = SortedFiles.begin(), E = SortedFiles.end(); I != E; ++I) {
    const InputFileEntry &Entry = *I;

    uint32_t &InputFileID = InputFileIDs[Entry.File];
    if (InputFileID != 0)
      continue; // already recorded this file.

    // Record this entry's offset.
    InputFileOffsets.push_back(Stream.GetCurrentBitNo());

    InputFileID = InputFileOffsets.size();

    if (!Entry.IsSystemFile)
      ++UserFilesNum;

    Record.clear();
    Record.push_back(INPUT_FILE);
    Record.push_back(InputFileOffsets.size());

    // Emit size/modification time for this file.
    Record.push_back(Entry.File->getSize());
    Record.push_back(Entry.File->getModificationTime());

    // Whether this file was overridden.
    Record.push_back(Entry.BufferOverridden);

    EmitRecordWithPath(IFAbbrevCode, Record, Entry.File->getName());
  }

  Stream.ExitBlock();

  // Create input file offsets abbreviation.
  BitCodeAbbrev *OffsetsAbbrev = new BitCodeAbbrev();
  OffsetsAbbrev->Add(BitCodeAbbrevOp(INPUT_FILE_OFFSETS));
  OffsetsAbbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6)); // # input files
  OffsetsAbbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6)); // # non-system
                                                                //   input files
  OffsetsAbbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));   // Array
  unsigned OffsetsAbbrevCode = Stream.EmitAbbrev(OffsetsAbbrev);

  // Write input file offsets.
  Record.clear();
  Record.push_back(INPUT_FILE_OFFSETS);
  Record.push_back(InputFileOffsets.size());
  Record.push_back(UserFilesNum);
  Stream.EmitRecordWithBlob(OffsetsAbbrevCode, Record, bytes(InputFileOffsets));
}

//===----------------------------------------------------------------------===//
// Source Manager Serialization
//===----------------------------------------------------------------------===//

/// \brief Create an abbreviation for the SLocEntry that refers to a
/// file.
static unsigned CreateSLocFileAbbrev(llvm::BitstreamWriter &Stream) {
  using namespace llvm;
  BitCodeAbbrev *Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(SM_SLOC_FILE_ENTRY));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 8)); // Offset
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 8)); // Include location
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 2)); // Characteristic
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1)); // Line directives
  // FileEntry fields.
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6)); // Input File ID
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 8)); // NumCreatedFIDs
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 24)); // FirstDeclIndex
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 8)); // NumDecls
  return Stream.EmitAbbrev(Abbrev);
}

/// \brief Create an abbreviation for the SLocEntry that refers to a
/// buffer.
static unsigned CreateSLocBufferAbbrev(llvm::BitstreamWriter &Stream) {
  using namespace llvm;
  BitCodeAbbrev *Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(SM_SLOC_BUFFER_ENTRY));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 8)); // Offset
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 8)); // Include location
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 2)); // Characteristic
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1)); // Line directives
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // Buffer name blob
  return Stream.EmitAbbrev(Abbrev);
}

/// \brief Create an abbreviation for the SLocEntry that refers to a
/// buffer's blob.
static unsigned CreateSLocBufferBlobAbbrev(llvm::BitstreamWriter &Stream) {
  using namespace llvm;
  BitCodeAbbrev *Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(SM_SLOC_BUFFER_BLOB));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // Blob
  return Stream.EmitAbbrev(Abbrev);
}

/// \brief Create an abbreviation for the SLocEntry that refers to a macro
/// expansion.
static unsigned CreateSLocExpansionAbbrev(llvm::BitstreamWriter &Stream) {
  using namespace llvm;
  BitCodeAbbrev *Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(SM_SLOC_EXPANSION_ENTRY));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 8)); // Offset
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 8)); // Spelling location
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 8)); // Start location
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 8)); // End location
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6)); // Token length
  return Stream.EmitAbbrev(Abbrev);
}

namespace {
  // Trait used for the on-disk hash table of header search information.
  class HeaderFileInfoTrait {
    ASTWriter &Writer;
    const HeaderSearch &HS;
    
    // Keep track of the framework names we've used during serialization.
    SmallVector<char, 128> FrameworkStringData;
    llvm::StringMap<unsigned> FrameworkNameOffset;
    
  public:
    HeaderFileInfoTrait(ASTWriter &Writer, const HeaderSearch &HS)
      : Writer(Writer), HS(HS) { }
    
    struct key_type {
      const FileEntry *FE;
      const char *Filename;
    };
    typedef const key_type &key_type_ref;
    
    typedef HeaderFileInfo data_type;
    typedef const data_type &data_type_ref;
    typedef unsigned hash_value_type;
    typedef unsigned offset_type;
    
    static hash_value_type ComputeHash(key_type_ref key) {
      // The hash is based only on size/time of the file, so that the reader can
      // match even when symlinking or excess path elements ("foo/../", "../")
      // change the form of the name. However, complete path is still the key.
      //
      // FIXME: Using the mtime here will cause problems for explicit module
      // imports.
      return llvm::hash_combine(key.FE->getSize(),
                                key.FE->getModificationTime());
    }
    
    std::pair<unsigned,unsigned>
    EmitKeyDataLength(raw_ostream& Out, key_type_ref key, data_type_ref Data) {
      using namespace llvm::support;
      endian::Writer<little> Writer(Out);
      unsigned KeyLen = strlen(key.Filename) + 1 + 8 + 8;
      Writer.write<uint16_t>(KeyLen);
      unsigned DataLen = 1 + 2 + 4 + 4;
      if (Data.isModuleHeader)
        DataLen += 4;
      Writer.write<uint8_t>(DataLen);
      return std::make_pair(KeyLen, DataLen);
    }
    
    void EmitKey(raw_ostream& Out, key_type_ref key, unsigned KeyLen) {
      using namespace llvm::support;
      endian::Writer<little> LE(Out);
      LE.write<uint64_t>(key.FE->getSize());
      KeyLen -= 8;
      LE.write<uint64_t>(key.FE->getModificationTime());
      KeyLen -= 8;
      Out.write(key.Filename, KeyLen);
    }
    
    void EmitData(raw_ostream &Out, key_type_ref key,
                  data_type_ref Data, unsigned DataLen) {
      using namespace llvm::support;
      endian::Writer<little> LE(Out);
      uint64_t Start = Out.tell(); (void)Start;
      
      unsigned char Flags = (Data.HeaderRole << 6)
                          | (Data.isImport << 5)
                          | (Data.isPragmaOnce << 4)
                          | (Data.DirInfo << 2)
                          | (Data.Resolved << 1)
                          | Data.IndexHeaderMapHeader;
      LE.write<uint8_t>(Flags);
      LE.write<uint16_t>(Data.NumIncludes);
      
      if (!Data.ControllingMacro)
        LE.write<uint32_t>(Data.ControllingMacroID);
      else
        LE.write<uint32_t>(Writer.getIdentifierRef(Data.ControllingMacro));
      
      unsigned Offset = 0;
      if (!Data.Framework.empty()) {
        // If this header refers into a framework, save the framework name.
        llvm::StringMap<unsigned>::iterator Pos
          = FrameworkNameOffset.find(Data.Framework);
        if (Pos == FrameworkNameOffset.end()) {
          Offset = FrameworkStringData.size() + 1;
          FrameworkStringData.append(Data.Framework.begin(), 
                                     Data.Framework.end());
          FrameworkStringData.push_back(0);
          
          FrameworkNameOffset[Data.Framework] = Offset;
        } else
          Offset = Pos->second;
      }
      LE.write<uint32_t>(Offset);

      if (Data.isModuleHeader) {
        Module *Mod = HS.findModuleForHeader(key.FE).getModule();
        LE.write<uint32_t>(Writer.getExistingSubmoduleID(Mod));
      }

      assert(Out.tell() - Start == DataLen && "Wrong data length");
    }
    
    const char *strings_begin() const { return FrameworkStringData.begin(); }
    const char *strings_end() const { return FrameworkStringData.end(); }
  };
} // end anonymous namespace

/// \brief Write the header search block for the list of files that 
///
/// \param HS The header search structure to save.
void ASTWriter::WriteHeaderSearch(const HeaderSearch &HS) {
  SmallVector<const FileEntry *, 16> FilesByUID;
  HS.getFileMgr().GetUniqueIDMapping(FilesByUID);
  
  if (FilesByUID.size() > HS.header_file_size())
    FilesByUID.resize(HS.header_file_size());
  
  HeaderFileInfoTrait GeneratorTrait(*this, HS);
  llvm::OnDiskChainedHashTableGenerator<HeaderFileInfoTrait> Generator;
  SmallVector<const char *, 4> SavedStrings;
  unsigned NumHeaderSearchEntries = 0;
  for (unsigned UID = 0, LastUID = FilesByUID.size(); UID != LastUID; ++UID) {
    const FileEntry *File = FilesByUID[UID];
    if (!File)
      continue;

    // Use HeaderSearch's getFileInfo to make sure we get the HeaderFileInfo
    // from the external source if it was not provided already.
    HeaderFileInfo HFI;
    if (!HS.tryGetFileInfo(File, HFI) ||
        (HFI.External && Chain) ||
        (HFI.isModuleHeader && !HFI.isCompilingModuleHeader))
      continue;

    // Massage the file path into an appropriate form.
    const char *Filename = File->getName();
    SmallString<128> FilenameTmp(Filename);
    if (PreparePathForOutput(FilenameTmp)) {
      // If we performed any translation on the file name at all, we need to
      // save this string, since the generator will refer to it later.
      Filename = _strdup(FilenameTmp.c_str());
      SavedStrings.push_back(Filename);
    }

    HeaderFileInfoTrait::key_type key = { File, Filename };
    Generator.insert(key, HFI, GeneratorTrait);
    ++NumHeaderSearchEntries;
  }
  
  // Create the on-disk hash table in a buffer.
  SmallString<4096> TableData;
  uint32_t BucketOffset;
  {
    using namespace llvm::support;
    llvm::raw_svector_ostream Out(TableData);
    // Make sure that no bucket is at offset 0
    endian::Writer<little>(Out).write<uint32_t>(0);
    BucketOffset = Generator.Emit(Out, GeneratorTrait);
  }

  // Create a blob abbreviation
  using namespace llvm;
  BitCodeAbbrev *Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(HEADER_SEARCH_TABLE));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));
  unsigned TableAbbrev = Stream.EmitAbbrev(Abbrev);
  
  // Write the header search table
  RecordData Record;
  Record.push_back(HEADER_SEARCH_TABLE);
  Record.push_back(BucketOffset);
  Record.push_back(NumHeaderSearchEntries);
  Record.push_back(TableData.size());
  TableData.append(GeneratorTrait.strings_begin(),GeneratorTrait.strings_end());
  Stream.EmitRecordWithBlob(TableAbbrev, Record, TableData);
  
  // Free all of the strings we had to duplicate.
  for (unsigned I = 0, N = SavedStrings.size(); I != N; ++I)
    free(const_cast<char *>(SavedStrings[I]));
}

/// \brief Writes the block containing the serialized form of the
/// source manager.
///
/// TODO: We should probably use an on-disk hash table (stored in a
/// blob), indexed based on the file name, so that we only create
/// entries for files that we actually need. In the common case (no
/// errors), we probably won't have to create file entries for any of
/// the files in the AST.
void ASTWriter::WriteSourceManagerBlock(SourceManager &SourceMgr,
                                        const Preprocessor &PP) {
  RecordData Record;

  // Enter the source manager block.
  Stream.EnterSubblock(SOURCE_MANAGER_BLOCK_ID, 3);

  // Abbreviations for the various kinds of source-location entries.
  unsigned SLocFileAbbrv = CreateSLocFileAbbrev(Stream);
  unsigned SLocBufferAbbrv = CreateSLocBufferAbbrev(Stream);
  unsigned SLocBufferBlobAbbrv = CreateSLocBufferBlobAbbrev(Stream);
  unsigned SLocExpansionAbbrv = CreateSLocExpansionAbbrev(Stream);

  // Write out the source location entry table. We skip the first
  // entry, which is always the same dummy entry.
  std::vector<uint32_t> SLocEntryOffsets;
  RecordData PreloadSLocs;
  SLocEntryOffsets.reserve(SourceMgr.local_sloc_entry_size() - 1);
  for (unsigned I = 1, N = SourceMgr.local_sloc_entry_size();
       I != N; ++I) {
    // Get this source location entry.
    const SrcMgr::SLocEntry *SLoc = &SourceMgr.getLocalSLocEntry(I);
    FileID FID = FileID::get(I);
    assert(&SourceMgr.getSLocEntry(FID) == SLoc);

    // Record the offset of this source-location entry.
    SLocEntryOffsets.push_back(Stream.GetCurrentBitNo());

    // Figure out which record code to use.
    unsigned Code;
    if (SLoc->isFile()) {
      const SrcMgr::ContentCache *Cache = SLoc->getFile().getContentCache();
      if (Cache->OrigEntry) {
        Code = SM_SLOC_FILE_ENTRY;
      } else
        Code = SM_SLOC_BUFFER_ENTRY;
    } else
      Code = SM_SLOC_EXPANSION_ENTRY;
    Record.clear();
    Record.push_back(Code);

    // Starting offset of this entry within this module, so skip the dummy.
    Record.push_back(SLoc->getOffset() - 2);
    if (SLoc->isFile()) {
      const SrcMgr::FileInfo &File = SLoc->getFile();
      Record.push_back(File.getIncludeLoc().getRawEncoding());
      Record.push_back(File.getFileCharacteristic()); // FIXME: stable encoding
      Record.push_back(File.hasLineDirectives());

      const SrcMgr::ContentCache *Content = File.getContentCache();
      if (Content->OrigEntry) {
        assert(Content->OrigEntry == Content->ContentsEntry &&
               "Writing to AST an overridden file is not supported");

        // The source location entry is a file. Emit input file ID.
        assert(InputFileIDs[Content->OrigEntry] != 0 && "Missed file entry");
        Record.push_back(InputFileIDs[Content->OrigEntry]);

        Record.push_back(File.NumCreatedFIDs);
        
        FileDeclIDsTy::iterator FDI = FileDeclIDs.find(FID);
        if (FDI != FileDeclIDs.end()) {
          Record.push_back(FDI->second->FirstDeclIndex);
          Record.push_back(FDI->second->DeclIDs.size());
        } else {
          Record.push_back(0);
          Record.push_back(0);
        }
        
        Stream.EmitRecordWithAbbrev(SLocFileAbbrv, Record);
        
        if (Content->BufferOverridden) {
          Record.clear();
          Record.push_back(SM_SLOC_BUFFER_BLOB);
          const llvm::MemoryBuffer *Buffer
            = Content->getBuffer(PP.getDiagnostics(), PP.getSourceManager());
          Stream.EmitRecordWithBlob(SLocBufferBlobAbbrv, Record,
                                    StringRef(Buffer->getBufferStart(),
                                              Buffer->getBufferSize() + 1));          
        }
      } else {
        // The source location entry is a buffer. The blob associated
        // with this entry contains the contents of the buffer.

        // We add one to the size so that we capture the trailing NULL
        // that is required by llvm::MemoryBuffer::getMemBuffer (on
        // the reader side).
        const llvm::MemoryBuffer *Buffer
          = Content->getBuffer(PP.getDiagnostics(), PP.getSourceManager());
        const char *Name = Buffer->getBufferIdentifier();
        Stream.EmitRecordWithBlob(SLocBufferAbbrv, Record,
                                  StringRef(Name, strlen(Name) + 1));
        Record.clear();
        Record.push_back(SM_SLOC_BUFFER_BLOB);
        Stream.EmitRecordWithBlob(SLocBufferBlobAbbrv, Record,
                                  StringRef(Buffer->getBufferStart(),
                                                  Buffer->getBufferSize() + 1));

        if (strcmp(Name, "<built-in>") == 0) {
          PreloadSLocs.push_back(SLocEntryOffsets.size());
        }
      }
    } else {
      // The source location entry is a macro expansion.
      const SrcMgr::ExpansionInfo &Expansion = SLoc->getExpansion();
      Record.push_back(Expansion.getSpellingLoc().getRawEncoding());
      Record.push_back(Expansion.getExpansionLocStart().getRawEncoding());
      Record.push_back(Expansion.isMacroArgExpansion() ? 0
                             : Expansion.getExpansionLocEnd().getRawEncoding());

      // Compute the token length for this macro expansion.
      unsigned NextOffset = SourceMgr.getNextLocalOffset();
      if (I + 1 != N)
        NextOffset = SourceMgr.getLocalSLocEntry(I + 1).getOffset();
      Record.push_back(NextOffset - SLoc->getOffset() - 1);
      Stream.EmitRecordWithAbbrev(SLocExpansionAbbrv, Record);
    }
  }

  Stream.ExitBlock();

  if (SLocEntryOffsets.empty())
    return;

  // Write the source-location offsets table into the AST block. This
  // table is used for lazily loading source-location information.
  using namespace llvm;
  BitCodeAbbrev *Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(SOURCE_LOCATION_OFFSETS));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 16)); // # of slocs
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 16)); // total size
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // offsets
  unsigned SLocOffsetsAbbrev = Stream.EmitAbbrev(Abbrev);

  Record.clear();
  Record.push_back(SOURCE_LOCATION_OFFSETS);
  Record.push_back(SLocEntryOffsets.size());
  Record.push_back(SourceMgr.getNextLocalOffset() - 1); // skip dummy
  Stream.EmitRecordWithBlob(SLocOffsetsAbbrev, Record, bytes(SLocEntryOffsets));

  // Write the source location entry preloads array, telling the AST
  // reader which source locations entries it should load eagerly.
  Stream.EmitRecord(SOURCE_LOCATION_PRELOADS, PreloadSLocs);

  // Write the line table. It depends on remapping working, so it must come
  // after the source location offsets.
  if (SourceMgr.hasLineTable()) {
    LineTableInfo &LineTable = SourceMgr.getLineTable();

    Record.clear();
    // Emit the file names.
    Record.push_back(LineTable.getNumFilenames());
    for (unsigned I = 0, N = LineTable.getNumFilenames(); I != N; ++I)
      AddPath(LineTable.getFilename(I), Record);

    // Emit the line entries
    for (LineTableInfo::iterator L = LineTable.begin(), LEnd = LineTable.end();
         L != LEnd; ++L) {
      // Only emit entries for local files.
      if (L->first.ID < 0)
        continue;

      // Emit the file ID
      Record.push_back(L->first.ID);

      // Emit the line entries
      Record.push_back(L->second.size());
      for (std::vector<LineEntry>::iterator LE = L->second.begin(),
                                         LEEnd = L->second.end();
           LE != LEEnd; ++LE) {
        Record.push_back(LE->FileOffset);
        Record.push_back(LE->LineNo);
        Record.push_back(LE->FilenameID);
        Record.push_back((unsigned)LE->FileKind);
        Record.push_back(LE->IncludeOffset);
      }
    }
    Stream.EmitRecord(SOURCE_MANAGER_LINE_TABLE, Record);
  }
}

//===----------------------------------------------------------------------===//
// Preprocessor Serialization
//===----------------------------------------------------------------------===//

static bool shouldIgnoreMacro(MacroDirective *MD, bool IsModule,
                              const Preprocessor &PP) {
  if (MacroInfo *MI = MD->getMacroInfo())
    if (MI->isBuiltinMacro())
      return true;

  if (IsModule) {
    SourceLocation Loc = MD->getLocation();
    if (Loc.isInvalid())
      return true;
    if (PP.getSourceManager().getFileID(Loc) == PP.getPredefinesFileID())
      return true;
  }

  return false;
}

/// \brief Writes the block containing the serialized form of the
/// preprocessor.
///
void ASTWriter::WritePreprocessor(const Preprocessor &PP, bool IsModule) {
  PreprocessingRecord *PPRec = PP.getPreprocessingRecord();
  if (PPRec)
    WritePreprocessorDetail(*PPRec);

  RecordData Record;
  RecordData ModuleMacroRecord;

  // If the preprocessor __COUNTER__ value has been bumped, remember it.
  if (PP.getCounterValue() != 0) {
    Record.push_back(PP.getCounterValue());
    Stream.EmitRecord(PP_COUNTER_VALUE, Record);
    Record.clear();
  }

  // Enter the preprocessor block.
  Stream.EnterSubblock(PREPROCESSOR_BLOCK_ID, 3);

  // If the AST file contains __DATE__ or __TIME__ emit a warning about this.
  // FIXME: use diagnostics subsystem for localization etc.
  if (PP.SawDateOrTime())
    fprintf(stderr, "warning: precompiled header used __DATE__ or __TIME__.\n");


  // Loop over all the macro directives that are live at the end of the file,
  // emitting each to the PP section.

  // Construct the list of identifiers with macro directives that need to be
  // serialized.
  SmallVector<const IdentifierInfo *, 128> MacroIdentifiers;
  for (auto &Id : PP.getIdentifierTable())
    if (Id.second->hadMacroDefinition() &&
        (!Id.second->isFromAST() ||
         Id.second->hasChangedSinceDeserialization()))
      MacroIdentifiers.push_back(Id.second);
  // Sort the set of macro definitions that need to be serialized by the
  // name of the macro, to provide a stable ordering.
  std::sort(MacroIdentifiers.begin(), MacroIdentifiers.end(),
            llvm::less_ptr<IdentifierInfo>());

  // Emit the macro directives as a list and associate the offset with the
  // identifier they belong to.
  for (const IdentifierInfo *Name : MacroIdentifiers) {
    MacroDirective *MD = PP.getLocalMacroDirectiveHistory(Name);
    auto StartOffset = Stream.GetCurrentBitNo();

    // Emit the macro directives in reverse source order.
    for (; MD; MD = MD->getPrevious()) {
      // Once we hit an ignored macro, we're done: the rest of the chain
      // will all be ignored macros.
      if (shouldIgnoreMacro(MD, IsModule, PP))
        break;

      AddSourceLocation(MD->getLocation(), Record);
      Record.push_back(MD->getKind());
      if (auto *DefMD = dyn_cast<DefMacroDirective>(MD)) {
        Record.push_back(getMacroRef(DefMD->getInfo(), Name));
      } else if (auto *VisMD = dyn_cast<VisibilityMacroDirective>(MD)) {
        Record.push_back(VisMD->isPublic());
      }
    }

    // Write out any exported module macros.
    bool EmittedModuleMacros = false;
    if (IsModule) {
      auto Leafs = PP.getLeafModuleMacros(Name);
      SmallVector<ModuleMacro*, 8> Worklist(Leafs.begin(), Leafs.end());
      llvm::DenseMap<ModuleMacro*, unsigned> Visits;
      while (!Worklist.empty()) {
        auto *Macro = Worklist.pop_back_val();

        // Emit a record indicating this submodule exports this macro.
        ModuleMacroRecord.push_back(
            getSubmoduleID(Macro->getOwningModule()));
        ModuleMacroRecord.push_back(getMacroRef(Macro->getMacroInfo(), Name));
        for (auto *M : Macro->overrides())
          ModuleMacroRecord.push_back(getSubmoduleID(M->getOwningModule()));

        Stream.EmitRecord(PP_MODULE_MACRO, ModuleMacroRecord);
        ModuleMacroRecord.clear();

        // Enqueue overridden macros once we've visited all their ancestors.
        for (auto *M : Macro->overrides())
          if (++Visits[M] == M->getNumOverridingMacros())
            Worklist.push_back(M);

        EmittedModuleMacros = true;
      }
    }

    if (Record.empty() && !EmittedModuleMacros)
      continue;

    IdentMacroDirectivesOffsetMap[Name] = StartOffset;
    Stream.EmitRecord(PP_MACRO_DIRECTIVE_HISTORY, Record);
    Record.clear();
  }

  /// \brief Offsets of each of the macros into the bitstream, indexed by
  /// the local macro ID
  ///
  /// For each identifier that is associated with a macro, this map
  /// provides the offset into the bitstream where that macro is
  /// defined.
  std::vector<uint32_t> MacroOffsets;

  for (unsigned I = 0, N = MacroInfosToEmit.size(); I != N; ++I) {
    const IdentifierInfo *Name = MacroInfosToEmit[I].Name;
    MacroInfo *MI = MacroInfosToEmit[I].MI;
    MacroID ID = MacroInfosToEmit[I].ID;

    if (ID < FirstMacroID) {
      assert(0 && "Loaded MacroInfo entered MacroInfosToEmit ?");
      continue;
    }

    // Record the local offset of this macro.
    unsigned Index = ID - FirstMacroID;
    if (Index == MacroOffsets.size())
      MacroOffsets.push_back(Stream.GetCurrentBitNo());
    else {
      if (Index > MacroOffsets.size())
        MacroOffsets.resize(Index + 1);

      MacroOffsets[Index] = Stream.GetCurrentBitNo();
    }

    AddIdentifierRef(Name, Record);
    Record.push_back(inferSubmoduleIDFromLocation(MI->getDefinitionLoc()));
    AddSourceLocation(MI->getDefinitionLoc(), Record);
    AddSourceLocation(MI->getDefinitionEndLoc(), Record);
    Record.push_back(MI->isUsed());
    Record.push_back(MI->isUsedForHeaderGuard());
    unsigned Code;
    if (MI->isObjectLike()) {
      Code = PP_MACRO_OBJECT_LIKE;
    } else {
      Code = PP_MACRO_FUNCTION_LIKE;

      Record.push_back(MI->isC99Varargs());
      Record.push_back(MI->isGNUVarargs());
      Record.push_back(MI->hasCommaPasting());
      Record.push_back(MI->getNumArgs());
      for (const IdentifierInfo *Arg : MI->args())
        AddIdentifierRef(Arg, Record);
    }

    // If we have a detailed preprocessing record, record the macro definition
    // ID that corresponds to this macro.
    if (PPRec)
      Record.push_back(MacroDefinitions[PPRec->findMacroDefinition(MI)]);

    Stream.EmitRecord(Code, Record);
    Record.clear();

    // Emit the tokens array.
    for (unsigned TokNo = 0, e = MI->getNumTokens(); TokNo != e; ++TokNo) {
      // Note that we know that the preprocessor does not have any annotation
      // tokens in it because they are created by the parser, and thus can't
      // be in a macro definition.
      const Token &Tok = MI->getReplacementToken(TokNo);
      AddToken(Tok, Record);
      Stream.EmitRecord(PP_TOKEN, Record);
      Record.clear();
    }
    ++NumMacros;
  }

  Stream.ExitBlock();

  // Write the offsets table for macro IDs.
  using namespace llvm;
  auto *Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(MACRO_OFFSET));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32)); // # of macros
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32)); // first ID
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));

  unsigned MacroOffsetAbbrev = Stream.EmitAbbrev(Abbrev);
  Record.clear();
  Record.push_back(MACRO_OFFSET);
  Record.push_back(MacroOffsets.size());
  Record.push_back(FirstMacroID - NUM_PREDEF_MACRO_IDS);
  Stream.EmitRecordWithBlob(MacroOffsetAbbrev, Record,
                            bytes(MacroOffsets));
}

void ASTWriter::WritePreprocessorDetail(PreprocessingRecord &PPRec) {
  if (PPRec.local_begin() == PPRec.local_end())
    return;

  SmallVector<PPEntityOffset, 64> PreprocessedEntityOffsets;

  // Enter the preprocessor block.
  Stream.EnterSubblock(PREPROCESSOR_DETAIL_BLOCK_ID, 3);

  // If the preprocessor has a preprocessing record, emit it.
  unsigned NumPreprocessingRecords = 0;
  using namespace llvm;
  
  // Set up the abbreviation for 
  unsigned InclusionAbbrev = 0;
  {
    BitCodeAbbrev *Abbrev = new BitCodeAbbrev();
    Abbrev->Add(BitCodeAbbrevOp(PPD_INCLUSION_DIRECTIVE));
    Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32)); // filename length
    Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1)); // in quotes
    Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 2)); // kind
    Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1)); // imported module
    Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));
    InclusionAbbrev = Stream.EmitAbbrev(Abbrev);
  }
  
  unsigned FirstPreprocessorEntityID 
    = (Chain ? PPRec.getNumLoadedPreprocessedEntities() : 0) 
    + NUM_PREDEF_PP_ENTITY_IDS;
  unsigned NextPreprocessorEntityID = FirstPreprocessorEntityID;
  RecordData Record;
  for (PreprocessingRecord::iterator E = PPRec.local_begin(),
                                  EEnd = PPRec.local_end();
       E != EEnd; 
       (void)++E, ++NumPreprocessingRecords, ++NextPreprocessorEntityID) {
    Record.clear();

    PreprocessedEntityOffsets.push_back(
        PPEntityOffset((*E)->getSourceRange(), Stream.GetCurrentBitNo()));

    if (MacroDefinitionRecord *MD = dyn_cast<MacroDefinitionRecord>(*E)) {
      // Record this macro definition's ID.
      MacroDefinitions[MD] = NextPreprocessorEntityID;

      AddIdentifierRef(MD->getName(), Record);
      Stream.EmitRecord(PPD_MACRO_DEFINITION, Record);
      continue;
    }

    if (MacroExpansion *ME = dyn_cast<MacroExpansion>(*E)) {
      Record.push_back(ME->isBuiltinMacro());
      if (ME->isBuiltinMacro())
        AddIdentifierRef(ME->getName(), Record);
      else
        Record.push_back(MacroDefinitions[ME->getDefinition()]);
      Stream.EmitRecord(PPD_MACRO_EXPANSION, Record);
      continue;
    }

    if (InclusionDirective *ID = dyn_cast<InclusionDirective>(*E)) {
      Record.push_back(PPD_INCLUSION_DIRECTIVE);
      Record.push_back(ID->getFileName().size());
      Record.push_back(ID->wasInQuotes());
      Record.push_back(static_cast<unsigned>(ID->getKind()));
      Record.push_back(ID->importedModule());
      SmallString<64> Buffer;
      Buffer += ID->getFileName();
      // Check that the FileEntry is not null because it was not resolved and
      // we create a PCH even with compiler errors.
      if (ID->getFile())
        Buffer += ID->getFile()->getName();
      Stream.EmitRecordWithBlob(InclusionAbbrev, Record, Buffer);
      continue;
    }
    
    llvm_unreachable("Unhandled PreprocessedEntity in ASTWriter");
  }
  Stream.ExitBlock();

  // Write the offsets table for the preprocessing record.
  if (NumPreprocessingRecords > 0) {
    assert(PreprocessedEntityOffsets.size() == NumPreprocessingRecords);

    // Write the offsets table for identifier IDs.
    using namespace llvm;
    BitCodeAbbrev *Abbrev = new BitCodeAbbrev();
    Abbrev->Add(BitCodeAbbrevOp(PPD_ENTITIES_OFFSETS));
    Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32)); // first pp entity
    Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));
    unsigned PPEOffsetAbbrev = Stream.EmitAbbrev(Abbrev);

    Record.clear();
    Record.push_back(PPD_ENTITIES_OFFSETS);
    Record.push_back(FirstPreprocessorEntityID - NUM_PREDEF_PP_ENTITY_IDS);
    Stream.EmitRecordWithBlob(PPEOffsetAbbrev, Record,
                              bytes(PreprocessedEntityOffsets));
  }
}

unsigned ASTWriter::getSubmoduleID(Module *Mod) {
  llvm::DenseMap<Module *, unsigned>::iterator Known = SubmoduleIDs.find(Mod);
  if (Known != SubmoduleIDs.end())
    return Known->second;
  
  return SubmoduleIDs[Mod] = NextSubmoduleID++;
}

unsigned ASTWriter::getExistingSubmoduleID(Module *Mod) const {
  if (!Mod)
    return 0;

  llvm::DenseMap<Module *, unsigned>::const_iterator
    Known = SubmoduleIDs.find(Mod);
  if (Known != SubmoduleIDs.end())
    return Known->second;

  return 0;
}

/// \brief Compute the number of modules within the given tree (including the
/// given module).
static unsigned getNumberOfModules(Module *Mod) {
  unsigned ChildModules = 0;
  for (Module::submodule_iterator Sub = Mod->submodule_begin(),
                               SubEnd = Mod->submodule_end();
       Sub != SubEnd; ++Sub)
    ChildModules += getNumberOfModules(*Sub);
  
  return ChildModules + 1;
}

void ASTWriter::WriteSubmodules(Module *WritingModule) {
  // Enter the submodule description block.
  Stream.EnterSubblock(SUBMODULE_BLOCK_ID, /*bits for abbreviations*/5);
  
  // Write the abbreviations needed for the submodules block.
  using namespace llvm;
  BitCodeAbbrev *Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(SUBMODULE_DEFINITION));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6)); // ID
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6)); // Parent
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1)); // IsFramework
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1)); // IsExplicit
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1)); // IsSystem
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1)); // IsExternC
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1)); // InferSubmodules...
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1)); // InferExplicit...
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1)); // InferExportWild...
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1)); // ConfigMacrosExh...
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // Name
  unsigned DefinitionAbbrev = Stream.EmitAbbrev(Abbrev);

  Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(SUBMODULE_UMBRELLA_HEADER));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // Name
  unsigned UmbrellaAbbrev = Stream.EmitAbbrev(Abbrev);

  Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(SUBMODULE_HEADER));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // Name
  unsigned HeaderAbbrev = Stream.EmitAbbrev(Abbrev);

  Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(SUBMODULE_TOPHEADER));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // Name
  unsigned TopHeaderAbbrev = Stream.EmitAbbrev(Abbrev);

  Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(SUBMODULE_UMBRELLA_DIR));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // Name
  unsigned UmbrellaDirAbbrev = Stream.EmitAbbrev(Abbrev);

  Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(SUBMODULE_REQUIRES));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1)); // State
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));     // Feature
  unsigned RequiresAbbrev = Stream.EmitAbbrev(Abbrev);

  Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(SUBMODULE_EXCLUDED_HEADER));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // Name
  unsigned ExcludedHeaderAbbrev = Stream.EmitAbbrev(Abbrev);

  Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(SUBMODULE_TEXTUAL_HEADER));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // Name
  unsigned TextualHeaderAbbrev = Stream.EmitAbbrev(Abbrev);

  Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(SUBMODULE_PRIVATE_HEADER));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // Name
  unsigned PrivateHeaderAbbrev = Stream.EmitAbbrev(Abbrev);

  Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(SUBMODULE_PRIVATE_TEXTUAL_HEADER));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // Name
  unsigned PrivateTextualHeaderAbbrev = Stream.EmitAbbrev(Abbrev);

  Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(SUBMODULE_LINK_LIBRARY));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 1)); // IsFramework
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));     // Name
  unsigned LinkLibraryAbbrev = Stream.EmitAbbrev(Abbrev);

  Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(SUBMODULE_CONFIG_MACRO));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));    // Macro name
  unsigned ConfigMacroAbbrev = Stream.EmitAbbrev(Abbrev);

  Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(SUBMODULE_CONFLICT));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6));  // Other module
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));    // Message
  unsigned ConflictAbbrev = Stream.EmitAbbrev(Abbrev);

  // Write the submodule metadata block.
  RecordData Record;
  Record.push_back(getNumberOfModules(WritingModule));
  Record.push_back(FirstSubmoduleID - NUM_PREDEF_SUBMODULE_IDS);
  Stream.EmitRecord(SUBMODULE_METADATA, Record);
  
  // Write all of the submodules.
  std::queue<Module *> Q;
  Q.push(WritingModule);
  while (!Q.empty()) {
    Module *Mod = Q.front();
    Q.pop();
    unsigned ID = getSubmoduleID(Mod);
    
    // Emit the definition of the block.
    Record.clear();
    Record.push_back(SUBMODULE_DEFINITION);
    Record.push_back(ID);
    if (Mod->Parent) {
      assert(SubmoduleIDs[Mod->Parent] && "Submodule parent not written?");
      Record.push_back(SubmoduleIDs[Mod->Parent]);
    } else {
      Record.push_back(0);
    }
    Record.push_back(Mod->IsFramework);
    Record.push_back(Mod->IsExplicit);
    Record.push_back(Mod->IsSystem);
    Record.push_back(Mod->IsExternC);
    Record.push_back(Mod->InferSubmodules);
    Record.push_back(Mod->InferExplicitSubmodules);
    Record.push_back(Mod->InferExportWildcard);
    Record.push_back(Mod->ConfigMacrosExhaustive);
    Stream.EmitRecordWithBlob(DefinitionAbbrev, Record, Mod->Name);
    
    // Emit the requirements.
    for (unsigned I = 0, N = Mod->Requirements.size(); I != N; ++I) {
      Record.clear();
      Record.push_back(SUBMODULE_REQUIRES);
      Record.push_back(Mod->Requirements[I].second);
      Stream.EmitRecordWithBlob(RequiresAbbrev, Record,
                                Mod->Requirements[I].first);
    }

    // Emit the umbrella header, if there is one.
    if (auto UmbrellaHeader = Mod->getUmbrellaHeader()) {
      Record.clear();
      Record.push_back(SUBMODULE_UMBRELLA_HEADER);
      Stream.EmitRecordWithBlob(UmbrellaAbbrev, Record,
                                UmbrellaHeader.NameAsWritten);
    } else if (auto UmbrellaDir = Mod->getUmbrellaDir()) {
      Record.clear();
      Record.push_back(SUBMODULE_UMBRELLA_DIR);
      Stream.EmitRecordWithBlob(UmbrellaDirAbbrev, Record, 
                                UmbrellaDir.NameAsWritten);
    }

    // Emit the headers.
    struct {
      unsigned RecordKind;
      unsigned Abbrev;
      Module::HeaderKind HeaderKind;
    } HeaderLists[] = {
      {SUBMODULE_HEADER, HeaderAbbrev, Module::HK_Normal},
      {SUBMODULE_TEXTUAL_HEADER, TextualHeaderAbbrev, Module::HK_Textual},
      {SUBMODULE_PRIVATE_HEADER, PrivateHeaderAbbrev, Module::HK_Private},
      {SUBMODULE_PRIVATE_TEXTUAL_HEADER, PrivateTextualHeaderAbbrev,
        Module::HK_PrivateTextual},
      {SUBMODULE_EXCLUDED_HEADER, ExcludedHeaderAbbrev, Module::HK_Excluded}
    };
    for (auto &HL : HeaderLists) {
      Record.clear();
      Record.push_back(HL.RecordKind);
      for (auto &H : Mod->Headers[HL.HeaderKind])
        Stream.EmitRecordWithBlob(HL.Abbrev, Record, H.NameAsWritten);
    }

    // Emit the top headers.
    {
      auto TopHeaders = Mod->getTopHeaders(PP->getFileManager());
      Record.clear();
      Record.push_back(SUBMODULE_TOPHEADER);
      for (auto *H : TopHeaders)
        Stream.EmitRecordWithBlob(TopHeaderAbbrev, Record, H->getName());
    }

    // Emit the imports. 
    if (!Mod->Imports.empty()) {
      Record.clear();
      for (unsigned I = 0, N = Mod->Imports.size(); I != N; ++I) {
        unsigned ImportedID = getSubmoduleID(Mod->Imports[I]);
        assert(ImportedID && "Unknown submodule!");
        Record.push_back(ImportedID);
      }
      Stream.EmitRecord(SUBMODULE_IMPORTS, Record);
    }

    // Emit the exports. 
    if (!Mod->Exports.empty()) {
      Record.clear();
      for (unsigned I = 0, N = Mod->Exports.size(); I != N; ++I) {
        if (Module *Exported = Mod->Exports[I].getPointer()) {
          unsigned ExportedID = getSubmoduleID(Exported);
          Record.push_back(ExportedID);
        } else {
          Record.push_back(0);
        }
        
        Record.push_back(Mod->Exports[I].getInt());
      }
      Stream.EmitRecord(SUBMODULE_EXPORTS, Record);
    }

    //FIXME: How do we emit the 'use'd modules?  They may not be submodules.
    // Might be unnecessary as use declarations are only used to build the
    // module itself.

    // Emit the link libraries.
    for (unsigned I = 0, N = Mod->LinkLibraries.size(); I != N; ++I) {
      Record.clear();
      Record.push_back(SUBMODULE_LINK_LIBRARY);
      Record.push_back(Mod->LinkLibraries[I].IsFramework);
      Stream.EmitRecordWithBlob(LinkLibraryAbbrev, Record,
                                Mod->LinkLibraries[I].Library);
    }

    // Emit the conflicts.
    for (unsigned I = 0, N = Mod->Conflicts.size(); I != N; ++I) {
      Record.clear();
      Record.push_back(SUBMODULE_CONFLICT);
      unsigned OtherID = getSubmoduleID(Mod->Conflicts[I].Other);
      assert(OtherID && "Unknown submodule!");
      Record.push_back(OtherID);
      Stream.EmitRecordWithBlob(ConflictAbbrev, Record,
                                Mod->Conflicts[I].Message);
    }

    // Emit the configuration macros.
    for (unsigned I = 0, N =  Mod->ConfigMacros.size(); I != N; ++I) {
      Record.clear();
      Record.push_back(SUBMODULE_CONFIG_MACRO);
      Stream.EmitRecordWithBlob(ConfigMacroAbbrev, Record,
                                Mod->ConfigMacros[I]);
    }

    // Queue up the submodules of this module.
    for (Module::submodule_iterator Sub = Mod->submodule_begin(),
                                 SubEnd = Mod->submodule_end();
         Sub != SubEnd; ++Sub)
      Q.push(*Sub);
  }
  
  Stream.ExitBlock();

  // FIXME: This can easily happen, if we have a reference to a submodule that
  // did not result in us loading a module file for that submodule. For
  // instance, a cross-top-level-module 'conflict' declaration will hit this.
  assert((NextSubmoduleID - FirstSubmoduleID ==
          getNumberOfModules(WritingModule)) &&
         "Wrong # of submodules; found a reference to a non-local, "
         "non-imported submodule?");
}

serialization::SubmoduleID 
ASTWriter::inferSubmoduleIDFromLocation(SourceLocation Loc) {
  if (Loc.isInvalid() || !WritingModule)
    return 0; // No submodule
    
  // Find the module that owns this location.
  ModuleMap &ModMap = PP->getHeaderSearchInfo().getModuleMap();
  Module *OwningMod 
    = ModMap.inferModuleFromLocation(FullSourceLoc(Loc,PP->getSourceManager()));
  if (!OwningMod)
    return 0;
  
  // Check whether this submodule is part of our own module.
  if (WritingModule != OwningMod && !OwningMod->isSubModuleOf(WritingModule))
    return 0;
  
  return getSubmoduleID(OwningMod);
}

void ASTWriter::WritePragmaDiagnosticMappings(const DiagnosticsEngine &Diag,
                                              bool isModule) {
  // Make sure set diagnostic pragmas don't affect the translation unit that
  // imports the module.
  // FIXME: Make diagnostic pragma sections work properly with modules.
  if (isModule)
    return;

  llvm::SmallDenseMap<const DiagnosticsEngine::DiagState *, unsigned, 64>
      DiagStateIDMap;
  unsigned CurrID = 0;
  DiagStateIDMap[&Diag.DiagStates.front()] = ++CurrID; // the command-line one.
  RecordData Record;
  for (DiagnosticsEngine::DiagStatePointsTy::const_iterator
         I = Diag.DiagStatePoints.begin(), E = Diag.DiagStatePoints.end();
         I != E; ++I) {
    const DiagnosticsEngine::DiagStatePoint &point = *I;
    if (point.Loc.isInvalid())
      continue;

    Record.push_back(point.Loc.getRawEncoding());
    unsigned &DiagStateID = DiagStateIDMap[point.State];
    Record.push_back(DiagStateID);
    
    if (DiagStateID == 0) {
      DiagStateID = ++CurrID;
      for (DiagnosticsEngine::DiagState::const_iterator
             I = point.State->begin(), E = point.State->end(); I != E; ++I) {
        if (I->second.isPragma()) {
          Record.push_back(I->first);
          Record.push_back((unsigned)I->second.getSeverity());
        }
      }
      Record.push_back(-1); // mark the end of the diag/map pairs for this
                            // location.
    }
  }

  if (!Record.empty())
    Stream.EmitRecord(DIAG_PRAGMA_MAPPINGS, Record);
}

void ASTWriter::WriteCXXCtorInitializersOffsets() {
  if (CXXCtorInitializersOffsets.empty())
    return;

  RecordData Record;

  // Create a blob abbreviation for the C++ ctor initializer offsets.
  using namespace llvm;

  BitCodeAbbrev *Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(CXX_CTOR_INITIALIZERS_OFFSETS));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32)); // size
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));
  unsigned CtorInitializersOffsetAbbrev = Stream.EmitAbbrev(Abbrev);

  // Write the base specifier offsets table.
  Record.clear();
  Record.push_back(CXX_CTOR_INITIALIZERS_OFFSETS);
  Record.push_back(CXXCtorInitializersOffsets.size());
  Stream.EmitRecordWithBlob(CtorInitializersOffsetAbbrev, Record,
                            bytes(CXXCtorInitializersOffsets));
}

void ASTWriter::WriteCXXBaseSpecifiersOffsets() {
  if (CXXBaseSpecifiersOffsets.empty())
    return;

  RecordData Record;

  // Create a blob abbreviation for the C++ base specifiers offsets.
  using namespace llvm;
    
  BitCodeAbbrev *Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(CXX_BASE_SPECIFIER_OFFSETS));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32)); // size
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));
  unsigned BaseSpecifierOffsetAbbrev = Stream.EmitAbbrev(Abbrev);
  
  // Write the base specifier offsets table.
  Record.clear();
  Record.push_back(CXX_BASE_SPECIFIER_OFFSETS);
  Record.push_back(CXXBaseSpecifiersOffsets.size());
  Stream.EmitRecordWithBlob(BaseSpecifierOffsetAbbrev, Record,
                            bytes(CXXBaseSpecifiersOffsets));
}

//===----------------------------------------------------------------------===//
// Type Serialization
//===----------------------------------------------------------------------===//

/// \brief Write the representation of a type to the AST stream.
void ASTWriter::WriteType(QualType T) {
  TypeIdx &Idx = TypeIdxs[T];
  if (Idx.getIndex() == 0) // we haven't seen this type before.
    Idx = TypeIdx(NextTypeID++);

  assert(Idx.getIndex() >= FirstTypeID && "Re-writing a type from a prior AST");

  // Record the offset for this type.
  unsigned Index = Idx.getIndex() - FirstTypeID;
  if (TypeOffsets.size() == Index)
    TypeOffsets.push_back(Stream.GetCurrentBitNo());
  else if (TypeOffsets.size() < Index) {
    TypeOffsets.resize(Index + 1);
    TypeOffsets[Index] = Stream.GetCurrentBitNo();
  }

  RecordData Record;

  // Emit the type's representation.
  ASTTypeWriter W(*this, Record);
  W.AbbrevToUse = 0;

  if (T.hasLocalNonFastQualifiers()) {
    Qualifiers Qs = T.getLocalQualifiers();
    AddTypeRef(T.getLocalUnqualifiedType(), Record);
    Record.push_back(Qs.getAsOpaqueValue());
    W.Code = TYPE_EXT_QUAL;
    W.AbbrevToUse = TypeExtQualAbbrev;
  } else {
    switch (T->getTypeClass()) {
      // For all of the concrete, non-dependent types, call the
      // appropriate visitor function.
#define TYPE(Class, Base) \
    case Type::Class: W.Visit##Class##Type(cast<Class##Type>(T)); break;
#define ABSTRACT_TYPE(Class, Base)
#include "clang/AST/TypeNodes.def"
    }
  }

  // Emit the serialized record.
  Stream.EmitRecord(W.Code, Record, W.AbbrevToUse);

  // Flush any expressions that were written as part of this type.
  FlushStmts();
}

//===----------------------------------------------------------------------===//
// Declaration Serialization
//===----------------------------------------------------------------------===//

/// \brief Write the block containing all of the declaration IDs
/// lexically declared within the given DeclContext.
///
/// \returns the offset of the DECL_CONTEXT_LEXICAL block within the
/// bistream, or 0 if no block was written.
uint64_t ASTWriter::WriteDeclContextLexicalBlock(ASTContext &Context,
                                                 DeclContext *DC) {
  if (DC->decls_empty())
    return 0;

  uint64_t Offset = Stream.GetCurrentBitNo();
  RecordData Record;
  Record.push_back(DECL_CONTEXT_LEXICAL);
  SmallVector<KindDeclIDPair, 64> Decls;
  for (const auto *D : DC->decls())
    Decls.push_back(std::make_pair(D->getKind(), GetDeclRef(D)));

  ++NumLexicalDeclContexts;
  Stream.EmitRecordWithBlob(DeclContextLexicalAbbrev, Record, bytes(Decls));
  return Offset;
}

void ASTWriter::WriteTypeDeclOffsets() {
  using namespace llvm;
  RecordData Record;

  // Write the type offsets array
  BitCodeAbbrev *Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(TYPE_OFFSET));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32)); // # of types
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32)); // base type index
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // types block
  unsigned TypeOffsetAbbrev = Stream.EmitAbbrev(Abbrev);
  Record.clear();
  Record.push_back(TYPE_OFFSET);
  Record.push_back(TypeOffsets.size());
  Record.push_back(FirstTypeID - NUM_PREDEF_TYPE_IDS);
  Stream.EmitRecordWithBlob(TypeOffsetAbbrev, Record, bytes(TypeOffsets));

  // Write the declaration offsets array
  Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(DECL_OFFSET));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32)); // # of declarations
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32)); // base decl ID
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob)); // declarations block
  unsigned DeclOffsetAbbrev = Stream.EmitAbbrev(Abbrev);
  Record.clear();
  Record.push_back(DECL_OFFSET);
  Record.push_back(DeclOffsets.size());
  Record.push_back(FirstDeclID - NUM_PREDEF_DECL_IDS);
  Stream.EmitRecordWithBlob(DeclOffsetAbbrev, Record, bytes(DeclOffsets));
}

void ASTWriter::WriteFileDeclIDsMap() {
  using namespace llvm;
  RecordData Record;

  SmallVector<std::pair<FileID, DeclIDInFileInfo *>, 64> SortedFileDeclIDs(
      FileDeclIDs.begin(), FileDeclIDs.end());
  std::sort(SortedFileDeclIDs.begin(), SortedFileDeclIDs.end(),
            llvm::less_first());

  // Join the vectors of DeclIDs from all files.
  SmallVector<DeclID, 256> FileGroupedDeclIDs;
  for (auto &FileDeclEntry : SortedFileDeclIDs) {
    DeclIDInFileInfo &Info = *FileDeclEntry.second;
    Info.FirstDeclIndex = FileGroupedDeclIDs.size();
    for (auto &LocDeclEntry : Info.DeclIDs)
      FileGroupedDeclIDs.push_back(LocDeclEntry.second);
  }

  BitCodeAbbrev *Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(FILE_SORTED_DECLS));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));
  unsigned AbbrevCode = Stream.EmitAbbrev(Abbrev);
  Record.push_back(FILE_SORTED_DECLS);
  Record.push_back(FileGroupedDeclIDs.size());
  Stream.EmitRecordWithBlob(AbbrevCode, Record, bytes(FileGroupedDeclIDs));
}

void ASTWriter::WriteComments() {
  Stream.EnterSubblock(COMMENTS_BLOCK_ID, 3);
  ArrayRef<RawComment *> RawComments = Context->Comments.getComments();
  RecordData Record;
  for (ArrayRef<RawComment *>::iterator I = RawComments.begin(),
                                        E = RawComments.end();
       I != E; ++I) {
    Record.clear();
    AddSourceRange((*I)->getSourceRange(), Record);
    Record.push_back((*I)->getKind());
    Record.push_back((*I)->isTrailingComment());
    Record.push_back((*I)->isAlmostTrailingComment());
    Stream.EmitRecord(COMMENTS_RAW_COMMENT, Record);
  }
  Stream.ExitBlock();
}

//===----------------------------------------------------------------------===//
// Global Method Pool and Selector Serialization
//===----------------------------------------------------------------------===//

namespace {
// Trait used for the on-disk hash table used in the method pool.
class ASTMethodPoolTrait {
  ASTWriter &Writer;

public:
  typedef Selector key_type;
  typedef key_type key_type_ref;

  struct data_type {
    SelectorID ID;
    ObjCMethodList Instance, Factory;
  };
  typedef const data_type& data_type_ref;

  typedef unsigned hash_value_type;
  typedef unsigned offset_type;

  explicit ASTMethodPoolTrait(ASTWriter &Writer) : Writer(Writer) { }

  static hash_value_type ComputeHash(Selector Sel) {
    return serialization::ComputeHash(Sel);
  }

  std::pair<unsigned,unsigned>
    EmitKeyDataLength(raw_ostream& Out, Selector Sel,
                      data_type_ref Methods) {
    using namespace llvm::support;
    endian::Writer<little> LE(Out);
    unsigned KeyLen = 2 + (Sel.getNumArgs()? Sel.getNumArgs() * 4 : 4);
    LE.write<uint16_t>(KeyLen);
    unsigned DataLen = 4 + 2 + 2; // 2 bytes for each of the method counts
    for (const ObjCMethodList *Method = &Methods.Instance; Method;
         Method = Method->getNext())
      if (Method->getMethod())
        DataLen += 4;
    for (const ObjCMethodList *Method = &Methods.Factory; Method;
         Method = Method->getNext())
      if (Method->getMethod())
        DataLen += 4;
    LE.write<uint16_t>(DataLen);
    return std::make_pair(KeyLen, DataLen);
  }

  void EmitKey(raw_ostream& Out, Selector Sel, unsigned) {
    using namespace llvm::support;
    endian::Writer<little> LE(Out);
    uint64_t Start = Out.tell();
    assert((Start >> 32) == 0 && "Selector key offset too large");
    Writer.SetSelectorOffset(Sel, Start);
    unsigned N = Sel.getNumArgs();
    LE.write<uint16_t>(N);
    if (N == 0)
      N = 1;
    for (unsigned I = 0; I != N; ++I)
      LE.write<uint32_t>(
          Writer.getIdentifierRef(Sel.getIdentifierInfoForSlot(I)));
  }

  void EmitData(raw_ostream& Out, key_type_ref,
                data_type_ref Methods, unsigned DataLen) {
    using namespace llvm::support;
    endian::Writer<little> LE(Out);
    uint64_t Start = Out.tell(); (void)Start;
    LE.write<uint32_t>(Methods.ID);
    unsigned NumInstanceMethods = 0;
    for (const ObjCMethodList *Method = &Methods.Instance; Method;
         Method = Method->getNext())
      if (Method->getMethod())
        ++NumInstanceMethods;

    unsigned NumFactoryMethods = 0;
    for (const ObjCMethodList *Method = &Methods.Factory; Method;
         Method = Method->getNext())
      if (Method->getMethod())
        ++NumFactoryMethods;

    unsigned InstanceBits = Methods.Instance.getBits();
    assert(InstanceBits < 4);
    unsigned InstanceHasMoreThanOneDeclBit =
        Methods.Instance.hasMoreThanOneDecl();
    unsigned FullInstanceBits = (NumInstanceMethods << 3) |
                                (InstanceHasMoreThanOneDeclBit << 2) |
                                InstanceBits;
    unsigned FactoryBits = Methods.Factory.getBits();
    assert(FactoryBits < 4);
    unsigned FactoryHasMoreThanOneDeclBit =
        Methods.Factory.hasMoreThanOneDecl();
    unsigned FullFactoryBits = (NumFactoryMethods << 3) |
                               (FactoryHasMoreThanOneDeclBit << 2) |
                               FactoryBits;
    LE.write<uint16_t>(FullInstanceBits);
    LE.write<uint16_t>(FullFactoryBits);
    for (const ObjCMethodList *Method = &Methods.Instance; Method;
         Method = Method->getNext())
      if (Method->getMethod())
        LE.write<uint32_t>(Writer.getDeclID(Method->getMethod()));
    for (const ObjCMethodList *Method = &Methods.Factory; Method;
         Method = Method->getNext())
      if (Method->getMethod())
        LE.write<uint32_t>(Writer.getDeclID(Method->getMethod()));

    assert(Out.tell() - Start == DataLen && "Data length is wrong");
  }
};
} // end anonymous namespace

/// \brief Write ObjC data: selectors and the method pool.
///
/// The method pool contains both instance and factory methods, stored
/// in an on-disk hash table indexed by the selector. The hash table also
/// contains an empty entry for every other selector known to Sema.
void ASTWriter::WriteSelectors(Sema &SemaRef) {
  using namespace llvm;

  // Do we have to do anything at all?
  if (SemaRef.MethodPool.empty() && SelectorIDs.empty())
    return;
  unsigned NumTableEntries = 0;
  // Create and write out the blob that contains selectors and the method pool.
  {
    llvm::OnDiskChainedHashTableGenerator<ASTMethodPoolTrait> Generator;
    ASTMethodPoolTrait Trait(*this);

    // Create the on-disk hash table representation. We walk through every
    // selector we've seen and look it up in the method pool.
    SelectorOffsets.resize(NextSelectorID - FirstSelectorID);
    for (auto &SelectorAndID : SelectorIDs) {
      Selector S = SelectorAndID.first;
      SelectorID ID = SelectorAndID.second;
      Sema::GlobalMethodPool::iterator F = SemaRef.MethodPool.find(S);
      ASTMethodPoolTrait::data_type Data = {
        ID,
        ObjCMethodList(),
        ObjCMethodList()
      };
      if (F != SemaRef.MethodPool.end()) {
        Data.Instance = F->second.first;
        Data.Factory = F->second.second;
      }
      // Only write this selector if it's not in an existing AST or something
      // changed.
      if (Chain && ID < FirstSelectorID) {
        // Selector already exists. Did it change?
        bool changed = false;
        for (ObjCMethodList *M = &Data.Instance;
             !changed && M && M->getMethod(); M = M->getNext()) {
          if (!M->getMethod()->isFromASTFile())
            changed = true;
        }
        for (ObjCMethodList *M = &Data.Factory; !changed && M && M->getMethod();
             M = M->getNext()) {
          if (!M->getMethod()->isFromASTFile())
            changed = true;
        }
        if (!changed)
          continue;
      } else if (Data.Instance.getMethod() || Data.Factory.getMethod()) {
        // A new method pool entry.
        ++NumTableEntries;
      }
      Generator.insert(S, Data, Trait);
    }

    // Create the on-disk hash table in a buffer.
    SmallString<4096> MethodPool;
    uint32_t BucketOffset;
    {
      using namespace llvm::support;
      ASTMethodPoolTrait Trait(*this);
      llvm::raw_svector_ostream Out(MethodPool);
      // Make sure that no bucket is at offset 0
      endian::Writer<little>(Out).write<uint32_t>(0);
      BucketOffset = Generator.Emit(Out, Trait);
    }

    // Create a blob abbreviation
    BitCodeAbbrev *Abbrev = new BitCodeAbbrev();
    Abbrev->Add(BitCodeAbbrevOp(METHOD_POOL));
    Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32));
    Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32));
    Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));
    unsigned MethodPoolAbbrev = Stream.EmitAbbrev(Abbrev);

    // Write the method pool
    RecordData Record;
    Record.push_back(METHOD_POOL);
    Record.push_back(BucketOffset);
    Record.push_back(NumTableEntries);
    Stream.EmitRecordWithBlob(MethodPoolAbbrev, Record, MethodPool);

    // Create a blob abbreviation for the selector table offsets.
    Abbrev = new BitCodeAbbrev();
    Abbrev->Add(BitCodeAbbrevOp(SELECTOR_OFFSETS));
    Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32)); // size
    Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32)); // first ID
    Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));
    unsigned SelectorOffsetAbbrev = Stream.EmitAbbrev(Abbrev);

    // Write the selector offsets table.
    Record.clear();
    Record.push_back(SELECTOR_OFFSETS);
    Record.push_back(SelectorOffsets.size());
    Record.push_back(FirstSelectorID - NUM_PREDEF_SELECTOR_IDS);
    Stream.EmitRecordWithBlob(SelectorOffsetAbbrev, Record,
                              bytes(SelectorOffsets));
  }
}

/// \brief Write the selectors referenced in @selector expression into AST file.
void ASTWriter::WriteReferencedSelectorsPool(Sema &SemaRef) {
  using namespace llvm;
  if (SemaRef.ReferencedSelectors.empty())
    return;

  RecordData Record;

  // Note: this writes out all references even for a dependent AST. But it is
  // very tricky to fix, and given that @selector shouldn't really appear in
  // headers, probably not worth it. It's not a correctness issue.
  for (auto &SelectorAndLocation : SemaRef.ReferencedSelectors) {
    Selector Sel = SelectorAndLocation.first;
    SourceLocation Loc = SelectorAndLocation.second;
    AddSelectorRef(Sel, Record);
    AddSourceLocation(Loc, Record);
  }
  Stream.EmitRecord(REFERENCED_SELECTOR_POOL, Record);
}

//===----------------------------------------------------------------------===//
// Identifier Table Serialization
//===----------------------------------------------------------------------===//

/// Determine the declaration that should be put into the name lookup table to
/// represent the given declaration in this module. This is usually D itself,
/// but if D was imported and merged into a local declaration, we want the most
/// recent local declaration instead. The chosen declaration will be the most
/// recent declaration in any module that imports this one.
static NamedDecl *getDeclForLocalLookup(const LangOptions &LangOpts,
                                        NamedDecl *D) {
  if (!LangOpts.Modules || !D->isFromASTFile())
    return D;

  if (Decl *Redecl = D->getPreviousDecl()) {
    // For Redeclarable decls, a prior declaration might be local.
    for (; Redecl; Redecl = Redecl->getPreviousDecl()) {
      if (!Redecl->isFromASTFile())
        return cast<NamedDecl>(Redecl);
      // If we find a decl from a (chained-)PCH stop since we won't find a
      // local one.
      if (D->getOwningModuleID() == 0)
        break;
    }
  } else if (Decl *First = D->getCanonicalDecl()) {
    // For Mergeable decls, the first decl might be local.
    if (!First->isFromASTFile())
      return cast<NamedDecl>(First);
  }

  // All declarations are imported. Our most recent declaration will also be
  // the most recent one in anyone who imports us.
  return D;
}

namespace {
class ASTIdentifierTableTrait {
  ASTWriter &Writer;
  Preprocessor &PP;
  IdentifierResolver &IdResolver;
  
  /// \brief Determines whether this is an "interesting" identifier that needs a
  /// full IdentifierInfo structure written into the hash table. Notably, this
  /// doesn't check whether the name has macros defined; use PublicMacroIterator
  /// to check that.
  bool isInterestingIdentifier(IdentifierInfo *II, uint64_t MacroOffset) {
    if (MacroOffset ||
        II->isPoisoned() ||
        II->isExtensionToken() ||
        II->getObjCOrBuiltinID() ||
        II->hasRevertedTokenIDToIdentifier() ||
        II->getFETokenInfo<void>())
      return true;

    return false;
  }

public:
  typedef IdentifierInfo* key_type;
  typedef key_type  key_type_ref;

  typedef IdentID data_type;
  typedef data_type data_type_ref;

  typedef unsigned hash_value_type;
  typedef unsigned offset_type;

  ASTIdentifierTableTrait(ASTWriter &Writer, Preprocessor &PP,
                          IdentifierResolver &IdResolver)
      : Writer(Writer), PP(PP), IdResolver(IdResolver) {}

  static hash_value_type ComputeHash(const IdentifierInfo* II) {
    return llvm::HashString(II->getName());
  }

  std::pair<unsigned,unsigned>
  EmitKeyDataLength(raw_ostream& Out, IdentifierInfo* II, IdentID ID) {
    unsigned KeyLen = II->getLength() + 1;
    unsigned DataLen = 4; // 4 bytes for the persistent ID << 1
    auto MacroOffset = Writer.getMacroDirectivesOffset(II);
    if (isInterestingIdentifier(II, MacroOffset)) {
      DataLen += 2; // 2 bytes for builtin ID
      DataLen += 2; // 2 bytes for flags
      if (MacroOffset)
        DataLen += 4; // MacroDirectives offset.

      for (IdentifierResolver::iterator D = IdResolver.begin(II),
                                     DEnd = IdResolver.end();
           D != DEnd; ++D)
        DataLen += 4;
    }
    using namespace llvm::support;
    endian::Writer<little> LE(Out);

    assert((uint16_t)DataLen == DataLen && (uint16_t)KeyLen == KeyLen);
    LE.write<uint16_t>(DataLen);
    // We emit the key length after the data length so that every
    // string is preceded by a 16-bit length. This matches the PTH
    // format for storing identifiers.
    LE.write<uint16_t>(KeyLen);
    return std::make_pair(KeyLen, DataLen);
  }

  void EmitKey(raw_ostream& Out, const IdentifierInfo* II,
               unsigned KeyLen) {
    // Record the location of the key data.  This is used when generating
    // the mapping from persistent IDs to strings.
    Writer.SetIdentifierOffset(II, Out.tell());
    Out.write(II->getNameStart(), KeyLen);
  }

  void EmitData(raw_ostream& Out, IdentifierInfo* II,
                IdentID ID, unsigned) {
    using namespace llvm::support;
    endian::Writer<little> LE(Out);

    auto MacroOffset = Writer.getMacroDirectivesOffset(II);
    if (!isInterestingIdentifier(II, MacroOffset)) {
      LE.write<uint32_t>(ID << 1);
      return;
    }

    LE.write<uint32_t>((ID << 1) | 0x01);
    uint32_t Bits = (uint32_t)II->getObjCOrBuiltinID();
    assert((Bits & 0xffff) == Bits && "ObjCOrBuiltinID too big for ASTReader.");
    LE.write<uint16_t>(Bits);
    Bits = 0;
    bool HadMacroDefinition = MacroOffset != 0;
    Bits = (Bits << 1) | unsigned(HadMacroDefinition);
    Bits = (Bits << 1) | unsigned(II->isExtensionToken());
    Bits = (Bits << 1) | unsigned(II->isPoisoned());
    Bits = (Bits << 1) | unsigned(II->hasRevertedTokenIDToIdentifier());
    Bits = (Bits << 1) | unsigned(II->isCPlusPlusOperatorKeyword());
    LE.write<uint16_t>(Bits);

    if (HadMacroDefinition)
      LE.write<uint32_t>(MacroOffset);

    // Emit the declaration IDs in reverse order, because the
    // IdentifierResolver provides the declarations as they would be
    // visible (e.g., the function "stat" would come before the struct
    // "stat"), but the ASTReader adds declarations to the end of the list
    // (so we need to see the struct "stat" before the function "stat").
    // Only emit declarations that aren't from a chained PCH, though.
    SmallVector<NamedDecl *, 16> Decls(IdResolver.begin(II), IdResolver.end());
    for (SmallVectorImpl<NamedDecl *>::reverse_iterator D = Decls.rbegin(),
                                                        DEnd = Decls.rend();
         D != DEnd; ++D)
      LE.write<uint32_t>(
          Writer.getDeclID(getDeclForLocalLookup(PP.getLangOpts(), *D)));
  }
};
} // end anonymous namespace

/// \brief Write the identifier table into the AST file.
///
/// The identifier table consists of a blob containing string data
/// (the actual identifiers themselves) and a separate "offsets" index
/// that maps identifier IDs to locations within the blob.
void ASTWriter::WriteIdentifierTable(Preprocessor &PP, 
                                     IdentifierResolver &IdResolver,
                                     bool IsModule) {
  using namespace llvm;

  // Create and write out the blob that contains the identifier
  // strings.
  {
    llvm::OnDiskChainedHashTableGenerator<ASTIdentifierTableTrait> Generator;
    ASTIdentifierTableTrait Trait(*this, PP, IdResolver);

    // Look for any identifiers that were named while processing the
    // headers, but are otherwise not needed. We add these to the hash
    // table to enable checking of the predefines buffer in the case
    // where the user adds new macro definitions when building the AST
    // file.
    SmallVector<const IdentifierInfo *, 128> IIs;
    for (IdentifierTable::iterator ID = PP.getIdentifierTable().begin(),
                                IDEnd = PP.getIdentifierTable().end();
         ID != IDEnd; ++ID)
      IIs.push_back(ID->second);
    // Sort the identifiers lexicographically before getting them references so
    // that their order is stable.
    std::sort(IIs.begin(), IIs.end(), llvm::less_ptr<IdentifierInfo>());
    for (const IdentifierInfo *II : IIs)
      getIdentifierRef(II);

    // Create the on-disk hash table representation. We only store offsets
    // for identifiers that appear here for the first time.
    IdentifierOffsets.resize(NextIdentID - FirstIdentID);
    for (auto IdentIDPair : IdentifierIDs) {
      IdentifierInfo *II = const_cast<IdentifierInfo *>(IdentIDPair.first);
      IdentID ID = IdentIDPair.second;
      assert(II && "NULL identifier in identifier table");
      if (!Chain || !II->isFromAST() || II->hasChangedSinceDeserialization())
        Generator.insert(II, ID, Trait);
    }

    // Create the on-disk hash table in a buffer.
    SmallString<4096> IdentifierTable;
    uint32_t BucketOffset;
    {
      using namespace llvm::support;
      llvm::raw_svector_ostream Out(IdentifierTable);
      // Make sure that no bucket is at offset 0
      endian::Writer<little>(Out).write<uint32_t>(0);
      BucketOffset = Generator.Emit(Out, Trait);
    }

    // Create a blob abbreviation
    BitCodeAbbrev *Abbrev = new BitCodeAbbrev();
    Abbrev->Add(BitCodeAbbrevOp(IDENTIFIER_TABLE));
    Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32));
    Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));
    unsigned IDTableAbbrev = Stream.EmitAbbrev(Abbrev);

    // Write the identifier table
    RecordData Record;
    Record.push_back(IDENTIFIER_TABLE);
    Record.push_back(BucketOffset);
    Stream.EmitRecordWithBlob(IDTableAbbrev, Record, IdentifierTable);
  }

  // Write the offsets table for identifier IDs.
  BitCodeAbbrev *Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(IDENTIFIER_OFFSET));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32)); // # of identifiers
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32)); // first ID
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));
  unsigned IdentifierOffsetAbbrev = Stream.EmitAbbrev(Abbrev);

#ifndef NDEBUG
  for (unsigned I = 0, N = IdentifierOffsets.size(); I != N; ++I)
    assert(IdentifierOffsets[I] && "Missing identifier offset?");
#endif
  
  RecordData Record;
  Record.push_back(IDENTIFIER_OFFSET);
  Record.push_back(IdentifierOffsets.size());
  Record.push_back(FirstIdentID - NUM_PREDEF_IDENT_IDS);
  Stream.EmitRecordWithBlob(IdentifierOffsetAbbrev, Record,
                            bytes(IdentifierOffsets));
}

//===----------------------------------------------------------------------===//
// DeclContext's Name Lookup Table Serialization
//===----------------------------------------------------------------------===//

namespace {
// Trait used for the on-disk hash table used in the method pool.
class ASTDeclContextNameLookupTrait {
  ASTWriter &Writer;

public:
  typedef DeclarationName key_type;
  typedef key_type key_type_ref;

  typedef DeclContext::lookup_result data_type;
  typedef const data_type& data_type_ref;

  typedef unsigned hash_value_type;
  typedef unsigned offset_type;

  explicit ASTDeclContextNameLookupTrait(ASTWriter &Writer) : Writer(Writer) { }

  hash_value_type ComputeHash(DeclarationName Name) {
    llvm::FoldingSetNodeID ID;
    ID.AddInteger(Name.getNameKind());

    switch (Name.getNameKind()) {
    case DeclarationName::Identifier:
      ID.AddString(Name.getAsIdentifierInfo()->getName());
      break;
    case DeclarationName::ObjCZeroArgSelector:
    case DeclarationName::ObjCOneArgSelector:
    case DeclarationName::ObjCMultiArgSelector:
      ID.AddInteger(serialization::ComputeHash(Name.getObjCSelector()));
      break;
    case DeclarationName::CXXConstructorName:
    case DeclarationName::CXXDestructorName:
    case DeclarationName::CXXConversionFunctionName:
      break;
    case DeclarationName::CXXOperatorName:
      ID.AddInteger(Name.getCXXOverloadedOperator());
      break;
    case DeclarationName::CXXLiteralOperatorName:
      ID.AddString(Name.getCXXLiteralIdentifier()->getName());
    case DeclarationName::CXXUsingDirective:
      break;
    }

    return ID.ComputeHash();
  }

  std::pair<unsigned,unsigned>
    EmitKeyDataLength(raw_ostream& Out, DeclarationName Name,
                      data_type_ref Lookup) {
    using namespace llvm::support;
    endian::Writer<little> LE(Out);
    unsigned KeyLen = 1;
    switch (Name.getNameKind()) {
    case DeclarationName::Identifier:
    case DeclarationName::ObjCZeroArgSelector:
    case DeclarationName::ObjCOneArgSelector:
    case DeclarationName::ObjCMultiArgSelector:
    case DeclarationName::CXXLiteralOperatorName:
      KeyLen += 4;
      break;
    case DeclarationName::CXXOperatorName:
      KeyLen += 1;
      break;
    case DeclarationName::CXXConstructorName:
    case DeclarationName::CXXDestructorName:
    case DeclarationName::CXXConversionFunctionName:
    case DeclarationName::CXXUsingDirective:
      break;
    }
    LE.write<uint16_t>(KeyLen);

    // 2 bytes for num of decls and 4 for each DeclID.
    unsigned DataLen = 2 + 4 * Lookup.size();
    LE.write<uint16_t>(DataLen);

    return std::make_pair(KeyLen, DataLen);
  }

  void EmitKey(raw_ostream& Out, DeclarationName Name, unsigned) {
    using namespace llvm::support;
    endian::Writer<little> LE(Out);
    LE.write<uint8_t>(Name.getNameKind());
    switch (Name.getNameKind()) {
    case DeclarationName::Identifier:
      LE.write<uint32_t>(Writer.getIdentifierRef(Name.getAsIdentifierInfo()));
      return;
    case DeclarationName::ObjCZeroArgSelector:
    case DeclarationName::ObjCOneArgSelector:
    case DeclarationName::ObjCMultiArgSelector:
      LE.write<uint32_t>(Writer.getSelectorRef(Name.getObjCSelector()));
      return;
    case DeclarationName::CXXOperatorName:
      assert(Name.getCXXOverloadedOperator() < NUM_OVERLOADED_OPERATORS &&
             "Invalid operator?");
      LE.write<uint8_t>(Name.getCXXOverloadedOperator());
      return;
    case DeclarationName::CXXLiteralOperatorName:
      LE.write<uint32_t>(Writer.getIdentifierRef(Name.getCXXLiteralIdentifier()));
      return;
    case DeclarationName::CXXConstructorName:
    case DeclarationName::CXXDestructorName:
    case DeclarationName::CXXConversionFunctionName:
    case DeclarationName::CXXUsingDirective:
      return;
    }

    llvm_unreachable("Invalid name kind?");
  }

  void EmitData(raw_ostream& Out, key_type_ref,
                data_type Lookup, unsigned DataLen) {
    using namespace llvm::support;
    endian::Writer<little> LE(Out);
    uint64_t Start = Out.tell(); (void)Start;
    LE.write<uint16_t>(Lookup.size());
    for (DeclContext::lookup_iterator I = Lookup.begin(), E = Lookup.end();
         I != E; ++I)
      LE.write<uint32_t>(
          Writer.GetDeclRef(getDeclForLocalLookup(Writer.getLangOpts(), *I)));

    assert(Out.tell() - Start == DataLen && "Data length is wrong");
  }
};
} // end anonymous namespace

bool ASTWriter::isLookupResultExternal(StoredDeclsList &Result,
                                       DeclContext *DC) {
  return Result.hasExternalDecls() && DC->NeedToReconcileExternalVisibleStorage;
}

bool ASTWriter::isLookupResultEntirelyExternal(StoredDeclsList &Result,
                                               DeclContext *DC) {
  for (auto *D : Result.getLookupResult())
    if (!getDeclForLocalLookup(getLangOpts(), D)->isFromASTFile())
      return false;

  return true;
}

uint32_t
ASTWriter::GenerateNameLookupTable(const DeclContext *ConstDC,
                                   llvm::SmallVectorImpl<char> &LookupTable) {
  assert(!ConstDC->HasLazyLocalLexicalLookups &&
         !ConstDC->HasLazyExternalLexicalLookups &&
         "must call buildLookups first");

  // FIXME: We need to build the lookups table, which is logically const.
  DeclContext *DC = const_cast<DeclContext*>(ConstDC);
  assert(DC == DC->getPrimaryContext() && "only primary DC has lookup table");

  // Create the on-disk hash table representation.
  llvm::OnDiskChainedHashTableGenerator<ASTDeclContextNameLookupTrait>
      Generator;
  ASTDeclContextNameLookupTrait Trait(*this);

  // The first step is to collect the declaration names which we need to
  // serialize into the name lookup table, and to collect them in a stable
  // order.
  SmallVector<DeclarationName, 16> Names;

  // We also build up small sets of the constructor and conversion function
  // names which are visible.
  llvm::SmallSet<DeclarationName, 8> ConstructorNameSet, ConversionNameSet;

  for (auto &Lookup : *DC->buildLookup()) {
    auto &Name = Lookup.first;
    auto &Result = Lookup.second;

    // If there are no local declarations in our lookup result, we don't
    // need to write an entry for the name at all unless we're rewriting
    // the decl context. If we can't write out a lookup set without
    // performing more deserialization, just skip this entry.
    if (isLookupResultExternal(Result, DC) && !isRewritten(cast<Decl>(DC)) &&
        isLookupResultEntirelyExternal(Result, DC))
      continue;

    // We also skip empty results. If any of the results could be external and
    // the currently available results are empty, then all of the results are
    // external and we skip it above. So the only way we get here with an empty
    // results is when no results could have been external *and* we have
    // external results.
    //
    // FIXME: While we might want to start emitting on-disk entries for negative
    // lookups into a decl context as an optimization, today we *have* to skip
    // them because there are names with empty lookup results in decl contexts
    // which we can't emit in any stable ordering: we lookup constructors and
    // conversion functions in the enclosing namespace scope creating empty
    // results for them. This in almost certainly a bug in Clang's name lookup,
    // but that is likely to be hard or impossible to fix and so we tolerate it
    // here by omitting lookups with empty results.
    if (Lookup.second.getLookupResult().empty())
      continue;

    switch (Lookup.first.getNameKind()) {
    default:
      Names.push_back(Lookup.first);
      break;

    case DeclarationName::CXXConstructorName:
      assert(isa<CXXRecordDecl>(DC) &&
             "Cannot have a constructor name outside of a class!");
      ConstructorNameSet.insert(Name);
      break;

    case DeclarationName::CXXConversionFunctionName:
      assert(isa<CXXRecordDecl>(DC) &&
             "Cannot have a conversion function name outside of a class!");
      ConversionNameSet.insert(Name);
      break;
    }
  }

  // Sort the names into a stable order.
  std::sort(Names.begin(), Names.end());

  if (auto *D = dyn_cast<CXXRecordDecl>(DC)) {
    // We need to establish an ordering of constructor and conversion function
    // names, and they don't have an intrinsic ordering.

    // First we try the easy case by forming the current context's constructor
    // name and adding that name first. This is a very useful optimization to
    // avoid walking the lexical declarations in many cases, and it also
    // handles the only case where a constructor name can come from some other
    // lexical context -- when that name is an implicit constructor merged from
    // another declaration in the redecl chain. Any non-implicit constructor or
    // conversion function which doesn't occur in all the lexical contexts
    // would be an ODR violation.
    auto ImplicitCtorName = Context->DeclarationNames.getCXXConstructorName(
        Context->getCanonicalType(Context->getRecordType(D)));
    if (ConstructorNameSet.erase(ImplicitCtorName))
      Names.push_back(ImplicitCtorName);

    // If we still have constructors or conversion functions, we walk all the
    // names in the decl and add the constructors and conversion functions
    // which are visible in the order they lexically occur within the context.
    if (!ConstructorNameSet.empty() || !ConversionNameSet.empty())
      for (Decl *ChildD : cast<CXXRecordDecl>(DC)->decls())
        if (auto *ChildND = dyn_cast<NamedDecl>(ChildD)) {
          auto Name = ChildND->getDeclName();
          switch (Name.getNameKind()) {
          default:
            continue;

          case DeclarationName::CXXConstructorName:
            if (ConstructorNameSet.erase(Name))
              Names.push_back(Name);
            break;

          case DeclarationName::CXXConversionFunctionName:
            if (ConversionNameSet.erase(Name))
              Names.push_back(Name);
            break;
          }

          if (ConstructorNameSet.empty() && ConversionNameSet.empty())
            break;
        }

    assert(ConstructorNameSet.empty() && "Failed to find all of the visible "
                                         "constructors by walking all the "
                                         "lexical members of the context.");
    assert(ConversionNameSet.empty() && "Failed to find all of the visible "
                                        "conversion functions by walking all "
                                        "the lexical members of the context.");
  }

  // Next we need to do a lookup with each name into this decl context to fully
  // populate any results from external sources. We don't actually use the
  // results of these lookups because we only want to use the results after all
  // results have been loaded and the pointers into them will be stable.
  for (auto &Name : Names)
    DC->lookup(Name);

  // Now we need to insert the results for each name into the hash table. For
  // constructor names and conversion function names, we actually need to merge
  // all of the results for them into one list of results each and insert
  // those.
  SmallVector<NamedDecl *, 8> ConstructorDecls;
  SmallVector<NamedDecl *, 8> ConversionDecls;

  // Now loop over the names, either inserting them or appending for the two
  // special cases.
  for (auto &Name : Names) {
    DeclContext::lookup_result Result = DC->noload_lookup(Name);

    switch (Name.getNameKind()) {
    default:
      Generator.insert(Name, Result, Trait);
      break;

    case DeclarationName::CXXConstructorName:
      ConstructorDecls.append(Result.begin(), Result.end());
      break;

    case DeclarationName::CXXConversionFunctionName:
      ConversionDecls.append(Result.begin(), Result.end());
      break;
    }
  }

  // Handle our two special cases if we ended up having any. We arbitrarily use
  // the first declaration's name here because the name itself isn't part of
  // the key, only the kind of name is used.
  if (!ConstructorDecls.empty())
    Generator.insert(ConstructorDecls.front()->getDeclName(),
                     DeclContext::lookup_result(ConstructorDecls), Trait);
  if (!ConversionDecls.empty())
    Generator.insert(ConversionDecls.front()->getDeclName(),
                     DeclContext::lookup_result(ConversionDecls), Trait);

  // Create the on-disk hash table in a buffer.
  llvm::raw_svector_ostream Out(LookupTable);
  // Make sure that no bucket is at offset 0
  using namespace llvm::support;
  endian::Writer<little>(Out).write<uint32_t>(0);
  return Generator.Emit(Out, Trait);
}

/// \brief Write the block containing all of the declaration IDs
/// visible from the given DeclContext.
///
/// \returns the offset of the DECL_CONTEXT_VISIBLE block within the
/// bitstream, or 0 if no block was written.
uint64_t ASTWriter::WriteDeclContextVisibleBlock(ASTContext &Context,
                                                 DeclContext *DC) {
  // If we imported a key declaration of this namespace, write the visible
  // lookup results as an update record for it rather than including them
  // on this declaration. We will only look at key declarations on reload.
  if (isa<NamespaceDecl>(DC) && Chain &&
      Chain->getKeyDeclaration(cast<Decl>(DC))->isFromASTFile()) {
    // Only do this once, for the first local declaration of the namespace.
    for (NamespaceDecl *Prev = cast<NamespaceDecl>(DC)->getPreviousDecl(); Prev;
         Prev = Prev->getPreviousDecl())
      if (!Prev->isFromASTFile())
        return 0;

    // Note that we need to emit an update record for the primary context.
    UpdatedDeclContexts.insert(DC->getPrimaryContext());

    // Make sure all visible decls are written. They will be recorded later. We
    // do this using a side data structure so we can sort the names into
    // a deterministic order.
    StoredDeclsMap *Map = DC->getPrimaryContext()->buildLookup();
    SmallVector<std::pair<DeclarationName, DeclContext::lookup_result>, 16>
        LookupResults;
    if (Map) {
      LookupResults.reserve(Map->size());
      for (auto &Entry : *Map)
        LookupResults.push_back(
            std::make_pair(Entry.first, Entry.second.getLookupResult()));
    }

    std::sort(LookupResults.begin(), LookupResults.end(), llvm::less_first());
    for (auto &NameAndResult : LookupResults) {
      DeclarationName Name = NameAndResult.first;
      DeclContext::lookup_result Result = NameAndResult.second;
      if (Name.getNameKind() == DeclarationName::CXXConstructorName ||
          Name.getNameKind() == DeclarationName::CXXConversionFunctionName) {
        // We have to work around a name lookup bug here where negative lookup
        // results for these names get cached in namespace lookup tables (these
        // names should never be looked up in a namespace).
        assert(Result.empty() && "Cannot have a constructor or conversion "
                                 "function name in a namespace!");
        continue;
      }

      for (NamedDecl *ND : Result)
        if (!ND->isFromASTFile())
          GetDeclRef(ND);
    }

    return 0;
  }

  if (DC->getPrimaryContext() != DC)
    return 0;

  // Skip contexts which don't support name lookup.
  if (!DC->isLookupContext())
    return 0;

  // If not in C++, we perform name lookup for the translation unit via the
  // IdentifierInfo chains, don't bother to build a visible-declarations table.
  if (DC->isTranslationUnit() && !Context.getLangOpts().CPlusPlus)
    return 0;

  // Serialize the contents of the mapping used for lookup. Note that,
  // although we have two very different code paths, the serialized
  // representation is the same for both cases: a declaration name,
  // followed by a size, followed by references to the visible
  // declarations that have that name.
  uint64_t Offset = Stream.GetCurrentBitNo();
  StoredDeclsMap *Map = DC->buildLookup();
  if (!Map || Map->empty())
    return 0;

  // Create the on-disk hash table in a buffer.
  SmallString<4096> LookupTable;
  uint32_t BucketOffset = GenerateNameLookupTable(DC, LookupTable);

  // Write the lookup table
  RecordData Record;
  Record.push_back(DECL_CONTEXT_VISIBLE);
  Record.push_back(BucketOffset);
  Stream.EmitRecordWithBlob(DeclContextVisibleLookupAbbrev, Record,
                            LookupTable);
  ++NumVisibleDeclContexts;
  return Offset;
}

/// \brief Write an UPDATE_VISIBLE block for the given context.
///
/// UPDATE_VISIBLE blocks contain the declarations that are added to an existing
/// DeclContext in a dependent AST file. As such, they only exist for the TU
/// (in C++), for namespaces, and for classes with forward-declared unscoped
/// enumeration members (in C++11).
void ASTWriter::WriteDeclContextVisibleUpdate(const DeclContext *DC) {
  StoredDeclsMap *Map = DC->getLookupPtr();
  if (!Map || Map->empty())
    return;

  // Create the on-disk hash table in a buffer.
  SmallString<4096> LookupTable;
  uint32_t BucketOffset = GenerateNameLookupTable(DC, LookupTable);

  // If we're updating a namespace, select a key declaration as the key for the
  // update record; those are the only ones that will be checked on reload.
  if (isa<NamespaceDecl>(DC))
    DC = cast<DeclContext>(Chain->getKeyDeclaration(cast<Decl>(DC)));

  // Write the lookup table
  RecordData Record;
  Record.push_back(UPDATE_VISIBLE);
  Record.push_back(getDeclID(cast<Decl>(DC)));
  Record.push_back(BucketOffset);
  Stream.EmitRecordWithBlob(UpdateVisibleAbbrev, Record, LookupTable);
}

/// \brief Write an FP_PRAGMA_OPTIONS block for the given FPOptions.
void ASTWriter::WriteFPPragmaOptions(const FPOptions &Opts) {
  RecordData Record;
  Record.push_back(Opts.fp_contract);
  Stream.EmitRecord(FP_PRAGMA_OPTIONS, Record);
}

/// \brief Write an OPENCL_EXTENSIONS block for the given OpenCLOptions.
void ASTWriter::WriteOpenCLExtensions(Sema &SemaRef) {
  if (!SemaRef.Context.getLangOpts().OpenCL)
    return;

  const OpenCLOptions &Opts = SemaRef.getOpenCLOptions();
  RecordData Record;
#define OPENCLEXT(nm)  Record.push_back(Opts.nm);
#include "clang/Basic/OpenCLExtensions.def"
  Stream.EmitRecord(OPENCL_EXTENSIONS, Record);
}

void ASTWriter::WriteRedeclarations() {
  RecordData LocalRedeclChains;
  SmallVector<serialization::LocalRedeclarationsInfo, 2> LocalRedeclsMap;

  for (unsigned I = 0, N = Redeclarations.size(); I != N; ++I) {
    const Decl *Key = Redeclarations[I];
    assert((Chain ? Chain->getKeyDeclaration(Key) == Key
                  : Key->isFirstDecl()) &&
           "not the key declaration");

    const Decl *First = Key->getCanonicalDecl();
    const Decl *MostRecent = First->getMostRecentDecl();

    assert((getDeclID(First) >= NUM_PREDEF_DECL_IDS || First == Key) &&
           "should not have imported key decls for predefined decl");

    // If we only have a single declaration, there is no point in storing
    // a redeclaration chain.
    if (First == MostRecent)
      continue;
    
    unsigned Offset = LocalRedeclChains.size();
    unsigned Size = 0;
    LocalRedeclChains.push_back(0); // Placeholder for the size.

    // Collect the set of local redeclarations of this declaration.
    for (const Decl *Prev = MostRecent; Prev;
         Prev = Prev->getPreviousDecl()) { 
      if (!Prev->isFromASTFile() && Prev != Key) {
        AddDeclRef(Prev, LocalRedeclChains);
        ++Size;
      }
    }

    LocalRedeclChains[Offset] = Size;

    // Reverse the set of local redeclarations, so that we store them in
    // order (since we found them in reverse order).
    std::reverse(LocalRedeclChains.end() - Size, LocalRedeclChains.end());

    // Add the mapping from the first ID from the AST to the set of local
    // declarations.
    LocalRedeclarationsInfo Info = { getDeclID(Key), Offset };
    LocalRedeclsMap.push_back(Info);

    assert(N == Redeclarations.size() && 
           "Deserialized a declaration we shouldn't have");
  }

  if (LocalRedeclChains.empty())
    return;

  // Sort the local redeclarations map by the first declaration ID,
  // since the reader will be performing binary searches on this information.
  llvm::array_pod_sort(LocalRedeclsMap.begin(), LocalRedeclsMap.end());
  
  // Emit the local redeclarations map.
  using namespace llvm;
  llvm::BitCodeAbbrev *Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(LOCAL_REDECLARATIONS_MAP));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6)); // # of entries
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));
  unsigned AbbrevID = Stream.EmitAbbrev(Abbrev);
  
  RecordData Record;
  Record.push_back(LOCAL_REDECLARATIONS_MAP);
  Record.push_back(LocalRedeclsMap.size());
  Stream.EmitRecordWithBlob(AbbrevID, Record, 
    reinterpret_cast<char*>(LocalRedeclsMap.data()),
    LocalRedeclsMap.size() * sizeof(LocalRedeclarationsInfo));

  // Emit the redeclaration chains.
  Stream.EmitRecord(LOCAL_REDECLARATIONS, LocalRedeclChains);
}

void ASTWriter::WriteObjCCategories() {
  SmallVector<ObjCCategoriesInfo, 2> CategoriesMap;
  RecordData Categories;
  
  for (unsigned I = 0, N = ObjCClassesWithCategories.size(); I != N; ++I) {
    unsigned Size = 0;
    unsigned StartIndex = Categories.size();
    
    ObjCInterfaceDecl *Class = ObjCClassesWithCategories[I];
    
    // Allocate space for the size.
    Categories.push_back(0);
    
    // Add the categories.
    for (ObjCInterfaceDecl::known_categories_iterator
           Cat = Class->known_categories_begin(),
           CatEnd = Class->known_categories_end();
         Cat != CatEnd; ++Cat, ++Size) {
      assert(getDeclID(*Cat) != 0 && "Bogus category");
      AddDeclRef(*Cat, Categories);
    }
    
    // Update the size.
    Categories[StartIndex] = Size;
    
    // Record this interface -> category map.
    ObjCCategoriesInfo CatInfo = { getDeclID(Class), StartIndex };
    CategoriesMap.push_back(CatInfo);
  }

  // Sort the categories map by the definition ID, since the reader will be
  // performing binary searches on this information.
  llvm::array_pod_sort(CategoriesMap.begin(), CategoriesMap.end());

  // Emit the categories map.
  using namespace llvm;
  llvm::BitCodeAbbrev *Abbrev = new BitCodeAbbrev();
  Abbrev->Add(BitCodeAbbrevOp(OBJC_CATEGORIES_MAP));
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::VBR, 6)); // # of entries
  Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));
  unsigned AbbrevID = Stream.EmitAbbrev(Abbrev);
  
  RecordData Record;
  Record.push_back(OBJC_CATEGORIES_MAP);
  Record.push_back(CategoriesMap.size());
  Stream.EmitRecordWithBlob(AbbrevID, Record, 
                            reinterpret_cast<char*>(CategoriesMap.data()),
                            CategoriesMap.size() * sizeof(ObjCCategoriesInfo));
  
  // Emit the category lists.
  Stream.EmitRecord(OBJC_CATEGORIES, Categories);
}

void ASTWriter::WriteLateParsedTemplates(Sema &SemaRef) {
  Sema::LateParsedTemplateMapT &LPTMap = SemaRef.LateParsedTemplateMap;

  if (LPTMap.empty())
    return;

  RecordData Record;
  for (auto LPTMapEntry : LPTMap) {
    const FunctionDecl *FD = LPTMapEntry.first;
    LateParsedTemplate *LPT = LPTMapEntry.second;
    AddDeclRef(FD, Record);
    AddDeclRef(LPT->D, Record);
    Record.push_back(LPT->Toks.size());

    for (CachedTokens::iterator TokIt = LPT->Toks.begin(),
                                TokEnd = LPT->Toks.end();
         TokIt != TokEnd; ++TokIt) {
      AddToken(*TokIt, Record);
    }
  }
  Stream.EmitRecord(LATE_PARSED_TEMPLATE, Record);
}

/// \brief Write the state of 'pragma clang optimize' at the end of the module.
void ASTWriter::WriteOptimizePragmaOptions(Sema &SemaRef) {
  RecordData Record;
  SourceLocation PragmaLoc = SemaRef.getOptimizeOffPragmaLocation();
  AddSourceLocation(PragmaLoc, Record);
  Stream.EmitRecord(OPTIMIZE_PRAGMA_OPTIONS, Record);
}

//===----------------------------------------------------------------------===//
// General Serialization Routines
//===----------------------------------------------------------------------===//

/// \brief Write a record containing the given attributes.
void ASTWriter::WriteAttributes(ArrayRef<const Attr*> Attrs,
                                RecordDataImpl &Record) {
  Record.push_back(Attrs.size());
  for (ArrayRef<const Attr *>::iterator i = Attrs.begin(),
                                        e = Attrs.end(); i != e; ++i){
    const Attr *A = *i;
    Record.push_back(A->getKind()); // FIXME: stable encoding, target attrs
    AddSourceRange(A->getRange(), Record);

#include "clang/Serialization/AttrPCHWrite.inc"

  }
}

void ASTWriter::AddToken(const Token &Tok, RecordDataImpl &Record) {
  AddSourceLocation(Tok.getLocation(), Record);
  Record.push_back(Tok.getLength());

  // FIXME: When reading literal tokens, reconstruct the literal pointer
  // if it is needed.
  AddIdentifierRef(Tok.getIdentifierInfo(), Record);
  // FIXME: Should translate token kind to a stable encoding.
  Record.push_back(Tok.getKind());
  // FIXME: Should translate token flags to a stable encoding.
  Record.push_back(Tok.getFlags());
}

void ASTWriter::AddString(StringRef Str, RecordDataImpl &Record) {
  Record.push_back(Str.size());
  Record.insert(Record.end(), Str.begin(), Str.end());
}

bool ASTWriter::PreparePathForOutput(SmallVectorImpl<char> &Path) {
  assert(Context && "should have context when outputting path");

  bool Changed =
      cleanPathForOutput(Context->getSourceManager().getFileManager(), Path);

  // Remove a prefix to make the path relative, if relevant.
  const char *PathBegin = Path.data();
  const char *PathPtr =
      adjustFilenameForRelocatableAST(PathBegin, BaseDirectory);
  if (PathPtr != PathBegin) {
    Path.erase(Path.begin(), Path.begin() + (PathPtr - PathBegin));
    Changed = true;
  }

  return Changed;
}

void ASTWriter::AddPath(StringRef Path, RecordDataImpl &Record) {
  SmallString<128> FilePath(Path);
  PreparePathForOutput(FilePath);
  AddString(FilePath, Record);
}

void ASTWriter::EmitRecordWithPath(unsigned Abbrev, RecordDataImpl &Record,
                                   StringRef Path) {
  SmallString<128> FilePath(Path);
  PreparePathForOutput(FilePath);
  Stream.EmitRecordWithBlob(Abbrev, Record, FilePath);
}

void ASTWriter::AddVersionTuple(const VersionTuple &Version,
                                RecordDataImpl &Record) {
  Record.push_back(Version.getMajor());
  if (Optional<unsigned> Minor = Version.getMinor())
    Record.push_back(*Minor + 1);
  else
    Record.push_back(0);
  if (Optional<unsigned> Subminor = Version.getSubminor())
    Record.push_back(*Subminor + 1);
  else
    Record.push_back(0);
}

/// \brief Note that the identifier II occurs at the given offset
/// within the identifier table.
void ASTWriter::SetIdentifierOffset(const IdentifierInfo *II, uint32_t Offset) {
  IdentID ID = IdentifierIDs[II];
  // Only store offsets new to this AST file. Other identifier names are looked
  // up earlier in the chain and thus don't need an offset.
  if (ID >= FirstIdentID)
    IdentifierOffsets[ID - FirstIdentID] = Offset;
}

/// \brief Note that the selector Sel occurs at the given offset
/// within the method pool/selector table.
void ASTWriter::SetSelectorOffset(Selector Sel, uint32_t Offset) {
  unsigned ID = SelectorIDs[Sel];
  assert(ID && "Unknown selector");
  // Don't record offsets for selectors that are also available in a different
  // file.
  if (ID < FirstSelectorID)
    return;
  SelectorOffsets[ID - FirstSelectorID] = Offset;
}

ASTWriter::ASTWriter(llvm::BitstreamWriter &Stream)
    : Stream(Stream), Context(nullptr), PP(nullptr), Chain(nullptr),
      WritingModule(nullptr), WritingAST(false),
      DoneWritingDeclsAndTypes(false), ASTHasCompilerErrors(false),
      FirstDeclID(NUM_PREDEF_DECL_IDS), NextDeclID(FirstDeclID),
      FirstTypeID(NUM_PREDEF_TYPE_IDS), NextTypeID(FirstTypeID),
      FirstIdentID(NUM_PREDEF_IDENT_IDS), NextIdentID(FirstIdentID),
      FirstMacroID(NUM_PREDEF_MACRO_IDS), NextMacroID(FirstMacroID),
      FirstSubmoduleID(NUM_PREDEF_SUBMODULE_IDS),
      NextSubmoduleID(FirstSubmoduleID),
      FirstSelectorID(NUM_PREDEF_SELECTOR_IDS), NextSelectorID(FirstSelectorID),
      CollectedStmts(&StmtsToEmit), NumStatements(0), NumMacros(0),
      NumLexicalDeclContexts(0), NumVisibleDeclContexts(0),
      NextCXXBaseSpecifiersID(1), NextCXXCtorInitializersID(1),
      TypeExtQualAbbrev(0),
      TypeFunctionProtoAbbrev(0), DeclParmVarAbbrev(0),
      DeclContextLexicalAbbrev(0), DeclContextVisibleLookupAbbrev(0),
      UpdateVisibleAbbrev(0), DeclRecordAbbrev(0), DeclTypedefAbbrev(0),
      DeclVarAbbrev(0), DeclFieldAbbrev(0), DeclEnumAbbrev(0),
      DeclObjCIvarAbbrev(0), DeclCXXMethodAbbrev(0), DeclRefExprAbbrev(0),
      CharacterLiteralAbbrev(0), IntegerLiteralAbbrev(0),
      ExprImplicitCastAbbrev(0) {}

ASTWriter::~ASTWriter() {
  llvm::DeleteContainerSeconds(FileDeclIDs);
}

const LangOptions &ASTWriter::getLangOpts() const {
  assert(WritingAST && "can't determine lang opts when not writing AST");
  return Context->getLangOpts();
}

void ASTWriter::WriteAST(Sema &SemaRef,
                         const std::string &OutputFile,
                         Module *WritingModule, StringRef isysroot,
                         bool hasErrors) {
  WritingAST = true;
  
  ASTHasCompilerErrors = hasErrors;
  
  // Emit the file header.
  Stream.Emit((unsigned)'C', 8);
  Stream.Emit((unsigned)'P', 8);
  Stream.Emit((unsigned)'C', 8);
  Stream.Emit((unsigned)'H', 8);

  WriteBlockInfoBlock();

  Context = &SemaRef.Context;
  PP = &SemaRef.PP;
  this->WritingModule = WritingModule;
  WriteASTCore(SemaRef, isysroot, OutputFile, WritingModule);
  Context = nullptr;
  PP = nullptr;
  this->WritingModule = nullptr;
  this->BaseDirectory.clear();

  WritingAST = false;
}

template<typename Vector>
static void AddLazyVectorDecls(ASTWriter &Writer, Vector &Vec,
                               ASTWriter::RecordData &Record) {
  for (typename Vector::iterator I = Vec.begin(nullptr, true), E = Vec.end();
       I != E; ++I) {
    Writer.AddDeclRef(*I, Record);
  }
}

void ASTWriter::WriteASTCore(Sema &SemaRef,
                             StringRef isysroot,
                             const std::string &OutputFile, 
                             Module *WritingModule) {
  using namespace llvm;

  bool isModule = WritingModule != nullptr;

  // Make sure that the AST reader knows to finalize itself.
  if (Chain)
    Chain->finalizeForWriting();
  
  ASTContext &Context = SemaRef.Context;
  Preprocessor &PP = SemaRef.PP;

  // Set up predefined declaration IDs.
  auto RegisterPredefDecl = [&] (Decl *D, PredefinedDeclIDs ID) {
    if (D) {
      assert(D->isCanonicalDecl() && "predefined decl is not canonical");
      DeclIDs[D] = ID;
      if (D->getMostRecentDecl() != D)
        Redeclarations.push_back(D);
    }
  };
  RegisterPredefDecl(Context.getTranslationUnitDecl(),
                     PREDEF_DECL_TRANSLATION_UNIT_ID);
  RegisterPredefDecl(Context.ObjCIdDecl, PREDEF_DECL_OBJC_ID_ID);
  RegisterPredefDecl(Context.ObjCSelDecl, PREDEF_DECL_OBJC_SEL_ID);
  RegisterPredefDecl(Context.ObjCClassDecl, PREDEF_DECL_OBJC_CLASS_ID);
  RegisterPredefDecl(Context.ObjCProtocolClassDecl,
                     PREDEF_DECL_OBJC_PROTOCOL_ID);
  RegisterPredefDecl(Context.Int128Decl, PREDEF_DECL_INT_128_ID);
  RegisterPredefDecl(Context.UInt128Decl, PREDEF_DECL_UNSIGNED_INT_128_ID);
  RegisterPredefDecl(Context.ObjCInstanceTypeDecl,
                     PREDEF_DECL_OBJC_INSTANCETYPE_ID);
  RegisterPredefDecl(Context.BuiltinVaListDecl, PREDEF_DECL_BUILTIN_VA_LIST_ID);
  RegisterPredefDecl(Context.ExternCContext, PREDEF_DECL_EXTERN_C_CONTEXT_ID);

  // Build a record containing all of the tentative definitions in this file, in
  // TentativeDefinitions order.  Generally, this record will be empty for
  // headers.
  RecordData TentativeDefinitions;
  AddLazyVectorDecls(*this, SemaRef.TentativeDefinitions, TentativeDefinitions);
  
  // Build a record containing all of the file scoped decls in this file.
  RecordData UnusedFileScopedDecls;
  if (!isModule)
    AddLazyVectorDecls(*this, SemaRef.UnusedFileScopedDecls,
                       UnusedFileScopedDecls);

  // Build a record containing all of the delegating constructors we still need
  // to resolve.
  RecordData DelegatingCtorDecls;
  if (!isModule)
    AddLazyVectorDecls(*this, SemaRef.DelegatingCtorDecls, DelegatingCtorDecls);

  // Write the set of weak, undeclared identifiers. We always write the
  // entire table, since later PCH files in a PCH chain are only interested in
  // the results at the end of the chain.
  RecordData WeakUndeclaredIdentifiers;
  for (auto &WeakUndeclaredIdentifier : SemaRef.WeakUndeclaredIdentifiers) {
    IdentifierInfo *II = WeakUndeclaredIdentifier.first;
    WeakInfo &WI = WeakUndeclaredIdentifier.second;
    AddIdentifierRef(II, WeakUndeclaredIdentifiers);
    AddIdentifierRef(WI.getAlias(), WeakUndeclaredIdentifiers);
    AddSourceLocation(WI.getLocation(), WeakUndeclaredIdentifiers);
    WeakUndeclaredIdentifiers.push_back(WI.getUsed());
  }

  // Build a record containing all of the ext_vector declarations.
  RecordData ExtVectorDecls;
  AddLazyVectorDecls(*this, SemaRef.ExtVectorDecls, ExtVectorDecls);

  // Build a record containing all of the VTable uses information.
  RecordData VTableUses;
  if (!SemaRef.VTableUses.empty()) {
    for (unsigned I = 0, N = SemaRef.VTableUses.size(); I != N; ++I) {
      AddDeclRef(SemaRef.VTableUses[I].first, VTableUses);
      AddSourceLocation(SemaRef.VTableUses[I].second, VTableUses);
      VTableUses.push_back(SemaRef.VTablesUsed[SemaRef.VTableUses[I].first]);
    }
  }

  // Build a record containing all of the UnusedLocalTypedefNameCandidates.
  RecordData UnusedLocalTypedefNameCandidates;
  for (const TypedefNameDecl *TD : SemaRef.UnusedLocalTypedefNameCandidates)
    AddDeclRef(TD, UnusedLocalTypedefNameCandidates);

  // Build a record containing all of pending implicit instantiations.
  RecordData PendingInstantiations;
  for (std::deque<Sema::PendingImplicitInstantiation>::iterator
         I = SemaRef.PendingInstantiations.begin(),
         N = SemaRef.PendingInstantiations.end(); I != N; ++I) {
    AddDeclRef(I->first, PendingInstantiations);
    AddSourceLocation(I->second, PendingInstantiations);
  }
  assert(SemaRef.PendingLocalImplicitInstantiations.empty() &&
         "There are local ones at end of translation unit!");

  // Build a record containing some declaration references.
  RecordData SemaDeclRefs;
  if (SemaRef.StdNamespace || SemaRef.StdBadAlloc) {
    AddDeclRef(SemaRef.getStdNamespace(), SemaDeclRefs);
    AddDeclRef(SemaRef.getStdBadAlloc(), SemaDeclRefs);
  }

  RecordData CUDASpecialDeclRefs;
  if (Context.getcudaConfigureCallDecl()) {
    AddDeclRef(Context.getcudaConfigureCallDecl(), CUDASpecialDeclRefs);
  }

  // Build a record containing all of the known namespaces.
  RecordData KnownNamespaces;
  for (llvm::MapVector<NamespaceDecl*, bool>::iterator
            I = SemaRef.KnownNamespaces.begin(),
         IEnd = SemaRef.KnownNamespaces.end();
       I != IEnd; ++I) {
    if (!I->second)
      AddDeclRef(I->first, KnownNamespaces);
  }

  // Build a record of all used, undefined objects that require definitions.
  RecordData UndefinedButUsed;

  SmallVector<std::pair<NamedDecl *, SourceLocation>, 16> Undefined;
  SemaRef.getUndefinedButUsed(Undefined);
  for (SmallVectorImpl<std::pair<NamedDecl *, SourceLocation> >::iterator
         I = Undefined.begin(), E = Undefined.end(); I != E; ++I) {
    AddDeclRef(I->first, UndefinedButUsed);
    AddSourceLocation(I->second, UndefinedButUsed);
  }

  // Build a record containing all delete-expressions that we would like to
  // analyze later in AST.
  RecordData DeleteExprsToAnalyze;

  for (const auto &DeleteExprsInfo :
       SemaRef.getMismatchingDeleteExpressions()) {
    AddDeclRef(DeleteExprsInfo.first, DeleteExprsToAnalyze);
    DeleteExprsToAnalyze.push_back(DeleteExprsInfo.second.size());
    for (const auto &DeleteLoc : DeleteExprsInfo.second) {
      AddSourceLocation(DeleteLoc.first, DeleteExprsToAnalyze);
      DeleteExprsToAnalyze.push_back(DeleteLoc.second);
    }
  }

  // Write the control block
  WriteControlBlock(PP, Context, isysroot, OutputFile);

  // Write the remaining AST contents.
  RecordData Record;
  Stream.EnterSubblock(AST_BLOCK_ID, 5);

  // This is so that older clang versions, before the introduction
  // of the control block, can read and reject the newer PCH format.
  Record.clear();
  Record.push_back(VERSION_MAJOR);
  Stream.EmitRecord(METADATA_OLD_FORMAT, Record);

  // Create a lexical update block containing all of the declarations in the
  // translation unit that do not come from other AST files.
  const TranslationUnitDecl *TU = Context.getTranslationUnitDecl();
  SmallVector<KindDeclIDPair, 64> NewGlobalDecls;
  for (const auto *I : TU->noload_decls()) {
    if (!I->isFromASTFile())
      NewGlobalDecls.push_back(std::make_pair(I->getKind(), GetDeclRef(I)));
  }
  
  llvm::BitCodeAbbrev *Abv = new llvm::BitCodeAbbrev();
  Abv->Add(llvm::BitCodeAbbrevOp(TU_UPDATE_LEXICAL));
  Abv->Add(llvm::BitCodeAbbrevOp(llvm::BitCodeAbbrevOp::Blob));
  unsigned TuUpdateLexicalAbbrev = Stream.EmitAbbrev(Abv);
  Record.clear();
  Record.push_back(TU_UPDATE_LEXICAL);
  Stream.EmitRecordWithBlob(TuUpdateLexicalAbbrev, Record,
                            bytes(NewGlobalDecls));
  
  // And a visible updates block for the translation unit.
  Abv = new llvm::BitCodeAbbrev();
  Abv->Add(llvm::BitCodeAbbrevOp(UPDATE_VISIBLE));
  Abv->Add(llvm::BitCodeAbbrevOp(llvm::BitCodeAbbrevOp::VBR, 6));
  Abv->Add(llvm::BitCodeAbbrevOp(llvm::BitCodeAbbrevOp::Fixed, 32));
  Abv->Add(llvm::BitCodeAbbrevOp(llvm::BitCodeAbbrevOp::Blob));
  UpdateVisibleAbbrev = Stream.EmitAbbrev(Abv);
  WriteDeclContextVisibleUpdate(TU);

  // If we have any extern "C" names, write out a visible update for them.
  if (Context.ExternCContext)
    WriteDeclContextVisibleUpdate(Context.ExternCContext);
  
  // If the translation unit has an anonymous namespace, and we don't already
  // have an update block for it, write it as an update block.
  // FIXME: Why do we not do this if there's already an update block?
  if (NamespaceDecl *NS = TU->getAnonymousNamespace()) {
    ASTWriter::UpdateRecord &Record = DeclUpdates[TU];
    if (Record.empty())
      Record.push_back(DeclUpdate(UPD_CXX_ADDED_ANONYMOUS_NAMESPACE, NS));
  }

  // Add update records for all mangling numbers and static local numbers.
  // These aren't really update records, but this is a convenient way of
  // tagging this rare extra data onto the declarations.
  for (const auto &Number : Context.MangleNumbers)
    if (!Number.first->isFromASTFile())
      DeclUpdates[Number.first].push_back(DeclUpdate(UPD_MANGLING_NUMBER,
                                                     Number.second));
  for (const auto &Number : Context.StaticLocalNumbers)
    if (!Number.first->isFromASTFile())
      DeclUpdates[Number.first].push_back(DeclUpdate(UPD_STATIC_LOCAL_NUMBER,
                                                     Number.second));

  // Make sure visible decls, added to DeclContexts previously loaded from
  // an AST file, are registered for serialization.
  for (SmallVectorImpl<const Decl *>::iterator
         I = UpdatingVisibleDecls.begin(),
         E = UpdatingVisibleDecls.end(); I != E; ++I) {
    GetDeclRef(*I);
  }

  // Make sure all decls associated with an identifier are registered for
  // serialization.
  llvm::SmallVector<const IdentifierInfo*, 256> IIs;
  for (IdentifierTable::iterator ID = PP.getIdentifierTable().begin(),
                              IDEnd = PP.getIdentifierTable().end();
       ID != IDEnd; ++ID) {
    const IdentifierInfo *II = ID->second;
    if (!Chain || !II->isFromAST() || II->hasChangedSinceDeserialization())
      IIs.push_back(II);
  }
  // Sort the identifiers to visit based on their name.
  std::sort(IIs.begin(), IIs.end(), llvm::less_ptr<IdentifierInfo>());
  for (const IdentifierInfo *II : IIs) {
    for (IdentifierResolver::iterator D = SemaRef.IdResolver.begin(II),
                                   DEnd = SemaRef.IdResolver.end();
         D != DEnd; ++D) {
      GetDeclRef(*D);
    }
  }

  // Form the record of special types.
  RecordData SpecialTypes;
  AddTypeRef(Context.getRawCFConstantStringType(), SpecialTypes);
  AddTypeRef(Context.getFILEType(), SpecialTypes);
  AddTypeRef(Context.getjmp_bufType(), SpecialTypes);
  AddTypeRef(Context.getsigjmp_bufType(), SpecialTypes);
  AddTypeRef(Context.ObjCIdRedefinitionType, SpecialTypes);
  AddTypeRef(Context.ObjCClassRedefinitionType, SpecialTypes);
  AddTypeRef(Context.ObjCSelRedefinitionType, SpecialTypes);
  AddTypeRef(Context.getucontext_tType(), SpecialTypes);

  if (Chain) {
    // Write the mapping information describing our module dependencies and how
    // each of those modules were mapped into our own offset/ID space, so that
    // the reader can build the appropriate mapping to its own offset/ID space.
    // The map consists solely of a blob with the following format:
    // *(module-name-len:i16 module-name:len*i8
    //   source-location-offset:i32
    //   identifier-id:i32
    //   preprocessed-entity-id:i32
    //   macro-definition-id:i32
    //   submodule-id:i32
    //   selector-id:i32
    //   declaration-id:i32
    //   c++-base-specifiers-id:i32
    //   type-id:i32)
    // 
    llvm::BitCodeAbbrev *Abbrev = new BitCodeAbbrev();
    Abbrev->Add(BitCodeAbbrevOp(MODULE_OFFSET_MAP));
    Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));
    unsigned ModuleOffsetMapAbbrev = Stream.EmitAbbrev(Abbrev);
    SmallString<2048> Buffer;
    {
      llvm::raw_svector_ostream Out(Buffer);
      for (ModuleFile *M : Chain->ModuleMgr) {
        using namespace llvm::support;
        endian::Writer<little> LE(Out);
        StringRef FileName = M->FileName;
        LE.write<uint16_t>(FileName.size());
        Out.write(FileName.data(), FileName.size());

        // Note: if a base ID was uint max, it would not be possible to load
        // another module after it or have more than one entity inside it.
        uint32_t None = std::numeric_limits<uint32_t>::max();

        auto writeBaseIDOrNone = [&](uint32_t BaseID, bool ShouldWrite) {
          assert(BaseID < std::numeric_limits<uint32_t>::max() && "base id too high");
          if (ShouldWrite)
            LE.write<uint32_t>(BaseID);
          else
            LE.write<uint32_t>(None);
        };

        // These values should be unique within a chain, since they will be read
        // as keys into ContinuousRangeMaps.
        writeBaseIDOrNone(M->SLocEntryBaseOffset, M->LocalNumSLocEntries);
        writeBaseIDOrNone(M->BaseIdentifierID, M->LocalNumIdentifiers);
        writeBaseIDOrNone(M->BaseMacroID, M->LocalNumMacros);
        writeBaseIDOrNone(M->BasePreprocessedEntityID,
                          M->NumPreprocessedEntities);
        writeBaseIDOrNone(M->BaseSubmoduleID, M->LocalNumSubmodules);
        writeBaseIDOrNone(M->BaseSelectorID, M->LocalNumSelectors);
        writeBaseIDOrNone(M->BaseDeclID, M->LocalNumDecls);
        writeBaseIDOrNone(M->BaseTypeIndex, M->LocalNumTypes);
      }
    }
    Record.clear();
    Record.push_back(MODULE_OFFSET_MAP);
    Stream.EmitRecordWithBlob(ModuleOffsetMapAbbrev, Record,
                              Buffer.data(), Buffer.size());
  }

  RecordData DeclUpdatesOffsetsRecord;

  // Keep writing types, declarations, and declaration update records
  // until we've emitted all of them.
  Stream.EnterSubblock(DECLTYPES_BLOCK_ID, /*bits for abbreviations*/5);
  WriteTypeAbbrevs();
  WriteDeclAbbrevs();
  for (DeclsToRewriteTy::iterator I = DeclsToRewrite.begin(),
                                  E = DeclsToRewrite.end();
       I != E; ++I)
    DeclTypesToEmit.push(const_cast<Decl*>(*I));
  do {
    WriteDeclUpdatesBlocks(DeclUpdatesOffsetsRecord);
    while (!DeclTypesToEmit.empty()) {
      DeclOrType DOT = DeclTypesToEmit.front();
      DeclTypesToEmit.pop();
      if (DOT.isType())
        WriteType(DOT.getType());
      else
        WriteDecl(Context, DOT.getDecl());
    }
  } while (!DeclUpdates.empty());
  Stream.ExitBlock();

  DoneWritingDeclsAndTypes = true;

  // These things can only be done once we've written out decls and types.
  WriteTypeDeclOffsets();
  if (!DeclUpdatesOffsetsRecord.empty())
    Stream.EmitRecord(DECL_UPDATE_OFFSETS, DeclUpdatesOffsetsRecord);
  WriteCXXBaseSpecifiersOffsets();
  WriteCXXCtorInitializersOffsets();
  WriteFileDeclIDsMap();
  WriteSourceManagerBlock(Context.getSourceManager(), PP);

  WriteComments();
  WritePreprocessor(PP, isModule);
  WriteHeaderSearch(PP.getHeaderSearchInfo());
  WriteSelectors(SemaRef);
  WriteReferencedSelectorsPool(SemaRef);
  WriteIdentifierTable(PP, SemaRef.IdResolver, isModule);
  WriteFPPragmaOptions(SemaRef.getFPOptions());
  WriteOpenCLExtensions(SemaRef);
  WritePragmaDiagnosticMappings(Context.getDiagnostics(), isModule);

  // If we're emitting a module, write out the submodule information.  
  if (WritingModule)
    WriteSubmodules(WritingModule);

  Stream.EmitRecord(SPECIAL_TYPES, SpecialTypes);

  // Write the record containing external, unnamed definitions.
  if (!EagerlyDeserializedDecls.empty())
    Stream.EmitRecord(EAGERLY_DESERIALIZED_DECLS, EagerlyDeserializedDecls);

  // Write the record containing tentative definitions.
  if (!TentativeDefinitions.empty())
    Stream.EmitRecord(TENTATIVE_DEFINITIONS, TentativeDefinitions);

  // Write the record containing unused file scoped decls.
  if (!UnusedFileScopedDecls.empty())
    Stream.EmitRecord(UNUSED_FILESCOPED_DECLS, UnusedFileScopedDecls);

  // Write the record containing weak undeclared identifiers.
  if (!WeakUndeclaredIdentifiers.empty())
    Stream.EmitRecord(WEAK_UNDECLARED_IDENTIFIERS,
                      WeakUndeclaredIdentifiers);

  // Write the record containing ext_vector type names.
  if (!ExtVectorDecls.empty())
    Stream.EmitRecord(EXT_VECTOR_DECLS, ExtVectorDecls);

  // Write the record containing VTable uses information.
  if (!VTableUses.empty())
    Stream.EmitRecord(VTABLE_USES, VTableUses);

  // Write the record containing potentially unused local typedefs.
  if (!UnusedLocalTypedefNameCandidates.empty())
    Stream.EmitRecord(UNUSED_LOCAL_TYPEDEF_NAME_CANDIDATES,
                      UnusedLocalTypedefNameCandidates);

  // Write the record containing pending implicit instantiations.
  if (!PendingInstantiations.empty())
    Stream.EmitRecord(PENDING_IMPLICIT_INSTANTIATIONS, PendingInstantiations);

  // Write the record containing declaration references of Sema.
  if (!SemaDeclRefs.empty())
    Stream.EmitRecord(SEMA_DECL_REFS, SemaDeclRefs);

  // Write the record containing CUDA-specific declaration references.
  if (!CUDASpecialDeclRefs.empty())
    Stream.EmitRecord(CUDA_SPECIAL_DECL_REFS, CUDASpecialDeclRefs);
  
  // Write the delegating constructors.
  if (!DelegatingCtorDecls.empty())
    Stream.EmitRecord(DELEGATING_CTORS, DelegatingCtorDecls);

  // Write the known namespaces.
  if (!KnownNamespaces.empty())
    Stream.EmitRecord(KNOWN_NAMESPACES, KnownNamespaces);

  // Write the undefined internal functions and variables, and inline functions.
  if (!UndefinedButUsed.empty())
    Stream.EmitRecord(UNDEFINED_BUT_USED, UndefinedButUsed);

  if (!DeleteExprsToAnalyze.empty())
    Stream.EmitRecord(DELETE_EXPRS_TO_ANALYZE, DeleteExprsToAnalyze);

  // Write the visible updates to DeclContexts.
  for (auto *DC : UpdatedDeclContexts)
    WriteDeclContextVisibleUpdate(DC);

  if (!WritingModule) {
    // Write the submodules that were imported, if any.
    struct ModuleInfo {
      uint64_t ID;
      Module *M;
      ModuleInfo(uint64_t ID, Module *M) : ID(ID), M(M) {}
    };
    llvm::SmallVector<ModuleInfo, 64> Imports;
    for (const auto *I : Context.local_imports()) {
      assert(SubmoduleIDs.find(I->getImportedModule()) != SubmoduleIDs.end());
      Imports.push_back(ModuleInfo(SubmoduleIDs[I->getImportedModule()],
                         I->getImportedModule()));
    }

    if (!Imports.empty()) {
      auto Cmp = [](const ModuleInfo &A, const ModuleInfo &B) {
        return A.ID < B.ID;
      };
      auto Eq = [](const ModuleInfo &A, const ModuleInfo &B) {
        return A.ID == B.ID;
      };

      // Sort and deduplicate module IDs.
      std::sort(Imports.begin(), Imports.end(), Cmp);
      Imports.erase(std::unique(Imports.begin(), Imports.end(), Eq),
                    Imports.end());

      RecordData ImportedModules;
      for (const auto &Import : Imports) {
        ImportedModules.push_back(Import.ID);
        // FIXME: If the module has macros imported then later has declarations
        // imported, this location won't be the right one as a location for the
        // declaration imports.
        AddSourceLocation(PP.getModuleImportLoc(Import.M), ImportedModules);
      }

      Stream.EmitRecord(IMPORTED_MODULES, ImportedModules);
    }
  }

  WriteDeclReplacementsBlock();
  WriteRedeclarations();
  WriteObjCCategories();
  WriteLateParsedTemplates(SemaRef);
  if(!WritingModule)
    WriteOptimizePragmaOptions(SemaRef);

  // Some simple statistics
  Record.clear();
  Record.push_back(NumStatements);
  Record.push_back(NumMacros);
  Record.push_back(NumLexicalDeclContexts);
  Record.push_back(NumVisibleDeclContexts);
  Stream.EmitRecord(STATISTICS, Record);
  Stream.ExitBlock();
}

void ASTWriter::WriteDeclUpdatesBlocks(RecordDataImpl &OffsetsRecord) {
  if (DeclUpdates.empty())
    return;

  DeclUpdateMap LocalUpdates;
  LocalUpdates.swap(DeclUpdates);

  for (auto &DeclUpdate : LocalUpdates) {
    const Decl *D = DeclUpdate.first;
    if (isRewritten(D))
      continue; // The decl will be written completely,no need to store updates.

    bool HasUpdatedBody = false;
    RecordData Record;
    for (auto &Update : DeclUpdate.second) {
      DeclUpdateKind Kind = (DeclUpdateKind)Update.getKind();

      Record.push_back(Kind);
      switch (Kind) {
      case UPD_CXX_ADDED_IMPLICIT_MEMBER:
      case UPD_CXX_ADDED_TEMPLATE_SPECIALIZATION:
      case UPD_CXX_ADDED_ANONYMOUS_NAMESPACE:
        assert(Update.getDecl() && "no decl to add?");
        Record.push_back(GetDeclRef(Update.getDecl()));
        break;

      case UPD_CXX_ADDED_FUNCTION_DEFINITION:
        // An updated body is emitted last, so that the reader doesn't need
        // to skip over the lazy body to reach statements for other records.
        Record.pop_back();
        HasUpdatedBody = true;
        break;

      case UPD_CXX_INSTANTIATED_STATIC_DATA_MEMBER:
        AddSourceLocation(Update.getLoc(), Record);
        break;

      case UPD_CXX_INSTANTIATED_CLASS_DEFINITION: {
        auto *RD = cast<CXXRecordDecl>(D);
        UpdatedDeclContexts.insert(RD->getPrimaryContext());
        AddCXXDefinitionData(RD, Record);
        Record.push_back(WriteDeclContextLexicalBlock(
            *Context, const_cast<CXXRecordDecl *>(RD)));

        // This state is sometimes updated by template instantiation, when we
        // switch from the specialization referring to the template declaration
        // to it referring to the template definition.
        if (auto *MSInfo = RD->getMemberSpecializationInfo()) {
          Record.push_back(MSInfo->getTemplateSpecializationKind());
          AddSourceLocation(MSInfo->getPointOfInstantiation(), Record);
        } else {
          auto *Spec = cast<ClassTemplateSpecializationDecl>(RD);
          Record.push_back(Spec->getTemplateSpecializationKind());
          AddSourceLocation(Spec->getPointOfInstantiation(), Record);

          // The instantiation might have been resolved to a partial
          // specialization. If so, record which one.
          auto From = Spec->getInstantiatedFrom();
          if (auto PartialSpec =
                From.dyn_cast<ClassTemplatePartialSpecializationDecl*>()) {
            Record.push_back(true);
            AddDeclRef(PartialSpec, Record);
            AddTemplateArgumentList(&Spec->getTemplateInstantiationArgs(),
                                    Record);
          } else {
            Record.push_back(false);
          }
        }
        Record.push_back(RD->getTagKind());
        AddSourceLocation(RD->getLocation(), Record);
        AddSourceLocation(RD->getLocStart(), Record);
        AddSourceLocation(RD->getRBraceLoc(), Record);

        // Instantiation may change attributes; write them all out afresh.
        Record.push_back(D->hasAttrs());
        if (Record.back())
          WriteAttributes(llvm::makeArrayRef(D->getAttrs().begin(),
                                             D->getAttrs().size()), Record);

        // FIXME: Ensure we don't get here for explicit instantiations.
        break;
      }

      case UPD_CXX_RESOLVED_DTOR_DELETE:
        AddDeclRef(Update.getDecl(), Record);
        break;

      case UPD_CXX_RESOLVED_EXCEPTION_SPEC:
        addExceptionSpec(
            *this,
            cast<FunctionDecl>(D)->getType()->castAs<FunctionProtoType>(),
            Record);
        break;

      case UPD_CXX_DEDUCED_RETURN_TYPE:
        Record.push_back(GetOrCreateTypeID(Update.getType()));
        break;

      case UPD_DECL_MARKED_USED:
        break;

      case UPD_MANGLING_NUMBER:
      case UPD_STATIC_LOCAL_NUMBER:
        Record.push_back(Update.getNumber());
        break;

      case UPD_DECL_MARKED_OPENMP_THREADPRIVATE:
        AddSourceRange(D->getAttr<OMPThreadPrivateDeclAttr>()->getRange(),
                       Record);
        break;

      case UPD_DECL_EXPORTED:
        Record.push_back(getSubmoduleID(Update.getModule()));
        break;

      case UPD_ADDED_ATTR_TO_RECORD:
        WriteAttributes(llvm::makeArrayRef(Update.getAttr()), Record);
        break;
      }
    }

    if (HasUpdatedBody) {
      const FunctionDecl *Def = cast<FunctionDecl>(D);
      Record.push_back(UPD_CXX_ADDED_FUNCTION_DEFINITION);
      Record.push_back(Def->isInlined());
      AddSourceLocation(Def->getInnerLocStart(), Record);
      AddFunctionDefinition(Def, Record);
    }

    OffsetsRecord.push_back(GetDeclRef(D));
    OffsetsRecord.push_back(Stream.GetCurrentBitNo());

    Stream.EmitRecord(DECL_UPDATES, Record);

    FlushPendingAfterDecl();
  }
}

void ASTWriter::WriteDeclReplacementsBlock() {
  if (ReplacedDecls.empty())
    return;

  RecordData Record;
  for (SmallVectorImpl<ReplacedDeclInfo>::iterator
         I = ReplacedDecls.begin(), E = ReplacedDecls.end(); I != E; ++I) {
    Record.push_back(I->ID);
    Record.push_back(I->Offset);
    Record.push_back(I->Loc);
  }
  Stream.EmitRecord(DECL_REPLACEMENTS, Record);
}

void ASTWriter::AddSourceLocation(SourceLocation Loc, RecordDataImpl &Record) {
  Record.push_back(Loc.getRawEncoding());
}

void ASTWriter::AddSourceRange(SourceRange Range, RecordDataImpl &Record) {
  AddSourceLocation(Range.getBegin(), Record);
  AddSourceLocation(Range.getEnd(), Record);
}

void ASTWriter::AddAPInt(const llvm::APInt &Value, RecordDataImpl &Record) {
  Record.push_back(Value.getBitWidth());
  const uint64_t *Words = Value.getRawData();
  Record.append(Words, Words + Value.getNumWords());
}

void ASTWriter::AddAPSInt(const llvm::APSInt &Value, RecordDataImpl &Record) {
  Record.push_back(Value.isUnsigned());
  AddAPInt(Value, Record);
}

void ASTWriter::AddAPFloat(const llvm::APFloat &Value, RecordDataImpl &Record) {
  AddAPInt(Value.bitcastToAPInt(), Record);
}

void ASTWriter::AddIdentifierRef(const IdentifierInfo *II, RecordDataImpl &Record) {
  Record.push_back(getIdentifierRef(II));
}

IdentID ASTWriter::getIdentifierRef(const IdentifierInfo *II) {
  if (!II)
    return 0;

  IdentID &ID = IdentifierIDs[II];
  if (ID == 0)
    ID = NextIdentID++;
  return ID;
}

MacroID ASTWriter::getMacroRef(MacroInfo *MI, const IdentifierInfo *Name) {
  // Don't emit builtin macros like __LINE__ to the AST file unless they
  // have been redefined by the header (in which case they are not
  // isBuiltinMacro).
  if (!MI || MI->isBuiltinMacro())
    return 0;

  MacroID &ID = MacroIDs[MI];
  if (ID == 0) {
    ID = NextMacroID++;
    MacroInfoToEmitData Info = { Name, MI, ID };
    MacroInfosToEmit.push_back(Info);
  }
  return ID;
}

MacroID ASTWriter::getMacroID(MacroInfo *MI) {
  if (!MI || MI->isBuiltinMacro())
    return 0;
  
  assert(MacroIDs.find(MI) != MacroIDs.end() && "Macro not emitted!");
  return MacroIDs[MI];
}

uint64_t ASTWriter::getMacroDirectivesOffset(const IdentifierInfo *Name) {
  return IdentMacroDirectivesOffsetMap.lookup(Name);
}

void ASTWriter::AddSelectorRef(const Selector SelRef, RecordDataImpl &Record) {
  Record.push_back(getSelectorRef(SelRef));
}

SelectorID ASTWriter::getSelectorRef(Selector Sel) {
  if (Sel.getAsOpaquePtr() == nullptr) {
    return 0;
  }

  SelectorID SID = SelectorIDs[Sel];
  if (SID == 0 && Chain) {
    // This might trigger a ReadSelector callback, which will set the ID for
    // this selector.
    Chain->LoadSelector(Sel);
    SID = SelectorIDs[Sel];
  }
  if (SID == 0) {
    SID = NextSelectorID++;
    SelectorIDs[Sel] = SID;
  }
  return SID;
}

void ASTWriter::AddCXXTemporary(const CXXTemporary *Temp, RecordDataImpl &Record) {
  AddDeclRef(Temp->getDestructor(), Record);
}

void ASTWriter::AddCXXCtorInitializersRef(ArrayRef<CXXCtorInitializer *> Inits,
                                          RecordDataImpl &Record) {
  assert(!Inits.empty() && "Empty ctor initializer sets are not recorded");
  CXXCtorInitializersToWrite.push_back(
      QueuedCXXCtorInitializers(NextCXXCtorInitializersID, Inits));
  Record.push_back(NextCXXCtorInitializersID++);
}

void ASTWriter::AddCXXBaseSpecifiersRef(CXXBaseSpecifier const *Bases,
                                        CXXBaseSpecifier const *BasesEnd,
                                        RecordDataImpl &Record) {
  assert(Bases != BasesEnd && "Empty base-specifier sets are not recorded");
  CXXBaseSpecifiersToWrite.push_back(
                                QueuedCXXBaseSpecifiers(NextCXXBaseSpecifiersID,
                                                        Bases, BasesEnd));
  Record.push_back(NextCXXBaseSpecifiersID++);
}

void ASTWriter::AddTemplateArgumentLocInfo(TemplateArgument::ArgKind Kind,
                                           const TemplateArgumentLocInfo &Arg,
                                           RecordDataImpl &Record) {
  switch (Kind) {
  case TemplateArgument::Expression:
    AddStmt(Arg.getAsExpr());
    break;
  case TemplateArgument::Type:
    AddTypeSourceInfo(Arg.getAsTypeSourceInfo(), Record);
    break;
  case TemplateArgument::Template:
    AddNestedNameSpecifierLoc(Arg.getTemplateQualifierLoc(), Record);
    AddSourceLocation(Arg.getTemplateNameLoc(), Record);
    break;
  case TemplateArgument::TemplateExpansion:
    AddNestedNameSpecifierLoc(Arg.getTemplateQualifierLoc(), Record);
    AddSourceLocation(Arg.getTemplateNameLoc(), Record);
    AddSourceLocation(Arg.getTemplateEllipsisLoc(), Record);
    break;
  case TemplateArgument::Null:
  case TemplateArgument::Integral:
  case TemplateArgument::Declaration:
  case TemplateArgument::NullPtr:
  case TemplateArgument::Pack:
    // FIXME: Is this right?
    break;
  }
}

void ASTWriter::AddTemplateArgumentLoc(const TemplateArgumentLoc &Arg,
                                       RecordDataImpl &Record) {
  AddTemplateArgument(Arg.getArgument(), Record);

  if (Arg.getArgument().getKind() == TemplateArgument::Expression) {
    bool InfoHasSameExpr
      = Arg.getArgument().getAsExpr() == Arg.getLocInfo().getAsExpr();
    Record.push_back(InfoHasSameExpr);
    if (InfoHasSameExpr)
      return; // Avoid storing the same expr twice.
  }
  AddTemplateArgumentLocInfo(Arg.getArgument().getKind(), Arg.getLocInfo(),
                             Record);
}

void ASTWriter::AddTypeSourceInfo(TypeSourceInfo *TInfo, 
                                  RecordDataImpl &Record) {
  if (!TInfo) {
    AddTypeRef(QualType(), Record);
    return;
  }

  AddTypeLoc(TInfo->getTypeLoc(), Record);
}

void ASTWriter::AddTypeLoc(TypeLoc TL, RecordDataImpl &Record) {
  AddTypeRef(TL.getType(), Record);

  TypeLocWriter TLW(*this, Record);
  for (; !TL.isNull(); TL = TL.getNextTypeLoc())
    TLW.Visit(TL);
}

void ASTWriter::AddTypeRef(QualType T, RecordDataImpl &Record) {
  Record.push_back(GetOrCreateTypeID(T));
}

TypeID ASTWriter::GetOrCreateTypeID(QualType T) {
  assert(Context);
  return MakeTypeID(*Context, T, [&](QualType T) -> TypeIdx {
    if (T.isNull())
      return TypeIdx();
    assert(!T.getLocalFastQualifiers());

    TypeIdx &Idx = TypeIdxs[T];
    if (Idx.getIndex() == 0) {
      if (DoneWritingDeclsAndTypes) {
        assert(0 && "New type seen after serializing all the types to emit!");
        return TypeIdx();
      }

      // We haven't seen this type before. Assign it a new ID and put it
      // into the queue of types to emit.
      Idx = TypeIdx(NextTypeID++);
      DeclTypesToEmit.push(T);
    }
    return Idx;
  });
}

TypeID ASTWriter::getTypeID(QualType T) const {
  assert(Context);
  return MakeTypeID(*Context, T, [&](QualType T) -> TypeIdx {
    if (T.isNull())
      return TypeIdx();
    assert(!T.getLocalFastQualifiers());

    TypeIdxMap::const_iterator I = TypeIdxs.find(T);
    assert(I != TypeIdxs.end() && "Type not emitted!");
    return I->second;
  });
}

void ASTWriter::AddDeclRef(const Decl *D, RecordDataImpl &Record) {
  Record.push_back(GetDeclRef(D));
}

DeclID ASTWriter::GetDeclRef(const Decl *D) {
  assert(WritingAST && "Cannot request a declaration ID before AST writing");

  if (!D) {
    return 0;
  }
  
  // If D comes from an AST file, its declaration ID is already known and
  // fixed.
  if (D->isFromASTFile())
    return D->getGlobalID();
  
  assert(!(reinterpret_cast<uintptr_t>(D) & 0x01) && "Invalid decl pointer");
  DeclID &ID = DeclIDs[D];
  if (ID == 0) {
    if (DoneWritingDeclsAndTypes) {
      assert(0 && "New decl seen after serializing all the decls to emit!");
      return 0;
    }

    // We haven't seen this declaration before. Give it a new ID and
    // enqueue it in the list of declarations to emit.
    ID = NextDeclID++;
    DeclTypesToEmit.push(const_cast<Decl *>(D));
  }

  return ID;
}

DeclID ASTWriter::getDeclID(const Decl *D) {
  if (!D)
    return 0;

  // If D comes from an AST file, its declaration ID is already known and
  // fixed.
  if (D->isFromASTFile())
    return D->getGlobalID();

  assert(DeclIDs.find(D) != DeclIDs.end() && "Declaration not emitted!");
  return DeclIDs[D];
}

void ASTWriter::associateDeclWithFile(const Decl *D, DeclID ID) {
  assert(ID);
  assert(D);

  SourceLocation Loc = D->getLocation();
  if (Loc.isInvalid())
    return;

  // We only keep track of the file-level declarations of each file.
  if (!D->getLexicalDeclContext()->isFileContext())
    return;
  // FIXME: ParmVarDecls that are part of a function type of a parameter of
  // a function/objc method, should not have TU as lexical context.
  if (isa<ParmVarDecl>(D))
    return;

  SourceManager &SM = Context->getSourceManager();
  SourceLocation FileLoc = SM.getFileLoc(Loc);
  assert(SM.isLocalSourceLocation(FileLoc));
  FileID FID;
  unsigned Offset;
  std::tie(FID, Offset) = SM.getDecomposedLoc(FileLoc);
  if (FID.isInvalid())
    return;
  assert(SM.getSLocEntry(FID).isFile());

  DeclIDInFileInfo *&Info = FileDeclIDs[FID];
  if (!Info)
    Info = new DeclIDInFileInfo();

  std::pair<unsigned, serialization::DeclID> LocDecl(Offset, ID);
  LocDeclIDsTy &Decls = Info->DeclIDs;

  if (Decls.empty() || Decls.back().first <= Offset) {
    Decls.push_back(LocDecl);
    return;
  }

  LocDeclIDsTy::iterator I =
      std::upper_bound(Decls.begin(), Decls.end(), LocDecl, llvm::less_first());

  Decls.insert(I, LocDecl);
}

void ASTWriter::AddDeclarationName(DeclarationName Name, RecordDataImpl &Record) {
  // FIXME: Emit a stable enum for NameKind.  0 = Identifier etc.
  Record.push_back(Name.getNameKind());
  switch (Name.getNameKind()) {
  case DeclarationName::Identifier:
    AddIdentifierRef(Name.getAsIdentifierInfo(), Record);
    break;

  case DeclarationName::ObjCZeroArgSelector:
  case DeclarationName::ObjCOneArgSelector:
  case DeclarationName::ObjCMultiArgSelector:
    AddSelectorRef(Name.getObjCSelector(), Record);
    break;

  case DeclarationName::CXXConstructorName:
  case DeclarationName::CXXDestructorName:
  case DeclarationName::CXXConversionFunctionName:
    AddTypeRef(Name.getCXXNameType(), Record);
    break;

  case DeclarationName::CXXOperatorName:
    Record.push_back(Name.getCXXOverloadedOperator());
    break;

  case DeclarationName::CXXLiteralOperatorName:
    AddIdentifierRef(Name.getCXXLiteralIdentifier(), Record);
    break;

  case DeclarationName::CXXUsingDirective:
    // No extra data to emit
    break;
  }
}

unsigned ASTWriter::getAnonymousDeclarationNumber(const NamedDecl *D) {
  assert(needsAnonymousDeclarationNumber(D) &&
         "expected an anonymous declaration");

  // Number the anonymous declarations within this context, if we've not
  // already done so.
  auto It = AnonymousDeclarationNumbers.find(D);
  if (It == AnonymousDeclarationNumbers.end()) {
    auto *DC = D->getLexicalDeclContext();
    numberAnonymousDeclsWithin(DC, [&](const NamedDecl *ND, unsigned Number) {
      AnonymousDeclarationNumbers[ND] = Number;
    });

    It = AnonymousDeclarationNumbers.find(D);
    assert(It != AnonymousDeclarationNumbers.end() &&
           "declaration not found within its lexical context");
  }

  return It->second;
}

void ASTWriter::AddDeclarationNameLoc(const DeclarationNameLoc &DNLoc,
                                     DeclarationName Name, RecordDataImpl &Record) {
  switch (Name.getNameKind()) {
  case DeclarationName::CXXConstructorName:
  case DeclarationName::CXXDestructorName:
  case DeclarationName::CXXConversionFunctionName:
    AddTypeSourceInfo(DNLoc.NamedType.TInfo, Record);
    break;

  case DeclarationName::CXXOperatorName:
    AddSourceLocation(
       SourceLocation::getFromRawEncoding(DNLoc.CXXOperatorName.BeginOpNameLoc),
       Record);
    AddSourceLocation(
        SourceLocation::getFromRawEncoding(DNLoc.CXXOperatorName.EndOpNameLoc),
        Record);
    break;

  case DeclarationName::CXXLiteralOperatorName:
    AddSourceLocation(
     SourceLocation::getFromRawEncoding(DNLoc.CXXLiteralOperatorName.OpNameLoc),
     Record);
    break;

  case DeclarationName::Identifier:
  case DeclarationName::ObjCZeroArgSelector:
  case DeclarationName::ObjCOneArgSelector:
  case DeclarationName::ObjCMultiArgSelector:
  case DeclarationName::CXXUsingDirective:
    break;
  }
}

void ASTWriter::AddDeclarationNameInfo(const DeclarationNameInfo &NameInfo,
                                       RecordDataImpl &Record) {
  AddDeclarationName(NameInfo.getName(), Record);
  AddSourceLocation(NameInfo.getLoc(), Record);
  AddDeclarationNameLoc(NameInfo.getInfo(), NameInfo.getName(), Record);
}

void ASTWriter::AddQualifierInfo(const QualifierInfo &Info,
                                 RecordDataImpl &Record) {
  AddNestedNameSpecifierLoc(Info.QualifierLoc, Record);
  Record.push_back(Info.NumTemplParamLists);
  for (unsigned i=0, e=Info.NumTemplParamLists; i != e; ++i)
    AddTemplateParameterList(Info.TemplParamLists[i], Record);
}

void ASTWriter::AddNestedNameSpecifier(NestedNameSpecifier *NNS,
                                       RecordDataImpl &Record) {
  // Nested name specifiers usually aren't too long. I think that 8 would
  // typically accommodate the vast majority.
  SmallVector<NestedNameSpecifier *, 8> NestedNames;

  // Push each of the NNS's onto a stack for serialization in reverse order.
  while (NNS) {
    NestedNames.push_back(NNS);
    NNS = NNS->getPrefix();
  }

  Record.push_back(NestedNames.size());
  while(!NestedNames.empty()) {
    NNS = NestedNames.pop_back_val();
    NestedNameSpecifier::SpecifierKind Kind = NNS->getKind();
    Record.push_back(Kind);
    switch (Kind) {
    case NestedNameSpecifier::Identifier:
      AddIdentifierRef(NNS->getAsIdentifier(), Record);
      break;

    case NestedNameSpecifier::Namespace:
      AddDeclRef(NNS->getAsNamespace(), Record);
      break;

    case NestedNameSpecifier::NamespaceAlias:
      AddDeclRef(NNS->getAsNamespaceAlias(), Record);
      break;

    case NestedNameSpecifier::TypeSpec:
    case NestedNameSpecifier::TypeSpecWithTemplate:
      AddTypeRef(QualType(NNS->getAsType(), 0), Record);
      Record.push_back(Kind == NestedNameSpecifier::TypeSpecWithTemplate);
      break;

    case NestedNameSpecifier::Global:
      // Don't need to write an associated value.
      break;

    case NestedNameSpecifier::Super:
      AddDeclRef(NNS->getAsRecordDecl(), Record);
      break;
    }
  }
}

void ASTWriter::AddNestedNameSpecifierLoc(NestedNameSpecifierLoc NNS,
                                          RecordDataImpl &Record) {
  // Nested name specifiers usually aren't too long. I think that 8 would
  // typically accommodate the vast majority.
  SmallVector<NestedNameSpecifierLoc , 8> NestedNames;

  // Push each of the nested-name-specifiers's onto a stack for
  // serialization in reverse order.
  while (NNS) {
    NestedNames.push_back(NNS);
    NNS = NNS.getPrefix();
  }

  Record.push_back(NestedNames.size());
  while(!NestedNames.empty()) {
    NNS = NestedNames.pop_back_val();
    NestedNameSpecifier::SpecifierKind Kind
      = NNS.getNestedNameSpecifier()->getKind();
    Record.push_back(Kind);
    switch (Kind) {
    case NestedNameSpecifier::Identifier:
      AddIdentifierRef(NNS.getNestedNameSpecifier()->getAsIdentifier(), Record);
      AddSourceRange(NNS.getLocalSourceRange(), Record);
      break;

    case NestedNameSpecifier::Namespace:
      AddDeclRef(NNS.getNestedNameSpecifier()->getAsNamespace(), Record);
      AddSourceRange(NNS.getLocalSourceRange(), Record);
      break;

    case NestedNameSpecifier::NamespaceAlias:
      AddDeclRef(NNS.getNestedNameSpecifier()->getAsNamespaceAlias(), Record);
      AddSourceRange(NNS.getLocalSourceRange(), Record);
      break;

    case NestedNameSpecifier::TypeSpec:
    case NestedNameSpecifier::TypeSpecWithTemplate:
      Record.push_back(Kind == NestedNameSpecifier::TypeSpecWithTemplate);
      AddTypeLoc(NNS.getTypeLoc(), Record);
      AddSourceLocation(NNS.getLocalSourceRange().getEnd(), Record);
      break;

    case NestedNameSpecifier::Global:
      AddSourceLocation(NNS.getLocalSourceRange().getEnd(), Record);
      break;

    case NestedNameSpecifier::Super:
      AddDeclRef(NNS.getNestedNameSpecifier()->getAsRecordDecl(), Record);
      AddSourceRange(NNS.getLocalSourceRange(), Record);
      break;
    }
  }
}

void ASTWriter::AddTemplateName(TemplateName Name, RecordDataImpl &Record) {
  TemplateName::NameKind Kind = Name.getKind();
  Record.push_back(Kind);
  switch (Kind) {
  case TemplateName::Template:
    AddDeclRef(Name.getAsTemplateDecl(), Record);
    break;

  case TemplateName::OverloadedTemplate: {
    OverloadedTemplateStorage *OvT = Name.getAsOverloadedTemplate();
    Record.push_back(OvT->size());
    for (OverloadedTemplateStorage::iterator I = OvT->begin(), E = OvT->end();
           I != E; ++I)
      AddDeclRef(*I, Record);
    break;
  }

  case TemplateName::QualifiedTemplate: {
    QualifiedTemplateName *QualT = Name.getAsQualifiedTemplateName();
    AddNestedNameSpecifier(QualT->getQualifier(), Record);
    Record.push_back(QualT->hasTemplateKeyword());
    AddDeclRef(QualT->getTemplateDecl(), Record);
    break;
  }

  case TemplateName::DependentTemplate: {
    DependentTemplateName *DepT = Name.getAsDependentTemplateName();
    AddNestedNameSpecifier(DepT->getQualifier(), Record);
    Record.push_back(DepT->isIdentifier());
    if (DepT->isIdentifier())
      AddIdentifierRef(DepT->getIdentifier(), Record);
    else
      Record.push_back(DepT->getOperator());
    break;
  }

  case TemplateName::SubstTemplateTemplateParm: {
    SubstTemplateTemplateParmStorage *subst
      = Name.getAsSubstTemplateTemplateParm();
    AddDeclRef(subst->getParameter(), Record);
    AddTemplateName(subst->getReplacement(), Record);
    break;
  }
      
  case TemplateName::SubstTemplateTemplateParmPack: {
    SubstTemplateTemplateParmPackStorage *SubstPack
      = Name.getAsSubstTemplateTemplateParmPack();
    AddDeclRef(SubstPack->getParameterPack(), Record);
    AddTemplateArgument(SubstPack->getArgumentPack(), Record);
    break;
  }
  }
}

void ASTWriter::AddTemplateArgument(const TemplateArgument &Arg,
                                    RecordDataImpl &Record) {
  Record.push_back(Arg.getKind());
  switch (Arg.getKind()) {
  case TemplateArgument::Null:
    break;
  case TemplateArgument::Type:
    AddTypeRef(Arg.getAsType(), Record);
    break;
  case TemplateArgument::Declaration:
    AddDeclRef(Arg.getAsDecl(), Record);
    AddTypeRef(Arg.getParamTypeForDecl(), Record);
    break;
  case TemplateArgument::NullPtr:
    AddTypeRef(Arg.getNullPtrType(), Record);
    break;
  case TemplateArgument::Integral:
    AddAPSInt(Arg.getAsIntegral(), Record);
    AddTypeRef(Arg.getIntegralType(), Record);
    break;
  case TemplateArgument::Template:
    AddTemplateName(Arg.getAsTemplateOrTemplatePattern(), Record);
    break;
  case TemplateArgument::TemplateExpansion:
    AddTemplateName(Arg.getAsTemplateOrTemplatePattern(), Record);
    if (Optional<unsigned> NumExpansions = Arg.getNumTemplateExpansions())
      Record.push_back(*NumExpansions + 1);
    else
      Record.push_back(0);
    break;
  case TemplateArgument::Expression:
    AddStmt(Arg.getAsExpr());
    break;
  case TemplateArgument::Pack:
    Record.push_back(Arg.pack_size());
    for (const auto &P : Arg.pack_elements())
      AddTemplateArgument(P, Record);
    break;
  }
}

void
ASTWriter::AddTemplateParameterList(const TemplateParameterList *TemplateParams,
                                    RecordDataImpl &Record) {
  assert(TemplateParams && "No TemplateParams!");
  AddSourceLocation(TemplateParams->getTemplateLoc(), Record);
  AddSourceLocation(TemplateParams->getLAngleLoc(), Record);
  AddSourceLocation(TemplateParams->getRAngleLoc(), Record);
  Record.push_back(TemplateParams->size());
  for (TemplateParameterList::const_iterator
         P = TemplateParams->begin(), PEnd = TemplateParams->end();
         P != PEnd; ++P)
    AddDeclRef(*P, Record);
}

/// \brief Emit a template argument list.
void
ASTWriter::AddTemplateArgumentList(const TemplateArgumentList *TemplateArgs,
                                   RecordDataImpl &Record) {
  assert(TemplateArgs && "No TemplateArgs!");
  Record.push_back(TemplateArgs->size());
  for (int i=0, e = TemplateArgs->size(); i != e; ++i)
    AddTemplateArgument(TemplateArgs->get(i), Record);
}

void
ASTWriter::AddASTTemplateArgumentListInfo
(const ASTTemplateArgumentListInfo *ASTTemplArgList, RecordDataImpl &Record) {
  assert(ASTTemplArgList && "No ASTTemplArgList!");
  AddSourceLocation(ASTTemplArgList->LAngleLoc, Record);
  AddSourceLocation(ASTTemplArgList->RAngleLoc, Record);
  Record.push_back(ASTTemplArgList->NumTemplateArgs);
  const TemplateArgumentLoc *TemplArgs = ASTTemplArgList->getTemplateArgs();
  for (int i=0, e = ASTTemplArgList->NumTemplateArgs; i != e; ++i)
    AddTemplateArgumentLoc(TemplArgs[i], Record);
}

void
ASTWriter::AddUnresolvedSet(const ASTUnresolvedSet &Set, RecordDataImpl &Record) {
  Record.push_back(Set.size());
  for (ASTUnresolvedSet::const_iterator
         I = Set.begin(), E = Set.end(); I != E; ++I) {
    AddDeclRef(I.getDecl(), Record);
    Record.push_back(I.getAccess());
  }
}

void ASTWriter::AddCXXBaseSpecifier(const CXXBaseSpecifier &Base,
                                    RecordDataImpl &Record) {
  Record.push_back(Base.isVirtual());
  Record.push_back(Base.isBaseOfClass());
  Record.push_back(Base.getAccessSpecifierAsWritten());
  Record.push_back(Base.getInheritConstructors());
  AddTypeSourceInfo(Base.getTypeSourceInfo(), Record);
  AddSourceRange(Base.getSourceRange(), Record);
  AddSourceLocation(Base.isPackExpansion()? Base.getEllipsisLoc() 
                                          : SourceLocation(),
                    Record);
}

void ASTWriter::FlushCXXBaseSpecifiers() {
  RecordData Record;
  unsigned N = CXXBaseSpecifiersToWrite.size();
  for (unsigned I = 0; I != N; ++I) {
    Record.clear();
    
    // Record the offset of this base-specifier set.
    unsigned Index = CXXBaseSpecifiersToWrite[I].ID - 1;
    if (Index == CXXBaseSpecifiersOffsets.size())
      CXXBaseSpecifiersOffsets.push_back(Stream.GetCurrentBitNo());
    else {
      if (Index > CXXBaseSpecifiersOffsets.size())
        CXXBaseSpecifiersOffsets.resize(Index + 1);
      CXXBaseSpecifiersOffsets[Index] = Stream.GetCurrentBitNo();
    }

    const CXXBaseSpecifier *B = CXXBaseSpecifiersToWrite[I].Bases,
                        *BEnd = CXXBaseSpecifiersToWrite[I].BasesEnd;
    Record.push_back(BEnd - B);
    for (; B != BEnd; ++B)
      AddCXXBaseSpecifier(*B, Record);
    Stream.EmitRecord(serialization::DECL_CXX_BASE_SPECIFIERS, Record);
    
    // Flush any expressions that were written as part of the base specifiers.
    FlushStmts();
  }

  assert(N == CXXBaseSpecifiersToWrite.size() &&
         "added more base specifiers while writing base specifiers");
  CXXBaseSpecifiersToWrite.clear();
}

void ASTWriter::AddCXXCtorInitializers(
                             const CXXCtorInitializer * const *CtorInitializers,
                             unsigned NumCtorInitializers,
                             RecordDataImpl &Record) {
  Record.push_back(NumCtorInitializers);
  for (unsigned i=0; i != NumCtorInitializers; ++i) {
    const CXXCtorInitializer *Init = CtorInitializers[i];

    if (Init->isBaseInitializer()) {
      Record.push_back(CTOR_INITIALIZER_BASE);
      AddTypeSourceInfo(Init->getTypeSourceInfo(), Record);
      Record.push_back(Init->isBaseVirtual());
    } else if (Init->isDelegatingInitializer()) {
      Record.push_back(CTOR_INITIALIZER_DELEGATING);
      AddTypeSourceInfo(Init->getTypeSourceInfo(), Record);
    } else if (Init->isMemberInitializer()){
      Record.push_back(CTOR_INITIALIZER_MEMBER);
      AddDeclRef(Init->getMember(), Record);
    } else {
      Record.push_back(CTOR_INITIALIZER_INDIRECT_MEMBER);
      AddDeclRef(Init->getIndirectMember(), Record);
    }

    AddSourceLocation(Init->getMemberLocation(), Record);
    AddStmt(Init->getInit());
    AddSourceLocation(Init->getLParenLoc(), Record);
    AddSourceLocation(Init->getRParenLoc(), Record);
    Record.push_back(Init->isWritten());
    if (Init->isWritten()) {
      Record.push_back(Init->getSourceOrder());
    } else {
      Record.push_back(Init->getNumArrayIndices());
      for (unsigned i=0, e=Init->getNumArrayIndices(); i != e; ++i)
        AddDeclRef(Init->getArrayIndex(i), Record);
    }
  }
}

void ASTWriter::FlushCXXCtorInitializers() {
  RecordData Record;

  unsigned N = CXXCtorInitializersToWrite.size();
  (void)N; // Silence unused warning in non-assert builds.
  for (auto &Init : CXXCtorInitializersToWrite) {
    Record.clear();

    // Record the offset of this mem-initializer list.
    unsigned Index = Init.ID - 1;
    if (Index == CXXCtorInitializersOffsets.size())
      CXXCtorInitializersOffsets.push_back(Stream.GetCurrentBitNo());
    else {
      if (Index > CXXCtorInitializersOffsets.size())
        CXXCtorInitializersOffsets.resize(Index + 1);
      CXXCtorInitializersOffsets[Index] = Stream.GetCurrentBitNo();
    }

    AddCXXCtorInitializers(Init.Inits.data(), Init.Inits.size(), Record);
    Stream.EmitRecord(serialization::DECL_CXX_CTOR_INITIALIZERS, Record);

    // Flush any expressions that were written as part of the initializers.
    FlushStmts();
  }

  assert(N == CXXCtorInitializersToWrite.size() &&
         "added more ctor initializers while writing ctor initializers");
  CXXCtorInitializersToWrite.clear();
}

void ASTWriter::AddCXXDefinitionData(const CXXRecordDecl *D, RecordDataImpl &Record) {
  auto &Data = D->data();
  Record.push_back(Data.IsLambda);
  Record.push_back(Data.UserDeclaredConstructor);
  Record.push_back(Data.UserDeclaredSpecialMembers);
  Record.push_back(Data.Aggregate);
  Record.push_back(Data.PlainOldData);
  Record.push_back(Data.Empty);
  Record.push_back(Data.Polymorphic);
  Record.push_back(Data.Abstract);
  Record.push_back(Data.IsStandardLayout);
  Record.push_back(Data.HasNoNonEmptyBases);
  Record.push_back(Data.HasPrivateFields);
  Record.push_back(Data.HasProtectedFields);
  Record.push_back(Data.HasPublicFields);
  Record.push_back(Data.HasMutableFields);
  Record.push_back(Data.HasVariantMembers);
  Record.push_back(Data.HasOnlyCMembers);
  Record.push_back(Data.HasInClassInitializer);
  Record.push_back(Data.HasUninitializedReferenceMember);
  Record.push_back(Data.NeedOverloadResolutionForMoveConstructor);
  Record.push_back(Data.NeedOverloadResolutionForMoveAssignment);
  Record.push_back(Data.NeedOverloadResolutionForDestructor);
  Record.push_back(Data.DefaultedMoveConstructorIsDeleted);
  Record.push_back(Data.DefaultedMoveAssignmentIsDeleted);
  Record.push_back(Data.DefaultedDestructorIsDeleted);
  Record.push_back(Data.HasTrivialSpecialMembers);
  Record.push_back(Data.DeclaredNonTrivialSpecialMembers);
  Record.push_back(Data.HasIrrelevantDestructor);
  Record.push_back(Data.HasConstexprNonCopyMoveConstructor);
  Record.push_back(Data.DefaultedDefaultConstructorIsConstexpr);
  Record.push_back(Data.HasConstexprDefaultConstructor);
  Record.push_back(Data.HasNonLiteralTypeFieldsOrBases);
  Record.push_back(Data.ComputedVisibleConversions);
  Record.push_back(Data.UserProvidedDefaultConstructor);
  Record.push_back(Data.DeclaredSpecialMembers);
  Record.push_back(Data.ImplicitCopyConstructorHasConstParam);
  Record.push_back(Data.ImplicitCopyAssignmentHasConstParam);
  Record.push_back(Data.HasDeclaredCopyConstructorWithConstParam);
  Record.push_back(Data.HasDeclaredCopyAssignmentWithConstParam);
  // IsLambda bit is already saved.

  Record.push_back(Data.NumBases);
  if (Data.NumBases > 0)
    AddCXXBaseSpecifiersRef(Data.getBases(), Data.getBases() + Data.NumBases, 
                            Record);
  
  // FIXME: Make VBases lazily computed when needed to avoid storing them.
  Record.push_back(Data.NumVBases);
  if (Data.NumVBases > 0)
    AddCXXBaseSpecifiersRef(Data.getVBases(), Data.getVBases() + Data.NumVBases, 
                            Record);

  AddUnresolvedSet(Data.Conversions.get(*Context), Record);
  AddUnresolvedSet(Data.VisibleConversions.get(*Context), Record);
  // Data.Definition is the owning decl, no need to write it. 
  AddDeclRef(D->getFirstFriend(), Record);
  
  // Add lambda-specific data.
  if (Data.IsLambda) {
    auto &Lambda = D->getLambdaData();
    Record.push_back(Lambda.Dependent);
    Record.push_back(Lambda.IsGenericLambda);
    Record.push_back(Lambda.CaptureDefault);
    Record.push_back(Lambda.NumCaptures);
    Record.push_back(Lambda.NumExplicitCaptures);
    Record.push_back(Lambda.ManglingNumber);
    AddDeclRef(Lambda.ContextDecl, Record);
    AddTypeSourceInfo(Lambda.MethodTyInfo, Record);
    for (unsigned I = 0, N = Lambda.NumCaptures; I != N; ++I) {
      const LambdaCapture &Capture = Lambda.Captures[I];
      AddSourceLocation(Capture.getLocation(), Record);
      Record.push_back(Capture.isImplicit());
      Record.push_back(Capture.getCaptureKind());
      switch (Capture.getCaptureKind()) {
      case LCK_This:
      case LCK_VLAType:
        break;
      case LCK_ByCopy:
      case LCK_ByRef:
        VarDecl *Var =
            Capture.capturesVariable() ? Capture.getCapturedVar() : nullptr;
        AddDeclRef(Var, Record);
        AddSourceLocation(Capture.isPackExpansion() ? Capture.getEllipsisLoc()
                                                    : SourceLocation(),
                          Record);
        break;
      }
    }
  }
}

void ASTWriter::ReaderInitialized(ASTReader *Reader) {
  assert(Reader && "Cannot remove chain");
  assert((!Chain || Chain == Reader) && "Cannot replace chain");
  assert(FirstDeclID == NextDeclID &&
         FirstTypeID == NextTypeID &&
         FirstIdentID == NextIdentID &&
         FirstMacroID == NextMacroID &&
         FirstSubmoduleID == NextSubmoduleID &&
         FirstSelectorID == NextSelectorID &&
         "Setting chain after writing has started.");

  Chain = Reader;

  // Note, this will get called multiple times, once one the reader starts up
  // and again each time it's done reading a PCH or module.
  FirstDeclID = NUM_PREDEF_DECL_IDS + Chain->getTotalNumDecls();
  FirstTypeID = NUM_PREDEF_TYPE_IDS + Chain->getTotalNumTypes();
  FirstIdentID = NUM_PREDEF_IDENT_IDS + Chain->getTotalNumIdentifiers();
  FirstMacroID = NUM_PREDEF_MACRO_IDS + Chain->getTotalNumMacros();
  FirstSubmoduleID = NUM_PREDEF_SUBMODULE_IDS + Chain->getTotalNumSubmodules();
  FirstSelectorID = NUM_PREDEF_SELECTOR_IDS + Chain->getTotalNumSelectors();
  NextDeclID = FirstDeclID;
  NextTypeID = FirstTypeID;
  NextIdentID = FirstIdentID;
  NextMacroID = FirstMacroID;
  NextSelectorID = FirstSelectorID;
  NextSubmoduleID = FirstSubmoduleID;
}

void ASTWriter::IdentifierRead(IdentID ID, IdentifierInfo *II) {
  // Always keep the highest ID. See \p TypeRead() for more information.
  IdentID &StoredID = IdentifierIDs[II];
  if (ID > StoredID)
    StoredID = ID;
}

void ASTWriter::MacroRead(serialization::MacroID ID, MacroInfo *MI) {
  // Always keep the highest ID. See \p TypeRead() for more information.
  MacroID &StoredID = MacroIDs[MI];
  if (ID > StoredID)
    StoredID = ID;
}

void ASTWriter::TypeRead(TypeIdx Idx, QualType T) {
  // Always take the highest-numbered type index. This copes with an interesting
  // case for chained AST writing where we schedule writing the type and then,
  // later, deserialize the type from another AST. In this case, we want to
  // keep the higher-numbered entry so that we can properly write it out to
  // the AST file.
  TypeIdx &StoredIdx = TypeIdxs[T];
  if (Idx.getIndex() >= StoredIdx.getIndex())
    StoredIdx = Idx;
}

void ASTWriter::SelectorRead(SelectorID ID, Selector S) {
  // Always keep the highest ID. See \p TypeRead() for more information.
  SelectorID &StoredID = SelectorIDs[S];
  if (ID > StoredID)
    StoredID = ID;
}

void ASTWriter::MacroDefinitionRead(serialization::PreprocessedEntityID ID,
                                    MacroDefinitionRecord *MD) {
  assert(MacroDefinitions.find(MD) == MacroDefinitions.end());
  MacroDefinitions[MD] = ID;
}

void ASTWriter::ModuleRead(serialization::SubmoduleID ID, Module *Mod) {
  assert(SubmoduleIDs.find(Mod) == SubmoduleIDs.end());
  SubmoduleIDs[Mod] = ID;
}

void ASTWriter::CompletedTagDefinition(const TagDecl *D) {
  assert(D->isCompleteDefinition());
  assert(!WritingAST && "Already writing the AST!");
  if (const CXXRecordDecl *RD = dyn_cast<CXXRecordDecl>(D)) {
    // We are interested when a PCH decl is modified.
    if (RD->isFromASTFile()) {
      // A forward reference was mutated into a definition. Rewrite it.
      // FIXME: This happens during template instantiation, should we
      // have created a new definition decl instead ?
      assert(isTemplateInstantiation(RD->getTemplateSpecializationKind()) &&
             "completed a tag from another module but not by instantiation?");
      DeclUpdates[RD].push_back(
          DeclUpdate(UPD_CXX_INSTANTIATED_CLASS_DEFINITION));
    }
  }
}

void ASTWriter::AddedVisibleDecl(const DeclContext *DC, const Decl *D) {
  // TU and namespaces are handled elsewhere.
  if (isa<TranslationUnitDecl>(DC) || isa<NamespaceDecl>(DC))
    return;

  if (!(!D->isFromASTFile() && cast<Decl>(DC)->isFromASTFile()))
    return; // Not a source decl added to a DeclContext from PCH.

  assert(!getDefinitiveDeclContext(DC) && "DeclContext not definitive!");
  assert(!WritingAST && "Already writing the AST!");
  UpdatedDeclContexts.insert(DC);
  UpdatingVisibleDecls.push_back(D);
}

void ASTWriter::AddedCXXImplicitMember(const CXXRecordDecl *RD, const Decl *D) {
  assert(D->isImplicit());
  if (!(!D->isFromASTFile() && RD->isFromASTFile()))
    return; // Not a source member added to a class from PCH.
  if (!isa<CXXMethodDecl>(D))
    return; // We are interested in lazily declared implicit methods.

  // A decl coming from PCH was modified.
  assert(RD->isCompleteDefinition());
  assert(!WritingAST && "Already writing the AST!");
  DeclUpdates[RD].push_back(DeclUpdate(UPD_CXX_ADDED_IMPLICIT_MEMBER, D));
}

void ASTWriter::AddedCXXTemplateSpecialization(const ClassTemplateDecl *TD,
                                     const ClassTemplateSpecializationDecl *D) {
  // The specializations set is kept in the canonical template.
  TD = TD->getCanonicalDecl();
  if (!(!D->isFromASTFile() && TD->isFromASTFile()))
    return; // Not a source specialization added to a template from PCH.

  assert(!WritingAST && "Already writing the AST!");
  DeclUpdates[TD].push_back(DeclUpdate(UPD_CXX_ADDED_TEMPLATE_SPECIALIZATION,
                                       D));
}

void ASTWriter::AddedCXXTemplateSpecialization(
    const VarTemplateDecl *TD, const VarTemplateSpecializationDecl *D) {
  // The specializations set is kept in the canonical template.
  TD = TD->getCanonicalDecl();
  if (!(!D->isFromASTFile() && TD->isFromASTFile()))
    return; // Not a source specialization added to a template from PCH.

  assert(!WritingAST && "Already writing the AST!");
  DeclUpdates[TD].push_back(DeclUpdate(UPD_CXX_ADDED_TEMPLATE_SPECIALIZATION,
                                       D));
}

void ASTWriter::AddedCXXTemplateSpecialization(const FunctionTemplateDecl *TD,
                                               const FunctionDecl *D) {
  // The specializations set is kept in the canonical template.
  TD = TD->getCanonicalDecl();
  if (!(!D->isFromASTFile() && TD->isFromASTFile()))
    return; // Not a source specialization added to a template from PCH.

  assert(!WritingAST && "Already writing the AST!");
  DeclUpdates[TD].push_back(DeclUpdate(UPD_CXX_ADDED_TEMPLATE_SPECIALIZATION,
                                       D));
}

void ASTWriter::ResolvedExceptionSpec(const FunctionDecl *FD) {
  assert(!DoneWritingDeclsAndTypes && "Already done writing updates!");
  if (!Chain) return;
  Chain->forEachImportedKeyDecl(FD, [&](const Decl *D) {
    // If we don't already know the exception specification for this redecl
    // chain, add an update record for it.
    if (isUnresolvedExceptionSpec(cast<FunctionDecl>(D)
                                      ->getType()
                                      ->castAs<FunctionProtoType>()
                                      ->getExceptionSpecType()))
      DeclUpdates[D].push_back(UPD_CXX_RESOLVED_EXCEPTION_SPEC);
  });
}

void ASTWriter::DeducedReturnType(const FunctionDecl *FD, QualType ReturnType) {
  assert(!WritingAST && "Already writing the AST!");
  if (!Chain) return;
  Chain->forEachImportedKeyDecl(FD, [&](const Decl *D) {
    DeclUpdates[D].push_back(
        DeclUpdate(UPD_CXX_DEDUCED_RETURN_TYPE, ReturnType));
  });
}

void ASTWriter::ResolvedOperatorDelete(const CXXDestructorDecl *DD,
                                       const FunctionDecl *Delete) {
  assert(!WritingAST && "Already writing the AST!");
  assert(Delete && "Not given an operator delete");
  if (!Chain) return;
  Chain->forEachImportedKeyDecl(DD, [&](const Decl *D) {
    DeclUpdates[D].push_back(DeclUpdate(UPD_CXX_RESOLVED_DTOR_DELETE, Delete));
  });
}

void ASTWriter::CompletedImplicitDefinition(const FunctionDecl *D) {
  assert(!WritingAST && "Already writing the AST!");
  if (!D->isFromASTFile())
    return; // Declaration not imported from PCH.

  // Implicit function decl from a PCH was defined.
  DeclUpdates[D].push_back(DeclUpdate(UPD_CXX_ADDED_FUNCTION_DEFINITION));
}

void ASTWriter::FunctionDefinitionInstantiated(const FunctionDecl *D) {
  assert(!WritingAST && "Already writing the AST!");
  if (!D->isFromASTFile())
    return;

  DeclUpdates[D].push_back(DeclUpdate(UPD_CXX_ADDED_FUNCTION_DEFINITION));
}

void ASTWriter::StaticDataMemberInstantiated(const VarDecl *D) {
  assert(!WritingAST && "Already writing the AST!");
  if (!D->isFromASTFile())
    return;

  // Since the actual instantiation is delayed, this really means that we need
  // to update the instantiation location.
  DeclUpdates[D].push_back(
      DeclUpdate(UPD_CXX_INSTANTIATED_STATIC_DATA_MEMBER,
       D->getMemberSpecializationInfo()->getPointOfInstantiation()));
}

void ASTWriter::AddedObjCCategoryToInterface(const ObjCCategoryDecl *CatD,
                                             const ObjCInterfaceDecl *IFD) {
  assert(!WritingAST && "Already writing the AST!");
  if (!IFD->isFromASTFile())
    return; // Declaration not imported from PCH.
  
  assert(IFD->getDefinition() && "Category on a class without a definition?");
  ObjCClassesWithCategories.insert(
    const_cast<ObjCInterfaceDecl *>(IFD->getDefinition()));
}


void ASTWriter::AddedObjCPropertyInClassExtension(const ObjCPropertyDecl *Prop,
                                          const ObjCPropertyDecl *OrigProp,
                                          const ObjCCategoryDecl *ClassExt) {
  const ObjCInterfaceDecl *D = ClassExt->getClassInterface();
  if (!D)
    return;

  assert(!WritingAST && "Already writing the AST!");
  if (!D->isFromASTFile())
    return; // Declaration not imported from PCH.

  RewriteDecl(D);
}

void ASTWriter::DeclarationMarkedUsed(const Decl *D) {
  assert(!WritingAST && "Already writing the AST!");
  if (!D->isFromASTFile())
    return;

  DeclUpdates[D].push_back(DeclUpdate(UPD_DECL_MARKED_USED));
}

void ASTWriter::DeclarationMarkedOpenMPThreadPrivate(const Decl *D) {
  assert(!WritingAST && "Already writing the AST!");
  if (!D->isFromASTFile())
    return;

  DeclUpdates[D].push_back(DeclUpdate(UPD_DECL_MARKED_OPENMP_THREADPRIVATE));
}

void ASTWriter::RedefinedHiddenDefinition(const NamedDecl *D, Module *M) {
  assert(!WritingAST && "Already writing the AST!");
  assert(D->isHidden() && "expected a hidden declaration");
  DeclUpdates[D].push_back(DeclUpdate(UPD_DECL_EXPORTED, M));
}

void ASTWriter::AddedAttributeToRecord(const Attr *Attr,
                                       const RecordDecl *Record) {
  assert(!WritingAST && "Already writing the AST!");
  if (!Record->isFromASTFile())
    return;
  DeclUpdates[Record].push_back(DeclUpdate(UPD_ADDED_ATTR_TO_RECORD, Attr));
}
