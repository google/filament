groupshared int4 src_workgroup[4];

void tint_zero_workgroup_memory(uint local_idx) {
  {
    for(uint idx = local_idx; (idx < 4u); idx = (idx + 1u)) {
      uint i = idx;
      src_workgroup[i] = (0).xxxx;
    }
  }
  GroupMemoryBarrierWithGroupSync();
}

struct S {
  int4 arr[4];
};

static int4 src_private[4] = (int4[4])0;
cbuffer cbuffer_src_uniform : register(b0) {
  uint4 src_uniform[4];
};
RWByteAddressBuffer src_storage : register(u1);

typedef int4 ret_arr_ret[4];
ret_arr_ret ret_arr() {
  int4 tint_symbol_4[4] = (int4[4])0;
  return tint_symbol_4;
}

S ret_struct_arr() {
  S tint_symbol_5 = (S)0;
  return tint_symbol_5;
}

typedef int4 src_uniform_load_ret[4];
src_uniform_load_ret src_uniform_load(uint offset) {
  int4 arr_1[4] = (int4[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      const uint scalar_offset = ((offset + (i_1 * 16u))) / 4;
      arr_1[i_1] = asint(src_uniform[scalar_offset / 4]);
    }
  }
  return arr_1;
}

typedef int4 src_storage_load_ret[4];
src_storage_load_ret src_storage_load(uint offset) {
  int4 arr_2[4] = (int4[4])0;
  {
    for(uint i_2 = 0u; (i_2 < 4u); i_2 = (i_2 + 1u)) {
      arr_2[i_2] = asint(src_storage.Load4((offset + (i_2 * 16u))));
    }
  }
  return arr_2;
}

void foo(int4 src_param[4]) {
  int4 src_function[4] = (int4[4])0;
  int4 tint_symbol[4] = (int4[4])0;
  int4 tint_symbol_6[4] = {(1).xxxx, (2).xxxx, (3).xxxx, (3).xxxx};
  tint_symbol = tint_symbol_6;
  tint_symbol = src_param;
  tint_symbol = ret_arr();
  int4 src_let[4] = (int4[4])0;
  tint_symbol = src_let;
  tint_symbol = src_function;
  tint_symbol = src_private;
  tint_symbol = src_workgroup;
  S tint_symbol_1 = ret_struct_arr();
  tint_symbol = tint_symbol_1.arr;
  tint_symbol = src_uniform_load(0u);
  tint_symbol = src_storage_load(0u);
  int dst_nested[4][3][2] = (int[4][3][2])0;
  int src_nested[4][3][2] = (int[4][3][2])0;
  dst_nested = src_nested;
}

struct tint_symbol_3 {
  uint local_invocation_index : SV_GroupIndex;
};

void main_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  int4 val[4] = (int4[4])0;
  foo(val);
}

[numthreads(1, 1, 1)]
void main(tint_symbol_3 tint_symbol_2) {
  main_inner(tint_symbol_2.local_invocation_index);
  return;
}
