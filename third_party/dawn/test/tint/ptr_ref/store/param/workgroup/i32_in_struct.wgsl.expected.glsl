#version 310 es


struct str {
  int i;
};

shared str S;
void func() {
  S.i = 42;
}
void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    S = str(0);
  }
  barrier();
  func();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
