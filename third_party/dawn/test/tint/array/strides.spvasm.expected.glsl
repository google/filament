#version 310 es


struct strided_arr {
  float el;
  uint tint_pad_0;
};

struct strided_arr_1 {
  strided_arr el[3][2];
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  uint tint_pad_3;
  uint tint_pad_4;
  uint tint_pad_5;
  uint tint_pad_6;
  uint tint_pad_7;
  uint tint_pad_8;
  uint tint_pad_9;
  uint tint_pad_10;
  uint tint_pad_11;
  uint tint_pad_12;
  uint tint_pad_13;
  uint tint_pad_14;
  uint tint_pad_15;
  uint tint_pad_16;
  uint tint_pad_17;
  uint tint_pad_18;
  uint tint_pad_19;
};

struct S {
  strided_arr_1 a[4];
};

layout(binding = 0, std430)
buffer s_block_1_ssbo {
  S inner;
} v;
void tint_store_and_preserve_padding_4(uint target_indices[3], strided_arr value_param) {
  v.inner.a[target_indices[0u]].el[target_indices[1u]][target_indices[2u]].el = value_param.el;
}
void tint_store_and_preserve_padding_3(uint target_indices[2], strided_arr value_param[2]) {
  {
    uint v_1 = 0u;
    v_1 = 0u;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 2u)) {
        break;
      }
      tint_store_and_preserve_padding_4(uint[3](target_indices[0u], target_indices[1u], v_2), value_param[v_2]);
      {
        v_1 = (v_2 + 1u);
      }
      continue;
    }
  }
}
void tint_store_and_preserve_padding_2(uint target_indices[1], strided_arr value_param[3][2]) {
  {
    uint v_3 = 0u;
    v_3 = 0u;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 3u)) {
        break;
      }
      tint_store_and_preserve_padding_3(uint[2](target_indices[0u], v_4), value_param[v_4]);
      {
        v_3 = (v_4 + 1u);
      }
      continue;
    }
  }
}
void tint_store_and_preserve_padding_1(uint target_indices[1], strided_arr_1 value_param) {
  tint_store_and_preserve_padding_2(uint[1](target_indices[0u]), value_param.el);
}
void tint_store_and_preserve_padding(strided_arr_1 value_param[4]) {
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 4u)) {
        break;
      }
      tint_store_and_preserve_padding_1(uint[1](v_6), value_param[v_6]);
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
}
void f_1() {
  strided_arr_1 x_19[4] = v.inner.a;
  strided_arr x_24[3][2] = v.inner.a[3u].el;
  strided_arr x_28[2] = v.inner.a[3u].el[2u];
  float x_32 = v.inner.a[3u].el[2u][1u].el;
  tint_store_and_preserve_padding(strided_arr_1[4](strided_arr_1(strided_arr[3][2](strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u)), strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u)), strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u))), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), strided_arr_1(strided_arr[3][2](strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u)), strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u)), strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u))), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), strided_arr_1(strided_arr[3][2](strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u)), strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u)), strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u))), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), strided_arr_1(strided_arr[3][2](strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u)), strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u)), strided_arr[2](strided_arr(0.0f, 0u), strided_arr(0.0f, 0u))), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u)));
  v.inner.a[3u].el[2u][1u].el = 5.0f;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_1();
}
