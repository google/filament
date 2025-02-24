
cbuffer cbuffer_S : register(b0) {
  uint4 S[1];
};
float4 func() {
  return asfloat(S[0u]);
}

[numthreads(1, 1, 1)]
void main() {
  float4 r = func();
}

