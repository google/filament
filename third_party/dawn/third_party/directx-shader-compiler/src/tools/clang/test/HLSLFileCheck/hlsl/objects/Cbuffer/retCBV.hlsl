// RUN: %dxc -E main -T ps_6_0  %s  | FileCheck %s

// Make sure both a and b are used.
// CHECK:extractvalue %dx.types.CBufRet.f32 %{{.*}}, 0
// CHECK:extractvalue %dx.types.CBufRet.f32 %{{.*}}, 1
struct S {
  float a;
  float b;
};

ConstantBuffer<S> c: register(b2, space5);;

S getS() {
  return c;
}

float main() : SV_Target {
  return c.a + getS().b;
}