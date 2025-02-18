
RWByteAddressBuffer v : register(u0);
void v_1(uint offset, float2x3 obj) {
  v.Store3((offset + 0u), asuint(obj[0u]));
  v.Store3((offset + 16u), asuint(obj[1u]));
}

[numthreads(1, 1, 1)]
void f() {
  v_1(0u, float2x3(float3(0.0f, 1.0f, 2.0f), float3(3.0f, 4.0f, 5.0f)));
}

