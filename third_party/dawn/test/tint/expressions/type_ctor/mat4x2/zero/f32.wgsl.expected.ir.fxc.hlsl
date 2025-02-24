
static float4x2 m = float4x2((0.0f).xx, (0.0f).xx, (0.0f).xx, (0.0f).xx);
RWByteAddressBuffer v : register(u0);
void v_1(uint offset, float4x2 obj) {
  v.Store2((offset + 0u), asuint(obj[0u]));
  v.Store2((offset + 8u), asuint(obj[1u]));
  v.Store2((offset + 16u), asuint(obj[2u]));
  v.Store2((offset + 24u), asuint(obj[3u]));
}

[numthreads(1, 1, 1)]
void f() {
  v_1(0u, m);
}

