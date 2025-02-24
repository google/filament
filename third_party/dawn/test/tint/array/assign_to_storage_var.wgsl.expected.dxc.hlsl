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
RWByteAddressBuffer tint_symbol : register(u2);
RWByteAddressBuffer dst_nested : register(u3);

typedef int4 ret_arr_ret[4];
ret_arr_ret ret_arr() {
  int4 tint_symbol_4[4] = (int4[4])0;
  return tint_symbol_4;
}

S ret_struct_arr() {
  S tint_symbol_5 = (S)0;
  return tint_symbol_5;
}

void tint_symbol_store(uint offset, int4 value[4]) {
  int4 array_1[4] = value;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      tint_symbol.Store4((offset + (i_1 * 16u)), asuint(array_1[i_1]));
    }
  }
}

typedef int4 src_uniform_load_ret[4];
src_uniform_load_ret src_uniform_load(uint offset) {
  int4 arr_1[4] = (int4[4])0;
  {
    for(uint i_2 = 0u; (i_2 < 4u); i_2 = (i_2 + 1u)) {
      const uint scalar_offset = ((offset + (i_2 * 16u))) / 4;
      arr_1[i_2] = asint(src_uniform[scalar_offset / 4]);
    }
  }
  return arr_1;
}

typedef int4 src_storage_load_ret[4];
src_storage_load_ret src_storage_load(uint offset) {
  int4 arr_2[4] = (int4[4])0;
  {
    for(uint i_3 = 0u; (i_3 < 4u); i_3 = (i_3 + 1u)) {
      arr_2[i_3] = asint(src_storage.Load4((offset + (i_3 * 16u))));
    }
  }
  return arr_2;
}

void dst_nested_store_2(uint offset, int value[2]) {
  int array_4[2] = value;
  {
    for(uint i_4 = 0u; (i_4 < 2u); i_4 = (i_4 + 1u)) {
      dst_nested.Store((offset + (i_4 * 4u)), asuint(array_4[i_4]));
    }
  }
}

void dst_nested_store_1(uint offset, int value[3][2]) {
  int array_3[3][2] = value;
  {
    for(uint i_5 = 0u; (i_5 < 3u); i_5 = (i_5 + 1u)) {
      dst_nested_store_2((offset + (i_5 * 8u)), array_3[i_5]);
    }
  }
}

void dst_nested_store(uint offset, int value[4][3][2]) {
  int array_2[4][3][2] = value;
  {
    for(uint i_6 = 0u; (i_6 < 4u); i_6 = (i_6 + 1u)) {
      dst_nested_store_1((offset + (i_6 * 24u)), array_2[i_6]);
    }
  }
}

void foo(int4 src_param[4]) {
  int4 src_function[4] = (int4[4])0;
  int4 tint_symbol_6[4] = {(1).xxxx, (2).xxxx, (3).xxxx, (3).xxxx};
  tint_symbol_store(0u, tint_symbol_6);
  tint_symbol_store(0u, src_param);
  tint_symbol_store(0u, ret_arr());
  int4 src_let[4] = (int4[4])0;
  tint_symbol_store(0u, src_let);
  tint_symbol_store(0u, src_function);
  tint_symbol_store(0u, src_private);
  tint_symbol_store(0u, src_workgroup);
  S tint_symbol_1 = ret_struct_arr();
  tint_symbol_store(0u, tint_symbol_1.arr);
  tint_symbol_store(0u, src_uniform_load(0u));
  tint_symbol_store(0u, src_storage_load(0u));
  int src_nested[4][3][2] = (int[4][3][2])0;
  dst_nested_store(0u, src_nested);
}

struct tint_symbol_3 {
  uint local_invocation_index : SV_GroupIndex;
};

void main_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  int4 ary[4] = (int4[4])0;
  foo(ary);
}

[numthreads(1, 1, 1)]
void main(tint_symbol_3 tint_symbol_2) {
  main_inner(tint_symbol_2.local_invocation_index);
  return;
}
