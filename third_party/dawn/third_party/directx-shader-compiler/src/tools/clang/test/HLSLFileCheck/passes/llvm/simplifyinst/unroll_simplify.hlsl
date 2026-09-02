// RUN: %dxc -T ps_6_6 %s     -enable-lifetime-markers /HV 2021 | FileCheck %s
// RUN: %dxc -T ps_6_6 %s /Od -enable-lifetime-markers /HV 2021 | FileCheck %s

// CHECK: @main

//
// Regression check for loop unrolling failure. Running instcombine when
// lifetime markers are enabled rewrote the IR in a way that ScalarEvolution
// could no longer figure out an upper bound. The expression:
//
//   limit = (l % 2) ? 1 : 2;
//
// Got rewritten to
//
//   limit = 2 - (l & 1);
//
// This fix for this was switching from running instcombine to running
// simplifyinst, which is less aggressive in its rewriting.
//

cbuffer cb : register(b0) {
  float foo[2];
};

[RootSignature("CBV(b0)")]
float main(uint l : L) : SV_Target {
  uint limit = (l % 2) ? 1 : 2;
  float ret = 0;
  [unroll]
  for (uint i = 0; i < limit; i++) {
    ret += foo[i];
  }
  return ret;
}

