// Copyright (c) 2024 Google LLC
//
// This file is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef _HLSL_VK_KHR_COOPERATIVE_MATRIX_H_
#define _HLSL_VK_KHR_COOPERATIVE_MATRIX_H_

#if __SPIRV_MAJOR_VERSION__ == 1 && __SPIRV_MINOR_VERSION__ < 6
#error "CooperativeMatrix requires a minimum of SPIR-V 1.6"
#endif

#include "vk/spirv.h"

namespace vk {
namespace khr {

// The base cooperative matrix class. The template arguments correspond to the
// operands in the OpTypeCooperativeMatrixKHR instruction.
template <typename ComponentType, Scope scope, uint rows, uint columns,
          CooperativeMatrixUse use>
class CooperativeMatrix {
  template <class NewComponentType>
  CooperativeMatrix<NewComponentType, scope, rows, columns, use> cast();

  // Apply OpSNegate or OFNegate, depending on ComponentType, in a element by
  // element manner.
  CooperativeMatrix negate();

  // Apply OpIAdd or OFAdd, depending on ComponentType, in a element by element
  // manner.
  CooperativeMatrix operator+(CooperativeMatrix other);

  // Apply OpISub or OFSub, depending on ComponentType, in a element by element
  // manner.
  CooperativeMatrix operator-(CooperativeMatrix other);

  // Apply OpIMul or OFMul, depending on ComponentType, in a element by element
  // manner.
  CooperativeMatrix operator*(CooperativeMatrix other);

  // Apply OpSDiv, OpUDiv or OFDiv, depending on ComponentType, in a element by
  // element manner.
  CooperativeMatrix operator/(CooperativeMatrix other);

  // Apply OpMatrixTimesScalar in a element by element manner.
  CooperativeMatrix operator*(ComponentType scalar);

  // Store the cooperative matrix using OpCooperativeMatrixStoreKHR to
  // data using the given memory layout, stride, and memory access operands.
  // `NonPrivatePointer` and `MakePointerAvailable` with the workgroup scope
  // will be added to the memory access operands to make the memory coherent.
  //
  // This function uses a SPIR-V pointer because HLSL does not allow groupshared
  // memory object to be passed by reference. The pointer is a hack to get
  // around that.
  //
  // The layout and stride will be passed to the SPIR-V instruction as is. The
  // precise meaning can be found in the specification for
  // SPV_KHR_cooperative_matrix.
  template <uint32_t memoryAccessOperands, CooperativeMatrixLayout layout,
            class Type>
  void Store(WorkgroupSpirvPointer<Type> data, uint32_t stride);

  // Same as above, but uses MemoryAccessMaskNone for the memory access
  // operands.
  template <CooperativeMatrixLayout layout, class Type>
  void Store(WorkgroupSpirvPointer<Type> data, uint32_t stride) {
    Store<MemoryAccessMaskNone, layout>(data, stride);
  }

  // Store the cooperative matrix using OpCooperativeMatrixStoreKHR to
  // data[index] using the given memory layout, stride, and memory access
  // operands. The layout and stride will be passed to the SPIR-V instruction as
  // is. The precise meaning can be found in the specification for
  // SPV_KHR_cooperative_matrix.
  template <uint32_t memoryAccessOperands, CooperativeMatrixLayout layout,
            class Type>
  void Store(RWStructuredBuffer<Type> data, uint32_t index, uint32_t stride);

  // Same as above, but uses MemoryAccessMaskNone for the memory access
  // operands.
  template <CooperativeMatrixLayout layout, class Type>
  void Store(RWStructuredBuffer<Type> data, uint32_t index, uint32_t stride) {
    Store<MemoryAccessMaskNone, layout>(data, index, stride);
  }

  // Store the cooperative matrix using OpCooperativeMatrixStoreKHR to
  // data[index] using the given memory layout, stride, and memory access
  // operands. `NonPrivatePointer` and `MakePointerAvailable` with the
  // QueueFamily scope will be added to the memory access operands to make the
  // memory coherent.
  //
  // The layout and stride will be passed to the SPIR-V instruction as is. The
  // precise meaning can be found in the specification for
  // SPV_KHR_cooperative_matrix.
  template <uint32_t memoryAccessOperands, CooperativeMatrixLayout layout,
            class Type>
  void CoherentStore(globallycoherent RWStructuredBuffer<Type> data,
                     uint32_t index, uint32_t stride);

