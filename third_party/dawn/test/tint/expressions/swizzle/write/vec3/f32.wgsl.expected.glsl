#version 310 es


struct S {
  vec3 v;
};

S P = S(vec3(0.0f));
void f() {
  P.v = vec3(1.0f, 2.0f, 3.0f);
  P.v.x = 1.0f;
  P.v.y = 2.0f;
  P.v.z = 3.0f;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
