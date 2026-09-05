// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: call float @dx.op.dot3
// CHECK: call float @dx.op.dot3
// CHECK: call float @dx.op.dot3
// CHECK: call float @dx.op.dot3

// CHECK: call float @dx.op.dot3
// CHECK: call float @dx.op.dot3
// CHECK: call float @dx.op.dot3
// CHECK: call float @dx.op.dot3

// CHECK-NOT: call float @dx.op.dot3

uint gc[4];
uint g_bound;

float main(float3 a : A, float3 b : B) : SV_Target {

  float foo = 10;

  [unroll]
  for (uint i = 1; i < 3; i++) {
    
    if (i == gc[0]) {
      foo += dot(a*gc[0], b/gc[0]);
      continue;
    }
    else if (i == gc[1]) {
      foo += dot(a*gc[1], b/gc[1]);
      continue;
    }
    else if (i == gc[2]) { 
      foo += dot(a*gc[2], b/gc[2]);
      if (foo > g_bound)
        return foo;
      continue;
    }
    else if (i == gc[3]) { 
      foo += dot(a*gc[3], b/gc[3]);
      continue;
    }
    foo++;
  }

  if (foo > 300) {
    foo /= 2;
  }

  return foo;
}
