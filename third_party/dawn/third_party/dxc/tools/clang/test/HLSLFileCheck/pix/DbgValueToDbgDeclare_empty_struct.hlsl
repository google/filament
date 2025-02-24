// RUN: %dxc -Tps_6_0 /Od /Zi %s | %opt -S -dxil-dbg-value-to-dbg-declare

struct Empty {
};
struct S {
  float bar;
  Empty empty;
  float foo;
};

[RootSignature("")]
float main(float a : A) : SV_Target {
  S s;
  s.bar = a + 2;
  s.foo = a * 2;
  return s.foo / s.bar; 
}

