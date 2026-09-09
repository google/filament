// RUN: %dxc -T lib_6_6 -fcgl  %s -spirv | FileCheck %s

// CHECK: OpEntryPoint Fragment %x "x" %out_var_SV_TARGET
// CHECK: OpEntryPoint Fragment %y "y" %out_var_SV_TARGET_0
// CHECK: OpEntryPoint Fragment %z "z" %out_var_SV_TARGET1

[shader("pixel")]
float4 x(): SV_TARGET
{
    return float4(1.0, 0.0, 0.0, 0.0);
}

[shader("pixel")]
float4 y(): SV_TARGET
{
    return float4(0.0, 1.0, 0.0, 0.0);
}

[shader("pixel")]
float4 z(): SV_TARGET1
{
    return float4(0.0, 0.0, 1.0, 0.0);
}
