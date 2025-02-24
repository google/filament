#version 310 es


struct S {
  vec4 a;
  int b;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
};

layout(binding = 0, std430)
buffer sb_block_1_ssbo {
  S inner[];
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_1 = (uint(v.inner.length()) - 1u);
  uint v_2 = min(uint(1), v_1);
  S x = v.inner[v_2];
}
