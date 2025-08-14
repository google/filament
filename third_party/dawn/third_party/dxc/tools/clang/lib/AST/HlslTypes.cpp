//===--- HlslTypes.cpp  - Type system for HLSL                 ----*- C++
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// HlslTypes.cpp                                                             //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///
/// \file                                                                    //
/// \brief Defines the HLSL type system interface.                           //
///
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "clang/AST/HlslTypes.h"
#include "dxc/DXIL/DxilNodeProps.h"
#include "dxc/DXIL/DxilSemantic.h"
#include "dxc/Support/Global.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/CanonicalType.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Type.h"
#include "clang/Sema/AttributeList.h" // conceptually ParsedAttributes
#include "llvm/ADT/StringSwitch.h"

using namespace clang;

namespace hlsl {

/// <summary>Try to convert HLSL template vector/matrix type to
/// ExtVectorType.</summary>
const clang::ExtVectorType *
ConvertHLSLVecMatTypeToExtVectorType(const clang::ASTContext &context,
                                     clang::QualType type) {
  const Type *Ty = type.getCanonicalType().getTypePtr();

  if (const RecordType *RT = dyn_cast<RecordType>(Ty)) {
    if (const ClassTemplateSpecializationDecl *templateDecl =
            dyn_cast<ClassTemplateSpecializationDecl>(RT->getDecl())) {
      // TODO: check pointer instead of name
      if (templateDecl->getName() == "vector") {
        const TemplateArgumentList &argList = templateDecl->getTemplateArgs();
        const TemplateArgument &arg0 = argList[0];
        const TemplateArgument &arg1 = argList[1];
        QualType elemTy = arg0.getAsType();
        llvm::APSInt elmSize = arg1.getAsIntegral();
        return context.getExtVectorType(elemTy, elmSize.getLimitedValue())
            ->getAs<ExtVectorType>();
      }
    }
  }
  return nullptr;
}

template <typename AttrType> static AttrType *getAttr(clang::QualType type) {
  type = type.getCanonicalType();
  if (const RecordType *RT = type->getAs<RecordType>()) {
    if (const auto *Spec =
            dyn_cast<ClassTemplateSpecializationDecl>(RT->getDecl()))
      if (const auto *Template =
              dyn_cast<ClassTemplateDecl>(Spec->getSpecializedTemplate()))
        return Template->getTemplatedDecl()->getAttr<AttrType>();
    if (const auto *Decl = dyn_cast<CXXRecordDecl>(RT->getDecl()))
      return Decl->getAttr<AttrType>();
  }
  return nullptr;
}

bool IsHLSLVecMatType(clang::QualType type) {
  return getAttr<HLSLMatrixAttr>(type) || getAttr<HLSLVectorAttr>(type);
}

bool IsHLSLMatType(clang::QualType type) {
  if (getAttr<HLSLMatrixAttr>(type))
    return true;
  return false;
}

bool IsHLSLVecType(clang::QualType type) {
  if (getAttr<HLSLVectorAttr>(type))
    return true;
  return false;
}

bool IsHLSLNumericOrAggregateOfNumericType(clang::QualType type) {
  const clang::Type *Ty = type.getCanonicalType().getTypePtr();
  if (isa<RecordType>(Ty)) {
    if (IsHLSLVecMatType(type))
      return true;
    return IsHLSLCopyableAnnotatableRecord(type);
  } else if (type->isArrayType()) {
    return IsHLSLNumericOrAggregateOfNumericType(
        QualType(type->getArrayElementTypeNoTypeQual(), 0));
  } else if (type->isEnumeralType()) {
    return true;
  }

  // Chars can only appear as part of strings, which we don't consider numeric.
  const BuiltinType *BuiltinTy = dyn_cast<BuiltinType>(Ty);
  return BuiltinTy != nullptr &&
         BuiltinTy->getKind() != BuiltinType::Kind::Char_S;
}

// In some cases we need record types that are annotatable and trivially
// copyable from outside the shader. This excludes resource types which may be
// trivially copyable inside the shader, and builtin matrix and vector types
// which can't be annotated. But includes UDTs of trivially copyable data and
// the builtin trivially copyable raytracing structs.
bool IsHLSLCopyableAnnotatableRecord(clang::QualType QT) {
  assert(!QT->isIncompleteType() && "Type must be complete!");
  const clang::Type *Ty = QT.getCanonicalType().getTypePtr();
  if (const RecordType *RT = dyn_cast<RecordType>(Ty)) {
    const RecordDecl *RD = RT->getDecl();
    if (!IsUserDefinedRecordType(QT))
      return false;
    for (auto Member : RD->fields()) {
      if (!IsHLSLNumericOrAggregateOfNumericType(Member->getType()))
        return false;
    }
    if (auto *CXXRD = dyn_cast<CXXRecordDecl>(RD)) {
      // Walk up the inheritance chain and check base class fields
      for (const auto &Base : CXXRD->bases()) {
        if (!IsHLSLCopyableAnnotatableRecord(Base.getType()))
          return false;
      }
    }
    return true;
  }
  return false;
}

bool IsHLSLBuiltinRayAttributeStruct(clang::QualType QT) {
  QT = QT.getCanonicalType();
  const clang::Type *Ty = QT.getTypePtr();
  if (const RecordType *RT = dyn_cast<RecordType>(Ty)) {
    const RecordDecl *RD = RT->getDecl();
    if (RD->getName() == "BuiltInTriangleIntersectionAttributes" ||
        RD->getName() == "RayDesc")
      return true;
  }
  return false;
}

// Aggregate types are arrays and user-defined structs
bool IsHLSLAggregateType(clang::QualType type) {
  type = type.getCanonicalType();
  if (isa<clang::ArrayType>(type))
    return true;

  return IsUserDefinedRecordType(type);
}

bool GetHLSLNodeIORecordType(const ParmVarDecl *parmDecl, NodeFlags &nodeKind) {
  clang::QualType paramTy = parmDecl->getType().getCanonicalType();

  if (auto arrayType = dyn_cast<ConstantArrayType>(paramTy))
    paramTy = arrayType->getElementType();

  nodeKind = NodeFlags(GetNodeIOType(paramTy));
  return nodeKind.IsValidNodeKind();
}

clang::QualType GetElementTypeOrType(clang::QualType type) {
  if (const RecordType *RT = type->getAs<RecordType>()) {
    if (const ClassTemplateSpecializationDecl *templateDecl =
            dyn_cast<ClassTemplateSpecializationDecl>(RT->getDecl())) {
      // TODO: check pointer instead of name
      if (templateDecl->getName() == "vector") {
        const TemplateArgumentList &argList = templateDecl->getTemplateArgs();
        return argList[0].getAsType();
      } else if (templateDecl->getName() == "matrix") {
        const TemplateArgumentList &argList = templateDecl->getTemplateArgs();
        return argList[0].getAsType();
      }
    }
  }
  return type;
}

bool HasHLSLMatOrientation(clang::QualType type, bool *pIsRowMajor) {
  const AttributedType *AT = type->getAs<AttributedType>();
  while (AT) {
    AttributedType::Kind kind = AT->getAttrKind();
    switch (kind) {
    case AttributedType::attr_hlsl_row_major:
      if (pIsRowMajor)
        *pIsRowMajor = true;
      return true;
    case AttributedType::attr_hlsl_column_major:
      if (pIsRowMajor)
        *pIsRowMajor = false;
      return true;
    }
    AT = AT->getLocallyUnqualifiedSingleStepDesugaredType()
             ->getAs<AttributedType>();
  }
  return false;
}

bool IsHLSLMatRowMajor(clang::QualType type, bool defaultValue) {
  bool result = defaultValue;
  HasHLSLMatOrientation(type, &result);
  return result;
}

bool IsHLSLUnsigned(clang::QualType type) {
  if (type->getAs<clang::BuiltinType>() == nullptr) {
    type = type.getCanonicalType().getNonReferenceType();

    if (IsHLSLVecMatType(type))
      type = GetElementTypeOrType(type);

    if (type->isExtVectorType())
      type = type->getAs<clang::ExtVectorType>()->getElementType();
  }

  return type->isUnsignedIntegerType();
}

bool IsHLSLMinPrecision(clang::QualType Ty) {
  Ty = Ty.getCanonicalType().getNonReferenceType();
  if (auto BT = Ty->getAs<clang::BuiltinType>()) {
    switch (BT->getKind()) {
    case clang::BuiltinType::Min12Int:
    case clang::BuiltinType::Min16Int:
    case clang::BuiltinType::Min16UInt:
    case clang::BuiltinType::Min16Float:
    case clang::BuiltinType::Min10Float:
      return true;
    }
  }

  return false;
}

bool HasHLSLUNormSNorm(clang::QualType type, bool *pIsSNorm) {
  // snorm/unorm can be on outer vector/matrix as well as element type
  // in the template form.  Outer-most type attribute wins.
  // The following drills into attributed type for outer type,
  // setting *pIsSNorm and returning true if snorm/unorm found.
  // If not found on outer type, fall back to element type if different,
  // indicating a vector or matrix, and try again.
  clang::QualType elementType = GetElementTypeOrType(type);
  while (true) {
    const AttributedType *AT = type->getAs<AttributedType>();
    while (AT) {
      AttributedType::Kind kind = AT->getAttrKind();
      switch (kind) {
      case AttributedType::attr_hlsl_snorm:
        if (pIsSNorm)
          *pIsSNorm = true;
        return true;
      case AttributedType::attr_hlsl_unorm:
        if (pIsSNorm)
          *pIsSNorm = false;
        return true;
      }
      AT = AT->getLocallyUnqualifiedSingleStepDesugaredType()
               ->getAs<AttributedType>();
    }
    if (type == elementType)
      break;
    type = elementType;
  }
  return false;
}

bool HasHLSLGloballyCoherent(clang::QualType type) {
  const AttributedType *AT = type->getAs<AttributedType>();
  while (AT) {
    AttributedType::Kind kind = AT->getAttrKind();
    switch (kind) {
    case AttributedType::attr_hlsl_globallycoherent:
      return true;
    }
    AT = AT->getLocallyUnqualifiedSingleStepDesugaredType()
             ->getAs<AttributedType>();
  }
  return false;
}

bool HasHLSLReorderCoherent(clang::QualType type) {
  const AttributedType *AT = type->getAs<AttributedType>();
  while (AT) {
    AttributedType::Kind kind = AT->getAttrKind();
    if (kind == AttributedType::attr_hlsl_reordercoherent)
      return true;
    AT = AT->getLocallyUnqualifiedSingleStepDesugaredType()
             ->getAs<AttributedType>();
  }
  return false;
}

/// Checks whether the pAttributes indicate a parameter is inout or out; if
/// inout, pIsIn will be set to true.
bool IsParamAttributedAsOut(clang::AttributeList *pAttributes, bool *pIsIn);

/// <summary>Gets the type with structural information (elements and shape) for
/// the given type.</summary>
/// <remarks>This function will strip lvalue/rvalue references, attributes and
/// qualifiers.</remarks>
QualType GetStructuralForm(QualType type) {
  if (type.isNull()) {
    return type;
  }

  const ReferenceType *RefType = nullptr;
  const AttributedType *AttrType = nullptr;
  while ((RefType = dyn_cast<ReferenceType>(type)) ||
         (AttrType = dyn_cast<AttributedType>(type))) {
    type = RefType ? RefType->getPointeeType() : AttrType->getEquivalentType();
  }

  // Despite its name, getCanonicalTypeUnqualified will preserve const for array
  // elements or something
  return QualType(type->getCanonicalTypeUnqualified()->getTypePtr(), 0);
}

uint32_t GetElementCount(clang::QualType type) {
  uint32_t rowCount, colCount;
  GetRowsAndColsForAny(type, rowCount, colCount);
  return rowCount * colCount;
}

/// <summary>Returns the number of elements in the specified array
/// type.</summary>
uint32_t GetArraySize(clang::QualType type) {
  assert(type->isArrayType() && "otherwise caller shouldn't be invoking this");

  if (type->isConstantArrayType()) {
    const ConstantArrayType *arrayType =
        (const ConstantArrayType *)type->getAsArrayTypeUnsafe();
    return arrayType->getSize().getLimitedValue();
  } else {
    return 0;
  }
}

/// <summary>Returns the number of elements in the specified vector
/// type.</summary>
uint32_t GetHLSLVecSize(clang::QualType type) {
  type = GetStructuralForm(type);

  const Type *Ty = type.getCanonicalType().getTypePtr();
  const RecordType *RT = dyn_cast<RecordType>(Ty);
  assert(RT != nullptr && "otherwise caller shouldn't be invoking this");
  const ClassTemplateSpecializationDecl *templateDecl =
      dyn_cast<ClassTemplateSpecializationDecl>(RT->getAsCXXRecordDecl());
  assert(templateDecl != nullptr &&
         "otherwise caller shouldn't be invoking this");
  assert(templateDecl->getName() == "vector" &&
         "otherwise caller shouldn't be invoking this");

  const TemplateArgumentList &argList = templateDecl->getTemplateArgs();
  const TemplateArgument &arg1 = argList[1];
  llvm::APSInt vecSize = arg1.getAsIntegral();
  return vecSize.getLimitedValue();
}

void GetRowsAndCols(clang::QualType type, uint32_t &rowCount,
                    uint32_t &colCount) {
  type = GetStructuralForm(type);

  const Type *Ty = type.getCanonicalType().getTypePtr();
  const RecordType *RT = dyn_cast<RecordType>(Ty);
  assert(RT != nullptr && "otherwise caller shouldn't be invoking this");
  const ClassTemplateSpecializationDecl *templateDecl =
      dyn_cast<ClassTemplateSpecializationDecl>(RT->getAsCXXRecordDecl());
  assert(templateDecl != nullptr &&
         "otherwise caller shouldn't be invoking this");
  assert(templateDecl->getName() == "matrix" &&
         "otherwise caller shouldn't be invoking this");

  const TemplateArgumentList &argList = templateDecl->getTemplateArgs();
  const TemplateArgument &arg1 = argList[1];
  const TemplateArgument &arg2 = argList[2];
  llvm::APSInt rowSize = arg1.getAsIntegral();
  llvm::APSInt colSize = arg2.getAsIntegral();
  rowCount = rowSize.getLimitedValue();
  colCount = colSize.getLimitedValue();
}

bool IsArrayConstantStringType(const QualType type) {
  DXASSERT_NOMSG(type->isArrayType());
  return type->getArrayElementTypeNoTypeQual()->isSpecificBuiltinType(
      BuiltinType::Char_S);
}

bool IsPointerStringType(const QualType type) {
  DXASSERT_NOMSG(type->isPointerType());
  return type->getPointeeType()->isSpecificBuiltinType(BuiltinType::Char_S);
}

bool IsStringType(const QualType type) {
  QualType canType = type.getCanonicalType();
  return canType->isPointerType() && IsPointerStringType(canType);
}

bool IsStringLiteralType(const QualType type) {
  QualType canType = type.getCanonicalType();
  return canType->isArrayType() && IsArrayConstantStringType(canType);
}

void GetRowsAndColsForAny(QualType type, uint32_t &rowCount,
                          uint32_t &colCount) {
  assert(!type.isNull());

  type = GetStructuralForm(type);
  rowCount = 1;
  colCount = 1;
  const Type *Ty = type.getCanonicalType().getTypePtr();
  if (type->isArrayType() && !IsArrayConstantStringType(type)) {
    if (type->isConstantArrayType()) {
      const ConstantArrayType *arrayType =
          (const ConstantArrayType *)type->getAsArrayTypeUnsafe();
      colCount = arrayType->getSize().getLimitedValue();
    } else {
      colCount = 0;
    }
  } else if (const RecordType *RT = dyn_cast<RecordType>(Ty)) {
    if (const ClassTemplateSpecializationDecl *templateDecl =
            dyn_cast<ClassTemplateSpecializationDecl>(
                RT->getAsCXXRecordDecl())) {
      if (templateDecl->getName() == "matrix") {
        const TemplateArgumentList &argList = templateDecl->getTemplateArgs();
        const TemplateArgument &arg1 = argList[1];
        const TemplateArgument &arg2 = argList[2];
        llvm::APSInt rowSize = arg1.getAsIntegral();
        llvm::APSInt colSize = arg2.getAsIntegral();
        rowCount = rowSize.getLimitedValue();
        colCount = colSize.getLimitedValue();
      } else if (templateDecl->getName() == "vector") {
        const TemplateArgumentList &argList = templateDecl->getTemplateArgs();
        const TemplateArgument &arg1 = argList[1];
        llvm::APSInt rowSize = arg1.getAsIntegral();
        colCount = rowSize.getLimitedValue();
      }
    }
  }
}

void GetHLSLMatRowColCount(clang::QualType type, unsigned int &row,
                           unsigned int &col) {
  GetRowsAndColsForAny(type, row, col);
}
clang::QualType GetHLSLVecElementType(clang::QualType type) {
  type = GetStructuralForm(type);

  const Type *Ty = type.getCanonicalType().getTypePtr();
  const RecordType *RT = dyn_cast<RecordType>(Ty);
  assert(RT != nullptr && "otherwise caller shouldn't be invoking this");
  const ClassTemplateSpecializationDecl *templateDecl =
      dyn_cast<ClassTemplateSpecializationDecl>(RT->getAsCXXRecordDecl());
  assert(templateDecl != nullptr &&
         "otherwise caller shouldn't be invoking this");
  assert(templateDecl->getName() == "vector" &&
         "otherwise caller shouldn't be invoking this");

  const TemplateArgumentList &argList = templateDecl->getTemplateArgs();
  const TemplateArgument &arg0 = argList[0];
  QualType elemTy = arg0.getAsType();
  return elemTy;
}
clang::QualType GetHLSLMatElementType(clang::QualType type) {
  type = GetStructuralForm(type);

  const Type *Ty = type.getCanonicalType().getTypePtr();
  const RecordType *RT = dyn_cast<RecordType>(Ty);
  assert(RT != nullptr && "otherwise caller shouldn't be invoking this");
  const ClassTemplateSpecializationDecl *templateDecl =
      dyn_cast<ClassTemplateSpecializationDecl>(RT->getAsCXXRecordDecl());
  assert(templateDecl != nullptr &&
         "otherwise caller shouldn't be invoking this");
  assert(templateDecl->getName() == "matrix" &&
         "otherwise caller shouldn't be invoking this");

  const TemplateArgumentList &argList = templateDecl->getTemplateArgs();
  const TemplateArgument &arg0 = argList[0];
  QualType elemTy = arg0.getAsType();
  return elemTy;
}

// TODO: Add type cache to ASTContext.
bool IsHLSLInputPatchType(QualType type) {
  if (const HLSLTessPatchAttr *Attr = getAttr<HLSLTessPatchAttr>(type))
    return Attr->getIsInput();
  return false;
}

bool IsHLSLOutputPatchType(QualType type) {
  if (const HLSLTessPatchAttr *Attr = getAttr<HLSLTessPatchAttr>(type))
    return !Attr->getIsInput();
  return false;
}

bool IsHLSLPointStreamType(QualType type) {
  if (const HLSLStreamOutputAttr *Attr = getAttr<HLSLStreamOutputAttr>(type))
    return Attr->getPrimVertices() == (unsigned)DXIL::InputPrimitive::Point;
  return false;
}

bool IsHLSLLineStreamType(QualType type) {
  if (const HLSLStreamOutputAttr *Attr = getAttr<HLSLStreamOutputAttr>(type))
    return Attr->getPrimVertices() == (unsigned)DXIL::InputPrimitive::Line;
  return false;
}

bool IsHLSLTriangleStreamType(QualType type) {
  if (const HLSLStreamOutputAttr *Attr = getAttr<HLSLStreamOutputAttr>(type))
    return Attr->getPrimVertices() == (unsigned)DXIL::InputPrimitive::Triangle;
  return false;
}

bool IsHLSLStreamOutputType(QualType type) {
  if (getAttr<HLSLStreamOutputAttr>(type))
    return true;
  return false;
}

bool IsHLSLResourceType(clang::QualType type) {
  if (getAttr<HLSLResourceAttr>(type))
    return true;
  return false;
}

bool IsHLSLHitObjectType(QualType type) {
  return nullptr != getAttr<HLSLHitObjectAttr>(type);
}

DXIL::NodeIOKind GetNodeIOType(clang::QualType type) {
  if (const HLSLNodeObjectAttr *Attr = getAttr<HLSLNodeObjectAttr>(type))
    return Attr->getNodeIOType();
  return DXIL::NodeIOKind::Invalid;
}

bool IsHLSLNodeInputType(clang::QualType type) {
  return (static_cast<uint32_t>(GetNodeIOType(type)) &
          static_cast<uint32_t>(DXIL::NodeIOFlags::Input)) != 0;
}

bool IsHLSLDynamicResourceType(clang::QualType type) {
  if (const RecordType *RT = type->getAs<RecordType>()) {
    StringRef name = RT->getDecl()->getName();
    return name == ".Resource";
  }
  return false;
}

bool IsHLSLDynamicSamplerType(clang::QualType type) {
  if (const RecordType *RT = type->getAs<RecordType>()) {
    StringRef name = RT->getDecl()->getName();
    return name == ".Sampler";
  }
  return false;
}

bool IsHLSLNodeType(clang::QualType type) {
  if (const HLSLNodeObjectAttr *Attr = getAttr<HLSLNodeObjectAttr>(type))
    return true;
  return false;
}

bool IsHLSLObjectWithImplicitMemberAccess(clang::QualType type) {
  if (const HLSLResourceAttr *Attr = getAttr<HLSLResourceAttr>(type))
    return DXIL::IsCTBuffer(Attr->getResKind());
  return false;
}

bool IsHLSLObjectWithImplicitROMemberAccess(clang::QualType type) {
  if (const HLSLResourceAttr *Attr = getAttr<HLSLResourceAttr>(type))
    return DXIL::IsCTBuffer(Attr->getResKind());
  return false;
}

bool IsHLSLRWNodeInputRecordType(clang::QualType type) {
  return (static_cast<uint32_t>(GetNodeIOType(type)) &
          (static_cast<uint32_t>(DXIL::NodeIOFlags::ReadWrite) |
           static_cast<uint32_t>(DXIL::NodeIOFlags::Input))) ==
         (static_cast<uint32_t>(DXIL::NodeIOFlags::ReadWrite) |
          static_cast<uint32_t>(DXIL::NodeIOFlags::Input));
}

bool IsHLSLRONodeInputRecordType(clang::QualType type) {
  return (static_cast<uint32_t>(GetNodeIOType(type)) &
          (static_cast<uint32_t>(DXIL::NodeIOFlags::ReadWrite) |
           static_cast<uint32_t>(DXIL::NodeIOFlags::Input))) ==
         static_cast<uint32_t>(DXIL::NodeIOFlags::Input);
}

bool IsHLSLDispatchNodeInputRecordType(clang::QualType type) {
  return IsHLSLNodeInputType(type) &&
         (static_cast<uint32_t>(GetNodeIOType(type)) &
          static_cast<uint32_t>(DXIL::NodeIOFlags::DispatchRecord)) != 0;
}

bool IsHLSLNodeOutputType(clang::QualType type) {
  return (static_cast<uint32_t>(GetNodeIOType(type)) &
          (static_cast<uint32_t>(DXIL::NodeIOFlags::Output) |
           static_cast<uint32_t>(DXIL::NodeIOFlags::RecordGranularityMask))) ==
         static_cast<uint32_t>(DXIL::NodeIOFlags::Output);
}

bool IsHLSLNodeRecordArrayType(clang::QualType type) {
  if (const RecordType *RT = type->getAs<RecordType>()) {
    StringRef name = RT->getDecl()->getName();
    if (name == "ThreadNodeOutputRecords" || name == "GroupNodeOutputRecords" ||
        name == "GroupNodeInputRecords" || name == "RWGroupNodeInputRecords" ||
        name == "EmptyNodeInput")
      return true;
  }
  return false;
}

bool IsHLSLEmptyNodeRecordType(clang::QualType type) {
  return (static_cast<uint32_t>(GetNodeIOType(type)) &
          static_cast<uint32_t>(DXIL::NodeIOFlags::EmptyRecord)) ==
         static_cast<uint32_t>(DXIL::NodeIOFlags::EmptyRecord);
}

bool IsHLSLStructuredBufferType(clang::QualType type) {
  if (const HLSLResourceAttr *Attr = getAttr<HLSLResourceAttr>(type))
    return Attr->getResKind() == DXIL::ResourceKind::StructuredBuffer;
  return false;
}

bool IsHLSLSubobjectType(clang::QualType type) {
  DXIL::SubobjectKind kind;
  DXIL::HitGroupType hgType;
  return GetHLSLSubobjectKind(type, kind, hgType);
}

bool IsUserDefinedRecordType(clang::QualType QT) {
  const clang::Type *Ty = QT.getCanonicalType().getTypePtr();
  if (const RecordType *RT = dyn_cast<RecordType>(Ty)) {
    const RecordDecl *RD = RT->getDecl();
    // Built-in ray tracing struct types are considered user defined types.
    if (RD->isImplicit() && !IsHLSLBuiltinRayAttributeStruct(QT))
      return false;
    if (auto TD = dyn_cast<ClassTemplateSpecializationDecl>(RD))
      if (TD->getSpecializedTemplate()->isImplicit())
        return false;
    return true;
  }
  return false;
}

static bool HasTessFactorSemantic(const ValueDecl *decl) {
  for (const UnusualAnnotation *it : decl->getUnusualAnnotations()) {
    if (it->getKind() == UnusualAnnotation::UA_SemanticDecl) {
      const SemanticDecl *sd = cast<SemanticDecl>(it);
      StringRef semanticName;
      unsigned int index = 0;
      Semantic::DecomposeNameAndIndex(sd->SemanticName, &semanticName, &index);
      const hlsl::Semantic *pSemantic = hlsl::Semantic::GetByName(semanticName);
      if (pSemantic && pSemantic->GetKind() == hlsl::Semantic::Kind::TessFactor)
        return true;
    }
  }
  return false;
}

static bool HasTessFactorSemanticRecurse(const ValueDecl *decl, QualType Ty) {
  if (Ty->isBuiltinType() || hlsl::IsHLSLVecMatType(Ty))
    return false;

  if (const RecordType *RT = Ty->getAs<RecordType>()) {
    RecordDecl *RD = RT->getDecl();
    for (FieldDecl *fieldDecl : RD->fields()) {
      if (HasTessFactorSemanticRecurse(fieldDecl, fieldDecl->getType()))
        return true;
    }
    return false;
  }

  if (Ty->getAsArrayTypeUnsafe())
    return HasTessFactorSemantic(decl);

  return false;
}

bool IsPatchConstantFunctionDecl(const clang::FunctionDecl *FD) {
  // This checks whether the function is structurally capable of being a patch
  // constant function, not whether it is in fact the patch constant function
  // for the entry point of a compiled hull shader (which may not have been
  // seen yet). So the answer is conservative.
  if (!FD->getReturnType()->isVoidType()) {
    // Try to find TessFactor in return type.
    if (HasTessFactorSemanticRecurse(FD, FD->getReturnType()))
      return true;
  }
  // Try to find TessFactor in out param.
  for (const ParmVarDecl *param : FD->params()) {
    if (param->hasAttr<HLSLOutAttr>()) {
      if (HasTessFactorSemanticRecurse(param, param->getType()))
        return true;
    }
  }
  return false;
}

bool DoesTypeDefineOverloadedOperator(clang::QualType typeWithOperator,
                                      clang::OverloadedOperatorKind opc,
                                      clang::QualType paramType) {
  if (const RecordType *recordType = typeWithOperator->getAs<RecordType>()) {
    if (const CXXRecordDecl *cxxRecordDecl =
            dyn_cast<CXXRecordDecl>(recordType->getDecl())) {
      for (const auto *method : cxxRecordDecl->methods()) {
        if (!method->isUserProvided() || method->getNumParams() != 1)
          continue;
        // It must be an implicit assignment.
        if (opc == OO_Equal &&
            typeWithOperator != method->getParamDecl(0)->getOriginalType() &&
            typeWithOperator == paramType) {
          continue;
        }
        if (method->getOverloadedOperator() == opc)
          return true;
      }
    }
  }
  return false;
}

bool GetHLSLSubobjectKind(clang::QualType type,
                          DXIL::SubobjectKind &subobjectKind,
                          DXIL::HitGroupType &hgType) {
  type = type.getCanonicalType();
  if (const RecordType *RT = type->getAs<RecordType>()) {
    RecordDecl *RD = RT->getDecl();
    if (!RD->hasAttr<HLSLSubObjectAttr>()) {
      return false;
    }

    HLSLSubObjectAttr *Attr = RD->getAttr<HLSLSubObjectAttr>();
    subobjectKind = static_cast<DXIL::SubobjectKind>(Attr->getSubObjKindUint());
    hgType = static_cast<DXIL::HitGroupType>(Attr->getHitGroupType());
    if (subobjectKind == DXIL::SubobjectKind::HitGroup)
      DXASSERT(DXIL::IsValidHitGroupType(hgType), "invalid hit group type");

    return true;
  }
  return false;
}

clang::RecordDecl *GetRecordDeclFromNodeObjectType(clang::QualType ObjectTy) {
  ObjectTy = ObjectTy.getCanonicalType();
  DXASSERT(IsHLSLNodeType(ObjectTy), "Expected Node Object type");
  if (const CXXRecordDecl *CXXRD = ObjectTy->getAsCXXRecordDecl()) {

    if (const ClassTemplateSpecializationDecl *templateDecl =
            dyn_cast<ClassTemplateSpecializationDecl>(CXXRD)) {

      auto &TemplateArgs = templateDecl->getTemplateArgs();
      clang::QualType RecType = TemplateArgs[0].getAsType();
      if (const RecordType *RT = RecType->getAs<RecordType>())
        return RT->getDecl();
    }
  }

  return nullptr;
}

bool IsHLSLRayQueryType(clang::QualType type) {
  type = type.getCanonicalType();
  if (const RecordType *RT = dyn_cast<RecordType>(type)) {
    if (const ClassTemplateSpecializationDecl *templateDecl =
            dyn_cast<ClassTemplateSpecializationDecl>(
                RT->getAsCXXRecordDecl())) {
      StringRef name = templateDecl->getName();
      if (name == "RayQuery")
        return true;
    }
  }
  return false;
}

#ifdef ENABLE_SPIRV_CODEGEN
static llvm::Optional<std::pair<clang::QualType, unsigned>>
MaybeGetVKBufferPointerParams(clang::QualType type) {
  const RecordType *RT = dyn_cast<RecordType>(type.getCanonicalType());
  if (!RT)
    return llvm::None;

  const ClassTemplateSpecializationDecl *templateDecl =
      dyn_cast<ClassTemplateSpecializationDecl>(RT->getAsCXXRecordDecl());
  if (!templateDecl || !templateDecl->getName().equals("BufferPointer"))
    return llvm::None;

  auto *namespaceDecl =
      dyn_cast_or_null<NamespaceDecl>(templateDecl->getDeclContext());
  if (!namespaceDecl || !namespaceDecl->getName().equals("vk"))
    return llvm::None;

  const TemplateArgumentList &argList = templateDecl->getTemplateArgs();
  QualType bufferType = argList[0].getAsType();
  unsigned align =
      argList.size() > 1 ? argList[1].getAsIntegral().getLimitedValue() : 0;
  return std::make_pair(bufferType, align);
}

bool IsVKBufferPointerType(clang::QualType type) {
  return MaybeGetVKBufferPointerParams(type).hasValue();
}

QualType GetVKBufferPointerBufferType(clang::QualType type) {
  auto bpParams = MaybeGetVKBufferPointerParams(type);
  assert(bpParams.hasValue() &&
         "cannot get pointer type for type that is not a vk::BufferPointer");
  return bpParams.getValue().first;
}

unsigned GetVKBufferPointerAlignment(clang::QualType type) {
  auto bpParams = MaybeGetVKBufferPointerParams(type);
  assert(
      bpParams.hasValue() &&
      "cannot get pointer alignment for type that is not a vk::BufferPointer");
  return bpParams.getValue().second;
}
#endif

QualType GetHLSLResourceResultType(QualType type) {
  // Don't canonicalize the type as to not lose snorm in Buffer<snorm float>
  const RecordType *RT = type->getAs<RecordType>();
  const RecordDecl *RD = RT->getDecl();

  if (const ClassTemplateSpecializationDecl *templateDecl =
          dyn_cast<ClassTemplateSpecializationDecl>(RD)) {

    const HLSLResourceAttr *Attr = getAttr<HLSLResourceAttr>(type);
    if (Attr && DXIL::IsFeedbackTexture(Attr->getResKind())) {
      // Feedback textures are write-only and the data is opaque,
      // so there is no result type per se.
      return {};
    }

    // Type-templated resource types

    // Prefer getting the template argument from the TemplateSpecializationType
    // sugar, since this preserves 'snorm' from 'Buffer<snorm float>' which is
    // lost on the ClassTemplateSpecializationDecl since it's considered type
    // sugar.
    const TemplateArgument *templateArg = &templateDecl->getTemplateArgs()[0];
    if (const TemplateSpecializationType *specializationType =
            type->getAs<TemplateSpecializationType>()) {
      if (specializationType->getNumArgs() >= 1) {
        templateArg = &specializationType->getArg(0);
      }
    }

    if (templateArg->getKind() == TemplateArgument::ArgKind::Type)
      return templateArg->getAsType();
  }

  // Non-type-templated resource types like [RW][RasterOrder]ByteAddressBuffer
  // Get the result type from handle field.
  FieldDecl *HandleFieldDecl = *(RD->field_begin());
  DXASSERT(HandleFieldDecl->getName() == "h",
           "Resource must have a handle field");
  return HandleFieldDecl->getType();
}

QualType GetHLSLNodeIOResultType(ASTContext &astContext, QualType type) {
  if (hlsl::IsHLSLEmptyNodeRecordType(type)) {
    RecordDecl *RD = astContext.buildImplicitRecord("");
    RD->startDefinition();
    RD->completeDefinition();
    return astContext.getRecordType(RD);
  } else if (hlsl::IsHLSLNodeType(type)) {
    const RecordType *recordType = type->getAs<RecordType>();
    if (const auto *templateDecl =
            dyn_cast<ClassTemplateSpecializationDecl>(recordType->getDecl())) {
      const auto &templateArgs = templateDecl->getTemplateArgs();
      return templateArgs[0].getAsType();
    }
  }
  return type;
}

unsigned GetHLSLResourceTemplateUInt(clang::QualType type) {
  const ClassTemplateSpecializationDecl *templateDecl =
      cast<ClassTemplateSpecializationDecl>(
          type->castAs<RecordType>()->getDecl());
  return (unsigned)templateDecl->getTemplateArgs()[0]
      .getAsIntegral()
      .getZExtValue();
}

bool IsIncompleteHLSLResourceArrayType(clang::ASTContext &context,
                                       clang::QualType type) {
  if (type->isIncompleteArrayType()) {
    const IncompleteArrayType *IAT = context.getAsIncompleteArrayType(type);
    type = IAT->getElementType();
  }

  while (type->isArrayType())
    type = cast<ArrayType>(type)->getElementType();

  if (IsHLSLResourceType(type))
    return true;
  return false;
}

QualType GetHLSLResourceTemplateParamType(QualType type) {
  type = type.getCanonicalType();
  const RecordType *RT = cast<RecordType>(type);
  const ClassTemplateSpecializationDecl *templateDecl =
      cast<ClassTemplateSpecializationDecl>(RT->getAsCXXRecordDecl());
  const TemplateArgumentList &argList = templateDecl->getTemplateArgs();
  return argList[0].getAsType();
}

QualType GetHLSLInputPatchElementType(QualType type) {
  return GetHLSLResourceTemplateParamType(type);
}

unsigned GetHLSLInputPatchCount(QualType type) {
  type = type.getCanonicalType();
  const RecordType *RT = cast<RecordType>(type);
  const ClassTemplateSpecializationDecl *templateDecl =
      cast<ClassTemplateSpecializationDecl>(RT->getAsCXXRecordDecl());
  const TemplateArgumentList &argList = templateDecl->getTemplateArgs();
  return argList[1].getAsIntegral().getLimitedValue();
}
clang::QualType GetHLSLOutputPatchElementType(QualType type) {
  return GetHLSLResourceTemplateParamType(type);
}
unsigned GetHLSLOutputPatchCount(QualType type) {
  type = type.getCanonicalType();
  const RecordType *RT = cast<RecordType>(type);
  const ClassTemplateSpecializationDecl *templateDecl =
      cast<ClassTemplateSpecializationDecl>(RT->getAsCXXRecordDecl());
  const TemplateArgumentList &argList = templateDecl->getTemplateArgs();
  return argList[1].getAsIntegral().getLimitedValue();
}

bool IsParamAttributedAsOut(clang::AttributeList *pAttributes, bool *pIsIn) {
  bool anyFound = false;
  bool inFound = false;
  bool outFound = false;
  while (pAttributes != nullptr) {
    switch (pAttributes->getKind()) {
    case AttributeList::AT_HLSLIn:
      anyFound = true;
      inFound = true;
      break;
    case AttributeList::AT_HLSLOut:
      anyFound = true;
      outFound = true;
      break;
    case AttributeList::AT_HLSLInOut:
      anyFound = true;
      outFound = true;
      inFound = true;
      break;
    default:
      // Ignore the majority of attributes that don't have in/out
      // characteristics
      break;
    }
    pAttributes = pAttributes->getNext();
  }
  if (pIsIn)
    *pIsIn = inFound || anyFound == false;
  return outFound;
}

hlsl::ParameterModifier
ParamModFromAttributeList(clang::AttributeList *pAttributes) {
  bool isIn, isOut;
  isOut = IsParamAttributedAsOut(pAttributes, &isIn);
  return ParameterModifier::FromInOut(isIn, isOut);
}

HLSLScalarType MakeUnsigned(HLSLScalarType T) {
  switch (T) {
  case HLSLScalarType_int:
    return HLSLScalarType_uint;
  case HLSLScalarType_int_min16:
    return HLSLScalarType_uint_min16;
  case HLSLScalarType_int64:
    return HLSLScalarType_uint64;
  case HLSLScalarType_int16:
    return HLSLScalarType_uint16;
  default:
    // Only signed int types are relevant.
    break;
  }
  return T;
}

} // namespace hlsl
