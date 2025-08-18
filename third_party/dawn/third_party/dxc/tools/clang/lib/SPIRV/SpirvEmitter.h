//===------- SpirvEmitter.h - SPIR-V Binary Code Emitter --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines a SPIR-V emitter class that takes in HLSL AST and emits
//  SPIR-V binary words.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_SPIRV_SPIRVEMITTER_H
#define LLVM_CLANG_LIB_SPIRV_SPIRVEMITTER_H

#include <stack>
#include <string>
#include <utility>
#include <vector>

#include "dxc/DXIL/DxilShaderModel.h"
#include "dxc/HlslIntrinsicOp.h"
#include "spirv/unified1/GLSL.std.450.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ParentMap.h"
#include "clang/AST/TypeOrdering.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/SPIRV/FeatureManager.h"
#include "clang/SPIRV/SpirvBuilder.h"
#include "clang/SPIRV/SpirvContext.h"
#include "llvm/ADT/STLExtras.h"

#include "ConstEvaluator.h"
#include "DeclResultIdMapper.h"
#include "spirv-tools/optimizer.hpp"

namespace spvtools {
namespace opt {

// A struct for a pair of descriptor set and binding.
struct DescriptorSetAndBinding {
  uint32_t descriptor_set;
  uint32_t binding;
};

} // namespace opt
} // namespace spvtools

namespace clang {
namespace spirv {

/// SPIR-V emitter class. It consumes the HLSL AST and emits SPIR-V words.
///
/// This class only overrides the HandleTranslationUnit() method; Traversing
/// through the AST is done manually instead of using ASTConsumer's harness.
class SpirvEmitter : public ASTConsumer {
public:
  SpirvEmitter(CompilerInstance &ci);

  void HandleTranslationUnit(ASTContext &context) override;

  ASTContext &getASTContext() { return astContext; }
  SpirvBuilder &getSpirvBuilder() { return spvBuilder; }
  SpirvContext &getSpirvContext() { return spvContext; }
  DiagnosticsEngine &getDiagnosticsEngine() { return diags; }
  CompilerInstance &getCompilerInstance() { return theCompilerInstance; }
  SpirvCodeGenOptions &getSpirvOptions() { return spirvOptions; }

  /// \brief If DebugSource and DebugCompilationUnit for loc are already
  /// created, we just return RichDebugInfo containing it. Otherwise,
  /// create DebugSource and DebugCompilationUnit for loc and return it.
  RichDebugInfo *getOrCreateRichDebugInfo(const SourceLocation &loc);
  RichDebugInfo *getOrCreateRichDebugInfoImpl(llvm::StringRef file);

  void doDecl(const Decl *decl);
  void doStmt(const Stmt *stmt, llvm::ArrayRef<const Attr *> attrs = {});
  SpirvInstruction *doExpr(const Expr *expr, SourceRange rangeOverride = {});
  SpirvInstruction *doExprEnsuringRValue(const Expr *expr,
                                         SourceLocation location,
                                         SourceRange range);

  /// Processes the given expression and emits SPIR-V instructions. If the
  /// result is a GLValue, does an additional load.
  ///
  /// This method is useful for cases where ImplicitCastExpr (LValueToRValue) is
  /// missing when using an lvalue as rvalue in the AST, e.g., DeclRefExpr will
  /// not be wrapped in ImplicitCastExpr (LValueToRValue) when appearing in
  /// HLSLVectorElementExpr since the generated HLSLVectorElementExpr itself can
  /// be lvalue or rvalue.
  SpirvInstruction *loadIfGLValue(const Expr *expr,
                                  SourceRange rangeOverride = {});

  /// Casts the given value from fromType to toType. fromType and toType should
  /// both be scalar or vector types of the same size.
  SpirvInstruction *castToType(SpirvInstruction *value, QualType fromType,
                               QualType toType, SourceLocation,
                               SourceRange range = {});

  /// Returns true if the given VarDecl will be translated into a SPIR-V
  /// variable not in the Private or Function storage class.
  static inline bool isExternalVar(const VarDecl *var) {
    // Class static variables should be put in the Private storage class.
    // groupshared variables are allowed to be declared as "static". But we
    // still need to put them in the Workgroup storage class. That is, when
    // seeing "static groupshared", ignore "static".
    return var->hasExternalFormalLinkage()
               ? !var->isStaticDataMember()
               : (var->getAttr<HLSLGroupSharedAttr>() != nullptr);
  }

  /// Create SpirvIntrinsicInstruction for arbitrary SPIR-V instructions
  /// specified by [[vk::ext_instruction(..)]] or [[vk::ext_type_def(..)]]
  SpirvInstruction *
  createSpirvIntrInstExt(llvm::ArrayRef<const Attr *> attrs, QualType retType,
                         llvm::ArrayRef<SpirvInstruction *> spvArgs,
                         bool isInstr, SourceLocation loc);

  /// \brief Negates to get the additive inverse of SV_Position.y if requested.
  SpirvInstruction *invertYIfRequested(SpirvInstruction *position,
                                       SourceLocation loc,
                                       SourceRange range = {});

private:
  bool handleNodePayloadArrayType(const ParmVarDecl *decl,
                                  SpirvInstruction *instr);
  void doFunctionDecl(const FunctionDecl *decl);
  void doVarDecl(const VarDecl *decl);
  void doRecordDecl(const RecordDecl *decl);
  void doClassTemplateDecl(const ClassTemplateDecl *classTemplateDecl);
  void doEnumDecl(const EnumDecl *decl);
  void doHLSLBufferDecl(const HLSLBufferDecl *decl);
  void doImplicitDecl(const Decl *decl);

  void doBreakStmt(const BreakStmt *stmt);
  void doDiscardStmt(const DiscardStmt *stmt);
  inline void doDeclStmt(const DeclStmt *stmt);
  void doForStmt(const ForStmt *, llvm::ArrayRef<const Attr *> attrs = {});
  void doIfStmt(const IfStmt *ifStmt, llvm::ArrayRef<const Attr *> attrs = {});
  void doReturnStmt(const ReturnStmt *stmt);
  void doSwitchStmt(const SwitchStmt *stmt,
                    llvm::ArrayRef<const Attr *> attrs = {});
  void doWhileStmt(const WhileStmt *, llvm::ArrayRef<const Attr *> attrs = {});
  void doDoStmt(const DoStmt *, llvm::ArrayRef<const Attr *> attrs = {});
  void doContinueStmt(const ContinueStmt *);

  SpirvInstruction *doArraySubscriptExpr(const ArraySubscriptExpr *expr,
                                         SourceRange rangeOverride = {});
  SpirvInstruction *doBinaryOperator(const BinaryOperator *expr);
  SpirvInstruction *doCallExpr(const CallExpr *callExpr,
                               SourceRange rangeOverride = {});
  SpirvInstruction *doCastExpr(const CastExpr *expr,
                               SourceRange rangeOverride = {});
  SpirvInstruction *doCompoundAssignOperator(const CompoundAssignOperator *);
  SpirvInstruction *doConditionalOperator(const ConditionalOperator *expr);
  SpirvInstruction *doConditional(const Expr *expr, const Expr *cond,
                                  const Expr *falseExpr, const Expr *trueExpr);
  SpirvInstruction *
  doShortCircuitedConditionalOperator(const ConditionalOperator *expr);
  SpirvInstruction *doCXXMemberCallExpr(const CXXMemberCallExpr *expr);
  SpirvInstruction *doCXXOperatorCallExpr(const CXXOperatorCallExpr *expr,
                                          SourceRange rangeOverride = {});
  SpirvInstruction *doExtMatrixElementExpr(const ExtMatrixElementExpr *expr);
  SpirvInstruction *doHLSLVectorElementExpr(const HLSLVectorElementExpr *expr,
                                            SourceRange rangeOverride = {});
  SpirvInstruction *doInitListExpr(const InitListExpr *expr,
                                   SourceRange rangeOverride = {});
  SpirvInstruction *doMemberExpr(const MemberExpr *expr,
                                 SourceRange rangeOverride = {});
  SpirvInstruction *doUnaryOperator(const UnaryOperator *expr);
  SpirvInstruction *
  doUnaryExprOrTypeTraitExpr(const UnaryExprOrTypeTraitExpr *expr);

  /// Overload with pre computed SpirvEvalInfo.
  ///
  /// The given expr will not be evaluated again.
  SpirvInstruction *loadIfGLValue(const Expr *expr, SpirvInstruction *info,
                                  SourceRange rangeOverride = {});

  /// Loads the pointer of the aliased-to-variable if the given expression is a
  /// DeclRefExpr referencing an alias variable. See DeclResultIdMapper for
  /// more explanation regarding this.
  ///
  /// Note: legalization specific code
  SpirvInstruction *loadIfAliasVarRef(const Expr *expr,
                                      SourceRange rangeOverride = {});

  /// Loads the pointer of the aliased-to-variable and ajusts aliasVarInfo
  /// accordingly if aliasVarExpr is referencing an alias variable. Returns true
  /// if aliasVarInfo is changed, false otherwise.
  ///
  /// Note: legalization specific code
  bool loadIfAliasVarRef(const Expr *aliasVarExpr,
                         SpirvInstruction **aliasVarInstr,
                         SourceRange rangeOverride = {});

  /// Check whether a member value has a nointerpolation qualifier in its type
  /// declaration or any parents' type declaration recursively.
  bool isNoInterpMemberExpr(const MemberExpr *expr);

private:
  /// Translates the given frontend binary operator into its SPIR-V equivalent
  /// taking consideration of the operand type.
  spv::Op translateOp(BinaryOperator::Opcode op, QualType type);

