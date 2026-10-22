// RUN: %dxc -T lib_6_9 -DTYPE=float -DNUM=1025 -verify %s
// RUN: %dxc -T ps_6_9  -DTYPE=float -DNUM=1025 -verify %s

// A test to verify that declarations of longvecs are permitted in all the accepted places.
// Only tests for acceptance, most codegen is ignored for now.

struct LongVec {
  float4 f;
  vector<TYPE,NUM> vec; // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
};

template <int N>
struct LongVecTpl {
  float4 f;
  vector<TYPE,N> vec; // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
};

template <int N>
struct LongVecTpl2 {
  float4 f;
  vector<TYPE,N> vec; // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
};

groupshared vector<TYPE, NUM> gs_vec; // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
groupshared vector<TYPE, NUM> gs_vec_arr[10]; // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
groupshared LongVecTpl<NUM> gs_vec_tpl; // expected-note{{in instantiation of template class 'LongVecTpl<1025>' requested here}}

static vector<TYPE, NUM> static_vec; // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
static vector<TYPE, NUM> static_vec_arr[10]; // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
static LongVecTpl2<NUM> static_vec_tpl; // expected-note{{in instantiation of template class 'LongVecTpl2<1025>' requested here}}

export vector<TYPE, NUM> // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
lv_param_passthru(vector<TYPE, NUM> vec1) { // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
  vector<TYPE, NUM> ret = vec1; // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
  return ret;
}

export void lv_param_in_out(in vector<TYPE, NUM> vec1, // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
                            out vector<TYPE, NUM> vec2) { // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
  vec2 = vec1;
}

export void lv_param_inout(inout vector<TYPE, NUM> vec1, // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
                           inout vector<TYPE, NUM> vec2) { // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
  vector<TYPE, NUM> tmp = vec1; // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
  vec1 = vec2;
  vec2 = tmp;
}

export void lv_global_assign(vector<TYPE, NUM> vec) { // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
  static_vec = vec;
}

export vector<TYPE, NUM> lv_global_ret() { // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
  vector<TYPE, NUM> ret = static_vec; // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
  return ret;
}

export void lv_gs_assign(vector<TYPE, NUM> vec) { // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
  gs_vec = vec;
}

export vector<TYPE, NUM> lv_gs_ret() { // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
  vector<TYPE, NUM> ret = gs_vec; // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
  return ret;
}

#define DIMS 10

export vector<TYPE, NUM> // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
lv_param_arr_passthru(vector<TYPE, NUM> vec)[10] { // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
  vector<TYPE, NUM> ret[10]; // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
  for (int i = 0; i < DIMS; i++)
    ret[i] = vec;
  return ret;
}

export void lv_global_arr_assign(vector<TYPE, NUM> vec[10]) { // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
  for (int i = 0; i < DIMS; i++)
    static_vec_arr[i] = vec[i];
}

export vector<TYPE, NUM> lv_global_arr_ret()[10] { // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
  vector<TYPE, NUM> ret[10]; // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
  for (int i = 0; i < DIMS; i++)
    ret[i] = static_vec_arr[i];
  return ret;
}

export void lv_gs_arr_assign(vector<TYPE, NUM> vec[10]) { // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
  for (int i = 0; i < DIMS; i++)
    gs_vec_arr[i] = vec[i];
}

export vector<TYPE, NUM> lv_gs_arr_ret()[10] { // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
  vector<TYPE, NUM> ret[10]; // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
  for (int i = 0; i < DIMS; i++)
    ret[i] = gs_vec_arr[i];
  return ret;
}

export LongVec lv_param_rec_passthru(LongVec vec) {
  LongVec ret = vec;
  return ret;
}

export vector<TYPE,NUM> lv_splat(TYPE scalar) { // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
  vector<TYPE,NUM> ret = scalar; // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
  return ret;
}

export vector<TYPE, NUM> lv_array_cast(TYPE arr[NUM]) { // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
  vector<TYPE, NUM> ret = (vector<TYPE,NUM>)arr; // expected-error{{invalid value, valid range is between 1 and 1024 inclusive}}
  return ret;
}

