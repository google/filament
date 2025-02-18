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
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  Outer l_a[4] = v.inner;
  Outer l_a_3 = v.inner[3u];
  Inner l_a_3_a[4] = v.inner[3u].a;
  Inner l_a_3_a_2 = v.inner[3u].a[2u];
  mat2x4 l_a_3_a_2_m = v.inner[3u].a[2u].m;
  vec4 l_a_3_a_2_m_1 = v.inner[3u].a[2u].m[1u];
  float l_a_3_a_2_m_1_0 = v.inner[3u].a[2u].m[1u].x;
}