  spv::Op translateWaveOp(hlsl::IntrinsicOp op, QualType type, SourceLocation);

  /// Generates SPIR-V instructions for the given normal (non-intrinsic and
  /// non-operator) standalone or member function call.
  SpirvInstruction *processCall(const CallExpr *expr);

  /// Generates the necessary instructions for assigning rhs to lhs. If lhsPtr
  /// is not zero, it will be used as the pointer from lhs instead of evaluating
  /// lhs again.
  SpirvInstruction *processAssignment(const Expr *lhs, SpirvInstruction *rhs,
                                      bool isCompoundAssignment,
                                      SpirvInstruction *lhsPtr = nullptr,
                                      SourceRange range = {});

  /// Generates SPIR-V instructions to store rhsVal into lhsPtr. This will be
  /// recursive if lhsValType is a composite type. rhsExpr will be used as a
  /// reference to adjust the CodeGen if not nullptr.
  void storeValue(SpirvInstruction *lhsPtr, SpirvInstruction *rhsVal,
                  QualType lhsValType, SourceLocation loc,
                  SourceRange range = {});

  bool canUseOpCopyLogical(QualType type) const;

  /// Decomposes and reconstructs the given srcVal of the given valType to meet
  /// the requirements of the dstLR layout rule.
  SpirvInstruction *reconstructValue(SpirvInstruction *srcVal, QualType valType,
                                     SpirvLayoutRule dstLR, SourceLocation loc,
                                     SourceRange range = {});

  /// Generates the necessary instructions for conducting the given binary
  /// operation on lhs and rhs.
  ///
  /// computationType is the type for LHS and RHS when doing computation, while
  /// resultType is the type of the whole binary operation. They can be
  /// different for compound assignments like <some-int-value> *=
  /// <some-float-value>, where computationType is float and resultType is int.
  ///
  /// If lhsResultId is not nullptr, the evaluated pointer from lhs during the
  /// process will be written into it. If mandateGenOpcode is not spv::Op::Max,
  /// it will used as the SPIR-V opcode instead of deducing from Clang frontend
  /// opcode.
  SpirvInstruction *
  processBinaryOp(const Expr *lhs, const Expr *rhs, BinaryOperatorKind opcode,
                  QualType computationType, QualType resultType, SourceRange,
                  SourceLocation, SpirvInstruction **lhsInfo = nullptr,
                  spv::Op mandateGenOpcode = spv::Op::Max);

  /// Generates SPIR-V instructions to initialize the given variable once.
  void initOnce(QualType varType, std::string varName, SpirvVariable *,
                const Expr *varInit);

  /// Returns true if the given expression will be translated into a vector
  /// shuffle instruction in SPIR-V.
  ///
  /// We emit a vector shuffle instruction iff
  /// * We are not selecting only one element from the vector (OpAccessChain
  ///   or OpCompositeExtract for such case);
  /// * We are not selecting all elements in their original order (essentially
  ///   the original vector, no shuffling needed).
  bool isVectorShuffle(const Expr *expr);

  /// Returns true if the given expression is a short-circuited operator.
  bool isShortCircuitedOp(const Expr *expr);

  /// Returns true if the given statement or any of its children are a
  /// short-circuited operator.
  bool stmtTreeContainsShortCircuitedOp(const Stmt *stmt);

  /// \brief Returns true if the given CXXOperatorCallExpr is indexing into a
  /// Buffer/RWBuffer/Texture/RWTexture using operator[].
  /// On success, writes the base buffer into *base if base is not nullptr, and
  /// writes the index into *index if index is not nullptr.
  bool isBufferTextureIndexing(const CXXOperatorCallExpr *,
                               const Expr **base = nullptr,
                               const Expr **index = nullptr);

  bool isDescriptorHeap(const Expr *expr);

  void getDescriptorHeapOperands(const Expr *expr, const Expr **base,
                                 const Expr **index);

  /// \brief Returns true if the given CXXOperatorCallExpr is the .mips[][]
  /// access into a Texture or .sample[][] access into Texture2DMS(Array). On
  /// success, writes base texture object into *base if base is not nullptr,
  /// writes the index into *index if index is not nullptr, and writes the mip
  /// level (lod) to *lod if lod is not nullptr.
  bool isTextureMipsSampleIndexing(const CXXOperatorCallExpr *indexExpr,
                                   const Expr **base = nullptr,
                                   const Expr **index = nullptr,
                                   const Expr **lod = nullptr);

  /// Condenses a sequence of HLSLVectorElementExpr starting from the given
  /// expr into one. Writes the original base into *basePtr and the condensed
  /// accessor into *flattenedAccessor.
  void condenseVectorElementExpr(
      const HLSLVectorElementExpr *expr, const Expr **basePtr,
      hlsl::VectorMemberAccessPositions *flattenedAccessor);

  /// Generates necessary SPIR-V instructions to create a vector splat out of
  /// the given scalarExpr. The generated vector will have the same element
  /// type as scalarExpr and of the given size. If resultIsConstant is not
  /// nullptr, writes true to it if the generated instruction is a constant.
  SpirvInstruction *createVectorSplat(const Expr *scalarExpr, uint32_t size,
                                      SourceRange rangeOverride = {});

  /// Splits the given vector into the last element and the rest (as a new
  /// vector).
  void splitVecLastElement(QualType vecType, SpirvInstruction *vec,
                           SpirvInstruction **residual,
                           SpirvInstruction **lastElement, SourceLocation loc);

  /// Converts a vector value into the given struct type with its element type's
  /// <result-id> as elemTypeId.
  ///
  /// Assumes the vector and the struct have matching number of elements. Panics
  /// otherwise.
  SpirvInstruction *convertVectorToStruct(QualType structType,
                                          QualType elemType,
                                          SpirvInstruction *vector,
                                          SourceLocation loc,
                                          SourceRange range = {});

  /// Translates a floatN * float multiplication into SPIR-V instructions and
  /// returns the <result-id>. Returns 0 if the given binary operation is not
  /// floatN * float.
  SpirvInstruction *tryToGenFloatVectorScale(const BinaryOperator *expr);

  /// Translates a floatMxN * float multiplication into SPIR-V instructions and
  /// returns the <result-id>. Returns 0 if the given binary operation is not
  /// floatMxN * float.
  SpirvInstruction *tryToGenFloatMatrixScale(const BinaryOperator *expr);

  /// Tries to emit instructions for assigning to the given vector element
  /// accessing expression. Returns 0 if the trial fails and no instructions
  /// are generated.
  SpirvInstruction *tryToAssignToVectorElements(const Expr *lhs,
                                                SpirvInstruction *rhs,
                                                SourceRange range = {});

  /// Tries to emit instructions for assigning to the given matrix element
  /// accessing expression. Returns 0 if the trial fails and no instructions
  /// are generated.
  SpirvInstruction *tryToAssignToMatrixElements(const Expr *lhs,
                                                SpirvInstruction *rhs,
                                                SourceRange range = {});

  /// Tries to emit instructions for assigning to the given RWBuffer/RWTexture
  /// object. Returns 0 if the trial fails and no instructions are generated.
  SpirvInstruction *tryToAssignToRWBufferRWTexture(const Expr *lhs,
                                                   SpirvInstruction *rhs,
                                                   SourceRange range = {});

  /// Tries to emit instructions for assigning to the given mesh out attribute
  /// or indices object. Returns 0 if the trial fails and no instructions are
  /// generated.
  SpirvInstruction *
  tryToAssignToMSOutAttrsOrIndices(const Expr *lhs, SpirvInstruction *rhs,
                                   SpirvInstruction *vecComponent = nullptr,
                                   bool noWriteBack = false);

  /// Emit instructions for assigning to the given mesh out attribute.
  void assignToMSOutAttribute(
      const DeclaratorDecl *decl, SpirvInstruction *value,
      const llvm::SmallVector<SpirvInstruction *, 4> &indices);

  /// Emit instructions for assigning to the given mesh out indices object.
  void
  assignToMSOutIndices(const DeclaratorDecl *decl, SpirvInstruction *value,
                       const llvm::SmallVector<SpirvInstruction *, 4> &indices);

  /// Processes each vector within the given matrix by calling actOnEachVector.
  /// matrixVal should be the loaded value of the matrix. actOnEachVector takes
  /// three parameters for the current vector: the index, the <type-id>, and
  /// the value. It returns the <result-id> of the processed vector.
  SpirvInstruction *processEachVectorInMatrix(
      const Expr *matrix, SpirvInstruction *matrixVal,
      llvm::function_ref<SpirvInstruction *(uint32_t, QualType, QualType,
                                            SpirvInstruction *)>
          actOnEachVector,
      SourceLocation loc = {}, SourceRange range = {});

  SpirvInstruction *processEachVectorInMatrix(
      const Expr *matrix, QualType outputType, SpirvInstruction *matrixVal,
      llvm::function_ref<SpirvInstruction *(uint32_t, QualType, QualType,
                                            SpirvInstruction *)>
          actOnEachVector,
      SourceLocation loc = {}, SourceRange range = {});

  /// Translates the given varDecl into a spec constant.
  void createSpecConstant(const VarDecl *varDecl);

  /// Generates the necessary instructions for conducting the given binary
  /// operation on lhs and rhs.
  ///
  /// This method expects that both lhs and rhs are SPIR-V acceptable matrices.
  SpirvInstruction *processMatrixBinaryOp(const Expr *lhs, const Expr *rhs,
                                          const BinaryOperatorKind opcode,
                                          SourceRange, SourceLocation);

