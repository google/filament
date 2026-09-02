// RUN: %dxc -T cs_6_0 -E main -HV 202x -fcgl %s -spirv | FileCheck %s

// Test that the 'auto' keyword works correctly in template contexts when
// targeting SPIR-V:
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

// CHECK: [[INT:%[a-zA-Z0-9_]+]] = OpTypeInt 32 1
// CHECK: [[FLOAT:%[a-zA-Z0-9_]+]] = OpTypeFloat 32
// CHECK: [[PTR_INT:%_ptr_Function_int]] = OpTypePointer Function [[INT]]
// CHECK: [[PTR_FLOAT:%_ptr_Function_float]] = OpTypePointer Function [[FLOAT]]

// Auto-deduced locals in main() instantiated for int and float.
// CHECK: %a = OpVariable [[PTR_INT]] Function
// CHECK: %b = OpVariable [[PTR_FLOAT]] Function
// CHECK: %c = OpVariable [[PTR_INT]] Function
// CHECK: %d = OpVariable [[PTR_FLOAT]] Function

// Auto inside template bodies retains correct types after instantiation.
// CHECK: %result = OpVariable [[PTR_INT]] Function
// CHECK: %result_0 = OpVariable [[PTR_FLOAT]] Function
// CHECK: %zero = OpVariable [[PTR_INT]] Function
// CHECK: %clamped = OpVariable [[PTR_INT]] Function
// CHECK: %zero_0 = OpVariable [[PTR_FLOAT]] Function
// CHECK: %clamped_0 = OpVariable [[PTR_FLOAT]] Function

[numthreads(1,1,1)]
void main() {
    // auto variables assigned from template instantiation results.
    auto a = square(5);           // instantiated as int
    auto b = square(2.5f);        // instantiated as float
    auto c = clampPositive(-3);   // instantiated as int
    auto d = clampPositive(1.5f); // instantiated as float

    output[0] = (float)a + b + (float)c + d;
}
