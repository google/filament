#version 310 es


struct str {
  vec4 i;
};

shared str S;
void func() {
  S.i = vec4(0.0f);
}
void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    S = str(vec4(0.0f));
  }
  barrier();
  func();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
