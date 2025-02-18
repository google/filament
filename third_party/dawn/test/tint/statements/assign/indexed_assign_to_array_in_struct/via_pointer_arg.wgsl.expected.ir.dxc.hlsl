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
  uint v_1 = min(uniforms[0u].x, 7u);
  InnerS v_2 = v;
  p.a1[v_1] = v_2;
}

[numthreads(1, 1, 1)]
void main() {
  OuterS s1 = (OuterS)0;
  f(s1);
}

