struct InnerS {
  int v;
};
struct OuterS {
  InnerS a1[8];
};

cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};

void f(inout OuterS p) {
  InnerS v = (InnerS)0;
  {
    InnerS tint_symbol_1[8] = p.a1;
    tint_symbol_1[min(uniforms[0].x, 7u)] = v;
    p.a1 = tint_symbol_1;
  }
}

[numthreads(1, 1, 1)]
void main() {
  OuterS s1 = (OuterS)0;
  f(s1);
  return;
}
