#version 310 es

shared mat2 S;
vec2 func(uint pointer_indices[1]) {
  return S[pointer_indices[0u]];
}
void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    S = mat2(vec2(0.0f), vec2(0.0f));
  }
  barrier();
  vec2 r = func(uint[1](1u));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
