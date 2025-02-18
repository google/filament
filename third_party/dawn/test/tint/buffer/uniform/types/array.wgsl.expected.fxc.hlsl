cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};

typedef float4 tint_symbol_ret[4];
tint_symbol_ret tint_symbol(uint4 buffer[4], uint offset) {
  float4 arr[4] = (float4[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      const uint scalar_offset = ((offset + (i * 16u))) / 4;
      arr[i] = asfloat(buffer[scalar_offset / 4]);
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void main() {
  const float4 x[4] = tint_symbol(u, 0u);
  return;
}
