// RUN: %dxc -E main -T ps_6_2 -HV 2018 %s | FileCheck %s

// Make sure no branch.
// CHECK-NOT:br i1

float4 c;
float x;
float y;
float z;


float main() : SV_Target {

float  v = c.y> 0.00001 ?
    (c.y >= 0.2 ? (x+2) : y) : z;

return v;
}
