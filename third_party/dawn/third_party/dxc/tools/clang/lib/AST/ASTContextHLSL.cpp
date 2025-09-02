//===--- ASTContextHLSL.cpp - HLSL support for AST nodes and operations ---===//
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// ASTContextHLSL.cpp                                                        //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
//  This file implements the ASTContext interface for HLSL.                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/DXIL/DxilSemantic.h"
#include "dxc/HLSL/HLOperations.h"
#include "dxc/HlslIntrinsicOp.h"
#include "dxc/Support/Global.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/ExternalASTSource.h"
#include "clang/AST/HlslBuiltinTypeDeclBuilder.h"
#include "clang/AST/TypeLoc.h"
#include "clang/Basic/Specifiers.h"
#include "clang/Sema/Overload.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/SemaDiagnostic.h"

using namespace clang;
using namespace hlsl;

static const int FirstTemplateDepth = 0;
static const int FirstParamPosition = 0;
static const bool ForConstFalse =
    false; // a construct is targeting a const type
static const bool ForConstTrue =
    true; // a construct is targeting a non-const type
static const bool ParameterPackFalse =
    false; // template parameter is not an ellipsis.
static const bool TypenameFalse =
    false; // 'typename' specified rather than 'class' for a template argument.
static const bool DelayTypeCreationTrue =
    true;                          // delay type creation for a declaration
static const SourceLocation NoLoc; // no source location attribution available
static const bool InlineFalse = false; // namespace is not an inline namespace
static const bool InlineSpecifiedFalse =
    false; // function was not specified as inline
static const bool ExplicitFalse =
    false; // constructor was not specified as explicit
static const bool IsConstexprFalse = false; // function is not constexpr
static const bool VirtualFalse =
    false; // whether the base class is declares 'virtual'
static const bool BaseClassFalse =
    false; // whether the base class is declared as 'class' (vs. 'struct')

/// <summary>Names of HLSLScalarType enumeration values, in matching order to
/// HLSLScalarType.</summary>
const char *HLSLScalarTypeNames[] = {
    "<unknown>",      "bool",           "int",         "uint",
    "dword",          "half",           "float",       "double",
    "min10float",     "min16float",     "min12int",    "min16int",
    "min16uint",      "literal float",  "literal int", "int16_t",
    "int32_t",        "int64_t",        "uint16_t",    "uint32_t",
    "uint64_t",       "float16_t",      "float32_t",   "float64_t",
    "int8_t4_packed", "uint8_t4_packed"};

static_assert(HLSLScalarTypeCount == _countof(HLSLScalarTypeNames),
              "otherwise scalar constants are not aligned");

static HLSLScalarType FindScalarTypeByName(const char *typeName,
                                           const size_t typeLen,
                                           const LangOptions &langOptions) {
  // skipped HLSLScalarType: unknown, literal int, literal float

  switch (typeLen) {
  case 3: // int
    if (typeName[0] == 'i') {
      if (strncmp(typeName, "int", 3))
        break;
      return HLSLScalarType_int;
    }
    break;
  case 4: // bool, uint, half
    if (typeName[0] == 'b') {
      if (strncmp(typeName, "bool", 4))
        break;
      return HLSLScalarType_bool;
    } else if (typeName[0] == 'u') {
      if (strncmp(typeName, "uint", 4))
        break;
      return HLSLScalarType_uint;
    } else if (typeName[0] == 'h') {
      if (strncmp(typeName, "half", 4))
        break;
      return HLSLScalarType_half;
    }
    break;
  case 5: // dword, float
    if (typeName[0] == 'd') {
      if (strncmp(typeName, "dword", 5))
        break;
      return HLSLScalarType_dword;
    } else if (typeName[0] == 'f') {
      if (strncmp(typeName, "float", 5))
        break;
      return HLSLScalarType_float;
    }
    break;
  case 6: // double
    if (typeName[0] == 'd') {
      if (strncmp(typeName, "double", 6))
        break;
      return HLSLScalarType_double;
    }
    break;
  case 7: // int64_t
    if (typeName[0] == 'i' && typeName[1] == 'n') {
      if (typeName[3] == '6') {
        if (strncmp(typeName, "int64_t", 7))
          break;
        return HLSLScalarType_int64;
      }
    }
    break;
  case 8: // min12int, min16int, uint64_t
    if (typeName[0] == 'm' && typeName[1] == 'i') {
      if (typeName[4] == '2') {
        if (strncmp(typeName, "min12int", 8))
          break;
        return HLSLScalarType_int_min12;
      } else if (typeName[4] == '6') {
        if (strncmp(typeName, "min16int", 8))
          break;
        return HLSLScalarType_int_min16;
      }
    } else if (typeName[0] == 'u' && typeName[1] == 'i') {
      if (typeName[4] == '6') {
        if (strncmp(typeName, "uint64_t", 8))
          break;
        return HLSLScalarType_uint64;
      }
    }
    break;
  case 9: // min16uint
    if (typeName[0] == 'm' && typeName[1] == 'i') {
      if (strncmp(typeName, "min16uint", 9))
        break;
      return HLSLScalarType_uint_min16;
    }
    break;
  case 10: // min10float, min16float
    if (typeName[0] == 'm' && typeName[1] == 'i') {
      if (typeName[4] == '0') {
        if (strncmp(typeName, "min10float", 10))
          break;
        return HLSLScalarType_float_min10;
      }
      if (typeName[4] == '6') {
        if (strncmp(typeName, "min16float", 10))
          break;
        return HLSLScalarType_float_min16;
      }
    }
    break;
  case 14: // int8_t4_packed
    if (typeName[0] == 'i' && typeName[1] == 'n') {
      if (strncmp(typeName, "int8_t4_packed", 14))
        break;
      return HLSLScalarType_int8_4packed;
    }
    break;
  case 15: // uint8_t4_packed
    if (typeName[0] == 'u' && typeName[1] == 'i') {
      if (strncmp(typeName, "uint8_t4_packed", 15))
        break;
      return HLSLScalarType_uint8_4packed;
    }
    break;
  default:
    break;
  }
  // fixed width types (int16_t, uint16_t, int32_t, uint32_t, float16_t,
  // float32_t, float64_t) are only supported in HLSL 2018
  if (langOptions.HLSLVersion >= hlsl::LangStd::v2018) {
    switch (typeLen) {
    case 7: // int16_t, int32_t
      if (typeName[0] == 'i' && typeName[1] == 'n') {
        if (!langOptions.UseMinPrecision) {
          if (typeName[3] == '1') {
            if (strncmp(typeName, "int16_t", 7))
              break;
            return HLSLScalarType_int16;
          }
        }
        if (typeName[3] == '3') {
          if (strncmp(typeName, "int32_t", 7))
            break;
          return HLSLScalarType_int32;
        }
      }
      break;
    case 8: // uint16_t, uint32_t
      if (!langOptions.UseMinPrecision) {
        if (typeName[0] == 'u' && typeName[1] == 'i') {
          if (typeName[4] == '1') {
            if (strncmp(typeName, "uint16_t", 8))
              break;
            return HLSLScalarType_uint16;
          }
        }
      }
      if (typeName[4] == '3') {
        if (strncmp(typeName, "uint32_t", 8))
          break;
        return HLSLScalarType_uint32;
      }
      break;
    case 9: // float16_t, float32_t, float64_t
      if (typeName[0] == 'f' && typeName[1] == 'l') {
        if (!langOptions.UseMinPrecision) {
          if (typeName[5] == '1') {
            if (strncmp(typeName, "float16_t", 9))
              break;
            return HLSLScalarType_float16;
          }
        }
        if (typeName[5] == '3') {
          if (strncmp(typeName, "float32_t", 9))
            break;
          return HLSLScalarType_float32;
        } else if (typeName[5] == '6') {
          if (strncmp(typeName, "float64_t", 9))
            break;
          return HLSLScalarType_float64;
        }
      }
    }
  }
  return HLSLScalarType_unknown;
}

/// <summary>Provides the primitive type for lowering matrix types to
/// IR.</summary>
static CanQualType GetHLSLObjectHandleType(ASTContext &context) {
  return context.IntTy;
}

static void
AddSubscriptOperator(ASTContext &context, unsigned int templateDepth,
                     TemplateTypeParmDecl *elementTemplateParamDecl,
                     NonTypeTemplateParmDecl *colCountTemplateParamDecl,
                     QualType intType, CXXRecordDecl *templateRecordDecl,
                     ClassTemplateDecl *vectorTemplateDecl, bool forConst) {
  QualType elementType = context.getTemplateTypeParmType(
      templateDepth, 0, ParameterPackFalse, elementTemplateParamDecl);
  Expr *sizeExpr = DeclRefExpr::Create(
      context, NestedNameSpecifierLoc(), NoLoc, colCountTemplateParamDecl,
      false,
      DeclarationNameInfo(colCountTemplateParamDecl->getDeclName(), NoLoc),
      intType, ExprValueKind::VK_RValue);

  CXXRecordDecl *vecTemplateRecordDecl = vectorTemplateDecl->getTemplatedDecl();
  const clang::Type *vecTy = vecTemplateRecordDecl->getTypeForDecl();

  TemplateArgument templateArgs[2] = {TemplateArgument(elementType),
                                      TemplateArgument(sizeExpr)};
  TemplateName canonName =
      context.getCanonicalTemplateName(TemplateName(vectorTemplateDecl));
  QualType vectorType = context.getTemplateSpecializationType(
      canonName, templateArgs, _countof(templateArgs), QualType(vecTy, 0));

  vectorType = context.getLValueReferenceType(vectorType);

  if (forConst)
    vectorType = context.getConstType(vectorType);

  QualType indexType = intType;
  auto methodDecl = CreateObjectFunctionDeclarationWithParams(
      context, templateRecordDecl, vectorType, ArrayRef<QualType>(indexType),
      ArrayRef<StringRef>(StringRef("index")),
      context.DeclarationNames.getCXXOperatorName(OO_Subscript), forConst);
  methodDecl->addAttr(HLSLCXXOverloadAttr::CreateImplicit(context));
}

