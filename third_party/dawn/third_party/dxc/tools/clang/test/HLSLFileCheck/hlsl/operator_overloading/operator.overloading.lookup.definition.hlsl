// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s | FileCheck %s

// CHECK: define void @main()

struct S {
    float a;

    float operator-(float x) {
        return a - x;
    }

    float operator+(float x) {
        return a + x;
    }
};

struct Number {
    int n;

    int operator+(float x) {
        return n + x;
    }
};

int main(float4 pos: SV_Position) : SV_Target {
    S s1 = {0.2};
    S s2 = {0.2};
    float f = s1 + 0.1;

    Number a = {pos.x};
    Number b = {pos.y};
    a.n = b + pos.x;
    return b + pos.y;
}
