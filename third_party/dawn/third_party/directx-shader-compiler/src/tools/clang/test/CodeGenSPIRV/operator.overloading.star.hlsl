// RUN: %dxc -T ps_6_0 -E main -HV 2021 -fcgl  %s -spirv | FileCheck %s

// CHECK:           %a = OpVariable %_ptr_Function_Number Function
// CHECK:           %b = OpVariable %_ptr_Function_Number Function
// CHECK: %param_var_x = OpVariable %_ptr_Function_Number Function
// CHECK:   [[b:%[a-zA-Z0-9_]+]] = OpLoad %Number %b
// CHECK:                OpStore %param_var_x [[b]]
// CHECK: [[call:%[a-zA-Z0-9_]+]] = OpFunctionCall %int %Number_operator_Star %a %param_var_x
// CHECK:                OpReturnValue [[call]]

// CHECK: %Number_operator_Star = OpFunction %int None
// CHECK:  %param_this = OpFunctionParameter %_ptr_Function_Number
// CHECK:           %x = OpFunctionParameter %_ptr_Function_Number

struct Number {
    int n;

    int operator*(Number x) {
        return x.n * n;
    }
};

int main(float4 pos: SV_Position) : SV_Target {
    Number a = {pos.x};
    Number b = {pos.y};
    return a * b;
}
