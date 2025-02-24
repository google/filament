// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S {
    static const int FIVE = 5;
    static const int SIX;
};

const int S::SIX = 6;

class T {
    static const int FIVE = 5;
    static const int SIX;
};

const int T::SIX = 6;

int foo(int val) { return val; }

// CHECK:   %FIVE = OpVariable %_ptr_Private_int Private
// CHECK:    %SIX = OpVariable %_ptr_Private_int Private
// CHECK: %FIVE_0 = OpVariable %_ptr_Private_int Private
// CHECK:  %SIX_0 = OpVariable %_ptr_Private_int Private
int main() : A {
// CHECK-LABEL: %main = OpFunction

// CHECK: OpStore %FIVE %int_5
// CHECK: OpStore %SIX %int_6
// CHECK: OpStore %FIVE_0 %int_5
// CHECK: OpStore %SIX_0 %int_6
// CHECK: OpFunctionCall %int %src_main

// CHECK-LABEL: %src_main = OpFunction

// CHECK: OpLoad %int %FIVE
// CHECK: OpLoad %int %SIX
// CHECK: OpLoad %int %FIVE_0
// CHECK: OpLoad %int %SIX_0
    return foo(S::FIVE) + foo(S::SIX) + foo(T::FIVE) + foo(T::SIX);
}
