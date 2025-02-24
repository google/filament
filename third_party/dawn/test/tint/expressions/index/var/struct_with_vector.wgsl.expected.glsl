#version 310 es


struct S {
  int m;
  uvec3 n;
};

uint f() {
  S a = S(0, uvec3(0u));
  return a.n.z;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