  // Same as above, but uses MemoryAccessMaskNone for the memory access operands
  // template argument.
  template <CooperativeMatrixLayout layout, class Type>
  void CoherentStore(globallycoherent RWStructuredBuffer<Type> data,
                     uint32_t index, uint32_t stride) {
    CoherentStore<MemoryAccessMaskNone, layout>(data, index, stride);
  }

  // Loads a cooperative matrix using OpCooperativeMatrixLoadKHR from
  // data using the given memory layout, stride, and memory access operands.
  // `NonPrivatePointer` and `MakePointerVisible` with the workgroup scope
  // will be added to the memory access operands to make the memory coherent.
  //
  // This function uses a SPIR-V pointer because HLSL does not allow groupshared
  // memory object to be passed by reference. The pointer is a hack to get
  // around that.
  //
  // The layout and stride will be passed to the SPIR-V instruction as is. The
  // precise meaning can be found in the specification for
  // SPV_KHR_cooperative_matrix.
  template <uint32_t memoryAccessOperands, CooperativeMatrixLayout layout,
            class Type>
  static CooperativeMatrix Load(WorkgroupSpirvPointer<Type> data,
                                uint32_t stride);

  // Same as above, but uses MemoryAccessMaskNone for the memory access
  // operands.
  template <CooperativeMatrixLayout layout, class Type>
  static CooperativeMatrix Load(WorkgroupSpirvPointer<Type> data,
                                uint32_t stride) {
    return Load<MemoryAccessMaskNone, layout>(data, stride);
  }

  // Loads a cooperative matrix using OpCooperativeMatrixLoadKHR from
  // data[index] using the given memory layout, stride, and memory access
  // operands.
  //
  // The layout and stride will be passed to the SPIR-V instruction as is. The
  // precise meaning can be found in the specification for
  // SPV_KHR_cooperative_matrix.
  template <uint32_t memoryAccessOperands, CooperativeMatrixLayout layout,
            class Type>
  static CooperativeMatrix Load(RWStructuredBuffer<Type> data, uint32_t index,
                                uint32_t stride);

  // Same as above, but uses MemoryAccessMaskNone for the memory access
  // operands.
  template <CooperativeMatrixLayout layout, class Type>
  static CooperativeMatrix Load(RWStructuredBuffer<Type> data, uint32_t index,
                                uint32_t stride) {
    return Load<MemoryAccessMaskNone, layout>(data, index, stride);
  }

  // Loads a cooperative matrix using OpCooperativeMatrixLoadKHR from
  // data[index] using the given memory layout, stride, and memory access
  // operands. `NonPrivatePointer` and `MakePointerVisible` with the QueueFamily
  // scope will be added to the memory access operands to make the memory
  // coherent.
  //
  //
  // The layout and stride will be passed to the SPIR-V instruction as is. The
  // precise meaning can be found in the specification for
  // SPV_KHR_cooperative_matrix.
  template <uint32_t memoryAccessOperands, CooperativeMatrixLayout layout,
            class Type>
  static CooperativeMatrix
  CoherentLoad(globallycoherent RWStructuredBuffer<Type> data, uint32_t index,
               uint32_t stride);

  // Same as above, but uses MemoryAccessMaskNone for the memory access operands
  // template argument.
  template <CooperativeMatrixLayout layout, class Type>
  static CooperativeMatrix
  CoherentLoad(globallycoherent RWStructuredBuffer<Type> data, uint32_t index,
               uint32_t stride) {
    return CoherentLoad<MemoryAccessMaskNone, layout>(data, index, stride);
  }

  // Loads a cooperative matrix using OpCooperativeMatrixLoadKHR from
  // data[index] using the given memory layout, stride, and memory access
  // operands. No memory access bits are added to the operands. Since the memory
  // is readonly, there should be no need.
  //
  // The layout and stride will be passed to the SPIR-V instruction as is. The
  // precise meaning can be found in the specification for
  // SPV_KHR_cooperative_matrix.
  template <uint32_t memoryAccessOperands, CooperativeMatrixLayout layout,
            class Type>
  static CooperativeMatrix Load(StructuredBuffer<Type> data, uint32_t index,
                                uint32_t stride);

