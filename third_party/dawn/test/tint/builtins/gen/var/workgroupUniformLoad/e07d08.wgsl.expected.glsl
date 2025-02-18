#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  float16_t inner;
} v;
shared float16_t arg_0;
float16_t workgroupUniformLoad_e07d08() {
  barrier();
  float16_t v_1 = arg_0;
  barrier();
  float16_t res = v_1;
  return res;
}
void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    arg_0 = 0.0hf;
  }
  barrier();
  v.inner = workgroupUniformLoad_e07d08();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner(gl_LocalInvocationIndex);
}
