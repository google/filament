#version 310 es


struct S {
  uvec3 v;
};

S P = S(uvec3(0u));
void f() {
  P.v = uvec3(1u, 2u, 3u);
  P.v.x = 1u;
  P.v.y = 2u;
  P.v.z = 3u;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
