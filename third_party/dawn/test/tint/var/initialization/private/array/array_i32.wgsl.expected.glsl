#version 310 es

int zero[2][3] = int[2][3](int[3](0, 0, 0), int[3](0, 0, 0));
int init[2][3] = int[2][3](int[3](1, 2, 3), int[3](4, 5, 6));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int v0[2][3] = zero;
  int v1[2][3] = init;
}
