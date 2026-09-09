// RUN: %dxc -T lib_6_3 -HV 202x -verify %s

groupshared uint SharedData;

void fn1(inout uint Sh) {
  Sh = 5;
}

void fn2(inout groupshared uint Sh);
// expected-error@-1{{'inout' and 'groupshared' cannot be used together for a parameter}}
void fn3(in groupshared uint Sh);
// expected-error@-1{{'in' and 'groupshared' cannot be used together for a parameter}}
void fn4(out groupshared uint Sh);
// expected-error@-1{{'out' and 'groupshared' cannot be used together for a parameter}}
void fn5(groupshared inout uint Sh);
// expected-error@-1{{'inout' and 'groupshared' cannot be used together for a parameter}}
void fn6(groupshared in uint Sh);
// expected-error@-1{{'in' and 'groupshared' cannot be used together for a parameter}}
void fn7(groupshared out uint Sh);
// expected-error@-1{{'out' and 'groupshared' cannot be used together for a parameter}}

void fn8() {
  fn1(SharedData);
// expected-warning@-1{{Passing groupshared variable to a parameter annotated with inout. See 'groupshared' parameter annotation added in 202x}}
}

template<typename T>
T fn9(inout groupshared T A) {
// expected-error@-1{{'inout' and 'groupshared' cannot be used together for a parameter}}
  return A;
}
template<typename T>
T fn10(in groupshared T A) {
// expected-error@-1{{'in' and 'groupshared' cannot be used together for a parameter}}
  return A;
}
template<typename T>
T fn11(out groupshared T A) {
// expected-error@-1{{'out' and 'groupshared' cannot be used together for a parameter}}
  return A;
}
