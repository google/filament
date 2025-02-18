#version 310 es

layout(binding = 0, std430)
buffer buffer_block_1_ssbo {
  ivec4 inner;
} v;
void deref() {
  v.inner = v.inner.wzyx;
}
void no_deref() {
  v.inner = v.inner.wzyx;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  deref();
  no_deref();
}
