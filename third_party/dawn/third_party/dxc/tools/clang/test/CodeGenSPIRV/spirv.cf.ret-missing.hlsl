// RUN: %dxc -T vs_6_0 -E main -Wno-return-type -fcgl  %s -spirv | FileCheck %s

// CHECK:[[undef:%[0-9]+]] = OpUndef %float

float main(bool a: A) : B {
    if (a) return 1.0;
    // No return value for else

// CHECK:      %if_merge = OpLabel
// CHECK-NEXT: OpReturnValue [[undef]]
// CHECK-NEXT: OpFunctionEnd
}
