// RUN: %dxc -T ms_6_5 -E main -fspv-target-env=vulkan1.3 %s -spirv | FileCheck %s --check-prefix=VK13
// RUN: %dxc -T ms_6_5 -E main -fspv-target-env=vulkan1.1 -Vd %s -spirv | FileCheck %s --check-prefix=VK11

// VK13-DAG: OpCapability ComputeDerivativeGroupLinearKHR
// VK13-DAG: OpCapability DerivativeControl
// vk13-DAG: OpCapability MeshShadingEXT
// VK13-DAG: OpExtension "SPV_EXT_mesh_shader"
// VK13-DAG: OpExtension "SPV_KHR_compute_shader_derivatives"
// VK13: OpEntryPoint MeshEXT %main "main"
// VK13: OpExecutionMode %main DerivativeGroupLinearKHR

// VK11-DAG: OpExtension "SPV_NV_mesh_shader"
// VK11: OpEntryPoint MeshNV %main "main"
// VK11-NOT: OpExecutionMode %main DerivativeGroup

struct VSOut
{
    float4 pos : SV_Position;
};

[numthreads(4, 1, 1)]
[outputtopology("triangle")]
void main(in uint tid : SV_GroupThreadID, out vertices VSOut verts[3], out indices uint3 tris[1])
{
    SetMeshOutputCounts(3, 1);

    float4 val = ddx_coarse(float4(tid, 0, 0, 0));

    verts[0].pos = val;
    verts[1].pos = val + float4(0,1,0,0);
    verts[2].pos = val + float4(1,0,0,0);

    tris[0] = uint3(0,1,2);
}
