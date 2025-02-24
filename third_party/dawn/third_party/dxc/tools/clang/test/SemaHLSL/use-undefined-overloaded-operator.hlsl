// RUN: %dxc -Tlib_6_3  -Wno-unused-value  -verify -HV 2021 %s
// RUN: %dxc -Tps_6_0  -Wno-unused-value  -verify -HV 2021 %s

// This test checks that when we use undefined overloaded operator
// dxcompiler generates error and no crashes are observed.

struct S1 {
    float a;

    float operator+(float x) {
        return a + x;
    }
};

struct S2 {
    S1 s1;
};

[shader("pixel")]
void main(float4 pos: SV_Position) {
    S1 s1;
    S2 s2;
    pos.x = s2.s1 + 0.1;
    pos.x = s2.s1 + s1; // expected-error {{invalid operands to binary expression ('S1' and 'S1')}}
}
