#version 310 es


struct Inner {
  float scalar_f32;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  vec3 vec3_f32;
  uint tint_pad_3;
  mat2x4 mat2x4_f32;
};

struct S {
  Inner inner;
};

layout(binding = 0, std140)
uniform u_block_1_ubo {
  S inner;
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  S inner;
} v_1;
void tint_store_and_preserve_padding_1(Inner value_param) {
  v_1.inner.inner.scalar_f32 = value_param.scalar_f32;
  v_1.inner.inner.vec3_f32 = value_param.vec3_f32;
  v_1.inner.inner.mat2x4_f32 = value_param.mat2x4_f32;
}
void tint_store_and_preserve_padding(S value_param) {
  tint_store_and_preserve_padding_1(value_param.inner);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  S x = v.inner;
  tint_store_and_preserve_padding(x);
}
