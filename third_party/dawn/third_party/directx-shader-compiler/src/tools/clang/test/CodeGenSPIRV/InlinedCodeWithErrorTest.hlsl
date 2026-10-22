// RUN: not %dxc -T ps_6_0 -E PSMain -fcgl  %s -spirv 2>&1 | FileCheck %s

struct PSInput
{
        float4 color : COLOR;
};

// CHECK: error: cannot initialize return object of type 'float4' with an lvalue of type 'PSInput'
float4 PSMain(PSInput input) : SV_TARGET
{
        return input;
}