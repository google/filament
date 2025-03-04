// This file tests the -Rpass family of flags (-Rpass, -Rpass-missed
// and -Rpass-analysis) with the inliner. The test is designed to
// always trigger the inliner, so it should be independent of the
// optimization level.

// RUN: %clang_cc1 %s -Rpass=inline -Rpass-analysis=inline -Rpass-missed=inline -O0 -emit-llvm-only -verify
// RUN: %clang_cc1 %s -Rpass=inline -Rpass-analysis=inline -Rpass-missed=inline -O0 -emit-llvm-only -gline-tables-only -verify
// RUN: %clang_cc1 %s -Rpass=inline -emit-llvm -o - 2>/dev/null | FileCheck %s
//
// Check that we can override -Rpass= with -Rno-pass.
// RUN: %clang_cc1 %s -Rpass=inline -emit-llvm -o - 2>&1 | FileCheck %s --check-prefix=CHECK-REMARKS
// RUN: %clang_cc1 %s -Rpass=inline -Rno-pass -emit-llvm -o - 2>&1 | FileCheck %s --check-prefix=CHECK-NO-REMARKS
// RUN: %clang_cc1 %s -Rpass=inline -Rno-everything -emit-llvm -o - 2>&1 | FileCheck %s --check-prefix=CHECK-NO-REMARKS
// RUN: %clang_cc1 %s -Rpass=inline -Rno-everything -Reverything -emit-llvm -o - 2>&1 | FileCheck %s --check-prefix=CHECK-REMARKS
//
// FIXME: -Reverything should imply -Rpass=.*.
// RUN: %clang_cc1 %s -Reverything -emit-llvm -o - 2>/dev/null | FileCheck %s --check-prefix=CHECK-NO-REMARKS
//
// FIXME: -Rpass should either imply -Rpass=.* or should be rejected.
// RUN: %clang_cc1 %s -Rpass -emit-llvm -o - 2>/dev/null | FileCheck %s --check-prefix=CHECK-NO-REMARKS

// CHECK-REMARKS: remark:
// CHECK-NO-REMARKS-NOT: remark:

// -Rpass should produce source location annotations, exclusively (just
// like -gmlt).
// CHECK: , !dbg !
// CHECK-NOT: DW_TAG_base_type

// But llvm.dbg.cu should be missing (to prevent writing debug info to
// the final output).
// CHECK-NOT: !llvm.dbg.cu = !{

int foo(int x, int y) __attribute__((always_inline));
int foo(int x, int y) { return x + y; }

float foz(int x, int y) __attribute__((noinline));
float foz(int x, int y) { return x * y; }

// The negative diagnostics are emitted twice because the inliner runs
// twice.
//
int bar(int j) {
// expected-remark@+6 {{foz should never be inlined (cost=never)}}
// expected-remark@+5 {{foz will not be inlined into bar}}
// expected-remark@+4 {{foz should never be inlined}}
// expected-remark@+3 {{foz will not be inlined into bar}}
// expected-remark@+2 {{foo should always be inlined}}
// expected-remark@+1 {{foo inlined into bar}}
  return foo(j, j - 2) * foz(j - 2, j);
}
