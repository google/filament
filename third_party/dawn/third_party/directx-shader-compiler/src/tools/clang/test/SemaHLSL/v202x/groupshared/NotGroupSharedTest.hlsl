// RUN: %dxc -T cs_6_3 -enable-16bit-types -HV 202x -verify %s

void fn1(groupshared half Sh) {
// expected-note@-1{{candidate function not viable: 1st argument ('half') is in address space 0, but parameter must be in address space 3}}
  Sh = 5;
}

template<typename T>
T fn2(groupshared T Sh) {
// expected-note@-1{{candidate template ignored: can't deduce a type for 'T' that would make '__attribute__((address_space(3))) T' equal 'half'}}
  return Sh;
}

[numthreads(4, 1, 1)]
void main(uint3 TID : SV_GroupThreadID) {
  half tmp = 1.0;
  fn1(tmp);
  // expected-error@-1{{no matching function for call to 'fn1'}}
  fn2(tmp);
  // expected-error@-1{{no matching function for call to 'fn2'}}
}
