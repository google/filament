#version 310 es


struct Inner {
  bool b;
  ivec4 v;
  mat3 m;
};

struct Outer {
  Inner a[4];
};

shared Outer v;
Outer foo() {
  barrier();
  Outer v_1 = v;
  barrier();
  return v_1;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
