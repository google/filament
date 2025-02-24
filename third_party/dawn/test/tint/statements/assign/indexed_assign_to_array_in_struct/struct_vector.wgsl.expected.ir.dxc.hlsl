struct OuterS {
  float3 v1;
};


cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
[numthreads(1, 1, 1)]
void main() {
  OuterS s1 = (OuterS)0;
  s1.v1[min(uniforms[0u].x, 2u)] = 1.0f;
}

