#version 310 es

shared vec4 S;
vec4 func() {
  return S;
}
void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    S = vec4(0.0f);
  }
  barrier();
  vec4 r = func();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
