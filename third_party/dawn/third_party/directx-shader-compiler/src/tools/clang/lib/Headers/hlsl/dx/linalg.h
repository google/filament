//===----------------------------------------------------------------------===//
//
// Part of the DirectXShaderCompiler, under the Apache License v2.0 with LLVM
// Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// DirectX Shader Model 6.10 Linear Algebra objects and APIs.
//===----------------------------------------------------------------------===//

#include <enable_if>
#include <type_traits>

#if ((__SHADER_TARGET_MAJOR > 6) ||                                            \
     (__SHADER_TARGET_MAJOR == 6 && __SHADER_TARGET_MINOR >= 10)) &&           \
    (__HLSL_VERSION >= 2021)

#pragma dxc diagnostic push
#pragma dxc diagnostic ignored "-Whlsl-groupshared-202x"

namespace dxil {

// This enum must _exactly_ match the DXIL constants.
enum class ComponentType : uint32_t {
  Invalid = 0,
  I1 = 1,
  I16 = 2,
  U16 = 3,
  I32 = 4,
  U32 = 5,
  I64 = 6,
  U64 = 7,
  F16 = 8,
  F32 = 9,
  F64 = 10,
  SNormF16 = 11,
  UNormF16 = 12,
  SNormF32 = 13,
  UNormF32 = 14,
  SNormF64 = 15,
  UNormF64 = 16,
  PackedS8x32 = 17,
  PackedU8x32 = 18,

  // BEGIN NEW FOR SM 6.9
  I8 = 19,
  U8 = 20,
  F8_E4M3FN = 21,
  F8_E5M2 = 22,
  // END

  // BEGIN NEW FOR SM 6.10
  BFloat16 = 23,
  // END

  LastEntry
};

} // namespace dxil

namespace dx {

namespace linalg {

#define __COMPONENT_TYPE(type) type = (uint)dxil::ComponentType::type

// This enum only defines values that are valid for Matrix component types.
// Each enumeration's value matches the cooresponding DXIL constant.
struct ComponentType {
  enum ComponentEnum {
    // Signed integers.
    __COMPONENT_TYPE(I8),
    __COMPONENT_TYPE(I16),
    __COMPONENT_TYPE(I32),
    __COMPONENT_TYPE(I64),

    // Unsigned integers.
    __COMPONENT_TYPE(U8),
    __COMPONENT_TYPE(U16),
    __COMPONENT_TYPE(U32),
    __COMPONENT_TYPE(U64),

