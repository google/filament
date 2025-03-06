// RUN: %dxr -E main -remove-unused-globals %s | FileCheck %s
// RUN: %dxr -E main -remove-unused-functions %s | FileCheck %s -check-prefix=KEEP_GLOBAL

// CHECK-NOT:struct
// CHECK-NOT:ConstantBuffer.
// CHECK:float main

// KEEP_GLOBAL:struct
// KEEP_GLOBAL:ConstantBuffer
// KEEP_GLOBAL:float main


struct ST
{
 uint t;
};
ConstantBuffer<ST> cbv;

float main() : SV_Target {
  return 1;
}