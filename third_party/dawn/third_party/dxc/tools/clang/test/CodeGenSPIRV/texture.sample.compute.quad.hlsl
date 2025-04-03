// RUN: %dxc -T cs_6_6 -E main -fspv-extension=SPV_NV_compute_shader_derivatives -fcgl  %s -spirv  2>&1 | FileCheck %s
// RUN: %dxc -T cs_6_6 -E main -fspv-extension=SPV_NV_compute_shader_derivatives %s -spirv  2>&1 | FileCheck %s

// CHECK: OpCapability ComputeDerivativeGroupQuadsKHR
// CHECK: OpExtension "SPV_NV_compute_shader_derivatives"
// CHECK: OpExecutionMode %main DerivativeGroupQuadsKHR

SamplerState ss : register(s2);
SamplerComparisonState scs;

RWStructuredBuffer<uint> o;
Texture1D        <float>  t1;

[numthreads(2,2,1)]
void main(uint3 id : SV_GroupThreadID)
{
    // CHECK:              [[t1:%[0-9]+]] = OpLoad %type_1d_image %t1
    // CHECK-NEXT:   [[ss:%[0-9]+]] = OpLoad %type_sampler %ss
    // CHECK-NEXT: [[sampledImg:%[0-9]+]] = OpSampledImage %type_sampled_image [[t1]] [[ss]]
    // CHECK-NEXT:            {{%[0-9]+}} = OpImageSampleImplicitLod %v4float [[sampledImg]] %float_1 None
    o[0] = t1.Sample(ss, 1);
}
