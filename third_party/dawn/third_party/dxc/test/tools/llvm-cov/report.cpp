// RUN: llvm-cov report %S/Inputs/report.covmapping -instr-profile %S/Inputs/report.profdata -filename-equivalence 2>&1 | FileCheck %s
// RUN: llvm-cov report %S/Inputs/report.covmapping -instr-profile %S/Inputs/report.profdata -filename-equivalence report.cpp 2>&1 | FileCheck -check-prefix=FILT-NEXT %s

// CHECK:      Filename   Regions  Miss   Cover  Functions  Executed
// CHECK-NEXT: ---
// CHECK-NEXT: report.cpp       5     2  60.00%          4    75.00%
// CHECK-NEXT: ---
// CHECK-NEXT: TOTAL            5     2  60.00%          4    75.00%

// FILT: File 'report.cpp':
// FILT-NEXT: Name        Regions  Miss   Cover  Lines  Miss   Cover
// FILT-NEXT: ---
// FILT-NEXT: _Z3foob           2     1  50.00%      4     2  50.00%
// FILT-NEXT: _Z3barv           1     0 100.00%      2     0 100.00%
// FILT-NEXT: _Z4funcv          1     1   0.00%      2     2   0.00%
// FILT-NEXT: main              1     0 100.00%      5     0 100.00%
// FILT-NEXT: ---
// FILT-NEXT: TOTAL             5     2  60.00%     13     4  69.23%

void foo(bool cond) {
  if (cond) {
  }
}

void bar() {
}

void func() {
}

int main() {
  foo(false);
  bar();
  return 0;
}
