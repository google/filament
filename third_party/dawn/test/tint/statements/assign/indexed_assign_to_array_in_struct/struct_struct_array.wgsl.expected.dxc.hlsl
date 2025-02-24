struct InnerS {
  int v;
};
struct S1 {
  InnerS a[8];
};
struct OuterS {
  S1 s2;
};

cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};

[numthreads(1, 1, 1)]
void main() {
  InnerS v = (InnerS)0;
  OuterS s1 = (OuterS)0;
  {
    InnerS tint_symbol_1[8] = s1.s2.a;
    tint_symbol_1[min(uniforms[0].x, 7u)] = v;
    s1.s2.a = tint_symbol_1;
  }
  return;
}
