// RUN: %dxc -Od -E main -T lib_6_x %s | FileCheck %s

// Regression test for when there are multiple functions with loops that can't
// be unrolled at the point of doing [unroll], the unroll pass crashes because
// it didn't clean up memory between runs.

// CHECK: pass_reuse_regression.hlsl:{{[0-9]+}}:{{[0-9]+}}: error: Could not unroll loop
// CHECK: pass_reuse_regression.hlsl:{{[0-9]+}}:{{[0-9]+}}: error: Could not unroll loop

AppendStructuredBuffer<float4> buf0;
uint g_cond;

struct Params {
  int foo;
};

float f(Params p) {
  [unroll]
  for (uint j = 0; j < p.foo; j++) {
    buf0.Append(1);
  }
  return 0;
}

float g(Params p) {
  [unroll]
  for (uint j = 0; j < p.foo; j++) {
    buf0.Append(1);
  }
  return 0;
}

float main() : SV_Target {
  Params p;
  p.foo = g_cond;
  return f(p) + g(p);
}

