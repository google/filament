// RUN: %dxc -E main -T cs_6_2 %s | FileCheck %s

// Regression test for GitHub #1631, where SROA would generate more uses
// of a value while processing it (due to expanding a memcpy) and fail
// to process the new uses. This caused global structs of matrices to reach HLMatrixLower,
// which couldn't handle them and would unexpectedly leave matrix intrinsics untouched.
// Compilation would then fail with "error: Fail to lower matrix load/store."

// CHECK: ret void

struct S { int1x1 x, y; };
groupshared S gs[1];
void f(S s[1]) {}
[numthreads(1,1,1)]
void main() { f(gs); }