/// <summary>Adds up-front support for HLSL matrix types (just the template
/// declaration).</summary>
void hlsl::AddHLSLMatrixTemplate(ASTContext &context,
                                 ClassTemplateDecl *vectorTemplateDecl,
                                 ClassTemplateDecl **matrixTemplateDecl) {
  DXASSERT_NOMSG(matrixTemplateDecl != nullptr);
  DXASSERT_NOMSG(vectorTemplateDecl != nullptr);

  // Create a matrix template declaration in translation unit scope.
  // template<typename element, int row_count, int col_count> matrix { ... }
  BuiltinTypeDeclBuilder typeDeclBuilder(context.getTranslationUnitDecl(),
                                         "matrix");
  TemplateTypeParmDecl *elementTemplateParamDecl =
      typeDeclBuilder.addTypeTemplateParam("element",
                                           (QualType)context.FloatTy);
  NonTypeTemplateParmDecl *rowCountTemplateParamDecl =
      typeDeclBuilder.addIntegerTemplateParam("row_count", context.IntTy, 4);
  NonTypeTemplateParmDecl *colCountTemplateParamDecl =
      typeDeclBuilder.addIntegerTemplateParam("col_count", context.IntTy, 4);
  typeDeclBuilder.startDefinition();
  CXXRecordDecl *templateRecordDecl = typeDeclBuilder.getRecordDecl();
  ClassTemplateDecl *classTemplateDecl = typeDeclBuilder.getTemplateDecl();

  // Add an 'h' field to hold the handle.
  // The type is vector<element, col>[row].
  QualType elementType = context.getTemplateTypeParmType(
      /*templateDepth*/ 0, 0, ParameterPackFalse, elementTemplateParamDecl);
  Expr *sizeExpr = DeclRefExpr::Create(
      context, NestedNameSpecifierLoc(), NoLoc, rowCountTemplateParamDecl,
      false,
      DeclarationNameInfo(rowCountTemplateParamDecl->getDeclName(), NoLoc),
      context.IntTy, ExprValueKind::VK_RValue);

  Expr *rowSizeExpr = DeclRefExpr::Create(
      context, NestedNameSpecifierLoc(), NoLoc, colCountTemplateParamDecl,
      false,
      DeclarationNameInfo(colCountTemplateParamDecl->getDeclName(), NoLoc),
      context.IntTy, ExprValueKind::VK_RValue);

  QualType vectorType = context.getDependentSizedExtVectorType(
      elementType, rowSizeExpr, SourceLocation());
  QualType vectorArrayType = context.getDependentSizedArrayType(
      vectorType, sizeExpr, ArrayType::Normal, 0, SourceRange());

  typeDeclBuilder.addField("h", vectorArrayType);

  typeDeclBuilder.getRecordDecl()->addAttr(
      HLSLMatrixAttr::CreateImplicit(context));

  // Add an operator[]. The operator ranges from zero to rowcount-1, and returns
  // a vector of colcount elements.
  const unsigned int templateDepth = 0;
  AddSubscriptOperator(context, templateDepth, elementTemplateParamDecl,
                       colCountTemplateParamDecl, context.UnsignedIntTy,
                       templateRecordDecl, vectorTemplateDecl, ForConstFalse);
  AddSubscriptOperator(context, templateDepth, elementTemplateParamDecl,
                       colCountTemplateParamDecl, context.UnsignedIntTy,
                       templateRecordDecl, vectorTemplateDecl, ForConstTrue);

  typeDeclBuilder.completeDefinition();
  *matrixTemplateDecl = classTemplateDecl;
}

static void AddHLSLVectorSubscriptAttr(Decl *D, ASTContext &context) {
  StringRef group = GetHLOpcodeGroupName(HLOpcodeGroup::HLSubscript);
  D->addAttr(HLSLIntrinsicAttr::CreateImplicit(
      context, group, "",
      static_cast<unsigned>(HLSubscriptOpcode::VectorSubscript)));
  D->addAttr(HLSLCXXOverloadAttr::CreateImplicit(context));
}

/// <summary>Adds up-front support for HLSL vector types (just the template
/// declaration).</summary>
void hlsl::AddHLSLVectorTemplate(ASTContext &context,
                                 ClassTemplateDecl **vectorTemplateDecl) {
  DXASSERT_NOMSG(vectorTemplateDecl != nullptr);

  // Create a vector template declaration in translation unit scope.
  // template<typename element, int element_count> vector { ... }
  BuiltinTypeDeclBuilder typeDeclBuilder(context.getTranslationUnitDecl(),
                                         "vector");
  TemplateTypeParmDecl *elementTemplateParamDecl =
      typeDeclBuilder.addTypeTemplateParam("element",
                                           (QualType)context.FloatTy);
  NonTypeTemplateParmDecl *colCountTemplateParamDecl =
      typeDeclBuilder.addIntegerTemplateParam("element_count", context.IntTy,
                                              4);
  typeDeclBuilder.startDefinition();
  CXXRecordDecl *templateRecordDecl = typeDeclBuilder.getRecordDecl();
  ClassTemplateDecl *classTemplateDecl = typeDeclBuilder.getTemplateDecl();

  Expr *vecSizeExpr = DeclRefExpr::Create(
      context, NestedNameSpecifierLoc(), NoLoc, colCountTemplateParamDecl,
      false,
      DeclarationNameInfo(colCountTemplateParamDecl->getDeclName(), NoLoc),
      context.IntTy, ExprValueKind::VK_RValue);

  const unsigned int templateDepth = 0;
  QualType resultType = context.getTemplateTypeParmType(
      templateDepth, 0, ParameterPackFalse, elementTemplateParamDecl);
  QualType vectorType = context.getDependentSizedExtVectorType(
      resultType, vecSizeExpr, SourceLocation());
  // Add an 'h' field to hold the handle.
  typeDeclBuilder.addField("h", vectorType);

  typeDeclBuilder.getRecordDecl()->addAttr(
      HLSLVectorAttr::CreateImplicit(context));

  // Add an operator[]. The operator ranges from zero to colcount-1, and returns
  // a scalar.

  // ForConstTrue:
  QualType refResultType =
      context.getConstType(context.getLValueReferenceType(resultType));
  CXXMethodDecl *functionDecl = CreateObjectFunctionDeclarationWithParams(
      context, templateRecordDecl, refResultType,
      ArrayRef<QualType>(context.UnsignedIntTy),
      ArrayRef<StringRef>(StringRef("index")),
      context.DeclarationNames.getCXXOperatorName(OO_Subscript), ForConstTrue);
  AddHLSLVectorSubscriptAttr(functionDecl, context);
  // ForConstFalse:
  resultType = context.getLValueReferenceType(resultType);
  functionDecl = CreateObjectFunctionDeclarationWithParams(
      context, templateRecordDecl, resultType,
      ArrayRef<QualType>(context.UnsignedIntTy),
      ArrayRef<StringRef>(StringRef("index")),
      context.DeclarationNames.getCXXOperatorName(OO_Subscript), ForConstFalse);
  AddHLSLVectorSubscriptAttr(functionDecl, context);

  typeDeclBuilder.completeDefinition();
  *vectorTemplateDecl = classTemplateDecl;
}

static void AddRecordAccessMethod(clang::ASTContext &Ctx,
                                  clang::CXXRecordDecl *RD,
                                  clang::QualType ReturnTy,
                                  bool IsGetOrSubscript, bool IsConst,
                                  bool IsArray) {
  DeclarationName DeclName =
      IsGetOrSubscript ? DeclarationName(&Ctx.Idents.get("Get"))
                       : Ctx.DeclarationNames.getCXXOperatorName(OO_Subscript);

  if (IsConst)
    ReturnTy.addConst();

  ReturnTy = Ctx.getLValueReferenceType(ReturnTy);

  QualType ArgTypes[] = {Ctx.UnsignedIntTy};
  ArrayRef<QualType> Types = IsArray ? ArgTypes : ArrayRef<QualType>();
  StringRef ArgNames[] = {"Index"};
  ArrayRef<StringRef> Names = IsArray ? ArgNames : ArrayRef<StringRef>();

  CXXMethodDecl *MethodDecl = CreateObjectFunctionDeclarationWithParams(
      Ctx, RD, ReturnTy, Types, Names, DeclName, IsConst);

  if (IsGetOrSubscript && IsArray) {
    ParmVarDecl *IndexParam = MethodDecl->getParamDecl(0);
    Expr *ConstantZero = IntegerLiteral::Create(
        Ctx, llvm::APInt(Ctx.getIntWidth(Ctx.UnsignedIntTy), 0),
        Ctx.UnsignedIntTy, NoLoc);
    IndexParam->setDefaultArg(ConstantZero);
  }

  StringRef OpcodeGroup = GetHLOpcodeGroupName(HLOpcodeGroup::HLSubscript);
  unsigned Opcode = static_cast<unsigned>(HLSubscriptOpcode::DefaultSubscript);
  MethodDecl->addAttr(
      HLSLIntrinsicAttr::CreateImplicit(Ctx, OpcodeGroup, "", Opcode));
  MethodDecl->addAttr(HLSLCXXOverloadAttr::CreateImplicit(Ctx));
}

static void AddRecordGetMethods(clang::ASTContext &Ctx,
                                clang::CXXRecordDecl *RD,
                                clang::QualType ReturnTy, bool IsConstOnly,
                                bool IsArray) {
  if (!IsConstOnly)
    AddRecordAccessMethod(Ctx, RD, ReturnTy, true, false, IsArray);
  AddRecordAccessMethod(Ctx, RD, ReturnTy, true, true, IsArray);
}

static void AddRecordSubscriptAccess(clang::ASTContext &Ctx,
                                     clang::CXXRecordDecl *RD,
                                     clang::QualType ReturnTy,
                                     bool IsConstOnly) {
  if (!IsConstOnly)
    AddRecordAccessMethod(Ctx, RD, ReturnTy, false, false, true);
  AddRecordAccessMethod(Ctx, RD, ReturnTy, false, true, true);
}

/// <summary>Adds up-front support for HLSL *NodeOutputRecords template
/// types.</summary>
void hlsl::AddHLSLNodeOutputRecordTemplate(
    ASTContext &context, DXIL::NodeIOKind Type,
    ClassTemplateDecl **outputRecordTemplateDecl,
    bool isCompleteType /*= true*/) {
  DXASSERT_NOMSG(outputRecordTemplateDecl != nullptr);

  StringRef templateName = HLSLNodeObjectAttr::ConvertRecordTypeToStr(Type);

  // Create a *NodeOutputRecords template declaration in translation unit scope.
  BuiltinTypeDeclBuilder typeDeclBuilder(context.getTranslationUnitDecl(),
                                         templateName,
                                         TagDecl::TagKind::TTK_Struct);
  TemplateTypeParmDecl *outputTemplateParamDecl =
      typeDeclBuilder.addTypeTemplateParam("recordType");
  typeDeclBuilder.startDefinition();
  ClassTemplateDecl *classTemplateDecl = typeDeclBuilder.getTemplateDecl();

  // Add an 'h' field to hold the handle.
  typeDeclBuilder.addField("h", GetHLSLObjectHandleType(context));

  typeDeclBuilder.getRecordDecl()->addAttr(
      HLSLNodeObjectAttr::CreateImplicit(context, Type));

  QualType elementType = context.getTemplateTypeParmType(
      0, 0, ParameterPackFalse, outputTemplateParamDecl);

  CXXRecordDecl *record = typeDeclBuilder.getRecordDecl();

  // Subscript operator is required for Node Array Types.
  AddRecordSubscriptAccess(context, record, elementType, false);
  AddRecordGetMethods(context, record, elementType, false, true);

  if (isCompleteType)
    typeDeclBuilder.completeDefinition();
  *outputRecordTemplateDecl = classTemplateDecl;
}