    // Floating point types.
    __COMPONENT_TYPE(F8_E4M3FN),
    __COMPONENT_TYPE(F8_E5M2),
    __COMPONENT_TYPE(F16),
    __COMPONENT_TYPE(F32),
    __COMPONENT_TYPE(F64),
    __COMPONENT_TYPE(BFloat16),
  };
};

#undef __COMPONENT_TYPE

using ComponentEnum = ComponentType::ComponentEnum;

struct MatrixUse {
  enum MatrixUseEnum {
    A = 0,
    B = 1,
    Accumulator = 2,
  };
};
using MatrixUseEnum = MatrixUse::MatrixUseEnum;

struct MatrixScope {
  enum MatrixScopeEnum {
    Thread = 0,
    Wave = 1,
    ThreadGroup = 2,
  };
};
using MatrixScopeEnum = MatrixScope::MatrixScopeEnum;

struct MatrixLayout {
  enum MatrixLayoutEnum {
    RowMajor = 0,
    ColMajor = 1,
    MulOptimal = 2,
    MulOptimalTranspose = 3,
    OuterProductOptimal = 4,
    OuterProductOptimalTranspose = 5,
  };
};
using MatrixLayoutEnum = MatrixLayout::MatrixLayoutEnum;

namespace __detail {
template <ComponentEnum CompTy> struct ComponentTypeTraits {
  using Type = uint;
  static const bool IsNativeScalar = false;
  static const uint ElementsPerScalar = 4;
};

template <typename T> struct TypeTraits {
  static const ComponentEnum CompType =
      (ComponentEnum)dxil::ComponentType::Invalid;
};

template <> struct ComponentTypeTraits<ComponentType::BFloat16> {
  using Type = uint;
  static const bool IsNativeScalar = false;
  static const uint ElementsPerScalar = 2;
};

#define __MATRIX_SCALAR_COMPONENT_MAPPING(enum_val, type)                      \
  template <> struct ComponentTypeTraits<enum_val> {                           \
    using Type = type;                                                         \
    static const bool IsNativeScalar = true;                                   \
    static const uint ElementsPerScalar = 1;                                   \
  };                                                                           \
  template <> struct TypeTraits<type> {                                        \
    static const ComponentEnum CompType = enum_val;                            \
  };

#if __HLSL_ENABLE_16_BIT
__MATRIX_SCALAR_COMPONENT_MAPPING(ComponentType::I16, int16_t)
__MATRIX_SCALAR_COMPONENT_MAPPING(ComponentType::U16, uint16_t)
__MATRIX_SCALAR_COMPONENT_MAPPING(ComponentType::F16, float16_t)
#endif

__MATRIX_SCALAR_COMPONENT_MAPPING(ComponentType::I32, int32_t)
__MATRIX_SCALAR_COMPONENT_MAPPING(ComponentType::U32, uint32_t)
__MATRIX_SCALAR_COMPONENT_MAPPING(ComponentType::F32, float)
__MATRIX_SCALAR_COMPONENT_MAPPING(ComponentType::I64, int64_t)
__MATRIX_SCALAR_COMPONENT_MAPPING(ComponentType::U64, uint64_t)
__MATRIX_SCALAR_COMPONENT_MAPPING(ComponentType::F64, double)

template <ComponentEnum DstTy, ComponentEnum SrcTy, int SrcN> struct DstN {
  // Make sure to round up in case SrcN isn't an even multiple of the number of
  // elements per scalar
  static const int Value =
      (SrcN * ComponentTypeTraits<SrcTy>::ElementsPerScalar +
       ComponentTypeTraits<DstTy>::ElementsPerScalar - 1) /
      ComponentTypeTraits<DstTy>::ElementsPerScalar;
};

template <SIZE_TYPE MVal, SIZE_TYPE NVal, bool Transposed> struct DimMN {
  static const SIZE_TYPE M = MVal;
  static const SIZE_TYPE N = NVal;
};

template <SIZE_TYPE MVal, SIZE_TYPE NVal> struct DimMN<MVal, NVal, true> {
  static const SIZE_TYPE M = NVal;
  static const SIZE_TYPE N = MVal;
};

template <ComponentEnum CompTy, SIZE_TYPE PackedComponentCount>
struct ScalarCountFromPackedComponents {
  static const SIZE_TYPE ElementsPerScalar =
      ComponentTypeTraits<CompTy>::ElementsPerScalar;
  static const SIZE_TYPE Value =
      (PackedComponentCount + ElementsPerScalar - 1) / ElementsPerScalar;
};

} // namespace __detail

template <ComponentEnum ElementType, uint DimA> struct VectorRef {
  ByteAddressBuffer Buf;
  uint Offset;
};

template <typename T, int N, ComponentEnum DT> struct InterpretedVector {
  vector<T, N> Data;
  static const ComponentEnum Interpretation = DT;
  static const SIZE_TYPE Size =
      __detail::ComponentTypeTraits<DT>::ElementsPerScalar * N;
};

template <ComponentEnum DT, typename T, int N>
InterpretedVector<T, N, DT> MakeInterpretedVector(vector<T, N> Vec) {
  InterpretedVector<T, N, DT> IV = {Vec};
  return IV;
}

template <ComponentEnum DestTy, ComponentEnum OriginTy, typename T, int N>
typename hlsl::enable_if<
    DestTy != OriginTy,
    InterpretedVector<typename __detail::ComponentTypeTraits<DestTy>::Type,
                      __detail::DstN<DestTy, OriginTy, N>::Value,
                      DestTy> >::type
Convert(vector<T, N> Vec) {
  vector<typename __detail::ComponentTypeTraits<DestTy>::Type,
         __detail::DstN<DestTy, OriginTy, N>::Value>
      Result;
  __builtin_LinAlg_Convert(Result, Vec, OriginTy, DestTy);
  return MakeInterpretedVector<DestTy>(Result);
}

template <ComponentEnum DestTy, ComponentEnum OriginTy, typename T, int N>
typename hlsl::enable_if<DestTy == OriginTy,
                         InterpretedVector<T, N, DestTy> >::type
Convert(vector<T, N> Vec) {
  return MakeInterpretedVector<DestTy>(Vec);
}

template <ComponentEnum ComponentTy, SIZE_TYPE M, SIZE_TYPE N,
          MatrixUseEnum Use, MatrixScopeEnum Scope>
class Matrix {
  using ElementType = typename __detail::ComponentTypeTraits<ComponentTy>::Type;
  // If this isn't a native scalar, we have a type that may pack more than 1
  // element in each scalar value. (Ex. 8bit => 4elems, 16bit => 2elems)
  static const uint ElementsPerScalar =
      __detail::ComponentTypeTraits<ComponentTy>::ElementsPerScalar;
  static const bool IsNativeScalar =
      __detail::ComponentTypeTraits<ComponentTy>::IsNativeScalar;

