#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct Inner {
  float16_t scalar_f16;
  uint tint_pad_0;
  f16vec3 vec3_f16;
  f16mat2x4 mat2x4_f16;
};

struct S {
  Inner inner;
};

layout(binding = 0, std430)
buffer in_block_1_ssbo {
  S inner;
} v;
layout(binding = 1, std430)
buffer out_block_1_ssbo {
  S inner;
} v_1;
void tint_store_and_preserve_padding_1(Inner value_param) {
  v_1.inner.inner.scalar_f16 = value_param.scalar_f16;
  v_1.inner.inner.vec3_f16 = value_param.vec3_f16;
  v_1.inner.inner.mat2x4_f16 = value_param.mat2x4_f16;
}
void tint_store_and_preserve_padding(S value_param) {
  tint_store_and_preserve_padding_1(value_param.inner);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  S t = v.inner;
  tint_store_and_preserve_padding(t);
}
