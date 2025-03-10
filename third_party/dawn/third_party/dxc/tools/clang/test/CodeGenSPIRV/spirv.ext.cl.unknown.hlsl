// RUN: not %dxc -T ps_6_1 -E main -fspv-extension=MyExtension -fcgl  %s -spirv  2>&1 | FileCheck %s

float4 main(uint viewid: SV_ViewID) : SV_Target {
    return viewid;
}

// CHECK: error: unknown SPIR-V extension 'MyExtension'
// CHECK: note: known extensions are