  /// Creates a temporary local variable in the current function of the given
  /// varType and varName. Initializes the variable with the given initValue.
  /// Returns the instruction pointer for the variable.
  SpirvVariable *createTemporaryVar(QualType varType, llvm::StringRef varName,
                                    SpirvInstruction *initValue,
                                    SourceLocation loc);

  /// Collects all indices from consecutive MemberExprs, ArraySubscriptExprs and
  /// CXXOperatorCallExprs. Also special handles all mesh shader out attributes
  /// to return the entire expression in order for caller to extract the member
  /// expression.
  const Expr *
  collectArrayStructIndices(const Expr *expr, bool rawIndex,
                            llvm::SmallVectorImpl<uint32_t> *rawIndices,
                            llvm::SmallVectorImpl<SpirvInstruction *> *indices,
                            bool *isMSOutAttribute = nullptr,
                            bool *isNointerp = nullptr);

  /// For L-values, creates an access chain to index into the given SPIR-V
  /// evaluation result and returns the new SPIR-V evaluation result.
  /// For R-values, stores it in a variable, then create the access chain and
  /// return the evaluation result.
  SpirvInstruction *derefOrCreatePointerToValue(
      QualType baseType, SpirvInstruction *base, QualType elemType,
      const llvm::SmallVector<SpirvInstruction *, 4> &indices,
      SourceLocation loc, SourceRange range = {});

  SpirvVariable *turnIntoLValue(QualType type, SpirvInstruction *source,
                                SourceLocation loc);

private:
  /// Validates that vk::* attributes are used correctly and returns false if
  /// errors are found.
  bool validateVKAttributes(const NamedDecl *decl);

  /// Records any Spir-V capabilities and extensions for the given varDecl so
  /// they will be added to the SPIR-V module.
  void registerCapabilitiesAndExtensionsForVarDecl(const VarDecl *varDecl);

private:
  /// Converts the given value from the bitwidth of 'fromType' to the bitwidth
  /// of 'toType'. If the two have the same bitwidth, returns the value itself.
  /// If resultType is not nullptr, the resulting value's type will be written
  /// to resultType. Panics if the given types are not scalar or vector of
  /// float/integer type.
  SpirvInstruction *convertBitwidth(SpirvInstruction *value, SourceLocation loc,
                                    QualType fromType, QualType toType,
                                    QualType *resultType = nullptr,
                                    SourceRange range = {});

  /// Processes the given expr, casts the result into the given bool (vector)
  /// type and returns the <result-id> of the casted value.
  SpirvInstruction *castToBool(SpirvInstruction *value, QualType fromType,
                               QualType toType, SourceLocation loc,
                               SourceRange range = {});

  /// Processes the given expr, casts the result into the given integer (vector)
  /// type and returns the <result-id> of the casted value.
  SpirvInstruction *castToInt(SpirvInstruction *value, QualType fromType,
                              QualType toType, SourceLocation,
                              SourceRange srcRange = {});

  /// Processes the given expr, casts the result into the given float (vector)
  /// type and returns the <result-id> of the casted value.
  SpirvInstruction *castToFloat(SpirvInstruction *value, QualType fromType,
                                QualType toType, SourceLocation,
                                SourceRange range = {});

private:
  /// Processes HLSL instrinsic functions.
  SpirvInstruction *processIntrinsicCallExpr(const CallExpr *);

  /// Processes the 'clip' intrinsic function. Discards the current pixel if the
  /// specified value is less than zero.
  SpirvInstruction *processIntrinsicClip(const CallExpr *);

  /// Processes the 'dst' intrinsic function.
  SpirvInstruction *processIntrinsicDst(const CallExpr *);

  /// Processes the 'clamp' intrinsic function.
  SpirvInstruction *processIntrinsicClamp(const CallExpr *);

  /// Processes the 'frexp' intrinsic function.
  SpirvInstruction *processIntrinsicFrexp(const CallExpr *);

  /// Processes the 'ldexp' intrinsic function.
  SpirvInstruction *processIntrinsicLdexp(const CallExpr *);

  /// Processes the 'D3DCOLORtoUBYTE4' intrinsic function.
  SpirvInstruction *processD3DCOLORtoUBYTE4(const CallExpr *);

  /// Processes the 'lit' intrinsic function.
  SpirvInstruction *processIntrinsicLit(const CallExpr *);

  /// Processes the 'vk::static_pointer_cast' and 'vk_reinterpret_pointer_cast'
  /// intrinsic functions.
  SpirvInstruction *processIntrinsicPointerCast(const CallExpr *,
                                                bool isStatic);

  /// Processes the vk::BufferPointer intrinsic function 'Get'.
  SpirvInstruction *
  processIntrinsicGetBufferContents(const CXXMemberCallExpr *);

  /// Processes the 'Barrier' intrinsic function.
  SpirvInstruction *processIntrinsicBarrier(const CallExpr *);

  /// Processes the 'GroupMemoryBarrier', 'GroupMemoryBarrierWithGroupSync',
  /// 'DeviceMemoryBarrier', 'DeviceMemoryBarrierWithGroupSync',
  /// 'AllMemoryBarrier', and 'AllMemoryBarrierWithGroupSync' intrinsic
  /// functions.
  SpirvInstruction *processIntrinsicMemoryBarrier(const CallExpr *,
                                                  bool isDevice, bool groupSync,
                                                  bool isAllBarrier);

  /// Processes the 'GetRemainingRecursionLevels' intrinsic function.
  SpirvInstruction *
  processIntrinsicGetRemainingRecursionLevels(const CallExpr *callExpr);

  /// Processes the 'IsValid' intrinsic function.
  SpirvInstruction *processIntrinsicIsValid(const CXXMemberCallExpr *callExpr);

  /// Processes the 'Get' intrinsic function for (arrays of) node records and
  /// the array subscript operator for node record arrays.
  SpirvInstruction *
  processIntrinsicExtractRecordStruct(const CXXMemberCallExpr *callExpr);

  /// Processes the 'GetGroupNodeOutputRecords' and 'GetThreadNodeOutputRecords'
  /// intrinsic functions.
  SpirvInstruction *
  processIntrinsicGetNodeOutputRecords(const CXXMemberCallExpr *callExpr,
                                       bool isGroupShared);

  /// Processes the 'IncrementOutputCount' intrinsic function.
  SpirvInstruction *
  processIntrinsicIncrementOutputCount(const CXXMemberCallExpr *callExpr,
                                       bool isGroupShared);

  /// Processes the 'Count' intrinsic function for node input record arrays.
  SpirvInstruction *
  processIntrinsicGetRecordCount(const CXXMemberCallExpr *callExpr);

  /// Processes the 'OutputComplete' intrinsic function.
  void processIntrinsicOutputComplete(const CXXMemberCallExpr *callExpr);

  /// Processes the 'FinishedCrossGroupSharing' intrinsic function.
  SpirvInstruction *
  processIntrinsicFinishedCrossGroupSharing(const CXXMemberCallExpr *callExpr);

  /// Processes the 'mad' intrinsic function.
  SpirvInstruction *processIntrinsicMad(const CallExpr *);

  /// Processes the 'modf' intrinsic function.
  SpirvInstruction *processIntrinsicModf(const CallExpr *);

  /// Processes the 'msad4' intrinsic function.
  SpirvInstruction *processIntrinsicMsad4(const CallExpr *);

  /// Processes the 'mul' intrinsic function.
  SpirvInstruction *processIntrinsicMul(const CallExpr *);

  /// Processes the 'printf' intrinsic function.
  SpirvInstruction *processIntrinsicPrintf(const CallExpr *);

  /// Transposes a non-floating point matrix and returns the result-id of the
  /// transpose.
  SpirvInstruction *processNonFpMatrixTranspose(QualType matType,
                                                SpirvInstruction *matrix,
                                                SourceLocation loc,
                                                SourceRange range = {});

  /// Processes the dot product of two non-floating point vectors. The SPIR-V
  /// OpDot only accepts float vectors. Assumes that the two vectors are of the
  /// same size and have the same element type (elemType).
  SpirvInstruction *processNonFpDot(SpirvInstruction *vec1Id,
                                    SpirvInstruction *vec2Id, uint32_t vecSize,
                                    QualType elemType, SourceLocation loc,
                                    SourceRange range = {});

  /// Processes the multiplication of a *non-floating point* matrix by a scalar.
  /// Assumes that the matrix element type and the scalar type are the same.
  SpirvInstruction *
  processNonFpScalarTimesMatrix(QualType scalarType, SpirvInstruction *scalar,
                                QualType matType, SpirvInstruction *matrix,
                                SourceLocation loc, SourceRange range = {});

  /// Processes the multiplication of a *non-floating point* matrix by a vector.
  /// Assumes the matrix element type and the vector element type are the same.
  /// Notice that the vector in this case is a "row vector" and will be
  /// multiplied by the matrix columns (dot product). As a result, the given
  /// matrix must be transposed in order to easily get each column. If
  /// 'matrixTranspose' is non-zero, it will be used as the transpose matrix
  /// result-id; otherwise the function will perform the transpose itself.
  SpirvInstruction *processNonFpVectorTimesMatrix(
      QualType vecType, SpirvInstruction *vector, QualType matType,
      SpirvInstruction *matrix, SourceLocation loc,
      SpirvInstruction *matrixTranspose = nullptr, SourceRange range = {});

