//===--- HlslTypes.h  - Type system for HLSL                 ----*- C++ -*-===//
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// HlslTypes.h                                                               //
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

#ifndef LLVM_CLANG_AST_HLSLTYPES_H
#define LLVM_CLANG_AST_HLSLTYPES_H

#include "dxc/DXIL/DxilConstants.h"
#include "dxc/DXIL/DxilNodeProps.h"
#include "dxc/WinAdapter.h"
#include "clang/AST/DeclarationName.h"
#include "clang/AST/Type.h" // needs QualType
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/Specifiers.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Casting.h"

namespace clang {
class ASTContext;
class AttributeList;
class CXXConstructorDecl;
class CXXMethodDecl;
class CXXRecordDecl;
class ClassTemplateDecl;
class ExtVectorType;
class FunctionDecl;
class FunctionTemplateDecl;
class InheritableAttr;
class NamedDecl;
class ParmVarDecl;
class Sema;
class TypeSourceInfo;
class TypedefDecl;
class VarDecl;
} // namespace clang

namespace hlsl {

/// <summary>Initializes the specified context to support HLSL
/// compilation.</summary>
void InitializeASTContextForHLSL(clang::ASTContext &context);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Type system enumerations.

/// <summary>Scalar types for HLSL identified by a single keyword.</summary>
enum HLSLScalarType {
  HLSLScalarType_unknown,
  HLSLScalarType_bool,
  HLSLScalarType_int,
  HLSLScalarType_uint,
  HLSLScalarType_dword,
  HLSLScalarType_half,
  HLSLScalarType_float,
  HLSLScalarType_double,
  HLSLScalarType_float_min10,
  HLSLScalarType_float_min16,
  HLSLScalarType_int_min12,
  HLSLScalarType_int_min16,
  HLSLScalarType_uint_min16,
  HLSLScalarType_float_lit,
  HLSLScalarType_int_lit,
  HLSLScalarType_int16,
  HLSLScalarType_int32,
  HLSLScalarType_int64,
  HLSLScalarType_uint16,
  HLSLScalarType_uint32,
  HLSLScalarType_uint64,
  HLSLScalarType_float16,
  HLSLScalarType_float32,
  HLSLScalarType_float64,
  HLSLScalarType_int8_4packed,
  HLSLScalarType_uint8_4packed
};

HLSLScalarType MakeUnsigned(HLSLScalarType T);

static const HLSLScalarType HLSLScalarType_minvalid = HLSLScalarType_bool;
static const HLSLScalarType HLSLScalarType_max = HLSLScalarType_uint8_4packed;
static const size_t HLSLScalarTypeCount =
    static_cast<size_t>(HLSLScalarType_max) + 1;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Type annotations and descriptors.

struct MatrixMemberAccessPositions {
  uint32_t IsValid : 1; // Whether the member access is valid.
  uint32_t Count : 3;   // Count of row/col pairs.
  uint32_t R0_Row : 2;  // Zero-based row index for first position.
  uint32_t R0_Col : 2;  // Zero-based column index for first position.
  uint32_t R1_Row : 2;  // ...
  uint32_t R1_Col : 2;
  uint32_t R2_Row : 2;
  uint32_t R2_Col : 2;
  uint32_t R3_Row : 2;
  uint32_t R3_Col : 2;

  bool ContainsDuplicateElements() const {
    return IsValid &&
           ((Count > 1 && ((R1_Row == R0_Row && R1_Col == R0_Col))) ||
            (Count > 2 && ((R2_Row == R0_Row && R2_Col == R0_Col) ||
                           (R2_Row == R1_Row && R2_Col == R1_Col))) ||
            (Count > 3 && ((R3_Row == R0_Row && R3_Col == R0_Col) ||
                           (R3_Row == R1_Row && R3_Col == R1_Col) ||
                           (R3_Row == R2_Row && R3_Col == R2_Col))));
  }

  void GetPosition(uint32_t index, uint32_t *row, uint32_t *col) const {
    assert(index < 4);
    switch (index) {
    case 0:
      *row = R0_Row;
      *col = R0_Col;
      break;
    case 1:
      *row = R1_Row;
      *col = R1_Col;
      break;
    case 2:
      *row = R2_Row;
      *col = R2_Col;
      break;
    default:
    case 3:
      *row = R3_Row;
      *col = R3_Col;
      break;
    }
    assert(*row <= 3);
    assert(*col <= 3);
  }

