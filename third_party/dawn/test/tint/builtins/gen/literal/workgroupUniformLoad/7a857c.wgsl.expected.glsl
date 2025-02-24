#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  float inner;
} v;
shared float arg_0;
float workgroupUniformLoad_7a857c() {
  barrier();
  float v_1 = arg_0;
  barrier();
  float res = v_1;
  return res;
}
void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    arg_0 = 0.0f;
  }
  barrier();
  v.inner = workgroupUniformLoad_7a857c();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner(gl_LocalInvocationIndex);
}
