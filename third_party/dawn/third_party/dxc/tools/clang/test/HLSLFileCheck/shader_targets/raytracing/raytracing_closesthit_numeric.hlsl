// RUN: %dxc -enable-16bit-types -T lib_6_3 -auto-binding-space 11 %s | FileCheck %s

// CHECK: define void @"\01?closesthit_numeric

struct MyNumericTest {
  float f;
  int i;
  uint u;
  half h;
  int16_t i16;
  float2 f2;
  double d;
};

struct MyPayload {
  MyNumericTest t;
};

struct MyAttributes {
  MyNumericTest t;
};

[shader("closesthit")]
void closesthit_numeric( inout MyPayload payload, MyAttributes attr ) {}
