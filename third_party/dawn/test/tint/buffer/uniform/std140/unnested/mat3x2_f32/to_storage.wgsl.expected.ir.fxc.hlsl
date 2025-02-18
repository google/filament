
cbuffer cbuffer_u : register(b0) {
  uint4 u[2];
};
RWByteAddressBuffer s : register(u1);
void v(uint offset, float3x2 obj) {
  s.Store2((offset + 0u), asuint(obj[0u]));
  s.Store2((offset + 8u), asuint(obj[1u]));
  s.Store2((offset + 16u), asuint(obj[2u]));
}

float3x2 v_1(uint start_byte_offset) {
  uint4 v_2 = u[(start_byte_offset / 16u)];
  float2 v_3 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_2.zw) : (v_2.xy)));
  uint4 v_4 = u[((8u + start_byte_offset) / 16u)];
  float2 v_5 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_4.zw) : (v_4.xy)));
  uint4 v_6 = u[((16u + start_byte_offset) / 16u)];
  return float3x2(v_3, v_5, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_6.zw) : (v_6.xy))));
}

[numthreads(1, 1, 1)]
void f() {
  v(0u, v_1(0u));
  s.Store2(8u, asuint(asfloat(u[0u].xy)));
  s.Store2(8u, asuint(asfloat(u[0u].xy).yx));
  s.Store(4u, asuint(asfloat(u[0u].z)));
}

