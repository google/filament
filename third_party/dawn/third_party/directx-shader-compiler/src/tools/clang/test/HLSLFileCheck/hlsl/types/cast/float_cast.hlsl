// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Result should be 63.
// CHECK: float 6.300000e+01


static const float x = 15;
static const float ar[] = { 18, 23 };

float main(float2 a : A) : SV_Target {
  float lar[4];
  (float[2])lar = ar;
  return 3.0f * (float)x + lar[0];
}