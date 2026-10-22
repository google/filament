// RUN: %dxc -Emain -Tps_6_0 %s | %opt -S -hlsl-dxil-constantColor,constant-red=1 | %FileCheck %s

// Check that we overrode output color (0.0 becomes 1.0):
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 1.000000e+00)

// Check output depth wasn't affected (0.0 stays 0.0):
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 1, i32 0, i8 0, float 0.000000e+00)

struct PSOutput
{
  float color : SV_Target;
  float depth : SV_Depth;
};
PSOutput main() : SV_Target {
  PSOutput Output;
  Output.color = 0.f;
  Output.depth = 0.f;
  return Output;
}
