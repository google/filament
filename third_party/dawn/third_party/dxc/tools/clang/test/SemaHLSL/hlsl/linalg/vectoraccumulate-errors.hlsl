// XFAIL: *
// RUN: %dxc -I %hlsl_headers -T lib_6_9 %s | FileCheck %s

#include <dx/linalg.h>

RWByteAddressBuffer RWBuf;

export void Test5(vector<float, 128> Input) {
  using namespace dx::linalg;

  RWBuf.Store<vector<half, 128> >(0, Input);

  // PREVIEW CHECK TODO:
  // CHECK: Something about an error due to illegal conversions
  VectorAccumulate(Input, RWBuf, 0);
}
