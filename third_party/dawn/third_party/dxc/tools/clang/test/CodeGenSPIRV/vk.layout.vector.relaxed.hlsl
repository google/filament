// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

    // For ConstantBuffer & cbuffer
// CHECK: OpMemberDecorate %S 0 Offset 0
// CHECK: OpMemberDecorate %S 1 Offset 4
// CHECK: OpMemberDecorate %S 2 Offset 16
// CHECK: OpMemberDecorate %S 3 Offset 28
// CHECK: OpMemberDecorate %S 4 Offset 32
// CHECK: OpMemberDecorate %S 5 Offset 36
// CHECK: OpMemberDecorate %S 6 Offset 44
// CHECK: OpMemberDecorate %S 7 Offset 48
// CHECK: OpMemberDecorate %S 8 Offset 56
// CHECK: OpMemberDecorate %S 9 Offset 64
// CHECK: OpMemberDecorate %S 10 Offset 80
// CHECK: OpMemberDecorate %S 11 Offset 92
// CHECK: OpMemberDecorate %S 12 Offset 96
// CHECK: OpMemberDecorate %S 13 Offset 112
// CHECK: OpMemberDecorate %S 14 Offset 128
// CHECK: OpMemberDecorate %S 15 Offset 140
// CHECK: OpMemberDecorate %S 16 Offset 144
// CHECK: OpMemberDecorate %S 17 Offset 160
// CHECK: OpMemberDecorate %S 18 Offset 176
// CHECK: OpMemberDecorate %S 19 Offset 192
// CHECK: OpMemberDecorate %S 20 Offset 208
// CHECK: OpMemberDecorate %S 21 Offset 240
// CHECK: OpMemberDecorate %S 22 Offset 272
// CHECK: OpMemberDecorate %S 23 Offset 304

    // For StructuredBuffer & tbuffer
// CHECK: OpMemberDecorate %S_0 0 Offset 0
// CHECK: OpMemberDecorate %S_0 1 Offset 4
// CHECK: OpMemberDecorate %S_0 2 Offset 16
// CHECK: OpMemberDecorate %S_0 3 Offset 28
// CHECK: OpMemberDecorate %S_0 4 Offset 32
// CHECK: OpMemberDecorate %S_0 5 Offset 36
// CHECK: OpMemberDecorate %S_0 6 Offset 44
// CHECK: OpMemberDecorate %S_0 7 Offset 48
// CHECK: OpMemberDecorate %S_0 8 Offset 56
// CHECK: OpMemberDecorate %S_0 9 Offset 64
// CHECK: OpMemberDecorate %S_0 10 Offset 80
// CHECK: OpMemberDecorate %S_0 11 Offset 92
// CHECK: OpMemberDecorate %S_0 12 Offset 96
// CHECK: OpMemberDecorate %S_0 13 Offset 112
// CHECK: OpMemberDecorate %S_0 14 Offset 128
// CHECK: OpMemberDecorate %S_0 15 Offset 140
// CHECK: OpMemberDecorate %S_0 16 Offset 144
// CHECK: OpMemberDecorate %S_0 17 Offset 160
// CHECK: OpMemberDecorate %S_0 18 Offset 176
// CHECK: OpMemberDecorate %S_0 19 Offset 192
// CHECK: OpMemberDecorate %S_0 20 Offset 196
// CHECK: OpMemberDecorate %S_0 21 Offset 208
// CHECK: OpMemberDecorate %S_0 22 Offset 240
// CHECK: OpMemberDecorate %S_0 23 Offset 272

// CHECK: OpDecorate %_runtimearr_T ArrayStride 288

// CHECK:     %type_ConstantBuffer_T = OpTypeStruct %S
// CHECK:                         %T = OpTypeStruct %S_0
// CHECK:   %type_StructuredBuffer_T = OpTypeStruct %_runtimearr_T
// CHECK: %type_RWStructuredBuffer_T = OpTypeStruct %_runtimearr_T
// CHECK:              %type_CBuffer = OpTypeStruct %S
// CHECK:              %type_TBuffer = OpTypeStruct %S_0

// CHECK:   %MyCBuffer = OpVariable %_ptr_Uniform_type_ConstantBuffer_T Uniform
// CHECK:   %MySBuffer = OpVariable %_ptr_Uniform_type_StructuredBuffer_T Uniform
// CHECK: %MyRWSBuffer = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_T Uniform
// CHECK:     %CBuffer = OpVariable %_ptr_Uniform_type_CBuffer Uniform
// CHECK:     %TBuffer = OpVariable %_ptr_Uniform_type_TBuffer Uniform

struct S {
    float  f0;
    float3 f1;

    float3 f2;
    float1 f3;

    float  f4;
    float2 f5;
    float1 f6;

    float2 f7;
    float2 f8;

    float2 f9;
    float3 f10;
    float  f11;

    float1 f12;
    float4 f13;
    float3 f14;
    float  f15;

    float1 f16[1];
    float3 f17[1];

    float3 f18[1];
    float  f19[1];

    float1 f20[2];
    float3 f21[2];

    float3 f22[2];
    float  f23[2];
};

struct T {
    S s;
};


    ConstantBuffer<T> MyCBuffer;
  StructuredBuffer<T> MySBuffer;
RWStructuredBuffer<T> MyRWSBuffer;

cbuffer CBuffer {
    S CB_s;
};

tbuffer TBuffer {
    S TB_s;
};

float4 main() : SV_Target {
    return MyCBuffer.s.f0 + MySBuffer[0].s.f4 + CB_s.f11 + TB_s.f15;
}
