
RWByteAddressBuffer v : register(u0);
void v_1(uint offset, float4x4 obj) {
  v.Store4((offset + 0u), asuint(obj[0u]));
  v.Store4((offset + 16u), asuint(obj[1u]));
  v.Store4((offset + 32u), asuint(obj[2u]));
  v.Store4((offset + 48u), asuint(obj[3u]));
}

[numthreads(1, 1, 1)]
void f() {
  v_1(0u, float4x4(float4(0.0f, 1.0f, 2.0f, 3.0f), float4(4.0f, 5.0f, 6.0f, 7.0f), float4(8.0f, 9.0f, 10.0f, 11.0f), float4(12.0f, 13.0f, 14.0f, 15.0f)));
}

