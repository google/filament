// RUN: %dxc -Emain -Tps_6_0 %s | %opt -S -hlsl-dxil-add-pixel-hit-instrmentation,rt-width=16,num-pixels=64 | %FileCheck %s

// Check that the input semantic was read correctly:
// CHECK: %XPos = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 undef)
// CHECK: %YPos = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 1, i32 undef)

// The cast-to-int:
// CHECK: %XIndex = fptoui float %XPos to i32
// CHECK: %YIndex = fptoui float %YPos to i32

// Calculation of offset:
// CHECK: = mul i32 %YIndex, 16
// CHECK: = add i32 %XIndex,

// Check the write to the UAV was emitted:
// CHECK: %UAVIncResult = call i32 @dx.op.atomicBinOp.i32(i32 78, %dx.types.Handle %PIX_CountUAV_Handle, i32 0, i32 %ByteIndex, i32 undef, i32 undef, i32 1)

float4 main(float4 pos : SV_Position) : SV_Target {
  return pos;
}
