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
  int arg_1 = 0;
  int arg_2 = 0;
  tint_struct res = tint_struct(0, false);
  arg_1 = 1;
  arg_2 = 1;
  int x_23 = arg_2;
  int x_24 = arg_1;
  int v_1 = atomicCompSwap(v.inner.arg_0, x_24, x_23);
  int old_value_1 = atomic_compare_exchange_result_i32(v_1, (v_1 == x_24)).old_value;
  int x_25 = old_value_1;
  res = tint_struct(x_25, (x_25 == x_23));
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
  int arg_1 = 0;
  int arg_2 = 0;
  tint_struct res = tint_struct(0, false);
  arg_1 = 1;
  arg_2 = 1;
  int x_23 = arg_2;
  int x_24 = arg_1;
  int v_1 = atomicCompSwap(v.inner.arg_0, x_24, x_23);
  int old_value_1 = atomic_compare_exchange_result_i32(v_1, (v_1 == x_24)).old_value;
  int x_25 = old_value_1;
  res = tint_struct(x_25, (x_25 == x_23));
}
void compute_main_1() {
  atomicCompareExchangeWeak_1bd40a();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_1();
}