  void SetPosition(uint32_t index, uint32_t row, uint32_t col) {
    assert(index < 4);
    assert(row <= 3);
    assert(col <= 3);
    switch (index) {
    case 0:
      R0_Row = row;
      R0_Col = col;
      break;
    case 1:
      R1_Row = row;
      R1_Col = col;
      break;
    case 2:
      R2_Row = row;
      R2_Col = col;
      break;
    default:
    case 3:
      R3_Row = row;
      R3_Col = col;
      break;
    }
  }
};

struct VectorMemberAccessPositions {
  uint32_t IsValid : 1; // Whether the member access is valid.
  uint32_t Count : 3;   // Count of swizzle components.
  uint32_t Swz0 : 2;    // Zero-based swizzle index for first position.
  uint32_t Swz1 : 2;
  uint32_t Swz2 : 2;
  uint32_t Swz3 : 2;

  bool ContainsDuplicateElements() const {
    return IsValid && ((Count > 1 && (Swz1 == Swz0)) ||
                       (Count > 2 && ((Swz2 == Swz0) || (Swz2 == Swz1))) ||
                       (Count > 3 &&
                        ((Swz3 == Swz0) || (Swz3 == Swz1) || (Swz3 == Swz2))));
  }

  void GetPosition(uint32_t index, uint32_t *col) const {
    assert(index < 4);
    switch (index) {
    case 0:
      *col = Swz0;
      break;
    case 1:
      *col = Swz1;
      break;
    case 2:
      *col = Swz2;
      break;
    default:
    case 3:
      *col = Swz3;
      break;
    }
    assert(*col <= 3);
  }

  void SetPosition(uint32_t index, uint32_t col) {
    assert(index < 4);
    assert(col <= 3);
    switch (index) {
    case 0:
      Swz0 = col;
      break;
    case 1:
      Swz1 = col;
      break;
    case 2:
      Swz2 = col;
      break;
    default:
    case 3:
      Swz3 = col;
      break;
    }
  }
};

/// <summary>Base class for annotations that are rarely used.</summary>
struct UnusualAnnotation {
public:
  enum UnusualAnnotationKind {
    UA_RegisterAssignment,
    UA_ConstantPacking,
    UA_SemanticDecl,
    UA_PayloadAccessQualifier
  };

private:
  const UnusualAnnotationKind Kind;

public:
  UnusualAnnotation(UnusualAnnotationKind kind) : Kind(kind), Loc() {}
  UnusualAnnotation(UnusualAnnotationKind kind, clang::SourceLocation loc)
      : Kind(kind), Loc(loc) {}
  UnusualAnnotation(const UnusualAnnotation &other)
      : Kind(other.Kind), Loc(other.Loc) {}
  UnusualAnnotationKind getKind() const { return Kind; }

  UnusualAnnotation *CopyToASTContext(clang::ASTContext &Context);
  static llvm::ArrayRef<UnusualAnnotation *>
  CopyToASTContextArray(clang::ASTContext &Context, UnusualAnnotation **begin,
                        size_t count);

  /// <summary>Location where the annotation was parsed.</summary>
  clang::SourceLocation Loc;
};

/// <summary>Use this structure to capture a ': register' definition.</summary>
struct RegisterAssignment : public UnusualAnnotation {
  /// <summary>Initializes a new RegisterAssignment in invalid state.</summary>
  RegisterAssignment() : UnusualAnnotation(UA_RegisterAssignment) {}

  llvm::StringRef ShaderProfile;
  bool IsValid = false;
  char RegisterType = 0;       // Lower-case letter, 0 if not explicitly set
  uint32_t RegisterNumber = 0; // Iff RegisterType != 0
  llvm::Optional<uint32_t>
      RegisterSpace; // Set only if explicit "spaceN" syntax
  uint32_t RegisterOffset = 0;

  void setIsValid(bool value) { IsValid = value; }

  bool isSpaceOnly() const {
    return RegisterType == 0 && RegisterSpace.hasValue();
  }

