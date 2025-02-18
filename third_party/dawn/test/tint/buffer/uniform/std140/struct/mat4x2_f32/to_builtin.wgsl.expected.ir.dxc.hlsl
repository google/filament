
cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
float4x2 v(uint start_byte_offset) {
  uint4 v_1 = u[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint4 v_3 = u[((8u + start_byte_offset) / 16u)];
  float2 v_4 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_3.zw) : (v_3.xy)));
  uint4 v_5 = u[((16u + start_byte_offset) / 16u)];
  float2 v_6 = asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_5.zw) : (v_5.xy)));
  uint4 v_7 = u[((24u + start_byte_offset) / 16u)];
  return float4x2(v_2, v_4, v_6, asfloat(((((((24u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_7.zw) : (v_7.xy))));
}

[numthreads(1, 1, 1)]
void f() {
  float2x4 t = transpose(v(264u));
  float l = length(asfloat(u[1u].xy).yx);
  float a = abs(asfloat(u[1u].xy).yx.x);
}

