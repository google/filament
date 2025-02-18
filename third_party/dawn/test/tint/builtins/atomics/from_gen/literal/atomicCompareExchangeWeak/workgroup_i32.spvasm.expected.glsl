#version 310 es


struct tint_struct {
  int old_value;
  bool exchanged;
};

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

uint local_invocation_index_1 = 0u;
shared int arg_0;
void atomicCompareExchangeWeak_e88938() {
  tint_struct res = tint_struct(0, false);
  int v = atomicCompSwap(arg_0, 1, 1);
  int old_value_1 = atomic_compare_exchange_result_i32(v, (v == 1)).old_value;
  int x_18 = old_value_1;
  res = tint_struct(x_18, (x_18 == 1));
}
void compute_main_inner(uint local_invocation_index_2) {
  atomicExchange(arg_0, 0);
  barrier();
  atomicCompareExchangeWeak_e88938();
}
void compute_main_1() {
  uint x_36 = local_invocation_index_1;
  compute_main_inner(x_36);
}
void compute_main_inner_1(uint local_invocation_index_1_param) {
  if ((local_invocation_index_1_param < 1u)) {
    atomicExchange(arg_0, 0);
  }
  barrier();
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner_1(gl_LocalInvocationIndex);
}
