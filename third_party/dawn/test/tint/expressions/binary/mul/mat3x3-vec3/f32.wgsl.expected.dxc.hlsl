cbuffer cbuffer_data : register(b0) {
  uint4 data[4];
};

float3x3 data_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  return float3x3(asfloat(data[scalar_offset / 4].xyz), asfloat(data[scalar_offset_1 / 4].xyz), asfloat(data[scalar_offset_2 / 4].xyz));
}

void main() {
  float3 x = mul(asfloat(data[3].xyz), data_load(0u));
  return;
}