  using HandleT = __builtin_LinAlgMatrix
      [[__LinAlgMatrix_Attributes(ComponentTy, M, N, Use, Scope)]];
  HandleT __handle;

  template <ComponentEnum NewCompTy, MatrixUseEnum NewUse = Use,
            bool Transpose = false>
  [[nodiscard]] Matrix<NewCompTy, __detail::DimMN<M, N, Transpose>::M,
                       __detail::DimMN<M, N, Transpose>::N, NewUse, Scope>
  Cast() {
    Matrix<NewCompTy, __detail::DimMN<M, N, Transpose>::M,
           __detail::DimMN<M, N, Transpose>::N, NewUse, Scope>
        Result;
    __builtin_LinAlg_CopyConvertMatrix(Result.__handle, __handle, Transpose);
    return Result;
  }

  template <typename T>
  [[nodiscard]] static
      typename hlsl::enable_if<hlsl::is_arithmetic<T>::value, Matrix>::type
      Splat(T Val) {
    Matrix Result;
    __builtin_LinAlg_FillMatrix(Result.__handle, Val);
    return Result;
  }

  template <uint Align = 128>
  [[nodiscard]] static Matrix Load(ByteAddressBuffer Res, uint StartOffset,
                                   uint Stride, MatrixLayoutEnum Layout) {
    Matrix Result;
    __builtin_LinAlg_MatrixLoadFromDescriptor(Result.__handle, Res, StartOffset,
                                              Stride, Layout, Align);
    return Result;
  }

  template <uint Align = 128>
  [[nodiscard]] static Matrix Load(RWByteAddressBuffer Res, uint StartOffset,
                                   uint Stride, MatrixLayoutEnum Layout) {
    Matrix Result;
    __builtin_LinAlg_MatrixLoadFromDescriptor(Result.__handle, Res, StartOffset,
                                              Stride, Layout, Align);
    return Result;
  }

  template <typename T, SIZE_TYPE Size>
  [[nodiscard]] static typename hlsl::enable_if<
      (hlsl::is_same<typename hlsl::strip_vector_type<T>::type,
                     ElementType>::value ||
       hlsl::is_same<typename hlsl::strip_vector_type<T>::type,
                     uint8_t4_packed>::value),
      Matrix>::type
  Load(groupshared T Arr[Size], uint StartIdx, uint Stride,
       MatrixLayoutEnum Layout) {
    Matrix Result;
    __builtin_LinAlg_MatrixLoadFromMemory(Result.__handle, Arr, StartIdx,
                                          Stride, Layout);
    return Result;
  }

  template <ComponentEnum LocalComp = ComponentTy>
  typename hlsl::enable_if<LocalComp == ComponentTy && IsNativeScalar,
                           uint>::type
  Length() {
    return __builtin_LinAlg_MatrixLength(__handle);
  }

