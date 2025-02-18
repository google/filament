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
  uint v_2 = uniforms[0u].y;
  S1 tint_array_copy[8] = s.a1;
  InnerS v_3 = v;
  tint_array_copy[min(v_1, 7u)].a2[min(v_2, 7u)] = v_3;
  S1 v_4[8] = tint_array_copy;
  s.a1 = v_4;
}

