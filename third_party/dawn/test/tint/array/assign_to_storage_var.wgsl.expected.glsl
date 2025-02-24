#version 310 es


struct S {
  ivec4 arr[4];
};

struct S_nested {
  int arr[4][3][2];
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
layout(binding = 2, std430)
buffer dst_block_1_ssbo {
  S inner;
} v_2;
layout(binding = 3, std430)
buffer dst_nested_block_1_ssbo {
  S_nested inner;
} v_3;
ivec4[4] ret_arr() {
  return ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
}
S ret_struct_arr() {
  return S(ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0)));
}
void foo(ivec4 src_param[4]) {
  ivec4 src_function[4] = ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
  v_2.inner.arr = ivec4[4](ivec4(1), ivec4(2), ivec4(3), ivec4(3));
  v_2.inner.arr = src_param;
  v_2.inner.arr = ret_arr();
  ivec4 src_let[4] = ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
  v_2.inner.arr = src_let;
  v_2.inner.arr = src_function;
  v_2.inner.arr = src_private;
  v_2.inner.arr = src_workgroup;
  v_2.inner.arr = ret_struct_arr().arr;
  v_2.inner.arr = v.inner.arr;
  v_2.inner.arr = v_1.inner.arr;
  int src_nested[4][3][2] = int[4][3][2](int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)), int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)), int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)), int[3][2](int[2](0, 0), int[2](0, 0), int[2](0, 0)));
  v_3.inner.arr = src_nested;
}
void main_inner(uint tint_local_index) {
  {
    uint v_4 = 0u;
    v_4 = tint_local_index;
    while(true) {
      uint v_5 = v_4;
      if ((v_5 >= 4u)) {
        break;
      }
      src_workgroup[v_5] = ivec4(0);
      {
        v_4 = (v_5 + 1u);
      }
      continue;
    }
  }
  barrier();
  ivec4 ary[4] = ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
  foo(ary);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