  template <ComponentEnum LocalComp = ComponentTy>
  typename hlsl::enable_if<LocalComp == ComponentTy && IsNativeScalar,
                           uint2>::type
  GetCoordinate(uint Index) {
    return __builtin_LinAlg_MatrixGetCoordinate(__handle, Index);
  }

  template <ComponentEnum LocalComp = ComponentTy>
  typename hlsl::enable_if<LocalComp == ComponentTy && IsNativeScalar,
                           ElementType>::type
  Get(uint Index) {
    ElementType Result;
    __builtin_LinAlg_MatrixGetElement(Result, __handle, Index);
    return Result;
  }

  template <ComponentEnum LocalComp = ComponentTy>
  typename hlsl::enable_if<LocalComp == ComponentTy && IsNativeScalar,
                           void>::type
  Set(uint Index, ElementType Value) {
    __builtin_LinAlg_MatrixSetElement(__handle, __handle, Index, Value);
  }

  template <uint Align = 128>
  void Store(RWByteAddressBuffer Res, uint StartOffset, uint Stride,
             MatrixLayoutEnum Layout) {
    __builtin_LinAlg_MatrixStoreToDescriptor(__handle, Res, StartOffset, Stride,
                                             Layout, Align);
  }

  template <typename T, SIZE_TYPE Size>
  typename hlsl::enable_if<
      (hlsl::is_same<typename hlsl::strip_vector_type<T>::type,
                     ElementType>::value ||
       hlsl::is_same<typename hlsl::strip_vector_type<T>::type,
                     uint8_t4_packed>::value),
      void>::type
  Store(groupshared T Arr[Size], uint StartIdx, uint Stride,
        MatrixLayoutEnum Layout) {
    __builtin_LinAlg_MatrixStoreToMemory(__handle, Arr, StartIdx, Stride,
                                         Layout);
  }

  // Accumulate methods
  template <uint Align = 128, MatrixUseEnum UseLocal = Use>
  typename hlsl::enable_if<Use == MatrixUse::Accumulator && UseLocal == Use,
                           void>::type
  InterlockedAccumulate(RWByteAddressBuffer Res, uint StartOffset, uint Stride,
                        MatrixLayoutEnum Layout) {
    __builtin_LinAlg_MatrixAccumulateToDescriptor(__handle, Res, StartOffset,
                                                  Stride, Layout, Align);
  }

  template <typename T, MatrixUseEnum UseLocal = Use,
            MatrixScopeEnum ScopeLocal = Scope, SIZE_TYPE Size>
  typename hlsl::enable_if<
      hlsl::is_arithmetic_vector<T>::value && Use == MatrixUse::Accumulator &&
          UseLocal == Use && Scope == MatrixScope::Wave && ScopeLocal == Scope,
      void>::type
  InterlockedAccumulate(groupshared T Arr[Size], uint StartIdx, uint Stride,
                        MatrixLayoutEnum Layout) {
    __builtin_LinAlg_MatrixAccumulateToMemory(__handle, Arr, ComponentTy,
                                              StartIdx, Stride, Layout);
  }

  template <ComponentEnum TargetCompTy = ComponentTy, typename T,
            MatrixUseEnum UseLocal = Use, MatrixScopeEnum ScopeLocal = Scope,
            SIZE_TYPE Size>
  typename hlsl::enable_if<
      hlsl::is_same<typename hlsl::strip_vector_type<T>::type,
                    uint8_t4_packed>::value &&
          Use == MatrixUse::Accumulator && UseLocal == Use &&
          Scope == MatrixScope::Wave && ScopeLocal == Scope,
      void>::type
  InterlockedAccumulate(groupshared T Arr[Size], uint StartIdx, uint Stride,
                        MatrixLayoutEnum Layout) {
    __builtin_LinAlg_MatrixAccumulateToMemory(__handle, Arr, TargetCompTy,
                                              StartIdx, Stride, Layout);
  }

  template <ComponentEnum CompTy, MatrixUseEnum UseLocal = Use>
  typename hlsl::enable_if<Use == MatrixUse::Accumulator && UseLocal == Use,
                           void>::type
  Accumulate(const Matrix<CompTy, M, N, MatrixUse::A, Scope> MatrixA) {
    __builtin_LinAlg_MatrixAccumulate(__handle, __handle, MatrixA.__handle);
  }

