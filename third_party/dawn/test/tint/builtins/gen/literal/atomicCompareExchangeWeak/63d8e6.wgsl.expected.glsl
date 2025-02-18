//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;


struct SB_RW {
  uint arg_0;
};

struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};

layout(binding = 0, std430)
buffer f_sb_rw_block_ssbo {
  SB_RW inner;
} v;
void atomicCompareExchangeWeak_63d8e6() {
  uint v_1 = atomicCompSwap(v.inner.arg_0, 1u, 1u);
  atomic_compare_exchange_result_u32 res = atomic_compare_exchange_result_u32(v_1, (v_1 == 1u));
}
void main() {
  atomicCompareExchangeWeak_63d8e6();
}
//
// compute_main
//
#version 310 es


struct SB_RW {
  uint arg_0;
};

struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};

layout(binding = 0, std430)
buffer sb_rw_block_1_ssbo {
  SB_RW inner;
} v;
void atomicCompareExchangeWeak_63d8e6() {
  uint v_1 = atomicCompSwap(v.inner.arg_0, 1u, 1u);
  atomic_compare_exchange_result_u32 res = atomic_compare_exchange_result_u32(v_1, (v_1 == 1u));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  atomicCompareExchangeWeak_63d8e6();
}
