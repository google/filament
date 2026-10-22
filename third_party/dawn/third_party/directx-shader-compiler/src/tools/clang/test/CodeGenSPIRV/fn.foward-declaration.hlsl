// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

float4 foo(float4 input);

float4 main(float4 input: A) : SV_Target0
{
    return foo(input);
}

float4 foo(float4 input)
{
    return input;
}

// CHECK:  %src_main = OpFunction %v4float None {{%[0-9]+}}

// CHECK:              OpFunctionCall %v4float %foo

// CHECK:       %foo = OpFunction %v4float None {{%[0-9]+}}
