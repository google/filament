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
  uint v_1 = uniforms[0u].x;
  InnerS tint_array_copy[8] = s1.a1;
  InnerS v_2 = v;
  tint_array_copy[min(v_1, 7u)] = v_2;
  InnerS v_3[8] = tint_array_copy;
  s1.a1 = v_3;
  uint v_4 = uniforms[0u].x;
  InnerS tint_array_copy_1[8] = s1.a2;
  InnerS v_5 = v;
  tint_array_copy_1[min(v_4, 7u)] = v_5;
  InnerS v_6[8] = tint_array_copy_1;
  s1.a2 = v_6;
}

