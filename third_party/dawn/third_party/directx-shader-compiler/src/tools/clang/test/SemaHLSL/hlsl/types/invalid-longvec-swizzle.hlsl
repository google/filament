// RUN: %dxc -Tlib_6_9 -verify %s -DTYPE=float
// RUN: %dxc -Tlib_6_9 -verify %s -DTYPE=bool
// RUN: %dxc -Tlib_6_9 -verify %s -DTYPE=uint64_t
// RUN: %dxc -Tlib_6_9 -verify %s -DTYPE=double
// RUN: %dxc -Tlib_6_9 -verify %s -enable-16bit-types -DTYPE=float16_t
// RUN: %dxc -Tlib_6_9 -verify %s -enable-16bit-types -DTYPE=int16_t

export
vector<double, 3> doit(vector<double, 5> vec5) {
  vec5.x = 1; // expected-error {{invalid swizzle 'x' on vector of over 4 elements.}}
  return vec5.xyw; // expected-error {{invalid swizzle 'xyw' on vector of over 4 elements.}}
}

export
TYPE arr_to_vec(TYPE arr[5]) {

  TYPE val = (vector<TYPE, 6>(arr, 1)).x; // expected-error {{invalid swizzle 'x' on vector of over 4 elements.}}

  TYPE val2 = ((vector<TYPE, 5>)arr).x; // expected-error {{invalid swizzle 'x' on vector of over 4 elements.}}

  return val;
}

export TYPE lv_ctor(TYPE s) {
  TYPE ret = (vector<TYPE,6>(1, 2, 3, 4, 5, s)).x; // expected-error {{invalid swizzle 'x' on vector of over 4 elements.}}
  return ret;
}