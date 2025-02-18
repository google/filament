#version 310 es


struct strided_arr {
  vec2 el;
  uint tint_pad_0;
  uint tint_pad_1;
};

struct SSBO {
  strided_arr m[2];
};

layout(binding = 0, std430)
buffer ssbo_block_1_ssbo {
  SSBO inner;
} v;
strided_arr[2] mat2x2_stride_16_to_arr(mat2 m) {
  strided_arr v_1 = strided_arr(m[0u], 0u, 0u);
  return strided_arr[2](v_1, strided_arr(m[1u], 0u, 0u));
}
mat2 arr_to_mat2x2_stride_16(strided_arr arr[2]) {
  return mat2(arr[0u].el, arr[1u].el);
}
void tint_store_and_preserve_padding_1(uint target_indices[1], strided_arr value_param) {
  v.inner.m[target_indices[0u]].el = value_param.el;
}
void tint_store_and_preserve_padding(strided_arr value_param[2]) {
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 2u)) {
        break;
      }
      tint_store_and_preserve_padding_1(uint[1](v_3), value_param[v_3]);
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
}
void f_1() {
  tint_store_and_preserve_padding(mat2x2_stride_16_to_arr(arr_to_mat2x2_stride_16(v.inner.m)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_1();
}
