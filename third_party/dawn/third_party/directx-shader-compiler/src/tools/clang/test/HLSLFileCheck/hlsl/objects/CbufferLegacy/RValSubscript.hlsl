// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK-DAG: alloca [16 x i32]
// CHECK-DAG: alloca [4 x i1]

// For b4[2]
// CHECK: cbufferLoadLegacy
// CHECK: i32 5)
// CHECK: extractvalue
// CHECK: , 2
// CHECK: icmp ne
// CHECK 0

// For (x4 < 3)[1]
// CHECK: cbufferLoadLegacy
// CHECK: i32 4)
// CHECK: extractvalue
// CHECK: , 1
// CHECK: fcmp fast olt
// CHECK: 3.000000e+00

// For (xt == 0)[0][0]
// CHECK: cbufferLoadLegacy
// CHECK: i32 0)
// CHECK: extractvalue
// CHECK:, 0
// CHECK: fcmp fast oeq

// For (x4 < i)[i]
// CHECK: fcmp fast olt
// CHECK: fcmp fast olt
// CHECK: fcmp fast olt
// CHECK: fcmp fast olt

// For (xt == 0)[i][i]
// CHECK: fcmp fast oeq
// CHECK: fcmp fast oeq
// CHECK: fcmp fast oeq
// CHECK: fcmp fast oeq
// CHECK: fcmp fast oeq
// CHECK: fcmp fast oeq
// CHECK: fcmp fast oeq
// CHECK: fcmp fast oeq
// CHECK: fcmp fast oeq
// CHECK: fcmp fast oeq
// CHECK: fcmp fast oeq
// CHECK: fcmp fast oeq
// CHECK: fcmp fast oeq
// CHECK: fcmp fast oeq
// CHECK: fcmp fast oeq
// CHECK: fcmp fast oeq


float4x4 xt;
float4 x4;

bool4 b4;
uint i;

float4 main(uint4 a : A) : SV_TARGET
{
  uint x = b4[2] + (x4 < 3)[1] + (xt == 0)[0][0];
  x += (x4 < i)[i];
  x += (xt == 6)[i][i];
  return x;
}
