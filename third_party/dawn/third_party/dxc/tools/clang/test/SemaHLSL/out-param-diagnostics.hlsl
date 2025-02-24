// RUN: %dxc -Tlib_6_3  -verify %s

void UnusedEmpty(out int Val) {} // expected-warning{{parameter 'Val' is uninitialized when used here}} expected-note{{variable 'Val' is declared here}}

// Neither of these should warn
void UnusedInAndOut(in out int Val) {}
void UnusedInOut(inout int Val) {}


int Returned(out int Val) { // expected-note{{variable 'Val' is declared here}}
  return Val; // expected-warning{{parameter 'Val' is uninitialized when used here}}
}

int ReturnedPassthrough(int Cond, out int Val) { // expected-note{{variable 'Val' is declared here}}
  if (Cond % 3)
    return Returned(Val);
  else if (Cond % 2)
    return Returned(Val);
  return Val; // expected-warning{{parameter 'Val' is uninitialized when used here}}
}

// No disagnostic expected here because all paths to the exit return, and they
// all initialize Val.
int AllPathsReturn(int Cond, out int Val) {
  if (Cond % 3)
    return Returned(Val);
  else
    return Returned(Val);
}

void AllPathsReturnSwitch(int Cond, out int Val) {
  switch(Cond % 3) {
    case 0:
      Val = 0;
      return;
    case 1:
      Val = 1;
      return;
    case 2:
      Val = 2;
      return;
  }
}

int ReturnedMaybePassthrough(int Cond, out int Val) { // expected-note{{variable 'Val' is declared here}}
  if (Cond % 3)
    UnusedEmpty(Val);
  else if (Cond % 2) // expected-warning{{parameter 'Val' is used uninitialized whenever 'if' condition is false}} expected-note{{remove the 'if' if its condition is always true}}
    UnusedEmpty(Val);
  return Val; // expected-note{{uninitialized use occurs here}}
}

void SomePathsReturnSwitch(int Cond, out int Val) { // expected-note{{variable 'Val' is declared here}}
  switch(Cond) {
    case 0:
      Val = 0;
      return;
    default: // expected-warning{{parameter 'Val' is used uninitialized whenever switch default is taken}}
      break;
  }
}  // expected-note{{uninitialized use occurs here}}

void SomePathsReturnSwitch2(int Cond, out int Val) { // expected-note{{variable 'Val' is declared here}}
  switch(Cond) {
    case 0:
      Val = 0;
      return;
    case 1:
      return; // expected-warning{{parameter 'Val' is uninitialized when used here}}
    default:
      Val = 0;
      break;
  }
}

int Dbl(int V) {
  return V + V;
}

int UsedAsIn(out int Num) { // expected-note{{variable 'Num' is declared here}}
  return Dbl(Num); // expected-warning{{parameter 'Num' is uninitialized when used here}}
}

// No diagnostic for this one either!
int GetOne(out int O) {
  return O = 1;
}

// Both of these functions should not produce diagnostics because inout and in +
// out specifiers are ignored by the analysis.
void DblInPlace(in out int V) {
  V += V;
}

void DblInPlace2(inout int V) {
  V += V;
}

void MaybePassthrough(int Cond, out int Val) { // expected-note{{variable 'Val' is declared here}}
  if (Cond % 3)
    UnusedEmpty(Val);
  else if (Cond % 2) // expected-warning{{parameter 'Val' is used uninitialized whenever 'if' condition is false}} expected-note{{remove the 'if' if its condition is always true}}
    UnusedEmpty(Val);
} // expected-note{{uninitialized use occurs here}}

void EarlyOut(int Cond, out int Val) { // expected-note{{variable 'Val' is declared here}}
  if (Cond % 11)
    return; // expected-warning {{parameter 'Val' is uninitialized when used here}}
  Val = 1;
}

// In parameters are read from, so they should be treated as uninitialized
// values. Out parameters are written to but not read from, so they are
// initializers.

void SomethingCalledOut(out int V) {
  V = 1;
}

int Something1(out int Num) {
  // no diagnostic since this writes Num but doesn't read it
  SomethingCalledOut(Num);
  return Num;
}


void SomethingCalledInAndOut(in out int V) {
  V = 1;
}

int Something2(out int Num) { // expected-note {{variable 'Num' is declared here}}
  SomethingCalledInAndOut(Num); // expected-warning {{parameter 'Num' is uninitialized when used here}}
  return Num;
}

void SomethingCalledInOut(inout int V) {
  V = 1;
}

int Something3(out int Num) { // expected-note {{variable 'Num' is declared here}}
  SomethingCalledInOut(Num); // expected-warning {{parameter 'Num' is uninitialized when used here}}
  return Num;
}

struct SomeObj {
  int Integer;
  int Float;
};

// No errors are generated for struct types :(
void UnusedObjectOut(out SomeObj V) {}

// This test case is copied from tools/clang/test/HLSL/functions.hlsl to verify
// that the analysis does produce a diagnostic for this case. Because
// analysis-based warnings require valid ASTs, they don't run in the presence of
// errors. As a result that test doesn't produce these diagnostics.
void fn_uint_oload3(uint u) { }
void fn_uint_oload3(inout uint u) { }
void fn_uint_oload3(out uint u) { } // expected-warning {{parameter 'u' is uninitialized when used here}} expected-note{{variable 'u' is declared here}}

// Verify attribute annotation to opt out of uninitialized parameter analysis.
void UnusedOutput([maybe_unused] out int Val) {}

void UsedMaybeOutput([maybe_unused] out int Val) { // expected-note{{variable 'Val' is declared here}}
  Val += Val; // expected-warning{{parameter 'Val' is uninitialized when used here}}
}

void MaybeUsedMaybeUnused([maybe_unused] out int Val, int Cnt) { // expected-note{{variable 'Val' is declared here}}
  if (Cnt % 2) // expected-warning{{parameter 'Val' is used uninitialized whenever 'if' condition is fals}} expected-note{{remove the 'if' if its condition is always true}}
    Val = 1;
} // expected-note{{uninitialized use occurs here}}

void Use(int V) {}

void NoAnnotationIsUse(out int V) { // expected-note{{variable 'V' is declared here}}
  Use(V); // expected-warning{{parameter 'V' is uninitialized when used here}}
}

RWByteAddressBuffer buffer;

// No expected diagnostic here. InterlockedAdd is not annotated with HLSL
// parameter annotations, so we fall back to C/C++ rules, which don't treat
// reference passed parameters as uses.
void interlockWrapper(out uint original) {
  buffer.InterlockedAdd(16, 1, original);
}

// Neither of these will warn because we don't support element-based tracking.
void UnusedSizedArray(out uint u[2]) { }
void UnusedUnsizedArray(out uint u[]) { }

// Warnings for struct types are not supported yet.
struct S { uint a; uint b; };

void Initializes(out S a) {
  a.a = 1;
  a.b = 1;
}

void InitializesIndirectly(out S a) {
  Initializes(a);
}