/// <summary>
/// Adds a new record type in the specified context with the given name. The
/// record type will have a handle field.
/// </summary>
CXXRecordDecl *
hlsl::DeclareRecordTypeWithHandleAndNoMemberFunctions(ASTContext &context,
                                                      StringRef name) {
  BuiltinTypeDeclBuilder typeDeclBuilder(context.getTranslationUnitDecl(), name,
                                         TagDecl::TagKind::TTK_Struct);
  typeDeclBuilder.startDefinition();
  typeDeclBuilder.addField("h", GetHLSLObjectHandleType(context));
  typeDeclBuilder.completeDefinition();
  return typeDeclBuilder.getRecordDecl();
}

/// <summary>
/// Adds a new record type in the specified context with the given name. The
/// record type will have a handle field.
/// </summary>
CXXRecordDecl *
hlsl::DeclareRecordTypeWithHandle(ASTContext &context, StringRef name,
                                  bool isCompleteType /*= true */,
                                  InheritableAttr *Attr) {
  BuiltinTypeDeclBuilder typeDeclBuilder(context.getTranslationUnitDecl(), name,
                                         TagDecl::TagKind::TTK_Struct);
  typeDeclBuilder.startDefinition();
  typeDeclBuilder.addField("h", GetHLSLObjectHandleType(context));
  if (Attr)
    typeDeclBuilder.getRecordDecl()->addAttr(Attr);

  if (isCompleteType)
    return typeDeclBuilder.completeDefinition();
  return typeDeclBuilder.getRecordDecl();
}

AvailabilityAttr *ConstructAvailabilityAttribute(clang::ASTContext &context,
                                                 VersionTuple Introduced) {
  AvailabilityAttr *AAttr = AvailabilityAttr::CreateImplicit(
      context, &context.Idents.get(""), clang::VersionTuple(6, 9),
      clang::VersionTuple(), clang::VersionTuple(), false, "");
  return AAttr;
}

// creates a global static constant unsigned integer with value.
// equivalent to: static const uint name = val;
static void AddConstUInt(clang::ASTContext &context, DeclContext *DC,
                         StringRef name, unsigned val,
                         AvailabilityAttr *AAttr = nullptr) {
  IdentifierInfo &Id = context.Idents.get(name, tok::TokenKind::identifier);
  QualType type = context.getConstType(context.UnsignedIntTy);
  VarDecl *varDecl = VarDecl::Create(context, DC, NoLoc, NoLoc, &Id, type,
                                     context.getTrivialTypeSourceInfo(type),
                                     clang::StorageClass::SC_Static);
  Expr *exprVal = IntegerLiteral::Create(
      context, llvm::APInt(context.getIntWidth(type), val), type, NoLoc);
  varDecl->setInit(exprVal);
  varDecl->setImplicit(true);
  if (AAttr)
    varDecl->addAttr(AAttr);

  DC->addDecl(varDecl);
}

static void AddConstUInt(clang::ASTContext &context, StringRef name,
                         unsigned val) {
  AddConstUInt(context, context.getTranslationUnitDecl(), name, val);
}

// Adds a top-level enum with the given enumerants.
struct Enumerant {
  StringRef name;
  unsigned value;
  AvailabilityAttr *avail = nullptr;
};

static void AddTypedefPseudoEnum(ASTContext &context, StringRef name,
                                 ArrayRef<Enumerant> enumerants) {
  DeclContext *curDC = context.getTranslationUnitDecl();
  // typedef uint <name>;
  IdentifierInfo &enumId = context.Idents.get(name, tok::TokenKind::identifier);
  TypeSourceInfo *uintTypeSource =
      context.getTrivialTypeSourceInfo(context.UnsignedIntTy, NoLoc);
  TypedefDecl *enumDecl = TypedefDecl::Create(context, curDC, NoLoc, NoLoc,
                                              &enumId, uintTypeSource);
  curDC->addDecl(enumDecl);
  enumDecl->setImplicit(true);
  // static const uint <enumerant.name> = <enumerant.value>;
  for (const Enumerant &enumerant : enumerants) {
    AddConstUInt(context, curDC, enumerant.name, enumerant.value,
                 enumerant.avail);
  }
}

/// <summary> Adds all constants and enums for ray tracing </summary>
void hlsl::AddRaytracingConstants(ASTContext &context) {

  // Create aversion tuple for availability attributes
  // for the RAYQUERY_FLAG enum
  VersionTuple VT69 = VersionTuple(6, 9);

  AddTypedefPseudoEnum(
      context, "RAY_FLAG",
      {{"RAY_FLAG_NONE", (unsigned)DXIL::RayFlag::None},
       {"RAY_FLAG_FORCE_OPAQUE", (unsigned)DXIL::RayFlag::ForceOpaque},
       {"RAY_FLAG_FORCE_NON_OPAQUE", (unsigned)DXIL::RayFlag::ForceNonOpaque},
       {"RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH",
        (unsigned)DXIL::RayFlag::AcceptFirstHitAndEndSearch},
       {"RAY_FLAG_SKIP_CLOSEST_HIT_SHADER",
        (unsigned)DXIL::RayFlag::SkipClosestHitShader},
       {"RAY_FLAG_CULL_BACK_FACING_TRIANGLES",
        (unsigned)DXIL::RayFlag::CullBackFacingTriangles},
       {"RAY_FLAG_CULL_FRONT_FACING_TRIANGLES",
        (unsigned)DXIL::RayFlag::CullFrontFacingTriangles},
       {"RAY_FLAG_CULL_OPAQUE", (unsigned)DXIL::RayFlag::CullOpaque},
       {"RAY_FLAG_CULL_NON_OPAQUE", (unsigned)DXIL::RayFlag::CullNonOpaque},
       {"RAY_FLAG_SKIP_TRIANGLES", (unsigned)DXIL::RayFlag::SkipTriangles},
       {"RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES",
        (unsigned)DXIL::RayFlag::SkipProceduralPrimitives},
       {"RAY_FLAG_FORCE_OMM_2_STATE", (unsigned)DXIL::RayFlag::ForceOMM2State,
        ConstructAvailabilityAttribute(context, VT69)}});

  AddTypedefPseudoEnum(
      context, "RAYQUERY_FLAG",
      {{"RAYQUERY_FLAG_NONE", (unsigned)DXIL::RayQueryFlag::None},
       {"RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS",
        (unsigned)DXIL::RayQueryFlag::AllowOpacityMicromaps,
        ConstructAvailabilityAttribute(context, VT69)}});

  AddTypedefPseudoEnum(
      context, "COMMITTED_STATUS",
      {{"COMMITTED_NOTHING", (unsigned)DXIL::CommittedStatus::CommittedNothing},
       {"COMMITTED_TRIANGLE_HIT",
        (unsigned)DXIL::CommittedStatus::CommittedTriangleHit},
       {"COMMITTED_PROCEDURAL_PRIMITIVE_HIT",
        (unsigned)DXIL::CommittedStatus::CommittedProceduralPrimitiveHit}});

  AddTypedefPseudoEnum(
      context, "CANDIDATE_TYPE",
      {{"CANDIDATE_NON_OPAQUE_TRIANGLE",
        (unsigned)DXIL::CandidateType::CandidateNonOpaqueTriangle},
       {"CANDIDATE_PROCEDURAL_PRIMITIVE",
        (unsigned)DXIL::CandidateType::CandidateProceduralPrimitive}});

  // static const uint HIT_KIND_* = *;
  AddConstUInt(context, StringRef("HIT_KIND_NONE"),
               (unsigned)DXIL::HitKind::None);
  AddConstUInt(context, StringRef("HIT_KIND_TRIANGLE_FRONT_FACE"),
               (unsigned)DXIL::HitKind::TriangleFrontFace);
  AddConstUInt(context, StringRef("HIT_KIND_TRIANGLE_BACK_FACE"),
               (unsigned)DXIL::HitKind::TriangleBackFace);

  AddConstUInt(
      context,
      StringRef(
          "STATE_OBJECT_FLAGS_ALLOW_LOCAL_DEPENDENCIES_ON_EXTERNAL_DEFINITONS"),
      (unsigned)
          DXIL::StateObjectFlags::AllowLocalDependenciesOnExternalDefinitions);
  AddConstUInt(
      context,
      StringRef("STATE_OBJECT_FLAGS_ALLOW_EXTERNAL_DEPENDENCIES_ON_LOCAL_"
                "DEFINITIONS"),
      (unsigned)
          DXIL::StateObjectFlags::AllowExternalDependenciesOnLocalDefinitions);
  // The above "_FLAGS_" was a typo, leaving in to avoid breaking anyone.
  // Supposed to be _FLAG_ below.
  AddConstUInt(
      context,
      StringRef(
          "STATE_OBJECT_FLAG_ALLOW_LOCAL_DEPENDENCIES_ON_EXTERNAL_DEFINITONS"),
      (unsigned)
          DXIL::StateObjectFlags::AllowLocalDependenciesOnExternalDefinitions);
  AddConstUInt(
      context,
      StringRef(
          "STATE_OBJECT_FLAG_ALLOW_EXTERNAL_DEPENDENCIES_ON_LOCAL_DEFINITIONS"),
      (unsigned)
          DXIL::StateObjectFlags::AllowExternalDependenciesOnLocalDefinitions);
  AddConstUInt(context,
               StringRef("STATE_OBJECT_FLAG_ALLOW_STATE_OBJECT_ADDITIONS"),
               (unsigned)DXIL::StateObjectFlags::AllowStateObjectAdditions);

  AddConstUInt(context, StringRef("RAYTRACING_PIPELINE_FLAG_NONE"),
               (unsigned)DXIL::RaytracingPipelineFlags::None);
  AddConstUInt(context, StringRef("RAYTRACING_PIPELINE_FLAG_SKIP_TRIANGLES"),
               (unsigned)DXIL::RaytracingPipelineFlags::SkipTriangles);
  AddConstUInt(
      context, StringRef("RAYTRACING_PIPELINE_FLAG_SKIP_PROCEDURAL_PRIMITIVES"),
      (unsigned)DXIL::RaytracingPipelineFlags::SkipProceduralPrimitives);
  AddConstUInt(context, context.getTranslationUnitDecl(),
               StringRef("RAYTRACING_PIPELINE_FLAG_ALLOW_OPACITY_MICROMAPS"),
               (unsigned)DXIL::RaytracingPipelineFlags::AllowOpacityMicromaps,
               ConstructAvailabilityAttribute(context, VT69));
}

