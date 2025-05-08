// RUN: %dxc dxc -T cs_6_6 -E Main -spirv %s -fcgl | FileCheck %s


// Make sure no code is generated for the cast to void.

// CHECK: %src_Main = OpFunction %void None
// CHECK-NEXT: OpLabel
// CHECK-NEXT: %x = OpVariable
// CHECK-NEXT: OpStore %x %false
// CHECK-NEXT: OpReturn
// CHECK-NEXT: OpFunctionEnd

[numthreads(1, 1, 1)]
void Main()
{
    bool x = false;
    (void)x;
}
