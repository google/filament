// RUN: %dxc -T lib_6_3 -HV 2021 -verify %s

// In HLSL 2021 and earlier, the fixed-size float suffixes f16/f32/f64 should
// still work, but produce an extension warning. With -enable-16bit-types not
// passed, the 16-bit suffix should also error.

template <typename T, typename U>
struct is_same {
  static const bool value = false;
};

template <typename T>
struct is_same<T, T> {
  static const bool value = true;
};

float test_f32() {
  // expected-warning@+1{{fixed-size floating-point literal suffix 'f32' is a HLSL 202x extension}}
  return 1.0f32;
}

float test_F32() {
  // expected-warning@+1{{fixed-size floating-point literal suffix 'F32' is a HLSL 202x extension}}
  return 1.0F32;
}

double test_f64() {
  // expected-warning@+1{{fixed-size floating-point literal suffix 'f64' is a HLSL 202x extension}}
  return 1.0f64;
}

double test_F64() {
  // expected-warning@+1{{fixed-size floating-point literal suffix 'F64' is a HLSL 202x extension}}
  return 1.0F64;
}

// Without -enable-16bit-types the 16-bit suffix must error (in addition to the
// extension warning).
min16float test_f16() {
  // expected-warning@+2{{fixed-size floating-point literal suffix 'f16' is a HLSL 202x extension}}
  // expected-error@+1{{16-bit floating-point literal suffix 'f16' requires '-enable-16bit-types'}}
  return 1.0f16;
}

min16float test_F16() {
  // expected-warning@+2{{fixed-size floating-point literal suffix 'F16' is a HLSL 202x extension}}
  // expected-error@+1{{16-bit floating-point literal suffix 'F16' requires '-enable-16bit-types'}}
  return 1.0F16;
}
