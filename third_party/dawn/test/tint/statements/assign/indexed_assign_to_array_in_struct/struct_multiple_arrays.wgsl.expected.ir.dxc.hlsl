struct InnerS {
  int v;
};

struct OuterS {
  InnerS a1[8];
  InnerS a2[8];
};


cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
[numthreads(1, 1, 1)]
void main() {
  InnerS v = (InnerS)0;
  OuterS s1 = (OuterS)0;
  uint v_1 = min(uniforms[0u].x, 7u);
  InnerS v_2 = v;
  s1.a1[v_1] = v_2;
  uint v_3 = min(uniforms[0u].x, 7u);
  InnerS v_4 = v;
  s1.a2[v_3] = v_4;
}

