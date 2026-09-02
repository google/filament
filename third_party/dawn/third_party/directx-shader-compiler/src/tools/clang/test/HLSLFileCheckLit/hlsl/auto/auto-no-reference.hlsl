// RUN: %dxc -T cs_6_0 -HV 202x -verify %s

// Test that 'auto' cannot be used to declare reference types in HLSL.
// References are unsupported in HLSL.

int gVal = 42;

[numthreads(1,1,1)]
void main() {
    auto& ref = gVal; // expected-error {{references are unsupported in HLSL}}
}
