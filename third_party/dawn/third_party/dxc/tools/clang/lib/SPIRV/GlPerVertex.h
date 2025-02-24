//===--- GlPerVertex.h - For handling gl_PerVertex members -------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_SPIRV_GLPERVERTEX_H
#define LLVM_CLANG_LIB_SPIRV_GLPERVERTEX_H

#include "dxc/DXIL/DxilSemantic.h"
#include "dxc/DXIL/DxilSigPoint.h"
#include "clang/SPIRV/SpirvBuilder.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallVector.h"

namespace clang {
namespace spirv {

/// The class for handling ClipDistance and CullDistance builtin variables that
/// belong to gl_PerVertex.
///
/// Reading/writing of the ClipDistance/CullDistance builtin is not as
/// straightforward as other builtins. This is because in HLSL, we can have
/// multiple entities annotated with SV_ClipDistance/SV_CullDistance and they
/// can be float or vector of float type. For example,
///
///   float2 var1 : SV_ClipDistance2,
///   float  var2 : SV_ClipDistance0,
///   float3 var3 : SV_ClipDistance1,
///
/// But in Vulkan, ClipDistance/CullDistance is required to be a float array.
/// So we need to combine these variables into one single float array. The way
/// we do it is by sorting all entities according to the SV_ClipDistance/
/// SV_CullDistance index, and concatenating them tightly. So for the above,
/// var2 will take the first two floats in the array, var3 will take the next
/// three, and var1 will take the next two. In total, we have an size-6 float
/// array for ClipDistance builtin.
class GlPerVertex {
public:
  GlPerVertex(ASTContext &context, SpirvContext &spvContext,
              SpirvBuilder &spvBuilder);

  /// Records a declaration of SV_ClipDistance/SV_CullDistance so later
  /// we can caculate the ClipDistance/CullDistance array layout.
  /// Also records the semantic strings provided for them.
  bool recordGlPerVertexDeclFacts(const DeclaratorDecl *decl, bool asInput);

  /// Calculates the layout for ClipDistance/CullDistance arrays.
  void calculateClipCullDistanceArraySize();

  /// Emits SPIR-V code for the input and/or ouput ClipDistance/CullDistance
  /// builtin variables. If inputArrayLength is not zero, the input variable
  /// will have an additional arrayness of the given size. Similarly for
  /// outputArrayLength.
  ///
  /// Note that this method should be called after recordClipCullDistanceDecl()
  /// and calculateClipCullDistanceArraySize().
  void generateVars(uint32_t inputArrayLength, uint32_t outputArrayLength);

  /// Returns the stage input variables.
  llvm::SmallVector<SpirvVariable *, 2> getStageInVars() const;
  /// Returns the stage output variables.
  llvm::SmallVector<SpirvVariable *, 2> getStageOutVars() const;

  /// Tries to access the builtin translated from the given HLSL semantic of the
  /// given index.
  ///
  /// If sigPoint indicates this is input, builtins will be read to compose a
  /// new temporary value of the correct type and writes to *value. Otherwise,
  /// the *value will be decomposed and writes to the builtins, unless
  /// noWriteBack is true, which means do not write back the value.
  ///
  /// If invocation (should only be used for HS) is not llvm::None, only
  /// accesses the element at the invocation offset in the gl_PerVeterx array.
  ///
  /// Creates SPIR-V instructions and returns true if we are accessing builtins
  /// that are ClipDistance or CullDistance. Does nothing and returns true if
  /// accessing builtins for others. Returns false if errors occurs.
  bool tryToAccess(hlsl::SigPoint::Kind sigPoint, hlsl::Semantic::Kind,
                   uint32_t semanticIndex,
                   llvm::Optional<SpirvInstruction *> invocation,
                   SpirvInstruction **value, bool noWriteBack,
                   SpirvInstruction *vecComponent, SourceLocation loc,
                   SourceRange range = {});

private:
  using SemanticIndexToTypeMap = llvm::DenseMap<uint32_t, QualType>;
  using SemanticIndexToArrayOffsetMap = llvm::DenseMap<uint32_t, uint32_t>;

  template <unsigned N>
  DiagnosticBuilder emitError(const char (&message)[N], SourceLocation loc) {
    const auto diagId = astContext.getDiagnostics().getCustomDiagID(
        clang::DiagnosticsEngine::Error, message);
    return astContext.getDiagnostics().Report(loc, diagId);
  }

  /// Creates a stand-alone ClipDistance/CullDistance builtin variable.
  SpirvVariable *createClipCullDistanceVar(bool asInput, bool isClip,
                                           uint32_t arraySize, bool isPrecise);

  /// Creates SPIR-V instructions for reading the data starting from offset in
  /// the ClipDistance/CullDistance builtin. The data read will be transformed
  /// into the given type asType.
  SpirvInstruction *readClipCullArrayAsType(bool isClip, uint32_t offset,
                                            QualType asType, SourceLocation loc,
                                            SourceRange range = {}) const;
  /// Creates SPIR-V instructions to read a field in gl_PerVertex.
  bool readField(hlsl::Semantic::Kind semanticKind, uint32_t semanticIndex,
                 SpirvInstruction **value, SourceLocation loc,
                 SourceRange range = {});

