
static float3x3 m = float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
RWByteAddressBuffer v : register(u0);
void v_1(uint offset, float3x3 obj) {
  v.Store3((offset + 0u), asuint(obj[0u]));
  v.Store3((offset + 16u), asuint(obj[1u]));
  v.Store3((offset + 32u), asuint(obj[2u]));
}

[numthreads(1, 1, 1)]
void f() {
  v_1(0u, m);
}

