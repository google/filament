// RUN: %dxc /T vs_6_2 /E main %s | FileCheck %s

// Regression test for GitHub #1947, where matrix input parameters were expected to have
// exactly one use in HLSignatureLower, instead of zero or one, leading to a crash.

// CHECK: ret void

void main(int2x2 mat : IN) {}