#version 310 es

shared mat2x4 S;
void func(uint pointer_indices[1]) {
  S[pointer_indices[0u]] = vec4(0.0f);
}
void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    S = mat2x4(vec4(0.0f), vec4(0.0f));
  }
  barrier();
  func(uint[1](1u));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
