
cbuffer cbuffer_data : register(b0) {
  uint4 data[5];
};
float4x3 v(uint start_byte_offset) {
  return float4x3(asfloat(data[(start_byte_offset / 16u)].xyz), asfloat(data[((16u + start_byte_offset) / 16u)].xyz), asfloat(data[((32u + start_byte_offset) / 16u)].xyz), asfloat(data[((48u + start_byte_offset) / 16u)].xyz));
}

void main() {
  float3 v_1 = asfloat(data[4u].xyz);
  float4 x = mul(v(0u), v_1);
}

