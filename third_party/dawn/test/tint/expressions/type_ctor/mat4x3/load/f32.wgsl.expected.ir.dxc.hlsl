
RWByteAddressBuffer v : register(u0);
void v_1(uint offset, float4x3 obj) {
  v.Store3((offset + 0u), asuint(obj[0u]));
  v.Store3((offset + 16u), asuint(obj[1u]));
  v.Store3((offset + 32u), asuint(obj[2u]));
  v.Store3((offset + 48u), asuint(obj[3u]));
}

[numthreads(1, 1, 1)]
void f() {
  float4x3 m = float4x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
  v_1(0u, float4x3(m));
}

