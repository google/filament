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
  int i = int(0);
  {
    uint2 tint_loop_idx = (0u).xx;
    uint v_1 = uniforms[0u].x;
    InnerS tint_array_copy[8] = s1.a1;
    InnerS v_2 = v;
    tint_array_copy[min(v_1, 7u)] = v_2;
    InnerS v_3[8] = tint_array_copy;
    s1.a1 = v_3;
    while(true) {
      if (all((tint_loop_idx == (4294967295u).xx))) {
        break;
      }
      if ((i < int(4))) {
      } else {
        break;
      }
      {
        uint tint_low_inc = (tint_loop_idx.x + 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 0u));
        tint_loop_idx.y = (tint_loop_idx.y + tint_carry);
        i = (i + int(1));
      }
      continue;
    }
  }
}

