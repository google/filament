// RUN: %dxc -Emain -Tps_6_0 %s | %opt -S -hlsl-dxil-debug-instrumentation,UAVSize=1024 | %FileCheck %s

// Check that the UAV size is reflected in the instrumentation.
// The AND should be (1024/4-1), and the or should be 1024/2:

// CHECK: %PIXOffsetOr = phi i32 [ 0, %PIXInterestingBlock ], [ 512, %PIXNonInterestingBlock ]
// CHECK: and i32 {{.*}}, 255

[RootSignature("")]
float4 main() : SV_Target {
    return float4(0,0,0,0);
}