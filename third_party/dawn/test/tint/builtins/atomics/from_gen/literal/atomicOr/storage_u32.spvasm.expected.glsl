//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;


struct SB_RW_atomic {
  uint arg_0;
};

layout(binding = 0, std430)
buffer f_sb_rw_block_ssbo {
  SB_RW_atomic inner;
} v;
void atomicOr_5e95d4() {
  uint res = 0u;
  uint x_9 = atomicOr(v.inner.arg_0, 1u);
  res = x_9;
}
void fragment_main_1() {
  atomicOr_5e95d4();
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

layout(binding = 0, std430)
buffer sb_rw_block_1_ssbo {
  SB_RW_atomic inner;
} v;
void atomicOr_5e95d4() {
  uint res = 0u;
  uint x_9 = atomicOr(v.inner.arg_0, 1u);
  res = x_9;
}
void compute_main_1() {
  atomicOr_5e95d4();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_1();
}
