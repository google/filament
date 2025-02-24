// RUN: %dxc -T ps_6_0 -E main -O0 %s -spirv | FileCheck %s

Texture1D   <float4> t : register(t1);
SamplerState gSampler : register(s2);

// CHECK: OpCapability MinLod

// TODO: add a rasterizer order view and check that these are not trimmed
//       once ROVs are supported
// CHECK-NOT: OpCapability FragmentShaderSampleInterlockEXT
// CHECK-NOT: OpCapability FragmentShaderPixelInterlockEXT
// CHECK-NOT: OpCapability FragmentShaderShadingRateInterlockEXT

// CHECK-NOT: OpExtension "SPV_EXT_fragment_shader_interlock"
float4 main(uint clamp : A) : SV_Target {
    if (clamp < 0) {
      return t.Sample(gSampler, 0.5f, 2, float(clamp));
    }
    return float4(0, 0, 0, 0);
}
