// RUN: %dxc -Emain -Tps_6_0 %s | %opt -S -hlsl-dxil-debug-instrumentation,parameter0=1,parameter1=2 -viewid-state -hlsl-dxilemit | %FileCheck %s

// CHECK: !dx.viewIdState = !{![[VIEWIDDATA:[0-9]*]]}

// The debug instrumentation will have added SV_Position to the input signature for this PS.
// If view id state is correct, then this entry should have expanded to 6 i32s (previously it would have been 4)
// CHECK: ![[VIEWIDDATA]] = !{[6 x i32]


struct VS_OUTPUT {
  float2 Tex : TEXCOORD0;
};

float4 main(VS_OUTPUT input) : SV_Target {
  return input.Tex.xyxy;
}