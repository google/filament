
cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
RWByteAddressBuffer s : register(u1);
void v(uint offset, float2x2 obj) {
  s.Store2((offset + 0u), asuint(obj[0u]));
  s.Store2((offset + 8u), asuint(obj[1u]));
}

float2x2 v_1(uint start_byte_offset) {
  uint4 v_2 = u[(start_byte_offset / 16u)];
  float2 v_3 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_2.zw) : (v_2.xy)));
  uint4 v_4 = u[((8u + start_byte_offset) / 16u)];
  return float2x2(v_3, asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_4.zw) : (v_4.xy))));
}

[numthreads(1, 1, 1)]
void main() {
  float2x2 x = v_1(0u);
  v(0u, x);
}

