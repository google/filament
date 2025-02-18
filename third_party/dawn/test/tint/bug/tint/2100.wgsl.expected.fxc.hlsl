cbuffer cbuffer_buffer : register(b0) {
  uint4 buffer[7];
};

struct tint_symbol {
  float4 value : SV_Position;
};

float4 main_inner() {
  float x = asfloat(buffer[0].z);
  return float4(x, 0.0f, 0.0f, 1.0f);
}

tint_symbol main() {
  float4 inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
