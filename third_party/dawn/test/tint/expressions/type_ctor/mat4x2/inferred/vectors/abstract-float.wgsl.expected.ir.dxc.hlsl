
RWByteAddressBuffer v : register(u0);
void v_1(uint offset, float4x2 obj) {
  v.Store2((offset + 0u), asuint(obj[0u]));
  v.Store2((offset + 8u), asuint(obj[1u]));
  v.Store2((offset + 16u), asuint(obj[2u]));
  v.Store2((offset + 24u), asuint(obj[3u]));
}

[numthreads(1, 1, 1)]
void f() {
  v_1(0u, float4x2(float2(0.0f, 1.0f), float2(2.0f, 3.0f), float2(4.0f, 5.0f), float2(6.0f, 7.0f)));
}

