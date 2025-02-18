struct InnerS {
  int v;
};

struct S1 {
  InnerS s2;
};

struct OuterS {
  S1 a1[8];
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
  s1.a1[v_1].s2 = v_2;
}

