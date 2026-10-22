// RUN: %dxc -T cs_6_0 -HV 202x -fcgl %s | FileCheck %s

// Test that the 'auto' keyword works correctly in template contexts in HLSL:
//  - auto variables inside template function bodies
//  - auto variables assigned from results of template function instantiations



RWBuffer<float> output : register(u0);

// Template with explicit return type; auto is used inside the body.
template<typename T>
T square(T val) {
    auto result = val * val;
    return result;
}

// Template with explicit return type using a conditional expression.
template<typename T>
T clampPositive(T val) {
    auto zero = (T)0;
    auto clamped = val < zero ? zero : val;
    return clamped;
}

// CHECK-LABEL: define void @main()
// CHECK: {{.*}} = alloca i32
// CHECK: {{.*}} = alloca float
// CHECK: {{.*}} = alloca i32
// CHECK: {{.*}} = alloca float

[numthreads(1,1,1)]
void main() {
    // auto variables assigned from template instantiation results.
    auto a = square(5);         // instantiated as int
    auto b = square(2.5f);      // instantiated as float
    auto c = clampPositive(-3); // instantiated as int, result = 0
    auto d = clampPositive(1.5f); // instantiated as float

    output[0] = (float)a + b + (float)c + d;
}
