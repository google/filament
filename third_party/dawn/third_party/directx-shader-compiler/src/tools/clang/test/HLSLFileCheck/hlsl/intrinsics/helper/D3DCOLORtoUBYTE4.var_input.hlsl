// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Test the following:
// 1) Input to D3DCOLORtoUBYTE4() is multipled by 255.001953
// 2) No rounding is applied

// CHECK: fmul
// CHECK: 0x406FE01000000000
// CHECK: fmul
// CHECK: 0x406FE01000000000
// CHECK: fmul
// CHECK: 0x406FE01000000000
// CHECK: fmul
// CHECK: 0x406FE01000000000
// CHECK-NOT: Round
// CHECK-NOT: Round
// CHECK-NOT: Round
// CHECK-NOT: Round
// CHECK: fptosi
// CHECK: fptosi
// CHECK: fptosi
// CHECK: fptosi


int4 main (float4 f : IN): OUT
{
    return D3DCOLORtoUBYTE4(f);
}