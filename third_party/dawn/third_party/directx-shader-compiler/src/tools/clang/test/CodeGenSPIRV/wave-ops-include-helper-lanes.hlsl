// RUN: %dxc -T ps_6_7 -E main -spirv %s | FileCheck %s

// CHECK: OpCapability QuadControlKHR
// CHECK-DAG: OpExtension "SPV_KHR_maximal_reconvergence"
// CHECK-DAG: OpExtension "SPV_KHR_quad_control"

// CHECK: OpExecutionMode %main MaximallyReconvergesKHR
// CHECK: OpExecutionMode %main RequireFullQuadsKHR

[WaveOpsIncludeHelperLanes]
float4 main(float4 pos : SV_Position) : SV_Target
{
    return pos;
}
