struct strided_arr {
  float el;
};

struct strided_arr_1 {
  strided_arr el[3][2];
};


RWByteAddressBuffer s : register(u0);
void v(uint offset, strided_arr obj) {
  s.Store((offset + 0u), asuint(obj.el));
}

void v_1(uint offset, strided_arr obj[2]) {
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 2u)) {
        break;
      }
      strided_arr v_4 = obj[v_3];
      v((offset + (v_3 * 8u)), v_4);
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
}

void v_5(uint offset, strided_arr obj[3][2]) {
  {
    uint v_6 = 0u;
    v_6 = 0u;
    while(true) {
      uint v_7 = v_6;
      if ((v_7 >= 3u)) {
        break;
      }
      strided_arr v_8[2] = obj[v_7];
      v_1((offset + (v_7 * 16u)), v_8);
      {
        v_6 = (v_7 + 1u);
      }
      continue;
    }
  }
}

void v_9(uint offset, strided_arr_1 obj) {
  strided_arr v_10[3][2] = obj.el;
  v_5((offset + 0u), v_10);
}

void v_11(uint offset, strided_arr_1 obj[4]) {
  {
    uint v_12 = 0u;
    v_12 = 0u;
    while(true) {
      uint v_13 = v_12;
      if ((v_13 >= 4u)) {
        break;
      }
      strided_arr_1 v_14 = obj[v_13];
      v_9((offset + (v_13 * 128u)), v_14);
      {
        v_12 = (v_13 + 1u);
      }
      continue;
    }
  }
}

strided_arr v_15(uint offset) {
  strided_arr v_16 = {asfloat(s.Load((offset + 0u)))};
  return v_16;
}

typedef strided_arr ary_ret[2];
ary_ret v_17(uint offset) {
  strided_arr a[2] = (strided_arr[2])0;
  {
    uint v_18 = 0u;
    v_18 = 0u;
    while(true) {
      uint v_19 = v_18;
      if ((v_19 >= 2u)) {
        break;
      }
      strided_arr v_20 = v_15((offset + (v_19 * 8u)));
      a[v_19] = v_20;
      {
        v_18 = (v_19 + 1u);
      }
      continue;
    }
  }
  strided_arr v_21[2] = a;
  return v_21;
}

typedef strided_arr ary_ret_1[3][2];
ary_ret_1 v_22(uint offset) {
  strided_arr a[3][2] = (strided_arr[3][2])0;
  {
    uint v_23 = 0u;
    v_23 = 0u;
    while(true) {
      uint v_24 = v_23;
      if ((v_24 >= 3u)) {
        break;
      }
      strided_arr v_25[2] = v_17((offset + (v_24 * 16u)));
      a[v_24] = v_25;
      {
        v_23 = (v_24 + 1u);
      }
      continue;
    }
  }
  strided_arr v_26[3][2] = a;
  return v_26;
}

strided_arr_1 v_27(uint offset) {
  strided_arr v_28[3][2] = v_22((offset + 0u));
  strided_arr_1 v_29 = {v_28};
  return v_29;
}

typedef strided_arr_1 ary_ret_2[4];
ary_ret_2 v_30(uint offset) {
  strided_arr_1 a[4] = (strided_arr_1[4])0;
  {
    uint v_31 = 0u;
    v_31 = 0u;
    while(true) {
      uint v_32 = v_31;
      if ((v_32 >= 4u)) {
        break;
      }
      strided_arr_1 v_33 = v_27((offset + (v_32 * 128u)));
      a[v_32] = v_33;
      {
        v_31 = (v_32 + 1u);
      }
      continue;
    }
  }
  strided_arr_1 v_34[4] = a;
  return v_34;
}

void f_1() {
  strided_arr_1 x_19[4] = v_30(0u);
  strided_arr x_24[3][2] = v_22(384u);
  strided_arr x_28[2] = v_17(416u);
  float x_32 = asfloat(s.Load(424u));
  strided_arr_1 v_35[4] = (strided_arr_1[4])0;
  v_11(0u, v_35);
  s.Store(424u, asuint(5.0f));
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
}

