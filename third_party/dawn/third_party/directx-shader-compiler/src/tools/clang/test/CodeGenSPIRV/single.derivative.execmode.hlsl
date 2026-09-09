// RUN: %dxc -T cs_6_6 -E main -fspv-extension=SPV_NV_compute_shader_derivatives -fcgl  %s -spirv  2>&1 | FileCheck %s

// This test checks that the execution mode is not added multiple times. Other
// tests will verify that the code generation is correct.

// CHECK: OpCapability ComputeDerivativeGroupQuadsKHR
// CHECK: OpExtension "SPV_NV_compute_shader_derivatives"
// CHECK: OpExecutionMode %main DerivativeGroupQuadsKHR
// CHECK-NOT: OpExecutionMode %main DerivativeGroupQuadsKHR

SamplerState ss : register(s2);
SamplerComparisonState scs;

RWStructuredBuffer<uint> o;
Texture1D        <float>  t1;

[numthreads(2,2,1)]
void main(uint3 id : SV_GroupThreadID)
{
    uint v = id.x;
    o[0] = t1.CalculateLevelOfDetail(ss, 0.5);
    o[1] = t1.CalculateLevelOfDetailUnclamped(ss, 0.5);
    o[2] = t1.Sample(ss, 1);
    o[3] = t1.SampleBias(ss, 1, 0.5);
    o[4] = t1.SampleCmp(scs, 1, 0.5);
}