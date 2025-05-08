// RUN: %dxc -T cs_6_6 -E main -fspv-extension=SPV_NV_compute_shader_derivatives -fcgl  %s -spirv  2>&1 | FileCheck %s
// RUN: %dxc -T cs_6_6 -E main -fspv-extension=SPV_NV_compute_shader_derivatives %s -spirv  2>&1 | FileCheck %s

// CHECK: OpCapability ComputeDerivativeGroupQuadsKHR
// CHECK: OpExtension "SPV_NV_compute_shader_derivatives"
// CHECK: OpExecutionMode %main DerivativeGroupQuadsKHR

SamplerState ss : register(s2);
SamplerComparisonState scs;

RWStructuredBuffer<uint> o;
Texture1D        <float>  t1;

[numthreads(4,24,1)]
void main(uint3 id : SV_GroupThreadID)
{
    //CHECK:          [[t1:%[0-9]+]] = OpLoad %type_1d_image %t1
    //CHECK-NEXT:    [[ss1:%[0-9]+]] = OpLoad %type_sampler %ss
    //CHECK-NEXT:    [[si1:%[0-9]+]] = OpSampledImage %type_sampled_image [[t1]] [[ss1]]
    //CHECK-NEXT: [[query1:%[0-9]+]] = OpImageQueryLod %v2float [[si1]] %float_0_5
    //CHECK-NEXT:        {{%[0-9]+}} = OpCompositeExtract %float [[query1]] 1
    o[0] = t1.CalculateLevelOfDetailUnclamped(ss, 0.5);
}
