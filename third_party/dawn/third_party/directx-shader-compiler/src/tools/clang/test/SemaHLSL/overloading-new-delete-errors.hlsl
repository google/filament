// RUN: %dxc -Tlib_6_3  -Wno-unused-value  -verify -HV 2021 %s
// RUN: %dxc -Tvs_6_0  -Wno-unused-value  -verify -HV 2021 %s

// This test checks that when we overload new or delete operator
// dxcompiler generates error and no crashes are observed.

struct S
{
    float foo;
    S operator new(int size) { // expected-error {{overloading 'operator new' is not allowed}}
        return (S)0;
    }
    void operator delete(int ptr) { // expected-error {{overloading 'operator delete' is not allowed}}
        (void) ptr;
    }
};

[shader("vertex")]
void main() {
    S a = new S(); // expected-error {{'new' is a reserved keyword in HLSL}}
    delete a; // expected-error {{'delete' is a reserved keyword in HLSL}}
}
