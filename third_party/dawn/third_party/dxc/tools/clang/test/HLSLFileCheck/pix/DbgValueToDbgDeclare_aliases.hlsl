// RUN: %dxc -Tps_6_0 /Od /Zi %s | %opt -S -dxil-dbg-value-to-dbg-declare

void foo(inout float2 v) {
  v.x *= 2;
  v.y *= 2;
}

void foo(inout float v[2]) {
  v[0] *= 2;
  v[1] *= 2;
}

[RootSignature("")]
float main() : SV_Target {
  float v[2] = {0.0, 1.0};
  float2 u = float2(1, 2);
  foo(v);
  foo(u);
  return v[0] + v[1] + u.x + u.y;
}

