//===--- SemaHLSL.h - Semantic Analysis & AST Building for HLSL --*- C++
//-*-===//
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// SemaHLSL.h                                                                //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
//  This file defines the semantic support for HLSL.                         //
//
///////////////////////////////////////////////////////////////////////////////

#ifndef LLVM_CLANG_SEMA_SEMAHLSL_H
#define LLVM_CLANG_SEMA_SEMAHLSL_H

#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/Sema/Initialization.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/Overload.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/SemaDiagnostic.h"
#include "clang/Sema/Template.h"
#include "clang/Sema/TemplateDeduction.h"

// Forward declarations.
struct IDxcIntrinsicTable;
namespace clang {
class Expr;
class ExternalSemaSource;
class ImplicitConversionSequence;
} // namespace clang

namespace hlsl {

void CheckBinOpForHLSL(clang::Sema &self, clang::SourceLocation OpLoc,
                       clang::BinaryOperatorKind Opc, clang::ExprResult &LHS,
                       clang::ExprResult &RHS, clang::QualType &ResultTy,
                       clang::QualType &CompLHSTy,
                       clang::QualType &CompResultTy);

bool CheckTemplateArgumentListForHLSL(clang::Sema &self, clang::TemplateDecl *,
                                      clang::SourceLocation,
                                      clang::TemplateArgumentListInfo &);

clang::QualType
CheckUnaryOpForHLSL(clang::Sema &self, clang::SourceLocation OpLoc,
                    clang::UnaryOperatorKind Opc, clang::ExprResult &InputExpr,
                    clang::ExprValueKind &VK, clang::ExprObjectKind &OK);

clang::Sema::TemplateDeductionResult DeduceTemplateArgumentsForHLSL(
    clang::Sema *, clang::FunctionTemplateDecl *,
    clang::TemplateArgumentListInfo *, llvm::ArrayRef<clang::Expr *>,
    clang::FunctionDecl *&, clang::sema::TemplateDeductionInfo &);

bool DiagnoseNodeStructArgument(clang::Sema *self,
                                clang::TemplateArgumentLoc ArgLoc,
                                clang::QualType ArgTy, bool &Empty,
                                const clang::FieldDecl *FD = nullptr);

// Keep this in sync with err_hlsl_unsupported_object in DiagnosticSemaKinds.td
enum class TypeDiagContext {
  // Indices that the type context is valid and no diagnostics should be emitted
  // for this type category.
  Valid = -1,
  // Supported indices for both `err_hlsl_unsupported_object_context` and
  // `err_hlsl_unsupported_long_vector`
  ConstantBuffersOrTextureBuffers = 0,
  TessellationPatches = 1,
  GeometryStreams = 2,
  NodeRecords = 3,
  CBuffersOrTBuffers = 4,
  UserDefinedStructParameter = 5,
  EntryFunctionParameters = 6,
  EntryFunctionReturnType = 7,
  PatchConstantFunctionParameters = 8,
  PatchConstantFunctionReturnType = 9,
  PayloadParameters = 10,
  Attributes = 11,
  TypeParameter = 12,
  LongVecDiagMaxSelectIndex = TypeParameter,
  // Below only supported for `err_hlsl_diag_unsupported_object_context`
  StructuredBuffers = 13,
  GlobalVariables = 14,
  GroupShared = 15,
  DiagMaxSelectIndex = 15,
};
bool DiagnoseTypeElements(clang::Sema &S, clang::SourceLocation Loc,
                          clang::QualType Ty, TypeDiagContext ObjDiagContext,
                          TypeDiagContext LongVecDiagContext,
                          const clang::FieldDecl *FD = nullptr);

void DiagnoseControlFlowConditionForHLSL(clang::Sema *self,
                                         clang::Expr *condExpr,
                                         llvm::StringRef StmtName);

void DiagnosePackingOffset(clang::Sema *self, clang::SourceLocation loc,
                           clang::QualType type, int componentOffset);

void DiagnoseRegisterType(clang::Sema *self, clang::SourceLocation loc,
                          clang::QualType type, char registerType);

void DiagnoseTranslationUnit(clang::Sema *self);

void DiagnoseUnusualAnnotationsForHLSL(
    clang::Sema &S, std::vector<hlsl::UnusualAnnotation *> &annotations);

void DiagnosePayloadAccessQualifierAnnotations(
    clang::Sema &S, clang::Declarator &D, const clang::QualType &T,
    const std::vector<hlsl::UnusualAnnotation *> &annotations);

void DiagnoseRaytracingPayloadAccess(clang::Sema &S,
                                     clang::TranslationUnitDecl *TU);

void DiagnoseCallableEntry(clang::Sema &S, clang::FunctionDecl *FD,
                           llvm::StringRef StageName);

void DiagnoseMissOrAnyHitEntry(clang::Sema &S, clang::FunctionDecl *FD,
                               llvm::StringRef StageName,
                               DXIL::ShaderKind Stage);

void DiagnoseRayGenerationOrIntersectionEntry(clang::Sema &S,
                                              clang::FunctionDecl *FD,
                                              llvm::StringRef StageName);

void DiagnoseClosestHitEntry(clang::Sema &S, clang::FunctionDecl *FD,
                             llvm::StringRef StageName);

void DiagnoseEntry(clang::Sema &S, clang::FunctionDecl *FD);

/// <summary>Finds the best viable function on this overload set, if it
/// exists.</summary>
clang::OverloadingResult
GetBestViableFunction(clang::Sema &S, clang::SourceLocation Loc,
                      clang::OverloadCandidateSet &set,
                      clang::OverloadCandidateSet::iterator &Best);

bool ShouldSkipNRVO(clang::Sema &sema, clang::QualType returnType,
                    clang::VarDecl *VD, clang::FunctionDecl *FD);

/// <summary>Processes an attribute for a declaration.</summary>
/// <param name="S">Sema with context.</param>
/// <param name="D">Annotated declaration.</param>
/// <param name="A">Single parsed attribute to process.</param>
/// <param name="Handled">After execution, whether this was recognized and
/// handled.</param>
void HandleDeclAttributeForHLSL(clang::Sema &S, clang::Decl *D,
                                const clang::AttributeList &Attr,
                                bool &Handled);

void InitializeInitSequenceForHLSL(clang::Sema *sema,
                                   const clang::InitializedEntity &Entity,
                                   const clang::InitializationKind &Kind,
                                   clang::MultiExprArg Args,
                                   bool TopLevelOfInitList,
                                   clang::InitializationSequence *initSequence);

unsigned CaculateInitListArraySizeForHLSL(clang::Sema *sema,
                                          const clang::InitListExpr *InitList,
                                          const clang::QualType EltTy);

bool ContainsLongVector(clang::QualType);

bool IsConversionToLessOrEqualElements(clang::Sema *self,
                                       const clang::ExprResult &sourceExpr,
                                       const clang::QualType &targetType,
                                       bool explicitConversion);

clang::ExprResult LookupMatrixMemberExprForHLSL(
    clang::Sema *self, clang::Expr &BaseExpr, clang::DeclarationName MemberName,
    bool IsArrow, clang::SourceLocation OpLoc, clang::SourceLocation MemberLoc);

clang::ExprResult LookupVectorMemberExprForHLSL(
    clang::Sema *self, clang::Expr &BaseExpr, clang::DeclarationName MemberName,
    bool IsArrow, clang::SourceLocation OpLoc, clang::SourceLocation MemberLoc);

clang::ExprResult LookupArrayMemberExprForHLSL(
    clang::Sema *self, clang::Expr &BaseExpr, clang::DeclarationName MemberName,
    bool IsArrow, clang::SourceLocation OpLoc, clang::SourceLocation MemberLoc);

bool LookupRecordMemberExprForHLSL(clang::Sema *self, clang::Expr &BaseExpr,
                                   clang::DeclarationName MemberName,
                                   bool IsArrow, clang::SourceLocation OpLoc,
                                   clang::SourceLocation MemberLoc,
                                   clang::ExprResult &result);

clang::ExprResult MaybeConvertMemberAccess(clang::Sema *Self, clang::Expr *E);

/// <summary>Performs the HLSL-specific type conversion steps.</summary>
/// <param name="self">Sema with context.</param>
/// <param name="E">Expression to convert.</param>
/// <param name="targetType">Type to convert to.</param>
/// <param name="SCS">Standard conversion sequence from which Second and
/// ComponentConversion will be used.</param> <param name="CCK">Conversion
/// kind.</param> <returns>Expression result of conversion.</returns>
clang::ExprResult
PerformHLSLConversion(clang::Sema *self, clang::Expr *E,
                      clang::QualType targetType,
                      const clang::StandardConversionSequence &SCS,
                      clang::Sema::CheckedConversionKind CCK);

/// <summary>Processes an attribute for a statement.</summary>
/// <param name="S">Sema with context.</param>
/// <param name="St">Annotated statement.</param>
/// <param name="A">Single parsed attribute to process.</param>
/// <param name="Range">Range of all attribute lists (useful for FixIts to
/// suggest inclusions).</param> <param name="Handled">After execution, whether
/// this was recognized and handled.</param> <returns>An attribute instance if
/// processed, nullptr if not recognized or an error was found.</returns>
clang::Attr *ProcessStmtAttributeForHLSL(clang::Sema &S, clang::Stmt *St,
                                         const clang::AttributeList &A,
                                         clang::SourceRange Range,
                                         bool &Handled);

bool TryStaticCastForHLSL(clang::Sema *Self, clang::ExprResult &SrcExpr,
                          clang::QualType DestType,
                          clang::Sema::CheckedConversionKind CCK,
                          const clang::SourceRange &OpRange, unsigned &msg,
                          clang::CastKind &Kind, clang::CXXCastPath &BasePath,
                          bool ListInitialization, bool SuppressDiagnostics,
                          clang::StandardConversionSequence *standard);

clang::ImplicitConversionSequence
TrySubscriptIndexInitialization(clang::Sema *Self, clang::Expr *SrcExpr,
                                clang::QualType DestType);

bool IsHLSLAttr(clang::attr::Kind AttrKind);
void CustomPrintHLSLAttr(const clang::Attr *A, llvm::raw_ostream &Out,
                         const clang::PrintingPolicy &Policy,
                         unsigned int Indentation);
void PrintClipPlaneIfPresent(clang::Expr *ClipPlane, llvm::raw_ostream &Out,
                             const clang::PrintingPolicy &Policy);
void Indent(unsigned int Indentation, llvm::raw_ostream &Out);
void GetHLSLAttributedTypes(clang::Sema *self, clang::QualType type,
                            const clang::AttributedType **ppMatrixOrientation,
                            const clang::AttributedType **ppNorm,
                            const clang::AttributedType **ppGLC,
                            const clang::AttributedType **ppRDC);

bool IsMatrixType(clang::Sema *self, clang::QualType type);
bool IsVectorType(clang::Sema *self, clang::QualType type);
clang::QualType GetOriginalMatrixOrVectorElementType(clang::QualType type);
clang::QualType GetOriginalElementType(clang::Sema *self, clang::QualType type);

bool IsObjectType(clang::Sema *self, clang::QualType type,
                  bool *isDeprecatedEffectObject = nullptr);

bool CanConvert(clang::Sema *self, clang::SourceLocation loc,
                clang::Expr *sourceExpr, clang::QualType target,
                bool explicitConversion,
                clang::StandardConversionSequence *standard);

// This function takes the external sema source rather than the sema object
// itself because the wire-up doesn't happen until parsing is initialized and we
// want to set this up earlier. If the HLSL constructs in the external sema move
// to Sema itself, this can be invoked on the Sema object directly.
void RegisterIntrinsicTable(clang::ExternalSemaSource *self,
                            IDxcIntrinsicTable *table);

clang::QualType CheckVectorConditional(clang::Sema *self,
                                       clang::ExprResult &Cond,
                                       clang::ExprResult &LHS,
                                       clang::ExprResult &RHS,
                                       clang::SourceLocation QuestionLoc);
} // namespace hlsl

bool IsTypeNumeric(clang::Sema *self, clang::QualType &type);
bool IsExprAccessingOutIndicesArray(clang::Expr *BaseExpr);

// This function reads the given declaration TSS and returns the corresponding
// parsedType with the corresponding type. Replaces the given parsed type with
// the new type
clang::QualType ApplyTypeSpecSignToParsedType(clang::Sema *self,
                                              clang::QualType &type,
                                              clang::TypeSpecifierSign TSS,
                                              clang::SourceLocation Loc);

#endif
