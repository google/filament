// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
// CHECK: %int_1 = OpConstant %int 1
    int1 c_int1 = int1(1);
// CHECK: {{%[0-9]+}} = OpConstantComposite %v2int %int_1 %int_2
    int2 c_int2 = int2(1, 2);
// CHECK: {{%[0-9]+}} = OpConstantComposite %v3int %int_1 %int_2 %int_3
    int3 c_int3 = int3(1, 2, 3);
// CHECK: {{%[0-9]+}} = OpConstantComposite %v4int %int_1 %int_2 %int_3 %int_4
    int4 c_int4 = int4(1, 2, 3, 4);
}