/// <summary> Adds all constants and enums for sampler feedback </summary>
void hlsl::AddSamplerFeedbackConstants(ASTContext &context) {
  AddConstUInt(context, StringRef("SAMPLER_FEEDBACK_MIN_MIP"),
               (unsigned)DXIL::SamplerFeedbackType::MinMip);
  AddConstUInt(context, StringRef("SAMPLER_FEEDBACK_MIP_REGION_USED"),
               (unsigned)DXIL::SamplerFeedbackType::MipRegionUsed);
}

/// <summary> Adds all enums for Barrier intrinsic</summary>
void hlsl::AddBarrierConstants(ASTContext &context) {
  VersionTuple VT69 = VersionTuple(6, 9);

  AddTypedefPseudoEnum(
      context, "MEMORY_TYPE_FLAG",
      {{"UAV_MEMORY", (unsigned)DXIL::MemoryTypeFlag::UavMemory},
       {"GROUP_SHARED_MEMORY",
        (unsigned)DXIL::MemoryTypeFlag::GroupSharedMemory},
       {"NODE_INPUT_MEMORY", (unsigned)DXIL::MemoryTypeFlag::NodeInputMemory},
       {"NODE_OUTPUT_MEMORY", (unsigned)DXIL::MemoryTypeFlag::NodeOutputMemory},
       {"ALL_MEMORY", (unsigned)DXIL::MemoryTypeFlag::AllMemory}});
  AddTypedefPseudoEnum(
      context, "BARRIER_SEMANTIC_FLAG",
      {{"GROUP_SYNC", (unsigned)DXIL::BarrierSemanticFlag::GroupSync},
       {"GROUP_SCOPE", (unsigned)DXIL::BarrierSemanticFlag::GroupScope},
       {"DEVICE_SCOPE", (unsigned)DXIL::BarrierSemanticFlag::DeviceScope},
       {"REORDER_SCOPE", (unsigned)DXIL::BarrierSemanticFlag::ReorderScope,
        ConstructAvailabilityAttribute(context, VT69)}});
}

static Expr *IntConstantAsBoolExpr(clang::Sema &sema, uint64_t value) {
  return sema
      .ImpCastExprToType(sema.ActOnIntegerConstant(NoLoc, value).get(),
                         sema.getASTContext().BoolTy, CK_IntegralToBoolean)
      .get();
}

static CXXRecordDecl *CreateStdStructWithStaticBool(clang::ASTContext &context,
                                                    NamespaceDecl *stdNamespace,
                                                    IdentifierInfo &trueTypeId,
                                                    IdentifierInfo &valueId,
                                                    Expr *trueExpression) {
  // struct true_type { static const bool value = true; }
  TypeSourceInfo *boolTypeSource =
      context.getTrivialTypeSourceInfo(context.BoolTy.withConst());
  CXXRecordDecl *trueTypeDecl = CXXRecordDecl::Create(
      context, TagTypeKind::TTK_Struct, stdNamespace, NoLoc, NoLoc, &trueTypeId,
      nullptr, DelayTypeCreationTrue);

  // static fields are variables in the AST
  VarDecl *trueValueDecl =
      VarDecl::Create(context, trueTypeDecl, NoLoc, NoLoc, &valueId,
                      context.BoolTy.withConst(), boolTypeSource, SC_Static);

  trueValueDecl->setInit(trueExpression);
  trueValueDecl->setConstexpr(true);
  trueValueDecl->setAccess(AS_public);
  trueTypeDecl->setLexicalDeclContext(stdNamespace);
  trueTypeDecl->startDefinition();
  trueTypeDecl->addDecl(trueValueDecl);
  trueTypeDecl->completeDefinition();
  stdNamespace->addDecl(trueTypeDecl);

  return trueTypeDecl;
}

static void DefineRecordWithBase(CXXRecordDecl *decl,
                                 DeclContext *lexicalContext,
                                 const CXXBaseSpecifier *base) {
  decl->setLexicalDeclContext(lexicalContext);
  decl->startDefinition();
  decl->setBases(&base, 1);
  decl->completeDefinition();
  lexicalContext->addDecl(decl);
}

static void SetPartialExplicitSpecialization(
    ClassTemplateDecl *templateDecl,
    ClassTemplatePartialSpecializationDecl *specializationDecl) {
  specializationDecl->setSpecializationKind(TSK_ExplicitSpecialization);
  templateDecl->AddPartialSpecialization(specializationDecl, nullptr);
}

static void CreateIsEqualSpecialization(
    ASTContext &context, ClassTemplateDecl *templateDecl,
    TemplateName &templateName, DeclContext *lexicalContext,
    const CXXBaseSpecifier *base, TemplateParameterList *templateParamList,
    TemplateArgument (&templateArgs)[2]) {
  QualType specializationCanonType = context.getTemplateSpecializationType(
      templateName, templateArgs, _countof(templateArgs));

  TemplateArgumentListInfo templateArgsListInfo =
      TemplateArgumentListInfo(NoLoc, NoLoc);
  templateArgsListInfo.addArgument(TemplateArgumentLoc(
      templateArgs[0],
      context.getTrivialTypeSourceInfo(templateArgs[0].getAsType())));
  templateArgsListInfo.addArgument(TemplateArgumentLoc(
      templateArgs[1],
      context.getTrivialTypeSourceInfo(templateArgs[1].getAsType())));

  ClassTemplatePartialSpecializationDecl *specializationDecl =
      ClassTemplatePartialSpecializationDecl::Create(
          context, TTK_Struct, lexicalContext, NoLoc, NoLoc, templateParamList,
          templateDecl, templateArgs, _countof(templateArgs),
          templateArgsListInfo, specializationCanonType, nullptr);
  context.getTagDeclType(specializationDecl); // Fault this in now.
  DefineRecordWithBase(specializationDecl, lexicalContext, base);
  SetPartialExplicitSpecialization(templateDecl, specializationDecl);
}

/// <summary>Adds the implementation for std::is_equal.</summary>
void hlsl::AddStdIsEqualImplementation(clang::ASTContext &context,
                                       clang::Sema &sema) {
  // The goal is to support std::is_same<T, T>::value for testing purposes, in a
  // manner that can evolve into a compliant feature in the future.
  //
  // The definitions necessary are as follows (all in the std namespace).
  //  template <class T, T v>
  //  struct integral_constant {
  //    typedef T value_type;
  //    static const value_type value = v;
  //    operator value_type() { return value; }
  //  };
  //
  //  typedef integral_constant<bool, true> true_type;
  //  typedef integral_constant<bool, false> false_type;
  //
  //  template<typename T, typename U> struct is_same : public false_type {};
  //  template<typename T>             struct is_same<T, T> : public
  //  true_type{};
  //
  // We instead use these simpler definitions for true_type and false_type.
  //  struct false_type { static const bool value = false; };
  //  struct true_type { static const bool value = true; };
  DeclContext *tuContext = context.getTranslationUnitDecl();
  IdentifierInfo &stdId =
      context.Idents.get(StringRef("std"), tok::TokenKind::identifier);
  IdentifierInfo &trueTypeId =
      context.Idents.get(StringRef("true_type"), tok::TokenKind::identifier);
  IdentifierInfo &falseTypeId =
      context.Idents.get(StringRef("false_type"), tok::TokenKind::identifier);
  IdentifierInfo &valueId =
      context.Idents.get(StringRef("value"), tok::TokenKind::identifier);
  IdentifierInfo &isSameId =
      context.Idents.get(StringRef("is_same"), tok::TokenKind::identifier);
  IdentifierInfo &tId =
      context.Idents.get(StringRef("T"), tok::TokenKind::identifier);
  IdentifierInfo &vId =
      context.Idents.get(StringRef("V"), tok::TokenKind::identifier);

  Expr *trueExpression = IntConstantAsBoolExpr(sema, 1);
  Expr *falseExpression = IntConstantAsBoolExpr(sema, 0);

  // namespace std
  NamespaceDecl *stdNamespace = NamespaceDecl::Create(
      context, tuContext, InlineFalse, NoLoc, NoLoc, &stdId, nullptr);

  CXXRecordDecl *trueTypeDecl = CreateStdStructWithStaticBool(
      context, stdNamespace, trueTypeId, valueId, trueExpression);
  CXXRecordDecl *falseTypeDecl = CreateStdStructWithStaticBool(
      context, stdNamespace, falseTypeId, valueId, falseExpression);

  //  template<typename T, typename U> struct is_same : public false_type {};
  CXXRecordDecl *isSameFalseRecordDecl =
      CXXRecordDecl::Create(context, TagTypeKind::TTK_Struct, stdNamespace,
                            NoLoc, NoLoc, &isSameId, nullptr, false);
  TemplateTypeParmDecl *tParam = TemplateTypeParmDecl::Create(
      context, stdNamespace, NoLoc, NoLoc, FirstTemplateDepth,
      FirstParamPosition, &tId, TypenameFalse, ParameterPackFalse);
  TemplateTypeParmDecl *uParam = TemplateTypeParmDecl::Create(
      context, stdNamespace, NoLoc, NoLoc, FirstTemplateDepth,
      FirstParamPosition + 1, &vId, TypenameFalse, ParameterPackFalse);
  NamedDecl *falseParams[] = {tParam, uParam};
  TemplateParameterList *falseParamList = TemplateParameterList::Create(
      context, NoLoc, NoLoc, falseParams, _countof(falseParams), NoLoc);
  ClassTemplateDecl *isSameFalseTemplateDecl = ClassTemplateDecl::Create(
      context, stdNamespace, NoLoc, DeclarationName(&isSameId), falseParamList,
      isSameFalseRecordDecl, nullptr);
  context.getTagDeclType(isSameFalseRecordDecl); // Fault this in now.
  CXXBaseSpecifier *falseBase = new (context) CXXBaseSpecifier(
      SourceRange(), VirtualFalse, BaseClassFalse, AS_public,
      context.getTrivialTypeSourceInfo(context.getTypeDeclType(falseTypeDecl)),
      NoLoc);
  isSameFalseRecordDecl->setDescribedClassTemplate(isSameFalseTemplateDecl);
  isSameFalseTemplateDecl->setLexicalDeclContext(stdNamespace);
  DefineRecordWithBase(isSameFalseRecordDecl, stdNamespace, falseBase);

  // is_same for 'true' is a specialization of is_same for 'false', taking a
  // single T, where both T will match
  //  template<typename T> struct is_same<T, T> : public true_type{};
  TemplateName tn = TemplateName(isSameFalseTemplateDecl);
  NamedDecl *trueParams[] = {tParam};
  TemplateParameterList *trueParamList = TemplateParameterList::Create(
      context, NoLoc, NoLoc, trueParams, _countof(trueParams), NoLoc);
  CXXBaseSpecifier *trueBase = new (context) CXXBaseSpecifier(
      SourceRange(), VirtualFalse, BaseClassFalse, AS_public,
      context.getTrivialTypeSourceInfo(context.getTypeDeclType(trueTypeDecl)),
      NoLoc);

  TemplateArgument ta = TemplateArgument(
      context.getCanonicalType(context.getTypeDeclType(tParam)));
  TemplateArgument isSameTrueTemplateArgs[] = {ta, ta};
  CreateIsEqualSpecialization(context, isSameFalseTemplateDecl, tn,
                              stdNamespace, trueBase, trueParamList,
                              isSameTrueTemplateArgs);

  stdNamespace->addDecl(isSameFalseTemplateDecl);
  stdNamespace->setImplicit(true);
  tuContext->addDecl(stdNamespace);

  // This could be a parameter if ever needed.
  const bool SupportExtensions = true;

  // Consider right-hand const and right-hand ref to be true for is_same:
  // template<typename T> struct is_same<T, const T> : public true_type{};
  // template<typename T> struct is_same<T, T&>      : public true_type{};
  if (SupportExtensions) {
    TemplateArgument trueConstArg = TemplateArgument(
        context.getCanonicalType(context.getTypeDeclType(tParam)).withConst());
    TemplateArgument isSameTrueConstTemplateArgs[] = {ta, trueConstArg};
    CreateIsEqualSpecialization(context, isSameFalseTemplateDecl, tn,
                                stdNamespace, trueBase, trueParamList,
                                isSameTrueConstTemplateArgs);

    TemplateArgument trueRefArg =
        TemplateArgument(context.getLValueReferenceType(
            context.getCanonicalType(context.getTypeDeclType(tParam))));
    TemplateArgument isSameTrueRefTemplateArgs[] = {ta, trueRefArg};
    CreateIsEqualSpecialization(context, isSameFalseTemplateDecl, tn,
                                stdNamespace, trueBase, trueParamList,
                                isSameTrueRefTemplateArgs);
  }
}

