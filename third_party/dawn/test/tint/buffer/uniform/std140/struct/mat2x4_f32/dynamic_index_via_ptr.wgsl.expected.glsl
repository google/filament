#version 310 es


struct Inner {
  mat2x4 m;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  uint tint_pad_3;
  uint tint_pad_4;
  uint tint_pad_5;
  uint tint_pad_6;
  uint tint_pad_7;
};

struct Outer {
  Inner a[4];
};

layout(binding = 0, std140)
uniform a_block_1_ubo {
  Outer inner[4];
} v;
int counter = 0;
int i() {
  counter = (counter + 1);
  return counter;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_1 = min(uint(i()), 3u);
  uint v_2 = min(uint(i()), 3u);
  uint v_3 = min(uint(i()), 1u);
  Outer l_a[4] = v.inner;
  Outer l_a_i = v.inner[v_1];
  Inner l_a_i_a[4] = v.inner[v_1].a;
  Inner l_a_i_a_i = v.inner[v_1].a[v_2];
  mat2x4 l_a_i_a_i_m = v.inner[v_1].a[v_2].m;
  vec4 l_a_i_a_i_m_i = v.inner[v_1].a[v_2].m[v_3];
  float l_a_i_a_i_m_i_i = v.inner[v_1].a[v_2].m[v_3][min(uint(i()), 3u)];
}
