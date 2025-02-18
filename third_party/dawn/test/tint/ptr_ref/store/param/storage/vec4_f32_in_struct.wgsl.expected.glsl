#version 310 es


struct str {
  vec4 i;
};

layout(binding = 0, std430)
buffer S_block_1_ssbo {
  str inner;
} v;
void func() {
  v.inner.i = vec4(0.0f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  func();
}
