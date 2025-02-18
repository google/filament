struct main_outputs {
  float4 tint_symbol : SV_Position;
};


cbuffer cbuffer_buffer : register(b0) {
  uint4 buffer[7];
};
float4 main_inner() {
  float x = asfloat(buffer[0u].z);
  return float4(x, 0.0f, 0.0f, 1.0f);
}

main_outputs main() {
  main_outputs v = {main_inner()};
  return v;
}

