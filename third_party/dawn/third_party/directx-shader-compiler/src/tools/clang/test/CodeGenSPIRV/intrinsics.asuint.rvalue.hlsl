// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

StructuredBuffer<float4> input0;
StructuredBuffer<float4x3> input1;

[numthreads(64,1,1)]
void main(uint3 dtID : SV_DispatchThreadID)
{
// CHECK: [[ptr_input0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_v4float %input0 %int_0 %int_0
// CHECK:     [[input0:%[0-9]+]] = OpLoad %v4float [[ptr_input0]]
// CHECK:                       OpBitcast %v4uint [[input0]]
  uint4 val0 = asuint(input0.Load(0));

// CHECK: [[ptr_input1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_mat4v3float %input1 %int_0 %int_0
// CHECK:     [[input1:%[0-9]+]] = OpLoad %mat4v3float [[ptr_input1]]
// CHECK:   [[input1_0:%[0-9]+]] = OpCompositeExtract %v3float [[input1]] 0
// CHECK:                       OpBitcast %v3uint [[input1_0]]
// CHECK:                       OpCompositeExtract %v3float {{%[0-9]+}} 1
// CHECK:                       OpBitcast %v3uint
// CHECK:                       OpCompositeExtract %v3float {{%[0-9]+}} 2
// CHECK:                       OpBitcast %v3uint
// CHECK:                       OpCompositeExtract %v3float {{%[0-9]+}} 3
// CHECK:                       OpBitcast %v3uint
  uint4x3 val1 = asuint(input1.Load(0));
}
