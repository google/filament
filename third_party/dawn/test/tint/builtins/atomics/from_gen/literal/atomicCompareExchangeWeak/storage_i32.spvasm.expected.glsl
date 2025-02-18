//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;


struct SB_RW_atomic {
  int arg_0;
};

struct tint_struct {
  int old_value;
  bool exchanged;
};

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

layout(binding = 0, std430)
buffer f_sb_rw_block_ssbo {
  SB_RW_atomic inner;
} v;
void atomicCompareExchangeWeak_1bd40a() {
  tint_struct res = tint_struct(0, false);
  int v_1 = atomicCompSwap(v.inner.arg_0, 1, 1);
  int old_value_1 = atomic_compare_exchange_result_i32(v_1, (v_1 == 1)).old_value;
  int x_19 = old_value_1;
  res = tint_struct(x_19, (x_19 == 1));
}
void fragment_main_1() {
  atomicCompareExchangeWeak_1bd40a();
}
void main() {
  fragment_main_1();
}
//
// compute_main
//
#version 310 es


struct SB_RW_atomic {
  int arg_0;
};

struct tint_struct {
  int old_value;
  bool exchanged;
};

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

layout(binding = 0, std430)
buffer sb_rw_block_1_ssbo {
  SB_RW_atomic inner;
} v;
void atomicCompareExchangeWeak_1bd40a() {
  tint_struct res = tint_struct(0, false);
  int v_1 = atomicCompSwap(v.inner.arg_0, 1, 1);
  int old_value_1 = atomic_compare_exchange_result_i32(v_1, (v_1 == 1)).old_value;
  int x_19 = old_value_1;
  res = tint_struct(x_19, (x_19 == 1));
}
void compute_main_1() {
  atomicCompareExchangeWeak_1bd40a();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_1();
}
