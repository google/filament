
RWByteAddressBuffer v : register(u0);
void v_1(uint offset, float3x2 obj) {
  v.Store2((offset + 0u), asuint(obj[0u]));
  v.Store2((offset + 8u), asuint(obj[1u]));
  v.Store2((offset + 16u), asuint(obj[2u]));
}

[numthreads(1, 1, 1)]
void f() {
  float3x2 m = float3x2((0.0f).xx, (0.0f).xx, (0.0f).xx);
  v_1(0u, float3x2(m));
}

