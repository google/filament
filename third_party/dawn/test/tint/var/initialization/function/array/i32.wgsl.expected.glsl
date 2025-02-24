#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int zero[3] = int[3](0, 0, 0);
  int init[3] = int[3](1, 2, 3);
}
