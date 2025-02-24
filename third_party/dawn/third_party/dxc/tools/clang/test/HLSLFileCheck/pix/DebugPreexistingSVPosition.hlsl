// RUN: %dxc -Emain -Tps_6_0 %s | %opt -S -hlsl-dxil-debug-instrumentation | %FileCheck %s

// Check that the basic SV_Position check is present:

// Check we added the UAV:                                                                      v----metadata position: not important for this check
// CHECK: %PIX_DebugUAV_Handle = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 [[S:[0-9]+]], i32 0, i1 false)
// CHECK: %XPos = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 undef)
// CHECK: %YPos = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1, i32 undef)
// CHECK: %XIndex = fptoui float %XPos to i32
// CHECK: %YIndex = fptoui float %YPos to i32
// CHECK: %CompareToX = icmp eq i32 %XIndex, 0
// CHECK: %CompareToY = icmp eq i32 %YIndex, 0
// CHECK: %ComparePos = and i1 %CompareToX, %CompareToY


[RootSignature("")]
float4 main(float4 pos : SV_Position) : SV_Target {
    return pos;
}