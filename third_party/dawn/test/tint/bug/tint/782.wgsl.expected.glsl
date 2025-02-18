#version 310 es

void foo() {
  int explicitStride[2] = int[2](0, 0);
  int implictStride[2] = int[2](0, 0);
  implictStride = explicitStride;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