  template <ComponentEnum CompTy, MatrixUseEnum UseLocal = Use>
  typename hlsl::enable_if<Use == MatrixUse::Accumulator && UseLocal == Use,
                           void>::type
  Accumulate(const Matrix<CompTy, M, N, MatrixUse::B, Scope> MatrixB) {
    __builtin_LinAlg_MatrixAccumulate(__handle, __handle, MatrixB.__handle);
  }

  template <ComponentEnum LHSTy, ComponentEnum RHSTy, SIZE_TYPE K,
            MatrixUseEnum UseLocal = Use>
  typename hlsl::enable_if<Use == MatrixUse::Accumulator && UseLocal == Use,
                           void>::type
  MultiplyAccumulate(const Matrix<LHSTy, M, K, MatrixUse::A, Scope> MatrixA,
                     const Matrix<RHSTy, K, N, MatrixUse::B, Scope> MatrixB) {
    __builtin_LinAlg_MatrixMatrixMultiplyAccumulate(__handle, MatrixA.__handle,
                                                    MatrixB.__handle, __handle);
  }
};

// Thread-scope Matrices are read-only. Using a template partial
// specialization for this simplifies the SFINAE-foo above.
template <ComponentEnum ComponentTy, SIZE_TYPE M, SIZE_TYPE N,
          MatrixUseEnum Use>
class Matrix<ComponentTy, M, N, Use, MatrixScope::Thread> {
  using ElementType = typename __detail::ComponentTypeTraits<ComponentTy>::Type;

  using HandleT = __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(
      ComponentTy, M, N, Use, MatrixScope::Thread)]];
  HandleT __handle;

  template <MatrixLayoutEnum Layout, uint Align = 128,
            MatrixUseEnum UseLocal = Use>
  [[nodiscard]] static
      typename hlsl::enable_if<Use == MatrixUse::A && UseLocal == Use,
                               Matrix>::type
      Load(ByteAddressBuffer Res, uint StartOffset, uint Stride) {
    Matrix Result;
    __builtin_LinAlg_MatrixLoadFromDescriptor(Result.__handle, Res, StartOffset,
                                              Stride, Layout, Align);
    return Result;
  }

  template <uint Align = 128, MatrixUseEnum UseLocal = Use>
  typename hlsl::enable_if<Use == MatrixUse::Accumulator && UseLocal == Use,
                           void>::type
  InterlockedAccumulate(RWByteAddressBuffer Res, uint StartOffset) {
    __builtin_LinAlg_MatrixAccumulateToDescriptor(
        __handle, Res, StartOffset, 0, MatrixLayout::OuterProductOptimal,
        Align);
  }
};

MatrixUseEnum AccumulatorLayout() {
  return (MatrixUseEnum)(__builtin_LinAlg_MatrixQueryAccumulatorLayout());
}

template <ComponentEnum OutTy, ComponentEnum ATy, ComponentEnum BTy,
          SIZE_TYPE M, SIZE_TYPE N, SIZE_TYPE K>
[[nodiscard]] Matrix<OutTy, M, N, MatrixUse::Accumulator, MatrixScope::Wave>
Multiply(const Matrix<ATy, M, K, MatrixUse::A, MatrixScope::Wave> MatrixA,
         const Matrix<BTy, K, N, MatrixUse::B, MatrixScope::Wave> MatrixB) {
  Matrix<OutTy, M, N, MatrixUse::Accumulator, MatrixScope::Wave> Result;
  __builtin_LinAlg_MatrixMatrixMultiply(Result.__handle, MatrixA.__handle,
                                        MatrixB.__handle);
  return Result;
}

template <ComponentEnum CompTy, SIZE_TYPE M, SIZE_TYPE N, SIZE_TYPE K>
[[nodiscard]] Matrix<CompTy, M, N, MatrixUse::Accumulator, MatrixScope::Wave>
Multiply(const Matrix<CompTy, M, K, MatrixUse::A, MatrixScope::Wave> MatrixA,
         const Matrix<CompTy, K, N, MatrixUse::B, MatrixScope::Wave> MatrixB) {
  Matrix<CompTy, M, N, MatrixUse::Accumulator, MatrixScope::Wave> Result;
  __builtin_LinAlg_MatrixMatrixMultiply(Result.__handle, MatrixA.__handle,
                                        MatrixB.__handle);
  return Result;
}

