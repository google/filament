// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Test the following:
// 1) Input i to D3DCOLORtoUBYTE4() is swizzled to i.zyxw
// 2) Swizzled value is multiplied with constant 255.001953

// CHECK: 7650058
// CHECK: 5100039
// CHECK: 2550019
// CHECK: 10200078

int4 main (): OUT
{
    return D3DCOLORtoUBYTE4(float4(10000, 20000, 30000, 40000));
}

