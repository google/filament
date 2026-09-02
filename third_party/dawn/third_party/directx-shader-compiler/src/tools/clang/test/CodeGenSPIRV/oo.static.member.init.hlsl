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

// CHECK-DAG: %int_5 = OpConstant %int 5
// CHECK-DAG: %int_6 = OpConstant %int 6

int foo(int val) { return val; }

int main() : A {
// CHECK-LABEL: %src_main = OpFunction
    return
        // CHECK: OpStore [[FOO_PARAM_0:%.*]] %int_5
        // CHECK-NEXT: [[FOO_RETURN_0:%.*]] = OpFunctionCall %int %foo [[FOO_PARAM_0]]
        foo(S::FIVE) +
        // CHECK: OpStore [[FOO_PARAM_1:%.*]] %int_6
        // CHECK-NEXT: [[FOO_RETURN_1:%.*]] = OpFunctionCall %int %foo [[FOO_PARAM_1]]
        foo(S::SIX) +
        // CHECK: OpStore [[FOO_PARAM_2:%.*]] %int_5
        // CHECK-NEXT: [[FOO_RETURN_2:%.*]] = OpFunctionCall %int %foo [[FOO_PARAM_2]]
        foo(T::FIVE) +
        // CHECK: OpStore [[FOO_PARAM_3:%.*]] %int_6
        // CHECK-NEXT: [[FOO_RETURN_3:%.*]] = OpFunctionCall %int %foo [[FOO_PARAM_3]]
        foo(T::SIX);
}