template <ComponentEnum OutTy, ComponentEnum ATy, ComponentEnum BTy,
          SIZE_TYPE M, SIZE_TYPE N, SIZE_TYPE K>
[[nodiscard]] Matrix<OutTy, M, N, MatrixUse::Accumulator,
                     MatrixScope::ThreadGroup>
Multiply(
    const Matrix<ATy, M, K, MatrixUse::A, MatrixScope::ThreadGroup> MatrixA,
    const Matrix<BTy, K, N, MatrixUse::B, MatrixScope::ThreadGroup> MatrixB) {
  Matrix<OutTy, M, N, MatrixUse::Accumulator, MatrixScope::ThreadGroup> Result;
  __builtin_LinAlg_MatrixMatrixMultiply(Result.__handle, MatrixA.__handle,
                                        MatrixB.__handle);
  return Result;
}

template <ComponentEnum CompTy, SIZE_TYPE M, SIZE_TYPE N, SIZE_TYPE K>
[[nodiscard]] Matrix<CompTy, M, N, MatrixUse::Accumulator,
                     MatrixScope::ThreadGroup>
Multiply(
    const Matrix<CompTy, M, K, MatrixUse::A, MatrixScope::ThreadGroup> MatrixA,
    const Matrix<CompTy, K, N, MatrixUse::B, MatrixScope::ThreadGroup>
        MatrixB) {
  Matrix<CompTy, M, N, MatrixUse::Accumulator, MatrixScope::ThreadGroup> Result;
  __builtin_LinAlg_MatrixMatrixMultiply(Result.__handle, MatrixA.__handle,
                                        MatrixB.__handle);
  return Result;
}

// Cooperative Vector Replacement API
// Cooperative Vector operates on per-thread vectors multiplying against B
// matrices with thread scope.

template <typename OutputElTy, typename InputElTy, SIZE_TYPE M, SIZE_TYPE K,
          ComponentEnum MatrixDT>
typename hlsl::enable_if<hlsl::is_arithmetic<InputElTy>::value,
                         vector<OutputElTy, M> >::type
Multiply(Matrix<MatrixDT, M, K, MatrixUse::A, MatrixScope::Thread> MatrixA,
         vector<InputElTy, K> Vec) {
  vector<OutputElTy, M> Result;
  __builtin_LinAlg_MatrixVectorMultiply(Result, MatrixA.__handle,
                                        hlsl::is_signed<OutputElTy>::value, Vec,
                                        MatrixDT);
  return Result;
}

template <typename OutputElTy, typename InputElTy, ComponentEnum InputInterp,
          SIZE_TYPE M, SIZE_TYPE K, SIZE_TYPE VecK, ComponentEnum MatrixDT>
typename hlsl::enable_if<
    InterpretedVector<InputElTy, VecK, InputInterp>::Size == K,
    vector<OutputElTy, M> >::type
Multiply(Matrix<MatrixDT, M, K, MatrixUse::A, MatrixScope::Thread> MatrixA,
         InterpretedVector<InputElTy, VecK, InputInterp> InterpVec) {
  vector<OutputElTy, M> Result;
  __builtin_LinAlg_MatrixVectorMultiply(
      Result, MatrixA.__handle, hlsl::is_signed<OutputElTy>::value,
      InterpVec.Data, InterpVec.Interpretation);
  return Result;
}

template <typename OutputElTy, typename InputElTy, typename BiasElTy,
          SIZE_TYPE M, SIZE_TYPE K, ComponentEnum MatrixDT>
typename hlsl::enable_if<hlsl::is_arithmetic<InputElTy>::value &&
                             hlsl::is_arithmetic<BiasElTy>::value,
                         vector<OutputElTy, M> >::type
