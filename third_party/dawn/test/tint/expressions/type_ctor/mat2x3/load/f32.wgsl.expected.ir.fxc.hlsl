
RWByteAddressBuffer v : register(u0);
void v_1(uint offset, float2x3 obj) {
  v.Store3((offset + 0u), asuint(obj[0u]));
  v.Store3((offset + 16u), asuint(obj[1u]));
}

[numthreads(1, 1, 1)]
void f() {
  float2x3 m = float2x3((0.0f).xxx, (0.0f).xxx);
  v_1(0u, float2x3(m));
}

