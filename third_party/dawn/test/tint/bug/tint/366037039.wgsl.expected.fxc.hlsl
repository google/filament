[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct S {
  uint3 a;
  uint b;
  uint3 c[4];
};

cbuffer cbuffer_ubuffer : register(b0) {
  uint4 ubuffer[5];
};
RWByteAddressBuffer sbuffer : register(u1);
groupshared S wbuffer;

typedef uint3 ubuffer_load_3_ret[4];
ubuffer_load_3_ret ubuffer_load_3(uint offset) {
  uint3 arr[4] = (uint3[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      const uint scalar_offset = ((offset + (i * 16u))) / 4;
      arr[i] = ubuffer[scalar_offset / 4].xyz;
    }
  }
  return arr;
}

S ubuffer_load(uint offset) {
  const uint scalar_offset_1 = ((offset + 0u)) / 4;
  const uint scalar_offset_2 = ((offset + 12u)) / 4;
  S tint_symbol = {ubuffer[scalar_offset_1 / 4].xyz, ubuffer[scalar_offset_2 / 4][scalar_offset_2 % 4], ubuffer_load_3((offset + 16u))};
  return tint_symbol;
}

typedef uint3 sbuffer_load_3_ret[4];
sbuffer_load_3_ret sbuffer_load_3(uint offset) {
  uint3 arr_1[4] = (uint3[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr_1[i_1] = sbuffer.Load3((offset + (i_1 * 16u)));
    }
  }
  return arr_1;
}

S sbuffer_load(uint offset) {
  S tint_symbol_1 = {sbuffer.Load3((offset + 0u)), sbuffer.Load((offset + 12u)), sbuffer_load_3((offset + 16u))};
  return tint_symbol_1;
}

void sbuffer_store_3(uint offset, uint3 value[4]) {
  uint3 array_1[4] = value;
  {
    for(uint i_2 = 0u; (i_2 < 4u); i_2 = (i_2 + 1u)) {
      sbuffer.Store3((offset + (i_2 * 16u)), asuint(array_1[i_2]));
    }
  }
}

void sbuffer_store(uint offset, S value) {
  sbuffer.Store3((offset + 0u), asuint(value.a));
  sbuffer.Store((offset + 12u), asuint(value.b));
  sbuffer_store_3((offset + 16u), value.c);
}

void foo() {
  S u = ubuffer_load(0u);
  S s = sbuffer_load(0u);
  S w = sbuffer_load(0u);
  S tint_symbol_2 = (S)0;
  sbuffer_store(0u, tint_symbol_2);
  S tint_symbol_3 = (S)0;
  wbuffer = tint_symbol_3;
}