MultiplyAdd(Matrix<MatrixDT, M, K, MatrixUse::A, MatrixScope::Thread> MatrixA,
            vector<InputElTy, K> Vec, vector<BiasElTy, M> Bias) {

  InterpretedVector<OutputElTy, M, __detail::TypeTraits<OutputElTy>::CompType>
      BiasConvInterp = Convert<__detail::TypeTraits<OutputElTy>::CompType,
                               __detail::TypeTraits<BiasElTy>::CompType>(Bias);

  vector<OutputElTy, M> Result;
  __builtin_LinAlg_MatrixVectorMultiplyAdd(
      Result, MatrixA.__handle, hlsl::is_signed<OutputElTy>::value, Vec,
      __detail::TypeTraits<InputElTy>::CompType, BiasConvInterp.Data);
  return Result;
}

template <typename OutputElTy, typename InputElTy, ComponentEnum InputInterp,
          typename BiasElTy, SIZE_TYPE M, SIZE_TYPE K, SIZE_TYPE VecK,
          ComponentEnum MatrixDT>
typename hlsl::enable_if<
    VecK == __detail::ScalarCountFromPackedComponents<InputInterp, K>::Value &&
        hlsl::is_arithmetic<BiasElTy>::value,
    vector<OutputElTy, M> >::type
MultiplyAdd(Matrix<MatrixDT, M, K, MatrixUse::A, MatrixScope::Thread> MatrixA,
            InterpretedVector<InputElTy, VecK, InputInterp> InterpVec,
            vector<BiasElTy, M> Bias) {

  InterpretedVector<OutputElTy, M, __detail::TypeTraits<OutputElTy>::CompType>
      BiasConvInterp = Convert<__detail::TypeTraits<OutputElTy>::CompType,
                               __detail::TypeTraits<BiasElTy>::CompType>(Bias);

  vector<OutputElTy, M> Result;
  __builtin_LinAlg_MatrixVectorMultiplyAdd(
      Result, MatrixA.__handle, hlsl::is_signed<OutputElTy>::value,
      InterpVec.Data, InterpVec.Interpretation, BiasConvInterp.Data);
  return Result;
}

template <typename OutputElTy, typename InputElTy, ComponentEnum BiasElTy,
          SIZE_TYPE M, SIZE_TYPE K, ComponentEnum MatrixDT>
typename hlsl::enable_if<hlsl::is_arithmetic<InputElTy>::value,
                         vector<OutputElTy, M> >::type
MultiplyAdd(Matrix<MatrixDT, M, K, MatrixUse::A, MatrixScope::Thread> MatrixA,
            vector<InputElTy, K> Vec, VectorRef<BiasElTy, M> BiasRef) {

  using BiasVecTy =
      vector<typename __detail::ComponentTypeTraits<BiasElTy>::Type,
             __detail::ScalarCountFromPackedComponents<BiasElTy, M>::Value>;
  BiasVecTy Bias = BiasRef.Buf.template Load<BiasVecTy>(BiasRef.Offset);

  // FIXME: Convert currently does not support packed type vector sizes that
  // are not a multiple of the number of elements per scalar, so we
  // need to do an extra conversion here to get it into the right shape.
  // For example if BiasRef is F8_E4M3FN and M is 7, it gets loaded to into
  // vector<uint, 2>, and if OutputElTy is half, Convert will return
  // vector<half, 8> instead of vector<half, 7>.
  // https://github.com/microsoft/DirectXShaderCompiler/issues/8418

  // Convert to OutputElTy vector with padding
  using BiasConvInterpPaddedTy = InterpretedVector<
      OutputElTy,
      __detail::DstN<__detail::TypeTraits<OutputElTy>::CompType, BiasElTy,
                     __detail::ScalarCountFromPackedComponents<
                         BiasElTy, M>::Value>::Value,
      __detail::TypeTraits<OutputElTy>::CompType>;

  BiasConvInterpPaddedTy BiasConvInterpPadded =
      Convert<__detail::TypeTraits<OutputElTy>::CompType, BiasElTy>(Bias);

  // Truncate the vector to the correct size M
  vector<OutputElTy, M> BiasConv =
      (vector<OutputElTy, M>)BiasConvInterpPadded.Data;

  vector<OutputElTy, M> Result;
  __builtin_LinAlg_MatrixVectorMultiplyAdd(
      Result, MatrixA.__handle, hlsl::is_signed<OutputElTy>::value, Vec,
      __detail::TypeTraits<InputElTy>::CompType, BiasConv);
  return Result;
}

