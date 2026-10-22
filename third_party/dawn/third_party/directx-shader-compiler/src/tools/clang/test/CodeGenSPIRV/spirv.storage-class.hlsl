// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct VSOut {
    float4 out1: C;
    float4 out2: D;
};

static float4 sgVar; // Privte

// Note: The entry function in the source code is treated as a normal function.
// Another wrapper function take care of handling stage input/output variables.
// and calling the source code entry function. So there are no Input/Ouput
// storage class involved in the following.

VSOut main(float4 input: A /* Function */, uint index: B /* Function */) {
    static float4 slVar; // Private

    VSOut ret; // Function

// CHECK:      OpAccessChain %_ptr_Function_float %input
// CHECK:      OpAccessChain %_ptr_Private_float %sgVar
// CHECK:      OpAccessChain %_ptr_Private_float %slVar
// CHECK:      OpAccessChain %_ptr_Function_float %ret %int_0 {{%[0-9]+}}
    ret.out1[index] = input[index] + sgVar[index] + slVar[index];

    return ret;
}
