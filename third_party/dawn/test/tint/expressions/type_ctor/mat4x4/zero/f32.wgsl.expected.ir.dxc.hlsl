
static float4x4 m = float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
RWByteAddressBuffer v : register(u0);
void v_1(uint offset, float4x4 obj) {
  v.Store4((offset + 0u), asuint(obj[0u]));
  v.Store4((offset + 16u), asuint(obj[1u]));
  v.Store4((offset + 32u), asuint(obj[2u]));
  v.Store4((offset + 48u), asuint(obj[3u]));
}

[numthreads(1, 1, 1)]
void f() {
  v_1(0u, m);
}

