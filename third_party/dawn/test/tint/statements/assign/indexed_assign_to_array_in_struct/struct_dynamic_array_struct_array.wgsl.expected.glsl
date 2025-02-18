#version 310 es


struct Uniforms {
  uint i;
  uint j;
};

struct InnerS {
  int v;
};

struct S1 {
  InnerS a2[8];
};

layout(binding = 4, std140)
uniform uniforms_block_1_ubo {
  Uniforms inner;
} v_1;
layout(binding = 0, std430)
buffer OuterS_1_ssbo {
  S1 a1[];
} s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  InnerS v = InnerS(0);
  uint v_2 = v_1.inner.i;
  uint v_3 = v_1.inner.j;
  uint v_4 = min(v_2, (uint(s.a1.length()) - 1u));
  s.a1[v_4].a2[min(v_3, 7u)] = v;
}