  /// Processes the multiplication of a vector by a *non-floating point* matrix.
  /// Assumes the matrix element type and the vector element type are the same.
  SpirvInstruction *
  processNonFpMatrixTimesVector(QualType matType, SpirvInstruction *matrix,
                                QualType vecType, SpirvInstruction *vector,
                                SourceLocation loc, SourceRange range = {});

  /// Processes a non-floating point matrix multiplication. Assumes that the
  /// number of columns in lhs matrix is the same as number of rows in the rhs
  /// matrix. Also assumes that the two matrices have the same element type.
  SpirvInstruction *
  processNonFpMatrixTimesMatrix(QualType lhsType, SpirvInstruction *lhs,
                                QualType rhsType, SpirvInstruction *rhs,
                                SourceLocation loc, SourceRange range = {});

  /// Processes the 'dot' intrinsic function.
  SpirvInstruction *processIntrinsicDot(const CallExpr *);

  /// Processes the 'log10' intrinsic function.
  SpirvInstruction *processIntrinsicLog10(const CallExpr *);

  /// Processes the 'all' and 'any' intrinsic functions.
  SpirvInstruction *processIntrinsicAllOrAny(const CallExpr *, spv::Op);

  /// Processes the 'asfloat', 'asint', and 'asuint' intrinsic functions.
  SpirvInstruction *processIntrinsicAsType(const CallExpr *);

  /// Processes the 'saturate' intrinsic function.
  SpirvInstruction *processIntrinsicSaturate(const CallExpr *);

  /// Processes the 'sincos' intrinsic function.
  SpirvInstruction *processIntrinsicSinCos(const CallExpr *);

  /// Processes the 'isFinite' intrinsic function.
  SpirvInstruction *processIntrinsicIsFinite(const CallExpr *);

  /// Processes the 'rcp' intrinsic function.
  SpirvInstruction *processIntrinsicRcp(const CallExpr *);

  /// Processes the 'ReadClock' intrinsic function.
  SpirvInstruction *processIntrinsicReadClock(const CallExpr *);

  /// Processes the 'sign' intrinsic function for float types.
  /// The FSign instruction in the GLSL instruction set returns a floating point
  /// result. The HLSL sign function, however, returns an integer. An extra
  /// casting from float to integer is therefore performed by this method.
  SpirvInstruction *processIntrinsicFloatSign(const CallExpr *);

  /// Processes the 'f16to32' intrinsic function.
  SpirvInstruction *processIntrinsicF16ToF32(const CallExpr *);
  /// Processes the 'f32tof16' intrinsic function.
  SpirvInstruction *processIntrinsicF32ToF16(const CallExpr *);

  /// Processes the given intrinsic function call using the given GLSL
  /// extended instruction. If the given instruction cannot operate on matrices,
  /// it performs the instruction on each row of the matrix and uses composite
  /// construction to generate the resulting matrix.
  SpirvInstruction *processIntrinsicUsingGLSLInst(const CallExpr *,
                                                  GLSLstd450 instr,
                                                  bool canOperateOnMatrix,
                                                  SourceLocation,
                                                  SourceRange range = {});

  /// Processes the given intrinsic function call using the given SPIR-V
  /// instruction. If the given instruction cannot operate on matrices, it
  /// performs the instruction on each row of the matrix and uses composite
  /// construction to generate the resulting matrix.
  SpirvInstruction *processIntrinsicUsingSpirvInst(const CallExpr *, spv::Op,
                                                   bool canOperateOnMatrix);

  /// Processes the given intrinsic member call.
  SpirvInstruction *processIntrinsicMemberCall(const CXXMemberCallExpr *expr,
                                               hlsl::IntrinsicOp opcode);

  /// Processes EvaluateAttributeAt* intrinsic calls.
  SpirvInstruction *processEvaluateAttributeAt(const CallExpr *expr,
                                               hlsl::IntrinsicOp opcode,
                                               SourceLocation loc,
                                               SourceRange range);

  /// Processes Interlocked* intrinsic functions.
  SpirvInstruction *processIntrinsicInterlockedMethod(const CallExpr *,
                                                      hlsl::IntrinsicOp);
  /// Processes SM6.0 wave query intrinsic calls.
  SpirvInstruction *processWaveQuery(const CallExpr *, spv::Op opcode);

  /// Processes SM6.6 IsHelperLane intrisic calls.
  SpirvInstruction *processIsHelperLane(const CallExpr *, SourceLocation loc,
                                        SourceRange range);

  /// Processes SM6.0 wave vote intrinsic calls.
  SpirvInstruction *processWaveVote(const CallExpr *, spv::Op opcode);

  /// Processes SM6.0 wave active/prefix count bits.
  SpirvInstruction *processWaveCountBits(const CallExpr *,
                                         spv::GroupOperation groupOp);

  /// Processes SM6.0 wave reduction or scan/prefix and SM6.5 wave multiprefix
  /// intrinsic calls.
  SpirvInstruction *processWaveReductionOrPrefix(const CallExpr *, spv::Op op,
                                                 spv::GroupOperation groupOp);

  /// Processes SM6.0 wave broadcast intrinsic calls.
  SpirvInstruction *processWaveBroadcast(const CallExpr *);

  /// Processes SM6.0 quad-wide shuffle.
  SpirvInstruction *processWaveQuadWideShuffle(const CallExpr *,
                                               hlsl::IntrinsicOp op);

  /// Processes SM6.7 quad any/all.
  SpirvInstruction *processWaveQuadAnyAll(const CallExpr *,
                                          hlsl::IntrinsicOp op);

  /// Generates the Spir-V instructions needed to implement the given call to
  /// WaveActiveAllEqual. Returns a pointer to the instruction that produces the
  /// final result.
  SpirvInstruction *processWaveActiveAllEqual(const CallExpr *);

  /// Generates the Spir-V instructions needed to implement WaveActiveAllEqual
  /// with the scalar input `arg`. Returns a pointer to the instruction that
  /// produces the final result. srcLoc should be the source location of the
  /// original call.
  SpirvInstruction *
  processWaveActiveAllEqualScalar(SpirvInstruction *arg,
                                  clang::SourceLocation srcLoc);

  /// Generates the Spir-V instructions needed to implement WaveActiveAllEqual
  /// with the vector input `arg`. Returns a pointer to the instruction that
  /// produces the final result. srcLoc should be the source location of the
  /// original call.
  SpirvInstruction *
  processWaveActiveAllEqualVector(SpirvInstruction *arg,
                                  clang::SourceLocation srcLoc);

  /// Generates the Spir-V instructions needed to implement WaveActiveAllEqual
  /// with the matrix input `arg`. Returns a pointer to the instruction that
  /// produces the final result. srcLoc should be the source location of the
  /// original call.
  SpirvInstruction *
  processWaveActiveAllEqualMatrix(SpirvInstruction *arg, QualType,
                                  clang::SourceLocation srcLoc);

  /// Processes SM6.5 WaveMatch function.
  SpirvInstruction *processWaveMatch(const CallExpr *);

  /// Processes the NonUniformResourceIndex intrinsic function.
  SpirvInstruction *processIntrinsicNonUniformResourceIndex(const CallExpr *);

  /// Processes the SM 6.4 dot4add_{i|u}8packed intrinsic functions.
  SpirvInstruction *processIntrinsicDP4a(const CallExpr *callExpr,
                                         hlsl::IntrinsicOp op);

  /// Processes the SM 6.4 dot2add intrinsic function.
  SpirvInstruction *processIntrinsicDP2a(const CallExpr *callExpr);

  /// Processes the SM 6.6 pack_{s|u}8 and pack_clamp_{s|u}8 intrinsic
  /// functions.
  SpirvInstruction *processIntrinsic8BitPack(const CallExpr *,
                                             hlsl::IntrinsicOp);

  /// Processes the SM 6.6 unpack_{s|u}8{s|u}{16|32} intrinsic functions.
  SpirvInstruction *processIntrinsic8BitUnpack(const CallExpr *,
                                               hlsl::IntrinsicOp);

  /// Process builtins specific to raytracing.
  SpirvInstruction *processRayBuiltins(const CallExpr *, hlsl::IntrinsicOp op);

  /// Process raytracing intrinsics.
  SpirvInstruction *processReportHit(const CallExpr *);
  void processCallShader(const CallExpr *callExpr);
  void processTraceRay(const CallExpr *callExpr);

  /// Process amplification shader intrinsics.
  void processDispatchMesh(const CallExpr *callExpr);

  /// Process mesh shader intrinsics.
  void processMeshOutputCounts(const CallExpr *callExpr);

  /// Process GetAttributeAtVertex for barycentrics.
  SpirvInstruction *processGetAttributeAtVertex(const CallExpr *expr);

  /// Process ray query traceinline intrinsics.
  SpirvInstruction *processTraceRayInline(const CXXMemberCallExpr *expr);

  /// Process ray query intrinsics
  SpirvInstruction *processRayQueryIntrinsics(const CXXMemberCallExpr *expr,
                                              hlsl::IntrinsicOp opcode);
  /// Process spirv intrinsic instruction
  SpirvInstruction *processSpvIntrinsicCallExpr(const CallExpr *expr);

  /// Process spirv intrinsic type definition
  SpirvInstruction *processSpvIntrinsicTypeDef(const CallExpr *expr);

  /// Process `T vk::RawBufferLoad<T>(in uint64_t address
  /// [, in uint alignment])` that loads data from a given device address.
  SpirvInstruction *processRawBufferLoad(const CallExpr *callExpr);
  SpirvInstruction *loadDataFromRawAddress(SpirvInstruction *addressInUInt64,
                                           QualType bufferType,
                                           uint32_t alignment,
                                           SourceLocation loc);

