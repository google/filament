// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpName %type__Globals "type.$Globals"

// CHECK: OpMemberName %type__Globals 0 "gScalar"
// CHECK: OpMemberName %type__Globals 1 "gVec"
// CHECK: OpMemberName %type__Globals 2 "gMat1"
// CHECK: OpMemberName %type__Globals 3 "gMat2"
// CHECK: OpMemberName %type__Globals 4 "gArray"
// CHECK: OpMemberName %type__Globals 5 "gStruct"
// CHECK: OpMemberName %type__Globals 6 "gAnonStruct"

// CHECK: OpName %_Globals "$Globals"

// CHECK: OpDecorate %_Globals DescriptorSet 0
// CHECK: OpDecorate %_Globals Binding 0

    // Check that resources have their bindings
// CHECK: OpDecorate %gSampler Binding 1
// CHECK: OpDecorate %gTex Binding 2
// CHECK: OpDecorate %gSBuffer Binding 3
// CHECK: OpDecorate %MyCBuffer Binding 4
// CHECK: OpDecorate %gCBuffer Binding 5
// CHECK: OpDecorate %gTBuffer Binding 6
// CHECK: OpDecorate %MyTBuffer Binding 7
// CHECK: OpDecorate %gSPInput Binding 8
// CHECK: OpDecorate %gRWBuffer DescriptorSet 0
// CHECK: OpDecorate %gRWBuffer Binding 9

// CHECK: OpMemberDecorate %type__Globals 0 Offset 0
// CHECK: OpMemberDecorate %type__Globals 1 Offset 4
// CHECK: OpMemberDecorate %type__Globals 2 Offset 16
// CHECK: OpMemberDecorate %type__Globals 2 MatrixStride 16
// CHECK: OpMemberDecorate %type__Globals 2 RowMajor
// CHECK: OpMemberDecorate %type__Globals 3 Offset 64
// CHECK: OpMemberDecorate %type__Globals 3 MatrixStride 16
// CHECK: OpMemberDecorate %type__Globals 3 ColMajor
// CHECK: OpMemberDecorate %type__Globals 4 Offset 96
// CHECK: OpMemberDecorate %type__Globals 4 MatrixStride 16
// CHECK: OpMemberDecorate %type__Globals 4 ColMajor
// CHECK: OpMemberDecorate %type__Globals 5 Offset 160
// CHECK: OpMemberDecorate %type__Globals 6 Offset 176
// CHECK: OpDecorate %type__Globals Block

          int           gScalar;   // 0
          SamplerState  gSampler;  // Not included - 1
          float2        gVec;      // 1
          Texture2D     gTex;      // Not included - 2
          float2x3      gMat1;     // 2
row_major float2x3      gMat2;     // 3

StructuredBuffer<float> gSBuffer;  // Not included - 3

row_major float2x3      gArray[2]; // 4

struct S {
    float f;
};

          S             gStruct;   // 5

cbuffer MyCBuffer {                // Not included - 4
    float4 CBData[16];
};

ConstantBuffer<S>       gCBuffer;  // Not included - 5
TextureBuffer<S>        gTBuffer;  // Not included - 6

typedef SamplerState SamplerStateType; // Not included - type definition

// CHECK: [[v2f_struct:%[a-zA-Z0-9_]+]] = OpTypeStruct %v2float
struct {
    float2 f;
}                       gAnonStruct; // 6

tbuffer MyTBuffer {                // Not included - 7
    float4 TBData[16];
};

[[vk::input_attachment_index(0)]]
SubpassInput            gSPInput;  // Not included - 8

RWBuffer<float4>        gRWBuffer[4]; // Not included - 9 (array)

// CHECK: %type__Globals = OpTypeStruct %int %v2float %mat2v3float %mat2v3float %_arr_mat2v3float_uint_2 %S [[v2f_struct]]
// CHECK: %_ptr_Uniform_type__Globals = OpTypePointer Uniform %type__Globals

// %_Globals = OpVariable %_ptr_Uniform_type__Globals Uniform

float4 main() : SV_Target {

// CHECK: OpAccessChain %_ptr_Uniform_int %_Globals %int_0
// CHECK: OpAccessChain %_ptr_Uniform_mat2v3float %_Globals %int_3
// CHECK: OpAccessChain %_ptr_Uniform_S %_Globals %int_5
    return gScalar + gMat2[0][0] + gStruct.f;
}