template <typename OutputElTy, typename InputElTy, ComponentEnum InputInterp,
          ComponentEnum BiasElTy, SIZE_TYPE M, SIZE_TYPE K, SIZE_TYPE VecK,
          ComponentEnum MatrixDT>
typename hlsl::enable_if<
    VecK == __detail::ScalarCountFromPackedComponents<InputInterp, K>::Value,
    vector<OutputElTy, M> >::type
MultiplyAdd(Matrix<MatrixDT, M, K, MatrixUse::A, MatrixScope::Thread> MatrixA,
            InterpretedVector<InputElTy, VecK, InputInterp> InterpVec,
            VectorRef<BiasElTy, M> BiasRef) {
  using BiasVecTy =
      vector<typename __detail::ComponentTypeTraits<BiasElTy>::Type,
             __detail::ScalarCountFromPackedComponents<BiasElTy, M>::Value>;
  BiasVecTy Bias = BiasRef.Buf.template Load<BiasVecTy>(BiasRef.Offset);

  // FIXME: Convert currently does not support packed type vector sizes that
  // are not a multiple of the number of elements per scalar, so we
  // need to do an extra conversion here to get it into the right shape.
  // For example if BiasRef is F8_E4M3FN and M is 7, it gets loaded to into
  // vector<uint, 2>, and if OutputElTy is half, Convert will return
  // vector<half, 8> instead of vector<half, 7>.
  // https://github.com/microsoft/DirectXShaderCompiler/issues/8418

  // Convert to OutputElTy vector with padding
  using BiasConvInterpPaddedTy = InterpretedVector<
      OutputElTy,
      __detail::DstN<__detail::TypeTraits<OutputElTy>::CompType, BiasElTy,
                     __detail::ScalarCountFromPackedComponents<
                         BiasElTy, M>::Value>::Value,
      __detail::TypeTraits<OutputElTy>::CompType>;

  BiasConvInterpPaddedTy BiasConvInterpPadded =
      Convert<__detail::TypeTraits<OutputElTy>::CompType, BiasElTy>(Bias);

  // Truncate the vector to the correct size M
  vector<OutputElTy, M> BiasConv =
      (vector<OutputElTy, M>)BiasConvInterpPadded.Data;

  vector<OutputElTy, M> Result;
  __builtin_LinAlg_MatrixVectorMultiplyAdd(
      Result, MatrixA.__handle, hlsl::is_signed<OutputElTy>::value,
      InterpVec.Data, InterpVec.Interpretation, BiasConv);
  return Result;
}

// Outer product functions
template <ComponentEnum OutTy, typename InputElTy, SIZE_TYPE M, SIZE_TYPE N>
[[nodiscard]] typename hlsl::enable_if<
    hlsl::is_arithmetic<InputElTy>::value,
    Matrix<OutTy, M, N, MatrixUse::Accumulator, MatrixScope::Thread> >::type
OuterProduct(vector<InputElTy, M> VecA, vector<InputElTy, N> VecB) {
  Matrix<OutTy, M, N, MatrixUse::Accumulator, MatrixScope::Thread> Result;
  __builtin_LinAlg_MatrixOuterProduct(Result.__handle, VecA, VecB);
  return Result;
}

template <uint Align = 64, typename InputElTy, SIZE_TYPE M>
typename hlsl::enable_if<hlsl::is_arithmetic<InputElTy>::value, void>::type
InterlockedAccumulate(RWByteAddressBuffer Res, uint StartOffset,
                      vector<InputElTy, M> Vec) {
  __builtin_LinAlg_VectorAccumulateToDescriptor(Res, StartOffset, Align, Vec);
}

} // namespace linalg

} // namespace dx

#pragma dxc diagnostic pop

#endif // SM 6.10 check and HV version check
