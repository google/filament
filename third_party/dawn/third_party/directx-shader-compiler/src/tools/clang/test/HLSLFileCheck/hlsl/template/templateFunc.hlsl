// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s | FileCheck %s

// CHECK:define void @main

template<typename T>
T foo(T t0, T t1) {
  return sin(t0) * cos(t1);
}

float2 main(float4 a:A) : SV_Target {
  return foo(a.x, a.y) + foo(a.xy, a.zw);
}
