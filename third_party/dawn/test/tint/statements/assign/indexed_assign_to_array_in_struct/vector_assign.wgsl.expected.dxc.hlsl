void set_vector_element(inout float3 vec, int idx, float val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

struct OuterS {
  uint a1[8];
};

cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};

uint f(uint i) {
  return (i + 1u);
}

[numthreads(1, 1, 1)]
void main() {
  OuterS s1 = (OuterS)0;
  float3 v = float3(0.0f, 0.0f, 0.0f);
  set_vector_element(v, min(s1.a1[min(uniforms[0].x, 7u)], 2u), 1.0f);
  uint tint_symbol = f(s1.a1[min(uniforms[0].x, 7u)]);
  set_vector_element(v, min(tint_symbol, 2u), 1.0f);
  return;
}
