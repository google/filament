//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;


struct SB_RW {
  int arg_0;
};

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

layout(binding = 0, std430)
buffer f_sb_rw_block_ssbo {
  SB_RW inner;
} v;
void atomicCompareExchangeWeak_1bd40a() {
  int arg_1 = 1;
  int arg_2 = 1;
  int v_1 = arg_1;
  int v_2 = atomicCompSwap(v.inner.arg_0, v_1, arg_2);
  atomic_compare_exchange_result_i32 res = atomic_compare_exchange_result_i32(v_2, (v_2 == v_1));
}
void main() {
  atomicCompareExchangeWeak_1bd40a();
}
//
// compute_main
//
#version 310 es


struct SB_RW {
  int arg_0;
};

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};

layout(binding = 0, std430)
buffer sb_rw_block_1_ssbo {
  SB_RW inner;
} v;
void atomicCompareExchangeWeak_1bd40a() {
  int arg_1 = 1;
  int arg_2 = 1;
  int v_1 = arg_1;
  int v_2 = atomicCompSwap(v.inner.arg_0, v_1, arg_2);
  atomic_compare_exchange_result_i32 res = atomic_compare_exchange_result_i32(v_2, (v_2 == v_1));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  atomicCompareExchangeWeak_1bd40a();
}