  /// Creates SPIR-V instructions for writing data into the ClipDistance/
  /// CullDistance builtin starting from offset. The value to be written is
  /// fromValue, whose type is fromType. Necessary transformations will be
  /// generated to make sure type correctness.
  void
  writeClipCullArrayFromType(llvm::Optional<SpirvInstruction *> invocationId,
                             bool isClip, SpirvInstruction *offset,
                             QualType fromType, SpirvInstruction *fromValue,
                             SourceLocation loc, SourceRange range = {}) const;
  /// Creates SPIR-V instructions to write a field in gl_PerVertex.
  bool writeField(hlsl::Semantic::Kind semanticKind, uint32_t semanticIndex,
                  llvm::Optional<SpirvInstruction *> invocationId,
                  SpirvInstruction **value, SpirvInstruction *vecComponent,
                  SourceLocation loc, SourceRange range = {});

  /// Internal implementation for recordClipCullDistanceDecl().
  bool doGlPerVertexFacts(const NamedDecl *decl, QualType type, bool asInput);

  /// Returns whether the type is a scalar, vector, or array that contains
  /// only scalars with float type.
  bool containOnlyFloatType(QualType type) const;

  /// Returns the number of all scalar components recursively included in a
  /// scalar, vector, or array type. If type is not a scalar, vector, or array,
  /// returns 0.
  uint32_t getNumberOfScalarComponentsInScalarVectorArray(QualType type) const;

  /// Creates load instruction for clip or cull distance with a scalar type.
  SpirvInstruction *createScalarClipCullDistanceLoad(
      SpirvInstruction *ptr, QualType asType, uint32_t offset,
      SourceLocation loc,
      llvm::Optional<uint32_t> arrayIndex = llvm::None) const;
  /// Creates load instruction for clip or cull distance with a scalar or vector
  /// type.
  SpirvInstruction *createScalarOrVectorClipCullDistanceLoad(
      SpirvInstruction *ptr, QualType asType, uint32_t offset,
      SourceLocation loc,
      llvm::Optional<uint32_t> arrayIndex = llvm::None) const;
  /// Creates load instruction for clip or cull distance with a scalar or vector
  /// or array type of them.
  SpirvInstruction *createClipCullDistanceLoad(
      SpirvInstruction *ptr, QualType asType, uint32_t offset,
      SourceLocation loc,
      llvm::Optional<uint32_t> arrayIndex = llvm::None) const;

  /// Creates store instruction for clip or cull distance with a scalar type.
  bool createScalarClipCullDistanceStore(
      SpirvInstruction *ptr, SpirvInstruction *value, QualType valueType,
      SpirvInstruction *offset, SourceLocation loc,
      llvm::ArrayRef<uint32_t> valueIndices,
      llvm::Optional<SpirvInstruction *> arrayIndex = llvm::None) const;
  /// Creates store instruction for clip or cull distance with a scalar or
  /// vector type.
  bool createScalarOrVectorClipCullDistanceStore(
      SpirvInstruction *ptr, SpirvInstruction *value, QualType valueType,
      SpirvInstruction *offset, SourceLocation loc,
      llvm::Optional<uint32_t> valueOffset,
      llvm::Optional<SpirvInstruction *> arrayIndex = llvm::None) const;
  /// Creates store instruction for clip or cull distance with a scalar or
  /// vector or array type of them.
  bool createClipCullDistanceStore(
      SpirvInstruction *ptr, SpirvInstruction *value, QualType valueType,
      SpirvInstruction *offset, SourceLocation loc,
      llvm::Optional<SpirvInstruction *> arrayIndex = llvm::None) const;

  /// Keeps the mapping semanticIndex to clipCullDistanceType in typeMap and
  /// returns true if clipCullDistanceType is a valid type for clip/cull
  /// distance. Otherwise, returns false.
  bool setClipCullDistanceType(SemanticIndexToTypeMap *typeMap,
                               uint32_t semanticIndex,
                               QualType clipCullDistanceType) const;

private:
  ASTContext &astContext;
  SpirvContext &spvContext;
  SpirvBuilder &spvBuilder;

  /// Input/output ClipDistance/CullDistance variable.
  SpirvVariable *inClipVar, *inCullVar;
  SpirvVariable *outClipVar, *outCullVar;

  // We need to record whether the variables with 'SV_ClipDistance' or
  // 'SV_CullDistance' have the HLSL 'precise' keyword.
  bool inClipPrecise, outClipPrecise;
  bool inCullPrecise, outCullPrecise;

  /// The array size for the input/output gl_PerVertex block member variables.
  /// HS input and output, DS input, GS input has an additional level of
  /// arrayness. The array size is stored in this variable. Zero means
  /// the corresponding variable does not need extra arrayness.
  uint32_t inArraySize, outArraySize;
  /// The array size of input/output ClipDistance/CullDistance float arrays.
  /// This is not the array size of the whole gl_PerVertex struct.
  uint32_t inClipArraySize, outClipArraySize;
  uint32_t inCullArraySize, outCullArraySize;

  /// We need to record all SV_ClipDistance/SV_CullDistance decls' types
  /// since we need to generate the necessary conversion instructions when
  /// accessing the ClipDistance/CullDistance builtins.
  SemanticIndexToTypeMap inClipType, outClipType;
  SemanticIndexToTypeMap inCullType, outCullType;
  /// We also need to keep track of all SV_ClipDistance/SV_CullDistance decls'
  /// offsets in the float array.
  SemanticIndexToArrayOffsetMap inClipOffset, outClipOffset;
  SemanticIndexToArrayOffsetMap inCullOffset, outCullOffset;

  enum { kSemanticStrCount = 2 };

  /// Keeps track of the semantic strings provided in the source code for the
  /// builtins in gl_PerVertex.
  llvm::SmallVector<std::string, kSemanticStrCount> inSemanticStrs;
  llvm::SmallVector<std::string, kSemanticStrCount> outSemanticStrs;
};

} // end namespace spirv
} // end namespace clang

#endif
