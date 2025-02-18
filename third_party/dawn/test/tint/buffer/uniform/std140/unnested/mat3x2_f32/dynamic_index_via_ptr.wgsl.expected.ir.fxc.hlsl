
cbuffer cbuffer_m : register(b0) {
  uint4 m[2];
};
static int counter = int(0);
int i() {
  counter = (counter + int(1));
  return counter;
}

float3x2 v(uint start_byte_offset) {
  uint4 v_1 = m[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint4 v_3 = m[((8u + start_byte_offset) / 16u)];
  float2 v_4 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_3.zw) : (v_3.xy)));
  uint4 v_5 = m[((16u + start_byte_offset) / 16u)];
  return float3x2(v_2, v_4, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_5.zw) : (v_5.xy))));
}

[numthreads(1, 1, 1)]
void f() {
  uint v_6 = (8u * min(uint(i()), 2u));
  float3x2 l_m = v(0u);
  uint4 v_7 = m[(v_6 / 16u)];
  float2 l_m_i = asfloat((((((v_6 % 16u) / 4u) == 2u)) ? (v_7.zw) : (v_7.xy)));
}

