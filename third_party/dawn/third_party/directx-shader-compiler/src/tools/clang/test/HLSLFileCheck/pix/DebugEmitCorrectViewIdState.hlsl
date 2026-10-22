// RUN: %dxc -Emain -Tvs_6_0 %s | %opt -S -hlsl-dxil-debug-instrumentation,parameter0=1,parameter1=2 -viewid-state -hlsl-dxilemit | %FileCheck %s

// CHECK: !dx.viewIdState = !{![[VIEWIDDATA:[0-9]*]]}

// The debug instrumentation will have added SV_InstanceId and SV_VertexId to the input signature for this VS.
// If view id state is correct, then this entry should have expanded to 7 i32s (previously it would have been 2)
// CHECK: ![[VIEWIDDATA]] = !{[7 x i32]


[RootSignature("")]
float4 main() : SV_Position{
  return float4(0,0,0,0);
}