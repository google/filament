// RUN: %dxc -T ps_6_0 -E main -fcgl %s -spirv | FileCheck %s

struct s
{
    float field;
};

struct subscriptable
{
    s operator[](const uint64_t index)
    {
        return s(123.0);
    }
};

float4 main() : SV_TARGET
{
    subscriptable o;

    float f;
// CHECK: %param_var_index = OpVariable %_ptr_Function_ulong Function
// CHECK: OpStore %param_var_index %ulong_123
// CHECK: [[op_result:%[0-9]+]] = OpFunctionCall %s %subscriptable_operator_Subscript %o %param_var_index
// CHECK: OpStore %temp_var_s [[op_result]]
// CHECK: {{%[0-9]+}} = OpAccessChain %_ptr_Function_float %temp_var_s %int_0
    f = o[123].field;
    return f;
}
