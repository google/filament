// RUN: %dxc -Emain -Tvs_6_0 %s | %opt -S -hlsl-dxil-debug-instrumentation | %FileCheck %s

// Check that the SV_InstanceId check is present:

// Check we added the UAV:                                                                      v----metadata position: not important for this check
// CHECK: %PIX_DebugUAV_Handle = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 [[S:[0-9]+]], i32 0, i1 false)
// CHECK: %VertId = call i32 @dx.op.loadInput.i32(i32 4, i32 1, i32 0, i8 0, i32 undef)
// CHECK: %InstanceId = call i32 @dx.op.loadInput.i32(i32 4, i32 0, i32 0, i8 0, i32 undef)
// CHECK: %CompareToVertId = icmp eq i32 %VertId, 0
// CHECK: %CompareToInstanceId = icmp eq i32 %InstanceId, 0
// CHECK: %CompareBoth = and i1 %CompareToVertId, %CompareToInstanceId

[RootSignature("")]
float4 main(uint id : SV_InstanceId) : SV_Position{
    return float4(id,0,0,0);
}