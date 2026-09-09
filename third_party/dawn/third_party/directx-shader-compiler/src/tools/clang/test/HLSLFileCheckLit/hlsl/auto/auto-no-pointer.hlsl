// RUN: %dxc -T cs_6_0 -HV 202x -verify %s

// Test that 'auto' cannot be used to declare pointer types in HLSL.
// Pointers are unsupported in HLSL.

[numthreads(1,1,1)]
void main() {
    int x = 1;
    auto* ptr = &x; // expected-error {{pointers are unsupported in HLSL}}
}
