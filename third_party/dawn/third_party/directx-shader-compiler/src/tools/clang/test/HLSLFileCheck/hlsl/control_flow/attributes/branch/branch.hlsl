// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s
// Make sure if is not flattened.
// CHECK:br
// CHECK:dx.controlflow.hints

float c;
float main(float2 a:A) : SV_Target {
    float x = a.x;
    [branch]
    if (c > 2)
      x = x + 1;
    else
      x = x *3;
    return x;
}