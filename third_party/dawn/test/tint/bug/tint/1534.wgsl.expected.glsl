#version 310 es


struct g {
  uvec3 a;
  uint tint_pad_0;
};

struct h {
  uint a;
};

layout(binding = 0, std140)
uniform i_block_1_ubo {
  g inner;
} v;
layout(binding = 1, std430)
buffer j_block_1_ssbo {
  h inner;
} v_1;
uint tint_int_dot(uvec3 x, uvec3 y) {
  return (((x.x * y.x) + (x.y * y.y)) + (x.z * y.z));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint l = tint_int_dot(v.inner.a, v.inner.a);
  v_1.inner.a = v.inner.a.x;
}
