// RUN: %dxc -T lib_6_3 -HV 202x -verify %s

export void fn1(groupshared uint Sh) {
// expected-error@-1{{groupshared and export/noinline cannot be used together for a parameter}}
  Sh = 5;
}

[noinline] void fn2(groupshared uint Sh) {
// expected-error@-1{{groupshared and export/noinline cannot be used together for a parameter}}
  Sh = 6;
}

template<typename T>
void fn3(groupshared T A, groupshared T B) {
  A = B;
}

export template void fn3<uint>(groupshared uint A, groupshared uint B);
// expected-error@-1{{'template' is a reserved keyword in HLSL}}
template [noinline] void fn3<float>(groupshared float A, groupshared float B);
// expected-error@-1{{use of undeclared identifier 'noinline'}}
// expected-error@-2{{expected unqualified-id}}
