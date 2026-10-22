// RUN: %dxc -E main -T cs_6_2 -enable-16bit-types -HV 202x -fcgl %s | FileCheck %s

groupshared uint16_t SharedData;

// Make sure the mangling changes for groupshared parameter annotation is reflected in the function signature (the first G)
// The mangled function signature for void fn(uint16_t Sh) should be @"\01?fn1@@YAXAAG@Z" (without the extra G)
// CHECK-LABEL: @"\01?fn1@@YAXAGAG@Z"
// CHECK: store i16 5, i16 addrspace(3)* %Sh, align 4
void fn1(groupshared uint16_t Sh) {
  Sh = 5;
}

[numthreads(4, 1, 1)]
void main(uint3 TID : SV_GroupThreadID) {
  fn1(SharedData);
}