  static bool classof(const UnusualAnnotation *UA) {
    return UA->getKind() == UA_RegisterAssignment;
  }
};

// <summary>Use this structure to capture a ': in/out' definiton.</summary>
struct PayloadAccessAnnotation : public UnusualAnnotation {
  /// <summary>Initializes a new PayloadAccessAnnotation in invalid
  /// state.</summary>
  PayloadAccessAnnotation() : UnusualAnnotation(UA_PayloadAccessQualifier){};

  DXIL::PayloadAccessQualifier qualifier =
      DXIL::PayloadAccessQualifier::NoAccess;

  llvm::SmallVector<DXIL::PayloadAccessShaderStage, 4> ShaderStages;

  static bool classof(const UnusualAnnotation *UA) {
    return UA->getKind() == UA_PayloadAccessQualifier;
  }
};

/// <summary>Use this structure to capture a ': packoffset'
/// definition.</summary>
struct ConstantPacking : public UnusualAnnotation {
  /// <summary>Initializes a new ConstantPacking in invalid state.</summary>
  ConstantPacking()
      : UnusualAnnotation(UA_ConstantPacking), Subcomponent(0),
        ComponentOffset(0), IsValid(0) {}

  uint32_t Subcomponent;        // Subcomponent specified.
  unsigned ComponentOffset : 2; // 0-3 for the offset specified.
  unsigned IsValid : 1;         // Whether the declaration is valid.

  void setIsValid(bool value) { IsValid = value ? 1 : 0; }

  static bool classof(const UnusualAnnotation *UA) {
    return UA->getKind() == UA_ConstantPacking;
  }
};

/// <summary>Use this structure to capture a ': SEMANTIC' definition.</summary>
struct SemanticDecl : public UnusualAnnotation {
  /// <summary>Initializes a new SemanticDecl in invalid state.</summary>
  SemanticDecl() : UnusualAnnotation(UA_SemanticDecl), SemanticName() {}

  /// <summary>Initializes a new SemanticDecl with the specified name.</summary>
  SemanticDecl(llvm::StringRef name)
      : UnusualAnnotation(UA_SemanticDecl), SemanticName(name) {}

  /// <summary>Name for semantic.</summary>
  llvm::StringRef SemanticName;

