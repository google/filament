#version 310 es


struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

shared int arg_0;
void atomicCompareExchangeWeak_e88938() {
  int v = atomicCompSwap(arg_0, 1, 1);
  atomic_compare_exchange_result_i32 res = atomic_compare_exchange_result_i32(v, (v == 1));
}
void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    atomicExchange(arg_0, 0);
  }
  barrier();
  atomicCompareExchangeWeak_e88938();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner(gl_LocalInvocationIndex);
}
