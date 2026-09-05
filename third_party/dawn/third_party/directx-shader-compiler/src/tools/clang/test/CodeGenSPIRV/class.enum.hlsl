// RUN: %dxc -T ps_6_2 -E PSMain -fcgl  %s -spirv | FileCheck %s

// The value for the enum is stored in a variable, which will be optimized
// away when optimizations are enabled.
// CHECK: [[enum_var:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Private_int Private %int_1
struct TestStruct {
    enum EnumInTestStruct {
        A = 1,
    };
};

// CHECK: %testFunc = OpFunction %int
// CHECK-NEXT: OpLabel
// CHECK-NEXT: [[ld:%[a-zA-Z0-9_]+]] = OpLoad %int [[enum_var]]
// CHECK-NEXT: OpReturnValue [[ld]]
TestStruct::EnumInTestStruct testFunc() {
    return TestStruct::A;
}

uint PSMain() : SV_TARGET
{
    TestStruct::EnumInTestStruct i = testFunc();
    return i;
}
