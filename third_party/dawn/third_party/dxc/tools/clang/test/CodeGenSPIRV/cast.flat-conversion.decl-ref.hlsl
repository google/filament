// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct A {
    float a;
    uint b;
    float GetA() { return a; }
};

ConstantBuffer<A> foo : register(b0, space2);

void main(out float4 col : SV_Target0) {
// CHECK:        [[foo:%[0-9]+]] = OpLoad %type_ConstantBuffer_A %foo
// CHECK:      [[foo_a:%[0-9]+]] = OpCompositeExtract %float [[foo]] 0
// CHECK:      [[foo_b:%[0-9]+]] = OpCompositeExtract %uint [[foo]] 1
// CHECK: [[foo_rvalue:%[0-9]+]] = OpCompositeConstruct %A [[foo_a]] [[foo_b]]
// CHECK:                       OpStore %temp_var_A [[foo_rvalue]]
// CHECK:                       OpFunctionCall %float %A_GetA %temp_var_A
    col.x = foo.GetA();
}
