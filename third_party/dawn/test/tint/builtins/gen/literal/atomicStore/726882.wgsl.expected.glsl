#version 310 es

shared uint arg_0;
void atomicStore_726882() {
  atomicExchange(arg_0, 1u);
}
void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    atomicExchange(arg_0, 0u);
  }
  barrier();
  atomicStore_726882();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner(gl_LocalInvocationIndex);
}
