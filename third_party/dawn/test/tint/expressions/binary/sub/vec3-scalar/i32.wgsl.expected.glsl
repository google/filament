#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  ivec3 a = ivec3(1, 2, 3);
  int b = 4;
  ivec3 r = (a - b);
}
