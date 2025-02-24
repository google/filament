cbuffer cbuffer_S : register(b0) {
  uint4 S[2];
};

float4 func_S_X(uint pointer[1]) {
  const uint scalar_offset = ((16u * pointer[0])) / 4;
  return asfloat(S[scalar_offset / 4]);
}

[numthreads(1, 1, 1)]
void main() {
  uint tint_symbol[1] = {1u};
  float4 r = func_S_X(tint_symbol);
  return;
}
