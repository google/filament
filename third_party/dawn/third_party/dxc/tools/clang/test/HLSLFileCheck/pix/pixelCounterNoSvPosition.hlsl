// RUN: %dxc -Emain -Tps_6_0 %s | %opt -S -hlsl-dxil-add-pixel-hit-instrmentation,rt-width=16,num-pixels=64 | %FileCheck %s

// Check the read from SV_Position was added:
// CHECK: %XPos = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 undef)
// CHECK: %YPos = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1, i32 undef)

// Check the SV_Position meta-data was added:
// CHECK: !{i32 0, !"SV_Position", i8 9, i8 3,

float4 main() : SV_Target{
  discard;
  return float4(0,0,0,0);
}