/// <summary>
/// Adds a new template type in the specified context with the given name. The
/// record type will have a handle field.
/// </summary>
/// <parm name="context">AST context to which template will be added.</param>
/// <parm name="typeName">Name of template to create.</param>
/// <parm name="templateArgCount">Number of template arguments (one or
/// two).</param> <parm name="defaultTypeArgValue">If assigned, the default
/// argument for the element template.</param>
CXXRecordDecl *hlsl::DeclareTemplateTypeWithHandle(
    ASTContext &context, StringRef name, uint8_t templateArgCount,
    TypeSourceInfo *defaultTypeArgValue, InheritableAttr *Attr) {
  return DeclareTemplateTypeWithHandleInDeclContext(
      context, context.getTranslationUnitDecl(), name, templateArgCount,
      defaultTypeArgValue, Attr);
}

CXXRecordDecl *hlsl::DeclareTemplateTypeWithHandleInDeclContext(
    ASTContext &context, DeclContext *declContext, StringRef name,
    uint8_t templateArgCount, TypeSourceInfo *defaultTypeArgValue,
    InheritableAttr *Attr) {

  DXASSERT(templateArgCount != 0,
           "otherwise caller should be creating a class or struct");
  DXASSERT(templateArgCount <= 2, "otherwise the function needs to be updated "
                                  "for a different template pattern");

  // Create an object template declaration in translation unit scope.
  // templateArgCount=1: template<typename element> typeName { ... }
  // templateArgCount=2: template<typename element, int count> typeName { ... }
  BuiltinTypeDeclBuilder typeDeclBuilder(declContext, name);
  TemplateTypeParmDecl *elementTemplateParamDecl =
      typeDeclBuilder.addTypeTemplateParam("element", defaultTypeArgValue);
  NonTypeTemplateParmDecl *countTemplateParamDecl = nullptr;
  if (templateArgCount > 1)
    countTemplateParamDecl =
        typeDeclBuilder.addIntegerTemplateParam("count", context.IntTy, 0);

  typeDeclBuilder.startDefinition();
  CXXRecordDecl *templateRecordDecl = typeDeclBuilder.getRecordDecl();

  // Add an 'h' field to hold the handle.
  QualType elementType = context.getTemplateTypeParmType(
      /*templateDepth*/ 0, 0, ParameterPackFalse, elementTemplateParamDecl);

  // Only need array type for inputpatch and outputpatch.
  if (Attr && isa<HLSLTessPatchAttr>(Attr)) {
    DXASSERT(templateArgCount == 2, "Tess patches need 2 template params");
    Expr *countExpr = DeclRefExpr::Create(
        context, NestedNameSpecifierLoc(), NoLoc, countTemplateParamDecl, false,
        DeclarationNameInfo(countTemplateParamDecl->getDeclName(), NoLoc),
        context.IntTy, ExprValueKind::VK_RValue);

    elementType = context.getDependentSizedArrayType(
        elementType, countExpr, ArrayType::ArraySizeModifier::Normal, 0,
        SourceRange());

    // InputPatch and OutputPatch also have a "Length" static const member for
    // the number of control points
    IdentifierInfo &lengthId =
        context.Idents.get(StringRef("Length"), tok::TokenKind::identifier);
    TypeSourceInfo *lengthTypeSource =
        context.getTrivialTypeSourceInfo(context.IntTy.withConst());
    VarDecl *lengthValueDecl =
        VarDecl::Create(context, templateRecordDecl, NoLoc, NoLoc, &lengthId,
                        context.IntTy.withConst(), lengthTypeSource, SC_Static);
    lengthValueDecl->setInit(countExpr);
    lengthValueDecl->setAccess(AS_public);
    templateRecordDecl->addDecl(lengthValueDecl);
  }

  typeDeclBuilder.addField("h", elementType);

  if (Attr)
    typeDeclBuilder.getRecordDecl()->addAttr(Attr);

  return typeDeclBuilder.getRecordDecl();
}

FunctionTemplateDecl *hlsl::CreateFunctionTemplateDecl(
    ASTContext &context, CXXRecordDecl *recordDecl, CXXMethodDecl *functionDecl,
    NamedDecl **templateParamNamedDecls, size_t templateParamNamedDeclsCount) {
  DXASSERT_NOMSG(recordDecl != nullptr);
  DXASSERT_NOMSG(templateParamNamedDecls != nullptr);
  DXASSERT(templateParamNamedDeclsCount > 0,
           "otherwise caller shouldn't invoke this function");

  TemplateParameterList *templateParams = TemplateParameterList::Create(
      context, NoLoc, NoLoc, &templateParamNamedDecls[0],
      templateParamNamedDeclsCount, NoLoc);
  FunctionTemplateDecl *functionTemplate = FunctionTemplateDecl::Create(
      context, recordDecl, NoLoc, functionDecl->getDeclName(), templateParams,
      functionDecl);
  functionTemplate->setAccess(AccessSpecifier::AS_public);
  functionTemplate->setLexicalDeclContext(recordDecl);
  functionDecl->setDescribedFunctionTemplate(functionTemplate);
  recordDecl->addDecl(functionTemplate);

  return functionTemplate;
}

static void AssociateParametersToFunctionPrototype(TypeSourceInfo *tinfo,
                                                   ParmVarDecl **paramVarDecls,
                                                   unsigned int numParams) {
  FunctionProtoTypeLoc protoLoc =
      tinfo->getTypeLoc().getAs<FunctionProtoTypeLoc>();
  DXASSERT(protoLoc.getNumParams() == numParams,
           "otherwise unexpected number of parameters available");
  for (unsigned i = 0; i < numParams; i++) {
    DXASSERT(protoLoc.getParam(i) == nullptr,
             "otherwise prototype parameters were already initialized");
    protoLoc.setParam(i, paramVarDecls[i]);
  }
}

static void CreateConstructorDeclaration(
    ASTContext &context, CXXRecordDecl *recordDecl, QualType resultType,
    ArrayRef<QualType> args, DeclarationName declarationName, bool isConst,
    CXXConstructorDecl **constructorDecl, TypeSourceInfo **tinfo) {
  DXASSERT_NOMSG(recordDecl != nullptr);
  DXASSERT_NOMSG(constructorDecl != nullptr);

  FunctionProtoType::ExtProtoInfo functionExtInfo;
  functionExtInfo.TypeQuals = isConst ? Qualifiers::Const : 0;
  QualType functionQT = context.getFunctionType(
      resultType, args, functionExtInfo, ArrayRef<ParameterModifier>());
  DeclarationNameInfo declNameInfo(declarationName, NoLoc);
  *tinfo = context.getTrivialTypeSourceInfo(functionQT, NoLoc);
  DXASSERT_NOMSG(*tinfo != nullptr);
  *constructorDecl = CXXConstructorDecl::Create(
      context, recordDecl, NoLoc, declNameInfo, functionQT, *tinfo,
      StorageClass::SC_None, ExplicitFalse, InlineSpecifiedFalse,
      IsConstexprFalse);
  DXASSERT_NOMSG(*constructorDecl != nullptr);
  (*constructorDecl)->setLexicalDeclContext(recordDecl);
  (*constructorDecl)->setAccess(AccessSpecifier::AS_public);
}

