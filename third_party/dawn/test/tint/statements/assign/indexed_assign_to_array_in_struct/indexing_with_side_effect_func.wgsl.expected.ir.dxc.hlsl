struct InnerS {
  int v;
};

struct S1 {
  InnerS a2[8];
};

struct OuterS {
  S1 a1[8];
};


static uint nextIndex = 0u;
cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
uint getNextIndex() {
  nextIndex = (nextIndex + 1u);
  return nextIndex;
}

[numthreads(1, 1, 1)]
void main() {
  InnerS v = (InnerS)0;
  OuterS s = (OuterS)0;
  uint v_1 = getNextIndex();
  uint v_2 = min(uniforms[0u].y, 7u);
  InnerS v_3 = v;
  s.a1[min(v_1, 7u)].a2[v_2] = v_3;
}

