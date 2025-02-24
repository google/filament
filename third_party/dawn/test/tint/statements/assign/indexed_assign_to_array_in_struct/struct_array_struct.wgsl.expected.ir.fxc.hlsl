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
  uint v_1 = uniforms[0u].x;
  S1 tint_array_copy[8] = s1.a1;
  InnerS v_2 = v;
  tint_array_copy[min(v_1, 7u)].s2 = v_2;
  S1 v_3[8] = tint_array_copy;
  s1.a1 = v_3;
}

