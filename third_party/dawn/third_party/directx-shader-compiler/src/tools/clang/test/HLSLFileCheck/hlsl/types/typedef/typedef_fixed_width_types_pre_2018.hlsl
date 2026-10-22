// RUN: %dxc -E main -T ps_6_0 -HV 2017 %s | FileCheck %s

// Checking that typedef for fixed width types for HLSL before 2018 works properly

// CHECK: type { i32, i32, i32, i32, float, float, float }

typedef uint uint16_t;
typedef uint uint32_t;
typedef int int16_t;
typedef int int32_t;
typedef float float16_t;
typedef float float32_t;
typedef float float64_t;

uint16_t i1;
uint32_t i2;
int16_t i3;
int32_t i4;
float16_t f1;
float32_t f2;
float64_t f3;

float4 main() : SV_Target {
    return i1 + i2 + i3 + i4 + f1 + f2 + f3;
}