// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s | FileCheck %s

// CHECK:define void @main

struct Test {

template<typename T>
T foo(T t) {
  return sin(t);
}

};

float2 main(float4 a:A) : SV_Target {
  Test t0;
  Test t1;
  return t0.foo<float>(a.y) + t1.foo<float2>(a.zw);
}
