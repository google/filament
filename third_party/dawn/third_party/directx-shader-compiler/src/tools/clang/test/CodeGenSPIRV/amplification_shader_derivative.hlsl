// RUN: %dxc -T as_6_5 -E main -fspv-target-env=vulkan1.3 %s -spirv | FileCheck %s --check-prefix=VK13
// RUN: %dxc -T as_6_5 -E main -fspv-target-env=vulkan1.1 -Vd %s -spirv | FileCheck %s --check-prefix=VK11

// VK13-DAG: OpCapability ComputeDerivativeGroupLinearKHR
// VK13-DAG: OpCapability DerivativeControl
// VK13-DAG: OpCapability MeshShadingEXT
// VK13-DAG: OpExtension "SPV_EXT_mesh_shader"
// VK13-DAG: OpExtension "SPV_KHR_compute_shader_derivatives"
// VK13: OpEntryPoint TaskEXT %main "main"
// VK13: OpExecutionMode %main DerivativeGroupLinearKHR

// VK11-DAG: OpExtension "SPV_NV_mesh_shader"
// VK11: OpEntryPoint TaskNV %main "main"
// VK11-NOT: OpExecutionMode %main DerivativeGroup

struct AmplificationPayload
{
    float4 value;
};

groupshared AmplificationPayload payload;

[numthreads(4, 1, 1)]
void main(in uint tid : SV_GroupThreadID, in uint gtid : SV_GroupID)
{
    payload.value = ddx_coarse(float4(tid, 0, 0, 0));
    DispatchMesh(1,1,1, payload);
}
