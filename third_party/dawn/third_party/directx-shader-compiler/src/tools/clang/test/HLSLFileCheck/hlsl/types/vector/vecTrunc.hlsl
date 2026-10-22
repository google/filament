// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

// CHECK: cannot truncate lvalue vector/matrix

void test(out float a) {
   a = 2;
}

float4 main() : SV_Target
{
    float4 x;
    x.y = 3;
    test(x);
    return x;
}