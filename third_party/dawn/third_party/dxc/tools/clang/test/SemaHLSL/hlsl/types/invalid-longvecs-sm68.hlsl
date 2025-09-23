// RUN: %dxc -T ps_6_8 -verify %s

#define TYPE float
#define NUM 5

StructuredBuffer<vector<TYPE,NUM> > sbuf; // expected-error{{invalid value, valid range is between 1 and 4 inclusive}}

struct LongVec {
  float4 f;
  vector<TYPE,NUM> vec; // expected-error{{invalid value, valid range is between 1 and 4 inclusive}}
};
groupshared vector<TYPE, NUM> gs_vec; // expected-error{{invalid value, valid range is between 1 and 4 inclusive}}
groupshared vector<TYPE, NUM> gs_vec_arr[10]; // expected-error{{invalid value, valid range is between 1 and 4 inclusive}}

static vector<TYPE, NUM> static_vec; // expected-error{{invalid value, valid range is between 1 and 4 inclusive}}
static vector<TYPE, NUM> static_vec_arr[10]; // expected-error{{invalid value, valid range is between 1 and 4 inclusive}}

export vector<TYPE, NUM> lv_param_passthru( // expected-error{{invalid value, valid range is between 1 and 4 inclusive}}
                                           vector<TYPE, NUM> vec1) { // expected-error{{invalid value, valid range is between 1 and 4 inclusive}}
  vector<TYPE, NUM> ret = vec1; // expected-error{{invalid value, valid range is between 1 and 4 inclusive}}
  vector<TYPE, NUM> arr[10]; // expected-error{{invalid value, valid range is between 1 and 4 inclusive}}
  arr[1]= vec1;
  return ret;
}

export void lv_param_in_out(in vector<TYPE, NUM> vec1, // expected-error{{invalid value, valid range is between 1 and 4 inclusive}}
                            out vector<TYPE, NUM> vec2) { // expected-error{{invalid value, valid range is between 1 and 4 inclusive}}
  vec2 = vec1;
}

export void lv_param_inout(inout vector<TYPE, NUM> vec1, // expected-error{{invalid value, valid range is between 1 and 4 inclusive}}
                           inout vector<TYPE, NUM> vec2) { // expected-error{{invalid value, valid range is between 1 and 4 inclusive}}
  vector<TYPE, NUM> tmp = vec1; // expected-error{{invalid value, valid range is between 1 and 4 inclusive}}
  vec1 = vec2;
  vec2 = tmp;
}
