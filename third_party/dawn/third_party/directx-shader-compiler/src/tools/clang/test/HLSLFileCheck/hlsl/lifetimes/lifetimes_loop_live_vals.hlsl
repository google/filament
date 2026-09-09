// RUN: %dxc -T lib_6_6 %s | FileCheck %s

// Regression tests where enabling lifetimes caused some inefficiencies due to
// missing cleanup optimizations.
// The first two tests are modeled to make it easy to compare against stock
// LLVM: The code is converted easily to standard C++.

//------------------------------------------------------------------------------
// CHECK: define void @"\01?test{{[@$?.A-Za-z0-9_]+}}"
// CHECK-NOT: undef
bool done(int);
float loop_code();

void fn(in int loopCount, out float res) {
  for (int i = 0; i < loopCount; i++) {
    float f = loop_code();
    if (done(i)) {
      res = f;
      return;
    }
  }
  res = 1;
}

export
void test(in int loopCount, out float res) {
  res = 0;
  float f;
  fn(loopCount, f);
  if (f > 0)
    res = f;
}

//------------------------------------------------------------------------------
// CHECK: define void @"\01?fn2{{[@$?.A-Za-z0-9_]+}}"
// CHECK-NOT: undef
export
void fn2(in int loopCount, out float res) {
  for (int i = 0; i < loopCount; i++) {
    float f = loop_code();
    if (done(i)) {
      res = f;
      return;
    }
    if (done(-i)) {
      res = 2;
      return;
    }
  }
  res = 1;
}

export
void test2(in int loopCount, out float res) {
  float f;
  fn2(loopCount, f);
  res = f;
}

//------------------------------------------------------------------------------
// There must not be any phi with undef (or any undef in general) in the final
// code.
// There can be 'undef' in the metadata, so we limit the check until metadata
// starts.

// CHECK: define void @"\01?main{{[@$?.A-Za-z0-9_]+}}"
// CHECK-NOT: undef
// CHECK: !dx.version
int loopCountGlobal;

void fn3(out float res) {
  for (int i = 0; i < loopCountGlobal; i++) {
    float f = loop_code();
    if (done(i)) {
      res = f;
      return;
    }
    if (done(-i)) {
      res = 2;
      return;
    }
  }
  res = 1;
}

export
void main(out float res : OUT) {
  float f;
  fn3(f);
  res = f;
}
