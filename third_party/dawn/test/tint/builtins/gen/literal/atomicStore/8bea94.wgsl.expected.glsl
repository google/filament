#version 310 es

shared int arg_0;
void atomicStore_8bea94() {
  atomicExchange(arg_0, 1);
}
void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    atomicExchange(arg_0, 0);
  }
  barrier();
  atomicStore_8bea94();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner(gl_LocalInvocationIndex);
}
