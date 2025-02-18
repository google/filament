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
  uint v_2 = s1.a1[v_1];
  float3 v_3 = v;
  float3 v_4 = float3((1.0f).xxx);
  float3 v_5 = float3((v_2).xxx);
  v = (((v_5 == float3(int(0), int(1), int(2)))) ? (v_4) : (v_3));
  uint v_6 = min(uniforms[0u].x, 7u);
  uint v_7 = f(s1.a1[v_6]);
  float3 v_8 = v;
  float3 v_9 = float3((1.0f).xxx);
  float3 v_10 = float3((v_7).xxx);
  v = (((v_10 == float3(int(0), int(1), int(2)))) ? (v_9) : (v_8));
}

