// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// Compositing a struct by casting from its single member

struct S {
    float4 val;
};

struct T {
    S val;
};

float4 main(float4 a: A) : SV_Target {
// CHECK:      [[a:%[0-9]+]] = OpLoad %v4float %a
// CHECK-NEXT: [[s:%[0-9]+]] = OpCompositeConstruct %S [[a]]
// CHECK-NEXT:              OpStore %s [[s]]
    S s = (S)a;

// CHECK:           [[s_0:%[0-9]+]] = OpLoad %S %s
// CHECK-NEXT:    [[val:%[0-9]+]] = OpCompositeExtract %v4float [[s_0]] 0
// CHECK-NEXT:      [[s_1:%[0-9]+]] = OpCompositeConstruct %S [[val]]
// CHECK-NEXT:      [[t:%[0-9]+]] = OpCompositeConstruct %T [[s_1]]
// CHECK-NEXT:                   OpStore %t [[t]]
    T t = (T)s;

    return s.val + t.val.val;
}
