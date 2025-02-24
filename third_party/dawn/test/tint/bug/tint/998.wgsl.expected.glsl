#version 310 es


struct Constants {
  uint zero;
};

struct S {
  uint data[3];
};

layout(binding = 0, std140)
uniform constants_block_1_ubo {
  Constants inner;
} v;
S s = S(uint[3](0u, 0u, 0u));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_1 = min(v.inner.zero, 2u);
  s.data[v_1] = 0u;
}
