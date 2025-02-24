
cbuffer cbuffer_u : register(b0) {
  uint4 u[3];
};
RWByteAddressBuffer s : register(u1);
void v(uint offset, float3x4 obj) {
  s.Store4((offset + 0u), asuint(obj[0u]));
  s.Store4((offset + 16u), asuint(obj[1u]));
  s.Store4((offset + 32u), asuint(obj[2u]));
}

float3x4 v_1(uint start_byte_offset) {
  return float3x4(asfloat(u[(start_byte_offset / 16u)]), asfloat(u[((16u + start_byte_offset) / 16u)]), asfloat(u[((32u + start_byte_offset) / 16u)]));
}

[numthreads(1, 1, 1)]
void f() {
  v(0u, v_1(0u));
  s.Store4(16u, asuint(asfloat(u[0u])));
  s.Store4(16u, asuint(asfloat(u[0u]).ywxz));
  s.Store(4u, asuint(asfloat(u[1u].x)));
}

