// RUN: %dxc -T lib_6_3 -HV 2021 -verify %s

groupshared uint SharedData;

void fn1(groupshared uint Sh) {
// expected-warning@-1{{Support for groupshared parameter annotation not added until HLSL 202x}}
  Sh = 5;
}

void fn2() {
  fn1(SharedData);
}

template<typename T>
T fn3(groupshared T A) {
// expected-warning@-1{{Support for groupshared parameter annotation not added until HLSL 202x}}
  return A;
}
