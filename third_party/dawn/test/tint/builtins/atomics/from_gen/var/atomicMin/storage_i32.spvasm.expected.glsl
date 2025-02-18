//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;


struct SB_RW_atomic {
  int arg_0;
};

layout(binding = 0, std430)
buffer f_sb_rw_block_ssbo {
  SB_RW_atomic inner;
} v;
void atomicMin_8e38dc() {
  int arg_1 = 0;
  int res = 0;
  arg_1 = 1;
  int x_20 = arg_1;
  int x_13 = atomicMin(v.inner.arg_0, x_20);
  res = x_13;
}
void fragment_main_1() {
  atomicMin_8e38dc();
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

layout(binding = 0, std430)
buffer sb_rw_block_1_ssbo {
  SB_RW_atomic inner;
} v;
void atomicMin_8e38dc() {
  int arg_1 = 0;
  int res = 0;
  arg_1 = 1;
  int x_20 = arg_1;
  int x_13 = atomicMin(v.inner.arg_0, x_20);
  res = x_13;
}
void compute_main_1() {
  atomicMin_8e38dc();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_1();
}
