// RUN: %dxc -T lib_6_3 -enable-16bit-types -HV 202x -verify %s

groupshared uint16_t SharedData;

void fn1(groupshared half Sh) {
// expected-note@-1{{candidate function not viable: 1st argument ('half') is in address space 0, but parameter must be in address space 3}}
  Sh = 5;
}

template<typename T>
T fnT(groupshared T A) {
// expected-note@-1{{candidate function [with T = half] not viable: 1st argument ('half') is in address space 0, but parameter must be in address space 3}}
  return A;
}

void fn2() {
  fn1((half)SharedData);
  // expected-error@-1{{no matching function for call to 'fn1'}}
  // not sure why someone would write this but make sure templates do something sane.
  fnT<half>((half)SharedData);
  // expected-error@-1{{no matching function for call to 'fnT'}}
}
