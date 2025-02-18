struct S {
  int4 arr[4];
};

struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


static int4 src_private[4] = (int4[4])0;
groupshared int4 src_workgroup[4];
cbuffer cbuffer_src_uniform : register(b0) {
  uint4 src_uniform[4];
};
RWByteAddressBuffer src_storage : register(u1);
typedef int4 ary_ret[4];
ary_ret ret_arr() {
  int4 v[4] = (int4[4])0;
  return v;
}

S ret_struct_arr() {
  S v_1 = (S)0;
  return v_1;
}

typedef int4 ary_ret_1[4];
ary_ret_1 v_2(uint offset) {
  int4 a[4] = (int4[4])0;
  {
    uint v_3 = 0u;
    v_3 = 0u;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 4u)) {
        break;
      }
      a[v_4] = asint(src_storage.Load4((offset + (v_4 * 16u))));
      {
        v_3 = (v_4 + 1u);
      }
      continue;
    }
  }
  int4 v_5[4] = a;
  return v_5;
}

typedef int4 ary_ret_2[4];
ary_ret_2 v_6(uint start_byte_offset) {
  int4 a[4] = (int4[4])0;
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 4u)) {
        break;
      }
      a[v_8] = asint(src_uniform[((start_byte_offset + (v_8 * 16u)) / 16u)]);
      {
        v_7 = (v_8 + 1u);
      }
      continue;
    }
  }
  int4 v_9[4] = a;
  return v_9;
}

void foo(int4 src_param[4]) {
  int4 src_function[4] = (int4[4])0;
  int4 v_10[4] = (int4[4])0;
  int4 v_11[4] = {(int(1)).xxxx, (int(2)).xxxx, (int(3)).xxxx, (int(3)).xxxx};
  v_10 = v_11;
  v_10 = src_param;
  int4 v_12[4] = ret_arr();
  v_10 = v_12;
  int4 src_let[4] = (int4[4])0;
  v_10 = src_let;
  int4 v_13[4] = src_function;
  v_10 = v_13;
  int4 v_14[4] = src_private;
  v_10 = v_14;
  int4 v_15[4] = src_workgroup;
  v_10 = v_15;
  S v_16 = ret_struct_arr();
  int4 v_17[4] = v_16.arr;
  v_10 = v_17;
  int4 v_18[4] = v_6(0u);
  v_10 = v_18;
  int4 v_19[4] = v_2(0u);
  v_10 = v_19;
  int dst_nested[4][3][2] = (int[4][3][2])0;
  int src_nested[4][3][2] = (int[4][3][2])0;
  int v_20[4][3][2] = src_nested;
  dst_nested = v_20;
}

void main_inner(uint tint_local_index) {
  {
    uint v_21 = 0u;
    v_21 = tint_local_index;
    while(true) {
      uint v_22 = v_21;
      if ((v_22 >= 4u)) {
        break;
      }
      src_workgroup[v_22] = (int(0)).xxxx;
      {
        v_21 = (v_22 + 1u);
      }
      continue;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  int4 val[4] = (int4[4])0;
  foo(val);
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