  /// Process `void vk::RawBufferStore<T>(in uint64_t address, in T value
  /// [, in uint alignment])` that stores data to a given device address.
  SpirvInstruction *processRawBufferStore(const CallExpr *callExpr);
  SpirvInstruction *storeDataToRawAddress(SpirvInstruction *addressInUInt64,
                                          SpirvInstruction *value,
                                          QualType bufferType,
                                          uint32_t alignment,
                                          SourceLocation loc,
                                          SourceRange range);

  /// Returns the value of the alignment argument for `vk::RawBufferLoad()` and
  /// `vk::RawBufferStore()`.
  uint32_t getRawBufferAlignment(const Expr *expr);

  /// Returns a spirv OpCooperativeMatrixLengthKHR instruction generated from a
  /// call to __builtin_spv_CooperativeMatrixLengthKHR.
  SpirvInstruction *processCooperativeMatrixGetLength(const CallExpr *call);

  /// Process vk::ext_execution_mode intrinsic
  SpirvInstruction *processIntrinsicExecutionMode(const CallExpr *expr);
  /// Process vk::ext_execution_mode_id intrinsic
  SpirvInstruction *processIntrinsicExecutionModeId(const CallExpr *expr);

  /// Processes the 'firstbit{high|low}' intrinsic functions.
  SpirvInstruction *processIntrinsicFirstbit(const CallExpr *,
                                             GLSLstd450 glslOpcode);

  SpirvInstruction *
  processMatrixDerivativeIntrinsic(hlsl::IntrinsicOp hlslOpcode,
                                   const Expr *arg, SourceLocation loc,
                                   SourceRange range);

  SpirvInstruction *processDerivativeIntrinsic(hlsl::IntrinsicOp hlslOpcode,
                                               const Expr *arg,
                                               SourceLocation loc,
                                               SourceRange range);

  SpirvInstruction *processDerivativeIntrinsic(hlsl::IntrinsicOp hlslOpcode,
                                               SpirvInstruction *arg,
                                               SourceLocation loc,
                                               SourceRange range);

private:
  /// Returns the <result-id> for constant value 0 of the given type.
  SpirvConstant *getValueZero(QualType type);

  /// Returns the <result-id> for a constant zero vector of the given size and
  /// element type.
  SpirvConstant *getVecValueZero(QualType elemType, uint32_t size);

  /// Returns the <result-id> for constant value 1 of the given type.
  SpirvConstant *getValueOne(QualType type);

  /// Returns the <result-id> for a constant one vector of the given size and
  /// element type.
  SpirvConstant *getVecValueOne(QualType elemType, uint32_t size);

  /// Returns the <result-id> for a constant one (vector) having the same
  /// element type as the given matrix type.
  ///
  /// If a 1x1 matrix is given, the returned value one will be a scalar;
  /// if a Mx1 or 1xN matrix is given, the returned value one will be a
  /// vector of size M or N; if a MxN matrix is given, the returned value
  /// one will be a vector of size N.
  SpirvConstant *getMatElemValueOne(QualType type);

  /// Returns a SPIR-V constant equal to the bitwdith of the given type minus
  /// one. The returned constant has the same component count and bitwidth as
  /// the given type.
  SpirvConstant *getMaskForBitwidthValue(QualType type);

private:
  /// \brief Performs a FlatConversion implicit cast. Fills an instance of the
  /// given type with initializer <result-id>.
  SpirvInstruction *processFlatConversion(const QualType type,
                                          SpirvInstruction *initId,
                                          SourceLocation,
                                          SourceRange range = {});

private:
  /// Translates the given HLSL loop attribute into SPIR-V loop control mask.
  /// Emits an error if the given attribute is not a loop attribute.
  spv::LoopControlMask translateLoopAttribute(const Stmt *, const Attr &);

  static hlsl::ShaderModel::Kind getShaderModelKind(StringRef stageName);
  static spv::ExecutionModel getSpirvShaderStage(hlsl::ShaderModel::Kind smk,
                                                 bool);
  void checkForWaveSizeAttr(const FunctionDecl *decl);

  /// \brief Handle inline SPIR-V attributes for the entry function.
  void processInlineSpirvAttributes(const FunctionDecl *entryFunction);

  /// \brief Adds necessary execution modes for the hull/domain shaders based on
  /// the HLSL attributes of the entry point function.
  /// In the case of hull shaders, also writes the number of output control
  /// points to *numOutputControlPoints. Returns true on success, and false on
  /// failure.
  bool processTessellationShaderAttributes(const FunctionDecl *entryFunction,
                                           uint32_t *numOutputControlPoints);

  /// \brief Adds necessary execution modes for the geometry shader based on the
  /// HLSL attributes of the entry point function. Also writes the array size of
  /// the input, which depends on the primitive type, to *arraySize.
  bool processGeometryShaderAttributes(const FunctionDecl *entryFunction,
                                       uint32_t *arraySize);

  /// \brief Adds necessary execution modes for the pixel shader based on the
  /// HLSL attributes of the entry point function.
  void processPixelShaderAttributes(const FunctionDecl *decl);

  /// \brief Adds necessary execution modes for the compute shader based on the
  /// HLSL attributes of the entry point function.
  void processComputeShaderAttributes(const FunctionDecl *entryFunction);

  /// \brief Adds necessary execution modes for the node shader based on the
  /// HLSL attributes of the entry point function.
  void processNodeShaderAttributes(const FunctionDecl *entryFunction);

  /// \brief Adds necessary execution modes for the mesh/amplification shader
  /// based on the HLSL attributes of the entry point function.
  bool
  processMeshOrAmplificationShaderAttributes(const FunctionDecl *decl,
                                             uint32_t *outVerticesArraySize);

  /// \brief Emits a SpirvDebugFunction to match given SpirvFunction, and
  /// returns a pointer to it.
  SpirvDebugFunction *emitDebugFunction(const FunctionDecl *decl,
                                        SpirvFunction *func,
                                        RichDebugInfo **info, std::string name);

  /// \brief Emits a wrapper function for the entry function and returns a
  /// pointer to the wrapper SpirvFunction on success.
  ///
  /// The wrapper function loads the values of all stage input variables and
  /// creates composites as expected by the source code entry function. It then
  /// calls the source code entry point and writes out stage output variables
  /// by extracting sub-values from the return value. In this way, we can handle
  /// the source code entry point as a normal function.
  ///
  /// The wrapper function is also responsible for initializing global static
  /// variables for some cases.
  SpirvFunction *emitEntryFunctionWrapper(const FunctionDecl *entryFunction,
                                          RichDebugInfo **info,
                                          SpirvDebugFunction **debugFunction,
                                          SpirvFunction *entryFuncId);

  /// \brief Emits a wrapper function for the entry functions for raytracing
  /// stages and returns true on success.
  ///
  /// Wrapper is specific to raytracing stages since for specific stages we
  /// create specific module scoped stage variables and perform copies to them.
  /// The wrapper function is also responsible for initializing global static
  /// variables for some cases.
  bool emitEntryFunctionWrapperForRayTracing(const FunctionDecl *entryFunction,
                                             RichDebugInfo **info,
                                             SpirvDebugFunction *debugFunction,
                                             SpirvFunction *entryFuncId);

  /// \brief Performs the following operations for the Hull shader:
  /// * Creates an output variable which is an Array containing results for all
  /// control points.
  ///
  /// * If the Patch Constant Function (PCF) takes the Hull main entry function
  /// results (OutputPatch), it creates a temporary function-scope variable that
  /// is then passed to the PCF.
  ///
  /// * Adds a control barrier (OpControlBarrier) to ensure all invocations are
  /// done before PCF is called.
  ///
  /// * Prepares the necessary parameters to pass to the PCF (Can be one or more
  /// of InputPatch, OutputPatch, PrimitiveId).
  ///
  /// * The execution thread with ControlPointId (invocationID) of 0 calls the
  /// PCF. e.g. if(id == 0) pcf();
  ///
  /// * Gathers the results of the PCF and assigns them to stage output
  /// variables.
  ///
  /// The method panics if it is called for any shader kind other than Hull
  /// shaders.
  bool processHSEntryPointOutputAndPCF(
      const FunctionDecl *hullMainFuncDecl, QualType retType,
      SpirvInstruction *retVal, uint32_t numOutputControlPoints,
      SpirvInstruction *outputControlPointId, SpirvInstruction *primitiveId,
      SpirvInstruction *viewId, SpirvInstruction *hullMainInputPatch);

private:
  /// \brief Returns true iff *all* the case values in the given switch
  /// statement are integer literals. In such cases OpSwitch can be used to
  /// represent the switch statement.
  /// We only care about the case values to be compared with the selector. They
  /// may appear in the top level CaseStmt or be nested in a CompoundStmt.Fall
  /// through cases will result in the second situation.
  bool allSwitchCasesAreIntegerLiterals(const Stmt *root);

  /// \brief Recursively discovers all CaseStmt and DefaultStmt under the
  /// sub-tree of the given root. Recursively goes down the tree iff it finds a
  /// CaseStmt, DefaultStmt, or CompoundStmt. It does not recurse on other
  /// statement types. For each discovered case, a basic block is created and
  /// registered within the module, and added as a successor to the current
  /// active basic block.
  ///
  /// Writes a vector of (integer, basic block label) pairs for all cases to the
  /// given 'targets' argument. If a DefaultStmt is found, it also returns the
  /// label for the default basic block through the defaultBB parameter. This
  /// method panics if it finds a case value that is not an integer literal.
  void discoverAllCaseStmtInSwitchStmt(
      const Stmt *root, SpirvBasicBlock **defaultBB,
      std::vector<std::pair<llvm::APInt, SpirvBasicBlock *>> *targets);