  static bool classof(const UnusualAnnotation *UA) {
    return UA->getKind() == UA_SemanticDecl;
  }
};

/// Returns a ParameterModifier initialized as per the attribute list.
ParameterModifier ParamModFromAttributeList(clang::AttributeList *pAttributes);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AST manipulation functions.

void AddHLSLMatrixTemplate(clang::ASTContext &context,
                           clang::ClassTemplateDecl *vectorTemplateDecl,
                           clang::ClassTemplateDecl **matrixTemplateDecl);

void AddHLSLVectorTemplate(clang::ASTContext &context,
                           clang::ClassTemplateDecl **vectorTemplateDecl);

void AddHLSLNodeOutputRecordTemplate(
    clang::ASTContext &context, DXIL::NodeIOKind Type,
    _Outptr_ clang::ClassTemplateDecl **outputRecordTemplateDecl,
    bool isCompleteType = true);

clang::CXXRecordDecl *
DeclareRecordTypeWithHandle(clang::ASTContext &context, llvm::StringRef name,
                            bool isCompleteType = true,
                            clang::InheritableAttr *Attr = nullptr);

void AddRaytracingConstants(clang::ASTContext &context);
void AddSamplerFeedbackConstants(clang::ASTContext &context);
void AddBarrierConstants(clang::ASTContext &context);

/// <summary>Adds the implementation for std::is_equal.</summary>
void AddStdIsEqualImplementation(clang::ASTContext &context, clang::Sema &sema);

/// <summary>
/// Adds a new template type in the specified context with the given name. The
/// record type will have a handle field.
/// </summary>
/// <parm name="context">AST context to which template will be added.</param>
/// <parm name="templateArgCount">Number of template arguments (one or
/// two).</param> <parm name="defaultTypeArgValue">If assigned, the default
/// argument for the element template.</param>
clang::CXXRecordDecl *DeclareTemplateTypeWithHandle(
    clang::ASTContext &context, llvm::StringRef name,
    uint8_t templateArgCount = 1,
    clang::TypeSourceInfo *defaultTypeArgValue = nullptr,
    clang::InheritableAttr *Attr = nullptr);

clang::CXXRecordDecl *DeclareTemplateTypeWithHandleInDeclContext(
    clang::ASTContext &context, clang::DeclContext *declContext,
    llvm::StringRef name, uint8_t templateArgCount,
    clang::TypeSourceInfo *defaultTypeArgValue,
    clang::InheritableAttr *Attr = nullptr);

clang::CXXRecordDecl *DeclareUIntTemplatedTypeWithHandle(
    clang::ASTContext &context, llvm::StringRef typeName,
    llvm::StringRef templateParamName, clang::InheritableAttr *Attr = nullptr);
clang::CXXRecordDecl *DeclareUIntTemplatedTypeWithHandleInDeclContext(
    clang::ASTContext &context, clang::DeclContext *declContext,
    llvm::StringRef typeName, llvm::StringRef templateParamName,
    clang::InheritableAttr *Attr = nullptr);
clang::CXXRecordDecl *
DeclareConstantBufferViewType(clang::ASTContext &context,
                              clang::InheritableAttr *Attr);
clang::CXXRecordDecl *DeclareRayQueryType(clang::ASTContext &context);
clang::CXXRecordDecl *DeclareHitObjectType(clang::NamespaceDecl &NSDecl);
clang::CXXRecordDecl *DeclareResourceType(clang::ASTContext &context,
                                          bool bSampler);

clang::CXXRecordDecl *
DeclareNodeOrRecordType(clang::ASTContext &Ctx, DXIL::NodeIOKind Type,
                        bool IsRecordTypeTemplate = false, bool IsConst = false,
                        bool HasGetMethods = false, bool IsArray = false,
                        bool IsCompleteType = false);

#ifdef ENABLE_SPIRV_CODEGEN
clang::CXXRecordDecl *
DeclareVkBufferPointerType(clang::ASTContext &context,
                           clang::DeclContext *declContext);

clang::CXXRecordDecl *DeclareInlineSpirvType(clang::ASTContext &context,
                                             clang::DeclContext *declContext,
                                             llvm::StringRef typeName,
                                             bool opaque);
clang::CXXRecordDecl *DeclareVkIntegralConstant(
    clang::ASTContext &context, clang::DeclContext *declContext,
    llvm::StringRef typeName, clang::ClassTemplateDecl **templateDecl);
#endif

clang::CXXRecordDecl *DeclareNodeOutputArray(clang::ASTContext &Ctx,
                                             DXIL::NodeIOKind Type,
                                             clang::CXXRecordDecl *OutputType,
                                             bool IsRecordTypeTemplate);

clang::CXXRecordDecl *
DeclareRecordTypeWithHandleAndNoMemberFunctions(clang::ASTContext &context,
                                                llvm::StringRef name);

clang::VarDecl *DeclareBuiltinGlobal(llvm::StringRef name, clang::QualType Ty,
                                     clang::ASTContext &context);

/// <summary>Create a function template declaration for the specified
/// method.</summary> <param name="context">AST context in which to
/// work.</param> <param name="recordDecl">Class in which the function template
/// is declared.</param> <param name="functionDecl">Function for which a
/// template is created.</param> <param
/// name="templateParamNamedDecls">Declarations for templates to the
/// function.</param> <param name="templateParamNamedDeclsCount">Count of
/// template declarations.</param> <returns>A new function template declaration
/// already declared in the class scope.</returns>
clang::FunctionTemplateDecl *
CreateFunctionTemplateDecl(clang::ASTContext &context,
                           clang::CXXRecordDecl *recordDecl,
                           clang::CXXMethodDecl *functionDecl,
                           clang::NamedDecl **templateParamNamedDecls,
                           size_t templateParamNamedDeclsCount);

clang::TypedefDecl *CreateMatrixSpecializationShorthand(
    clang::ASTContext &context, clang::QualType matrixSpecialization,
    HLSLScalarType scalarType, size_t rowCount, size_t colCount);

clang::TypedefDecl *
CreateVectorSpecializationShorthand(clang::ASTContext &context,
                                    clang::QualType vectorSpecialization,
                                    HLSLScalarType scalarType, size_t colCount);

const clang::ExtVectorType *
ConvertHLSLVecMatTypeToExtVectorType(const clang::ASTContext &,
                                     clang::QualType);
bool IsHLSLVecMatType(clang::QualType);
clang::RecordDecl *GetRecordDeclFromNodeObjectType(clang::QualType ObjectTy);
bool IsHLSLVecType(clang::QualType type);
bool IsHLSLMatType(clang::QualType type);
clang::QualType GetElementTypeOrType(clang::QualType type);
bool HasHLSLMatOrientation(clang::QualType type, bool *pIsRowMajor = nullptr);
bool IsHLSLMatRowMajor(clang::QualType type, bool defaultValue);
bool IsHLSLUnsigned(clang::QualType type);
bool IsHLSLMinPrecision(clang::QualType type);
bool HasHLSLUNormSNorm(clang::QualType type, bool *pIsSNorm = nullptr);
bool HasHLSLGloballyCoherent(clang::QualType type);
bool HasHLSLReorderCoherent(clang::QualType type);
bool IsHLSLInputPatchType(clang::QualType type);
bool IsHLSLOutputPatchType(clang::QualType type);
bool IsHLSLPointStreamType(clang::QualType type);
bool IsHLSLLineStreamType(clang::QualType type);
bool IsHLSLTriangleStreamType(clang::QualType type);
bool IsHLSLStreamOutputType(clang::QualType type);
bool IsHLSLResourceType(clang::QualType type);
bool IsHLSLNodeInputType(clang::QualType type);
bool IsHLSLDynamicResourceType(clang::QualType type);
bool IsHLSLDynamicSamplerType(clang::QualType type);
bool IsHLSLNodeType(clang::QualType type);
bool IsHLSLHitObjectType(clang::QualType type);

bool IsHLSLObjectWithImplicitMemberAccess(clang::QualType type);
bool IsHLSLObjectWithImplicitROMemberAccess(clang::QualType type);
bool IsHLSLRWNodeInputRecordType(clang::QualType type);
bool IsHLSLRONodeInputRecordType(clang::QualType type);
bool IsHLSLDispatchNodeInputRecordType(clang::QualType type);
bool IsHLSLNodeRecordArrayType(clang::QualType type);
bool IsHLSLNodeOutputType(clang::QualType type);
bool IsHLSLEmptyNodeRecordType(clang::QualType type);

DXIL::NodeIOKind GetNodeIOType(clang::QualType type);

bool IsHLSLStructuredBufferType(clang::QualType type);
bool IsHLSLNumericOrAggregateOfNumericType(clang::QualType type);
bool IsHLSLCopyableAnnotatableRecord(clang::QualType QT);
bool IsHLSLBuiltinRayAttributeStruct(clang::QualType QT);
bool IsHLSLAggregateType(clang::QualType type);
clang::QualType GetHLSLResourceResultType(clang::QualType type);
clang::QualType GetHLSLNodeIOResultType(clang::ASTContext &astContext,
                                        clang::QualType type);
unsigned GetHLSLResourceTemplateUInt(clang::QualType type);
bool IsIncompleteHLSLResourceArrayType(clang::ASTContext &context,
                                       clang::QualType type);
clang::QualType GetHLSLResourceTemplateParamType(clang::QualType type);
clang::QualType GetHLSLInputPatchElementType(clang::QualType type);
unsigned GetHLSLInputPatchCount(clang::QualType type);
clang::QualType GetHLSLOutputPatchElementType(clang::QualType type);
unsigned GetHLSLOutputPatchCount(clang::QualType type);

bool IsHLSLSubobjectType(clang::QualType type);
bool GetHLSLSubobjectKind(clang::QualType type,
                          DXIL::SubobjectKind &subobjectKind,
                          DXIL::HitGroupType &ghType);
bool IsHLSLRayQueryType(clang::QualType type);
bool GetHLSLNodeIORecordType(const clang::ParmVarDecl *parmDecl,
                             NodeFlags &nodeKind);

bool IsArrayConstantStringType(const clang::QualType type);
bool IsPointerStringType(const clang::QualType type);
bool IsStringType(const clang::QualType type);
bool IsStringLiteralType(const clang::QualType type);

void GetRowsAndColsForAny(clang::QualType type, uint32_t &rowCount,
                          uint32_t &colCount);
uint32_t GetElementCount(clang::QualType type);
uint32_t GetArraySize(clang::QualType type);
uint32_t GetHLSLVecSize(clang::QualType type);
void GetRowsAndCols(clang::QualType type, uint32_t &rowCount,
                    uint32_t &colCount);
void GetHLSLMatRowColCount(clang::QualType type, uint32_t &row, uint32_t &col);
clang::QualType GetHLSLMatElementType(clang::QualType type);
clang::QualType GetHLSLVecElementType(clang::QualType type);
bool IsIntrinsicOp(const clang::FunctionDecl *FD);
bool GetIntrinsicOp(const clang::FunctionDecl *FD, unsigned &opcode,
                    llvm::StringRef &group);
bool GetIntrinsicLowering(const clang::FunctionDecl *FD, llvm::StringRef &S);

bool IsUserDefinedRecordType(clang::QualType type);
bool DoesTypeDefineOverloadedOperator(clang::QualType typeWithOperator,
                                      clang::OverloadedOperatorKind opc,
                                      clang::QualType paramType);
bool IsPatchConstantFunctionDecl(const clang::FunctionDecl *FD);

#ifdef ENABLE_SPIRV_CODEGEN
bool IsVKBufferPointerType(clang::QualType type);
clang::QualType GetVKBufferPointerBufferType(clang::QualType type);
unsigned GetVKBufferPointerAlignment(clang::QualType type);
#endif

/// <summary>Adds a constructor declaration to the specified class
/// record.</summary> <param name="context">ASTContext that owns
/// declarations.</param> <param name="recordDecl">Record declaration in which
/// to add constructor.</param> <param name="resultType">Result type for
/// constructor.</param> <param name="paramTypes">Types for constructor
/// parameters.</param> <param name="paramNames">Names for constructor
/// parameters.</param> <param name="declarationName">Name for
/// constructor.</param> <param name="isConst">Whether the constructor is a
/// const function.</param> <returns>The method declaration for the
/// constructor.</returns>
clang::CXXConstructorDecl *CreateConstructorDeclarationWithParams(
    clang::ASTContext &context, clang::CXXRecordDecl *recordDecl,
    clang::QualType resultType, llvm::ArrayRef<clang::QualType> paramTypes,
    llvm::ArrayRef<clang::StringRef> paramNames,
    clang::DeclarationName declarationName, bool isConst,
    bool isTemplateFunction = false);

/// <summary>Adds a function declaration to the specified class
/// record.</summary> <param name="context">ASTContext that owns
/// declarations.</param> <param name="recordDecl">Record declaration in which
/// to add function.</param> <param name="resultType">Result type for
/// function.</param> <param name="paramTypes">Types for function
/// parameters.</param> <param name="paramNames">Names for function
/// parameters.</param> <param name="declarationName">Name for function.</param>
/// <param name="isConst">Whether the function is a const function.</param>
/// <returns>The method declaration for the function.</returns>
clang::CXXMethodDecl *CreateObjectFunctionDeclarationWithParams(
    clang::ASTContext &context, clang::CXXRecordDecl *recordDecl,
    clang::QualType resultType, llvm::ArrayRef<clang::QualType> paramTypes,
    llvm::ArrayRef<clang::StringRef> paramNames,
    clang::DeclarationName declarationName, bool isConst,
    clang::StorageClass SC = clang::StorageClass::SC_None,
    bool isTemplateFunction = false);

DXIL::ResourceClass GetResourceClassForType(const clang::ASTContext &context,
                                            clang::QualType Ty);

bool TryParseMatrixShorthand(const char *typeName, size_t typeNameLen,
                             HLSLScalarType *parsedType, int *rowCount,
                             int *colCount,
                             const clang::LangOptions &langOption);

bool TryParseVectorShorthand(const char *typeName, size_t typeNameLen,
                             HLSLScalarType *parsedType, int *elementCount,
                             const clang::LangOptions &langOption);

bool TryParseScalar(const char *typeName, size_t typeNameLen,
                    HLSLScalarType *parsedType,
                    const clang::LangOptions &langOption);

bool TryParseAny(const char *typeName, size_t typeNameLen,
                 HLSLScalarType *parsedType, int *rowCount, int *colCount,
                 const clang::LangOptions &langOption);

bool TryParseString(const char *typeName, size_t typeNameLen,
                    const clang::LangOptions &langOptions);

bool TryParseMatrixOrVectorDimension(const char *typeName, size_t typeNameLen,
                                     int *rowCount, int *colCount,
                                     const clang::LangOptions &langOption);

} // namespace hlsl
#endif
