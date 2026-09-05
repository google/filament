// RUN: %dxc -T cs_6_6 -E main -fspv-extension=SPV_KHR_compute_shader_derivatives -fcgl  %s -spirv  2>&1 | FileCheck %s

// CHECK: OpCapability ComputeDerivativeGroupQuadsKHR
// CHECK: OpExtension "SPV_KHR_compute_shader_derivatives"
// CHECK: OpExecutionMode %main DerivativeGroupQuadsKHR


SamplerState ss : register(s2);
SamplerComparisonState scs;

RWStructuredBuffer<uint> o;
Texture1D        <float>  t1;

[numthreads(2,2,1)]
void main(uint3 id : SV_GroupThreadID)
{
    // CHECK: OpDPdx %float %float_0_5
    o[0] = ddx(0.5);
    // CHECK: OpDPdxCoarse %float %float_0_5
    o[1] = ddx_coarse(0.5);
    // CHECK: OpDPdy %float %float_0_5
    o[2] = ddy(0.5);
    // CHECK: OpDPdyCoarse %float %float_0_5
    o[3] = ddy_coarse(0.5);
    // CHECK: OpDPdxFine %float %float_0_5
    o[4] = ddx_fine(0.5);
    // CHECK: OpDPdyFine %float %float_0_5
    o[5] = ddy_fine(0.5);
}