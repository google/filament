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
  float3 v = (0.0f).xxx;
  uint v_1 = min(uniforms[0u].x, 7u);
  v[min(s1.a1[v_1], 2u)] = 1.0f;
  uint v_2 = min(uniforms[0u].x, 7u);
  v[min(f(s1.a1[v_2]), 2u)] = 1.0f;
}