  // Same as above, but uses MemoryAccessMaskNone for the memory access
  // operands.
  template <CooperativeMatrixLayout layout, class Type>
  static CooperativeMatrix Load(StructuredBuffer<Type> data, uint32_t index,
                                uint32_t stride) {
    return Load<MemoryAccessMaskNone, layout>(data, index, stride);
  }

  // Constructs a cooperative matrix with all values initialized to v. Note that
  // all threads in scope must have the same value for v.
  static CooperativeMatrix Splat(ComponentType v);

  // Returns the result of OpCooperativeMatrixLengthKHR on the current type.￼
  static uint32_t GetLength();

  // Functions to access the elements of the cooperative matrix. The index must
  // be less than GetLength().
  void Set(ComponentType value, uint32_t index);
  ComponentType Get(uint32_t index);

  static const bool hasSignedIntegerComponentType =
      (ComponentType(0) - ComponentType(1) < ComponentType(0));

  // clang-format off
  using SpirvMatrixType = vk::SpirvOpaqueType<
      /* OpTypeCooperativeMatrixKHR */ 4456, ComponentType,
      vk::integral_constant<uint, scope>, vk::integral_constant<uint, rows>,
      vk::integral_constant<uint, columns>, vk::integral_constant<uint, use> >;

  [[vk::ext_extension("SPV_KHR_cooperative_matrix")]]
  [[vk::ext_capability(/* CooperativeMatrixKHRCapability */ 6022)]]
  [[vk::ext_capability(/* VulkanMemoryModel */ 5345)]]
  SpirvMatrixType _matrix;
  // clang-format on
};

// Cooperative matrix that can be used in the "a" position of a multiply add
// instruction (r = (a * b) + c).
template <typename ComponentType, Scope scope, uint rows, uint columns>
using CooperativeMatrixA =
    CooperativeMatrix<ComponentType, scope, rows, columns,
                      CooperativeMatrixUseMatrixAKHR>;

// Cooperative matrix that can be used in the "b" position of a multiply add
// instruction (r = (a * b) + c).
template <typename ComponentType, Scope scope, uint rows, uint columns>
using CooperativeMatrixB =
    CooperativeMatrix<ComponentType, scope, rows, columns,
                      CooperativeMatrixUseMatrixBKHR>;

// Cooperative matrix that can be used in the "r" and "c" position of a multiply
// add instruction (r = (a * b) + c).
template <typename ComponentType, Scope scope, uint rows, uint columns>
using CooperativeMatrixAccumulator =
    CooperativeMatrix<ComponentType, scope, rows, columns,
                      CooperativeMatrixUseMatrixAccumulatorKHR>;

// Returns the result of OpCooperativeMatrixMulAddKHR when applied to a, b, and
// c. The cooperative matrix operands are inferred, with the
// SaturatingAccumulationKHR bit not set.
template <typename ComponentType, Scope scope, uint rows, uint columns, uint K>
CooperativeMatrixAccumulator<ComponentType, scope, rows, columns>
cooperativeMatrixMultiplyAdd(
    CooperativeMatrixA<ComponentType, scope, rows, K> a,
    CooperativeMatrixB<ComponentType, scope, K, columns> b,
    CooperativeMatrixAccumulator<ComponentType, scope, rows, columns> c);

// Returns the result of OpCooperativeMatrixMulAddKHR when applied to a, b, and
// c. The cooperative matrix operands are inferred, with the
// SaturatingAccumulationKHR bit set.
template <typename ComponentType, Scope scope, uint rows, uint columns, uint K>
CooperativeMatrixAccumulator<ComponentType, scope, rows, columns>
cooperativeMatrixSaturatingMultiplyAdd(
    CooperativeMatrixA<ComponentType, scope, rows, K> a,
    CooperativeMatrixB<ComponentType, scope, K, columns> b,
    CooperativeMatrixAccumulator<ComponentType, scope, rows, columns> c);

} // namespace khr
} // namespace vk

#include "cooperative_matrix.impl"
#endif // _HLSL_VK_KHR_COOPERATIVE_MATRIX_H_
