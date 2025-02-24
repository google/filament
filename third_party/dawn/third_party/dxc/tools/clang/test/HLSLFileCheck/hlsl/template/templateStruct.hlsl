// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s | FileCheck %s

// CHECK:define void @main

template<typename T>
struct TS {
  T t;
};

struct TS<float4> ts;

float4 main() : SV_Target {
  return ts.t;
}
