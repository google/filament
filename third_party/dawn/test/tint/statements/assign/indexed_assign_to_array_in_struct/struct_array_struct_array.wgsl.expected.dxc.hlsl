struct InnerS {
  int v;
};
struct S1 {
  InnerS a2[8];
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
  OuterS s = (OuterS)0;
  {
    S1 tint_symbol_1[8] = s.a1;
    uint tint_symbol_2_save = min(uniforms[0].x, 7u);
    InnerS tint_symbol_3[8] = tint_symbol_1[tint_symbol_2_save].a2;
    tint_symbol_3[min(uniforms[0].y, 7u)] = v;
    tint_symbol_1[tint_symbol_2_save].a2 = tint_symbol_3;
    s.a1 = tint_symbol_1;
  }
  return;
}
