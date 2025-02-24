// RUN: %dxc -E ps -T ps_6_0 %s | FileCheck %s

// Make sure we don't initialize @dx.ishelper with coverage when IsHelperLane is not used, but discard is.
// CHECK-NOT: call i32 @dx.op.coverage.i32

float4 a;

[shader("pixel")]
float4 ps(float f : IN): SV_Target
{
  if (f < 0.0)
    discard;
  float4 result = a;
  return ddx(result);
}
