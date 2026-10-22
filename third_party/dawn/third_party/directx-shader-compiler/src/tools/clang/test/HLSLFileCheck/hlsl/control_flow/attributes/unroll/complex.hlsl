// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s
// CHECK: @main

uint g_cond[3];
uint g_bound;

float main() : SV_Target {

  float foo = 10;

  [unroll]
  for (uint i = 0; i < 4; i++) {
    
    if (i == g_cond[0]) {
      foo += 100;
      break;
    }
    else if (i == g_cond[1]) {
      foo += 200;
      break;
    }
    else if (i == g_cond[2]) { 
      return 10;
    }
    foo++;
  }

  if (foo > 300) {
    foo /= 2;
  }

  return foo;
}
