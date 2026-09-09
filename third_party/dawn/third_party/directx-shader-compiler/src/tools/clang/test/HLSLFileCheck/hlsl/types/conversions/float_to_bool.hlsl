// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: fcmp fast une
// CHECK: fcmp fast une

float test(bool2 x, float b) {
    if (x.x)
        return b;
    else
        return x.y;
}

float main(float2 a : A, float b : B) : SV_Target
{
  return test(a, b);
}
