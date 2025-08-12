//===-- TypeProbe.h - Static functions for probing QualType -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//===----------------------------------------------------------------------===//
#ifndef LLVM_CLANG_SPIRV_TYPEPROBE_H
#define LLVM_CLANG_SPIRV_TYPEPROBE_H

#include <string>

#include "dxc/Support/SPIRVOptions.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Type.h"
#include "clang/SPIRV/SpirvType.h"
#include "clang/Sema/Sema.h"

namespace clang {
namespace spirv {

/// Returns a string name for the function if |fn| is not an overloaded
/// operator. Otherwise, returns the name of the operator. If
/// |addClassNameWithOperator| is true, adds the name of RecordType that
/// defines the overloaded operator in front of the operator name.
std::string getFunctionOrOperatorName(const FunctionDecl *fn,
                                      bool addClassNameWithOperator);

/// Returns a string name for the given type.
std::string getAstTypeName(QualType type);

/// Returns true if the given type will be translated into a SPIR-V scalar type.
///
/// This includes normal scalar types, vectors of size 1, and 1x1 matrices.
///
/// If scalarType is not nullptr, writes the scalar type to *scalarType.
bool isScalarType(QualType type, QualType *scalarType = nullptr);

/// Returns true if the given type will be translated into a SPIR-V vector type.
///
/// This includes normal vector types (either ExtVectorType or HLSL vector type)
/// with more than one elements and matrices with exactly one row or one column.
///
/// Writes the element type and count into *elementType and *count respectively
/// if they are not nullptr.
bool isVectorType(QualType type, QualType *elemType = nullptr,
                  uint32_t *elemCount = nullptr);

/// Returns true if the given type will be translated into a SPIR-V scalar type
/// or vector type.
///
/// This includes:
/// scalar types
/// vector types (vec1, vec2, vec3, and vec4)
/// Mx1 matrices (where M can be 1,2,3,4)
/// 1xN matrices (where N can be 1,2,3,4)
///
/// Writes the element type and count into *elementType and *count respectively
/// if they are not nullptr.
bool isScalarOrVectorType(QualType type, QualType *elemType = nullptr,
                          uint32_t *elemCount = nullptr);

/// Returns true if the given type is an array with constant known size.
bool isConstantArrayType(const ASTContext &, QualType);

/// Returns true if the given type is enum type based on AST parse.
bool isEnumType(QualType type);

/// Returns true if the given type is a 1x1 matrix type.
///
/// If elemType is not nullptr, writes the element type to *elemType.
bool is1x1Matrix(QualType type, QualType *elemType = nullptr);

/// Returns true if the given type is a 1xN (N > 1) matrix type.
///
/// If elemType is not nullptr, writes the element type to *elemType.
/// If count is not nullptr, writes the value of N into *count.
bool is1xNMatrix(QualType type, QualType *elemType = nullptr,
                 uint32_t *count = nullptr);

/// Returns true if the given type is a Mx1 (M > 1) matrix type.
///
/// If elemType is not nullptr, writes the element type to *elemType.
/// If count is not nullptr, writes the value of M into *count.
bool isMx1Matrix(QualType type, QualType *elemType = nullptr,
                 uint32_t *count = nullptr);

/// Returns true if the given type is a matrix with more than 1 row and
/// more than 1 column.
///
/// If elemType is not nullptr, writes the element type to *elemType.
/// If rowCount is not nullptr, writes the number of rows (M) into *rowCount.
/// If colCount is not nullptr, writes the number of cols (N) into *colCount.
bool isMxNMatrix(QualType type, QualType *elemType = nullptr,
                 uint32_t *rowCount = nullptr, uint32_t *colCount = nullptr);

/// Returns true if the given type will be translated into a SPIR-V array type.
///
/// Writes the element type and count into *elementType and *count respectively
/// if they are not nullptr.
bool isArrayType(QualType type, QualType *elemType = nullptr,
                 uint32_t *elemCount = nullptr);

/// \brief Returns true if the given type is a ConstantBuffer or an array of
/// ConstantBuffers.
bool isConstantBuffer(QualType);

/// \brief Returns true if the given type is a TextureBuffer or an array of
/// TextureBuffers.
bool isTextureBuffer(QualType);

/// \brief Returns true if the given type is a ConstantBuffer or TextureBuffer
/// or an array of ConstantBuffers/TextureBuffers.
bool isConstantTextureBuffer(QualType);

/// \brief Returns true if the given type will have a SPIR-V resource type.
///
/// Note that this function covers the following HLSL types:
/// * ConstantBuffer/TextureBuffer
/// * Various structured buffers
/// * (RW)ByteAddressBuffer
/// * SubpassInput(MS)
bool isResourceType(QualType);

/// \brief Returns true if the given type is a user-defined struct or class
/// type (not HLSL built-in type).
bool isUserDefinedRecordType(const ASTContext &, QualType);

/// Returns true if the given type is or contains a 16-bit type.
/// The caller must also specify whether 16-bit types have been enabled via
/// command line options.
bool isOrContains16BitType(QualType type, bool enable16BitTypesOption);

/// NOTE: This method doesn't handle Literal types correctly at the moment.
///
/// Note: This method will be deprecated once resolving of literal types are
/// moved to a dedicated pass.
///
/// \brief Returns the realized bitwidth of the given type when represented in
/// SPIR-V. Panics if the given type is not a scalar, a vector/matrix of float
/// or integer, or an array of them. In case of vectors, it returns the
/// realized SPIR-V bitwidth of the vector elements.
uint32_t getElementSpirvBitwidth(const ASTContext &astContext, QualType type,
                                 bool is16BitTypeEnabled);

/// Returns true if the two types can be treated as the same scalar
/// type, which means they have the same canonical type, regardless of
/// constnesss and literalness.
bool canTreatAsSameScalarType(QualType type1, QualType type2);

/// \brief Returns true if the two types are the same scalar or vector type,
/// regardless of constness and literalness.
bool isSameScalarOrVecType(QualType type1, QualType type2);

/// \brief Returns true if the two types are the same type, regardless of
/// constness and literalness.
bool isSameType(const ASTContext &, QualType type1, QualType type2);

/// Returns true if all members in structType are of the same element
/// type and can be fit into a 4-component vector. Writes element type and
/// count to *elemType and *elemCount if not nullptr. Otherwise, emit errors
/// explaining why not.
bool canFitIntoOneRegister(const ASTContext &, QualType structType,
                           QualType *elemType, uint32_t *elemCount = nullptr);

/// Returns the element type of the given type. The given type may be a scalar
/// type, vector type, matrix type, or array type. It may also be a struct with
/// members that can fit into a register. In such case, the result would be the
/// struct member type.
QualType getElementType(const ASTContext &, QualType type);

/// \brief Evluates the given type at the given bitwidth and returns the
/// result-id for it. Panics if the given type is not a scalar or vector of
/// float or integer type. For example: if QualType of an int4 and bitwidth of
/// 64 is passed in, the result-id of a SPIR-V vector of size 4 of signed
/// 64-bit integers is returned.
/// Acceptable bitwidths are 16, 32, and 64.
QualType getTypeWithCustomBitwidth(const ASTContext &, QualType type,
                                   uint32_t bitwidth);

/// Returns true if the given type is a matrix or an array of matrices.
bool isMatrixOrArrayOfMatrix(const ASTContext &, QualType type);

/// Returns true if the given type is a LitInt or LitFloat type or a vector of
/// them. Returns false otherwise.
bool isLitTypeOrVecOfLitType(QualType type);

/// Strips the attributes and typedefs from the given type and returns the
/// desugared one. If isRowMajor is not nullptr, and a 'row_major' or
/// 'column-major' attribute is found during desugaring, this information is
/// written to *isRowMajor.
QualType desugarType(QualType type, llvm::Optional<bool> *isRowMajor);

/// Returns true if type is a SPIR-V row-major matrix or array of matrices.
/// Returns false if type is a SPIR-V col-major matrix or array of matrices.
/// It does so by checking the majorness of the HLSL matrix either with
/// explicit attribute or implicit command-line option.
///
/// Note that HLSL matrices are conceptually row major, while SPIR-V matrices
/// are conceptually column major. We are mapping what HLSL semantically mean
/// a row into a column here.
bool isRowMajorMatrix(const SpirvCodeGenOptions &, QualType type);

/// \brief Returns true if the given type is a (RW)StructuredBuffer type.
bool isStructuredBuffer(QualType type);

/// \brief Returns true if the given type is a non-writable StructuredBuffer
/// type.
bool isNonWritableStructuredBuffer(QualType type);

/// \brief Returns true if the given type is an AppendStructuredBuffer type.
bool isAppendStructuredBuffer(QualType type);

/// \brief Returns true if the given type is a ConsumeStructuredBuffer type.
bool isConsumeStructuredBuffer(QualType type);

/// \brief Returns true if the given type is a RWStructuredBuffer type.
bool isRWStructuredBuffer(QualType type);

/// \brief Returns true if the given type is a RW/Append/Consume
/// StructuredBuffer type.
bool isRWAppendConsumeSBuffer(QualType type);

/// \brief Returns true if the given type is a ResourceDescriptorHeap.
bool isResourceDescriptorHeap(QualType type);

/// \brief Returns true if the given type is a SamplerDescriptorHeap.
bool isSamplerDescriptorHeap(QualType type);

/// \brief Returns true if the given type is the HLSL ByteAddressBufferType.
bool isByteAddressBuffer(QualType type);

/// \brief Returns true if the given type is the HLSL RWByteAddressBufferType.
bool isRWByteAddressBuffer(QualType type);

/// \brief Returns true if the given type is the HLSL (RW)StructuredBuffer,
/// (RW)ByteAddressBuffer, or {Append|Consume}StructuredBuffer.
bool isAKindOfStructuredOrByteBuffer(QualType type);

/// \brief Returns true if the given type is the HLSL (RW)StructuredBuffer,
/// (RW)ByteAddressBuffer, {Append|Consume}StructuredBuffer, or a struct
/// containing one of the above.
bool isOrContainsAKindOfStructuredOrByteBuffer(QualType type);

/// \brief Returns true if the given type is the HLSL Buffer type.
bool isBuffer(QualType type);

/// \brief Returns true if the given type is the HLSL RWBuffer type.
bool isRWBuffer(QualType type);

/// \brief Returns true if the given type is an HLSL Texture type.
bool isTexture(QualType);

/// \brief Returns true if the given type is an HLSL Texture2DMS or
/// Texture2DMSArray type.
bool isTextureMS(QualType);

/// \brief Returns true if the given type is an HLSL RWTexture type.
bool isRWTexture(QualType);

/// \brief Returns true if the given type is an HLSL sampler type.
bool isSampler(QualType);

/// \brief Returns true if the given type is InputPatch.
bool isInputPatch(QualType type);

/// \brief Returns true if the given type is OutputPatch.
bool isOutputPatch(QualType type);

/// \brief Returns true if the given type is SubpassInput.
bool isSubpassInput(QualType);

/// \brief Returns true if the given type is SubpassInputMS.
bool isSubpassInputMS(QualType);

/// \brief If the given QualType is an HLSL resource type (or array of
/// resources), returns its HLSL type name. e.g. "RWTexture2D". Returns an empty
/// string otherwise.
std::string getHlslResourceTypeName(QualType);

/// Returns true if the given type will be translated into a SPIR-V image,
/// sampler or struct containing images or samplers.
///
/// Note: legalization specific code
bool isOpaqueType(QualType type);

/// Returns true if the given type will be translated into a array of SPIR-V
/// images or samplers.
bool isOpaqueArrayType(QualType type);

/// Returns true if the given type is a struct type who has an opaque field
/// (in a recursive away).
///
/// Note: legalization specific code
bool isOpaqueStructType(QualType type);

/// \brief Returns true if the given type can use relaxed precision
/// decoration. Integer and float types with lower than 32 bits can be
/// operated on with a relaxed precision.
bool isRelaxedPrecisionType(QualType, const SpirvCodeGenOptions &);

/// Returns true if the given type is a rasterizer ordered view.
bool isRasterizerOrderedView(QualType type);

/// Returns true if the given type is a bool or vector of bool type.
bool isBoolOrVecOfBoolType(QualType type);

/// Returns true if the given type is a signed integer or vector of signed
/// integer type.
bool isSintOrVecOfSintType(QualType type);

/// Returns true if the given type is an unsigned integer or vector of unsigned
/// integer type.
bool isUintOrVecOfUintType(QualType type);

/// Returns true if the given type is a float or vector of float type.
bool isFloatOrVecOfFloatType(QualType type);

/// Returns true if the given type is a bool or vector/matrix of bool type.
bool isBoolOrVecMatOfBoolType(QualType type);

/// Returns true if the given type is a signed integer or vector/matrix of
/// signed integer type.
bool isSintOrVecMatOfSintType(QualType type);

/// Returns true if the given type is an unsigned integer or vector/matrix of
/// unsigned integer type.
bool isUintOrVecMatOfUintType(QualType type);

/// Returns true if the given type is a float or vector/matrix of float type.
bool isFloatOrVecMatOfFloatType(QualType type);

/// \brief Returns true if the decl type is a non-floating-point matrix and
/// the matrix is column major, or if it is an array/struct containing such
/// matrices.
bool isOrContainsNonFpColMajorMatrix(const ASTContext &,
                                     const SpirvCodeGenOptions &, QualType type,
                                     const Decl *decl);

/// brief Returns true if the type is a boolean type or an aggragate type that
/// contains a boolean type.
bool isOrContainsBoolType(QualType type);

/// \brief Returns true if the given type is `vk::ext_result_id<T>`.
bool isExtResultIdType(QualType type);

/// \brief Returns true if the given type is defined in `vk` namespace.
bool isTypeInVkNamespace(const RecordType *type);

/// \brief Returns true if the given type is a String or StringLiteral type.
bool isStringType(QualType);

/// \brief Returns true if the given type is a bindless array of an opaque type.
bool isBindlessOpaqueArray(QualType type);

/// \brief Generates the corresponding SPIR-V vector type for the given Clang
/// frontend matrix type's vector component and returns the <result-id>.
///
/// This method will panic if the given matrix type is not a SPIR-V acceptable
/// matrix type.
QualType getComponentVectorType(const ASTContext &, QualType matrixType);

/// \brief Returns a QualType corresponding to HLSL matrix of given element type
/// and rows/columns.
QualType getHLSLMatrixType(ASTContext &, Sema &, ClassTemplateDecl *,
                           QualType elemType, int rows, int columns);

/// Returns true if the given type is a structure or array of structures for
/// which flattening all of its members recursively results in resources ONLY.
bool isResourceOnlyStructure(QualType type);

/// Returns true if the given type is a structure or array of structures for
/// which flattening all of its members recursively results in at least one
/// resoure variable.
bool isStructureContainingResources(QualType type);

/// Returns true if the given type is a structure or array of structures for
/// which flattening all of its members recursively results in at least one
/// non-resoure variable.
bool isStructureContainingNonResources(QualType type);

/// Returns true if the given type is a structure or array of structures for
/// which flattening all of its members recursively results in a mix of resource
/// variables and non-resource variables.
bool isStructureContainingMixOfResourcesAndNonResources(QualType type);

/// Returns true if the given type is a structure or array of structures for
/// which flattening all of its members recursively results in at least one kind
/// of buffer: cbuffer, tbuffer, (RW)ByteAddressBuffer, or
/// (RW|Append|Consume)StructuredBuffer.
bool isStructureContainingAnyKindOfBuffer(QualType type);

/// Returns true if the given type is a scalar, vector, or matrix of numeric
/// types, or it's an array of scalar, vector, or matrix of numeric types.
bool isScalarOrNonStructAggregateOfNumericalTypes(QualType type);

/// Calls `operation` on for each field in the base and derives class defined by
/// `recordType`. The `operation` will receive the AST type linked to the field,
/// the SPIRV type linked to the field, and the index of the field in the final
/// SPIR-V representation. This index of the field can vary from the AST
/// field-index because bitfields are merged into a single field in the SPIR-V
/// representation.
///
/// If `includeMerged` is true, `operation` will be called on the same spir-v
/// field for each field it represents. For example, if a spir-v field holds the
/// values for 3 bit-fields, `operation` will be called 3 times with the same
/// `spirvFieldIndex`. The `bitfield` information in `field` will be different.
///
/// If false, `operation` will be called once on the first field in the merged
/// field.
///
/// If the operation returns false, we stop processing fields.
void forEachSpirvField(
    const RecordType *recordType, const StructType *spirvType,
    std::function<bool(size_t spirvFieldIndex, const QualType &fieldType,
                       const StructType::FieldInfo &field)>
        operation,
    bool includeMerged = false);
} // namespace spirv
} // namespace clang

#endif // LLVM_CLANG_SPIRV_TYPEPROBE_H
