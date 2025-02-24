
RWByteAddressBuffer v : register(u0);
void v_1(uint offset, float2x2 obj) {
  v.Store2((offset + 0u), asuint(obj[0u]));
  v.Store2((offset + 8u), asuint(obj[1u]));
}

[numthreads(1, 1, 1)]
void f() {
  float2x2 m = float2x2((0.0f).xx, (0.0f).xx);
  v_1(0u, float2x2(m));
}

