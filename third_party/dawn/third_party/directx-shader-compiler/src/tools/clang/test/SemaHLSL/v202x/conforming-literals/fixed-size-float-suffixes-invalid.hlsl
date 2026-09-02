// RUN: %dxc -T lib_6_3 -HV 202x -enable-16bit-types -verify %s

// Verify that float literal suffixes that look like fixed-size width
// suffixes but specify an unsupported width are rejected. Only 16, 32 and
// 64 are valid widths; everything else must produce an
// "invalid suffix on floating constant" error.

float plausible_f8() {
  // 'f8' would be an 8-bit float, which we don't support.
  // expected-error@+1{{invalid suffix 'f8' on floating constant}}
  return 1.0f8;
}

float plausible_F8() {
  // expected-error@+1{{invalid suffix 'F8' on floating constant}}
  return 1.0F8;
}

float plausible_f128() {
  // 'f128' would be a 128-bit float, which we don't support.
  // expected-error@+1{{invalid suffix 'f128' on floating constant}}
  return 1.0f128;
}

float plausible_F128() {
  // expected-error@+1{{invalid suffix 'F128' on floating constant}}
  return 1.0F128;
}

float implausible_f23() {
  // 'f23' is not a meaningful width but the parser should still reject it.
  // expected-error@+1{{invalid suffix 'f23' on floating constant}}
  return 1.0f23;
}

float implausible_f7() {
  // 'f7' is not a meaningful width but the parser should still reject it.
  // expected-error@+1{{invalid suffix 'f7' on floating constant}}
  return 1.0f7;
}

float implausible_f0() {
  // expected-error@+1{{invalid suffix 'f0' on floating constant}}
  return 1.0f0;
}

float implausible_f31() {
  // One off from a valid width.
  // expected-error@+1{{invalid suffix 'f31' on floating constant}}
  return 1.0f31;
}

float implausible_f33() {
  // expected-error@+1{{invalid suffix 'f33' on floating constant}}
  return 1.0f33;
}

float trailing_garbage_after_valid_width() {
  // A valid fixed-size suffix followed by extra characters is still an
  // invalid suffix.
  // expected-error@+1{{invalid suffix 'f32x' on floating constant}}
  return 1.0f32x;
}

float trailing_number_after_valid_width() {
  // A valid fixed-size suffix followed by an extra number is still an invalid
  // suffix.
  // expected-error@+1{{invalid suffix 'f324' on floating constant}}
  return 1.0f324;
}
