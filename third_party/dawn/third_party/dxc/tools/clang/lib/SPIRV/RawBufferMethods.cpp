//===---- RawBufferMethods.cpp ---- Raw Buffer Methods ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//===----------------------------------------------------------------------===//

#include "RawBufferMethods.h"
#include "AlignmentSizeCalculator.h"
#include "LowerTypeVisitor.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/CharUnits.h"
#include "clang/AST/RecordLayout.h"
#include "clang/AST/Type.h"
#include "clang/SPIRV/AstTypeProbe.h"
#include "clang/SPIRV/SpirvBuilder.h"
#include "clang/SPIRV/SpirvInstruction.h"
#include <cstdint>

namespace {
/// Rounds the given value up to the given power of 2.
inline uint32_t roundToPow2(uint32_t val, uint32_t pow2) {
  assert(pow2 != 0);
  return (val + pow2 - 1) & ~(pow2 - 1);
}
} // anonymous namespace

namespace clang {
namespace spirv {

SpirvInstruction *RawBufferHandler::bitCastToNumericalOrBool(
    SpirvInstruction *instr, QualType fromType, QualType toType,
    SourceLocation loc, SourceRange range) {
  if (isSameType(astContext, fromType, toType))
    return instr;

  if (toType->isBooleanType() || fromType->isBooleanType())
    return theEmitter.castToType(instr, fromType, toType, loc, range);

  // Perform a bitcast
  return spvBuilder.createUnaryOp(spv::Op::OpBitcast, toType, instr, loc,
                                  range);
}

SpirvInstruction *RawBufferHandler::load16Bits(SpirvInstruction *buffer,
                                               BufferAddress &address,
                                               QualType target16BitType,
                                               SourceRange range) {
  const auto loc = buffer->getSourceLocation();
  SpirvInstruction *result = nullptr;
  auto *constUint0 =
      spvBuilder.getConstantInt(astContext.UnsignedIntTy, llvm::APInt(32, 0));
  auto *constUint3 =
      spvBuilder.getConstantInt(astContext.UnsignedIntTy, llvm::APInt(32, 3));
  auto *constUint4 =
      spvBuilder.getConstantInt(astContext.UnsignedIntTy, llvm::APInt(32, 4));

  auto *index = address.getWordIndex(loc, range);

  // Take the remainder and multiply by 8 to get the bit offset within the word.
  auto *bitOffset = spvBuilder.createBinaryOp(
      spv::Op::OpUMod, astContext.UnsignedIntTy, address.getByteAddress(),
      constUint4, loc, range);
  bitOffset = spvBuilder.createBinaryOp(spv::Op::OpShiftLeftLogical,
                                        astContext.UnsignedIntTy, bitOffset,
                                        constUint3, loc, range);

  // The underlying element type of the ByteAddressBuffer is uint. So we
  // need to load 32-bits at the very least.
  auto *ptr = spvBuilder.createAccessChain(astContext.UnsignedIntTy, buffer,
                                           {constUint0, index}, loc, range);
  result = spvBuilder.createLoad(astContext.UnsignedIntTy, ptr, loc, range);
  result = spvBuilder.createBinaryOp(spv::Op::OpShiftRightLogical,
                                     astContext.UnsignedIntTy, result,
                                     bitOffset, loc, range);
  result = spvBuilder.createUnaryOp(
      spv::Op::OpUConvert, astContext.UnsignedShortTy, result, loc, range);
  result = bitCastToNumericalOrBool(result, astContext.UnsignedShortTy,
                                    target16BitType, loc, range);
  result->setRValue();

  address.incrementByteAddress(2, loc, range);
  return result;
}

SpirvInstruction *RawBufferHandler::load32Bits(SpirvInstruction *buffer,
                                               BufferAddress &address,
                                               QualType target32BitType,
                                               SourceRange range) {
  const auto loc = buffer->getSourceLocation();
  SpirvInstruction *result = nullptr;
  // Only need to perform one 32-bit uint load.
  auto *constUint0 =
      spvBuilder.getConstantInt(astContext.UnsignedIntTy, llvm::APInt(32, 0));

  auto *index = address.getWordIndex(loc, range);

  auto *loadPtr = spvBuilder.createAccessChain(astContext.UnsignedIntTy, buffer,
                                               {constUint0, index}, loc, range);
  result = spvBuilder.createLoad(astContext.UnsignedIntTy, loadPtr, loc, range);
  result = bitCastToNumericalOrBool(result, astContext.UnsignedIntTy,
                                    target32BitType, loc, range);
  result->setRValue();

  address.incrementWordIndex(loc, range);

  return result;
}

SpirvInstruction *RawBufferHandler::load64Bits(SpirvInstruction *buffer,
                                               BufferAddress &address,
                                               QualType target64BitType,
                                               SourceRange range) {
  const auto loc = buffer->getSourceLocation();
  SpirvInstruction *result = nullptr;
  SpirvInstruction *ptr = nullptr;
  auto *constUint0 =
      spvBuilder.getConstantInt(astContext.UnsignedIntTy, llvm::APInt(32, 0));

  // Load the first word and increment index.
  auto *index = address.getWordIndex(loc, range);
  ptr = spvBuilder.createAccessChain(astContext.UnsignedIntTy, buffer,
                                     {constUint0, index}, loc, range);
  SpirvInstruction *word0 =
      spvBuilder.createLoad(astContext.UnsignedIntTy, ptr, loc, range);
  address.incrementWordIndex(loc, range);

  // Load the second word and increment index.
  index = address.getWordIndex(loc, range);
  ptr = spvBuilder.createAccessChain(astContext.UnsignedIntTy, buffer,
                                     {constUint0, index}, loc, range);
  SpirvInstruction *word1 =
      spvBuilder.createLoad(astContext.UnsignedIntTy, ptr, loc, range);
  address.incrementWordIndex(loc, range);

  // Combine the 2 words into a composite, and bitcast into the destination
  // type.
  const auto uintVec2Type =
      astContext.getExtVectorType(astContext.UnsignedIntTy, 2);
  auto *operand = spvBuilder.createCompositeConstruct(
      uintVec2Type, {word0, word1}, loc, range);
  result = spvBuilder.createUnaryOp(spv::Op::OpBitcast, target64BitType,
                                    operand, loc, range);
  result->setRValue();
  return result;
}

SpirvInstruction *RawBufferHandler::processTemplatedLoadFromBuffer(
    SpirvInstruction *buffer, BufferAddress &address, const QualType targetType,
    SourceRange range) {
  const auto loc = buffer->getSourceLocation();
  SpirvInstruction *result = nullptr;

  // Scalar types
  if (isScalarType(targetType)) {
    SpirvInstruction *scalarResult = nullptr;
    auto loadWidth = getElementSpirvBitwidth(
        astContext, targetType, theEmitter.getSpirvOptions().enable16BitTypes);
    switch (loadWidth) {
    case 16:
      scalarResult = load16Bits(buffer, address, targetType, range);
      break;
    case 32:
      scalarResult = load32Bits(buffer, address, targetType, range);
      break;
    case 64:
      scalarResult = load64Bits(buffer, address, targetType, range);
      break;
    default:
      theEmitter.emitError(
          "templated load of ByteAddressBuffer is only implemented for "
          "16, 32, and 64-bit types",
          loc);
      return nullptr;
    }
    assert(scalarResult != nullptr);
    // We set the layout rule for scalars. Other types are built up from the
    // scalars, and should inherit this layout rule or default to Void.
    scalarResult->setLayoutRule(SpirvLayoutRule::Void);
    return scalarResult;
  }

  // Vector types
  {
    QualType elemType = {};
    uint32_t elemCount = 0;
    if (isVectorType(targetType, &elemType, &elemCount)) {
      llvm::SmallVector<SpirvInstruction *, 4> loadedElems;
      for (uint32_t i = 0; i < elemCount; ++i) {
        loadedElems.push_back(
            processTemplatedLoadFromBuffer(buffer, address, elemType, range));
      }
      result = spvBuilder.createCompositeConstruct(targetType, loadedElems, loc,
                                                   range);
      result->setRValue();
      return result;
    }
  }

  // Array types
  {
    QualType elemType = {};
    uint32_t elemCount = 0;
    if (const auto *arrType = astContext.getAsConstantArrayType(targetType)) {
      elemCount = static_cast<uint32_t>(arrType->getSize().getZExtValue());
      elemType = arrType->getElementType();
      llvm::SmallVector<SpirvInstruction *, 4> loadedElems;
      for (uint32_t i = 0; i < elemCount; ++i) {
        loadedElems.push_back(
            processTemplatedLoadFromBuffer(buffer, address, elemType, range));
      }
      result = spvBuilder.createCompositeConstruct(targetType, loadedElems, loc,
                                                   range);
      result->setRValue();
      return result;
    }
  }

  // Matrix types
  {
    QualType elemType = {};
    uint32_t numRows = 0;
    uint32_t numCols = 0;
    if (isMxNMatrix(targetType, &elemType, &numRows, &numCols)) {
      // In DX, the default matrix orientation in ByteAddressBuffer is column
      // major. If HLSL/DXIL support the `column_major` and `row_major`
      // attributes in the future, we will have to check for them here and
      // override the behavior.
      //
      // The assume buffer matrix order is controlled by the
      // `-fspv-use-legacy-buffer-matrix-order` flag:
      //   (a) false --> assume the matrix is stored column major
      //   (b) true  --> assume the matrix is stored row major
      //
      // We provide (b) for compatibility with legacy shaders that depend on
      // the previous, incorrect, raw buffer matrix order assumed by the SPIR-V
      // codegen.
      const bool isBufferColumnMajor =
          !theEmitter.getSpirvOptions().useLegacyBufferMatrixOrder;
      const uint32_t numElements = numRows * numCols;
      llvm::SmallVector<SpirvInstruction *, 16> loadedElems(numElements);
      for (uint32_t i = 0; i != numElements; ++i)
        loadedElems[i] =
            processTemplatedLoadFromBuffer(buffer, address, elemType, range);

      llvm::SmallVector<SpirvInstruction *, 4> loadedRows;
      for (uint32_t i = 0; i < numRows; ++i) {
        llvm::SmallVector<SpirvInstruction *, 4> loadedColumn;
        for (uint32_t j = 0; j < numCols; ++j) {
          const uint32_t elementIndex =
              isBufferColumnMajor ? (j * numRows + i) : (i * numCols + j);
          loadedColumn.push_back(loadedElems[elementIndex]);
        }
        const auto rowType = astContext.getExtVectorType(elemType, numCols);
        loadedRows.push_back(spvBuilder.createCompositeConstruct(
            rowType, loadedColumn, loc, range));
      }

      result = spvBuilder.createCompositeConstruct(targetType, loadedRows, loc,
                                                   range);
      result->setRValue();
      return result;
    }
  }

  // Struct types
  // The "natural" layout for structure types dictates that structs are
  // aligned like their field with the largest alignment.
  // As a result, there might exist some padding after some struct members.
  if (const auto *structType = targetType->getAs<RecordType>()) {
    LowerTypeVisitor lowerTypeVisitor(astContext, theEmitter.getSpirvContext(),
                                      theEmitter.getSpirvOptions(), spvBuilder);
    auto *decl = targetType->getAsTagDecl();
    assert(decl && "Expected all structs to be tag decls.");
    const StructType *spvType = dyn_cast<StructType>(lowerTypeVisitor.lowerType(
        targetType, theEmitter.getSpirvOptions().sBufferLayoutRule, llvm::None,
        decl->getLocation()));
    llvm::SmallVector<SpirvInstruction *, 4> loadedElems;
    forEachSpirvField(
        structType, spvType,
        [this, &buffer, &address, range,
         &loadedElems](size_t spirvFieldIndex, const QualType &fieldType,
                       const auto &field) {
          auto *baseOffset = address.getByteAddress();
          if (field.offset.hasValue() && field.offset.getValue() != 0) {
            const auto loc = buffer->getSourceLocation();
            SpirvConstant *offset = spvBuilder.getConstantInt(
                astContext.UnsignedIntTy,
                llvm::APInt(32, field.offset.getValue()));
            baseOffset = spvBuilder.createBinaryOp(
                spv::Op::OpIAdd, astContext.UnsignedIntTy, baseOffset, offset,
                loc, range);
          }

          loadedElems.push_back(processTemplatedLoadFromBuffer(
              buffer, baseOffset, fieldType, range));
          return true;
        });

    // After we're done with loading the entire struct, we need to update the
    // byteAddress (in case we are loading an array of structs).
    //
    // struct size = 34 bytes (34 / 8) = 4 full words (34 % 8) = 2 > 0,
    // therefore need to move to the next aligned address So the starting byte
    // offset after loading the entire struct is: 8 * (4 + 1) = 40
    uint32_t structAlignment = 0, structSize = 0, stride = 0;
    std::tie(structAlignment, structSize) =
        AlignmentSizeCalculator(astContext, theEmitter.getSpirvOptions())
            .getAlignmentAndSize(targetType,
                                 theEmitter.getSpirvOptions().sBufferLayoutRule,
                                 llvm::None, &stride);

    assert(structAlignment != 0);
    SpirvInstruction *structWidth = spvBuilder.getConstantInt(
        astContext.UnsignedIntTy,
        llvm::APInt(32, roundToPow2(structSize, structAlignment)));
    address.incrementByteAddress(structWidth, loc, range);

    result = spvBuilder.createCompositeConstruct(targetType, loadedElems, loc,
                                                 range);
    result->setRValue();
    return result;
  }

  llvm_unreachable("templated buffer load unimplemented for type");
}

SpirvInstruction *RawBufferHandler::processTemplatedLoadFromBuffer(
    SpirvInstruction *buffer, SpirvInstruction *byteAddress,
    const QualType targetType, SourceRange range) {
  BufferAddress address(byteAddress, theEmitter);

  return processTemplatedLoadFromBuffer(buffer, address, targetType, range);
}

void RawBufferHandler::store16Bits(SpirvInstruction *value,
                                   SpirvInstruction *buffer,
                                   BufferAddress &address,
                                   const QualType valueType,
                                   SourceRange range) {
  const auto loc = buffer->getSourceLocation();
  SpirvInstruction *result = nullptr;
  auto *constUint0 =
      spvBuilder.getConstantInt(astContext.UnsignedIntTy, llvm::APInt(32, 0));
  auto *constUint3 =
      spvBuilder.getConstantInt(astContext.UnsignedIntTy, llvm::APInt(32, 3));
  auto *constUint4 =
      spvBuilder.getConstantInt(astContext.UnsignedIntTy, llvm::APInt(32, 4));
  auto *constUint16 =
      spvBuilder.getConstantInt(astContext.UnsignedIntTy, llvm::APInt(32, 16));
  auto *constUintFFFF = spvBuilder.getConstantInt(astContext.UnsignedIntTy,
                                                  llvm::APInt(32, 0xffff));

  auto *index = address.getWordIndex(loc, range);

  // Take the remainder and multiply by 8 to get the bit offset within the word.
  auto *bitOffset = spvBuilder.createBinaryOp(
      spv::Op::OpUMod, astContext.UnsignedIntTy, address.getByteAddress(),
      constUint4, loc, range);
  bitOffset = spvBuilder.createBinaryOp(spv::Op::OpShiftLeftLogical,
                                        astContext.UnsignedIntTy, bitOffset,
                                        constUint3, loc, range);

  // The underlying element type of the ByteAddressBuffer is uint. So we
  // need to store a 32-bit value.
  auto *ptr = spvBuilder.createAccessChain(astContext.UnsignedIntTy, buffer,
                                           {constUint0, index}, loc, range);

  result = bitCastToNumericalOrBool(value, valueType,
                                    astContext.UnsignedShortTy, loc, range);
  result = spvBuilder.createUnaryOp(
      spv::Op::OpUConvert, astContext.UnsignedIntTy, result, loc, range);
  result = spvBuilder.createBinaryOp(spv::Op::OpShiftLeftLogical,
                                     astContext.UnsignedIntTy, result,
                                     bitOffset, loc, range);

  auto *maskOffset =
      spvBuilder.createBinaryOp(spv::Op::OpISub, astContext.UnsignedIntTy,
                                constUint16, bitOffset, loc, range);

  auto *mask = spvBuilder.createBinaryOp(spv::Op::OpShiftLeftLogical,
                                         astContext.UnsignedIntTy,
                                         constUintFFFF, maskOffset, loc, range);

  // Load and mask the other value in the word.
  auto *masked = spvBuilder.createBinaryOp(
      spv::Op::OpBitwiseAnd, astContext.UnsignedIntTy,
      spvBuilder.createLoad(astContext.UnsignedIntTy, ptr, loc), mask, loc,
      range);

  result =
      spvBuilder.createBinaryOp(spv::Op::OpBitwiseOr, astContext.UnsignedIntTy,
                                masked, result, loc, range);
  spvBuilder.createStore(ptr, result, loc, range);
  address.incrementByteAddress(2, loc, range);
}

void RawBufferHandler::store32Bits(SpirvInstruction *value,
                                   SpirvInstruction *buffer,
                                   BufferAddress &address,
                                   const QualType valueType,
                                   SourceRange range) {
  const auto loc = buffer->getSourceLocation();
  auto *constUint0 =
      spvBuilder.getConstantInt(astContext.UnsignedIntTy, llvm::APInt(32, 0));

  auto *index = address.getWordIndex(loc, range);

  // The underlying element type of the ByteAddressBuffer is uint. So we
  // need to store a 32-bit value.
  auto *ptr = spvBuilder.createAccessChain(astContext.UnsignedIntTy, buffer,
                                           {constUint0, index}, loc, range);
  value = bitCastToNumericalOrBool(value, valueType, astContext.UnsignedIntTy,
                                   loc, range);
  spvBuilder.createStore(ptr, value, loc, range);
  address.incrementWordIndex(loc, range);
}

void RawBufferHandler::store64Bits(SpirvInstruction *value,
                                   SpirvInstruction *buffer,
                                   BufferAddress &address,
                                   const QualType valueType,
                                   SourceRange range) {
  const auto loc = buffer->getSourceLocation();
  auto *constUint0 =
      spvBuilder.getConstantInt(astContext.UnsignedIntTy, llvm::APInt(32, 0));

  // Bitcast the source into a 32-bit words composite.
  const auto uintVec2Type =
      astContext.getExtVectorType(astContext.UnsignedIntTy, 2);
  auto *tmp = spvBuilder.createUnaryOp(spv::Op::OpBitcast, uintVec2Type, value,
                                       loc, range);

  // Extract the low and high word (careful! word order).
  auto *A = spvBuilder.createCompositeExtract(astContext.UnsignedIntTy, tmp,
                                              {0}, loc, range);
  auto *B = spvBuilder.createCompositeExtract(astContext.UnsignedIntTy, tmp,
                                              {1}, loc, range);

  // Store the first word, and increment counter.
  auto *index = address.getWordIndex(loc, range);
  auto *ptr = spvBuilder.createAccessChain(astContext.UnsignedIntTy, buffer,
                                           {constUint0, index}, loc, range);
  spvBuilder.createStore(ptr, A, loc, range);
  address.incrementWordIndex(loc, range);

  // Store the second word, and increment counter.
  index = address.getWordIndex(loc, range);
  ptr = spvBuilder.createAccessChain(astContext.UnsignedIntTy, buffer,
                                     {constUint0, index}, loc, range);
  spvBuilder.createStore(ptr, B, loc, range);
  address.incrementWordIndex(loc, range);
}

QualType RawBufferHandler::serializeToScalarsOrStruct(
    std::deque<SpirvInstruction *> *values, QualType valueType,
    SourceLocation loc, SourceRange range) {
  uint32_t size = values->size();

  // Vector type
  {
    QualType elemType = {};
    uint32_t elemCount = 0;
    if (isVectorType(valueType, &elemType, &elemCount)) {
      for (uint32_t i = 0; i < size; ++i) {
        for (uint32_t j = 0; j < elemCount; ++j) {
          values->push_back(spvBuilder.createCompositeExtract(
              elemType, values->front(), {j}, loc, range));
        }
        values->pop_front();
      }
      return elemType;
    }
  }

  // Matrix type
  {
    QualType elemType = {};
    uint32_t numRows = 0, numCols = 0;
    if (isMxNMatrix(valueType, &elemType, &numRows, &numCols)) {
      // Check if the destination buffer expects matrices in column major or row
      // major order. In the future, we may also need to consider the
      // `row_major` and `column_major` attribures. This is not handled by
      // HLSL/DXIL at the moment, so we ignore them too.
      const bool isBufferColumnMajor =
          !theEmitter.getSpirvOptions().useLegacyBufferMatrixOrder;
      for (uint32_t i = 0; i < size; ++i) {
        if (isBufferColumnMajor) {
          // Access the matrix in the column major order.
          for (uint32_t j = 0; j != numCols; ++j) {
            for (uint32_t k = 0; k != numRows; ++k) {
              values->push_back(spvBuilder.createCompositeExtract(
                  elemType, values->front(), {k, j}, loc, range));
            }
          }
        } else {
          // Access the matrix in the row major order.
          for (uint32_t j = 0; j != numRows; ++j) {
            for (uint32_t k = 0; k != numCols; ++k) {
              values->push_back(spvBuilder.createCompositeExtract(
                  elemType, values->front(), {j, k}, loc, range));
            }
          }
        }
        values->pop_front();
      }
      return serializeToScalarsOrStruct(values, elemType, loc, range);
    }
  }

  // Array type
  {
    if (const auto *arrType = astContext.getAsConstantArrayType(valueType)) {
      const uint32_t arrElemCount =
          static_cast<uint32_t>(arrType->getSize().getZExtValue());
      const QualType arrElemType = arrType->getElementType();
      for (uint32_t i = 0; i < size; ++i) {
        for (uint32_t j = 0; j < arrElemCount; ++j) {
          values->push_back(spvBuilder.createCompositeExtract(
              arrElemType, values->front(), {j}, loc, range));
        }
        values->pop_front();
      }
      return serializeToScalarsOrStruct(values, arrElemType, loc, range);
    }
  }

  if (isScalarType(valueType))
    return valueType;

  if (valueType->getAs<RecordType>())
    return valueType;

  llvm_unreachable("unhandled type when serializing an array");
}

void RawBufferHandler::processTemplatedStoreToBuffer(SpirvInstruction *value,
                                                     SpirvInstruction *buffer,
                                                     BufferAddress &address,
                                                     const QualType valueType,
                                                     SourceRange range) {
  const auto loc = buffer->getSourceLocation();

  // Scalar types
  if (isScalarType(valueType)) {
    auto storeWidth = getElementSpirvBitwidth(
        astContext, valueType, theEmitter.getSpirvOptions().enable16BitTypes);
    switch (storeWidth) {
    case 16:
      store16Bits(value, buffer, address, valueType, range);
      return;
    case 32:
      store32Bits(value, buffer, address, valueType, range);
      return;
    case 64:
      store64Bits(value, buffer, address, valueType, range);
      return;
    default:
      theEmitter.emitError(
          "templated store of ByteAddressBuffer is only implemented for "
          "16, 32, and 64-bit types",
          loc);
      return;
    }
  }

  // Vectors, Matrices, and Arrays can all be serialized and stored.
  if (isVectorType(valueType) || isMxNMatrix(valueType) ||
      isConstantArrayType(astContext, valueType)) {
    std::deque<SpirvInstruction *> elems;
    elems.push_back(value);
    auto serializedType =
        serializeToScalarsOrStruct(&elems, valueType, loc, range);
    if (isScalarType(serializedType) || serializedType->getAs<RecordType>()) {
      for (auto elem : elems)
        processTemplatedStoreToBuffer(elem, buffer, address, serializedType,
                                      range);
    }
    return;
  }

  // Struct types
  // The "natural" layout for structure types dictates that structs are
  // aligned like their field with the largest alignment.
  // As a result, there might exist some padding after some struct members.
  if (const auto *structType = valueType->getAs<RecordType>()) {
    LowerTypeVisitor lowerTypeVisitor(astContext, theEmitter.getSpirvContext(),
                                      theEmitter.getSpirvOptions(), spvBuilder);
    auto *decl = valueType->getAsTagDecl();
    assert(decl && "Expected all structs to be tag decls.");
    const StructType *spvType = dyn_cast<StructType>(lowerTypeVisitor.lowerType(
        valueType, theEmitter.getSpirvOptions().sBufferLayoutRule, llvm::None,
        decl->getLocation()));
    assert(spvType);
    forEachSpirvField(
        structType, spvType,
        [this, &address, loc, range, buffer, value](size_t spirvFieldIndex,
                                                    const QualType &fieldType,
                                                    const auto &field) {
          auto *baseOffset = address.getByteAddress();
          if (field.offset.hasValue() && field.offset.getValue() != 0) {
            SpirvConstant *offset = spvBuilder.getConstantInt(
                astContext.UnsignedIntTy,
                llvm::APInt(32, field.offset.getValue()));
            baseOffset = spvBuilder.createBinaryOp(
                spv::Op::OpIAdd, astContext.UnsignedIntTy, baseOffset, offset,
                loc, range);
          }

          processTemplatedStoreToBuffer(
              spvBuilder.createCompositeExtract(
                  fieldType, value, {static_cast<uint32_t>(spirvFieldIndex)},
                  loc, range),
              buffer, baseOffset, fieldType, range);
          return true;
        });

    // After we're done with storing the entire struct, we need to update the
    // byteAddress (in case we are storing an array of structs).
    //
    // Example: struct alignment = 8. struct size = 34 bytes
    // (34 / 8) = 4 full words
    // (34 % 8) = 2 > 0, therefore need to move to the next aligned address
    // So the starting byte offset after loading the entire struct is:
    // 8 * (4 + 1) = 40
    uint32_t structAlignment = 0, structSize = 0, stride = 0;
    std::tie(structAlignment, structSize) =
        AlignmentSizeCalculator(astContext, theEmitter.getSpirvOptions())
            .getAlignmentAndSize(valueType,
                                 theEmitter.getSpirvOptions().sBufferLayoutRule,
                                 llvm::None, &stride);

    assert(structAlignment != 0);
    auto *structWidth = spvBuilder.getConstantInt(
        astContext.UnsignedIntTy,
        llvm::APInt(32, roundToPow2(structSize, structAlignment)));
    address.incrementByteAddress(structWidth, loc, range);

    return;
  }

  llvm_unreachable("templated buffer store unimplemented for type");
}

void RawBufferHandler::processTemplatedStoreToBuffer(
    SpirvInstruction *value, SpirvInstruction *buffer,
    SpirvInstruction *&byteAddress, const QualType valueType,
    SourceRange range) {
  BufferAddress address(byteAddress, theEmitter);

  processTemplatedStoreToBuffer(value, buffer, address, valueType, range);
}

SpirvInstruction *RawBufferHandler::BufferAddress::getByteAddress() {
  return byteAddress;
}

SpirvInstruction *
RawBufferHandler::BufferAddress::getWordIndex(SourceLocation loc,
                                              SourceRange range) {
  if (!wordIndex.hasValue()) {
    auto *constUint2 =
        spvBuilder.getConstantInt(astContext.UnsignedIntTy, llvm::APInt(32, 2));

    // Divide the byte index by 4 (shift right by 2) to get the index in the
    // word-sized buffer.
    wordIndex = spvBuilder.createBinaryOp(spv::Op::OpShiftRightLogical,
                                          astContext.UnsignedIntTy, byteAddress,
                                          constUint2, loc, range);
  }

  return wordIndex.getValue();
}

void RawBufferHandler::BufferAddress::incrementByteAddress(
    SpirvInstruction *width, SourceLocation loc, SourceRange range) {
  byteAddress =
      spvBuilder.createBinaryOp(spv::Op::OpIAdd, astContext.UnsignedIntTy,
                                byteAddress, width, loc, range);
  wordIndex.reset();
}

void RawBufferHandler::BufferAddress::incrementByteAddress(uint32_t width,
                                                           SourceLocation loc,
                                                           SourceRange range) {
  incrementByteAddress(spvBuilder.getConstantInt(astContext.UnsignedIntTy,
                                                 llvm::APInt(32, width)),
                       loc, range);
}

void RawBufferHandler::BufferAddress::incrementWordIndex(SourceLocation loc,
                                                         SourceRange range) {

  auto *constUint1 =
      spvBuilder.getConstantInt(astContext.UnsignedIntTy, llvm::APInt(32, 1));

  auto *oldWordIndex = getWordIndex(loc, range);

  // Keep byte address up-to-date. If this is unneeded the optimizer will remove
  // it.
  incrementByteAddress(4, loc, range);

  wordIndex =
      spvBuilder.createBinaryOp(spv::Op::OpIAdd, astContext.UnsignedIntTy,
                                oldWordIndex, constUint1, loc, range);
}

} // namespace spirv
} // namespace clang