CXXConstructorDecl *hlsl::CreateConstructorDeclarationWithParams(
    ASTContext &context, CXXRecordDecl *recordDecl, QualType resultType,
    ArrayRef<QualType> paramTypes, ArrayRef<StringRef> paramNames,
    DeclarationName declarationName, bool isConst, bool isTemplateFunction) {
  DXASSERT_NOMSG(recordDecl != nullptr);
  DXASSERT_NOMSG(!resultType.isNull());
  DXASSERT_NOMSG(paramTypes.size() == paramNames.size());

  TypeSourceInfo *tinfo;
  CXXConstructorDecl *constructorDecl;
  CreateConstructorDeclaration(context, recordDecl, resultType, paramTypes,
                               declarationName, isConst, &constructorDecl,
                               &tinfo);

  // Create and associate parameters to constructor.
  SmallVector<ParmVarDecl *, 2> parmVarDecls;
  if (!paramTypes.empty()) {
    for (unsigned int i = 0; i < paramTypes.size(); ++i) {
      IdentifierInfo *argIi = &context.Idents.get(paramNames[i]);
      ParmVarDecl *parmVarDecl = ParmVarDecl::Create(
          context, constructorDecl, NoLoc, NoLoc, argIi, paramTypes[i],
          context.getTrivialTypeSourceInfo(paramTypes[i], NoLoc),
          StorageClass::SC_None, nullptr);
      parmVarDecl->setScopeInfo(0, i);
      DXASSERT(parmVarDecl->getFunctionScopeIndex() == i,
               "otherwise failed to set correct index");
      parmVarDecls.push_back(parmVarDecl);
    }
    constructorDecl->setParams(ArrayRef<ParmVarDecl *>(parmVarDecls));
    AssociateParametersToFunctionPrototype(tinfo, &parmVarDecls.front(),
                                           parmVarDecls.size());
  }

  // If this is going to be part of a template function decl, don't add it to
  // the record because the template function decl will be added instead.
  if (!isTemplateFunction)
    recordDecl->addDecl(constructorDecl);

  return constructorDecl;
}

static void CreateObjectFunctionDeclaration(
    ASTContext &context, CXXRecordDecl *recordDecl, QualType resultType,
    ArrayRef<QualType> args, DeclarationName declarationName, bool isConst,
    StorageClass SC, CXXMethodDecl **functionDecl, TypeSourceInfo **tinfo) {
  DXASSERT_NOMSG(recordDecl != nullptr);
  DXASSERT_NOMSG(functionDecl != nullptr);

  FunctionProtoType::ExtProtoInfo functionExtInfo;
  functionExtInfo.TypeQuals = isConst ? Qualifiers::Const : 0;
  QualType functionQT = context.getFunctionType(
      resultType, args, functionExtInfo, ArrayRef<ParameterModifier>());
  DeclarationNameInfo declNameInfo(declarationName, NoLoc);
  *tinfo = context.getTrivialTypeSourceInfo(functionQT, NoLoc);
  DXASSERT_NOMSG(*tinfo != nullptr);
  *functionDecl = CXXMethodDecl::Create(
      context, recordDecl, NoLoc, declNameInfo, functionQT, *tinfo, SC,
      InlineSpecifiedFalse, IsConstexprFalse, NoLoc);
  DXASSERT_NOMSG(*functionDecl != nullptr);
  (*functionDecl)->setLexicalDeclContext(recordDecl);
  (*functionDecl)->setAccess(AccessSpecifier::AS_public);
}

CXXMethodDecl *hlsl::CreateObjectFunctionDeclarationWithParams(
    ASTContext &context, CXXRecordDecl *recordDecl, QualType resultType,
    ArrayRef<QualType> paramTypes, ArrayRef<StringRef> paramNames,
    DeclarationName declarationName, bool isConst, StorageClass SC,
    bool isTemplateFunction) {
  DXASSERT_NOMSG(recordDecl != nullptr);
  DXASSERT_NOMSG(!resultType.isNull());
  DXASSERT_NOMSG(paramTypes.size() == paramNames.size());

  TypeSourceInfo *tinfo;
  CXXMethodDecl *functionDecl;
  CreateObjectFunctionDeclaration(context, recordDecl, resultType, paramTypes,
                                  declarationName, isConst, SC, &functionDecl,
                                  &tinfo);

  // Create and associate parameters to method.
  SmallVector<ParmVarDecl *, 2> parmVarDecls;
  if (!paramTypes.empty()) {
    for (unsigned int i = 0; i < paramTypes.size(); ++i) {
      IdentifierInfo *argIi = &context.Idents.get(paramNames[i]);
      ParmVarDecl *parmVarDecl = ParmVarDecl::Create(
          context, functionDecl, NoLoc, NoLoc, argIi, paramTypes[i],
          context.getTrivialTypeSourceInfo(paramTypes[i], NoLoc),
          StorageClass::SC_None, nullptr);
      parmVarDecl->setScopeInfo(0, i);
      DXASSERT(parmVarDecl->getFunctionScopeIndex() == i,
               "otherwise failed to set correct index");
      parmVarDecls.push_back(parmVarDecl);
    }
    functionDecl->setParams(ArrayRef<ParmVarDecl *>(parmVarDecls));
    AssociateParametersToFunctionPrototype(tinfo, &parmVarDecls.front(),
                                           parmVarDecls.size());
  }

  // If this is going to be part of a template function decl, don't add it to
  // the record because the template function decl will be added instead.
  if (!isTemplateFunction)
    recordDecl->addDecl(functionDecl);

  return functionDecl;
}

CXXRecordDecl *hlsl::DeclareUIntTemplatedTypeWithHandle(
    ASTContext &context, StringRef typeName, StringRef templateParamName,
    InheritableAttr *Attr) {
  return DeclareUIntTemplatedTypeWithHandleInDeclContext(
      context, context.getTranslationUnitDecl(), typeName, templateParamName,
      Attr);
}

CXXRecordDecl *hlsl::DeclareUIntTemplatedTypeWithHandleInDeclContext(
    ASTContext &context, DeclContext *declContext, StringRef typeName,
    StringRef templateParamName, InheritableAttr *Attr) {
  // template<uint kind> FeedbackTexture2D[Array] { ... }
  BuiltinTypeDeclBuilder typeDeclBuilder(declContext, typeName,
                                         TagTypeKind::TTK_Class);
  typeDeclBuilder.addIntegerTemplateParam(templateParamName,
                                          context.UnsignedIntTy);
  typeDeclBuilder.startDefinition();
  typeDeclBuilder.addField(
      "h", context.UnsignedIntTy); // Add an 'h' field to hold the handle.
  if (Attr)
    typeDeclBuilder.getRecordDecl()->addAttr(Attr);

  return typeDeclBuilder.getRecordDecl();
}

clang::CXXRecordDecl *
hlsl::DeclareConstantBufferViewType(clang::ASTContext &context,
                                    InheritableAttr *Attr) {
  // Create ConstantBufferView template declaration in translation unit scope
  // like other resource.
  // template<typename T> ConstantBuffer { int h; }
  DeclContext *DC = context.getTranslationUnitDecl();
  DXASSERT(Attr, "Constbuffer types require an attribute");

  const char *TypeName = "ConstantBuffer";
  if (IsTBuffer(cast<HLSLResourceAttr>(Attr)->getResKind()))
    TypeName = "TextureBuffer";
  BuiltinTypeDeclBuilder typeDeclBuilder(DC, TypeName,
                                         TagDecl::TagKind::TTK_Struct);
  (void)typeDeclBuilder.addTypeTemplateParam("T");
  typeDeclBuilder.startDefinition();
  CXXRecordDecl *templateRecordDecl = typeDeclBuilder.getRecordDecl();

  typeDeclBuilder.addField(
      "h", context.UnsignedIntTy); // Add an 'h' field to hold the handle.
  typeDeclBuilder.getRecordDecl()->addAttr(Attr);

  typeDeclBuilder.getRecordDecl();

  return templateRecordDecl;
}

CXXRecordDecl *hlsl::DeclareRayQueryType(ASTContext &context) {
  // template<uint kind> RayQuery { ... }
  BuiltinTypeDeclBuilder typeDeclBuilder(context.getTranslationUnitDecl(),
                                         "RayQuery");
  typeDeclBuilder.addIntegerTemplateParam("constRayFlags",
                                          context.UnsignedIntTy);
  // create an optional second template argument with default value
  // that contains the value of DXIL::RayFlag::None
  llvm::Optional<int64_t> DefaultRayQueryFlag =
      static_cast<int64_t>(DXIL::RayFlag::None);
  typeDeclBuilder.addIntegerTemplateParam(
      "RayQueryFlags", context.UnsignedIntTy, DefaultRayQueryFlag);
  typeDeclBuilder.startDefinition();
  typeDeclBuilder.addField(
      "h", context.UnsignedIntTy); // Add an 'h' field to hold the handle.

  // Add constructor that will be lowered to the intrinsic that produces
  // the RayQuery handle for this object.
  CanQualType canQualType = typeDeclBuilder.getRecordDecl()
                                ->getTypeForDecl()
                                ->getCanonicalTypeUnqualified();
  CXXConstructorDecl *pConstructorDecl = nullptr;
  TypeSourceInfo *pTypeSourceInfo = nullptr;
  CreateConstructorDeclaration(
      context, typeDeclBuilder.getRecordDecl(), context.VoidTy, {},
      context.DeclarationNames.getCXXConstructorName(canQualType), false,
      &pConstructorDecl, &pTypeSourceInfo);
  typeDeclBuilder.getRecordDecl()->addDecl(pConstructorDecl);
  typeDeclBuilder.getRecordDecl()->addAttr(
      HLSLRayQueryObjectAttr::CreateImplicit(context));
  return typeDeclBuilder.getRecordDecl();
}

CXXRecordDecl *hlsl::DeclareHitObjectType(NamespaceDecl &NSDecl) {
  ASTContext &Context = NSDecl.getASTContext();
  // HitObject { ... }
  BuiltinTypeDeclBuilder TypeDeclBuilder(&NSDecl, "HitObject");
  TypeDeclBuilder.startDefinition();

  // Add handle to mark as HLSL object.
  TypeDeclBuilder.addField("h", GetHLSLObjectHandleType(Context));
  CXXRecordDecl *RecordDecl = TypeDeclBuilder.getRecordDecl();

  CanQualType canQualType = Context.getCanonicalType(
      Context.getRecordType(TypeDeclBuilder.getRecordDecl()));

  // Add constructor that will be lowered to MOP_HitObject_MakeNop.
  CXXConstructorDecl *pConstructorDecl = nullptr;
  TypeSourceInfo *pTypeSourceInfo = nullptr;
  CreateConstructorDeclaration(
      Context, RecordDecl, Context.VoidTy, {},
      Context.DeclarationNames.getCXXConstructorName(canQualType), false,
      &pConstructorDecl, &pTypeSourceInfo);
  RecordDecl->addDecl(pConstructorDecl);
  pConstructorDecl->addAttr(HLSLIntrinsicAttr::CreateImplicit(
      Context, "op", "",
      static_cast<int>(hlsl::IntrinsicOp::MOP_DxHitObject_MakeNop)));
  pConstructorDecl->addAttr(HLSLCXXOverloadAttr::CreateImplicit(Context));

  // Add AvailabilityAttribute for SM6.9+
  VersionTuple VT69 = VersionTuple(6, 9);
  RecordDecl->addAttr(ConstructAvailabilityAttribute(Context, VT69));

  // Add the implicit HLSLHitObjectAttr attribute to unambiguously recognize the
  // builtin HitObject type.
  RecordDecl->addAttr(HLSLHitObjectAttr::CreateImplicit(Context));
  RecordDecl->setImplicit(true);

  // Add to namespace
  RecordDecl->setDeclContext(&NSDecl);
  return RecordDecl;
}

