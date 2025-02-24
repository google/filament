
cbuffer cbuffer_data : register(b0) {
  uint4 data[3];
};
float3x2 v(uint start_byte_offset) {
  uint4 v_1 = data[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint4 v_3 = data[((8u + start_byte_offset) / 16u)];
  float2 v_4 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_3.zw) : (v_3.xy)));
  uint4 v_5 = data[((16u + start_byte_offset) / 16u)];
  return float3x2(v_2, v_4, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_5.zw) : (v_5.xy))));
}

void main() {
  float3x2 v_6 = v(0u);
  float2 x = mul(asfloat(data[2u].xyz), v_6);
}

