// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s
// Make sure if is flattened.
// CHECK-NOT:br
// CHECK-NOT:dx.controlflow.hints

float c;
float main(float2 a:A) : SV_Target {
    float x = a.x;
    [flatten]
    if (c > 2)
      x = x + 1;
    else
      x = x *3;
    return x;
}