CXXRecordDecl *hlsl::DeclareResourceType(ASTContext &context, bool bSampler) {
  // struct ResourceDescriptor { uint8 desc; }
  StringRef Name = bSampler ? ".Sampler" : ".Resource";
  BuiltinTypeDeclBuilder typeDeclBuilder(context.getTranslationUnitDecl(), Name,
                                         TagDecl::TagKind::TTK_Struct);
  typeDeclBuilder.startDefinition();

  typeDeclBuilder.addField("h", GetHLSLObjectHandleType(context));

  CXXRecordDecl *recordDecl = typeDeclBuilder.getRecordDecl();

  QualType indexType = context.UnsignedIntTy;
  QualType resultType = context.getRecordType(recordDecl);
  resultType = context.getConstType(resultType);

  CXXMethodDecl *functionDecl = CreateObjectFunctionDeclarationWithParams(
      context, recordDecl, resultType, ArrayRef<QualType>(indexType),
      ArrayRef<StringRef>(StringRef("index")),
      context.DeclarationNames.getCXXOperatorName(OO_Subscript), true);
  // Mark function as createResourceFromHeap intrinsic.
  functionDecl->addAttr(HLSLIntrinsicAttr::CreateImplicit(
      context, "op", "",
      static_cast<int>(hlsl::IntrinsicOp::IOP_CreateResourceFromHeap)));
  functionDecl->addAttr(HLSLCXXOverloadAttr::CreateImplicit(context));
  return recordDecl;
}

CXXRecordDecl *hlsl::DeclareNodeOrRecordType(
    clang::ASTContext &Ctx, DXIL::NodeIOKind Type, bool IsRecordTypeTemplate,
    bool IsConst, bool HasGetMethods, bool IsArray, bool IsCompleteType) {
  StringRef TypeName = HLSLNodeObjectAttr::ConvertRecordTypeToStr(Type);

  BuiltinTypeDeclBuilder Builder(Ctx.getTranslationUnitDecl(), TypeName,
                                 TagDecl::TagKind::TTK_Struct);
  TemplateTypeParmDecl *TyParamDecl = nullptr;

  if (IsRecordTypeTemplate)
    TyParamDecl = Builder.addTypeTemplateParam("recordtype");

  Builder.startDefinition();
  Builder.addField("h", GetHLSLObjectHandleType(Ctx));

  Builder.getRecordDecl()->addAttr(
      HLSLNodeObjectAttr::CreateImplicit(Ctx, Type));

  if (IsRecordTypeTemplate) {
    QualType ParamTy = QualType(TyParamDecl->getTypeForDecl(), 0);
    CXXRecordDecl *Record = Builder.getRecordDecl();

    if (HasGetMethods || IsArray)
      AddRecordGetMethods(Ctx, Record, ParamTy, IsConst, IsArray);

    if (IsArray)
      AddRecordSubscriptAccess(Ctx, Record, ParamTy, IsConst);
  }

  if (IsCompleteType)
    return Builder.completeDefinition();

  return Builder.getRecordDecl();
}

#ifdef ENABLE_SPIRV_CODEGEN
CXXRecordDecl *hlsl::DeclareVkBufferPointerType(ASTContext &context,
                                                DeclContext *declContext) {
  BuiltinTypeDeclBuilder Builder(declContext, "BufferPointer",
                                 TagDecl::TagKind::TTK_Struct);
  TemplateTypeParmDecl *TyParamDecl =
      Builder.addTypeTemplateParam("recordtype");
  Builder.addIntegerTemplateParam("alignment", context.UnsignedIntTy, 0);

  Builder.startDefinition();

  QualType paramType = QualType(TyParamDecl->getTypeForDecl(), 0);
  CXXRecordDecl *recordDecl = Builder.getRecordDecl();

  CXXMethodDecl *methodDecl = CreateObjectFunctionDeclarationWithParams(
      context, recordDecl, context.getLValueReferenceType(paramType), {}, {},
      DeclarationName(&context.Idents.get("Get")), true);
  CanQualType canQualType =
      recordDecl->getTypeForDecl()->getCanonicalTypeUnqualified();
  auto *copyConstructorDecl = CreateConstructorDeclarationWithParams(
      context, recordDecl, context.VoidTy,
      {context.getRValueReferenceType(canQualType)}, {"bufferPointer"},
      context.DeclarationNames.getCXXConstructorName(canQualType), false, true);
  auto *addressConstructorDecl = CreateConstructorDeclarationWithParams(
      context, recordDecl, context.VoidTy, {context.UnsignedIntTy}, {"address"},
      context.DeclarationNames.getCXXConstructorName(canQualType), false, true);
  hlsl::CreateFunctionTemplateDecl(
      context, recordDecl, copyConstructorDecl,
      Builder.getTemplateDecl()->getTemplateParameters()->begin(), 2);
  hlsl::CreateFunctionTemplateDecl(
      context, recordDecl, addressConstructorDecl,
      Builder.getTemplateDecl()->getTemplateParameters()->begin(), 2);

  StringRef OpcodeGroup = GetHLOpcodeGroupName(HLOpcodeGroup::HLIntrinsic);
  unsigned Opcode = static_cast<unsigned>(IntrinsicOp::MOP_GetBufferContents);
  methodDecl->addAttr(
      HLSLIntrinsicAttr::CreateImplicit(context, OpcodeGroup, "", Opcode));
  methodDecl->addAttr(HLSLCXXOverloadAttr::CreateImplicit(context));
  copyConstructorDecl->addAttr(HLSLCXXOverloadAttr::CreateImplicit(context));
  addressConstructorDecl->addAttr(HLSLCXXOverloadAttr::CreateImplicit(context));

  return Builder.completeDefinition();
}

CXXRecordDecl *hlsl::DeclareInlineSpirvType(clang::ASTContext &context,
                                            clang::DeclContext *declContext,
                                            llvm::StringRef typeName,
                                            bool opaque) {
  // template<uint opcode, int size, int alignment> vk::SpirvType { ... }
  // template<uint opcode> vk::SpirvOpaqueType { ... }
  BuiltinTypeDeclBuilder typeDeclBuilder(declContext, typeName,
                                         clang::TagTypeKind::TTK_Class);
  typeDeclBuilder.addIntegerTemplateParam("opcode", context.UnsignedIntTy);
  if (!opaque) {
    typeDeclBuilder.addIntegerTemplateParam("size", context.UnsignedIntTy);
    typeDeclBuilder.addIntegerTemplateParam("alignment", context.UnsignedIntTy);
  }
  typeDeclBuilder.addTypeTemplateParam("operands", nullptr, true);
  typeDeclBuilder.startDefinition();
  typeDeclBuilder.addField(
      "h", context.UnsignedIntTy); // Add an 'h' field to hold the handle.
  return typeDeclBuilder.getRecordDecl();
}

CXXRecordDecl *hlsl::DeclareVkIntegralConstant(
    clang::ASTContext &context, clang::DeclContext *declContext,
    llvm::StringRef typeName, ClassTemplateDecl **templateDecl) {
  // template<typename T, T v> vk::integral_constant { ... }
  BuiltinTypeDeclBuilder typeDeclBuilder(declContext, typeName,
                                         clang::TagTypeKind::TTK_Class);
  typeDeclBuilder.addTypeTemplateParam("T");
  typeDeclBuilder.addIntegerTemplateParam("v", context.UnsignedIntTy);
  typeDeclBuilder.startDefinition();
  typeDeclBuilder.addField(
      "h", context.UnsignedIntTy); // Add an 'h' field to hold the handle.
  *templateDecl = typeDeclBuilder.getTemplateDecl();
  return typeDeclBuilder.getRecordDecl();
}
#endif

CXXRecordDecl *hlsl::DeclareNodeOutputArray(clang::ASTContext &Ctx,
                                            DXIL::NodeIOKind Type,
                                            CXXRecordDecl *OutputType,
                                            bool IsRecordTypeTemplate) {
  StringRef TypeName = HLSLNodeObjectAttr::ConvertRecordTypeToStr(Type);
  BuiltinTypeDeclBuilder Builder(Ctx.getTranslationUnitDecl(), TypeName,
                                 TagDecl::TagKind::TTK_Struct);
  TemplateTypeParmDecl *elementTemplateParamDecl = nullptr;

  if (IsRecordTypeTemplate)
    elementTemplateParamDecl = Builder.addTypeTemplateParam("recordtype");

  Builder.startDefinition();
  Builder.addField("h", GetHLSLObjectHandleType(Ctx));

  Builder.getRecordDecl()->addAttr(
      HLSLNodeObjectAttr::CreateImplicit(Ctx, Type));

  QualType ResultType;
  if (IsRecordTypeTemplate) {
    QualType elementType = Ctx.getTemplateTypeParmType(
        /*templateDepth*/ 0, /*index*/ 0, ParameterPackFalse,
        elementTemplateParamDecl);

    const clang::Type *nodeOutputTy = OutputType->getTypeForDecl();

    TemplateArgument templateArgs[1] = {TemplateArgument(elementType)};

    TemplateName canonName = Ctx.getCanonicalTemplateName(
        TemplateName(OutputType->getDescribedClassTemplate()));
    ResultType = Ctx.getTemplateSpecializationType(canonName, templateArgs,
                                                   _countof(templateArgs),
                                                   QualType(nodeOutputTy, 0));
  } else {
    // For Non Template types(like EmptyNodeOutput)
    ResultType = Ctx.getTypeDeclType(OutputType);
  }

  QualType indexType = Ctx.UnsignedIntTy;

  auto methodDecl = CreateObjectFunctionDeclarationWithParams(
      Ctx, Builder.getRecordDecl(), ResultType, ArrayRef<QualType>(indexType),
      ArrayRef<StringRef>(StringRef("index")),
      Ctx.DeclarationNames.getCXXOperatorName(OO_Subscript), false);

  StringRef OpcodeGroup =
      GetHLOpcodeGroupName(HLOpcodeGroup::HLIndexNodeHandle);
  unsigned Opcode = static_cast<unsigned>(HLOpcodeGroup::HLIndexNodeHandle);
  methodDecl->addAttr(
      HLSLIntrinsicAttr::CreateImplicit(Ctx, OpcodeGroup, "", Opcode));
  methodDecl->addAttr(HLSLCXXOverloadAttr::CreateImplicit(Ctx));

  return Builder.completeDefinition();
}