  /// Flattens structured AST of the given switch statement into a vector of AST
  /// nodes and stores into flatSwitch.
  ///
  /// The AST for a switch statement may look arbitrarily different based on
  /// several factors such as placement of cases, placement of breaks, placement
  /// of braces, and fallthrough cases.
  ///
  /// A CaseStmt for instance is the child node of a CompoundStmt for
  /// regular cases and it is the child node of another CaseStmt for fallthrough
  /// cases.
  ///
  /// A BreakStmt for instance could be the child node of a CompoundStmt
  /// for regular cases, or the child node of a CaseStmt for some fallthrough
  /// cases.
  ///
  /// This method flattens the AST representation of a switch statement to make
  /// it easier to process for translation.
  /// For example:
  ///
  /// switch(a) {
  ///   case 1:
  ///     <Stmt1>
  ///   case 2:
  ///     <Stmt2>
  ///     break;
  ///   case 3:
  ///   case 4:
  ///     <Stmt4>
  ///     break;
  ///   deafult:
  ///     <Stmt5>
  /// }
  ///
  /// is flattened to the following vector:
  ///
  /// +-----+-----+-----+-----+-----+-----+-----+-----+-----+-------+-----+
  /// |Case1|Stmt1|Case2|Stmt2|Break|Case3|Case4|Stmt4|Break|Default|Stmt5|
  /// +-----+-----+-----+-----+-----+-----+-----+-----+-----+-------+-----+
  ///
  void flattenSwitchStmtAST(const Stmt *root,
                            std::vector<const Stmt *> *flatSwitch);

  void processCaseStmtOrDefaultStmt(const Stmt *stmt);

  void processSwitchStmtUsingSpirvOpSwitch(const SwitchStmt *switchStmt);
  /// Translates a switch statement into SPIR-V conditional branches.
  ///
  /// This is done by constructing AST if statements out of the cases using the
  /// following pattern:
  ///   if { ... } else if { ... } else if { ... } else { ... }
  /// And then calling the SPIR-V codegen methods for these if statements.
  ///
  /// Each case comparison is turned into an if statement, and the "then" body
  /// of the if statement will be the body of the case.
  /// If a default statements exists, it becomes the body of the "else"
  /// statement.
  void processSwitchStmtUsingIfStmts(const SwitchStmt *switchStmt);

  /// Handles the offset argument in the given method call at the given argument
  /// index. Panics if the argument at the given index does not exist. Writes
  /// the <result-id> to either *constOffset or *varOffset, depending on the
  /// constantness of the offset.
  void handleOffsetInMethodCall(const CXXMemberCallExpr *expr, uint32_t index,
                                SpirvInstruction **constOffset,
                                SpirvInstruction **varOffset);

  void handleOptionalTextureSampleArgs(const CXXMemberCallExpr *expr,
                                       uint32_t index,
                                       SpirvInstruction **constOffset,
                                       SpirvInstruction **varOffset,
                                       SpirvInstruction **clamp,
                                       SpirvInstruction **status);

  /// \brief Processes .Load() method call for Buffer/RWBuffer and texture
  /// objects.
  SpirvInstruction *processBufferTextureLoad(const CXXMemberCallExpr *);

  /// \brief Loads one element from the given Buffer/RWBuffer/Texture object at
  /// the given location. The type of the loaded element matches the type in the
  /// declaration for the Buffer/Texture object.
  /// If residencyCodeId is not zero,  the SPIR-V instruction for storing the
  /// resulting residency code will also be emitted.
  SpirvInstruction *
  processBufferTextureLoad(const Expr *object, SpirvInstruction *location,
                           SpirvInstruction *constOffset, SpirvInstruction *lod,
                           SpirvInstruction *residencyCode, SourceLocation loc,
                           SourceRange range = {});

  /// \brief Processes .Sample() and .Gather() method calls for texture objects.
  SpirvInstruction *processTextureSampleGather(const CXXMemberCallExpr *expr,
                                               bool isSample);

  /// \brief Processes .SampleBias() and .SampleLevel() method calls for texture
  /// objects.
  SpirvInstruction *processTextureSampleBiasLevel(const CXXMemberCallExpr *expr,
                                                  bool isBias);

  /// \brief Processes .SampleGrad() method call for texture objects.
  SpirvInstruction *processTextureSampleGrad(const CXXMemberCallExpr *expr);

  /// \brief Processes .SampleCmp() method call for texture objects.
  SpirvInstruction *processTextureSampleCmp(const CXXMemberCallExpr *expr);

  /// \brief Processes .SampleCmpBias() method call for texture objects.
  SpirvInstruction *processTextureSampleCmpBias(const CXXMemberCallExpr *expr);

  /// \brief Processes .SampleCmpGrad() method call for texture objects.
  SpirvInstruction *processTextureSampleCmpGrad(const CXXMemberCallExpr *expr);

  /// \brief Processes .SampleCmpLevelZero() method call for texture objects.
  SpirvInstruction *
  processTextureSampleCmpLevelZero(const CXXMemberCallExpr *expr);

  /// \brief Processes .SampleCmpLevel() method call for texture objects.
  SpirvInstruction *processTextureSampleCmpLevel(const CXXMemberCallExpr *expr);

  /// \brief Handles .Gather{|Cmp}{Red|Green|Blue|Alpha}() calls on texture
  /// types.
  SpirvInstruction *
  processTextureGatherRGBACmpRGBA(const CXXMemberCallExpr *expr, bool isCmp,
                                  uint32_t component);

  /// \brief Handles .GatherCmp() calls on texture types.
  SpirvInstruction *processTextureGatherCmp(const CXXMemberCallExpr *expr);

  /// \brief Returns the calculated level-of-detail (a single float value) for
  /// the given texture. Handles intrinsic HLSL CalculateLevelOfDetail or
  /// CalculateLevelOfDetailUnclamped function depending on the given unclamped
  /// parameter.
  SpirvInstruction *processTextureLevelOfDetail(const CXXMemberCallExpr *expr,
                                                bool unclamped);

  /// \brief Processes the .GetDimensions() call on supported objects.
  SpirvInstruction *processGetDimensions(const CXXMemberCallExpr *);

  /// \brief Queries the given (RW)Buffer/(RW)Texture image in the given expr
  /// for the requested information. Based on the dimension of the image, the
  /// following info can be queried: width, height, depth, number of mipmap
  /// levels.
  SpirvInstruction *
  processBufferTextureGetDimensions(const CXXMemberCallExpr *);

  /// \brief Generates an OpAccessChain instruction for the given
  /// (RW)StructuredBuffer.Load() method call.
  SpirvInstruction *processStructuredBufferLoad(const CXXMemberCallExpr *expr);

  /// \brief Increments or decrements the counter for RW/Append/Consume
  /// structured buffer. If loadObject is true, the object upon which the call
  /// is made will be evaluated and translated into SPIR-V.
  SpirvInstruction *incDecRWACSBufferCounter(const CXXMemberCallExpr *call,
                                             bool isInc,
                                             bool loadObject = true);

  /// Assigns the counter variable associated with srcExpr to the one associated
  /// with dstDecl if the dstDecl is an internal RW/Append/Consume structured
  /// buffer. Returns false if there is no associated counter variable for
  /// srcExpr or dstDecl.
  ///
  /// Note: legalization specific code
  bool tryToAssignCounterVar(const DeclaratorDecl *dstDecl,
                             const Expr *srcExpr);
  bool tryToAssignCounterVar(const Expr *dstExpr, const Expr *srcExpr);

  /// Returns an instruction that points to the alias counter variable with the
  /// entity represented by expr.
  ///
  /// This method only handles final alias structured buffers, which means
  /// AssocCounter#1 and AssocCounter#2.
  SpirvInstruction *
  getFinalACSBufferCounterAliasAddressInstruction(const Expr *expr);

  /// Returns an instruction that points to the counter variable with the entity
  /// represented by expr.
  ///
  /// This method only handles final alias structured buffers, which means
  /// AssocCounter#1 and AssocCounter#2.
  SpirvInstruction *getFinalACSBufferCounterInstruction(const Expr *expr);

  /// Returns the counter variable's information associated with the entity
  /// represented by the given decl.
  ///
  /// This method only handles final alias structured buffers, which means
  /// AssocCounter#1 and AssocCounter#2.
  const CounterIdAliasPair *getFinalACSBufferCounter(const Expr *expr);
  /// This method handles AssocCounter#3 and AssocCounter#4.
  const CounterVarFields *
  getIntermediateACSBufferCounter(const Expr *expr,
                                  llvm::SmallVector<uint32_t, 4> *indices);

  /// Gets or creates an ImplicitParamDecl to represent the implicit object
  /// parameter of the given method.
  const ImplicitParamDecl *
  getOrCreateDeclForMethodObject(const CXXMethodDecl *method);

  /// \brief Loads numWords 32-bit unsigned integers or stores numWords 32-bit
  /// unsigned integers (based on the doStore parameter) to the given
  /// ByteAddressBuffer. Loading is allowed from a ByteAddressBuffer or
  /// RWByteAddressBuffer. Storing is allowed only to RWByteAddressBuffer.
  /// Panics if it is not the case.
  SpirvInstruction *processByteAddressBufferLoadStore(const CXXMemberCallExpr *,
                                                      uint32_t numWords,
                                                      bool doStore);

