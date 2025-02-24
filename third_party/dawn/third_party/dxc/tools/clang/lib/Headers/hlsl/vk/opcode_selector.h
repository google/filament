// Copyright (c) 2024 Google LLC
//
// This file is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef _HLSL_VK_KHR_OPCODE_SELECTOR_H_
#define _HLSL_VK_KHR_OPCODE_SELECTOR_H_

#define DECLARE_UNARY_OP(name, opcode)                                         \
  template <typename ResultType>                                               \
  [[vk::ext_instruction(opcode)]] ResultType __builtin_spv_##name(             \
      ResultType a)

DECLARE_UNARY_OP(CopyObject, 83);
DECLARE_UNARY_OP(SNegate, 126);
DECLARE_UNARY_OP(FNegate, 127);

#define DECLARE_CONVERSION_OP(name, opcode)                                    \
  template <typename ResultType, typename OperandType>                         \
  [[vk::ext_instruction(opcode)]] ResultType __builtin_spv_##name(             \
      OperandType a)

DECLARE_CONVERSION_OP(ConvertFtoU, 109);
DECLARE_CONVERSION_OP(ConvertFtoS, 110);
DECLARE_CONVERSION_OP(ConvertSToF, 111);
DECLARE_CONVERSION_OP(ConvertUToF, 112);
DECLARE_CONVERSION_OP(UConvert, 113);
DECLARE_CONVERSION_OP(SConvert, 114);
DECLARE_CONVERSION_OP(FConvert, 115);
DECLARE_CONVERSION_OP(Bitcast, 124);

#undef DECLARY_UNARY_OP

#define DECLARE_BINOP(name, opcode)                                            \
  template <typename ResultType>                                               \
  [[vk::ext_instruction(opcode)]] ResultType __builtin_spv_##name(             \
      ResultType a, ResultType b)

DECLARE_BINOP(IAdd, 128);
DECLARE_BINOP(FAdd, 129);
DECLARE_BINOP(ISub, 130);
DECLARE_BINOP(FSub, 131);
DECLARE_BINOP(IMul, 132);
DECLARE_BINOP(FMul, 133);
DECLARE_BINOP(UDiv, 134);
DECLARE_BINOP(SDiv, 135);
DECLARE_BINOP(FDiv, 136);

