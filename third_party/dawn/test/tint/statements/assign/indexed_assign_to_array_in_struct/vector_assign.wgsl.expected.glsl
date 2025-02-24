#version 310 es


struct Uniforms {
  uint i;
};

struct OuterS {
  uint a1[8];
};

layout(binding = 4, std140)
uniform uniforms_block_1_ubo {
  Uniforms inner;
} v_1;
uint f(uint i) {
  return (i + 1u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  OuterS s1 = OuterS(uint[8](0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u));
  vec3 v = vec3(0.0f);
  uint v_2 = min(v_1.inner.i, 7u);
  v[min(s1.a1[v_2], 2u)] = 1.0f;
  uint v_3 = min(v_1.inner.i, 7u);
  v[min(f(s1.a1[v_3]), 2u)] = 1.0f;
}
