// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main(out float a: A, out float b: B) {
// CHECK-LABEL: %bb_entry = OpLabel
    if (3 + 5) {
// CHECK-NEXT: OpStore %a %float_1
        a = 1.0;
    } else {
        a = 0.0;
    }

    if (4 + 3 > 7 || 4 + 3 < 8) {
// CHECK-NEXT: OpStore %b %float_2
        b = 2.0;
    }

    if (4 + 3 > 7 && true) {
        b = 0.0;
    }

    if (true)
        ;

    if (false) {}
// CHECK-NEXT: OpReturn
}
