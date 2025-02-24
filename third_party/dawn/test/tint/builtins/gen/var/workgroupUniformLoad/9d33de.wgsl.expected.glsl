#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  int inner;
} v;
shared int arg_0;
int workgroupUniformLoad_9d33de() {
  barrier();
  int v_1 = arg_0;
  barrier();
  int res = v_1;
  return res;
}
void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    arg_0 = 0;
  }
  barrier();
  v.inner = workgroupUniformLoad_9d33de();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner(gl_LocalInvocationIndex);
}
