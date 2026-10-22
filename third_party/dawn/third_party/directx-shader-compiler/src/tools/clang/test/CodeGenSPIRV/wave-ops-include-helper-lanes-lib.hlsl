// RUN: %dxc -T lib_6_7 -spirv %s | FileCheck %s

// CHECK: OpCapability QuadControlKHR
// CHECK-DAG: OpExtension "SPV_KHR_maximal_reconvergence"
// CHECK-DAG: OpExtension "SPV_KHR_quad_control"

// CHECK: OpEntryPoint Fragment %ps_main1 "ps_main1"
// CHECK: OpEntryPoint Fragment %ps_main2 "ps_main2"
// CHECK: OpEntryPoint Fragment %ps_main3 "ps_main3"

// CHECK-DAG: OpExecutionMode %ps_main1 MaximallyReconvergesKHR
// CHECK-DAG: OpExecutionMode %ps_main1 RequireFullQuadsKHR

// CHECK-NOT: OpExecutionMode %ps_main2 MaximallyReconvergesKHR
// CHECK-NOT: OpExecutionMode %ps_main2 RequireFullQuadsKHR

// CHECK-DAG: OpExecutionMode %ps_main3 MaximallyReconvergesKHR
// CHECK-DAG: OpExecutionMode %ps_main3 RequireFullQuadsKHR

[WaveOpsIncludeHelperLanes]
[shader("pixel")]
void ps_main1() : SV_Target0
{
}

[shader("pixel")]
void ps_main2() : SV_Target0
{
}

[WaveOpsIncludeHelperLanes]
[shader("pixel")]
void ps_main3() : SV_Target0
{
}
