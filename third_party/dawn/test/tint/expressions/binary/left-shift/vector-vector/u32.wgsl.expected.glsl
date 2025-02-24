#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uvec3 a = uvec3(1u, 2u, 3u);
  uvec3 b = uvec3(4u, 5u, 6u);
  uvec3 r = (a << (b & uvec3(31u)));
}
