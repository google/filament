// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure no phi of pointers
// CHECK-NOT: phi float*

static float a[4] = {1,2,3,4};
static float b[4] = {5,6,7,8};

float main(float xx : X, uint y : Y) : SV_Target {
  float c[4] = {xx, xx, xx, xx};
  float x = 0;
  if (xx > 2.5)
    x = a[y];
  else if (xx > 1.0)
    x = b[y];
  else
    x = c[y];

  float x2 = 0;
  if (xx < 1)
    x2 = b[y+1];
  else
    x2 = a[y-1];

  return x + y;

}