// RUN: %dxc -T ps_6_6 -E PSMain %s -spirv | FileCheck %s

struct PSInput
{   
    static const uint Val;
    uint Func() { return Val; }
};

static const uint PSInput::Val = 3;

// CHECK: OpStore %out_var_SV_Target0 %uint_3
uint PSMain(PSInput input) : SV_Target0
{   
    return input.Func();
}
