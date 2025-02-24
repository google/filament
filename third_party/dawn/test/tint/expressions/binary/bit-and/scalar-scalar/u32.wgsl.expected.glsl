#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint a = 1u;
  uint b = 2u;
  uint r = (a & b);
}
