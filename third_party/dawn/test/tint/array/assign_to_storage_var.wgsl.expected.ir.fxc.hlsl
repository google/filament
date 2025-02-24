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
RWByteAddressBuffer v : register(u2);
RWByteAddressBuffer dst_nested : register(u3);
typedef int4 ary_ret[4];
ary_ret ret_arr() {
  int4 v_1[4] = (int4[4])0;
  return v_1;
}

S ret_struct_arr() {
  S v_2 = (S)0;
  return v_2;
}

void v_3(uint offset, int obj[2]) {
  {
    uint v_4 = 0u;
    v_4 = 0u;
    while(true) {
      uint v_5 = v_4;
      if ((v_5 >= 2u)) {
        break;
      }
      dst_nested.Store((offset + (v_5 * 4u)), asuint(obj[v_5]));
      {
        v_4 = (v_5 + 1u);
      }
      continue;
    }
  }
}

void v_6(uint offset, int obj[3][2]) {
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 3u)) {
        break;
      }
      int v_9[2] = obj[v_8];
      v_3((offset + (v_8 * 8u)), v_9);
      {
        v_7 = (v_8 + 1u);
      }
      continue;
    }
  }
}

void v_10(uint offset, int obj[4][3][2]) {
  {
    uint v_11 = 0u;
    v_11 = 0u;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 4u)) {
        break;
      }
      int v_13[3][2] = obj[v_12];
      v_6((offset + (v_12 * 24u)), v_13);
      {
        v_11 = (v_12 + 1u);
      }
      continue;
    }
  }
}

void v_14(uint offset, int4 obj[4]) {
  {
    uint v_15 = 0u;
    v_15 = 0u;
    while(true) {
      uint v_16 = v_15;
      if ((v_16 >= 4u)) {
        break;
      }
      v.Store4((offset + (v_16 * 16u)), asuint(obj[v_16]));
      {
        v_15 = (v_16 + 1u);
      }
      continue;
    }
  }
}

typedef int4 ary_ret_1[4];
ary_ret_1 v_17(uint offset) {
  int4 a[4] = (int4[4])0;
  {
    uint v_18 = 0u;
    v_18 = 0u;
    while(true) {
      uint v_19 = v_18;
      if ((v_19 >= 4u)) {
        break;
      }
      a[v_19] = asint(src_storage.Load4((offset + (v_19 * 16u))));
      {
        v_18 = (v_19 + 1u);
      }
      continue;
    }
  }
  int4 v_20[4] = a;
  return v_20;
}

typedef int4 ary_ret_2[4];
ary_ret_2 v_21(uint start_byte_offset) {
  int4 a[4] = (int4[4])0;
  {
    uint v_22 = 0u;
    v_22 = 0u;
    while(true) {
      uint v_23 = v_22;
      if ((v_23 >= 4u)) {
        break;
      }
      a[v_23] = asint(src_uniform[((start_byte_offset + (v_23 * 16u)) / 16u)]);
      {
        v_22 = (v_23 + 1u);
      }
      continue;
    }
  }
  int4 v_24[4] = a;
  return v_24;
}

void foo(int4 src_param[4]) {
  int4 src_function[4] = (int4[4])0;
  int4 v_25[4] = {(int(1)).xxxx, (int(2)).xxxx, (int(3)).xxxx, (int(3)).xxxx};
  v_14(0u, v_25);
  v_14(0u, src_param);
  int4 v_26[4] = ret_arr();
  v_14(0u, v_26);
  int4 src_let[4] = (int4[4])0;
  v_14(0u, src_let);
  int4 v_27[4] = src_function;
  v_14(0u, v_27);
  int4 v_28[4] = src_private;
  v_14(0u, v_28);
  int4 v_29[4] = src_workgroup;
  v_14(0u, v_29);
  S v_30 = ret_struct_arr();
  int4 v_31[4] = v_30.arr;
  v_14(0u, v_31);
  int4 v_32[4] = v_21(0u);
  v_14(0u, v_32);
  int4 v_33[4] = v_17(0u);
  v_14(0u, v_33);
  int src_nested[4][3][2] = (int[4][3][2])0;
  int v_34[4][3][2] = src_nested;
  v_10(0u, v_34);
}

void main_inner(uint tint_local_index) {
  {
    uint v_35 = 0u;
    v_35 = tint_local_index;
    while(true) {
      uint v_36 = v_35;
      if ((v_36 >= 4u)) {
        break;
      }
      src_workgroup[v_36] = (int(0)).xxxx;
      {
        v_35 = (v_36 + 1u);
      }
      continue;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  int4 ary[4] = (int4[4])0;
  foo(ary);
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

