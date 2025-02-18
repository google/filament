#version 310 es


struct Uniforms {
  uint i;
};

struct InnerS {
  int v;
};

layout(binding = 4, std140)
uniform uniforms_block_1_ubo {
  Uniforms inner;
} v_1;
layout(binding = 0, std430)
buffer OuterS_1_ssbo {
  InnerS a1[];
} s1;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  InnerS v = InnerS(0);
  uint v_2 = v_1.inner.i;
  uint v_3 = min(v_2, (uint(s1.a1.length()) - 1u));
  s1.a1[v_3] = v;
}
