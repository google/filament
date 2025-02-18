#version 310 es


struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};

shared uint arg_0;
void atomicCompareExchangeWeak_83580d() {
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  uint v = arg_1;
  uint v_1 = atomicCompSwap(arg_0, v, arg_2);
  atomic_compare_exchange_result_u32 res = atomic_compare_exchange_result_u32(v_1, (v_1 == v));
}
void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    atomicExchange(arg_0, 0u);
  }
  barrier();
  atomicCompareExchangeWeak_83580d();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner(gl_LocalInvocationIndex);
}
