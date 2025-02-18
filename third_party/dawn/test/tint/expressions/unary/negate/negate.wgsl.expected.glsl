#version 310 es

int i(int x) {
  return -(x);
}
ivec4 vi(ivec4 x) {
  return -(x);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
