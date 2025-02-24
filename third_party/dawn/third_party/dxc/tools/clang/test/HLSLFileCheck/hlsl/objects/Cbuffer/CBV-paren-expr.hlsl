// RUN: %dxc -E main -T ps_6_0  %s  | FileCheck %s

// Make sure ParenExpr compiles properly.
// CHECK:extractvalue %dx.types.CBufRet.f32 %{{.*}}, 0

struct S {
  float a;
  float b;
};

ConstantBuffer<S> c[2]: register(b2, space5);

export ConstantBuffer<S> getCBV(uint i) {
  return c[i];
}

float fn(S s) {
  return s.a;
}

export float useCBV(ConstantBuffer<S> cbv) {
  return fn(cbv);
}

float main(uint4 i : IN1, uint4 j : IN2) : SV_Target {
  float result = 0.0;
  result += fn(c[i.x]);
  result += fn((c[i.y]));
  result += fn(((getCBV(i.z))));
  result += useCBV(c[j.x]);
  result += useCBV((c[j.y]));
  result += useCBV(getCBV(j.z));
  result += useCBV((getCBV(j.w)));
  return result;
}
