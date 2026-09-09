// RUN: %dxc -T lib_6_3 -HV 202x -verify %s

// In HLSL 202x mode without -enable-16bit-types, the 16-bit suffix must error
// while f32/f64 are accepted silently.

float test_f32() {
  return 1.0f32; // no diagnostic
}

double test_f64() {
  return 1.0f64; // no diagnostic
}

min16float test_f16() {
  // expected-error@+1{{16-bit floating-point literal suffix 'f16' requires '-enable-16bit-types'}}
  return 1.0f16;
}

min16float test_F16() {
  // expected-error@+1{{16-bit floating-point literal suffix 'F16' requires '-enable-16bit-types'}}
  return 1.0F16;
}
