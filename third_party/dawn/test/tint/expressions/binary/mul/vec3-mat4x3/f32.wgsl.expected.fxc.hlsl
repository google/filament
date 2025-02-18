cbuffer cbuffer_data : register(b0) {
  uint4 data[5];
};

float4x3 data_load_1(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x3(asfloat(data[scalar_offset / 4].xyz), asfloat(data[scalar_offset_1 / 4].xyz), asfloat(data[scalar_offset_2 / 4].xyz), asfloat(data[scalar_offset_3 / 4].xyz));
}

void main() {
  float4 x = mul(data_load_1(0u), asfloat(data[4].xyz));
  return;
}
