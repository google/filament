struct OuterS {
  float2x4 m1;
};


cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
[numthreads(1, 1, 1)]
void main() {
  OuterS s1 = (OuterS)0;
  uint v = min(uniforms[0u].x, 1u);
  s1.m1[v] = (1.0f).xxxx;
  uint v_1 = min(uniforms[0u].x, 1u);
  s1.m1[v_1][min(uniforms[0u].x, 3u)] = 1.0f;
}

