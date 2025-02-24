struct InnerS {
  int v;
};
struct OuterS {
  InnerS a1[8];
};

cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};

[numthreads(1, 1, 1)]
void main() {
  InnerS v = (InnerS)0;
  OuterS s1 = (OuterS)0;
  uint p_save = uniforms[0].x;
  {
    InnerS tint_symbol_1[8] = s1.a1;
    tint_symbol_1[min(p_save, 7u)] = v;
    s1.a1 = tint_symbol_1;
  }
  return;
}
