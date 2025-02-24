cbuffer cbuffer_S : register(b0) {
  uint4 S[1];
};

float2 func_S_X(uint pointer[1]) {
  const uint scalar_offset = ((8u * pointer[0])) / 4;
  uint4 ubo_load = S[scalar_offset / 4];
  return asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy));
}

[numthreads(1, 1, 1)]
void main() {
  uint tint_symbol[1] = {1u};
  float2 r = func_S_X(tint_symbol);
  return;
}
