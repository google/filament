// RUN: %dxc -T cs_6_0 -E main - %s -verify
// RUN: %dxc -T cs_6_0 -E main -HV 202x -fcgl %s | FileCheck %s

// Test that the 'auto' keyword can be used to declare variables with inferred
// types from initialization expressions in HLSL.

// CHECK-LABEL: define void @main()
// CHECK: [[A:%[a-zA-Z0-9_]+]] = alloca i32
// CHECK: [[B:%[a-zA-Z0-9_]+]] = alloca float
// CHECK: [[C:%[a-zA-Z0-9_]+]] = alloca i32
// CHECK: [[D:%[a-zA-Z0-9_]+]] = alloca <4 x float>
// CHECK: {{.*}} = alloca i32
// CHECK: {{.*}} = alloca float
// CHECK: {{.*}} = alloca <4 x float>
// CHECK: {{.*}} = alloca i32
// CHECK: {{.*}} = alloca float
// CHECK: store i32 1, i32* [[A]]
// CHECK: store float 2.000000e+00, float* [[B]]
// CHECK: store i32 1, i32* [[C]]

RWBuffer<float> output : register(u0);

[numthreads(1,1,1)]
void main() {
    // Auto deduces int from integer literal
    // expected-warning@+1 {{'auto' type specifier is a HLSL 202x extension}}
    auto a = 1;
    // Auto deduces float from float literal
    // expected-warning@+1 {{'auto' type specifier is a HLSL 202x extension}}
    auto b = 2.0f;
    // Auto deduces bool (stored as i32) from bool literal
    // expected-warning@+1 {{'auto' type specifier is a HLSL 202x extension}}
    auto c = true;
    // Auto deduces float4 from vector type
    // expected-warning@+1 {{'auto' type specifier is a HLSL 202x extension}}
    auto d = float4(1.0f, 2.0f, 3.0f, 4.0f);

    // Auto from arithmetic expressions
    // expected-warning@+1 {{'auto' type specifier is a HLSL 202x extension}}
    auto sum = a + a;
    // expected-warning@+1 {{'auto' type specifier is a HLSL 202x extension}}
    auto product = b * b;

    // expected-warning@+1 {{'auto' type specifier is a HLSL 202x extension}}
    auto mulAdd = mad(d, d, d);

    // expected-warning@+1 {{'auto' type specifier is a HLSL 202x extension}} 
    auto m = min(sum, c);

    // expected-warning@+1 {{'auto' type specifier is a HLSL 202x extension}} 
    auto dP = dot(d, d);

    // Use the values to prevent dead-code elimination
    output[0] = (float)sum + product + d.x + (float)c;
}
