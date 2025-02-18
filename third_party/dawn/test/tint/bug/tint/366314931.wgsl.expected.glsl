#version 310 es


struct S {
  uvec3 v;
  uint u;
};

shared S wgvar;
layout(binding = 0, std430)
buffer output_block_1_ssbo {
  S inner;
} v_1;
void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    wgvar.v = uvec3(0u);
    atomicExchange(wgvar.u, 0u);
  }
  barrier();
  uint x = atomicOr(wgvar.u, 0u);
  atomicExchange(v_1.inner.u, x);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
