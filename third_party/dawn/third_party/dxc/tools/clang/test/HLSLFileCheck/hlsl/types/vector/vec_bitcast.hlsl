// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// f will be casted to vector<float,1> for f.x.
// And because f is global, so bitcast on f is Constant and shared between the 2 f.x.
// SimplifyBitCast will run on both load of f.x if we don't take care it.

// CHECK: @main
static float f;

float4 main() : SV_Target
{
  float f1 = f.x;

  float f2 = f.x;
  return f1 + f2;
}