  /// \brief Processes the GetDimensions intrinsic function call on a
  /// (RW)ByteAddressBuffer by querying the image in the given expr.
  SpirvInstruction *processByteAddressBufferStructuredBufferGetDimensions(
      const CXXMemberCallExpr *);

  /// \brief Processes the Interlocked* intrinsic function call on a
  /// RWByteAddressBuffer.
  SpirvInstruction *
  processRWByteAddressBufferAtomicMethods(hlsl::IntrinsicOp opcode,
                                          const CXXMemberCallExpr *);

  /// \brief Processes the GetSamplePosition intrinsic method call on a
  /// Texture2DMS(Array).
  SpirvInstruction *processGetSamplePosition(const CXXMemberCallExpr *);

  /// \brief Processes the SubpassLoad intrinsic function call on a
  /// SubpassInput(MS).
  SpirvInstruction *processSubpassLoad(const CXXMemberCallExpr *);

  /// \brief Generates SPIR-V instructions for the .Append()/.Consume() call on
  /// the given {Append|Consume}StructuredBuffer. Returns the <result-id> of
  /// the loaded value for .Consume; returns zero for .Append().
  SpirvInstruction *
  processACSBufferAppendConsume(const CXXMemberCallExpr *expr);

  /// \brief Generates SPIR-V instructions to emit the current vertex in GS.
  SpirvInstruction *processStreamOutputAppend(const CXXMemberCallExpr *expr);

  /// \brief Generates SPIR-V instructions to end emitting the current
  /// primitive in GS.
  SpirvInstruction *processStreamOutputRestart(const CXXMemberCallExpr *expr);

  /// \brief Emulates GetSamplePosition() for standard sample settings, i.e.,
  /// with 1, 2, 4, 8, or 16 samples. Returns float2(0) for other cases.
  SpirvInstruction *emitGetSamplePosition(SpirvInstruction *sampleCount,
                                          SpirvInstruction *sampleIndex,
                                          SourceLocation loc,
                                          SourceRange range = {});

  /// \brief Returns OpAccessChain to the struct/class object that defines
  /// memberFn when the struct/class is a base struct/class of objectType.
  /// If the struct/class that defines memberFn is not a base of objectType,
  /// returns nullptr.
  SpirvInstruction *getBaseOfMemberFunction(QualType objectType,
                                            SpirvInstruction *objInstr,
                                            const CXXMethodDecl *memberFn,
                                            SourceLocation loc);

  /// \brief Takes a vector of size 4, and returns a vector of size 1 or 2 or 3
  /// or 4. Creates a CompositeExtract or VectorShuffle instruction to extract
  /// a scalar or smaller vector from the beginning of the input vector if
  /// necessary. Assumes that 'fromId' is the <result-id> of a vector of size 4.
  /// Panics if the target vector size is not 1, 2, 3, or 4.
  SpirvInstruction *extractVecFromVec4(SpirvInstruction *fromInstr,
                                       uint32_t targetVecSize,
                                       QualType targetElemType,
                                       SourceLocation loc,
                                       SourceRange range = {});

  /// \brief Creates SPIR-V instructions for sampling the given image.
  /// It utilizes the ModuleBuilder's createImageSample and it ensures that the
  /// returned type is handled correctly.
  /// HLSL image sampling methods may return a scalar, vec1, vec2, vec3, or
  /// vec4. But non-Dref image sampling instructions in SPIR-V must always
  /// return a vec4. As a result, an extra processing step is necessary.
  SpirvInstruction *
  createImageSample(QualType retType, QualType imageType,
                    SpirvInstruction *image, SpirvInstruction *sampler,
                    SpirvInstruction *coordinate, SpirvInstruction *compareVal,
                    SpirvInstruction *bias, SpirvInstruction *lod,
                    std::pair<SpirvInstruction *, SpirvInstruction *> grad,
                    SpirvInstruction *constOffset, SpirvInstruction *varOffset,
                    SpirvInstruction *constOffsets, SpirvInstruction *sample,
                    SpirvInstruction *minLod, SpirvInstruction *residencyCodeId,
                    SourceLocation loc, SourceRange range = {});

  /// \brief Returns OpVariable to be used as 'Interface' operands of
  /// OpEntryPoint. entryPoint is the SpirvFunction for the OpEntryPoint.
  std::vector<SpirvVariable *>
  getInterfacesForEntryPoint(SpirvFunction *entryPoint);

  /// \brief Emits OpBeginInvocationInterlockEXT and add the appropriate
  /// execution mode, if it has not already been added.
  void beginInvocationInterlock(SourceLocation loc, SourceRange range);

  /// \brief If the given FunctionDecl is not already in the workQueue, creates
  /// a FunctionInfo object for it, and inserts it into the workQueue. It also
  /// updates the functionInfoMap with the proper mapping.
  void addFunctionToWorkQueue(hlsl::DXIL::ShaderKind,
                              const clang::FunctionDecl *,
                              bool isEntryFunction);

  /// \brief Helper function to run SPIRV-Tools optimizer's performance passes.
  /// Runs the SPIRV-Tools optimizer on the given SPIR-V module |mod|, and
  /// gets the info/warning/error messages via |messages|.
  /// Returns true on success and false otherwise.
  bool spirvToolsOptimize(std::vector<uint32_t> *mod, std::string *messages);

  // \brief Runs the pass represented by the given pass token on the module.
  // Returns true if the pass was successfully run. Any messages from the
  // optimizer are returned in `messages`.
  bool spirvToolsRunPass(std::vector<uint32_t> *mod,
                         spvtools::Optimizer::PassToken token,
                         std::string *messages);

  // \brief Calls SPIRV-Tools optimizer fix-opextinst-opcodes pass. This pass
  // fixes OpExtInst/OpExtInstWithForwardRefsKHR opcodes to use the correct one
  // depending of the presence of forward references.
  bool spirvToolsFixupOpExtInst(std::vector<uint32_t> *mod,
                                std::string *messages);

  // \brief Calls SPIRV-Tools optimizer's, but only with the capability trimming
  // pass. Removes unused capabilities from the given SPIR-V module |mod|, and
  // returns info/warning/error messages via |messages|. This pass doesn't trim
  // all capabilities. To see the list of supported capabilities, check the pass
  // headers.
  bool spirvToolsTrimCapabilities(std::vector<uint32_t> *mod,
                                  std::string *messages);

  // \brief Runs the upgrade memory model pass using SPIRV-Tools's optimizer.
  // This pass will modify the module, |mod|, so that it conforms to the Vulkan
  // memory model instead of the GLSL450 memory model. Returns
  // info/warning/error messages via |messages|.
  bool spirvToolsUpgradeToVulkanMemoryModel(std::vector<uint32_t> *mod,
                                            std::string *messages);

  /// \brief Helper function to run SPIRV-Tools optimizer's legalization passes.
  /// Runs the SPIRV-Tools legalization on the given SPIR-V module |mod|, and
  /// gets the info/warning/error messages via |messages|. If
  /// |dsetbindingsToCombineImageSampler| is not empty, runs
  /// --convert-to-sampled-image pass.
  /// Returns true on success and false otherwise.
  bool
  spirvToolsLegalize(std::vector<uint32_t> *mod, std::string *messages,
                     const std::vector<spvtools::opt::DescriptorSetAndBinding>
                         *dsetbindingsToCombineImageSampler);

  /// \brief Helper function to run the SPIRV-Tools validator.
  /// Runs the SPIRV-Tools validator on the given SPIR-V module |mod|, and
  /// gets the info/warning/error messages via |messages|.
  /// Returns true on success and false otherwise.
  bool spirvToolsValidate(std::vector<uint32_t> *mod, std::string *messages);

  /// Adds the appropriate derivative group execution mode to the entry point.
  /// The entry point must already have a LocalSize execution mode, which will
  ///  be used to determine which execution mode (quad or linear) is required.
  ///  This decision is made according to the rules in
  ///  https://microsoft.github.io/DirectX-Specs/d3d/HLSL_SM_6_6_Derivatives.html.
  void addDerivativeGroupExecutionMode();

  /// Creates an input variable for `param` that will be used by the patch
  /// constant function. The parameter is also added to the patch constant
  /// function. The wrapper function will copy the input variable to the
  /// parameter.
  SpirvVariable *
  createPCFParmVarAndInitFromStageInputVar(const ParmVarDecl *param);

  /// Returns a function scope parameter with the same type as |param|.
  SpirvVariable *createFunctionScopeTempFromParameter(const ParmVarDecl *param);

  /// Returns a vector of SpirvInstruction that is the decompostion of `inst`
  /// into scalars. This is recursive. For example, a struct of a 4 element
  /// vector will return 4 scalars.
  std::vector<SpirvInstruction *> decomposeToScalars(SpirvInstruction *inst);

  /// Returns a spirv instruction with the value of the given type and layout
  /// rule that is obtained by assigning each scalar in `type` to corresponding
  /// value in `scalars`. This is the inverse of `decomposeToScalars`.
  SpirvInstruction *
  generateFromScalars(QualType type, std::vector<SpirvInstruction *> &scalars,
                      SpirvLayoutRule layoutRule);

  /// Returns a spirv instruction with the value of the given type and layout
  /// rule that is obtained by assigning `scalar` each scalar in `type`. This is
  /// the same as calling `generateFromScalars` with a sufficiently large vector
  /// where every element is `scalar`.
  SpirvInstruction *splatScalarToGenerate(QualType type,
                                          SpirvInstruction *scalar,
                                          SpirvLayoutRule rule);

