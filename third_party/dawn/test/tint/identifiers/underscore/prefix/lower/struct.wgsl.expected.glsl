#version 310 es


struct _a {
  int _b;
};

layout(binding = 0, std430)
buffer s_block_1_ssbo {
  int inner;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  _a c = _a(0);
  int d = c._b;
  v.inner = (c._b + d);
}
