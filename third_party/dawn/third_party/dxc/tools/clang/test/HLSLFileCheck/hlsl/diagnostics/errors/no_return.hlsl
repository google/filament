// RUN: %dxc -E main -T vs_6_2 %s | FileCheck %s

// CHECK: 8:1: error: control reaches end of non-void function

export float4 fn() {
  float4 f = 1.0;
  f += 2.0;
} // return-type error here.

float4 main() : OUT {
  return fn();
}
