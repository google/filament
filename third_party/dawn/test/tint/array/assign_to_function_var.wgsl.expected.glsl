#version 310 es


struct S {
  ivec4 arr[4];
};

ivec4 src_private[4] = ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
shared ivec4 src_workgroup[4];
layout(binding = 0, std140)
uniform src_uniform_block_1_ubo {
  S inner;
} v;
layout(binding = 1, std430)
buffer src_storage_block_1_ssbo {
  S inner;
} v_1;
ivec4[4] ret_arr() {
  return ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
}
S ret_struct_arr() {
  return S(ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0)));
}
void foo(ivec4 src_param[4]) {
  ivec4 src_function[4] = ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
  ivec4 dst[4] = ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
  dst = ivec4[4](ivec4(1), ivec4(2), ivec4(3), ivec4(3));
  dst = src_param;
  dst = ret_arr();
  ivec4 src_let[4] = ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
  dst = src_let;
  dst = src_function;
  dst = src_private;
  dst = src_workgroup;
  dst = ret_struct_arr().arr;
  dst = v.inner.arr;
  dst = v_1.inner.arr;
  int dst_nested[4][3][2] = int[4][3][2](int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)), int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)), int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)), int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)));
  int src_nested[4][3][2] = int[4][3][2](int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)), int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)), int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)), int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)));
  dst_nested = src_nested;
}
void main_inner(uint tint_local_index) {
  {
    uint v_2 = 0u;
    v_2 = tint_local_index;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 4u)) {
        break;
      }
      src_workgroup[v_3] = ivec4(0);
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
  barrier();
  ivec4 val[4] = ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
  foo(val);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
