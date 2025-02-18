#version 310 es


struct tint_struct {
  uint old_value;
  bool exchanged;
};

struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};

uint local_invocation_index_1 = 0u;
shared uint arg_0;
void atomicCompareExchangeWeak_83580d() {
  tint_struct res = tint_struct(0u, false);
  uint v = atomicCompSwap(arg_0, 1u, 1u);
  uint old_value_1 = atomic_compare_exchange_result_u32(v, (v == 1u)).old_value;
  uint x_17 = old_value_1;
  res = tint_struct(x_17, (x_17 == 1u));
}
void compute_main_inner(uint local_invocation_index_2) {
  atomicExchange(arg_0, 0u);
  barrier();
  atomicCompareExchangeWeak_83580d();
}
void compute_main_1() {
  uint x_35 = local_invocation_index_1;
  compute_main_inner(x_35);
}
void compute_main_inner_1(uint local_invocation_index_1_param) {
  if ((local_invocation_index_1_param < 1u)) {
    atomicExchange(arg_0, 0u);
  }
  barrier();
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner_1(gl_LocalInvocationIndex);
}
