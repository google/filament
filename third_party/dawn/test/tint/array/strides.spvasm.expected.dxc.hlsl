struct strided_arr {
  float el;
};
struct strided_arr_1 {
  strided_arr el[3][2];
};

RWByteAddressBuffer s : register(u0);

strided_arr s_load_4(uint offset) {
  strided_arr tint_symbol = {asfloat(s.Load((offset + 0u)))};
  return tint_symbol;
}

typedef strided_arr s_load_3_ret[2];
s_load_3_ret s_load_3(uint offset) {
  strided_arr arr[2] = (strided_arr[2])0;
  {
    for(uint i = 0u; (i < 2u); i = (i + 1u)) {
      arr[i] = s_load_4((offset + (i * 8u)));
    }
  }
  return arr;
}

typedef strided_arr s_load_2_ret[3][2];
s_load_2_ret s_load_2(uint offset) {
  strided_arr arr_1[3][2] = (strided_arr[3][2])0;
  {
    for(uint i_1 = 0u; (i_1 < 3u); i_1 = (i_1 + 1u)) {
      arr_1[i_1] = s_load_3((offset + (i_1 * 16u)));
    }
  }
  return arr_1;
}

strided_arr_1 s_load_1(uint offset) {
  strided_arr_1 tint_symbol_1 = {s_load_2((offset + 0u))};
  return tint_symbol_1;
}

typedef strided_arr_1 s_load_ret[4];
s_load_ret s_load(uint offset) {
  strided_arr_1 arr_2[4] = (strided_arr_1[4])0;
  {
    for(uint i_2 = 0u; (i_2 < 4u); i_2 = (i_2 + 1u)) {
      arr_2[i_2] = s_load_1((offset + (i_2 * 128u)));
    }
  }
  return arr_2;
}

void s_store_4(uint offset, strided_arr value) {
  s.Store((offset + 0u), asuint(value.el));
}

void s_store_3(uint offset, strided_arr value[2]) {
  strided_arr array_3[2] = value;
  {
    for(uint i_3 = 0u; (i_3 < 2u); i_3 = (i_3 + 1u)) {
      s_store_4((offset + (i_3 * 8u)), array_3[i_3]);
    }
  }
}

void s_store_2(uint offset, strided_arr value[3][2]) {
  strided_arr array_2[3][2] = value;
  {
    for(uint i_4 = 0u; (i_4 < 3u); i_4 = (i_4 + 1u)) {
      s_store_3((offset + (i_4 * 16u)), array_2[i_4]);
    }
  }
}

void s_store_1(uint offset, strided_arr_1 value) {
  s_store_2((offset + 0u), value.el);
}

void s_store(uint offset, strided_arr_1 value[4]) {
  strided_arr_1 array_1[4] = value;
  {
    for(uint i_5 = 0u; (i_5 < 4u); i_5 = (i_5 + 1u)) {
      s_store_1((offset + (i_5 * 128u)), array_1[i_5]);
    }
  }
}

void f_1() {
  strided_arr_1 x_19[4] = s_load(0u);
  strided_arr x_24[3][2] = s_load_2(384u);
  strided_arr x_28[2] = s_load_3(416u);
  float x_32 = asfloat(s.Load(424u));
  strided_arr_1 tint_symbol_2[4] = (strided_arr_1[4])0;
  s_store(0u, tint_symbol_2);
  s.Store(424u, asuint(5.0f));
  return;
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
  return;
}
