struct S {
  int data[64];
};


cbuffer cbuffer_ubo : register(b0) {
  uint4 ubo[1];
};
RWByteAddressBuffer result : register(u1);
void x(inout S p) {
  int v = asint(ubo[0u].x);
  uint v_1 = min(uint(v), 63u);
  int tint_array_copy[64] = p.data;
  uint v_2 = min(uint(v), 63u);
  tint_array_copy[v_2] = int(1);
  int v_3[64] = tint_array_copy;
  p.data = v_3;
}

[numthreads(1, 1, 1)]
void f() {
  S s = (S)0;
  x(s);
  result.Store(0u, asuint(s.data[3u]));
}

