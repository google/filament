#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  bvec3 a = bvec3(true, true, false);
  bvec3 b = bvec3(true, false, true);
  uvec3 v = uvec3(a);
  bvec3 r = bvec3((v & uvec3(b)));
}
