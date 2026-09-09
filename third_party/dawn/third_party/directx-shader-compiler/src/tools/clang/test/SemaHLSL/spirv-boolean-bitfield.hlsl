// REQUIRES: spirv
// RUN: %dxc -T cs_6_0 -E main -spirv -verify %s

struct Test {
    bool a: 1;
    int b;
};

// This should not have an error because it is externally visible.
RWStructuredBuffer<Test> buffer;
Test g_non_static_global;

// Global static variable
static Test g_t; // expected-error {{type 'Test' contains a boolean bitfield which is not supported for variables that are not externally visible when targeting SPIR-V}}

// Global static array
static Test g_t_arr[2]; // expected-error {{type 'Test' contains a boolean bitfield which is not supported for variables that are not externally visible when targeting SPIR-V}}

void foo(Test p) {} // expected-error {{type 'Test' contains a boolean bitfield which is not supported for variables that are not externally visible when targeting SPIR-V}}

void bar(Test p[2]) {} // expected-error {{type 'Test' contains a boolean bitfield which is not supported for variables that are not externally visible when targeting SPIR-V}}

[numthreads(1, 1, 1)]
void main() {
    // Local variable
    Test t; // expected-error {{type 'Test' contains a boolean bitfield which is not supported for variables that are not externally visible when targeting SPIR-V}}

    // Local array
    Test t_arr[2]; // expected-error {{type 'Test' contains a boolean bitfield which is not supported for variables that are not externally visible when targeting SPIR-V}}
}
