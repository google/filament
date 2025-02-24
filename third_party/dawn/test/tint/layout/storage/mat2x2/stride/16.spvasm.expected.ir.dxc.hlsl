struct strided_arr {
  float2 el;
};


RWByteAddressBuffer ssbo : register(u0);
typedef strided_arr ary_ret[2];
ary_ret mat2x2_stride_16_to_arr(float2x2 m) {
  strided_arr v = {m[0u]};
  strided_arr v_1 = {m[1u]};
  strided_arr v_2[2] = {v, v_1};
  return v_2;
}

float2x2 arr_to_mat2x2_stride_16(strided_arr arr[2]) {
  return float2x2(arr[0u].el, arr[1u].el);
}

void v_3(uint offset, strided_arr obj) {
  ssbo.Store2((offset + 0u), asuint(obj.el));
}

void v_4(uint offset, strided_arr obj[2]) {
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 2u)) {
        break;
      }
      strided_arr v_7 = obj[v_6];
      v_3((offset + (v_6 * 16u)), v_7);
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
}

strided_arr v_8(uint offset) {
  strided_arr v_9 = {asfloat(ssbo.Load2((offset + 0u)))};
  return v_9;
}

typedef strided_arr ary_ret_1[2];
ary_ret_1 v_10(uint offset) {
  strided_arr a[2] = (strided_arr[2])0;
  {
    uint v_11 = 0u;
    v_11 = 0u;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 2u)) {
        break;
      }
      strided_arr v_13 = v_8((offset + (v_12 * 16u)));
      a[v_12] = v_13;
      {
        v_11 = (v_12 + 1u);
      }
      continue;
    }
  }
  strided_arr v_14[2] = a;
  return v_14;
}

void f_1() {
  strided_arr v_15[2] = v_10(0u);
  strided_arr v_16[2] = mat2x2_stride_16_to_arr(arr_to_mat2x2_stride_16(v_15));
  v_4(0u, v_16);
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
}

