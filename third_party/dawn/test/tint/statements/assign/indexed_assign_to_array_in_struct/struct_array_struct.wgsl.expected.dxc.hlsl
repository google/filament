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
  {
    S1 tint_symbol_1[8] = s1.a1;
    tint_symbol_1[min(uniforms[0].x, 7u)].s2 = v;
    s1.a1 = tint_symbol_1;
  }
  return;
}
