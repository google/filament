// RUN: %dxc -Emain -Tps_6_0 %s | %opt -S -hlsl-dxil-force-early-z | %FileCheck %s

// Just check that the an appropriately-formed line (which contains global flags) has the "8" meaning force-early-z:
// CHECK: !{i32 0, i64 8}

[RootSignature("")]
float4 main() : SV_Target {
    return float4(0,0,0,0);
}