//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;


struct SB_RW_atomic {
  uint arg_0;
};

struct tint_struct {
  uint old_value;
  bool exchanged;
};

struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};

layout(binding = 0, std430)
buffer f_sb_rw_block_ssbo {
  SB_RW_atomic inner;
} v;
void atomicCompareExchangeWeak_63d8e6() {
  uint arg_1 = 0u;
  uint arg_2 = 0u;
  tint_struct res = tint_struct(0u, false);
  arg_1 = 1u;
  arg_2 = 1u;
  uint x_21 = arg_2;
  uint x_22 = arg_1;
  uint v_1 = atomicCompSwap(v.inner.arg_0, x_22, x_21);
  uint old_value_1 = atomic_compare_exchange_result_u32(v_1, (v_1 == x_22)).old_value;
  uint x_23 = old_value_1;
  res = tint_struct(x_23, (x_23 == x_21));
}
void fragment_main_1() {
  atomicCompareExchangeWeak_63d8e6();
}
void main() {
  fragment_main_1();
}
//
// compute_main
//
#version 310 es


struct SB_RW_atomic {
  uint arg_0;
};

struct tint_struct {
  uint old_value;
  bool exchanged;
};

struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};

layout(binding = 0, std430)
buffer sb_rw_block_1_ssbo {
  SB_RW_atomic inner;
} v;
void atomicCompareExchangeWeak_63d8e6() {
  uint arg_1 = 0u;
  uint arg_2 = 0u;
  tint_struct res = tint_struct(0u, false);
  arg_1 = 1u;
  arg_2 = 1u;
  uint x_21 = arg_2;
  uint x_22 = arg_1;
  uint v_1 = atomicCompSwap(v.inner.arg_0, x_22, x_21);
  uint old_value_1 = atomic_compare_exchange_result_u32(v_1, (v_1 == x_22)).old_value;
  uint x_23 = old_value_1;
  res = tint_struct(x_23, (x_23 == x_21));
}
void compute_main_1() {
  atomicCompareExchangeWeak_63d8e6();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_1();
}
