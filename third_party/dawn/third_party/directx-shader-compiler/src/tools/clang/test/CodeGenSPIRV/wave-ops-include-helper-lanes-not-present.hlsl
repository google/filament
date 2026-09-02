// RUN: %dxc -T vs_6_7 -E main -spirv %s | FileCheck %s
//
// CHECK-NOT: OpCapability QuadControlKHR
// CHECK-NOT: OpExtension "SPV_KHR_maximal_reconvergence"
// CHECK-NOT: OpExtension "SPV_KHR_quad_control"
//
// CHECK-NOT: OpExecutionMode %main MaximallyReconvergesKHR
// CHECK-NOT: OpExecutionMode %main RequireFullQuadsKHR

[WaveOpsIncludeHelperLanes]
[shader("vertex")]
float4 main() : SV_Position
{
    return float4(0.0, 0.0, 0.0, 0.0);
}