  /// Modifies the instruction in the code that use the GLSL450 memory module to
  /// use the Vulkan memory model. This is done only if it has been requested or
  /// the Vulkan memory model capability has been added to the module.
  bool UpgradeToVulkanMemoryModelIfNeeded(std::vector<uint32_t> *module);

  // Splits the `value`, which must be a 64-bit scalar, into two 32-bit wide
  // uints, stored in `lowbits` and `highbits`.
  void splitDouble(SpirvInstruction *value, SourceLocation loc,
                   SourceRange range, SpirvInstruction *&lowbits,
                   SpirvInstruction *&highbits);

  // Splits the value, which must be a vector with element type `elemType` and
  // size `count`, into two composite values of size `count` and type
  // `outputType`. The elements are split component-wise: the vector
  // {0x0123456789abcdef, 0x0123456789abcdef} is split into `lowbits`
  // {0x89abcdef, 0x89abcdef} and and `highbits` {0x01234567, 0x01234567}.
  void splitDoubleVector(QualType elemType, uint32_t count, QualType outputType,
                         SpirvInstruction *value, SourceLocation loc,
                         SourceRange range, SpirvInstruction *&lowbits,
                         SpirvInstruction *&highbits);

  // Splits the value, which must be a matrix with element type `elemType` and
  // dimensions `rowCount` and `colCount`, into two composite values of
  // dimensions `rowCount` and `colCount`. The elements are split
  // component-wise: the matrix
  //
  // { 0x0123456789abcdef, 0x0123456789abcdef,
  //   0x0123456789abcdef, 0x0123456789abcdef }
  //
  // is split into `lowbits`
  //
  // { 0x89abcdef, 0x89abcdef,
  //   0x89abcdef, 0x89abcdef }
  //
  //  and `highbits`
  //
  // { 0x012345678, 0x012345678,
  //   0x012345678, 0x012345678 }.
  void splitDoubleMatrix(QualType elemType, uint32_t rowCount,
                         uint32_t colCount, QualType outputType,
                         SpirvInstruction *value, SourceLocation loc,
                         SourceRange range, SpirvInstruction *&lowbits,
                         SpirvInstruction *&highbits);

public:
  /// \brief Wrapper method to create a fatal error message and report it
  /// in the diagnostic engine associated with this consumer.
  template <unsigned N>
  DiagnosticBuilder emitFatalError(const char (&message)[N],
                                   SourceLocation loc) {
    const auto diagId =
        diags.getCustomDiagID(clang::DiagnosticsEngine::Fatal, message);
    return diags.Report(loc, diagId);
  }

  /// \brief Wrapper method to create an error message and report it
  /// in the diagnostic engine associated with this consumer.
  template <unsigned N>
  DiagnosticBuilder emitError(const char (&message)[N], SourceLocation loc) {
    const auto diagId =
        diags.getCustomDiagID(clang::DiagnosticsEngine::Error, message);
    return diags.Report(loc, diagId);
  }

  /// \brief Wrapper method to create a warning message and report it
  /// in the diagnostic engine associated with this consumer
  template <unsigned N>
  DiagnosticBuilder emitWarning(const char (&message)[N], SourceLocation loc) {
    const auto diagId =
        diags.getCustomDiagID(clang::DiagnosticsEngine::Warning, message);
    return diags.Report(loc, diagId);
  }

  /// \brief Wrapper method to create a note message and report it
  /// in the diagnostic engine associated with this consumer
  template <unsigned N>
  DiagnosticBuilder emitNote(const char (&message)[N], SourceLocation loc) {
    const auto diagId =
        diags.getCustomDiagID(clang::DiagnosticsEngine::Note, message);
    return diags.Report(loc, diagId);
  }

private:
  CompilerInstance &theCompilerInstance;
  ASTContext &astContext;
  DiagnosticsEngine &diags;

  SpirvCodeGenOptions &spirvOptions;

  /// \brief Entry function name, derived from the command line
  /// and should be const.
  const llvm::StringRef hlslEntryFunctionName;

  /// \brief Structure to maintain record of all entry functions and any
  /// reachable functions.
  struct FunctionInfo {
  public:
    hlsl::ShaderModel::Kind shaderModelKind;
    const DeclaratorDecl *funcDecl;
    SpirvFunction *entryFunction;
    bool isEntryFunction;

    FunctionInfo() = default;
    FunctionInfo(hlsl::ShaderModel::Kind smk, const DeclaratorDecl *fDecl,
                 SpirvFunction *entryFunc, bool isEntryFunc)
        : shaderModelKind(smk), funcDecl(fDecl), entryFunction(entryFunc),
          isEntryFunction(isEntryFunc) {}
  };

  SpirvContext spvContext;
  FeatureManager featureManager;
  SpirvBuilder spvBuilder;
  DeclResultIdMapper declIdMapper;
  ConstEvaluator constEvaluator;

  /// \brief A map of funcDecl to its FunctionInfo. Consists of all entry
  /// functions followed by all reachable functions from the entry functions.
  llvm::DenseMap<const DeclaratorDecl *, FunctionInfo *> functionInfoMap;

  /// A queue of FunctionInfo reachable from all the entry functions.
  std::vector<const FunctionInfo *> workQueue;

  /// Get SPIR-V entrypoint name for the given FunctionInfo.
  llvm::StringRef getEntryPointName(const FunctionInfo *entryInfo);

  /// <result-id> for the entry function. Initially it is zero and will be reset
  /// when starting to translate the entry function.
  SpirvFunction *entryFunction;
  /// The current function under traversal.
  const FunctionDecl *curFunction;
  /// The SPIR-V function parameter for the current this object.
  SpirvInstruction *curThis;

  /// The source location of a push constant block we have previously seen.
  /// Invalid means no push constant blocks defined thus far.
  SourceLocation seenPushConstantAt;

  /// Indicates whether the current emitter is in specialization constant mode:
  /// all 32-bit scalar constants will be translated into OpSpecConstant.
  bool isSpecConstantMode;

  /// Whether the translated SPIR-V binary needs legalization.
  ///
  /// The following cases will require legalization:
  ///
  /// 1. Opaque types (textures, samplers) within structs
  /// 2. Structured buffer aliasing
  /// 3. Using SPIR-V instructions not allowed in the currect shader stage
  ///
  /// This covers the first and third case.
  ///
  /// If this is true, SPIRV-Tools legalization passes will be executed after
  /// the translation to legalize the generated SPIR-V binary.
  ///
  /// Note: legalization specific code
  bool needsLegalization;

  /// Whether the translated SPIR-V binary passes --before-hlsl-legalization
  /// option to spirv-val because of illegal function parameter scope.
  bool beforeHlslLegalization;

  /// Mapping from methods to the decls to represent their implicit object
  /// parameters
  ///
  /// We need this map because that we need to update the associated counters on
  /// the implicit object when invoking method calls. The ImplicitParamDecl
  /// mapped to serves as a key to find the associated counters in
  /// DeclResultIdMapper.
  ///
  /// Note: legalization specific code
  llvm::DenseMap<const CXXMethodDecl *, const ImplicitParamDecl *> thisDecls;

  /// Global variables that should be initialized once at the begining of the
  /// entry function.
  llvm::SmallVector<const VarDecl *, 4> toInitGloalVars;

  /// For loops, while loops, and switch statements may encounter "break"
  /// statements that alter their control flow. At any point the break statement
  /// is observed, the control flow jumps to the inner-most scope's merge block.
  /// For instance: the break in the following example should cause a branch to
  /// the SwitchMergeBB, not ForLoopMergeBB:
  /// for (...) {
  ///   switch(...) {
  ///     case 1: break;
  ///   }
  ///   <--- SwitchMergeBB
  /// }
  /// <---- ForLoopMergeBB
  /// This stack keeps track of the basic blocks to which branching could occur.
  std::stack<SpirvBasicBlock *> breakStack;

  /// Loops (do, while, for) may encounter "continue" statements that alter
  /// their control flow. At any point the continue statement is observed, the
  /// control flow jumps to the inner-most scope's continue block.
  /// This stack keeps track of the basic blocks to which such branching could
  /// occur.
  std::stack<SpirvBasicBlock *> continueStack;

  /// Maps a given statement to the basic block that is associated with it.
  llvm::DenseMap<const Stmt *, SpirvBasicBlock *> stmtBasicBlock;

  /// Maintains mapping from a type to SPIR-V variable along with SPIR-V
  /// instruction for id of location decoration Used for raytracing stage
  /// variables of storage class RayPayloadNV, CallableDataNV and
  /// HitAttributeNV.
  llvm::SmallDenseMap<QualType,
                      std::pair<SpirvInstruction *, SpirvInstruction *>, 4>
      rayPayloadMap;
  llvm::SmallDenseMap<QualType, SpirvInstruction *, 4> hitAttributeMap;
  llvm::SmallDenseMap<QualType,
                      std::pair<SpirvInstruction *, SpirvInstruction *>, 4>
      callDataMap;

  /// Incoming ray payload for current entry function being translated.
  /// Only valid for any-hit/closest-hit ray tracing shaders.
  SpirvInstruction *currentRayPayload;

  /// This is the Patch Constant Function. This function is not explicitly
  /// called from the entry point function.
  FunctionDecl *patchConstFunc;

  /// The <result-id> of the OpString containing the main source file's path.
  SpirvString *mainSourceFile;

  /// ParentMap of the current function.
  std::unique_ptr<ParentMap> parentMap = nullptr;
};

void SpirvEmitter::doDeclStmt(const DeclStmt *declStmt) {
  for (auto *decl : declStmt->decls())
    doDecl(decl);
}

} // end namespace spirv
} // end namespace clang

#endif
