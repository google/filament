#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std430)
buffer out_block_1_ssbo {
  f16mat3 inner;
} v;
void tint_store_and_preserve_padding(f16mat3 value_param) {
  v.inner[0u] = value_param[0u];
  v.inner[1u] = value_param[1u];
  v.inner[2u] = value_param[2u];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16mat3 m = f16mat3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf));
  tint_store_and_preserve_padding(f16mat3(m));
}
