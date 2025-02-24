
RWByteAddressBuffer v : register(u0);
void v_1(uint offset, float2x2 obj) {
  v.Store2((offset + 0u), asuint(obj[0u]));
  v.Store2((offset + 8u), asuint(obj[1u]));
}

[numthreads(1, 1, 1)]
void f() {
  v_1(0u, float2x2(float2(0.0f, 1.0f), float2(2.0f, 3.0f)));
}

