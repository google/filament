#version 310 es


struct Inner {
  mat4 m;
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
  mat4 l_a_3_a_2_m = v.inner[3u].a[2u].m;
  vec4 l_a_3_a_2_m_1 = v.inner[3u].a[2u].m[1u];
  float l_a_3_a_2_m_1_0 = v.inner[3u].a[2u].m[1u].x;
}
