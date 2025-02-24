
cbuffer cbuffer_S : register(b0) {
  uint4 S[1];
};
float2 func(uint pointer_indices[1]) {
  uint4 v = S[((8u * pointer_indices[0u]) / 16u)];
  return asfloat(((((((8u * pointer_indices[0u]) % 16u) / 4u) == 2u)) ? (v.zw) : (v.xy)));
}

[numthreads(1, 1, 1)]
void main() {
  uint v_1[1] = {1u};
  float2 r = func(v_1);
}

