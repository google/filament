// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s | FileCheck %s

// CHECK: define void @main()

struct S1 {
    float a;
};

struct S2 {
    S1 s1;
    Texture2D<float> tex;
};

void main(float4 pos: SV_Position) {
    S1 s1;
    S2 s2;
    s2.s1 = s1;
}