#undef DECLARE_BINOP
namespace vk {
namespace util {

template <class ComponentType> class ArithmeticSelector;

#define ARITHMETIC_SELECTOR(BaseType, OpNegate, OpAdd, OpSub, OpMul, OpDiv,    \
                            SIGNED_INTEGER_TYPE)                               \
  template <> class ArithmeticSelector<BaseType> {                             \
    template <class T> static T Negate(T a) { return OpNegate(a); }            \
    template <class T> static T Add(T a, T b) { return OpAdd(a, b); }          \
    template <class T> static T Sub(T a, T b) { return OpSub(a, b); }          \
    template <class T> static T Mul(T a, T b) { return OpMul(a, b); }          \
    template <class T> static T Div(T a, T b) { return OpDiv(a, b); }          \
  };

ARITHMETIC_SELECTOR(half, __builtin_spv_FNegate, __builtin_spv_FAdd,
                    __builtin_spv_FSub, __builtin_spv_FMul, __builtin_spv_FDiv,
                    false);
ARITHMETIC_SELECTOR(float, __builtin_spv_FNegate, __builtin_spv_FAdd,
                    __builtin_spv_FSub, __builtin_spv_FMul, __builtin_spv_FDiv,
                    false);
ARITHMETIC_SELECTOR(double, __builtin_spv_FNegate, __builtin_spv_FAdd,
                    __builtin_spv_FSub, __builtin_spv_FMul, __builtin_spv_FDiv,
                    false);

#if __HLSL_ENABLE_16_BIT
ARITHMETIC_SELECTOR(int16_t, __builtin_spv_SNegate, __builtin_spv_IAdd,
                    __builtin_spv_ISub, __builtin_spv_IMul, __builtin_spv_SDiv,
                    true);
ARITHMETIC_SELECTOR(uint16_t, __builtin_spv_SNegate, __builtin_spv_IAdd,
                    __builtin_spv_ISub, __builtin_spv_IMul, __builtin_spv_UDiv,
                    false);
#endif // __HLSL_ENABLE_16_BIT

ARITHMETIC_SELECTOR(int32_t, __builtin_spv_SNegate, __builtin_spv_IAdd,
                    __builtin_spv_ISub, __builtin_spv_IMul, __builtin_spv_SDiv,
                    true);
ARITHMETIC_SELECTOR(int64_t, __builtin_spv_SNegate, __builtin_spv_IAdd,
                    __builtin_spv_ISub, __builtin_spv_IMul, __builtin_spv_SDiv,
                    true);
ARITHMETIC_SELECTOR(uint32_t, __builtin_spv_SNegate, __builtin_spv_IAdd,
                    __builtin_spv_ISub, __builtin_spv_IMul, __builtin_spv_UDiv,
                    false);
ARITHMETIC_SELECTOR(uint64_t, __builtin_spv_SNegate, __builtin_spv_IAdd,
                    __builtin_spv_ISub, __builtin_spv_IMul, __builtin_spv_UDiv,
                    false);

// The conversion selector is will be used to convert one type to another
// using the SPIR-V conversion instructions. See
// https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html#_conversion_instructions.
// SourceType and TargetType must be integer or floating point scalar type.

// ConversionSelector::Convert converts an object of type S to an object of type
// T. S must be SourceType, a vector of SourceType, or a cooperative matrix of
// SourceType. T must be TargetType, a vector of TargetType, or a cooperative
// matrix of TargetType. T must have the same number of components as S. T is a
// cooperative matrix if and only if S is a cooperative matrix.
template <class SourceType, class TargetType> class ConversionSelector;

#define CONVERSION_SELECTOR(SourceType, TargetType, OpConvert)                 \
  template <> class ConversionSelector<SourceType, TargetType> {               \
    template <class T, class S> static T Convert(S a) {                        \
      return OpConvert<T>(a);                                                  \
    }                                                                          \
  };

#if __HLSL_ENABLE_16_BIT
CONVERSION_SELECTOR(uint16_t, uint16_t, __builtin_spv_CopyObject);
CONVERSION_SELECTOR(uint16_t, int16_t, __builtin_spv_Bitcast);
CONVERSION_SELECTOR(uint16_t, uint32_t, __builtin_spv_UConvert);
CONVERSION_SELECTOR(uint16_t, int32_t, __builtin_spv_SConvert);
CONVERSION_SELECTOR(uint16_t, uint64_t, __builtin_spv_UConvert);
CONVERSION_SELECTOR(uint16_t, int64_t, __builtin_spv_SConvert);
CONVERSION_SELECTOR(uint16_t, half, __builtin_spv_ConvertUToF);
CONVERSION_SELECTOR(uint16_t, float, __builtin_spv_ConvertUToF);
CONVERSION_SELECTOR(uint16_t, double, __builtin_spv_ConvertUToF);

CONVERSION_SELECTOR(int16_t, uint16_t, __builtin_spv_Bitcast);
CONVERSION_SELECTOR(int16_t, int16_t, __builtin_spv_CopyObject);
CONVERSION_SELECTOR(int16_t, uint32_t, __builtin_spv_UConvert);
CONVERSION_SELECTOR(int16_t, int32_t, __builtin_spv_SConvert);
CONVERSION_SELECTOR(int16_t, uint64_t, __builtin_spv_UConvert);
CONVERSION_SELECTOR(int16_t, int64_t, __builtin_spv_SConvert);
CONVERSION_SELECTOR(int16_t, half, __builtin_spv_ConvertSToF);
CONVERSION_SELECTOR(int16_t, float, __builtin_spv_ConvertSToF);
CONVERSION_SELECTOR(int16_t, double, __builtin_spv_ConvertSToF);

CONVERSION_SELECTOR(uint32_t, uint16_t, __builtin_spv_UConvert);
CONVERSION_SELECTOR(uint32_t, int16_t, __builtin_spv_SConvert);

CONVERSION_SELECTOR(int32_t, uint16_t, __builtin_spv_UConvert);
CONVERSION_SELECTOR(int32_t, int16_t, __builtin_spv_SConvert);

CONVERSION_SELECTOR(uint64_t, uint16_t, __builtin_spv_UConvert);
CONVERSION_SELECTOR(uint64_t, int16_t, __builtin_spv_SConvert);

CONVERSION_SELECTOR(int64_t, uint16_t, __builtin_spv_UConvert);
CONVERSION_SELECTOR(int64_t, int16_t, __builtin_spv_SConvert);

CONVERSION_SELECTOR(half, uint16_t, __builtin_spv_ConvertFtoU);
CONVERSION_SELECTOR(half, int16_t, __builtin_spv_ConvertFtoS);

CONVERSION_SELECTOR(float, uint16_t, __builtin_spv_ConvertFtoU);
CONVERSION_SELECTOR(float, int16_t, __builtin_spv_ConvertFtoS);

CONVERSION_SELECTOR(double, uint16_t, __builtin_spv_ConvertFtoU);
CONVERSION_SELECTOR(double, int16_t, __builtin_spv_ConvertFtoS);
#endif

CONVERSION_SELECTOR(uint32_t, uint32_t, __builtin_spv_CopyObject);
CONVERSION_SELECTOR(uint32_t, int32_t, __builtin_spv_Bitcast);
CONVERSION_SELECTOR(uint32_t, uint64_t, __builtin_spv_UConvert);
CONVERSION_SELECTOR(uint32_t, int64_t, __builtin_spv_SConvert);
CONVERSION_SELECTOR(uint32_t, half, __builtin_spv_ConvertUToF);
CONVERSION_SELECTOR(uint32_t, float, __builtin_spv_ConvertUToF);
CONVERSION_SELECTOR(uint32_t, double, __builtin_spv_ConvertUToF);

CONVERSION_SELECTOR(int32_t, uint32_t, __builtin_spv_Bitcast);
CONVERSION_SELECTOR(int32_t, int32_t, __builtin_spv_CopyObject);
CONVERSION_SELECTOR(int32_t, uint64_t, __builtin_spv_UConvert);
CONVERSION_SELECTOR(int32_t, int64_t, __builtin_spv_SConvert);
CONVERSION_SELECTOR(int32_t, half, __builtin_spv_ConvertSToF);
CONVERSION_SELECTOR(int32_t, float, __builtin_spv_ConvertSToF);
CONVERSION_SELECTOR(int32_t, double, __builtin_spv_ConvertSToF);

CONVERSION_SELECTOR(uint64_t, uint32_t, __builtin_spv_UConvert);
CONVERSION_SELECTOR(uint64_t, int32_t, __builtin_spv_SConvert);
CONVERSION_SELECTOR(uint64_t, uint64_t, __builtin_spv_Bitcast);
CONVERSION_SELECTOR(uint64_t, int64_t, __builtin_spv_CopyObject);
CONVERSION_SELECTOR(uint64_t, half, __builtin_spv_ConvertUToF);
CONVERSION_SELECTOR(uint64_t, float, __builtin_spv_ConvertUToF);
CONVERSION_SELECTOR(uint64_t, double, __builtin_spv_ConvertUToF);

CONVERSION_SELECTOR(int64_t, uint32_t, __builtin_spv_UConvert);
CONVERSION_SELECTOR(int64_t, int32_t, __builtin_spv_SConvert);
CONVERSION_SELECTOR(int64_t, uint64_t, __builtin_spv_Bitcast);
CONVERSION_SELECTOR(int64_t, int64_t, __builtin_spv_CopyObject);
CONVERSION_SELECTOR(int64_t, half, __builtin_spv_ConvertSToF);
CONVERSION_SELECTOR(int64_t, float, __builtin_spv_ConvertSToF);
CONVERSION_SELECTOR(int64_t, double, __builtin_spv_ConvertSToF);

CONVERSION_SELECTOR(half, uint32_t, __builtin_spv_ConvertFtoU);
CONVERSION_SELECTOR(half, int32_t, __builtin_spv_ConvertFtoS);
CONVERSION_SELECTOR(half, uint64_t, __builtin_spv_ConvertFtoU);
CONVERSION_SELECTOR(half, int64_t, __builtin_spv_ConvertFtoS);
CONVERSION_SELECTOR(half, half, __builtin_spv_CopyObject);
#if __HLSL_ENABLE_16_BIT
CONVERSION_SELECTOR(half, float, __builtin_spv_FConvert);
#else
CONVERSION_SELECTOR(half, float, __builtin_spv_CopyObject);
#endif

CONVERSION_SELECTOR(half, double, __builtin_spv_FConvert);

CONVERSION_SELECTOR(float, uint32_t, __builtin_spv_ConvertFtoU);
CONVERSION_SELECTOR(float, int32_t, __builtin_spv_ConvertFtoS);
CONVERSION_SELECTOR(float, uint64_t, __builtin_spv_ConvertFtoU);
CONVERSION_SELECTOR(float, int64_t, __builtin_spv_ConvertFtoS);
#if __HLSL_ENABLE_16_BIT
CONVERSION_SELECTOR(float, half, __builtin_spv_FConvert);
#else
CONVERSION_SELECTOR(float, half, __builtin_spv_CopyObject);
#endif
CONVERSION_SELECTOR(float, float, __builtin_spv_CopyObject);
CONVERSION_SELECTOR(float, double, __builtin_spv_FConvert);

CONVERSION_SELECTOR(double, uint32_t, __builtin_spv_ConvertFtoU);
CONVERSION_SELECTOR(double, int32_t, __builtin_spv_ConvertFtoS);
CONVERSION_SELECTOR(double, uint64_t, __builtin_spv_ConvertFtoU);
CONVERSION_SELECTOR(double, int64_t, __builtin_spv_ConvertFtoS);
CONVERSION_SELECTOR(double, half, __builtin_spv_FConvert);
CONVERSION_SELECTOR(double, float, __builtin_spv_FConvert);
CONVERSION_SELECTOR(double, double, __builtin_spv_CopyObject);
}; // namespace util
} // namespace vk

#endif // _HLSL_VK_KHR_OPCODE_SELECTOR_H_
