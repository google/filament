// RUN: %dxc -T vs_6_0 -E main -DCOND=false -fspv-target-env=vulkan1.3 %s -spirv | FileCheck %s
// CHECK-NOT: OpCapability DerivativeControl
// CHECK-NOT: OpExtension "SPV_KHR_compute_shader_derivatives"

// RUN: not %dxc -T vs_6_0 -E main -DCOND=true -fspv-target-env=vulkan1.3 %s -spirv 2>&1 | FileCheck %s -check-prefix=ERROR
// ERROR: generated SPIR-V is invalid:
// ERROR-NEXT: Derivative instructions require Fragment, GLCompute, MeshEXT or TaskEXT execution model: DPdx

struct VSOut
{
    float4 pos : SV_Position;
};

VSOut main(float4 pos : POSITION)
{
    VSOut output;
    output.pos = pos;
    if (COND)
    {
        output.pos += ddx(pos);
    }
    return output;
}