VarDecl *hlsl::DeclareBuiltinGlobal(llvm::StringRef name, clang::QualType Ty,
                                    clang::ASTContext &context) {
  IdentifierInfo &II = context.Idents.get(name);

  auto *curDeclCtx = context.getTranslationUnitDecl();

  VarDecl *varDecl = VarDecl::Create(
      context, curDeclCtx, SourceLocation(), SourceLocation(), &II, Ty,
      context.getTrivialTypeSourceInfo(Ty), StorageClass::SC_Extern);
  // Mark implicit to avoid print it when rewrite.
  varDecl->setImplicit();
  curDeclCtx->addDecl(varDecl);
  return varDecl;
}

bool hlsl::IsIntrinsicOp(const clang::FunctionDecl *FD) {
  return FD != nullptr && FD->hasAttr<HLSLIntrinsicAttr>();
}

bool hlsl::GetIntrinsicOp(const clang::FunctionDecl *FD, unsigned &opcode,
                          llvm::StringRef &group) {
  if (FD == nullptr || !FD->hasAttr<HLSLIntrinsicAttr>()) {
    return false;
  }

  HLSLIntrinsicAttr *A = FD->getAttr<HLSLIntrinsicAttr>();
  opcode = A->getOpcode();
  group = A->getGroup();
  return true;
}

bool hlsl::GetIntrinsicLowering(const clang::FunctionDecl *FD,
                                llvm::StringRef &S) {
  if (FD == nullptr || !FD->hasAttr<HLSLIntrinsicAttr>()) {
    return false;
  }

  HLSLIntrinsicAttr *A = FD->getAttr<HLSLIntrinsicAttr>();
  S = A->getLowering();
  return true;
}

/// <summary>Parses a column or row digit.</summary>
static bool TryParseColOrRowChar(const char digit, int *count) {
  if ('1' <= digit && digit <= '4') {
    *count = digit - '0';
    return true;
  }

  *count = 0;
  return false;
}

/// <summary>Parses a matrix shorthand identifier (eg, float3x2).</summary>
bool hlsl::TryParseMatrixShorthand(const char *typeName, size_t typeNameLen,
                                   HLSLScalarType *parsedType, int *rowCount,
                                   int *colCount,
                                   const clang::LangOptions &langOptions) {
  //
  // Matrix shorthand format is PrimitiveTypeRxC, where R is the row count and C
  // is the column count. R and C should be between 1 and 4 inclusive. x is a
  // literal 'x' character. PrimitiveType is one of the HLSLScalarTypeNames
  // values.
  //
  if (TryParseMatrixOrVectorDimension(typeName, typeNameLen, rowCount, colCount,
                                      langOptions) &&
      *rowCount != 0 && *colCount != 0) {
    // compare scalar component
    HLSLScalarType type =
        FindScalarTypeByName(typeName, typeNameLen - 3, langOptions);
    if (type != HLSLScalarType_unknown) {
      *parsedType = type;
      return true;
    }
  }
  // Unable to parse.
  return false;
}

/// <summary>Parses a vector shorthand identifier (eg, float3).</summary>
bool hlsl::TryParseVectorShorthand(const char *typeName, size_t typeNameLen,
                                   HLSLScalarType *parsedType,
                                   int *elementCount,
                                   const clang::LangOptions &langOptions) {
  // At least *something*N characters necessary, where something is at least
  // 'int'
  if (TryParseColOrRowChar(typeName[typeNameLen - 1], elementCount)) {
    // compare scalar component
    HLSLScalarType type =
        FindScalarTypeByName(typeName, typeNameLen - 1, langOptions);
    if (type != HLSLScalarType_unknown) {
      *parsedType = type;
      return true;
    }
  }
  // Unable to parse.
  return false;
}

/// <summary>Parses a hlsl scalar type (e.g min16float, uint3x4) </summary>
bool hlsl::TryParseScalar(const char *typeName, size_t typeNameLen,
                          HLSLScalarType *parsedType,
                          const clang::LangOptions &langOptions) {
  HLSLScalarType type =
      FindScalarTypeByName(typeName, typeNameLen, langOptions);
  if (type != HLSLScalarType_unknown) {
    *parsedType = type;
    return true;
  }
  return false; // unable to parse
}

/// <summary>Parse any (scalar, vector, matrix) hlsl types (e.g float, int3x4,
/// uint2) </summary>
bool hlsl::TryParseAny(const char *typeName, size_t typeNameLen,
                       HLSLScalarType *parsedType, int *rowCount, int *colCount,
                       const clang::LangOptions &langOptions) {
  // at least 'int'
  const size_t MinValidLen = 3;
  if (typeNameLen >= MinValidLen) {
    TryParseMatrixOrVectorDimension(typeName, typeNameLen, rowCount, colCount,
                                    langOptions);
    int suffixLen = *colCount == 0 ? 0 : *rowCount == 0 ? 1 : 3;
    HLSLScalarType type =
        FindScalarTypeByName(typeName, typeNameLen - suffixLen, langOptions);
    if (type != HLSLScalarType_unknown) {
      *parsedType = type;
      return true;
    }
  }
  return false;
}

/// <summary>Parse string hlsl type</summary>
bool hlsl::TryParseString(const char *typeName, size_t typeNameLen,
                          const clang::LangOptions &langOptions) {

  if (typeNameLen == 6 && typeName[0] == 's' &&
      strncmp(typeName, "string", 6) == 0) {
    return true;
  }
  return false;
}

/// <summary>Parse any kind of dimension for vector or matrix (e.g 4,3 in
/// int4x3). If it's a matrix type, rowCount and colCount will be nonzero. If
/// it's a vector type, colCount is 0. Otherwise both rowCount and colCount is
/// 0. Returns true if either matrix or vector dimensions detected. </summary>
bool hlsl::TryParseMatrixOrVectorDimension(
    const char *typeName, size_t typeNameLen, int *rowCount, int *colCount,
    const clang::LangOptions &langOptions) {
  *rowCount = 0;
  *colCount = 0;
  size_t MinValidLen = 3; // at least int
  if (typeNameLen > MinValidLen) {
    if (TryParseColOrRowChar(typeName[typeNameLen - 1], colCount)) {
      // Try parse matrix
      if (typeName[typeNameLen - 2] == 'x')
        TryParseColOrRowChar(typeName[typeNameLen - 3], rowCount);
      return true;
    }
  }
  return false;
}

/// <summary>Creates a typedef for a matrix shorthand (eg, float3x2).</summary>
TypedefDecl *hlsl::CreateMatrixSpecializationShorthand(
    ASTContext &context, QualType matrixSpecialization,
    HLSLScalarType scalarType, size_t rowCount, size_t colCount) {
  DXASSERT(rowCount <= 4, "else caller didn't validate rowCount");
  DXASSERT(colCount <= 4, "else caller didn't validate colCount");
  char typeName[64];
  sprintf_s(typeName, _countof(typeName), "%s%ux%u",
            HLSLScalarTypeNames[scalarType], (unsigned)rowCount,
            (unsigned)colCount);
  IdentifierInfo &typedefId =
      context.Idents.get(StringRef(typeName), tok::TokenKind::identifier);
  DeclContext *currentDeclContext = context.getTranslationUnitDecl();
  TypedefDecl *decl = TypedefDecl::Create(
      context, currentDeclContext, NoLoc, NoLoc, &typedefId,
      context.getTrivialTypeSourceInfo(matrixSpecialization, NoLoc));
  decl->setImplicit(true);
  currentDeclContext->addDecl(decl);
  return decl;
}

/// <summary>Creates a typedef for a vector shorthand (eg, float3).</summary>
TypedefDecl *hlsl::CreateVectorSpecializationShorthand(
    ASTContext &context, QualType vectorSpecialization,
    HLSLScalarType scalarType, size_t colCount) {
  DXASSERT(colCount <= 4, "else caller didn't validate colCount");
  char typeName[64];
  sprintf_s(typeName, _countof(typeName), "%s%u",
            HLSLScalarTypeNames[scalarType], (unsigned)colCount);
  IdentifierInfo &typedefId =
      context.Idents.get(StringRef(typeName), tok::TokenKind::identifier);
  DeclContext *currentDeclContext = context.getTranslationUnitDecl();
  TypedefDecl *decl = TypedefDecl::Create(
      context, currentDeclContext, NoLoc, NoLoc, &typedefId,
      context.getTrivialTypeSourceInfo(vectorSpecialization, NoLoc));
  decl->setImplicit(true);
  currentDeclContext->addDecl(decl);
  return decl;
}

llvm::ArrayRef<hlsl::UnusualAnnotation *>
hlsl::UnusualAnnotation::CopyToASTContextArray(clang::ASTContext &Context,
                                               hlsl::UnusualAnnotation **begin,
                                               size_t count) {
  if (count == 0) {
    return llvm::ArrayRef<hlsl::UnusualAnnotation *>();
  }

  UnusualAnnotation **arr = ::new (Context) UnusualAnnotation *[count];
  for (size_t i = 0; i < count; ++i) {
    arr[i] = begin[i]->CopyToASTContext(Context);
  }

  return llvm::ArrayRef<hlsl::UnusualAnnotation *>(arr, count);
}

UnusualAnnotation *
hlsl::UnusualAnnotation::CopyToASTContext(ASTContext &Context) {
  // All UnusualAnnotation instances can be blitted.
  size_t instanceSize;
  switch (Kind) {
  case UA_RegisterAssignment:
    instanceSize = sizeof(hlsl::RegisterAssignment);
    break;
  case UA_ConstantPacking:
    instanceSize = sizeof(hlsl::ConstantPacking);
    break;
  case UA_PayloadAccessQualifier:
    instanceSize = sizeof(hlsl::PayloadAccessAnnotation);
    break;
  default:
    DXASSERT(Kind == UA_SemanticDecl,
             "Kind == UA_SemanticDecl -- otherwise switch is incomplete");
    instanceSize = sizeof(hlsl::SemanticDecl);
    break;
  }

  void *result = Context.Allocate(instanceSize);
  memcpy(result, this, instanceSize);
  return (UnusualAnnotation *)result;
}

bool ASTContext::IsPatchConstantFunctionDecl(const FunctionDecl *FD) const {
  return hlsl::IsPatchConstantFunctionDecl(FD);
}
