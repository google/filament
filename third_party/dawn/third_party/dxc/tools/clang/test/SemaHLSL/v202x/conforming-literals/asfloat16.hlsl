// RUN: %dxc -T lib_6_3 -HV 202x -verify -enable-16bit-types %s

void fn() {
  half H = asfloat16(1.0);
  // expected-error@-1{{no matching function for call to 'asfloat16'}}
  // expected-note@-2{{candidate function not viable: no known conversion from 'float' to 'half' for 1st argument}}
}
